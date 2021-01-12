#ifndef __KEYVALUESERVER_HPP_
#define __KEYVALUESERVER_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "DBclient.hpp"
#include "keyvalueclient.hpp"
#include "cache.hpp"

#include <grpcpp/grpcpp.h>

#include "assign4.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using assign4::KeyValueStore;
using assign4::Request;
using assign4::Response;

struct kv_pair {
  std::string key;
  std::string value;
};

static const kv_pair kvs_map[] = {
    
};

struct fetchaddr_state {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready;
    std::string other_super_node_address;
};

// Logic and data behind the server's behavior.
class KeyValueStoreServiceImpl final : public KeyValueStore::Service {
    public:
        explicit KeyValueStoreServiceImpl(std::vector<std::string> &_child_addresses,
                fetchaddr_state *_state, bool _is_child) {
            state = _state;
            is_child = _is_child;
            child_addresses = _child_addresses;
            int cache_size = is_child ? CHILD_MAX_CACHE_SIZE : SUPERNODE_MAX_CACHE_SIZE;
            cache = new Cache(cache_size);
            if (is_child) {
                assert(child_addresses.size() == 1);
                auto dbserver_address = child_addresses[0];
                dbclient = new DBClient(dbserver_address);
            } else {
                ConnectToChilds();
            }   
        }

        ~KeyValueStoreServiceImpl() {
            delete cache;
        }

        void ConnectToChilds(void) {
            for (auto &child_address: child_addresses) {
                KeyValueStoreClient *child = new KeyValueStoreClient(child_address);
                child_clients.push_back(child);
                map_results.push_back(std::unordered_map<std::string, std::string>());
            }
        }

        Status GetValues(ServerContext* context,
                    ServerReaderWriter<Response, Request>* stream) override {
            Request request;
            while (stream->Read(&request)) {
                Response response;
                response.set_res(_GetValues(request.req()));
                stream->Write(response);
            }
            return Status::OK;
        }

        std::string _GetValues(std::string key) {
            // check cache
            std::string value;
            value = cache->CheckCache(key);
            // cache hit
            if (value != "\0") 
                return value;
            // cache miss
            if (is_child) {
                assert(dbclient != NULL);
                value = dbclient->AccessDB(key);
            } else {
                value = AskChild(key);
            }
            // update cache
            cache->update(key, value);
            return value;
        }

        std::string AskChild(std::string key) {
            std::unordered_set<std::string> target;
            target.insert(key);
            for (auto &map_result: map_results) {
                map_result.clear();
            }
            // Ask to child nodes
            std::vector<std::thread *> child_threads;
            for (int i = 0; i < child_clients.size(); i++) {
                std::thread *child_thread = new std::thread(&KeyValueStoreClient::GetValues, 
                        child_clients[i], std::ref(target), std::ref(map_results[i]));
                child_threads.push_back(child_thread);
            }
            // reap child GetValue tasks
            for (auto &child_thread: child_threads) {
                child_thread->join();
                delete child_thread;
            }
            for (auto map_result: map_results) {
                for (auto it : map_result) {
                    return it.second;
                }
            }
            return "\0";
        }

        Status SendAddress(ServerContext* context, const Request* request,
            Response *response) override {
            std::unique_lock<std::mutex> lck(state->mtx);
            state->ready = true;
            assert (state != NULL);
            state->other_super_node_address = request->req();
            response->set_res("OK");
            recved_other_super_node_address = true;
            state->cv.notify_all();
            return Status::OK;
        }

        bool recved_other_super_node_address = false;
        fetchaddr_state *state = NULL;
        DBClient *dbclient = NULL;
        bool is_child = false;
        std::vector<KeyValueStoreClient *> child_clients;
        std::vector<std::unordered_map<std::string, std::string> > map_results;
        std::vector<std::string> child_addresses;
        Cache *cache;
};

class KeyValueStoreServer {
    public:
        KeyValueStoreServer(const grpc::string &port, std::vector<std::string> &child_addresses,
            fetchaddr_state *state, bool is_child) {
            KeyValueStoreServiceImpl service(child_addresses, state, is_child);
            std::string server_address("0.0.0.0:" + port);
            ServerBuilder builder;
            // Listen on the given address without any authentication mechanism.
            builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
            // Register "service" as the instance through which we'll communicate with
            // clients. In this case, it corresponds to an *synchronous* service.
            builder.RegisterService(&service);
            // Finally assemble the server.
            std::unique_ptr<Server> server(builder.BuildAndStart());
            std::cout << "Server listening on " << server_address << std::endl;

            // Wait for the server to shutdown. Note that some other thread must be
            // responsible for shutting down the server for this call to ever return.
            server->Wait();
        }
};

#endif
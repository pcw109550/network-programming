#ifndef __KEYVALUECLIENT_HPP_
#define __KEYVALUECLIENT_HPP_
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "assert.h"
#include "sockserver.hpp"
#include "file.hpp"
#include "cache.hpp"

#include <grpcpp/grpcpp.h>

#include "assign4.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using assign4::KeyValueStore;
using assign4::Request;
using assign4::Response;

#define SUPERNODE_MAX_CACHE_SIZE 30720
#define CHILD_MAX_CACHE_SIZE     10240

class KeyValueStoreClient {
    public:
        KeyValueStoreClient(const grpc::string &target) {
            stub_ = KeyValueStore::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
        }

        ~KeyValueStoreClient(void) { }

        // Requests each key in the vector and displays the key and its corresponding
        // value as a pair
        static void GetValues(KeyValueStoreClient *client,
            std::unordered_set<std::string> &parse_result,
            std::unordered_map<std::string, std::string> &map) {
            // Context for the client. It could be used to convey extra information to
            // the server and/or tweak certain RPC behaviors.
            ClientContext context;
            auto stream = client->stub_->GetValues(&context);
            for (const auto& key : parse_result) {
                // Key we are sending to the server.
                Request request;
                request.set_req(key);
                stream->Write(request);

                // Get the value for the sent key
                Response response;
                stream->Read(&response);
                if (response.res().size() == 0 || (response.res().size() == 1 && response.res()[0] == '\0')) {
                    // Failed to find key
                } else {
                    map[key] = response.res();
                }
            }
            stream->WritesDone();
            Status status = stream->Finish();
            if (!status.ok()) {
                std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                std::cout << "RPC failed";
            }
        }

        void SendAddress(std::string& address) {
            ClientContext context;
            Request request;
            Response response;
            request.set_req(address);
            Status status = stub_->SendAddress(&context, request, &response);
            if (status.ok()) {
                assert(response.res() == "OK");
            } else {
                std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl << "RPC failed" << std::endl;
            }
        }

        void HandleClientConnection(char *server_port, std::vector<std::string> _child_addresses, 
            std::string _other_super_node_address) {
            child_addresses = _child_addresses;
            other_super_node_address = _other_super_node_address;
            ConnectToChilds();
            ConnectToOtherSuperNode();
            Cache cache(SUPERNODE_MAX_CACHE_SIZE);

            int listenfd = Open_listenfd(server_port);

            while (true) {
                std::cout << std::endl;
                std::cout << "[*] Receiving connection from client" << std::endl;
                for (auto &map_result: map_results) {
                    map_result.clear();
                }
                // make connection with client
                SockServer sockserver(listenfd);
                File file(sockserver.connfd, NULL);
                file.recv_file();
                std::cout << "Connected to client" << std::endl;
                file.parse_file();
                int total_key_num = file.parse_result.size();
                std::cout << "Total key number: " << total_key_num << std::endl;
                // check cache
                std::unordered_map<std::string, std::string> cache_map_result;
                std::unordered_set<std::string> parse_result;
                for (auto key: file.parse_result) {
                    auto value = cache.CheckCache(key);
                    if (value != "\0") {
                        cache_map_result[key] = value;
                    } else {
                        parse_result.insert(key);
                    }
                }
                std::cout << "Cache hit number: " << cache_map_result.size() << std::endl;
                // send to each childs concurrently using threads
                std::vector<std::thread *> child_threads;
                for (int i = 0; i < child_clients.size(); i++) {
                    std::thread *child_thread = new std::thread(&KeyValueStoreClient::GetValues, 
                            child_clients[i], std::ref(parse_result), std::ref(map_results[i]));
                    child_threads.push_back(child_thread);
                }
                // reap child GetValue tasks
                for (auto &child_thread: child_threads) {
                    child_thread->join();
                    delete child_thread;
                }
                // accumulate recved key value pairs
                std::unordered_map<std::string, std::string> current_map_result;
                for (auto map_result: map_results) {
                    for (auto it : map_result) {
                        parse_result.erase(it.first);
                        current_map_result[it.first] = it.second;
                    }
                }
                std::cout << "Current supernode found: " << current_map_result.size() << std::endl;
                // ask other supernode if mapping is not perfect
                std::unordered_map<std::string, std::string> other_map_result;
                if (parse_result.size () != 0) {
                    KeyValueStoreClient::GetValues(other_supernode_client, parse_result, other_map_result);
                    std::cout << "Other supernode found: " << other_map_result.size() << std::endl;
                }
                assert(current_map_result.size() + other_map_result.size() + cache_map_result.size() == total_key_num);
                // merge and decode and send back to client
                auto map_result = current_map_result;
                current_map_result.insert(other_map_result.begin(), other_map_result.end());
                current_map_result.insert(cache_map_result.begin(), cache_map_result.end());
                file.decode(current_map_result);
                file.send_decoded_file();
                std::cout << "[+] Send decoded result to client" << std::endl;
                // update cache
                for (auto it : current_map_result) {
                    cache.update(it.first, it.second);
                }
            }
        }

        void ConnectToChilds(void) {
            for (auto &child_address: child_addresses) {
                KeyValueStoreClient *child = new KeyValueStoreClient(child_address);
                child_clients.push_back(child);
                map_results.push_back(std::unordered_map<std::string, std::string>());
            }
        }

        void ConnectToOtherSuperNode(void) {
            other_supernode_client = new KeyValueStoreClient(other_super_node_address);
        }

    private:
        std::unique_ptr<KeyValueStore::Stub> stub_;
        std::vector<std::string> child_addresses;
        std::string other_super_node_address;
        std::vector<KeyValueStoreClient *> child_clients;
        KeyValueStoreClient *other_supernode_client;
        std::vector<std::unordered_map<std::string, std::string> > map_results;
};

#endif
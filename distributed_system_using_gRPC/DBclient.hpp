#ifndef __DBCLIENT_HPP_
#define __DBCLIENT_HPP_
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "assign4.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using assign4::Database;
using assign4::Request;
using assign4::Response;

class DBClient {
    public:
        explicit DBClient(const grpc::string &target) {
            stub_ = Database::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
        }

        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        std::string AccessDB(const std::string& user) {
            // Data we are sending to the server.
            Request request;
            request.set_req(user);

            // Container for the data we expect from the server.
            Response reply;

            // Context for the client. It could be used to convey extra information to
            // the server and/or tweak certain RPC behaviors.
            ClientContext context;

            // The producer-consumer queue we use to communicate asynchronously with the
            // gRPC runtime.
            CompletionQueue cq;

            // Storage for the status of the RPC upon completion.
            Status status;

            // stub_->PrepareAsyncSayHello() creates an RPC object, returning
            // an instance to store in "call" but does not actually start the RPC
            // Because we are using the asynchronous API, we need to hold on to
            // the "call" instance in order to get updates on the ongoing RPC.
            std::unique_ptr<ClientAsyncResponseReader<Response> > rpc(
                stub_->PrepareAsyncAccessDB(&context, request, &cq));

            // StartCall initiates the RPC call
            rpc->StartCall();

            // Request that, upon completion of the RPC, "reply" be updated with the
            // server's response; "status" with the indication of whether the operation
            // was successful. Tag the request with the integer 1.
            rpc->Finish(&reply, &status, (void*)1);
            void* got_tag;
            bool ok = false;
            // Block until the next result is available in the completion queue "cq".
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or the cq_ is shutting down.
            GPR_ASSERT(cq.Next(&got_tag, &ok));

            // Verify that the result from "cq" corresponds, by its tag, our previous
            // request.
            GPR_ASSERT(got_tag == (void*)1);
            // ... and that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().
            GPR_ASSERT(ok);

            // Act upon the status of the actual RPC.
            if (status.ok()) {
                return reply.res();
            } else {
                return "RPC failed";
            }
        }

    private:
        // Out of the passed in Channel comes the stub, stored here, our view of the
        // server's exposed services.
        std::unique_ptr<Database::Stub> stub_;
};

#endif
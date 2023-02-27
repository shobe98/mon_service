#include "statsclient.h"

#include<iostream>

using messages::RuntimeStats;
using messages::StatsRequest;
using messages::StatsResponse;

using grpc::Channel;
using grpc::CompletionQueue;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientAsyncReader;
using std::cout;
using std::endl;
using std::string;

StatsClient::StatsClient(std::shared_ptr<grpc::Channel> channel) : stub_(RuntimeStats::NewStub(channel)) {}


void StatsClient::GetStats() {
    
    AsyncClientCall* call = new AsyncClientCall;

    call->context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(10));

    call->request.set_timestamp(static_cast<long>(std::time(nullptr)));


    call->response_reader = stub_->PrepareAsyncGetStats(&call->context, call->request, &cq_);

    call->response_reader->StartCall();
    call->response_reader->Finish(&call->response, &call->status, (void*)call);
}

void StatsClient::AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    while (cq_.Next(&got_tag, &ok)) {
        AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

        GPR_ASSERT(ok); // TODO add error handling

        if(call->status.ok()) {
            std::cout << "Received response for " << call->request.timestamp() <<  " issued at "
             << call->response.timestamp() << "\t delta: " << call->response.timestamp() - call->request.timestamp() << std::endl;
        }
        else {
            std::cout << call->request.timestamp() << " has failed. Status: " << call->status.error_message() << std::endl;
        }

        delete call;
    }
    
};
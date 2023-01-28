#ifndef __STATSCLIENT__
#define __STATSCLIENT__


#include <grpc/grpc.h>
#include <grpcpp/channel.h>

#include "stats_server.grpc.pb.h"



class StatsClient {
    public:
        explicit StatsClient(std::shared_ptr<grpc::Channel>);

        void AsyncCompleteRpc();

        void GetStats();

    private:
        struct AsyncClientCall {
            messages::StatsResponse response;
            messages::StatsRequest request;
            grpc::ClientContext context;  
            grpc::Status status; 
            std::unique_ptr<grpc::ClientAsyncResponseReader<messages::StatsResponse>> response_reader;
        };

        std::unique_ptr<messages::RuntimeStats::Stub> stub_;
        grpc::CompletionQueue cq_;
};

#endif //__STATSCLIENT__
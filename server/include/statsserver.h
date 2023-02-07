#ifndef __STATSSERVER__
#define __STATSSERVER__

#include "stats_server.grpc.pb.h"

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using messages::RuntimeStats;
using messages::StatsRequest;
using messages::StatsResponse;


class StatsServer {
 public:
  ~StatsServer();
  void Run(std::string server_address = "0.0.0.0:50051");

 private:
  class CallData {
   public:
    CallData(RuntimeStats::AsyncService*, std::shared_ptr<ServerCompletionQueue>);
    void Proceed();

   private:
    RuntimeStats::AsyncService* service_;
    std::shared_ptr<ServerCompletionQueue> cq_;
    ServerContext ctx_;

    StatsRequest request_;
    StatsResponse response_;
    ServerAsyncResponseWriter<StatsResponse> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  
  };

  void HandleRpcs();

  
  std::shared_ptr<ServerCompletionQueue> cq_;
  RuntimeStats::AsyncService service_;
  std::unique_ptr<Server> server_;
  std::string server_address;

};

#endif // __STATSSERVER__
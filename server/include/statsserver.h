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
  void Run();

 private:
  class CallData {
   public:
    CallData(RuntimeStats::AsyncService*, ServerCompletionQueue*);
    void Proceed();

   private:
    RuntimeStats::AsyncService* service_;
    ServerCompletionQueue* cq_;
    ServerContext ctx_;

    StatsRequest request_;
    StatsResponse reply_;
    ServerAsyncResponseWriter<StatsResponse> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  
  };

  void HandleRpcs();

  
  std::unique_ptr<ServerCompletionQueue> cq_;
  RuntimeStats::AsyncService service_;
  std::unique_ptr<Server> server_;

};

#endif // __STATSSERVER__
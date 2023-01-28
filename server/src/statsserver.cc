#include "statsserver.h"


using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using messages::RuntimeStats;
using messages::StatsRequest;
using messages::StatsResponse;

StatsServer::~StatsServer()
{
    server_->Shutdown();
    cq_->Shutdown();
}

void StatsServer::Run()
{
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
}

StatsServer::CallData::CallData(RuntimeStats::AsyncService *service, ServerCompletionQueue *cq)
    : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE)
{
    // Invoke the serving logic right away.
    Proceed();
}

void StatsServer::CallData::Proceed()
{
    if (status_ == CREATE)
    {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        //TODO

        service_->RequestGetStats(&ctx_, &request_, &responder_, cq_, cq_, this);
    }
    else if (status_ == PROCESS)
    {
     new CallData(service_, cq_);

        //TODO
        //reply_.set_message(prefix + request_.name());

        status_ = FINISH;
        responder_.Finish(reply_, Status::OK, this);
    }
    else
    {
        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}


// This can be run in multiple threads if needed.
void StatsServer::HandleRpcs()
{
    new StatsServer::CallData(&service_, cq_.get());
    void *tag; 
    bool ok;
    while (true)
    {
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        static_cast<CallData *>(tag)->Proceed();
    }
}

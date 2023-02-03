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
using std::string;

StatsServer::~StatsServer()
{
    server_->Shutdown();
    cq_->Shutdown();
}

void StatsServer::Run(string server_address)
{
    this->server_address = server_address;

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
    std::cout << "CallData " << this << " Proceed: ";
    if (status_ == CREATE)
    {
        std::cout << "CREATE";
    
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;
        service_->RequestGetStats(&ctx_, &request_, &responder_, cq_, cq_, this);
    }
    else if (status_ == PROCESS)
    {
        std::cout << "PROCESS";
    
        new CallData(service_, cq_);

        response_.set_timestamp(static_cast<long>(std::time(nullptr)));
        response_.set_time_online(std::rand());
        response_.set_mem_usage(std::rand());

        status_ = FINISH;
        responder_.Finish(response_, Status::OK, this);
    }
    else
    {
        std::cout << "FINISH"; 
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
        std::cout << " Mainloop iteration" << std::endl;
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        static_cast<CallData *>(tag)->Proceed();
    }
}

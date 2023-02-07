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

    std::thread thread_(&StatsServer::HandleRpcs, this);
    // Proceed to the server's main loop.
    HandleRpcs();
    thread_.join();
}

StatsServer::CallData::CallData(RuntimeStats::AsyncService *service, std::shared_ptr<ServerCompletionQueue> cq)
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
        service_->RequestGetStats(&ctx_, &request_, &responder_, cq_.get(), cq_.get(), this);
    }
    else if (status_ == PROCESS)
    {
        
    std::cout << "CallData " << this << " running; " << "Request received at: " << std::time(nullptr) << std::endl; // << "; Queue size: " << ;
        
        new CallData(service_, cq_);

        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 5000));


        response_.set_timestamp(static_cast<long>(std::time(nullptr)));
        response_.set_time_online(std::rand());
        response_.set_mem_usage(std::rand());

        status_ = FINISH;


        responder_.Finish(response_, Status::OK, this);
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
    new StatsServer::CallData(&service_, cq_);
    void *tag; 
    bool ok;
    while (true)
    {
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        static_cast<CallData *>(tag)->Proceed();
    }
}

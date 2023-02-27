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

void StatsServer::Run(string server_address, int numThreads)
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

    std::vector<std::thread> threads;
    for(int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(&StatsServer::HandleRpcs, this));
    }

    for(auto &thread_ : threads) {
        thread_.join();
    }
}

StatsServer::CallData::CallData(RuntimeStats::AsyncService *service, std::shared_ptr<ServerCompletionQueue> cq)
    : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE)
{
    // Invoke the serving logic right away.
    Proceed();
}

void StatsServer::CallData::Respawn() {
    new CallData(service_, cq_);

    std::cout  << "CallData " << this << "Respwaned; Request issued at: " << request_.timestamp() << "; terminated at " << std::time(nullptr) << "; Delta: " << std::time(nullptr) - request_.timestamp() << std::endl;

    delete this;
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
        new CallData(service_, cq_);
        if(request_.timestamp() + kPollingIntervalSeconds< std::time(nullptr)) {
            status_ = FINISH;
            std::cout << "Request " << request_.timestamp() << " cancelled at: " << std::time(nullptr) << "; Delta: " << std::time(nullptr) - request_.timestamp() << std::endl;
            responder_.Finish(response_, Status::CANCELLED, this);
        }
        else {
            //std::cout << "CallData " << this << " running; " 
            std::cout << "Request " << request_.timestamp() << " received at: " << std::time(nullptr) << "; Delta: " << std::time(nullptr) - request_.timestamp() << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 5000));


            response_.set_timestamp(static_cast<long>(std::time(nullptr)));
            response_.set_time_online(std::rand());
            response_.set_mem_usage(std::rand());

            status_ = FINISH;


            responder_.Finish(response_, Status::OK, this);
        }
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
        if(ok) { 
            static_cast<CallData *>(tag)->Proceed();
        }
        else {
            static_cast<CallData *>(tag)->Respawn();
        }
    }
}

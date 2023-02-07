#include "statsclient.h"
#include <grpcpp/grpcpp.h>
#include <thread>

using grpc::Channel;

int main() {
    StatsClient stats(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));
    

    std::thread thread_ = std::thread(&StatsClient::AsyncCompleteRpc, &stats);

    stats.GetStats();

    sleep(500);
    std::cout << "Done!";
    return 0;
}
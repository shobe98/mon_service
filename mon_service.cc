#include "statsclient.h"
#include <grpcpp/grpcpp.h>
#include <thread>

using grpc::Channel;

int main() {
    std::srand(2);

    StatsClient stats(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));
    

    std::thread thread_ = std::thread(&StatsClient::AsyncCompleteRpc, &stats);

    for(int i = 0; i < 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2000));

        stats.GetStats();
    }


    sleep(10);
    thread_.join();

    std::cout << "Done!";
    return 0;
}
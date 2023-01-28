#include "statsclient.h"
#include <grpcpp/grpcpp.h>

using grpc::Channel;

int main() {
    StatsClient stats(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));
    stats.GetStats();
}
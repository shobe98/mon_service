# include<statsserver.h>

int main(int argc, char *argv[]) {
    std::srand(1);

    StatsServer server;
    if(argc >= 2) {
        server.Run(argv[1]);
    }
    else {
        server.Run();
    }
    return 0;
}
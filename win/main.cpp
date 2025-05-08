#include "src/Server.cpp"
#include <iostream>
#include <cstdlib>

int main(int argc,char* argv[]){
    if(argc != 2){
        std::cout << "Usage : " << argv[0] << " port" << std::endl;
        std::exit(1);
    }
    Server server;
    server.StartServer(argv[1]);

    return 0;
}
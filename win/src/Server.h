#pragma once
extern "C" {
#include <winsock2.h>
}

class Server {
private:
    int BUF_SIZE = 2048;
    int BUF_SMALL = 100;
    char* port;
    static unsigned WINAPI RequestHandlerWrapper(void* arg);
public:
    Server();
    int StartServer(char* port);
    unsigned WINAPI RequestHandler(void* arg);
    const char* ContentType(char* file);
    void SendData(SOCKET sock,char* ct,char* fileName);
    void SendErrorMSG(SOCKET sock);
    void ErrorHandling(const char* message);
};

struct ThreadParams {
    Server* server;
    SOCKET socket;
};
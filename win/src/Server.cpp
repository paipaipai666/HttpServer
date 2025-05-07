#include "Server.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
extern "C" {
#include <winsock2.h>
#include <process.h>
}

Server::Server() {};

int Server::StartServer(char* port){
    WSADATA wsaData;
    SOCKET hServSock,hCLntSock;
    SOCKADDR_IN servAdr,clntAdr;

    HANDLE hThread;
    DWORD dwThreadID;
    int clntAdrSize;

    //检测端口号是否正确！

    if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
        ErrorHandling("WSAStartup() Error");
    
    hServSock = socket(PF_INET,SOCK_STREAM,0);
    memset(&servAdr,0,sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdr.sin_port = htons(atoi(port));

    if(bind(hServSock,(SOCKADDR*)&servAdr,sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() Error");
    if(listen(hServSock,5) == SOCKET_ERROR)
        ErrorHandling("listen() Error");

    while(1){
        clntAdrSize = sizeof(clntAdr);
        hCLntSock = accept(hServSock,(SOCKADDR*)&clntAdr,&clntAdrSize);
        std::cout << "Connection Request : " 
            << inet_ntoa(clntAdr.sin_addr) << " : "
            << ntohs(clntAdr.sin_port) << std::endl;
        ThreadParams* params = new ThreadParams{this, hCLntSock};
        hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandlerWrapper, params, 0, (unsigned*)&dwThreadID);
    }
    closesocket(hServSock);
    WSACleanup();
    
    return 0;
}

unsigned WINAPI Server::RequestHandler(void* arg){
    SOCKET hClntSock = (SOCKET)arg;
    std::vector<char> buf(BUF_SIZE);
    std::vector<char> method(BUF_SMALL);
    std::vector<char> ct(BUF_SMALL);
    std::vector<char> fileName(BUF_SMALL);

    recv(hClntSock,buf.data(),BUF_SIZE,0);
    if(strstr(buf.data(),"HTTP/") == NULL){
        SendErrorMSG(hClntSock);
        closesocket(hClntSock);
        return 1;
    }

    strcpy(method.data(),strtok(buf.data()," /"));
    if(strcmp(method.data(),"GET"))
        SendErrorMSG(hClntSock);

    strcpy(fileName.data(),strtok(NULL," /"));
    strcpy(ct.data(),ContentType(fileName.data()));
    SendData(hClntSock,ct.data(),fileName.data());

    return 0;
}

unsigned WINAPI Server::RequestHandlerWrapper(void* arg) {
    ThreadParams* params = static_cast<ThreadParams*>(arg);
    return params->server->RequestHandler((void*)params->socket);
}

void Server::SendData(SOCKET sock,char* ct,char* fileName){
    FILE* sendFile = fopen(fileName,"rb");
    if(sendFile == NULL){
        SendErrorMSG(sock);
        return ;
    }

    fseek(sendFile,0,SEEK_END);
    long fileSize = ftell(sendFile);
    fseek(sendFile,0,SEEK_SET);

    std::string response = "HTTP/1.0 200 OK\r\n";
    response += "Server:simple web server\r\n";
    response += "Content-length: " + std::to_string(fileSize) + "\r\n";
    response += "Content-type: " + std::string(ct) + "\r\n\r\n";

    send(sock,response.c_str(),response.size(),0);
    std::vector<char> buf(BUF_SIZE);
    size_t bytesRead;
    while((bytesRead = fread(buf.data(),1,buf.size(),sendFile)) > 0)
        send(sock,buf.data(),bytesRead,0);

    closesocket(sock);
}

void Server::SendErrorMSG(SOCKET sock) {
    std::string content = 
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<title>Error</title>"
        "<style>body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }</style>"
        "</head>"
        "<body>"
        "<h1>400 Bad Request</h1>"
        "<p>发生错误! 请检查请求文件名和请求方式!</p>"
        "</body>"
        "</html>";

    std::string response = 
        "HTTP/1.1 400 Bad Request\r\n"
        "Server: simple web server\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " + std::to_string(content.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + content;

    send(sock, response.c_str(), response.size(), 0);
    closesocket(sock);
}

const char* Server::ContentType(char* file){
    const char* extension = strrchr(file,'.');
    if(!strcmp(extension,".html") or !strcmp(extension,".htm"))
        return "text/html";
    else 
        return "text/plain";
}

void Server::ErrorHandling(const char* message){
    std::cerr << message << '\n';
    std::exit(1);
}
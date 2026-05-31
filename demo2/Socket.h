#pragma once
#include <arpa/inet.h>
#include "Logger.h" 

class Socket {
private:
    const int fd_;

public:
    explicit Socket(int fd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    int fd() const { return fd_; }
    void setReuseAddr(bool on);
    void bind(const char* ip, uint16_t port);
    void listen(int backlog = 128);
    int accept(struct sockaddr_in* client_addr);
    
    //设置文件描述符为非阻塞模式
    void setNonBlocking(); 
};
#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <fcntl.h> 

Socket::Socket(int fd) : fd_(fd) {
    if (fd_ == -1) {
        Logger::Error("Invalid Socket FD created.");
        throw std::runtime_error("Invalid Socket FD");
    }
}

Socket::~Socket() {
    if (fd_ != -1) {
        close(fd_);
        // Logger::Info("Socket FD " + std::to_string(fd_) + " closed."); 
    }
}

//将 Socket 设置为非阻塞模式
void Socket::setNonBlocking() {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) {
        Logger::Error("fcntl(F_GETFL) failed for FD " + std::to_string(fd_));
        throw std::runtime_error("fcntl failed");
    }
    
    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::Error("fcntl(F_SETFL, O_NONBLOCK) failed for FD " + std::to_string(fd_));
        throw std::runtime_error("setNonBlocking failed");
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        Logger::Error("setReuseAddr failed for FD " + std::to_string(fd_));
        throw std::runtime_error("setReuseAddr failed");
    }
}

void Socket::bind(const char* ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        Logger::Error("bind failed on " + std::string(ip) + ":" + std::to_string(port));
        throw std::runtime_error("bind failed");
    }
    Logger::Info("Socket FD " + std::to_string(fd_) + " bound to " + std::string(ip) + ":" + std::to_string(port));
}

void Socket::listen(int backlog) {
    if (::listen(fd_, backlog) == -1) {
        Logger::Error("listen failed for FD " + std::to_string(fd_));
        throw std::runtime_error("listen failed");
    }
    Logger::Info("Socket FD " + std::to_string(fd_) + " is now listening.");
}

int Socket::accept(struct sockaddr_in* client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)client_addr, &client_len);
    
    if (client_fd == -1) {
        // accept 返回 -1 不一定是致命错误，在非阻塞模式下有可能是 EAGAIN
        // 但在这个 Socket 类里如果报错了，我们先记录下来
        Logger::Error("accept failed on FD " + std::to_string(fd_));
        throw std::runtime_error("accept failed");
    }
    return client_fd;
}
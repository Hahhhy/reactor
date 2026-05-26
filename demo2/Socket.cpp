#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept> // 引入异常处理

Socket::Socket(int fd) : fd_(fd) {
    if (fd_ == -1) {
        throw std::runtime_error("Socket initialization failed: invalid FD");
    }
}

Socket::~Socket() {
    if (fd_ != -1) {
        close(fd_); // RAII：对象销毁时，文件描述符自动关闭，绝不泄漏！
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval))) == -1) {
        throw std::runtime_error("Socket setReuseAddr failed");
    }
}

void Socket::bind(const char* ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // inet_addr 将字符串 IP 转为网络字节序整数，"0.0.0.0" 等同于 INADDR_ANY
    addr.sin_addr.s_addr = inet_addr(ip); 
    addr.sin_port = htons(port);

    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Socket bind failed");
    }
}

void Socket::listen(int backlog) {
    if (::listen(fd_, backlog) == -1) {
        throw std::runtime_error("Socket listen failed");
    }
}

int Socket::accept(struct sockaddr_in* client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)client_addr, &client_len);
    if (client_fd == -1) {
        // 这里暂时用异常，后续进阶可以换成任务书提到的 std::optional 
        throw std::runtime_error("Socket accept failed"); 
    }
    return client_fd;
}
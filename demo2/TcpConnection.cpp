#include "TcpConnection.h"
#include <unistd.h>
#include <iostream>

TcpConnection::TcpConnection(int client_fd) : sock_(std::make_unique<Socket>(client_fd)) {}

TcpConnection::~TcpConnection() {
    std::cout << "[TcpConnection] FD " << sock_->fd() << " resource destroyed." << std::endl;
}

void TcpConnection::handleRead() {
    char buffer[1024];
    ssize_t bytes_read = read(sock_->fd(), buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "Received from FD " << sock_->fd() << ": " << buffer;
        // Echo back
        write(sock_->fd(), buffer, bytes_read); 
    } 
    else if (bytes_read == 0) {
        std::cout << "Client FD " << sock_->fd() << " disconnected." << std::endl;
        // 🌟 核心：客人走了，触发回调，通知 Server 销毁自己！
        if (close_callback_) {
            close_callback_(sock_->fd()); 
        }
    } 
    else {
        // read error，也可以当作断开处理
        if (close_callback_) close_callback_(sock_->fd());
    }
}
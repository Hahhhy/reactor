#include "Server.h"
#include <iostream>

Server::Server(int port) {
    ep_ = std::make_unique<Epoll>();
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_sock_ = std::make_unique<Socket>(listen_fd);

    serv_sock_->setReuseAddr(true);
    serv_sock_->bind("0.0.0.0", port);
    serv_sock_->listen();
}

Server::~Server() {}

void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    int client_fd = serv_sock_->accept(&client_addr);
    
    auto conn = std::make_unique<TcpConnection>(client_fd);
    ep_->updateChannel(client_fd, EPOLLIN, EPOLL_CTL_ADD);
    
    // 🌟 Server 给 TcpConnection 塞纸条 (使用 Lambda 表达式)
    conn->setCloseCallback([this](int fd) {
        // 当 TcpConnection 说自己要关闭时，Server 会在这里把它从 map 中删掉
        // 注意：erase 会触发 TcpConnection 的析构函数，从而触发 Socket 析构，最终 close(fd)
        connections_.erase(fd); 
    });

    connections_[client_fd] = std::move(conn);
    std::cout << "[Server] New client FD: " << client_fd << " added to map." << std::endl;
}

void Server::start() {
    ep_->updateChannel(serv_sock_->fd(), EPOLLIN, EPOLL_CTL_ADD);
    std::cout << "Server started. Event loop running..." << std::endl;

    while (true) {
        std::vector<struct epoll_event> events = ep_->poll();

        for (const auto& event : events) {
            int active_fd = event.data.fd;

            if (active_fd == serv_sock_->fd()) {
                handleNewConnection();
            } 
            else {
                if (connections_.find(active_fd) != connections_.end()) {
                    connections_[active_fd]->handleRead();
                }
            }
        }
    }
}
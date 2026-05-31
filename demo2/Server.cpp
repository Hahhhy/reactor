#include "Server.h"

Server::Server(int port) {
    ep_ = std::make_unique<Epoll>();
    thread_pool_ = std::make_unique<ThreadPool>(4); 
    
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_sock_ = std::make_unique<Socket>(listen_fd);

    serv_sock_->setNonBlocking(); 
    serv_sock_->setReuseAddr(true);
    serv_sock_->bind("0.0.0.0", port);
    serv_sock_->listen();
    
    Logger::Info("Server initialized with ThreadPool (4 workers).");
}

Server::~Server() {
    Logger::Info("Server shutting down. Clearing resources...");
}

void Server::handleNewConnection() {
    struct sockaddr_in client_addr;
    int client_fd = serv_sock_->accept(&client_addr);
    
    auto conn = std::make_unique<TcpConnection>(client_fd, thread_pool_.get(), ep_.get());
    
    ep_->updateChannel(client_fd, EPOLLIN, EPOLL_CTL_ADD);

    conn->setCloseCallback([this](int fd) {
        connections_.erase(fd); 
        Logger::Info("Client FD " + std::to_string(fd) + " removed from connections map."); 
    });

    connections_[client_fd] = std::move(conn);
    Logger::Info("New client connected! FD: " + std::to_string(client_fd));
}

void Server::start() {
    ep_->updateChannel(serv_sock_->fd(), EPOLLIN, EPOLL_CTL_ADD);
    Logger::Info("Server started. Event loop running on Boss thread...");

    while (true) {
        std::vector<struct epoll_event> events = ep_->poll(1000); 
        
        for (const auto& event : events) {
            int active_fd = event.data.fd;

            if (active_fd == serv_sock_->fd()) {
                handleNewConnection();
            } 
            else {
                auto it = connections_.find(active_fd);
                
                if (it != connections_.end()) {
                    if (event.events & EPOLLIN) {
                        it->second->handleRead(); 
                    }
                }
                
                it = connections_.find(active_fd);
                
                if (it != connections_.end()) {
                    if (event.events & EPOLLOUT) {
                        it->second->handleWrite();
                    }
                }
            }
        }
        
        for (auto it = connections_.begin(); it != connections_.end(); ) {
            if (it->second->isExpired(60)) {
                Logger::Info("Client FD " + std::to_string(it->first) + " timed out (60s). Kicking out...");
                it = connections_.erase(it); 
            } else {
                ++it;
            }
        }
    }
}
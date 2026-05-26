#pragma once
#include "Epoll.h"
#include "Socket.h"
#include "TcpConnection.h"
#include <memory>
#include <unordered_map>

class Server {
private:
    std::unique_ptr<Epoll> ep_;
    std::unique_ptr<Socket> serv_sock_;
    
    // 🌟 这就是编译器刚才找不到的 connections_
    std::unordered_map<int, std::unique_ptr<TcpConnection>> connections_;

    // 🌟 这就是编译器刚才找不到的 handleNewConnection
    void handleNewConnection();

public:
    Server(int port);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void start();
};
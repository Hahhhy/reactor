#pragma once
#include "Socket.h"
#include <memory>
#include <string>
#include <functional>

class TcpConnection {
private:
    std::unique_ptr<Socket> sock_;
    std::string read_buf_;
    
    //当连接关闭时调用
    std::function<void(int)> close_callback_; 

public:
    explicit TcpConnection(int client_fd);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    int fd() const { return sock_->fd(); }
    
    // 设置回调函数，当连接关闭时调用
    void setCloseCallback(std::function<void(int)> cb) { close_callback_ = cb; }

    // 处理读事件
    void handleRead();
};
#pragma once
#include "Socket.h"
#include "Logger.h" 
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include<chrono>

class ThreadPool; 
class Epoll; 

class TcpConnection {
private:
    std::unique_ptr<Socket> sock_;
    std::string read_buf_;
    
    std::string write_buf_;     
    std::mutex send_mutex_;     
    
    std::function<void(int)> close_callback_;
    ThreadPool* pool_; 
    Epoll* ep_; 

    std::chrono::time_point<std::chrono::steady_clock> last_active_time_;

public:
    explicit TcpConnection(int client_fd, ThreadPool* pool, Epoll* ep);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    int fd() const { return sock_->fd(); }
    void setCloseCallback(std::function<void(int)> cb) { close_callback_ = cb; }
    
    void handleRead();
    
    void send(const std::string& msg);
    void handleWrite(); 

    void updateActiveTime(){
        last_active_time_ = std::chrono::steady_clock::now();
    }

    bool isExpired(int timeout_seconds) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_active_time_).count();
        return duration > timeout_seconds;
    }
};
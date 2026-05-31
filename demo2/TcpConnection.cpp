#include "TcpConnection.h"
#include "ThreadPool.h"
#include "Epoll.h" // 必须包含，因为要调用 updateChannel
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>

TcpConnection::TcpConnection(int client_fd, ThreadPool* pool, Epoll* ep) 
    : pool_(pool), ep_(ep) { 
    sock_ = std::make_unique<Socket>(client_fd);
    sock_->setNonBlocking(); 

    updateActiveTime();
}

TcpConnection::~TcpConnection() {
    Logger::Info("TcpConnection FD " + std::to_string(sock_->fd()) + " destroyed.");
}

void TcpConnection::handleRead() {
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    int bytes = recv(sock_->fd(), buf, sizeof(buf), 0);

    if (bytes > 0) {
        updateActiveTime();
        std::string message(buf, bytes); 
        pool_->enqueue([this, message]() {
            Logger::Info("[Worker] Processing msg from FD " + std::to_string(this->sock_->fd()));

            std::string reply = "Server Processed: " + message;
            this->send(reply); 
        });
        
    } else if (bytes == 0) {
        Logger::Info("Client FD " + std::to_string(sock_->fd()) + " gracefully disconnected.");
        if (close_callback_) close_callback_(sock_->fd());
    } else if (bytes == -1 && errno != EAGAIN) {
        Logger::Error("Read error on FD " + std::to_string(sock_->fd()));
        if (close_callback_) close_callback_(sock_->fd());
    }
}

void TcpConnection::send(const std::string& msg) {
    std::lock_guard<std::mutex> lock(send_mutex_);

    int sent = 0;
    int remaining = msg.size();
    bool fault_error = false;

    if (write_buf_.empty()) {
        sent = ::send(sock_->fd(), msg.c_str(), msg.size(), 0);
        if (sent >= 0) {
            remaining -= sent;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                Logger::Error("Send directly failed on FD " + std::to_string(sock_->fd()));
                fault_error = true;
            }
            sent = 0; 
        }
    }

    if (!fault_error && remaining > 0) {
        Logger::Info("Send buffer full on FD " + std::to_string(sock_->fd()) + ". Appending to write_buf_.");
        write_buf_.append(msg.c_str() + sent, remaining);
        
        ep_->updateChannel(sock_->fd(), EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
    }
}

void TcpConnection::handleWrite() {
    std::lock_guard<std::mutex> lock(send_mutex_); 
    
    if (write_buf_.empty()) return;

    int sent = ::send(sock_->fd(), write_buf_.c_str(), write_buf_.size(), 0);
    
    if (sent > 0) {
        write_buf_.erase(0, sent);

        if (write_buf_.empty()) {
            Logger::Info("write_buf_ flushed entirely on FD " + std::to_string(sock_->fd()));
            ep_->updateChannel(sock_->fd(), EPOLLIN, EPOLL_CTL_MOD);
        }
    } else if (sent == -1 && errno != EAGAIN) {
        Logger::Error("handleWrite failed on FD " + std::to_string(sock_->fd()));
    }
}
#include "Epoll.h"
#include <unistd.h>
#include <stdexcept>
#include <cstring> 
#include <cerrno>  

Epoll::Epoll() : epoll_fd_(epoll_create1(0)), events_(16) {
    if (epoll_fd_ == -1) {
        Logger::Error("epoll_create1 failed! Error: " + std::string(strerror(errno)));
        throw std::runtime_error("epoll_create1 failed");
    }
    Logger::Info("Epoll Monitor created successfully with FD: " + std::to_string(epoll_fd_));
}

Epoll::~Epoll() {
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
        Logger::Info("Epoll Monitor FD " + std::to_string(epoll_fd_) + " destroyed.");
    }
}

void Epoll::updateChannel(int fd, uint32_t events, int op) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev)); 
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, op, fd, &ev) == -1) {
        Logger::Error("epoll_ctl failed for FD " + std::to_string(fd) + 
                      " with op " + std::to_string(op) + 
                      ". Error: " + std::string(strerror(errno)));
        throw std::runtime_error("epoll_ctl failed");
    }
    //这里不要加 Logger::Info，因为高并发下每秒可能有上万次 update，会把日志撑爆
}

std::vector<struct epoll_event> Epoll::poll(int timeout_ms) {
    int num_events = epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout_ms);
    
    if (num_events == -1) {
        // 处理 EINTR (系统信号打断)
        if (errno == EINTR) {
            Logger::Info("epoll_wait interrupted by system signal, retrying...");
            return {}; 
        }
        Logger::Error("epoll_wait failed! Error: " + std::string(strerror(errno)));
        throw std::runtime_error("epoll_wait failed");
    }

    std::vector<struct epoll_event> active_events;
    for (int i = 0; i < num_events; ++i) {
        active_events.push_back(events_[i]);
    }

    if (num_events == events_.size()) {
        events_.resize(events_.size() * 2);
        Logger::Info("High traffic! Epoll events buffer resized to " + std::to_string(events_.size()));
    }

    return active_events;
}
#include "Epoll.h"
#include <unistd.h>
#include <stdexcept>

// 构造函数：创建 epoll_fd，并且给 vector 预分配 16 个事件的空间
Epoll::Epoll() : epoll_fd_(epoll_create1(0)), events_(16) {
    if (epoll_fd_ == -1) {
        throw std::runtime_error("Epoll create failed");
    }
}

Epoll::~Epoll() {
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

void Epoll::updateChannel(int fd, uint32_t events, int op) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, op, fd, &ev) == -1) {
        throw std::runtime_error("Epoll epoll_ctl failed");
    }
}

std::vector<struct epoll_event> Epoll::poll(int timeout_ms) {
    // events_.data() 获取 vector 底层的原生数组指针
    // events_.size() 获取当前 vector 的容量
    int num_events = epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout_ms);
    
    if (num_events == -1) {
        throw std::runtime_error("Epoll wait failed");
    }

    // 把活跃的事件挑出来，装进一个新的 vector 返回给上层
    std::vector<struct epoll_event> active_events;
    for (int i = 0; i < num_events; ++i) {
        active_events.push_back(events_[i]);
    }
    return active_events;
}
#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Logger.h" 

class Epoll {
private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_; 

public:
    Epoll();
    ~Epoll();

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    void updateChannel(int fd, uint32_t events, int op);
    std::vector<struct epoll_event> poll(int timeout_ms = -1);
};
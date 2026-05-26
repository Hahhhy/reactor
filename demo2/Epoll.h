#pragma once
#include <sys/epoll.h>
#include <vector>

class Epoll {
private:
    int epoll_fd_;                            // epoll 实例的监控室大门钥匙
    std::vector<struct epoll_event> events_; // 用于存放 epoll_wait 返回的活跃事件的数组

public:
    Epoll();
    ~Epoll();

    // 禁用拷贝（同 Socket，epoll_fd 也是不能随意复制的系统资源）
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    // 对应 Demo 2 中的 epoll_ctl 操作
    // op 可以是 EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
    void updateChannel(int fd, uint32_t events, int op); 
    
    // 对应 Demo 2 中的 epoll_wait
    // 返回值是一个装满了“响铃的 fd 及其事件”的数组
    std::vector<struct epoll_event> poll(int timeout_ms = -1);
};
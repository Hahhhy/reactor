#pragma once
#include <arpa/inet.h>

class Socket {
private:
    const int fd_; // 内部包装的系统文件描述符

public:
    // 构造函数：接管一个已经存在的 fd
    explicit Socket(int fd);
    
    // 🌟 析构函数：核心所在！对象销毁时自动 close(fd_)，这就是 RAII
    ~Socket();

    // 🌟 禁用拷贝构造和拷贝赋值！(Modern C++ 核心规范)
    // 为什么？如果允许拷贝，两个 Socket 对象内部会有相同的 fd_。
    // 其中一个销毁时 close 了一次，另一个再销毁又 close 一次，程序直接崩溃！
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // 获取内部的 fd，供 Epoll 等其他类使用
    int fd() const { return fd_; }

    // 封装 Demo1/2 里的底层系统调用
    void bind(const char* ip, uint16_t port);
    void listen(int backlog = 128);
    
    // 接受连接，返回一个【新连接的文件描述符】
    // 为什么不直接返回 Socket 对象？这留给后面的 TcpConnection 去头疼
    int accept(struct sockaddr_in* client_addr); 
    
    // 任务书要求：设置端口复用等选项
    void setReuseAddr(bool on);
};
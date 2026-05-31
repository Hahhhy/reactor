#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h> //  引入 epoll 相关的头文件

const int MAX_EVENTS = 10; // 每次 epoll_wait 最多返回的事件数
const int PORT = 8080;

int main() {
    // 1. 创建监听 socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return -1;
    }

    // 设置端口复用 (避免重启程序时报 Address already in use)
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. 绑定和监听 (和 demo1 完全一样)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return -1;
    }

    if (listen(listen_fd, 128) == -1) {
        perror("listen");
        return -1;
    }
    std::cout << "[Server] Listening on port " << PORT << "..." << std::endl;

    // 3. 创建 epoll 实例 (建监控室)
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return -1;
    }

    // 4. 将 listen_fd 添加到 epoll 监控中 (装第一个摄像头)
    struct epoll_event event;
    event.events = EPOLLIN; //"可读" 事件 (有新客户端连接时，listen_fd 会变成可读)
    event.data.fd = listen_fd; // 记住这个事件是属于哪个 fd 的

    // epoll_ctl：操作监控室，EPOLL_CTL_ADD 代表添加
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        perror("epoll_ctl: listen_fd");
        return -1;
    }

    // 存放 epoll_wait 返回的活跃事件的数组
    struct epoll_event events[MAX_EVENTS]; 

    std::cout << "[Server] Epoll instance created. Entering event loop..." << std::endl;

    // 5. 事件大循环
    while (true) {
        // -1 表示永久阻塞，直到有事件发生
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait");
            break;
        }

        // 走到这里，说明有事件发生了！num_events 告诉你醒了几个摄像头
        for (int i = 0; i < num_events; ++i) {
            int active_fd = events[i].data.fd;

            // 情况 A：如果是 listen_fd 响了，说明有【新客户端】连进来了
            if (active_fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }
                std::cout << "[Epoll] New connection! FD: " << client_fd << std::endl;

                //把新来的 client_fd 也加到监控室里！
                struct epoll_event client_event;
                client_event.events = EPOLLIN; // 关心客户端发来的数据 (可读)
                client_event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
                    perror("epoll_ctl: client_fd");
                    close(client_fd);
                }
            } 
            // 情况 B：如果是普通的 client_fd 响了，说明老客户发【新数据】来了，或者断开了
            else {
                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                
                // 注意：这里 read 不会阻塞很久，因为 epoll 已经告诉我们有数据了
                ssize_t bytes_read = read(active_fd, buffer, sizeof(buffer) - 1);

                if (bytes_read > 0) {
                    std::cout << "[Epoll] Received from FD " << active_fd << ": " << buffer;
                    write(active_fd, buffer, bytes_read); // Echo
                } 
                else if (bytes_read == 0) {
                    // 客户端主动断开连接
                    std::cout << "[Epoll] Client FD " << active_fd << " disconnected." << std::endl;
                    // 关闭 fd 后，Linux 内核会自动将其从 epoll 监控列表中剔除
                    close(active_fd); 
                } 
                else {
                    perror("read error");
                    close(active_fd);
                }
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);
    return 0;
}
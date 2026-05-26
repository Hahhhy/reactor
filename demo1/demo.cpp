
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


int main(){
    //创建socket
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    //这些宏定义来自哪里？什么意思
    if(listen_fd==-1){
        perror("socket");
        return -1;
    }

    //绑定IP和端口——Bind
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));//为什么要清空结构体这一步
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;//监听所有IP地址，这个两次结构体的访问符是为什么
    server_addr.sin_port=htons(8080);//为什么要转换字节序，什么叫做主机字节序和网络字节序
    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        perror("bind");
        return -1;
    }//也就是说绑定socket（返回的是什么那个函数）和包含IP地址和端口的结构体（为什么要强制转换成sockaddr类型）和结构体的大小（为什么要传入这个参数）这三个参数
    //监听——Listen
    //为什么是5，和128有什么区别
    if(listen(listen_fd,5)==-1){
        perror("listen");
        return -1;
    }//监听socket和连接请求队列的长度（为什么要设置这个参数）这两个参数
    std::cout<<"Server is listening on port 8080..."<<std::endl;
    //接受连接——Accept
    struct sockaddr_in client_addr;
    socklen_t client_addr_len=sizeof(client_addr);
    std::cout<<"Waiting for a client to connect..."<<std::endl;
    int client_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&client_addr_len);
    if(client_fd==-1){
        perror("accept");
        close(listen_fd);
        return -1;
    }//接受监听socket和包含客户端地址信息的结构体和结构体大小的指针这三个参数
    std::cout<<"Client connected!"<<std::endl;
    //发送数据——Send
    char buffer[1024];
    while(true){
        memset(buffer,0,sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) 
        {
            std::cout << "Received from client: " << buffer << std::endl;
            write(client_fd, buffer, bytes_read); // Echo back to client
        }
        else if (bytes_read == 0) 
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }
        else 
        {
            perror("read");
            break;
        }
    }
    //关闭连接——Close
    close(client_fd);
    close(listen_fd);
    std::cout<<"Server shut down."<<std::endl;
    return 0;
}
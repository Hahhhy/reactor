# 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(UniqueWebServer CXX)

# 设置 C++ 标准为 C++17 (为了更好地支持智能指针和 auto 等特性)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启常用的编译警告，帮助排查错误
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
# ... 前面的配置保持不变 ...

# 1. 收集所有核心类的源文件 (注意 epoll.cpp 是小写)
set(SERVER_SOURCES
    Socket.cpp
    epoll.cpp
    TcpConnection.cpp
    Server.cpp
    Logger.cpp
)

# 2. 将这些源文件编译成一个名为 "reactor_server" 的静态库
add_library(reactor_server STATIC ${SERVER_SOURCES})

# 3. 编译主程序 (注意这里改成了 main.cpp)
add_executable(web_server main.cpp)

# 4. 将静态库链接到主程序上
target_link_libraries(web_server PRIVATE reactor_server)
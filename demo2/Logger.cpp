#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger() {
    // 打开日志文件（追加模式）
    access_file_.open("access.log", std::ios::app);
    error_file_.open("error.log", std::ios::app);
    if (!access_file_.is_open() || !error_file_.is_open()) {
        std::cerr << "Failed to open log files!" << std::endl;
    }
}

Logger::~Logger() {
    if (access_file_.is_open()) access_file_.close();
    if (error_file_.is_open()) error_file_.close();
}

Logger& Logger::getInstance() {
    static Logger instance; // C++11 保证局部静态变量初始化的线程安全
    return instance;
}

// 获取当前时间的格式化字符串
std::string getCurrentTimeStr() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_); // 加锁
    std::string time_str = "[" + getCurrentTimeStr() + "] ";

    if (level == LogLevel::INFO) {
        if (access_file_.is_open()) {
            access_file_ << time_str << "[INFO] " << message << std::endl;
        }
        // 同时输出到终端方便调试
        std::cout << time_str << "\033[32m[INFO]\033[0m " << message << std::endl; 
    } else {
        if (error_file_.is_open()) {
            error_file_ << time_str << "[ERROR] " << message << std::endl;
        }
        std::cerr << time_str << "\033[31m[ERROR]\033[0m " << message << std::endl;
    }
}

void Logger::Info(const std::string& message) {
    getInstance().log(LogLevel::INFO, message);
}

void Logger::Error(const std::string& message) {
    getInstance().log(LogLevel::ERROR, message);
}
#pragma once
#include <string>
#include <fstream>
#include <mutex>

// 日志级别枚举
enum class LogLevel {
    INFO,   // 对应 access log
    ERROR   // 对应 error log
};

class Logger {
private:
    std::ofstream access_file_;
    std::ofstream error_file_;
    std::mutex mutex_; // 保证多线程写入时的线程安全（为进阶任务做准备）

    // 私有构造，防止外部实例化
    Logger();
    ~Logger();

public:
    // 禁用拷贝
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 获取全局唯一实例
    static Logger& getInstance();

    // 核心写日志方法
    void log(LogLevel level, const std::string& message);

    // 快捷调用静态方法
    static void Info(const std::string& message);
    static void Error(const std::string& message);
};
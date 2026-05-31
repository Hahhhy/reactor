#pragma once
#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    INFO,   // 对应 access log
    ERROR   // 对应 error log
};

class Logger {
private:
    std::ofstream access_file_;
    std::ofstream error_file_;
    std::mutex mutex_; 

    Logger();
    ~Logger();

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance();
    void log(LogLevel level, const std::string& message);

    static void Info(const std::string& message);
    static void Error(const std::string& message);
};
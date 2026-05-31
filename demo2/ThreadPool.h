#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
private:
    std::vector<std::thread> workers_;      
    std::queue<std::function<void()>> tasks_; 

    std::mutex queue_mutex_;                
    std::condition_variable condition_;      
    bool stop_;                 

public:
    explicit ThreadPool(size_t threads) : stop_(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        
                        this->condition_.wait(lock, [this] {
                            return this->stop_ || !this->tasks_.empty();
                        });

                        if (this->stop_ && this->tasks_.empty()) return;

                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    task(); 
                }
            });
        }
    }
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

    template<class F>
    void enqueue(F&& task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks_.emplace(std::forward<F>(task));
        }
        condition_.notify_one(); 
    }
};
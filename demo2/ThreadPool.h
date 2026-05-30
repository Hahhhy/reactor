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
            stop_ = true; // 挂上打烊牌子
        }
        condition_.notify_all(); // 疯狂摇铃铛，把所有睡着的打工人全部叫醒！
        
        // 看着他们把手头的活干完再走（等待所有线程结束）
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

    // 老板专用的接口：往队列里扔点菜单
    // template 和万能引用让老板可以扔任何形式的任务进来
    template<class F>
    void enqueue(F&& task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) throw std::runtime_error("enqueue on stopped ThreadPool");
            
            // 把任务装进队列
            tasks_.emplace(std::forward<F>(task));
        }
        // 摇一下铃铛，叫醒宿舍里正在睡觉的一个打工人：“接客啦！”
        condition_.notify_one(); 
    }
};
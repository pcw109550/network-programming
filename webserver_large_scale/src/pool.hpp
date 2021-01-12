#ifndef POOL_H
#define POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class Pool {
    public:
        Pool(int);
        template<class F, class... Args>
        void enqueue(F&& f, Args&&... args);
        ~Pool();
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()> > tasks;
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
};

inline Pool::Pool(int threads) {
    stop = false;
    for (int i = 0; i < threads; i++)
        workers.emplace_back(
            [this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            }
        );
}

template<class F, class... Args>
void Pool::enqueue(F&& f, Args&&... args) {
    auto task = std::make_shared< std::packaged_task<typename std::result_of<F(Args...)>::type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop)
            throw std::runtime_error("Pool");
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
}

inline Pool::~Pool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(auto &worker: workers)
        worker.join();
}

#endif
#ifndef __QUEUE_HPP_
#define __QUEUE_HPP_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue {
    public:  
        T pop(void);
        void push(T& item);
        int size(void);
    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
};

#endif
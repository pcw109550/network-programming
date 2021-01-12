#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "server.hpp"
#include "client.hpp"
#include "queue.hpp"
#include <event2/event.h>

template <typename T>
T Queue<T>::pop(void)  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty()) {
        cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
}

template <typename T>
void Queue<T>::push(T& item)  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
}

template <typename T>
int Queue<T>::size(void)  {
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = queue_.size();
    mlock.unlock();
    return size;
}

template class Queue<Server **>;
template class Queue<Client **>;
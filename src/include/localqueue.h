#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<class T>
class LocalQueue {
public:
    void Push(const T& data) {
        std::lock_guard<std::mutex> lock(mtx_);
        que_.push(data);
        condition_.notify_one();
    }
    T Pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        while(que_.empty()) {
            condition_.wait(lock);
        }
        T t = move(que_.front());
        que_.pop();
        return t;
    }

private:

private:
    std::queue<T> que_;
    std::mutex mtx_;
    std::condition_variable condition_;
};
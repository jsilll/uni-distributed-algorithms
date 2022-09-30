#pragma once

#include <atomic>
#include <iostream>
#include <vector>
#include <mutex>
#include <string>

template <typename T>
class ThreadsafeList
{
private:
    mutable std::mutex mutex_;
    std::vector<T> vector_;

public:
    void PushBack(const T &item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        vector_.push_back(item);
    }

    std::atomic<bool> Contains(const T &item) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return (std::find(vector_.begin(), vector_.end(), item) != vector_.end());
    };

    // ---------- Getters ---------- //

    unsigned long size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return vector_.size();
    }

    std::vector<T> vector() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return vector_;
    };
};
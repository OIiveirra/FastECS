#pragma once

#include <vector>
#include <thread>
#include <deque>
#include <functional>
#include <atomic>
#include <memory>

namespace FastECS {

class SpinLock {
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (locked.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield(); 
        }
    }
    void unlock() {
        locked.clear(std::memory_order_release);
    }
    bool try_lock() {
        return !locked.test_and_set(std::memory_order_acquire);
    }
};

class WorkStealingQueue {
    std::deque<std::function<void()>> tasks;
    SpinLock lock;
public:
    void push(std::function<void()> task) {
        lock.lock();
        tasks.push_back(std::move(task));
        lock.unlock();
    }
    
    bool pop(std::function<void()>& task) {
        lock.lock();
        if (tasks.empty()) {
            lock.unlock();
            return false;
        }
        task = std::move(tasks.back());
        tasks.pop_back();
        lock.unlock();
        return true;
    }
    
    bool steal(std::function<void()>& task) {
        if (!lock.try_lock()) return false;
        if (tasks.empty()) {
            lock.unlock();
            return false;
        }
        task = std::move(tasks.front());
        tasks.pop_front();
        lock.unlock();
        return true;
    }
};

class JobSystem {
public:
    JobSystem() = default;

    void init(uint32_t numThreads = std::thread::hardware_concurrency()) {
        m_numThreads = numThreads;
        m_running = true;
        m_pendingTasks = 0;

        for (uint32_t i = 0; i < numThreads; ++i) {
            m_queues.push_back(std::make_unique<WorkStealingQueue>());
        }

        for (uint32_t i = 0; i < numThreads; ++i) {
            m_threads.emplace_back([this, i]() {
                s_threadIndex = i;
                
                std::function<void()> task;
                while (m_running) {
                    if (m_queues[i]->pop(task)) {
                        task();
                        m_pendingTasks--;
                    } else {
                        bool stolen = false;
                        for (uint32_t j = 1; j < m_numThreads; ++j) {
                            uint32_t targetIndex = (i + j) % m_numThreads;
                            if (m_queues[targetIndex]->steal(task)) {
                                task();
                                m_pendingTasks--;
                                stolen = true;
                                break;
                            }
                        }
                        if (!stolen) {
                            std::this_thread::yield();
                        }
                    }
                }
            });
        }
    }

    ~JobSystem() {
        m_running = false;
        for (auto& t : m_threads) {
            if (t.joinable()) t.join();
        }
    }

    void execute(std::function<void()> task) {
        m_pendingTasks++;
        uint32_t index = s_threadIndex < m_numThreads ? s_threadIndex : 0;
        m_queues[index]->push(std::move(task));
    }

    void wait() {
        std::function<void()> task;
        while (m_pendingTasks > 0) {
            bool worked = false;
            for (uint32_t i = 0; i < m_numThreads; ++i) {
                if (m_queues[i]->steal(task)) {
                    task();
                    m_pendingTasks--;
                    worked = true;
                    break;
                }
            }
            if (!worked) {
                std::this_thread::yield();
            }
        }
    }

private:
    std::vector<std::thread> m_threads;
    std::vector<std::unique_ptr<WorkStealingQueue>> m_queues;
    std::atomic<uint32_t> m_pendingTasks{0};
    std::atomic<bool> m_running{false};
    uint32_t m_numThreads = 0;

    inline static thread_local uint32_t s_threadIndex = 0xFFFFFFFF;
};

}

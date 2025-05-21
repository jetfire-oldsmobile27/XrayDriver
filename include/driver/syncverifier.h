#pragma once
#include <thread>
#include <mutex>
#include <unordered_set>

class SyncVerifier {
public:
    void check_deadlock() {
        std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
        
        if (lock.try_lock()) {
            if (m_locked_threads.count(std::this_thread::get_id())) {
                throw std::runtime_error("Potential deadlock detected");
            }
            m_locked_threads.insert(std::this_thread::get_id());
            lock.unlock();
        }
    }

    template<typename Func>
    void validate_access_patterns(Func&& f) {
        std::scoped_lock lock(m_mutex);
        m_access_counter++;
        f();
    }

private:
    std::mutex m_mutex;
    std::unordered_set<std::thread::id> m_locked_threads;
    size_t m_access_counter = 0;
};
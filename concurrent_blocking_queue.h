#ifndef LIBCAMERA_MEME_CONCURRENT_BLOCKING_QUEUE_H
#define LIBCAMERA_MEME_CONCURRENT_BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ConcurrentBlockingQueue {
public:
    ConcurrentBlockingQueue() = default;

    [[nodiscard]] bool empty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    typename std::queue<T>::size_type size() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [&]{ return !m_queue.empty(); });

        auto item = std::move(m_queue.front());
        m_queue.pop();
        return item;
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(item);
        lock.unlock();
        m_cond.notify_one();
    }

    void push(T&& item) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::forward<T>(item));
        lock.unlock();
        m_cond.notify_one();
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<Args>(args)...);
        lock.unlock();
        m_cond.notify_one();
    }
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

#endif //LIBCAMERA_MEME_CONCURRENT_BLOCKING_QUEUE_H

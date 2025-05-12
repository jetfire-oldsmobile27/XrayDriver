#pragma once
#include <functional>
#include <vector>
#include <mutex>

template<typename EventType>
class EventNotifier {
public:
    using Handler = std::function<void(const EventType&)>;

    static EventNotifier& instance() {
        static EventNotifier inst;
        return inst;
    }

    void subscribe(Handler handler) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.push_back(handler);
    }

    void notify(const EventType& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& handler : m_handlers) {
            handler(event);
        }
    }

private:
    EventNotifier() = default;
    std::vector<Handler> m_handlers;
    std::mutex m_mutex;
};

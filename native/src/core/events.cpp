#include "events.hpp"

namespace qth {

EventQueue& EventQueue::instance() {
    static EventQueue* q = new EventQueue();   // see HandleTable::instance()
    return *q;
}

void EventQueue::push(int32_t kind, int64_t handle, int32_t value, QString text,
                      int32_t x, int32_t y, int32_t mods) {
    if (queue_.size() >= max_size()) queue_.pop_front();
    queue_.push_back(Event{kind, handle, value, std::move(text), x, y, mods});
}

int32_t EventQueue::poll() {
    if (queue_.empty()) {
        current_ = Event{};
        return 0;
    }
    current_ = std::move(queue_.front());
    queue_.pop_front();
    return current_.kind;
}

void EventQueue::clear() {
    queue_.clear();
    current_ = Event{};
}

} // namespace qth

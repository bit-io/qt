#pragma once

#include <QString>
#include <cstdint>
#include <deque>

namespace qth {

struct Event {
    int32_t kind   = 0;   // QTH_EVENT_*
    int64_t handle = 0;   // widget / action / timer that produced it
    int32_t value  = 0;   // slider/spin value, checked state, row, key code, mouse button …
    QString text;         // line-edit text, combo text, list item text, typed key …
    int32_t x = 0;        // mouse x, table row, new width …
    int32_t y = 0;        // mouse y, table column, new height …
    int32_t mods = 0;     // keyboard modifiers: 1 shift, 2 ctrl, 4 alt, 8 meta
};

class EventQueue {
public:
    static EventQueue& instance();

    // Oldest events are dropped once `max_size()` is exceeded, so an H#
    // program that never polls cannot grow the queue without bound.
    void push(int32_t kind, int64_t handle, int32_t value = 0, QString text = {},
              int32_t x = 0, int32_t y = 0, int32_t mods = 0);

    // Pops the next event into `current()`; returns its kind or 0 if empty.
    int32_t poll();

    const Event& current() const { return current_; }
    size_t pending() const { return queue_.size(); }
    void clear();

    static constexpr size_t max_size() { return 4096; }

private:
    EventQueue() = default;
    std::deque<Event> queue_;
    Event current_;
};

inline EventQueue& events() { return EventQueue::instance(); }

} // namespace qth

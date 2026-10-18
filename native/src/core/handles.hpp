#pragma once

#include <QObject>
#include <QPointer>
#include <cstdint>
#include <unordered_map>

namespace qth {

class HandleTable {
public:
    static HandleTable& instance();

    // Registers `obj` (idempotent) and returns its handle; 0 for nullptr.
    int64_t add(QObject* obj);

    // nullptr when the handle is 0, unknown, or the object is gone.
    QObject* raw(int64_t handle) const;

    template <class T>
    T* as(int64_t handle) const {
        return qobject_cast<T*>(raw(handle));
    }

    // 0 when the object was never registered.
    int64_t id_of(const QObject* obj) const;

    bool valid(int64_t handle) const { return raw(handle) != nullptr; }

    size_t size() const { return by_id_.size(); }

private:
    HandleTable() = default;
    void forget(const QObject* obj);

    std::unordered_map<int64_t, QPointer<QObject>> by_id_;
    std::unordered_map<const QObject*, int64_t>    by_ptr_;
    int64_t next_ = 1;
};

inline HandleTable& handles() { return HandleTable::instance(); }

} // namespace qth

#include "handles.hpp"

namespace qth {

HandleTable& HandleTable::instance() {
    // Intentionally never destroyed: widgets are still torn down while the
    // QApplication goes away at process exit, and their `destroyed` hook
    // calls back into this table.
    static HandleTable* table = new HandleTable();
    return *table;
}

int64_t HandleTable::add(QObject* obj) {
    if (!obj) return 0;
    if (auto it = by_ptr_.find(obj); it != by_ptr_.end()) return it->second;

    const int64_t id = next_++;
    by_id_[id]   = obj;
    by_ptr_[obj] = id;
    QObject::connect(obj, &QObject::destroyed, [this](QObject* gone) {
        forget(gone);
    });
    return id;
}

QObject* HandleTable::raw(int64_t handle) const {
    auto it = by_id_.find(handle);
    return it == by_id_.end() ? nullptr : it->second.data();
}

int64_t HandleTable::id_of(const QObject* obj) const {
    auto it = by_ptr_.find(obj);
    return it == by_ptr_.end() ? 0 : it->second;
}

void HandleTable::forget(const QObject* obj) {
    auto it = by_ptr_.find(obj);
    if (it == by_ptr_.end()) return;
    by_id_.erase(it->second);
    by_ptr_.erase(it);
}

} // namespace qth

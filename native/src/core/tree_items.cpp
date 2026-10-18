#include "tree_items.hpp"

#include <vector>

namespace qth {

TreeItems& TreeItems::instance() {
    static TreeItems* t = new TreeItems();   // leaked on purpose, see HandleTable
    return *t;
}

int64_t TreeItems::add(QTreeWidget* tree, QTreeWidgetItem* item) {
    const int64_t id = next_++;
    by_id_[id] = Entry{tree, item};
    by_ptr_[item] = id;
    return id;
}

QTreeWidgetItem* TreeItems::get(QTreeWidget* tree, int64_t id) const {
    auto it = by_id_.find(id);
    if (it == by_id_.end() || it->second.tree.isNull() || it->second.tree.data() != tree) return nullptr;
    return it->second.item;
}

int64_t TreeItems::id_of(const QTreeWidgetItem* item) const {
    auto it = by_ptr_.find(item);
    return it == by_ptr_.end() ? 0 : it->second;
}

void TreeItems::purge_subtree(QTreeWidgetItem* item) {
    if (!item) return;
    for (int i = 0; i < item->childCount(); ++i) purge_subtree(item->child(i));
    auto it = by_ptr_.find(item);
    if (it == by_ptr_.end()) return;
    by_id_.erase(it->second);
    by_ptr_.erase(it);
}

void TreeItems::purge_tree(QTreeWidget* tree) {
    std::vector<int64_t> dead;
    for (auto& [id, e] : by_id_) {
        if (e.tree.isNull() || e.tree.data() == tree) dead.push_back(id);
    }
    for (int64_t id : dead) {
        auto it = by_id_.find(id);
        if (it == by_id_.end()) continue;
        by_ptr_.erase(it->second.item);
        by_id_.erase(it);
    }
}

void TreeItems::watch_tree(QTreeWidget* tree) {
    // QPointer is already null during ~QObject's `destroyed`, so purge by
    // pointer identity of the stale entries instead.
    QObject::connect(tree, &QObject::destroyed, [this](QObject*) {
        std::vector<int64_t> dead;
        for (auto& [id, e] : by_id_) if (e.tree.isNull()) dead.push_back(id);
        for (int64_t id : dead) {
            auto it = by_id_.find(id);
            by_ptr_.erase(it->second.item);
            by_id_.erase(it);
        }
    });
}

} // namespace qth

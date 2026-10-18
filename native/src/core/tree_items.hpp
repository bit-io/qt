#pragma once

#include <QPointer>
#include <QTreeWidget>
#include <cstdint>
#include <unordered_map>

namespace qth {

class TreeItems {
public:
    static TreeItems& instance();

    int64_t add(QTreeWidget* tree, QTreeWidgetItem* item);
    // nullptr when the id is unknown, purged, or belongs to another tree.
    QTreeWidgetItem* get(QTreeWidget* tree, int64_t id) const;
    int64_t id_of(const QTreeWidgetItem* item) const;

    void purge_subtree(QTreeWidgetItem* item);   // before deleting `item`
    void purge_tree(QTreeWidget* tree);          // before clear() / on destroy
    void watch_tree(QTreeWidget* tree);          // purge when the tree dies

private:
    struct Entry {
        QPointer<QTreeWidget> tree;
        QTreeWidgetItem* item;
    };
    std::unordered_map<int64_t, Entry> by_id_;
    std::unordered_map<const QTreeWidgetItem*, int64_t> by_ptr_;
    int64_t next_ = 1;
};

inline TreeItems& tree_items() { return TreeItems::instance(); }

} // namespace qth

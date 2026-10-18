#include "common.hpp"

#include "../core/tree_items.hpp"

#include <QTreeWidget>

using namespace qth;
using namespace qth::api;

namespace {

QTreeWidgetItem* item_of(QTreeWidget* t, int64_t id) {
    return (t && id != 0) ? tree_items().get(t, id) : nullptr;
}

} // namespace

extern "C" {

int64_t qth_tree_new(int32_t columns) {
    auto* t = new QTreeWidget();
    t->setColumnCount(columns > 0 ? columns : 1);
    t->setHeaderHidden(false);
    tree_items().watch_tree(t);
    const int64_t id = reg(t);
    QObject::connect(t, &QTreeWidget::currentItemChanged, [id](QTreeWidgetItem* cur, QTreeWidgetItem*) {
        emit_event(QTH_EVENT_TREE_SELECTION_CHANGED, id,
                   cur ? static_cast<int32_t>(tree_items().id_of(cur)) : 0, cur ? cur->text(0) : QString());
    });
    QObject::connect(t, &QTreeWidget::itemActivated, [id](QTreeWidgetItem* it, int) {
        emit_event(QTH_EVENT_TREE_ITEM_ACTIVATED, id, static_cast<int32_t>(tree_items().id_of(it)), it->text(0));
    });
    QObject::connect(t, &QTreeWidget::itemDoubleClicked, [id](QTreeWidgetItem* it, int) {
        emit_event(QTH_EVENT_TREE_ITEM_ACTIVATED, id, static_cast<int32_t>(tree_items().id_of(it)), it->text(0));
    });
    return id;
}

void qth_tree_set_header(int64_t h, int32_t col, const char* text) {
    auto* t = get<QTreeWidget>(h);
    if (t && col >= 0 && col < t->columnCount()) t->headerItem()->setText(col, qs(text));
}
void qth_tree_set_header_visible(int64_t h, int8_t visible) {
    if (auto* t = get<QTreeWidget>(h)) t->setHeaderHidden(!visible);
}

int64_t qth_tree_add_item(int64_t h, int64_t parent, const char* text) {
    auto* t = get<QTreeWidget>(h);
    if (!t) return 0;
    QTreeWidgetItem* item = nullptr;
    if (parent == 0) {
        item = new QTreeWidgetItem(t);
    } else {
        QTreeWidgetItem* p = item_of(t, parent);
        if (!p) return 0;
        item = new QTreeWidgetItem(p);
    }
    item->setText(0, qs(text));
    return tree_items().add(t, item);
}

void qth_tree_set_item_text(int64_t h, int64_t item, int32_t col, const char* text) {
    auto* t = get<QTreeWidget>(h);
    auto* it = item_of(t, item);
    if (it && col >= 0 && col < t->columnCount()) it->setText(col, qs(text));
}

const char* qth_tree_get_item_text(int64_t h, int64_t item, int32_t col) {
    auto* t = get<QTreeWidget>(h);
    auto* it = item_of(t, item);
    return out(it && col >= 0 && col < t->columnCount() ? it->text(col) : QString());
}

void qth_tree_remove_item(int64_t h, int64_t item) {
    auto* t = get<QTreeWidget>(h);
    auto* it = item_of(t, item);
    if (!it) return;
    tree_items().purge_subtree(it);
    delete it;   // detaches itself from its parent / the tree
}

void qth_tree_clear(int64_t h) {
    if (auto* t = get<QTreeWidget>(h)) {
        tree_items().purge_tree(t);
        t->clear();
    }
}

int64_t qth_tree_current_item(int64_t h) {
    auto* t = get<QTreeWidget>(h);
    return t && t->currentItem() ? tree_items().id_of(t->currentItem()) : 0;
}
void qth_tree_set_current_item(int64_t h, int64_t item) {
    auto* t = get<QTreeWidget>(h);
    if (auto* it = item_of(t, item)) t->setCurrentItem(it);
}
int64_t qth_tree_item_parent(int64_t h, int64_t item) {
    auto* it = item_of(get<QTreeWidget>(h), item);
    return it && it->parent() ? tree_items().id_of(it->parent()) : 0;
}
int32_t qth_tree_child_count(int64_t h, int64_t item) {
    auto* t = get<QTreeWidget>(h);
    if (!t) return 0;
    if (item == 0) return t->topLevelItemCount();
    auto* it = item_of(t, item);
    return it ? it->childCount() : 0;
}
int64_t qth_tree_child_at(int64_t h, int64_t item, int32_t index) {
    auto* t = get<QTreeWidget>(h);
    if (!t || index < 0) return 0;
    QTreeWidgetItem* c = nullptr;
    if (item == 0) {
        if (index < t->topLevelItemCount()) c = t->topLevelItem(index);
    } else if (auto* it = item_of(t, item)) {
        if (index < it->childCount()) c = it->child(index);
    }
    return c ? tree_items().id_of(c) : 0;
}
void qth_tree_set_expanded(int64_t h, int64_t item, int8_t expanded) {
    if (auto* it = item_of(get<QTreeWidget>(h), item)) it->setExpanded(expanded != 0);
}
void qth_tree_expand_all(int64_t h)   { if (auto* t = get<QTreeWidget>(h)) t->expandAll(); }
void qth_tree_collapse_all(int64_t h) { if (auto* t = get<QTreeWidget>(h)) t->collapseAll(); }
void qth_tree_set_sorting(int64_t h, int8_t enabled) {
    if (auto* t = get<QTreeWidget>(h)) t->setSortingEnabled(enabled != 0);
}

} // extern "C"

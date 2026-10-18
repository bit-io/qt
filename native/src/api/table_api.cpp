#include "common.hpp"

#include <QHeaderView>
#include <QSignalBlocker>
#include <QTableWidget>

using namespace qth;
using namespace qth::api;

namespace {

bool in_range(const QTableWidget* t, int row, int col) {
    return t && row >= 0 && col >= 0 && row < t->rowCount() && col < t->columnCount();
}

QTableWidgetItem* ensure_item(QTableWidget* t, int row, int col) {
    QTableWidgetItem* it = t->item(row, col);
    if (!it) {
        it = new QTableWidgetItem();
        t->setItem(row, col, it);
    }
    return it;
}

} // namespace

extern "C" {

int64_t qth_table_new(int32_t rows, int32_t cols) {
    auto* t = new QTableWidget(rows > 0 ? rows : 0, cols > 0 ? cols : 0);
    const int64_t id = reg(t);
    QObject::connect(t, &QTableWidget::currentCellChanged, [id](int r, int c, int, int) {
        emit_event(QTH_EVENT_TABLE_CELL_CHANGED, id, 0, {}, r, c);
    });
    QObject::connect(t, &QTableWidget::itemChanged, [id](QTableWidgetItem* it) {
        emit_event(QTH_EVENT_TABLE_CELL_EDITED, id, 0, it->text(), it->row(), it->column());
    });
    QObject::connect(t, &QTableWidget::cellActivated, [id, t](int r, int c) {
        auto* it = t->item(r, c);
        emit_event(QTH_EVENT_TABLE_CELL_ACTIVATED, id, 0, it ? it->text() : QString(), r, c);
    });
    QObject::connect(t, &QTableWidget::cellDoubleClicked, [id, t](int r, int c) {
        auto* it = t->item(r, c);
        emit_event(QTH_EVENT_TABLE_CELL_ACTIVATED, id, 0, it ? it->text() : QString(), r, c);
    });
    return id;
}

void qth_table_set_size(int64_t h, int32_t rows, int32_t cols) {
    if (auto* t = get<QTableWidget>(h)) {
        t->setRowCount(rows > 0 ? rows : 0);
        t->setColumnCount(cols > 0 ? cols : 0);
    }
}
int32_t qth_table_rows(int64_t h) { auto* t = get<QTableWidget>(h); return t ? t->rowCount() : 0; }
int32_t qth_table_cols(int64_t h) { auto* t = get<QTableWidget>(h); return t ? t->columnCount() : 0; }

// Programmatic writes are silent (no TABLE_CELL_EDITED) and never re-sort mid-fill.
void qth_table_set_cell(int64_t h, int32_t row, int32_t col, const char* text) {
    auto* t = get<QTableWidget>(h);
    if (!in_range(t, row, col)) return;
    QSignalBlocker block(t);
    const bool sorting = t->isSortingEnabled();
    t->setSortingEnabled(false);
    ensure_item(t, row, col)->setText(qs(text));
    t->setSortingEnabled(sorting);
}

const char* qth_table_get_cell(int64_t h, int32_t row, int32_t col) {
    auto* t = get<QTableWidget>(h);
    if (!in_range(t, row, col)) return out(QString());
    auto* it = t->item(row, col);
    return out(it ? it->text() : QString());
}

void qth_table_set_col_header(int64_t h, int32_t col, const char* text) {
    auto* t = get<QTableWidget>(h);
    if (!t || col < 0 || col >= t->columnCount()) return;
    auto* it = t->horizontalHeaderItem(col);
    if (!it) { it = new QTableWidgetItem(); t->setHorizontalHeaderItem(col, it); }
    it->setText(qs(text));
}

void qth_table_set_row_header(int64_t h, int32_t row, const char* text) {
    auto* t = get<QTableWidget>(h);
    if (!t || row < 0 || row >= t->rowCount()) return;
    auto* it = t->verticalHeaderItem(row);
    if (!it) { it = new QTableWidgetItem(); t->setVerticalHeaderItem(row, it); }
    it->setText(qs(text));
}

int32_t qth_table_add_row(int64_t h) {
    auto* t = get<QTableWidget>(h);
    if (!t) return -1;
    const int at = t->rowCount();
    t->insertRow(at);
    return at;
}
void qth_table_insert_row(int64_t h, int32_t at) {
    auto* t = get<QTableWidget>(h);
    if (t && at >= 0 && at <= t->rowCount()) t->insertRow(at);
}
void qth_table_remove_row(int64_t h, int32_t row) {
    auto* t = get<QTableWidget>(h);
    if (t && row >= 0 && row < t->rowCount()) t->removeRow(row);
}
void qth_table_clear(int64_t h) {
    if (auto* t = get<QTableWidget>(h)) { QSignalBlocker b(t); t->clearContents(); }
}

int32_t qth_table_current_row(int64_t h) { auto* t = get<QTableWidget>(h); return t ? t->currentRow() : -1; }
int32_t qth_table_current_col(int64_t h) { auto* t = get<QTableWidget>(h); return t ? t->currentColumn() : -1; }
void qth_table_set_current(int64_t h, int32_t row, int32_t col) {
    auto* t = get<QTableWidget>(h);
    if (in_range(t, row, col)) t->setCurrentCell(row, col);
}

void qth_table_set_sorting(int64_t h, int8_t enabled) {
    if (auto* t = get<QTableWidget>(h)) t->setSortingEnabled(enabled != 0);
}
void qth_table_sort_by(int64_t h, int32_t col, int8_t descending) {
    auto* t = get<QTableWidget>(h);
    if (t && col >= 0 && col < t->columnCount()) {
        t->sortItems(col, descending ? Qt::DescendingOrder : Qt::AscendingOrder);
    }
}
void qth_table_set_editable(int64_t h, int8_t editable) {
    if (auto* t = get<QTableWidget>(h)) {
        t->setEditTriggers(editable ? QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
                                    : QAbstractItemView::NoEditTriggers);
    }
}
void qth_table_set_select_rows(int64_t h, int8_t whole_rows) {
    if (auto* t = get<QTableWidget>(h)) {
        t->setSelectionBehavior(whole_rows ? QAbstractItemView::SelectRows : QAbstractItemView::SelectItems);
    }
}
void qth_table_set_alternating_colors(int64_t h, int8_t on) {
    if (auto* t = get<QTableWidget>(h)) t->setAlternatingRowColors(on != 0);
}
void qth_table_set_col_width(int64_t h, int32_t col, int32_t px) {
    auto* t = get<QTableWidget>(h);
    if (t && col >= 0 && col < t->columnCount() && px > 0) t->setColumnWidth(col, px);
}
void qth_table_stretch_columns(int64_t h, int8_t stretch) {
    if (auto* t = get<QTableWidget>(h)) {
        t->horizontalHeader()->setSectionResizeMode(stretch ? QHeaderView::Stretch : QHeaderView::Interactive);
    }
}
void qth_table_set_cell_background(int64_t h, int32_t row, int32_t col, int32_t rgb) {
    auto* t = get<QTableWidget>(h);
    if (!in_range(t, row, col)) return;
    QSignalBlocker b(t);
    ensure_item(t, row, col)->setBackground(color_from_rgb(rgb));
}
void qth_table_set_cell_foreground(int64_t h, int32_t row, int32_t col, int32_t rgb) {
    auto* t = get<QTableWidget>(h);
    if (!in_range(t, row, col)) return;
    QSignalBlocker b(t);
    ensure_item(t, row, col)->setForeground(color_from_rgb(rgb));
}

} // extern "C"

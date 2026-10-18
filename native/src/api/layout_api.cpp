#include "common.hpp"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

using namespace qth;
using namespace qth::api;

extern "C" {

int64_t qth_vbox_new(void) { return reg(new QVBoxLayout()); }
int64_t qth_hbox_new(void) { return reg(new QHBoxLayout()); }
int64_t qth_grid_new(void) { return reg(new QGridLayout()); }
int64_t qth_form_new(void) { return reg(new QFormLayout()); }

void qth_layout_add_widget(int64_t layout_h, int64_t widget_h) {
    auto* lay = layout(layout_h);
    auto* w   = widget(widget_h);
    if (lay && w) lay->addWidget(w);
}

void qth_layout_add_layout(int64_t layout_h, int64_t child_h) {
    auto* lay   = layout(layout_h);
    auto* child = layout(child_h);
    if (!lay || !child || lay == child) return;
    if (auto* box = qobject_cast<QBoxLayout*>(lay)) {
        box->addLayout(child);
    } else if (auto* grid = qobject_cast<QGridLayout*>(lay)) {
        grid->addLayout(child, grid->rowCount(), 0, 1, grid->columnCount() > 0 ? grid->columnCount() : 1);
    } else if (auto* form = qobject_cast<QFormLayout*>(lay)) {
        form->addRow(child);
    }
}

void qth_layout_add_spacing(int64_t layout_h, int32_t px) {
    if (auto* box = qobject_cast<QBoxLayout*>(layout(layout_h))) box->addSpacing(px);
}

void qth_layout_add_stretch(int64_t layout_h, int32_t stretch) {
    if (auto* box = qobject_cast<QBoxLayout*>(layout(layout_h))) box->addStretch(stretch);
}

void qth_layout_set_margins(int64_t layout_h, int32_t l, int32_t t, int32_t r, int32_t b) {
    if (auto* lay = layout(layout_h)) lay->setContentsMargins(l, t, r, b);
}

void qth_layout_set_spacing(int64_t layout_h, int32_t px) {
    if (auto* lay = layout(layout_h)) lay->setSpacing(px);
}

void qth_grid_add_widget(int64_t layout_h, int64_t widget_h, int32_t row, int32_t col,
                         int32_t row_span, int32_t col_span) {
    auto* grid = get<QGridLayout>(layout_h);
    auto* w    = widget(widget_h);
    if (grid && w) grid->addWidget(w, row, col, row_span > 0 ? row_span : 1, col_span > 0 ? col_span : 1);
}

void qth_form_add_row(int64_t layout_h, const char* label, int64_t widget_h) {
    auto* form = get<QFormLayout>(layout_h);
    auto* w    = widget(widget_h);
    if (form && w) form->addRow(qs(label), w);
}

void qth_widget_set_layout(int64_t widget_h, int64_t layout_h) {
    auto* w   = widget(widget_h);
    auto* lay = layout(layout_h);
    // QWidget::setLayout() warns and ignores a second layout; skip it quietly.
    if (w && lay && !w->layout()) w->setLayout(lay);
}

} // extern "C"

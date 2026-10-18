#include "common.hpp"

#include <QCheckBox>
#include <QFont>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>

#include "../core/hwindow.hpp"

using namespace qth;
using namespace qth::api;

extern "C" {

// ── creation ────────────────────────────────────────────────────────────

int64_t qth_window_new(const char* title, int32_t width, int32_t height) {
    auto* w = new HWindow();
    if (title) w->setWindowTitle(qs(title));
    if (width > 0 && height > 0) w->resize(width, height);
    return reg(w);
}

int64_t qth_widget_new(void) { return reg(new QWidget()); }

int64_t qth_button_new(const char* text) {
    auto* b = new QPushButton(qs(text));
    const int64_t id = reg(b);
    QObject::connect(b, &QPushButton::clicked, [id](bool) {
        emit_event(QTH_EVENT_CLICKED, id);
    });
    return id;
}

int64_t qth_label_new(const char* text) { return reg(new QLabel(qs(text))); }

int64_t qth_checkbox_new(const char* text) {
    auto* c = new QCheckBox(qs(text));
    const int64_t id = reg(c);
    QObject::connect(c, &QCheckBox::toggled, [id](bool on) {
        emit_event(QTH_EVENT_TOGGLED, id, on ? 1 : 0);
    });
    return id;
}

int64_t qth_progress_new(int32_t min, int32_t max, int32_t initial) {
    auto* p = new QProgressBar();
    p->setRange(min, max);
    p->setValue(initial);
    return reg(p);
}

int64_t qth_groupbox_new(const char* title) {
    auto* g = new QGroupBox(qs(title));
    const int64_t id = reg(g);
    QObject::connect(g, &QGroupBox::toggled, [id](bool on) {
        emit_event(QTH_EVENT_TOGGLED, id, on ? 1 : 0);
    });
    return id;
}

int64_t qth_tabs_new(void) {
    auto* t = new QTabWidget();
    const int64_t id = reg(t);
    QObject::connect(t, &QTabWidget::currentChanged, [id](int index) {
        emit_event(QTH_EVENT_TAB_CHANGED, id, index);
    });
    return id;
}

int64_t qth_scroll_new(void) { return reg(new QScrollArea()); }

int64_t qth_separator_new(int8_t vertical) {
    auto* f = new QFrame();
    f->setFrameShape(vertical ? QFrame::VLine : QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return reg(f);
}

// ── generic QWidget operations ──────────────────────────────────────────

void qth_widget_show(int64_t h)  { if (auto* w = widget(h)) w->show(); }
void qth_widget_hide(int64_t h)  { if (auto* w = widget(h)) w->hide(); }
void qth_widget_close(int64_t h) { if (auto* w = widget(h)) w->close(); }

void qth_widget_resize(int64_t h, int32_t w, int32_t hgt) {
    if (auto* wid = widget(h)) wid->resize(w, hgt);
}
void qth_widget_move(int64_t h, int32_t x, int32_t y) {
    if (auto* w = widget(h)) w->move(x, y);
}
void qth_widget_set_enabled(int64_t h, int8_t enabled) {
    if (auto* w = widget(h)) w->setEnabled(enabled != 0);
}
void qth_widget_set_visible(int64_t h, int8_t visible) {
    if (auto* w = widget(h)) w->setVisible(visible != 0);
}
void qth_widget_set_style_sheet(int64_t h, const char* css) {
    if (auto* w = widget(h)) w->setStyleSheet(qs(css));
}
void qth_widget_set_fixed_size(int64_t h, int32_t w, int32_t hgt) {
    if (auto* wid = widget(h)) wid->setFixedSize(w, hgt);
}
void qth_widget_set_minimum_size(int64_t h, int32_t w, int32_t hgt) {
    if (auto* wid = widget(h)) wid->setMinimumSize(w, hgt);
}
void qth_widget_set_maximum_size(int64_t h, int32_t w, int32_t hgt) {
    if (auto* wid = widget(h)) wid->setMaximumSize(w, hgt);
}
void qth_widget_set_tooltip(int64_t h, const char* text) {
    if (auto* w = widget(h)) w->setToolTip(qs(text));
}
void qth_widget_set_object_name(int64_t h, const char* name) {
    if (auto* w = widget(h)) w->setObjectName(qs(name));
}
void qth_widget_set_font(int64_t h, const char* family, int32_t point_size, int8_t bold, int8_t italic) {
    auto* w = widget(h);
    if (!w) return;
    QFont f = w->font();
    if (family && *family) f.setFamily(qs(family));
    if (point_size > 0) f.setPointSize(point_size);
    f.setBold(bold != 0);
    f.setItalic(italic != 0);
    w->setFont(f);
}
void qth_widget_set_focus(int64_t h) { if (auto* w = widget(h)) w->setFocus(); }

int8_t  qth_widget_is_visible(int64_t h) { auto* w = widget(h); return w && w->isVisible() ? 1 : 0; }
int8_t  qth_widget_is_enabled(int64_t h) { auto* w = widget(h); return w && w->isEnabled() ? 1 : 0; }
int32_t qth_widget_width(int64_t h)      { auto* w = widget(h); return w ? w->width()  : 0; }
int32_t qth_widget_height(int64_t h)     { auto* w = widget(h); return w ? w->height() : 0; }

void qth_widget_destroy(int64_t h) { if (auto* w = widget(h)) w->deleteLater(); }

// ── QLabel ──────────────────────────────────────────────────────────────

void qth_label_set_text(int64_t h, const char* text) {
    if (auto* l = get<QLabel>(h)) l->setText(qs(text));
}
const char* qth_label_get_text(int64_t h) {
    auto* l = get<QLabel>(h);
    return out(l ? l->text() : QString());
}
void qth_label_set_word_wrap(int64_t h, int8_t wrap) {
    if (auto* l = get<QLabel>(h)) l->setWordWrap(wrap != 0);
}
void qth_label_set_alignment(int64_t h, int32_t alignment) {
    auto* l = get<QLabel>(h);
    if (!l) return;
    switch (alignment) {
        case QTH_ALIGN_CENTER: l->setAlignment(Qt::AlignCenter); break;
        case QTH_ALIGN_RIGHT:  l->setAlignment(Qt::AlignRight | Qt::AlignVCenter); break;
        default:               l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); break;
    }
}

// ── QPushButton ─────────────────────────────────────────────────────────

void qth_button_set_text(int64_t h, const char* text) {
    if (auto* b = get<QPushButton>(h)) b->setText(qs(text));
}
const char* qth_button_get_text(int64_t h) {
    auto* b = get<QPushButton>(h);
    return out(b ? b->text() : QString());
}

// ── QCheckBox / QGroupBox ───────────────────────────────────────────────

int8_t qth_checkbox_is_checked(int64_t h) {
    auto* c = get<QCheckBox>(h);
    return c && c->isChecked() ? 1 : 0;
}
void qth_checkbox_set_checked(int64_t h, int8_t checked) {
    if (auto* c = get<QCheckBox>(h)) c->setChecked(checked != 0);
}
void qth_checkbox_set_text(int64_t h, const char* text) {
    if (auto* c = get<QCheckBox>(h)) c->setText(qs(text));
}

void qth_groupbox_set_title(int64_t h, const char* title) {
    if (auto* g = get<QGroupBox>(h)) g->setTitle(qs(title));
}
void qth_groupbox_set_checkable(int64_t h, int8_t checkable) {
    if (auto* g = get<QGroupBox>(h)) g->setCheckable(checkable != 0);
}
int8_t qth_groupbox_is_checked(int64_t h) {
    auto* g = get<QGroupBox>(h);
    return g && g->isChecked() ? 1 : 0;
}
void qth_groupbox_set_checked(int64_t h, int8_t checked) {
    if (auto* g = get<QGroupBox>(h)) g->setChecked(checked != 0);
}

// ── QProgressBar ────────────────────────────────────────────────────────

void qth_progress_set_value(int64_t h, int32_t value) {
    if (auto* p = get<QProgressBar>(h)) p->setValue(value);
}
int32_t qth_progress_get_value(int64_t h) {
    auto* p = get<QProgressBar>(h);
    return p ? p->value() : 0;
}
void qth_progress_set_range(int64_t h, int32_t min, int32_t max) {
    if (auto* p = get<QProgressBar>(h)) p->setRange(min, max);
}
void qth_progress_set_text_visible(int64_t h, int8_t visible) {
    if (auto* p = get<QProgressBar>(h)) p->setTextVisible(visible != 0);
}

// ── QTabWidget ──────────────────────────────────────────────────────────

int32_t qth_tabs_add_tab(int64_t tabs, int64_t page, const char* title) {
    auto* t = get<QTabWidget>(tabs);
    auto* p = widget(page);
    if (!t || !p) return -1;
    return t->addTab(p, qs(title));
}
int32_t qth_tabs_get_index(int64_t tabs) {
    auto* t = get<QTabWidget>(tabs);
    return t ? t->currentIndex() : -1;
}
void qth_tabs_set_index(int64_t tabs, int32_t index) {
    if (auto* t = get<QTabWidget>(tabs)) t->setCurrentIndex(index);
}
int32_t qth_tabs_count(int64_t tabs) {
    auto* t = get<QTabWidget>(tabs);
    return t ? t->count() : 0;
}

// ── QScrollArea ─────────────────────────────────────────────────────────

void qth_scroll_set_widget(int64_t scroll, int64_t w) {
    auto* s = get<QScrollArea>(scroll);
    auto* c = widget(w);
    if (s && c) s->setWidget(c);
}
void qth_scroll_set_widget_resizable(int64_t scroll, int8_t resizable) {
    if (auto* s = get<QScrollArea>(scroll)) s->setWidgetResizable(resizable != 0);
}

} // extern "C"

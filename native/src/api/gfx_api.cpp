#include "common.hpp"

#include "../core/canvas.hpp"
#include "../core/input_filter.hpp"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QShortcut>
#include <QKeySequence>

using namespace qth;
using namespace qth::api;

namespace {

// "theme:NAME" → desktop icon theme, anything else → file path.
QIcon load_icon(const QString& spec) {
    static const QString prefix = QStringLiteral("theme:");
    if (spec.startsWith(prefix)) return QIcon::fromTheme(spec.mid(prefix.size()));
    return QIcon(spec);
}

QColor fill_color(int32_t rgb) { return rgb < 0 ? QColor() : color_from_rgb(rgb); }

} // namespace

extern "C" {

// ── images & icons ──────────────────────────────────────────────────────

int8_t qth_label_set_image(int64_t h, const char* path) {
    auto* l = get<QLabel>(h);
    if (!l) return 0;
    const QString spec = qs(path);
    QPixmap pm;
    if (spec.startsWith(QStringLiteral("theme:"))) pm = load_icon(spec).pixmap(64, 64);
    else pm.load(spec);
    if (pm.isNull()) return 0;
    l->setPixmap(pm);
    return 1;
}

void qth_label_set_scaled(int64_t h, int8_t scaled) {
    if (auto* l = get<QLabel>(h)) l->setScaledContents(scaled != 0);
}

void qth_button_set_icon(int64_t h, const char* path) {
    if (auto* b = get<QAbstractButton>(h)) b->setIcon(load_icon(qs(path)));
}

void qth_action_set_icon(int64_t h, const char* path) {
    if (auto* a = get<QAction>(h)) a->setIcon(load_icon(qs(path)));
}

void qth_window_set_icon(int64_t h, const char* path) {
    if (auto* w = widget(h)) w->setWindowIcon(load_icon(qs(path)));
}

void qth_app_set_icon(const char* path) {
    if (qApp) QApplication::setWindowIcon(load_icon(qs(path)));
}

int8_t qth_widget_grab_png(int64_t h, const char* path) {
    auto* w = widget(h);
    return w && w->grab().save(qs(path), "PNG") ? 1 : 0;
}

// ── canvas ──────────────────────────────────────────────────────────────

int64_t qth_canvas_new(int32_t width, int32_t height) { return reg(new Canvas(width, height)); }

void qth_canvas_clear(int64_t h) { if (auto* c = get<Canvas>(h)) c->clear_ops(); }
void qth_canvas_update(int64_t h) { if (auto* c = get<Canvas>(h)) c->update(); }

void qth_canvas_set_background(int64_t h, int32_t rgb) {
    if (auto* c = get<Canvas>(h)) c->set_background(color_from_rgb(rgb));
}
void qth_canvas_set_pen(int64_t h, int32_t rgb, double width) {
    if (auto* c = get<Canvas>(h)) c->set_pen(color_from_rgb(rgb), width > 0 ? width : 1);
}
void qth_canvas_set_fill(int64_t h, int32_t rgb) {
    if (auto* c = get<Canvas>(h)) c->set_fill(fill_color(rgb));
}
void qth_canvas_set_font(int64_t h, const char* family, int32_t point_size, int8_t bold, int8_t italic) {
    if (auto* c = get<Canvas>(h)) c->set_font_spec(qs(family), point_size, bold != 0, italic != 0);
}

void qth_canvas_draw_line(int64_t h, double x1, double y1, double x2, double y2) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Line, x1, y1, x2, y2);
}
void qth_canvas_draw_rect(int64_t h, double x, double y, double w, double hh) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Rect, x, y, w, hh);
}
void qth_canvas_draw_round_rect(int64_t h, double x, double y, double w, double hh) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::RoundRect, x, y, w, hh);
}
void qth_canvas_draw_ellipse(int64_t h, double x, double y, double w, double hh) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Ellipse, x, y, w, hh);
}
void qth_canvas_draw_point(int64_t h, double x, double y) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Point, x, y, 0, 0);
}
void qth_canvas_draw_text(int64_t h, double x, double y, const char* text) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Text, x, y, 0, 0, qs(text));
}
void qth_canvas_draw_image(int64_t h, const char* path, double x, double y, double w, double hh) {
    if (auto* c = get<Canvas>(h)) c->add(Canvas::Kind::Image, x, y, w, hh, qs(path));
}
int32_t qth_canvas_op_count(int64_t h) {
    auto* c = get<Canvas>(h);
    return c ? static_cast<int32_t>(c->op_count()) : 0;
}

// ── input ───────────────────────────────────────────────────────────────

void qth_widget_watch_input(int64_t h, int32_t mask) {
    auto* w = widget(h);
    if (!w) return;
    InputFilter* f = InputFilter::of(w);
    if (mask == 0) {
        if (f) f->set_mask(0);
        return;
    }
    if (f) f->set_mask(mask); else new InputFilter(w, mask);
    if ((mask & InputFilter::Keys) && w->focusPolicy() == Qt::NoFocus) w->setFocusPolicy(Qt::StrongFocus);
    if (mask & InputFilter::MouseMove) w->setMouseTracking(true);
}

int64_t qth_shortcut_new(int64_t h, const char* sequence) {
    auto* w = widget(h);
    if (!w) return 0;
    auto* sc = new QShortcut(QKeySequence(qs(sequence)), w);
    const int64_t id = reg(sc);
    QObject::connect(sc, &QShortcut::activated, [id] { emit_event(QTH_EVENT_SHORTCUT, id); });
    return id;
}

} // extern "C"

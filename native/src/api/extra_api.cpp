#include "common.hpp"

#include <QCalendarWidget>
#include <QDate>
#include <QDateEdit>
#include <QDockWidget>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QSplitter>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTime>
#include <QTimeEdit>

using namespace qth;
using namespace qth::api;

namespace {

int32_t ymd(const QDate& d) { return d.isValid() ? d.year() * 10000 + d.month() * 100 + d.day() : 0; }
int32_t secs(const QTime& t) { return t.isValid() ? t.hour() * 3600 + t.minute() * 60 + t.second() : 0; }

QDate make_date(int y, int m, int d) {
    const QDate dt(y, m, d);
    return dt.isValid() ? dt : QDate::currentDate();
}
QTime make_time(int h, int m, int s) {
    const QTime t(h, m, s);
    return t.isValid() ? t : QTime(0, 0, 0);
}

void emit_date(int64_t id, const QDate& d) {
    emit_event(QTH_EVENT_DATE_CHANGED, id, ymd(d), d.toString(Qt::ISODate));
}

QIcon icon_from(const QString& spec) {
    static const QString prefix = QStringLiteral("theme:");
    return spec.startsWith(prefix) ? QIcon::fromTheme(spec.mid(prefix.size())) : QIcon(spec);
}

} // namespace

extern "C" {

// ── date / time / calendar ──────────────────────────────────────────────

int64_t qth_dateedit_new(int32_t y, int32_t m, int32_t d) {
    auto* e = new QDateEdit(make_date(y, m, d));
    e->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    const int64_t id = reg(e);
    QObject::connect(e, &QDateEdit::dateChanged, [id](const QDate& dt) { emit_date(id, dt); });
    return id;
}
void qth_dateedit_set_date(int64_t h, int32_t y, int32_t m, int32_t d) {
    if (auto* e = get<QDateEdit>(h)) e->setDate(make_date(y, m, d));
}
int32_t qth_dateedit_get_ymd(int64_t h) { auto* e = get<QDateEdit>(h); return e ? ymd(e->date()) : 0; }
const char* qth_dateedit_get_text(int64_t h) {
    auto* e = get<QDateEdit>(h);
    return out(e ? e->date().toString(Qt::ISODate) : QString());
}
void qth_dateedit_set_popup(int64_t h, int8_t popup) {
    if (auto* e = get<QDateEdit>(h)) e->setCalendarPopup(popup != 0);
}

int64_t qth_timeedit_new(int32_t hh, int32_t mm, int32_t ss) {
    auto* e = new QTimeEdit(make_time(hh, mm, ss));
    e->setDisplayFormat(QStringLiteral("HH:mm:ss"));
    const int64_t id = reg(e);
    QObject::connect(e, &QTimeEdit::timeChanged, [id](const QTime& t) {
        emit_event(QTH_EVENT_TIME_CHANGED, id, secs(t), t.toString(QStringLiteral("HH:mm:ss")));
    });
    return id;
}
void qth_timeedit_set_time(int64_t h, int32_t hh, int32_t mm, int32_t ss) {
    if (auto* e = get<QTimeEdit>(h)) e->setTime(make_time(hh, mm, ss));
}
int32_t qth_timeedit_get_seconds(int64_t h) { auto* e = get<QTimeEdit>(h); return e ? secs(e->time()) : 0; }
const char* qth_timeedit_get_text(int64_t h) {
    auto* e = get<QTimeEdit>(h);
    return out(e ? e->time().toString(QStringLiteral("HH:mm:ss")) : QString());
}

int64_t qth_calendar_new(void) {
    auto* c = new QCalendarWidget();
    const int64_t id = reg(c);
    QObject::connect(c, &QCalendarWidget::selectionChanged, [id, c] { emit_date(id, c->selectedDate()); });
    return id;
}
void qth_calendar_set_date(int64_t h, int32_t y, int32_t m, int32_t d) {
    if (auto* c = get<QCalendarWidget>(h)) c->setSelectedDate(make_date(y, m, d));
}
int32_t qth_calendar_get_ymd(int64_t h) { auto* c = get<QCalendarWidget>(h); return c ? ymd(c->selectedDate()) : 0; }
const char* qth_calendar_get_text(int64_t h) {
    auto* c = get<QCalendarWidget>(h);
    return out(c ? c->selectedDate().toString(Qt::ISODate) : QString());
}
int32_t qth_date_today_ymd(void) { return ymd(QDate::currentDate()); }

// ── splitter / stack / dock ─────────────────────────────────────────────

int64_t qth_splitter_new(int8_t vertical) {
    return reg(new QSplitter(vertical ? Qt::Vertical : Qt::Horizontal));
}
void qth_splitter_add_widget(int64_t h, int64_t w) {
    auto* s = get<QSplitter>(h);
    auto* c = widget(w);
    if (s && c) s->addWidget(c);
}
void qth_splitter_set_sizes2(int64_t h, int32_t a, int32_t b) {
    if (auto* s = get<QSplitter>(h)) s->setSizes({a, b});
}
void qth_splitter_set_sizes3(int64_t h, int32_t a, int32_t b, int32_t c) {
    if (auto* s = get<QSplitter>(h)) s->setSizes({a, b, c});
}
void qth_splitter_set_stretch(int64_t h, int32_t index, int32_t stretch) {
    auto* s = get<QSplitter>(h);
    if (s && index >= 0 && index < s->count()) s->setStretchFactor(index, stretch);
}

int64_t qth_stack_new(void) {
    auto* s = new QStackedWidget();
    const int64_t id = reg(s);
    QObject::connect(s, &QStackedWidget::currentChanged, [id](int i) {
        emit_event(QTH_EVENT_INDEX_CHANGED, id, i);
    });
    return id;
}
int32_t qth_stack_add_page(int64_t h, int64_t page) {
    auto* s = get<QStackedWidget>(h);
    auto* p = widget(page);
    return s && p ? s->addWidget(p) : -1;
}
void qth_stack_set_index(int64_t h, int32_t index) {
    auto* s = get<QStackedWidget>(h);
    if (s && index >= 0 && index < s->count()) s->setCurrentIndex(index);
}
int32_t qth_stack_get_index(int64_t h) { auto* s = get<QStackedWidget>(h); return s ? s->currentIndex() : -1; }
int32_t qth_stack_count(int64_t h) { auto* s = get<QStackedWidget>(h); return s ? s->count() : 0; }

int64_t qth_window_add_dock(int64_t window, const char* title, int64_t content, int32_t area) {
    auto* win = get<QMainWindow>(window);
    auto* w = widget(content);
    if (!win) return 0;
    auto* dock = new QDockWidget(qs(title), win);
    if (w) dock->setWidget(w);
    Qt::DockWidgetArea a = Qt::LeftDockWidgetArea;
    switch (area) {
        case QTH_DOCK_RIGHT:  a = Qt::RightDockWidgetArea;  break;
        case QTH_DOCK_TOP:    a = Qt::TopDockWidgetArea;    break;
        case QTH_DOCK_BOTTOM: a = Qt::BottomDockWidgetArea; break;
        default: break;
    }
    win->addDockWidget(a, dock);
    return reg(dock);
}

// ── standalone menus + tray ─────────────────────────────────────────────

int64_t qth_menu_new(const char* title) { return reg(new QMenu(qs(title))); }

void qth_menu_popup(int64_t h, int32_t x, int32_t y) {
    if (auto* m = get<QMenu>(h)) m->popup(QPoint(x, y));
}

int8_t qth_tray_available(void) { return QSystemTrayIcon::isSystemTrayAvailable() ? 1 : 0; }

int64_t qth_tray_new(const char* icon_path, const char* tooltip) {
    auto* t = new QSystemTrayIcon(icon_from(qs(icon_path)));
    t->setToolTip(qs(tooltip));
    const int64_t id = reg(t);
    QObject::connect(t, &QSystemTrayIcon::activated, [id](QSystemTrayIcon::ActivationReason r) {
        emit_event(QTH_EVENT_TRAY_ACTIVATED, id, static_cast<int32_t>(r));
    });
    return id;
}
void qth_tray_set_icon(int64_t h, const char* path) {
    if (auto* t = get<QSystemTrayIcon>(h)) t->setIcon(icon_from(qs(path)));
}
void qth_tray_set_tooltip(int64_t h, const char* text) {
    if (auto* t = get<QSystemTrayIcon>(h)) t->setToolTip(qs(text));
}
void qth_tray_set_menu(int64_t h, int64_t menu) {
    auto* t = get<QSystemTrayIcon>(h);
    auto* m = get<QMenu>(menu);
    if (t && m) t->setContextMenu(m);
}
void qth_tray_show(int64_t h) { if (auto* t = get<QSystemTrayIcon>(h)) t->show(); }
void qth_tray_hide(int64_t h) { if (auto* t = get<QSystemTrayIcon>(h)) t->hide(); }
void qth_tray_notify(int64_t h, const char* title, const char* message, int32_t kind, int32_t timeout_ms) {
    auto* t = get<QSystemTrayIcon>(h);
    if (!t) return;
    QSystemTrayIcon::MessageIcon ic = QSystemTrayIcon::NoIcon;
    if (kind == 1) ic = QSystemTrayIcon::Information;
    else if (kind == 2) ic = QSystemTrayIcon::Warning;
    else if (kind == 3) ic = QSystemTrayIcon::Critical;
    t->showMessage(qs(title), qs(message), ic, timeout_ms > 0 ? timeout_ms : 5000);
}

} // extern "C"

#include "common.hpp"

#include "../core/hwindow.hpp"

#include <QAction>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>

using namespace qth;
using namespace qth::api;

namespace {

int64_t make_action(QObject* parent, const QString& text) {
    auto* a = new QAction(text, parent);
    const int64_t id = reg(a);
    QObject::connect(a, &QAction::triggered, [id](bool checked) {
        emit_event(QTH_EVENT_ACTION_TRIGGERED, id, checked ? 1 : 0);
    });
    return id;
}

} // namespace

extern "C" {

void qth_window_set_central_widget(int64_t window_h, int64_t widget_h) {
    auto* win = get<QMainWindow>(window_h);
    auto* w   = widget(widget_h);
    if (win && w) win->setCentralWidget(w);
}

void qth_window_set_title(int64_t h, const char* title) {
    if (auto* w = widget(h)) w->setWindowTitle(qs(title));
}

void qth_window_set_auto_close(int64_t window_h, int8_t on) {
    // HWindow has no Q_OBJECT, so qobject_cast would also accept any plain
    // QMainWindow; dynamic_cast is the exact check.
    if (auto* w = dynamic_cast<HWindow*>(handles().raw(window_h))) w->set_auto_close(on != 0);
}

void qth_window_set_status(int64_t window_h, const char* text, int32_t timeout_ms) {
    if (auto* win = get<QMainWindow>(window_h)) {
        win->statusBar()->showMessage(qs(text), timeout_ms > 0 ? timeout_ms : 0);
    }
}

int64_t qth_window_add_menu(int64_t window_h, const char* title) {
    auto* win = get<QMainWindow>(window_h);
    if (!win) return 0;
    return reg(win->menuBar()->addMenu(qs(title)));
}

int64_t qth_window_add_toolbar_action(int64_t window_h, const char* text) {
    auto* win = get<QMainWindow>(window_h);
    if (!win) return 0;
    const int64_t id = make_action(win, qs(text));
    auto* tb = win->findChild<QToolBar*>(QStringLiteral("qth_toolbar"));
    if (!tb) {
        tb = win->addToolBar(QStringLiteral("Toolbar"));
        tb->setObjectName(QStringLiteral("qth_toolbar"));
    }
    if (auto* a = get<QAction>(id)) tb->addAction(a);
    return id;
}

int64_t qth_menu_add_action(int64_t menu_h, const char* text, const char* shortcut) {
    auto* menu = get<QMenu>(menu_h);
    if (!menu) return 0;
    const int64_t id = make_action(menu, qs(text));
    if (auto* a = get<QAction>(id)) {
        if (shortcut && *shortcut) a->setShortcut(QKeySequence(qs(shortcut)));
        menu->addAction(a);
    }
    return id;
}

void qth_menu_add_separator(int64_t menu_h) {
    if (auto* menu = get<QMenu>(menu_h)) menu->addSeparator();
}

int64_t qth_menu_add_submenu(int64_t menu_h, const char* title) {
    auto* menu = get<QMenu>(menu_h);
    return menu ? reg(menu->addMenu(qs(title))) : 0;
}

void qth_action_set_enabled(int64_t h, int8_t enabled) {
    if (auto* a = get<QAction>(h)) a->setEnabled(enabled != 0);
}
void qth_action_set_checkable(int64_t h, int8_t checkable) {
    if (auto* a = get<QAction>(h)) a->setCheckable(checkable != 0);
}
int8_t qth_action_is_checked(int64_t h) {
    auto* a = get<QAction>(h);
    return a && a->isChecked() ? 1 : 0;
}
void qth_action_set_checked(int64_t h, int8_t checked) {
    if (auto* a = get<QAction>(h)) a->setChecked(checked != 0);
}
void qth_action_set_text(int64_t h, const char* text) {
    if (auto* a = get<QAction>(h)) a->setText(qs(text));
}

} // extern "C"

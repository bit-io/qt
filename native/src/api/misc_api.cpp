#include "common.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>

using namespace qth;
using namespace qth::api;

extern "C" {

int64_t qth_timer_new(int32_t interval_ms, int8_t single_shot) {
    auto* t = new QTimer();           // parentless: lives until qth_object_destroy() / exit
    t->setInterval(interval_ms > 0 ? interval_ms : 1);
    t->setSingleShot(single_shot != 0);
    const int64_t id = reg(t);
    QObject::connect(t, &QTimer::timeout, [id] { emit_event(QTH_EVENT_TIMER, id); });
    return id;
}

void qth_timer_start(int64_t h) { if (auto* t = get<QTimer>(h)) t->start(); }
void qth_timer_stop(int64_t h)  { if (auto* t = get<QTimer>(h)) t->stop(); }

void qth_timer_set_interval(int64_t h, int32_t interval_ms) {
    if (auto* t = get<QTimer>(h)) t->setInterval(interval_ms > 0 ? interval_ms : 1);
}

int8_t qth_timer_is_active(int64_t h) {
    auto* t = get<QTimer>(h);
    return t && t->isActive() ? 1 : 0;
}

void qth_clipboard_set_text(const char* text) {
    if (auto* cb = QGuiApplication::clipboard()) cb->setText(qs(text));
}

const char* qth_clipboard_get_text(void) {
    auto* cb = QGuiApplication::clipboard();
    return out(cb ? cb->text() : QString());
}

} // extern "C"

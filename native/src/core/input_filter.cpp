#include "input_filter.hpp"

#include "events.hpp"
#include "handles.hpp"
#include "qthsharp.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QWidget>

namespace qth {

namespace {

int32_t mods_of(Qt::KeyboardModifiers m) {
    int32_t r = 0;
    if (m & Qt::ShiftModifier)   r |= 1;
    if (m & Qt::ControlModifier) r |= 2;
    if (m & Qt::AltModifier)     r |= 4;
    if (m & Qt::MetaModifier)    r |= 8;
    return r;
}

// 1 = left, 2 = right, 3 = middle, 0 = other
int32_t button_of(Qt::MouseButton b) {
    switch (b) {
        case Qt::LeftButton:   return 1;
        case Qt::RightButton:  return 2;
        case Qt::MiddleButton: return 3;
        default:               return 0;
    }
}

} // namespace

InputFilter::InputFilter(QObject* target, int mask) : QObject(target), mask_(mask) {
    target->installEventFilter(this);
}

InputFilter* InputFilter::of(QObject* target) {
    return target->findChild<InputFilter*>(QString(), Qt::FindDirectChildrenOnly);
}

bool InputFilter::eventFilter(QObject* watched, QEvent* ev) {
    const int64_t id = handles().id_of(watched);
    if (id == 0) return false;

    switch (ev->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        if (mask_ & Keys) {
            auto* k = static_cast<QKeyEvent*>(ev);
            events().push(ev->type() == QEvent::KeyPress ? QTH_EVENT_KEY_PRESSED : QTH_EVENT_KEY_RELEASED,
                          id, k->key(), k->text(), 0, 0, mods_of(k->modifiers()));
        }
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        if (mask_ & MouseButtons) {
            auto* m = static_cast<QMouseEvent*>(ev);
            events().push(ev->type() == QEvent::MouseButtonPress ? QTH_EVENT_MOUSE_PRESSED
                                                                 : QTH_EVENT_MOUSE_RELEASED,
                          id, button_of(m->button()), {}, m->position().toPoint().x(),
                          m->position().toPoint().y(), mods_of(m->modifiers()));
        }
        break;
    case QEvent::MouseMove:
        if (mask_ & MouseMove) {
            auto* m = static_cast<QMouseEvent*>(ev);
            events().push(QTH_EVENT_MOUSE_MOVED, id, static_cast<int32_t>(m->buttons()), {},
                          m->position().toPoint().x(), m->position().toPoint().y(),
                          mods_of(m->modifiers()));
        }
        break;
    case QEvent::Wheel:
        if (mask_ & Wheel) {
            auto* w = static_cast<QWheelEvent*>(ev);
            events().push(QTH_EVENT_WHEEL, id, w->angleDelta().y(), {}, w->position().toPoint().x(),
                          w->position().toPoint().y(), mods_of(w->modifiers()));
        }
        break;
    case QEvent::Resize:
        if (mask_ & Resize) {
            auto* r = static_cast<QResizeEvent*>(ev);
            events().push(QTH_EVENT_RESIZED, id, 0, {}, r->size().width(), r->size().height());
        }
        break;
    default:
        break;
    }
    return false;   // never swallow: normal widget behaviour continues
}

} // namespace qth

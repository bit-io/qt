#pragma once

#include "qthsharp.h"

#include "../core/events.hpp"
#include "../core/handles.hpp"
#include "../core/text.hpp"

#include <QColor>
#include <QLayout>
#include <QString>
#include <QWidget>

namespace qth::api {

// Typed handle lookup: nullptr when the handle is stale or of another type.
template <class T>
inline T* get(int64_t h) { return handles().as<T>(h); }

inline QWidget* widget(int64_t h) { return get<QWidget>(h); }
inline QLayout* layout(int64_t h) { return get<QLayout>(h); }

inline QString qs(const char* s) { return text::to_qstring(s); }
inline const char* out(const QString& s) { return text::dup(s); }

// Registers a freshly created object and returns its handle.
template <class T>
inline int64_t reg(T* obj) { return handles().add(obj); }

inline void emit_event(int32_t kind, int64_t handle, int32_t value = 0, QString text = {},
                       int32_t x = 0, int32_t y = 0, int32_t mods = 0) {
    events().push(kind, handle, value, std::move(text), x, y, mods);
}

// 0xRRGGBB <-> QColor
inline QColor color_from_rgb(int32_t rgb) {
    return QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

} // namespace qth::api

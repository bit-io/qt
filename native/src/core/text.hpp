#pragma once

#include <QString>
#include <cstdlib>
#include <cstring>

namespace qth::text {

// nullptr-safe: a null C string becomes an empty QString.
inline QString to_qstring(const char* s) {
    return s ? QString::fromUtf8(s) : QString();
}

// malloc'ed, NUL-terminated UTF-8 copy. The H# side releases it with
// qth_free_string() (see src/qt_ffi.h#). Never returns nullptr.
inline char* dup(const QString& s) {
    const QByteArray utf8 = s.toUtf8();
    const auto n = static_cast<size_t>(utf8.size());
    char* out = static_cast<char*>(std::malloc(n + 1));
    if (!out) {
        out = static_cast<char*>(std::calloc(1, 1));
        return out;
    }
    std::memcpy(out, utf8.constData(), n);
    out[n] = '\0';
    return out;
}

} // namespace qth::text

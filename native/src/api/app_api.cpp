#include "common.hpp"

#include "../core/app_host.hpp"

#include <QApplication>
#include <QEvent>
#include <QPalette>
#include <QStyle>
#include <QLatin1String>

using namespace qth;
using namespace qth::api;

extern "C" {

const char* qth_version(void) {
    return out(QStringLiteral("%1.%2.%3")
                   .arg(QTH_VERSION_MAJOR).arg(QTH_VERSION_MINOR).arg(QTH_VERSION_PATCH));
}

const char* qth_qt_version(void) { return out(QString::fromLatin1(qVersion())); }

int64_t qth_app_init(void) { return host().init() ? 1 : 0; }

int64_t qth_app_init_platform(const char* qpa) { return host().init(qpa) ? 1 : 0; }

void qth_app_shutdown(void) { host().shutdown(); }

int32_t qth_app_exec(void) {
    if (!host().app()) return -1;
    const int rc = host().app()->exec();
    host().set_running(false);
    return rc;
}

void qth_app_quit(void) {
    host().set_running(false);
    if (host().app()) host().app()->quit();
}

void qth_app_process_events(void) {
    if (!host().app()) return;
    QApplication::processEvents();
    // processEvents() never delivers DeferredDelete events, so without this a
    // poll-based H# loop would never actually free anything it destroy()ed.
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

int8_t qth_app_is_running(void) { return host().running() ? 1 : 0; }

void qth_app_set_style(const char* style_name) {
    if (style_name) QApplication::setStyle(qs(style_name));
}

void qth_app_set_app_name(const char* name) {
    if (name && host().app()) host().app()->setApplicationName(qs(name));
}

void qth_app_set_style_sheet(const char* css) {
    if (host().app()) host().app()->setStyleSheet(qs(css));
}

void qth_app_set_quit_on_last_window_closed(int8_t on) {
    if (host().app()) host().app()->setQuitOnLastWindowClosed(on != 0);
}

int8_t qth_app_set_theme(const char* name) {
    if (!host().app()) return 0;
    // Remember what the platform gave us so "system" can restore it.
    static const QString initial_style = QApplication::style()->objectName();
    static const QPalette initial_palette = QApplication::palette();

    const QString n = qs(name).toLower();
    if (n == QLatin1String("dark")) {
        QApplication::setStyle(QStringLiteral("Fusion"));
        QPalette p;
        const QColor base(30, 30, 34), window(45, 45, 50), text(228, 228, 232), accent(66, 133, 244);
        p.setColor(QPalette::Window, window);
        p.setColor(QPalette::WindowText, text);
        p.setColor(QPalette::Base, base);
        p.setColor(QPalette::AlternateBase, window);
        p.setColor(QPalette::ToolTipBase, window);
        p.setColor(QPalette::ToolTipText, text);
        p.setColor(QPalette::Text, text);
        p.setColor(QPalette::Button, window);
        p.setColor(QPalette::ButtonText, text);
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Link, accent);
        p.setColor(QPalette::Highlight, accent);
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 126));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 126));
        p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 126));
        QApplication::setPalette(p);
        return 1;
    }
    if (n == QLatin1String("light")) {
        QApplication::setStyle(QStringLiteral("Fusion"));
        QApplication::setPalette(QApplication::style()->standardPalette());
        return 1;
    }
    if (n == QLatin1String("system")) {
        if (!initial_style.isEmpty()) QApplication::setStyle(initial_style);
        QApplication::setPalette(initial_palette);
        return 1;
    }
    return 0;
}

int8_t qth_handle_is_valid(int64_t h) { return handles().valid(h) ? 1 : 0; }

void qth_object_destroy(int64_t h) {
    if (auto* o = handles().raw(h)) o->deleteLater();
}

int64_t qth_handle_count(void) { return static_cast<int64_t>(handles().size()); }

} // extern "C"

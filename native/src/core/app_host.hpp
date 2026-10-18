#pragma once

#include <QApplication>

namespace qth {

class AppHost {
public:
    static AppHost& instance();

    // true  → created now;  false → one already existed.
    // `platform` (e.g. "offscreen") is applied through QT_QPA_PLATFORM unless
    // the environment already sets it.
    bool init(const char* platform = nullptr);

    QApplication* app() const { return app_; }

    // Destroys the QApplication (and every top-level widget with it). Optional:
    // if never called, the QApplication is deliberately left alive at process
    // exit — destroying it from a static destructor crashes inside Qt, because
    // Qt's own static state is already gone by then.
    void shutdown();
    bool running() const { return running_; }
    void set_running(bool r) { running_ = r; }

private:
    AppHost() = default;

    // QApplication keeps pointers to argc/argv for its whole lifetime.
    int   argc_ = 1;
    char  prog_[16] = "h-sharp-qt-app";
    char* argv_[2]  = {prog_, nullptr};

    QApplication* app_ = nullptr;
    bool running_ = false;
};

inline AppHost& host() { return AppHost::instance(); }

} // namespace qth

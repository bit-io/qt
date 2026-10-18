#pragma once

#include <QMainWindow>

namespace qth {

class HWindow : public QMainWindow {
public:
    using QMainWindow::QMainWindow;

    // false (default): the window stays open after QTH_EVENT_WINDOW_CLOSED and
    //                  H# decides (quit / destroy / ignore).
    // true:            the event is still sent, then the window closes.
    void set_auto_close(bool on) { auto_close_ = on; }
    bool auto_close() const { return auto_close_; }

protected:
    void closeEvent(QCloseEvent* ev) override;

private:
    bool auto_close_ = false;
};

} // namespace qth

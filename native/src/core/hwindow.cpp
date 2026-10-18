#include "hwindow.hpp"

#include "events.hpp"
#include "handles.hpp"
#include "qthsharp.h"

#include <QCloseEvent>

namespace qth {

void HWindow::closeEvent(QCloseEvent* ev) {
    events().push(QTH_EVENT_WINDOW_CLOSED, handles().id_of(this));
    if (auto_close_) {
        ev->accept();
    } else {
        ev->ignore();
    }
}

} // namespace qth

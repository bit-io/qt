#include "app_host.hpp"

#include <QByteArray>

namespace qth {

AppHost& AppHost::instance() {
    static AppHost h;
    return h;
}

bool AppHost::init(const char* platform) {
    if (app_) return false;
    if (platform && *platform && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray(platform));
    }
    app_     = new QApplication(argc_, argv_);
    running_ = true;
    return true;
}

} // namespace qth

namespace qth {

void AppHost::shutdown() {
    if (!app_) return;
    running_ = false;
    delete app_;
    app_ = nullptr;
}

} // namespace qth

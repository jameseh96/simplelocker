#include "XIdleChecker.h"


#include <X11/extensions/scrnsaver.h>


XIdleChecker::XIdleChecker() {
    display = XOpenDisplay(nullptr);

    if (!display) {
        throw std::runtime_error("Could not open display to check idle time");
    }
}


void XIdleChecker::checkIdleTime() {

    XScreenSaverInfo* info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
    auto idleTime = std::chrono::milliseconds(info->idle);

    auto lowerBound = idleTime < lastIdleTime ? 0ms : lastIdleTime;

    for (auto& pair : idleCallbacks) {
        auto threshold = pair.first;
        auto& cb = pair.second;
        if (threshold > lowerBound && threshold <= idleTime) {
            cb();
        }
    }
    lastIdleTime = idleTime;

}

XIdleChecker::~XIdleChecker() {
    XCloseDisplay(display);
}

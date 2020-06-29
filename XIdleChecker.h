#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <vector>

using namespace std::chrono_literals;

typedef struct _XDisplay Display;

class XIdleChecker {
public:
    XIdleChecker();

    using Callback = std::function<void()>;

    void onIdle(std::chrono::milliseconds idleTime, const Callback& func) {
        idleCallbacks.emplace_back(idleTime, func);
    }

    void checkIdleTime();
    ~XIdleChecker();

private:
    Display* display;

    std::chrono::milliseconds lastIdleTime = 0ms;
    std::vector<std::pair<std::chrono::milliseconds, Callback>> idleCallbacks;

};


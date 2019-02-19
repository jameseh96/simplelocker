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

    template<class Func>
    void onIdle(std::chrono::milliseconds idleTime, Func&& func) {
        idleCallbacks.emplace_back(idleTime, std::forward<Func>(func));
    }

    void checkIdleTime();
    ~XIdleChecker();

private:
    Display* display;

    std::chrono::milliseconds lastIdleTime = 0ms;
    std::vector<std::pair<std::chrono::milliseconds, std::function<void()>>> idleCallbacks;

};


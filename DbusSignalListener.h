#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <vector>

#include "Inhibitor.h"


class DbusSignalListener {
public:
    DbusSignalListener() : proxy(
            sdbus::createObjectProxy("org.freedesktop.login1", "/org/freedesktop/login1")),
                                     inhibitor(proxy, "sleep:shutdown", "event-runner", "", "delay") {

        const char* interface = "org.freedesktop.login1.Manager";
        proxy->uponSignal("PrepareForSleep")
                .onInterface(interface)
                .call([this](bool before) { onSleepCallback(before); });

        proxy->uponSignal("PrepareForShutdown")
                .onInterface(interface)
                .call([this](bool before) { onShutdownCallback(before); });
        proxy->finishRegistration();
    }

    template<class Func>
    void onSleep(Func&& func) { addCallback(std::forward<Func>(func), sleepCallbacks); }

    template<class Func>
    void onWake(Func&& func) { addCallback(std::forward<Func>(func), wakeCallbacks); }

    template<class Func>
    void onShutdown(Func&& func) { addCallback(std::forward<Func>(func), shutdownCallbacks); }

    // does this actually happen?
    template<class Func>
    void afterShutdown(Func&& func) { addCallback(std::forward<Func>(func), afterShutdownCallbacks); }

private:
    template<class Func>
    void addCallback(Func&& func, std::vector<std::function<void()>>& callbacks) {
        callbacks.push_back(std::forward<Func>(func));
    }

    void onSleepCallback(bool before) {
        if (before) {
            dispatch(sleepCallbacks);
            inhibitor.release();
        } else {
            inhibitor.acquire();
            dispatch(wakeCallbacks);
        }
    }
    void onShutdownCallback(bool before) {
        if (before) {
            dispatch(shutdownCallbacks);
            inhibitor.release();
        } else {
            inhibitor.acquire();
            dispatch(afterShutdownCallbacks);
        }

    };
    void dispatch(std::vector<std::function<void()>>& callbacks) {
        for (auto& cb : callbacks) {
            cb();
        }
    }

    std::unique_ptr<sdbus::IObjectProxy> proxy;

    std::vector<std::function<void()>> sleepCallbacks;
    std::vector<std::function<void()>> wakeCallbacks;
    std::vector<std::function<void()>> shutdownCallbacks;
    std::vector<std::function<void()>> afterShutdownCallbacks;

    Inhibitor inhibitor;
};


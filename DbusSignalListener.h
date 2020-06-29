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
    
    using Callback = std::function<void()>;
    
    void onSleep(const Callback& callback) { addCallback(callback, sleepCallbacks); }
    
    void onWake(const Callback& callback) { addCallback(callback, wakeCallbacks); }
    
    void onShutdown(const Callback& callback) { addCallback(callback, shutdownCallbacks); }

    // does this actually happen?
    void afterShutdown(const Callback& callback) { addCallback(callback, afterShutdownCallbacks); }

private:
    static void addCallback(const Callback& callback, std::vector<Callback>& callbacks) {
        callbacks.push_back(callback);
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
    static void dispatch(std::vector<std::function<void()>>& callbacks) {
        for (auto& cb : callbacks) {
            cb();
        }
    }

    std::unique_ptr<sdbus::IObjectProxy> proxy;

    std::vector<Callback> sleepCallbacks;
    std::vector<Callback> wakeCallbacks;
    std::vector<Callback> shutdownCallbacks;
    std::vector<Callback> afterShutdownCallbacks;

    Inhibitor inhibitor;
};


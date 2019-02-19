#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <vector>


class DbusService {
public:
    DbusService(const char* serviceName, const char* objectPath, const char* interfaceName) : interfaceName(interfaceName) {
        connection = sdbus::createSessionBusConnection(serviceName);
        dbusObject = sdbus::createObject(*connection, objectPath);
    }

    template<class Func>
    void onMethod(const char* methodName, Func&& func) {
        auto& callbacks = methodCallbacks[methodName];
        callbacks.push_back(std::forward<Func>(func));

        if (callbacks.size() == 1) {
            dbusObject->registerMethod(methodName).onInterface(interfaceName).implementedAs([&](){
                dispatch(callbacks);
            });
        }
    }

    void finalize() {
        dbusObject->finishRegistration();
        connection->enterProcessingLoopAsync();
    }

private:


    void dispatch(std::vector<std::function<void()>>& callbacks) {
        for (auto& cb : callbacks) {
            cb();
        }
    }

    const char* interfaceName;

    std::map<const char*, std::vector<std::function<void()>>> methodCallbacks;

    std::unique_ptr<sdbus::IConnection> connection;
    std::unique_ptr<sdbus::IObject> dbusObject;


};


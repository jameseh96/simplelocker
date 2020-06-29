#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <vector>
#include <unordered_map>


class DbusService {
public:
    DbusService(const std::string& serviceName, const std::string& objectPath, const std::string& interfaceName)
            : interfaceName(interfaceName) {
        connection = sdbus::createSessionBusConnection(serviceName);
        dbusObject = sdbus::createObject(*connection, objectPath);
    }

    using Callback = std::function<void()>;

    void onMethod(const std::string& methodName, const Callback& func) {
        auto& callbacks = methodCallbacks[methodName];
        callbacks.push_back(func);

        // if this is the first time a callback has been added for this method,
        // register with dbus
        if (callbacks.size() == 1) {
            dbusObject->registerMethod(methodName)
                       .onInterface(interfaceName)
                       .implementedAs([&](){
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

    std::string interfaceName;

    std::unordered_map<std::string, std::vector<Callback>> methodCallbacks;

    std::unique_ptr<sdbus::IConnection> connection;
    std::unique_ptr<sdbus::IObject> dbusObject;


};


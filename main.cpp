#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/process.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include "Inhibitor.h"
#include "DbusSignalListener.h"
#include "DbusService.h"
#include "XIdleChecker.h"

namespace po = boost::program_options;
namespace bp = boost::process;


class ChildWrapper {
public:
    ChildWrapper() = default;
    explicit ChildWrapper(std::string cmd): cmd(std::move(cmd)) {
    }

    void start() {
        std::lock_guard lg(mutex);
        if (!child.running()) {
            child.wait();
            child = bp::child(cmd);
            child.detach();
        }
    }

private:
    std::string cmd;
    bp::child child;
    std::mutex mutex;
};

void callRemoteMethod(const std::string& name, const std::string& path, const std::string& interface,
                      const std::string& methodName) {
    auto connection = sdbus::createSessionBusConnection();
    auto proxy = sdbus::createObjectProxy(*connection, name, path);
    auto method = proxy->createMethodCall(interface, methodName);
    proxy->callMethod(method);
}


int main(int argc, char *argv[]) {
    bool onSleep;
    bool onShutdown;
    boost::optional<int> onIdle;
    std::string onDbus = "Lock";
    std::string onDbusDisable = "Disable";
    std::string onDbusEnable = "Enable";
    boost::optional<std::string> cmd;

    bool serveOnDbus;
    std::string dbusName = "io.github.jameseh96.simplelocker";
    std::string dbusPath = "/io/github/jameseh96/simplelocker";
    std::string dbusInterface = "io.github.jameseh96.Simplelocker";

    boost::optional<std::string> remoteCall;

    po::options_description allDesc("Allowed options");
    allDesc.add_options()
            ("sleep,s", po::bool_switch(&onSleep), "Run on sleep")
            ("shutdown,S", po::bool_switch(&onShutdown), "Run on shutdown")
            ("idle,i", po::value(&onIdle), "Run on idle")
            ("dbus,d", po::value(&onDbus), "Run on dbus method call")
            ("dbus-disable,b", po::value(&onDbusDisable), "Disable run-on-idle on this dbus method call")
            ("dbus-enable,e", po::value(&onDbusEnable), "Enable run-on-idle on this dbus method call")
            ("cmd,c", po::value(&cmd), "Command to run");

    po::options_description dbusDesc("DBus options");
    dbusDesc.add_options()
            ("listen,l", po::bool_switch(&serveOnDbus), "Register a dbus service to receive method calls for control")
            ("dbus-name,D", po::value(&dbusName), "Dbus service name to bind")
            ("dbus-path,P", po::value(&dbusPath), "Dbus object path to register")
            ("dbus-interface,I", po::value(&dbusInterface), "Dbus interface to register");

    po::options_description remoteDesc("Remote call options");
    remoteDesc.add_options()
            ("remote,r", po::value(&remoteCall), "Call dbus method (i.e., `-r Lock` to lock a running instance)");


    allDesc.add_options()
            ("help,h", "produce help message");
    allDesc.add(dbusDesc).add(remoteDesc);

    po::variables_map vm;
    try {
        auto parsed = po::command_line_parser(argc, argv).options(allDesc).run();
        po::store(parsed, vm);
        po::notify(vm);
    } catch(const po::error& e) {
        std::cout << e.what() << "\n" << allDesc << std::endl;
        return 1;
    }

    if (vm.count("help")) {
        std::cout << allDesc << std::endl;
        return 1;
    }

    if (remoteCall && (serveOnDbus || cmd) ) {
        std::cout << "--remote (-r) cannot be used in conjunction with --cmd (-c) or --listen (-l).\n" << allDesc
                  << std::endl;
        return 1;
    }

    if (!remoteCall && !cmd ) {
        std::cout << "Please specify either --remote (-r) or --cmd (-c).\n" << allDesc
                  << std::endl;
        return 1;
    }

    if (remoteCall) {
        callRemoteMethod(dbusName, dbusPath, dbusInterface, *remoteCall);
        return 1;
    }



    ChildWrapper child(*cmd);
    std::atomic<bool> idleIsEnabled = true;

    auto start = [&]{
        child.start();
    };

    DbusSignalListener signals;
    XIdleChecker xIdle;
    boost::optional<DbusService> service;


    if (onSleep) {
        signals.onSleep(start);
    }
    if (onShutdown) {
        signals.onShutdown(start);
    }
    if (onIdle) {
        xIdle.onIdle(std::chrono::seconds(*onIdle), [&] {
            if (idleIsEnabled) {
                start();
            }
        });
    }
    if (serveOnDbus) {
        service = DbusService(dbusName.c_str(), dbusPath.c_str(), dbusInterface.c_str());
        service->onMethod(onDbus.c_str(), start);
        service->onMethod(onDbusDisable.c_str(), [&] { idleIsEnabled = false; });
        service->onMethod(onDbusEnable.c_str(), [&] { idleIsEnabled = true; });
        service->finalize();
    }

    while (true) {
        try {
            xIdle.checkIdleTime();
            std::this_thread::sleep_for(1s);
        } catch (const std::exception& e) {
            throw e;
        }
    }

    return 0;
}

#pragma once

#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>
#include <system_error>
#include <memory>


class Inhibitor {
public:
    Inhibitor(std::unique_ptr<sdbus::IObjectProxy>& proxy, const char* what, const char* who, const char* why,
              const char* type) : what(what), who(who), why(why), type(type), fd(-1), proxy(proxy) {
        acquire();
    }

    void acquire() {
        if (fd == -1) {
            sdbus::FileHandle lockDescriptor;

            proxy->callMethod("Inhibit")
                    .onInterface("org.freedesktop.login1.Manager")
                    .withArguments(what, who, why, type)
                    .storeResultsTo(lockDescriptor);

            fd = lockDescriptor.get();
        }

    }
    void release() {
        if (fd != -1) {
            int r = close(fd);
            if (r < 0) {
                throw std::system_error(errno, std::generic_category(), "Failed to close inhibitor file descriptor");
            }
            fd = -1;
        }
    }
    ~Inhibitor() {
        release();
    }
private:
    const char* what;
    const char* who;
    const char* why;
    const char* type;
    int fd;
    std::unique_ptr<sdbus::IObjectProxy>& proxy;
};
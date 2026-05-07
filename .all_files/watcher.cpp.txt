#include <blip/platform/watcher.hpp>
#include <chrono>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace platform {
ConfigWatcher::ConfigWatcher() : running(false), fileDirty(false) {}
ConfigWatcher::~ConfigWatcher() { stop(); }

void ConfigWatcher::start(const std::string &path, OnChangeCallback callback) {
    if (running) {
        stop();
    }
    filepath = path;
    action = callback;
    running = true;
    fileDirty = false;

    worker = std::thread(&ConfigWatcher::loop, this);
}

void ConfigWatcher::stop() {
    running = false;
    if (worker.joinable()) {
        worker.join();
    }
}

void ConfigWatcher::check() {
    if (fileDirty) {
        fileDirty = false;
        if (action) {
            action();
        }
    }
}

void ConfigWatcher::loop() {
    int kq = kqueue();
    if (kq == -1) {
        return;
    }

    while (running) {
        int fd = open(filepath.c_str(), O_RDONLY);
        if (fd == -1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        struct kevent change;
        EV_SET(&change, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE | NOTE_RENAME, 0, NULL);
        kevent(kq, &change, 1, NULL, 0, NULL);

        struct kevent event;
        struct timespec timeout = {0, 500000000};

        int n = kevent(kq, NULL, 0, &event, 1, &timeout);
        if (n > 0) {
            if (event.fflags & (NOTE_WRITE | NOTE_RENAME)) {
                fileDirty = true;
                if (event.fflags & NOTE_RENAME) {
                    close(fd);
                    continue;
                }
            }
        }
        close(fd);
    }
    close(kq);
}
}

#include <blip/platform/watcher.hpp>
#include <errno.h>
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace platform {
ConfigWatcher::ConfigWatcher() : running(false), fileDirty(false) {}
ConfigWatcher::~ConfigWatcher() { stop(); }

void ConfigWatcher::start(const std::string &path, OnChangeCallback callback) {
    if (running) {
        stop();
    }
    if (worker.joinable()) {
        worker.join();
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
    if (fileDirty.exchange(false)) {
        if (action) {
            action();
        }
    }
}

void ConfigWatcher::loop() {
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        perror("inotify_init1");
        return;
    }

    int wd = inotify_add_watch(fd, filepath.c_str(), IN_MODIFY | IN_MOVE_SELF | IN_IGNORED);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        return;
    }

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

    while (running) {
        int poll_num = poll(fds, 1, 500);
        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (poll_num == 0) {
            if (wd == -1) {
                wd = inotify_add_watch(fd, filepath.c_str(), IN_MODIFY | IN_MOVE_SELF | IN_IGNORED);
            }
            continue;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t len = read(fd, buf, sizeof(buf));
            if (len <= 0)
                continue;

            for (char *ptr = buf; ptr < buf + len;) {
                struct inotify_event *event = (struct inotify_event *)ptr;

                if (event->mask & (IN_MODIFY | IN_MOVE_SELF)) {
                    fileDirty = true;
                }

                if (event->mask & IN_IGNORED) {
                    wd = -1;
                }

                ptr += sizeof(struct inotify_event) + event->len;
            }
        }
    }

    if (wd != -1) {
        inotify_rm_watch(fd, wd);
    }
    close(fd);
}
}

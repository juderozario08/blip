#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace platform {
class ConfigWatcher {
  public:
    using OnChangeCallback = std::function<void()>;
    ConfigWatcher();
    ~ConfigWatcher();
    void start(const std::string &path, OnChangeCallback callback);
    void stop();
    void check();

  private:
    void loop();
    std::string filepath;
    std::thread worker;
    std::atomic<bool> running;
    std::atomic<bool> fileDirty;
    OnChangeCallback action;
};
}

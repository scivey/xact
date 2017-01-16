#pragma once
#include <functional>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <stdexcept>

namespace xact_testing {

class ThreadGroup {
  using func_t = std::function<void(size_t)>;
 protected:
  size_t size_ {0};
  std::atomic<size_t> startCount_ {0};
  std::atomic<size_t> stopCount_ {0};
  func_t func_ {};
  std::vector<std::thread> threads_;
  std::atomic<bool> joining_ {false};
  inline ThreadGroup(size_t size, func_t func)
    : size_(size), func_(func) {}
  inline void start() {
    for (size_t i = 0; i < size_; i++) {
      threads_.emplace_back([this, i]() {
        size_t current = startCount_.fetch_add(1);
        while (current != startCount_.load(std::memory_order_acquire)) {
          current = startCount_.load(std::memory_order_acquire);
        }
        func_(i);
        stopCount_.fetch_add(1);
      });
    }
  }
 public:
  static inline std::shared_ptr<ThreadGroup> createShared(size_t n, func_t func) {
    std::shared_ptr<ThreadGroup> result {new ThreadGroup {n, func}};
    result->start();
    return result;
  }

  class AlreadyJoined: public std::runtime_error {
   public:
    inline AlreadyJoined(): std::runtime_error("Already joined!"){}
  };

  inline void join() {
    for (;;) {
      if (joining_.load(std::memory_order_acquire)) {
        throw AlreadyJoined();
      }
      bool expected = false, desired = true;
      if (joining_.compare_exchange_strong(expected, desired)) {
        break;
      }
    }
    while (stopCount_.load(std::memory_order_acquire) != size_) {
      ;
    }
    for (auto& t: threads_) {
      t.join();
    }
  }
  inline ~ThreadGroup() {
    if (!joining_.load(std::memory_order_relaxed)) {
      try {
        join();
      } catch (const AlreadyJoined& ex) {
        ;
      }
    }
  }
};

} // xact_testing

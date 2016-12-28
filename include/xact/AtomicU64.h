#pragma once
#include "xact/AlignedBox.h"
#include <cstdint>

namespace xact {

template<size_t>
class FixedAtomicU64Group;

class AtomicU64 {
 public:
  using value_type = uint64_t;
  using box_t = AlignedBox<value_type, 16>;

  template<size_t>
  friend class FixedAtomicU64Group;
 protected:
  box_t value_;
  value_type* getPointer() {
    return value_.ptr();
  }
 public:
  AtomicU64();
  AtomicU64(value_type value);
  void store(value_type value);
  AtomicU64& operator=(value_type);
  value_type load();
  value_type fetchAdd(value_type value);
  value_type fetchSub(value_type value);
  value_type compareExchange(value_type expected, value_type desired);
};


} // xact
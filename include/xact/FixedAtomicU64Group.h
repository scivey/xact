#pragma once
#include <array>
#include "xact/AtomicU64.h"
#include "xact/atomic_ops/multi.h"

namespace xact {

template<size_t N>
class FixedAtomicU64Group {
 public:
  static const size_t GroupSize = N;
  using atomic_t = AtomicU64;
  using atomic_array_t = std::array<atomic_t*, N>;
  using atomic_val_t = typename atomic_t::value_type;
  using atomic_val_ptr_array_t = std::array<atomic_val_t*, N>;
  using val_array_t = std::array<atomic_val_t, N>;
 protected:
  atomic_array_t atomicPtrs_;
  atomic_val_ptr_array_t atomicValPtrs_;
  void initPointers() {
    size_t idx = 0;
    for (auto ptr: atomicPtrs_) {
      if (ptr != nullptr) {
        atomicValPtrs_[idx] = ptr->getPointer();
      }
      idx++;
    }
  }
 public:
  FixedAtomicU64Group(const atomic_array_t& atoms): atomicPtrs_(atoms) {
    initPointers();
  }

  template<typename ...Types>
  FixedAtomicU64Group(Types&&... args): atomicPtrs_(std::forward<Types>(args)...){
    initPointers();
  }
  void clear() {
    memset(atomicPtrs_.data(), 0, sizeof(atomic_t*)*N);
  }
  static size_t capacity() {
    return GroupSize;
  }
  atomic_t*& at(size_t idx) {
    return atomicPtrs_.at(idx);
  }
  const atomic_t* at(size_t idx) const {
    return atomicPtrs_.at(idx);
  }
  atomic_t*& operator[](size_t idx) {
    return at(idx);
  }
  const atomic_t* operator[](size_t idx) const {
    return at(idx);
  }
  bool load(val_array_t& dest) {
    return atomic_ops::loadU64M(atomicValPtrs_, dest);
  }
  bool store(const val_array_t& input) {
    return atomic_ops::storeU64M(atomicValPtrs_, input);
  }
  bool compareExchange(const val_array_t& expected, const val_array_t& desired) {
    return atomic_ops::casU64M(atomicValPtrs_, expected, desired);
  }
  bool add(atomic_val_t addBy) {
    return atomic_ops::addU64M(atomicValPtrs_, addBy);
  }
  bool fetchAdd(val_array_t& dest, atomic_val_t addBy) {
    return atomic_ops::fetchAddU64M(atomicValPtrs_, addBy);
  }
  bool sub(atomic_val_t subBy) {
    return atomic_ops::subU64M(atomicValPtrs_, subBy);
  }
  bool fetchSub(val_array_t& dest, atomic_val_t subBy) {
    return atomic_ops::fetchSubU64M(atomicValPtrs_, dest, subBy);
  } 
};

} // xact

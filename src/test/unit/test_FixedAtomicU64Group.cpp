#include <gtest/gtest.h>
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <array>

#include "xact/atomic_ops/single.h"
#include "xact/atomic_ops/multi.h"
#include "xact/AlignedBox.h"
#include "xact/AtomicU64.h"
#include "xact/FixedAtomicU64Group.h"
#include "xact/fence.h"

using namespace xact::atomic_ops;
using namespace std;
using xact::mFence;
using Box64 = xact::AtomicU64::box_t;
using xact::FixedAtomicU64Group;
using xact::AtomicU64;

template<typename TArray>
void zeroArray(TArray& anArray) {
  for (auto& elem: anArray) {
    elem = 0;
  }
}

TEST(TestFixedAtomicU64Group, TestGeneralWorkiness) {
  static const size_t kNAtoms = 4;
  static const uint64_t kInitVal = 10;
  array<AtomicU64, kNAtoms> atoms;
  array<AtomicU64*, kNAtoms> atomPtrs;
  using val_array = array<uint64_t, kNAtoms>;
  val_array expected;
  for (size_t i = 0; i < kNAtoms; i++) {
    uint64_t val = kInitVal + i;
    atoms[i].store(val);
    expected[i] = val;
    atomPtrs[i] = atoms.data() + i;
  }
  FixedAtomicU64Group<4> atomGroup {atomPtrs};
  val_array values;
  zeroArray(values);
  for (;;) {
    if (atomGroup.load(values)) {
      break;
    }
  }
  for (size_t i = 0; i < kNAtoms; i++) {
    EXPECT_EQ(expected[i], values[i]);
    expected[i] += 10;
  }
  for (;;) {
    if (atomGroup.add(10)) {
      break;
    }
  }
  for (;;) {
    if (atomGroup.load(values)) {
      break;
    }
  }
  for (size_t i = 0; i < kNAtoms; i++) {
    EXPECT_EQ(expected[i], values[i]);
  }
  val_array toSet;
  for (size_t i = 0; i < kNAtoms; i++) {
    auto nextVal = (i+1) * 1000;
    expected[i] = nextVal;
    toSet[i] = nextVal;
  }
  for (;;) {
    if (atomGroup.store(toSet)) {
      break;
    }
  }
  for (;;) {
    if (atomGroup.load(values)) {
      break;
    }
  }
  for (size_t i = 0; i < kNAtoms; i++) {
    EXPECT_EQ(expected[i], values[i]);
  }
}

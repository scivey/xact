#include <glog/logging.h>
#include <thread>
#include <array>
#include <iostream>
#include <cassert>
#include <xact/LockableAtomicU64.h>
#include <bitset>
#include "xact/detail/asm/tests.h"

#include <vector>

using xact::LockableAtomicU64;
using namespace std;

array<uint64_t, 8> codeToParts(uint64_t code) {
  array<uint64_t, 8> result;
  uint8_t *uPtr = (uint8_t*) &code;
  for (size_t i = 0; i < 8; i++) {
    uint64_t current = *uPtr;
    result[i] = current;
    ++uPtr;
  }
  return result;
}

template<typename T>
void logAll(const T& ref) {
  std::cout << std::endl << "\t";
  for (const auto& part: ref) {
    std::cout << part << "\t";
  }
  std::cout << std::endl;
}

int main() {
  google::InstallFailureSignalHandler();
  auto code = xact_tsx_test_abort_explicit();
  auto parts = codeToParts(code);
  logAll(parts);
  bitset<64> bts = code;
  LOG(INFO) << bts;
  LOG(INFO) << "done";
}

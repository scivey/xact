#include "xact/AtomicU64.h"
#include "xact/atomic_ops/single.h"

namespace xact {

using value_type = AtomicU64::value_type;

AtomicU64::AtomicU64(value_type value) {
  store(value);
}

AtomicU64::AtomicU64() {
  store(0);
}

void AtomicU64::store(value_type value) {
  atomic_ops::storeU64S(value_.ptr(), value);
}

AtomicU64& AtomicU64::operator=(value_type value) {
  store(value);
  return *this;
}

value_type AtomicU64::fetchAdd(value_type value) {
  return atomic_ops::fetchAddU64S(value_.ptr(), value);
}

value_type AtomicU64::fetchSub(value_type value) {
  return atomic_ops::fetchSubU64S(value_.ptr(), value);
}

value_type AtomicU64::load() {
  return atomic_ops::loadU64S(value_.ptr());
}

value_type AtomicU64::compareExchange(value_type expected, value_type desired) {
  return atomic_ops::casU64S(value_.ptr(), expected, desired);
}

bool AtomicU64::fetchAddIfBetween(value_type *result, value_type addBy,
      value_type lowerBoundInclusive, value_type upperBoundExclusive) {
  return atomic_ops::fetchAddU64SIfBetween(
    value_.ptr(), result, addBy, lowerBoundInclusive, upperBoundExclusive
  );
}


AtomicU64Inspector::AtomicU64Inspector(AtomicU64& ref): ref_(ref){}

value_type*  AtomicU64Inspector::getPointer() {
  return ref_.getPointer();
}


} // xact
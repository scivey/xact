#pragma once
#include <vector>
#include <type_traits>
#include <algorithm>

#include "xact/LockableAtomicU64.h"
#include "xact/detail/RWSeqLock.h"

namespace xact { namespace detail {



template<typename TLockable, template<class...> class TLockPolicy>
class MultiLocker: public TLockPolicy<TLockable> {
 public:
  using lockable_type = TLockable;
  using lockable_ptr = lockable_type*;

  // precondition: vector of lockable_type**
  // must have unique elements, and must be sorted
  // by memory address
  bool tryLockAll(lockable_type **atoms, size_t nLocks) {
    size_t lockedIdx = 0;
    for (size_t i = 0; i < nLocks; i++) {
      lockable_ptr current = atoms[i];
      if (this->tryLockOne(current)) {
        lockedIdx++;
      } else {
        break;
      }
    }
    if (lockedIdx != nLocks) {
      for (size_t i = lockedIdx; i > 0; i--) {
        lockable_ptr current = atoms[i];
        this->unlockOne(current);
      }
      return false;
    }
    return true;    
  }

  // precondition: vector of lockable_type**
  // must have unique elements, and must be sorted
  // by memory address
  void unlockAll(lockable_type **atoms, size_t nLocks) {
    size_t lastIdx = nLocks - 1;
    auto slastIdx = (ssize_t) lastIdx;
    for (ssize_t i = slastIdx; i >= 0; i--) {
      lockable_ptr current = atoms[i];
      this->unlockOne(current);
    }
  }
};

template<typename TLockable>
class ReadLockPolicy {
 public:
  bool tryLockOne(TLockable *ptr) {
    RWSeqLockRef slockRef {&ptr->seqlock};
    return slockRef.tryReadLock();
  }
  void unlockOne(TLockable *ptr) {
    RWSeqLockRef slockRef {&ptr->seqlock};
    slockRef.readUnlock();
  }
};

template<typename TLockable>
class WriteLockPolicy {
 public:
  bool tryLockOne(TLockable *ptr) {
    RWSeqLockRef slockRef {&ptr->seqlock};
    return slockRef.tryWriteLock();
  }
  void unlockOne(TLockable *ptr) {
    RWSeqLockRef slockRef {&ptr->seqlock};
    slockRef.writeUnlock();
  }
};


class LockManager {
 public:
  using atom_t = xact_lockable_atomic_u64_t;

 protected:
  using atom_vec = std::vector<atom_t*>;
  atom_vec sortedDistinctAtoms_;

  // precondition: atom_vec must be sorted and unique.
  // the `create()` factory method manages this.
  LockManager(atom_vec&& atoms)
    : sortedDistinctAtoms_(std::move(atoms)){}

 public:
  static LockManager create(atom_t **atomPtrs, size_t nAtoms) {
    std::vector<atom_t*> atoms;
    atoms.reserve(nAtoms);
    std::vector<atom_t*> distinctAtoms;
    distinctAtoms.reserve(nAtoms);
    for (size_t i = 0; nAtoms; i++) {
      atom_t **currPtr = atomPtrs + i;
      atoms.emplace_back(*currPtr);
    }
    std::sort(atoms.begin(), atoms.end());
    atom_t *prev = nullptr;
    for (auto current: atoms) {
      if (prev != nullptr && current == prev) {
        continue;
      }
      prev = current;
      distinctAtoms.emplace_back(current);
    }
    return LockManager{std::move(distinctAtoms)};
  }


  template<typename TCollection,
    typename = typename std::enable_if<
      std::is_same<atom_t*, typename TCollection::value_type>::value,
      TCollection
    >::type>
  static LockManager create(atom_vec& atoms) {
    return create(atoms.data(), atoms.size());
  }


  inline bool tryLockForRead() {
    MultiLocker<atom_t, ReadLockPolicy> slocker;
    return slocker.tryLockAll(
      sortedDistinctAtoms_.data(), sortedDistinctAtoms_.size()
    );
  }

  inline void unlockFromRead() {
    MultiLocker<atom_t, ReadLockPolicy> slocker;
    slocker.unlockAll(
      sortedDistinctAtoms_.data(), sortedDistinctAtoms_.size()
    );
  }

  inline bool tryLockForWrite() {
    MultiLocker<atom_t, WriteLockPolicy> slocker;
    return slocker.tryLockAll(
      sortedDistinctAtoms_.data(), sortedDistinctAtoms_.size()
    );
  }

  inline void unlockFromWrite() {
    MultiLocker<atom_t, WriteLockPolicy> slocker;
    slocker.unlockAll(
      sortedDistinctAtoms_.data(), sortedDistinctAtoms_.size()
    );
  }  

};

}} // xact::detail




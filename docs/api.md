## api

The most interesting feature exposed by XACT right now is multi-CAS.

### Multi-CAS - the main interface
The main interface for multi-CAS revolves around two classes.

#### AtomicU64
See [include/xact/AtomicU64.h](/include/xact/AtomicU64.h).
This is very similar to an `std::atomic<uint64_t>`, and if you're familiar with c++11 or c11 atomics its interface should be self-explanatory.  The important methods are `fetchAdd`, `fetchSub`, `load`, `store`, and `compareExchange`.

#### FixedAtomicU64Group
See [include/xact/FixedAtomicU64Group.h](/include/xact/FixedAtomicU64Group.h)

This wraps around a fixed array of pointers to AtomicU64 instances.  Its API is analogous to AtomicU64's, with the following important differences:
* Wherever `AtomicU64` returns a `uint64_t`, the group method instead takes a reference to an std::array<uint64_t> where it should store the result.
* On `AtomicU64` instances, only the CAS operations are expected to fail.  But because the group versions are inherently transactional, even a group `load` or `store` can fail.  As with single-CAS, you need to wrap these operations in some kind of retry logic.


### Multi-CAS - the ugly interface
There is also a lower-level multi-CAS interface, which takes raw pointers to arrays.  It's a lot less friendly.  If the higher-level API gets in your way, you should look [here](/include/xact/atomic_ops/multi.h).


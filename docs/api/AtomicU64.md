## AtomicU64: a plain old atomic uint64_t

AtomicU64 is a basic atomic type, similar to an std::atomic<uint64_t>.

If you're familar with C++11 or C11 atomics, its interface should be self-explanatory -- see prototypes in the header here: [include/xact/AtomicU64.h](/include/xact/AtomicU64.h).

The important methods are `fetchAdd`, `fetchSub`, `load`, `store` and `compareExchange`.

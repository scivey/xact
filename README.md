# XACT

## TSX-based concurrency primitives for C++/linux/x64:
## Lock-free N-way atomic CAS
## Generalized, predicate-based N-way conditional store / CAS / FAA

XACT is a library of lock-free primitives based on brief hardware-level transactions.

It uses Intel's Thread Synchronization Extensions (TSX), which are intended to support transactional memory implementations, but XACT does not aim to provide true transactional memory.

Instead, it uses tightly scoped mini-transactions to power extended and generalized versions of the lock-free primitives programmers are already familiar with: compare-and-swap, fetch-and-add, and atomic load and store.  This enables low-level operations that would otherwise require locking, while avoiding the unfamiliar performance profile and other potential pitfalls of full-blown transactional memory.

XACT is a work in progress, but already offers features that don't otherwise appear to be available.


### Features
XACT currently has two main APIs.

#### The N-Way API
The first API is an N-way extension of existing x64 atomic operations.

Like all other mainstream architectures today, x64 only allows atomic compare-and-exchange (CAS) to a single memory location at a time.  Atomic load, store and fetch-add have similar limitations, especially when the addresses are on separate cache lines.

Through mini transactions, XACT's N-Way API provides analogus atomic operations on multiple separate memory locations.  This enables double compare-and-swap (DCAS), which is a prerequisite for a number of theoretical lock-free data structures.  It also enables N-way CAS in general: the exact bounds are currently unclear, but 8-way and 16-way CAS are both perfectly functional.

The N-way API similarly extends plain loads and stores: a reader can take a consistent snapshot of multiple memory addresses, and a writer can atomically store to multiple addresses.

See more detailed description [here](/docs/api/n_way.md), and an example [here](/examples/atomic_cas_reader_writer.cpp).

#### The Generalized CAS API
The N-Way API is a little bit like SIMD: it's doing more or less the same thing at multiple locations.  The Generalized CAS API is more flexible, and enables some operations that don't have analogues in among x86 atomic instructions.

This API enables atomic operations like the following logic: 
```
if (a == 10 && b == 20 && c > 30 && c < 50) {
  a = 100;
  d += 7;
  f -= 10;
  c += 1;
}
```
Basically: where CAS insists on a single equality predicate as its precondition and a single store as its effect, generalized CAS allows multiple different predicates and multiple different effects on success.

See a more thorough description [here](docs/api/generalized_cas.md).

### Implementation
XACT's underlying primitives are written in NASM assembly, but its main interface is in C++.
The C++ layer handles some logic on top of the assembly primitives, deals with alignment of the provided AtomicU64 type, and provides some level of type safety and human-friendliness.

If you're interested in the underlying details, the assembly parts are [documented decently well](https://github.com/scivey/xact/blob/master/src/xact_asm/atomic_u64_multi.asm#L258).


### Multi-CAS Example

See [here](/examples/atomic_cas_reader_writer.cpp).

### Generalized CAS Example

See [here](/examples/generalized_cas_single_thread.cpp).

### Benchmarks
See [here](docs/performance.md) for some recent, preliminary benchmarks.

### Building

#### Library Dependencies
* XACT itself has no external dependencies on other libraries.
* Unit tests depend on GTest (includes as a git submodule) and libglog
* Benchmarks rely on Google's Benchmark library (included as a submodule) and libglog

#### Compiler/System Requirements
Building XACT requires:
* CMake
* A C++-11-compatible compiler (tested on Clang and GCC)
* The Netwide Assembler (NASM)
* An Intel x86-64 processor with TSX extensions (currently, this will probably be a Skylake chip).
* Linux 

#### Build Instructions
```bash
make deps
mkdir build
cd build && cmake ../
make xact
make test # optional
make install
```

### Roadmap
Future plans:
* Optional spinlock-based fallbacks, both for non-TSX chips and for frequently failing transactions.

### License
MIT

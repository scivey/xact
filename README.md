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

XACT's first API extends these operations to multiple memory locations at a time.  This enables double compare-and-swap (DCAS), which is a prerequisite for a number of theoretical lock-free data structures.  It also allows for N-way CAS in general: the exact bounds are currently unclear, but 8-way and 16-way CAS are both perfectly functional.

The N-way API similarly extends plain loads and stores: a reader can take a consistent snapshot of multiple memory addresses, and a writer can atomically store to multiple addresses.


#### The Generalized CAS API
The N-Way API is a little bit like SIMD: it's doing more or less the same thing at multiple locations.  The Generalized CAS API is more flexible, and enables some operations that don't have analogues in among x86 atomic instructions.

This API is a generalization of compare-and-swap which allows for multiple predicate-based preconditions, as well as multiple effects when those preconditions are met.

To clarify, think of `cmpxchg` as a combination of one precondition and one effect:
```
[precondition] The value at address A is expected to equal X

[if preconditions satisfied....]
[effect]       Store Y at address A.
```

The Generalized CAS API allows for multiple preconditions and effects to be combined, so that the following can be performed as one atomic operation:
```
[precondition] The value at address A is expected to equal 10
[precondition] The value at address B is expected to be greater than 100
[precondition] The value at address B is expected to be less than 200
[precondition] The value at address C is expected not to equal 17
[precondition] The value at address D is expected to be greater than 1000
[precondition] The value at adresss E is greater than 0

[if preconditions satisfied...]
[effect]       Store the value 20 at address A
[effect]       Atomically increment the value at B by 5
[effect]       Store the value 17 at address C
[effect]       Atomically decrement the value at D by 1
```
This interface does have limitations: it doesn't allow for more general logic or branching, and it can't currently model dependencies between target memory locations well.  Despite this, it's still significantly more expressive than a single compare-and-swap.


### Implementation
XACT's underlying primitives are written in NASM assembly, but its main interface is in C++.
The C++ layer handles some logic on top of the assembly primitives, deals with alignment of the provided AtomicU64 type, and provides some level of type safety and human-friendliness.

If you're interested in the underlying details, the assembly parts are [documented decently well](https://github.com/scivey/xact/blob/master/src/xact_asm/atomic_u64_multi.asm#L258).

### API
See the example below, as well as some API notes [here](/docs/api.md).


### Multi-CAS Example
An ordinary CAS operation on X86-64 operates on a single memory location, like this:

```c++
#include <atomic>
#include <cassert>

int main() {
    std::atomic<uint64_t> atom {10};
    uint64_t expected = 10;
    uint64_t desired = 20;

    // if `atom` is currently equal to `expected`,
    // its value is replaced with `desired` and the method returns true.
    assert(atom.compare_exchange_strong(expected, desired));
    assert(atom.load() == 20);

    // if `atom` does not equal `expected`,
    // the CAS fails and `atom`'s value is unchanged.
    desired = 100;
    expected = 99;
    assert(!atom.compare_exchange_strong(expected, desired));
    assert(atom.load() == 20);
}
```

There's still no DCAS instruction.  But with TSX and XACT, you can now perform atomic loads / stores / CAS / fetch-adds on multiple memory locations simultaneously like this:

```c++
#include <array>
#include <xact/AtomicU64.h>
#include <xact/FixedAtomicU64Group.h>

using namespace std;
using AtomGroup = xact::FixedAtomicU64Group<4>;
static const size_t kAtoms = 4;
int main() {
  std::array<xact::AtomicU64, kAtoms> atoms {1, 2, 3, 4};
  AtomGroup group{{&atoms[0], &atoms[1], &atoms[2], &atoms[3]}};
  std::array<uint64_t, kAtoms> newValues {10, 20, 30, 40};
  std::array<uint64_t, kAtoms> expectedValues {1, 2, 3, 4};

  for (;;) {
    if (group.compareExchange(expectedValues, newValues)) {
      break;
    }
  }
}
```
(See the [extended, two-thread example here](/examples/atomic_cas_reader_writer.cpp).)

Above, XACT's `FixedAtomicU64Group` is an array type containing references to multiple `AtomicU64` instances.  You can think of it as a composite interface: where calling `compareExchange()` on a single AtomicU64 will either atomically replace its value or fail, calling `compareExchange` on a `FixedAtomicU64Group` will either atomically replace the values of all members or completely fail.

The API allows up to 64-way CAS, but I haven't pushed it too hard yet.  8- and 16- way CAS appear to be reliable.  The practical limit will depend on your particular hardware and workload, and on Intel's magic pixie dust.


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

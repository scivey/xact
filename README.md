# XACT

## Lock-free N-way atomic compare-and-swap:
## TSX-based concurrency primitives for C++/linux/x64

### Background
Intel's TSX instructions, included in some Haswell processors and more widely available in recent Skylake chips, implement a restricted form of hardware transactional memory.  TSX is powerful, but has some severe limitations: it gets cranky when you touch too much memory at once, and it likes to mysteriously abort transactions.  Intel hasn't released many details on the underlying algorithms.  You get the idea: it isn't really suitable for general purpose transactional memory.

Currently, TSX instructions are seeing some use in glibc and libraries like [ConcurrencyKit](http://concurrencykit.org/) to [optimize lock-based concurrency control](https://lwn.net/Articles/534758/).  While this approach enables some impressive speedups, it ends up presenting the same lock-based, blocking API to the programmer.

XACT takes a middle road: it doesn't use TSX for full-blown transactional memory, but also doesn't just use it to transparently elide locks.  Instead, it aims to provide a higher-level interface to a small set of non-lock-based primitives.

The basic idea is to use just enough TSX to get the job done.  XACT intentionally keeps hardware-level transactions very short, with as few memory accesses as necessary, to avoid the spurious failures and aborts that seem to have limited TSX's use so far.

One fundamental primitive in lock-free programming is compare-and-swap, as provided by the `cmpxchg` x86 instruction.  This is "single" CAS: it operates on a single memory location at a time, which places a number of constraints on its applications.  Double-CAS, or DCAS, comes up a lot in papers, but isn't meaningfully supported by any modern mainstream architecture.  This is unfortunate, as people have been dreaming up algorithms requiring DCAS for [a long time now](http://i.stanford.edu/pub/cstr/reports/cs/tr/99/1624/CS-TR-99-1624.pdf).

I'm also pretty tired of plain single CAS, so XACT's first main feature is a lock-free, multi-way, atomic compare-and-swap operation.  This allows for double-CAS as well as triple- and quadruple-CAS.  And also 8-way CAS.  As a bonus, N-way atomic stores, loads, and fetch-adds are also supported.


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
Future ideas:
* More general transactional operations, e.g. a write to two locations conditional on the values at four other locations.
* A generalized predicate-based compare and swap. (testing for not just equality but `<=`, `>`, etc.)
* Spinlock-based fallbacks, both for non-TSX chips and for frequently failing transactions.

### License
MIT

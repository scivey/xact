## The N-Way API

The N-Way API extends the familiar x64 atomic operations to operate atomically on N separate memory locations.

### The API

The N-Way API is a layer over [AtomicU64](AtomicU64.md), which is currently the only integral atomic type provided by XACT.  This API mainly consists of the `FixedAtomicU64Group` class.

#### `FixedAtomicU64Group`: the nice interface
See method prototypes at [include/xact/FixedAtomicU64Group.h](/include/xact/FixedAtomicU64Group.h)

FixedAtomicU64Group is a wrapper around a fixed array of pointers to AtomicU64 instances.  It's also a horrible name, but I'm starting to feel like that ship has sailed.

This is basically a composite interface: it has very similar methods to AtomicU64, but applies its actions atomically over multiple instances.

The group-based interface has the following important differences from `AtomicU64`'s interface:
* Wherever `AtomicU64` returns a `uint64_t`, the group method instead takes a reference to an std::array<uint64_t> where it should store the result.
* On `AtomicU64` instances, only the CAS operations are expected to fail.  Because the group versions are inherently transactional, even a group `load` or `store` can fail.  You need to wrap these operations in some kind of retry logic.


#### Multi-CAS - the ugly interface
There is also a lower-level multi-CAS interface, which takes raw pointers to arrays.  It's a lot less friendly, but considerably more flexible.  If the higher-level API gets in your way, you should look [here](/include/xact/atomic_ops/multi.h).


### Examples
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
Above, where calling `compareExchange()` on a single AtomicU64 will either atomically replace its value or fail, calling `compareExchange` on a `FixedAtomicU64Group` will either atomically replace the values of all members or completely fail.

The API allows up to 64-way CAS, but I haven't pushed it too hard yet.  8- and 16- way CAS appear to be reliable.  The practical limit will depend on your particular hardware and workload, and on Intel's magic pixie dust.

### Background on TSX

Intel's TSX instructions, included in some Haswell processors and more widely available in recent Skylake chips, implement a restricted form of hardware transactional memory.  TSX is powerful, but has some severe limitations: it gets cranky when you touch too much memory at once, and it likes to mysteriously abort transactions.  Intel hasn't released many details on the underlying algorithms.  You get the idea: it isn't really suitable for general purpose transactional memory.

Currently, TSX instructions are seeing some use in glibc and libraries like [ConcurrencyKit](http://concurrencykit.org/) to [optimize lock-based concurrency control](https://lwn.net/Articles/534758/).  While this approach enables some impressive speedups, it ends up presenting the same lock-based, blocking API to the programmer.

XACT takes a middle road: it doesn't use TSX for full-blown transactional memory, but also doesn't just use it to transparently elide locks.  Instead, it aims to provide a higher-level interface to a small set of non-lock-based primitives.

The basic idea is to use just enough TSX to get the job done.  XACT intentionally keeps hardware-level transactions very short, with as few memory accesses as necessary, to avoid the spurious failures and aborts that seem to have limited TSX's use so far.

One fundamental primitive in lock-free programming is compare-and-swap, as provided by the `cmpxchg` x86 instruction.  This is "single" CAS: it operates on a single memory location at a time, which places a number of constraints on its applications.  Double-CAS, or DCAS, comes up a lot in papers, but isn't meaningfully supported by any modern mainstream architecture.  This is unfortunate, as people have been dreaming up algorithms requiring DCAS for [a long time now](http://i.stanford.edu/pub/cstr/reports/cs/tr/99/1624/CS-TR-99-1624.pdf).

I'm also pretty tired of plain single CAS, so XACT's first main feature is a lock-free, multi-way, atomic compare-and-swap operation.  This allows for double-CAS as well as triple- and quadruple-CAS.  And also 8-way CAS.  As a bonus, N-way atomic stores, loads, and fetch-adds are also supported.


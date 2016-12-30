## The N-Way API

### Overview
Like all other mainstream architectures today, x64's only natively allows atomic compare-and-exchange (CAS) to a single memory location at a time.  Atomic load, store and fetch-add have similar limitations, especially when the addresses are on separate cache lines.

Through mini transactions, XACT's N-Way API provides analogus atomic operations on multiple separate memory locations.  This enables double compare-and-swap (DCAS), which is a prerequisite for a number of theoretical lock-free data structures.  It also enables N-way CAS in general: the exact bounds are currently unclear, but 8-way and 16-way CAS are both perfectly functional.

The N-way interface similarly extends plain loads and stores: readers can take consistent snapshots of multiple memory addresses, and writers can atomically store to multiple addresses.

### The API

The N-Way API is a layer over [AtomicU64](AtomicU64.md), which is currently the only integral atomic type exposed by XACT.


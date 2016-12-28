## performance

Avoiding locks has some advantages aside from raw performance, but performance is still important.  Below is a relatively simplistic benchmark, current as of revision d7efa508988f738bf02c81443e537e4bf43649b9.

### basic setup
This benchmark measures total nanoseconds for different numbers of reader/writer threads to complete a fixed number of atomic operations on a target `uint64_t*` array.  One "read operation" consists of taking an atomic snapshot of the pointer array, while one "write" consists of atomically making an increment to each element of the array.

The comparison is against `pthread_spinlock` and `std::mutex`.  In these cases an "atomic operation" means acquiring the lock, doing something to each element, and then releasing the lock.

Since single CAS can't accomplish the same consistency goal, it's not included in this benchmark.

This setup doesn't model most realistic use patterns -- I'm looking for a way to make more meaningful benchmarks.  In particular, it probably has much higher contention than many common use cases.

### compiler
The benchmark and library were compiled with clang++-3.9 on `-O2`.  Increasing compiler optimization seems to benefit the spinlock and mutex approaches more than the transactional one.  Since much of the core there is assembly, this probably isn't too surprising.


### overall conclusions
As with spinlocks and mutexes, some workloads seem to favor the transactional approach and some do not.
* In tests with a small number of memory locations and few writers, the transactional tests are often faster.
* With a large number of memory locations and/or writers, transactions seem to lose their edge.  In particular, the 8 addresses / 4 readers / 4 writers result is significantly worse than either the spinlock or mutex measurements. (This is analogous to the spinlock's poor performance in the 8 addresses / 16 readers / 1 writer result.)



### data

Below, `atoms` is the number of distinct memory locations being written to.  `readers` and `writers` are the number of reader and writer threads, respectively.  `incrPerThread` is the number of increment operations per writer thread, and `readersPerThread` is the number of snapshot operations per reader thread.

Time measurements are in nanoseconds, and were averaged over 20 trials for each set of test parameters.

```
    atoms=4     readers=4   writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               16265997
    mutex elapsed:                                  14172911
    transactional elapsed:                           9243515

    atoms=4     readers=4   writers=2   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               30979942
    mutex elapsed:                                  32596662
    transactional elapsed:                          26432399

    atoms=2     readers=4   writers=8   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               60688372
    mutex elapsed:                                  96967128
    transactional elapsed:                          94222949

    atoms=2     readers=4   writers=4   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               32042102
    mutex elapsed:                                  53772098
    transactional elapsed:                          31995567

    atoms=2     readers=16  writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                              110370921
    mutex elapsed:                                  12521265
    transactional elapsed:                          10993651

    atoms=1     readers=4   writers=4   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               41000950
    mutex elapsed:                                  39412621
    transactional elapsed:                          25240974

    atoms=8     readers=4   writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               25979691
    mutex elapsed:                                  19079545
    transactional elapsed:                          16337929

    atoms=8     readers=4   writers=2   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               33361011
    mutex elapsed:                                  35707374
    transactional elapsed:                          54482677

    atoms=8     readers=4   writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               27359380
    mutex elapsed:                                  18134342
    transactional elapsed:                          16681329

    atoms=8     readers=4   writers=2   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               35298658
    mutex elapsed:                                  37061500
    transactional elapsed:                          53372056

    atoms=8     readers=16  writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                              203222830
    mutex elapsed:                                  17487120
    transactional elapsed:                          25309772

    atoms=8     readers=8   writers=1   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               65059059
    mutex elapsed:                                  10369128
    transactional elapsed:                          12309725

    atoms=8     readers=4   writers=4   incrPerThread=100000    readsPerThread=100000
    spinlock elapsed:                               53880429
    mutex elapsed:                                  62160389
    transactional elapsed:                         469054332
```
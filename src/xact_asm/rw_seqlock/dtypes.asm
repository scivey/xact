%define RW_SEQLOCK_BITS 64
%define RW_SEQLOCK_VERSION_BITS 62
%define RW_SEQLOCK_LOCK_BITS 64 - RW_SEQLOCK_VERSION_BITS
%define RW_SEQLOCK_WRITE_LOCK_BIT 63
%define RW_SEQLOCK_READ_LOCK_BIT 62

%define RW_SEQLOCK_TRYLOCK__SUCCESS 1
%define RW_SEQLOCK_TRYLOCK__FAILURE 0

%define RW_SEQLOCK_IS_LOCKED__FALSE 0
%define RW_SEQLOCK_IS_LOCKED__TRUE 1

%define RW_SEQLOCK_VERSION_MASK       0x3fffffffffffffff
%define RW_SEQLOCK_WRITE_LOCK_MASK    0x8000000000000000
%define RW_SEQLOCK_READ_LOCK_MASK     0x4000000000000000
%define RW_SEQLOCK_LOCK_MASK          0xc000000000000000



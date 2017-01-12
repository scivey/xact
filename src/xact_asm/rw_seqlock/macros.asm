%include "rw_seqlock/dtypes.asm"

%macro rw_seqlock_make_version_mask 1
    mov %1, qword RW_SEQLOCK_VERSION_MASK
%endmacro

%macro rw_seqlock_make_lock_mask 1
    mov %1, qword RW_SEQLOCK_LOCK_MASK
%endmacro

%macro rw_seqlock_make_write_lock_mask 1
    mov %1, qword RW_SEQLOCK_WRITE_LOCK_MASK
%endmacro

%macro rw_seqlock_make_read_lock_mask 1
    mov %1, qword RW_SEQLOCK_READ_LOCK_MASK
%endmacro

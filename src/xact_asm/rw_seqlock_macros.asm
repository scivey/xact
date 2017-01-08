%define VERSION_BIT_COUNT 62
%define LOCK_BIT_COUNT 64 - VERSION_BIT_COUNT
%define LOCK_SUCCESS_RC 1

%macro jump_if_locked 2
    bt qword [%1], 0
    jc %2
    bt qword [%1], 1
    jc %2
%endmacro 

%macro make_version_mask 1
    xor %1, %1
    not %1
    shr %1, LOCK_BIT_COUNT
%endmacro

%macro make_lock_mask 1
    xor %1, %1
    not %1
    shl %1, VERSION_BIT_COUNT
%endmacro

%macro make_write_lock_mask 1
    xor %1, %1
    not %1
    shl %1, 63
%endmacro

%macro make_read_lock_mask 1
    make_write_lock_mask %1
    shr %1, 1
%endmacro

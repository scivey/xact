bits 64

%include "rw_seqlock/macros.asm"
%include "rw_seqlock/dtypes.asm"

section .text

%macro jump_if_locked 2
    bt qword [%1], 0
    jc %2
    bt qword [%1], 1
    jc %2
%endmacro 


global xact_rw_seqlock_init
    ; (xact_rw_seqlock_t*) -> void
xact_rw_seqlock_init:
    mov qword [rdi], 0
    mfence
    xor rax, rax
    ret


global xact_rw_seqlock_try_read_lock
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_try_read_lock:
    .begin:
        push r11
        push r12
        push r13
        mov rcx, 10

    .try_loop:
        mov rax, qword [rdi]
        mfence
        rw_seqlock_make_lock_mask r13
        mov r8, rax
        and r8, r13
        cmp r8, 0
        jnz .already_locked

        ; version + mask
        mov r9, rax
        rw_seqlock_make_version_mask r8
        and r9, r8

        xor r12, r12
        or r12, r9
        rw_seqlock_make_read_lock_mask r11
        or r12, r11

        lock cmpxchg qword [rdi], r12
        jz .got_lock
        jmp .already_locked

    .already_locked:
        cmp rcx, 0
        je .failure
        sub rcx, 1
        pause
        jump_if_locked rdi, .already_locked
        jmp .try_loop

    .failure:
        mov rax, RW_SEQLOCK_TRYLOCK__FAILURE
        jmp .end

    .got_lock:
        mov rax, RW_SEQLOCK_TRYLOCK__SUCCESS
        jmp .end

    .end:
        pop r13
        pop r12
        pop r11
        ret

global xact_rw_seqlock_read_lock
    ; (xact_rw_seqlock_t*) -> void
xact_rw_seqlock_read_lock:
    .begin:
        push r12
        mov r12, rdi

    .retry_loop:
        push r12
        mov rdi, r12
        call xact_rw_seqlock_try_read_lock
        pop r12
        cmp rax, RW_SEQLOCK_TRYLOCK__SUCCESS
        je .end
        pause
        jmp .retry_loop

    .end:
        xor rax, rax
        pop r12
        ret


global xact_rw_seqlock_read_unlock
    ; (xact_rw_seqlock_t*) -> void
xact_rw_seqlock_read_unlock:
    .begin:
        mov rax, qword [rdi]
        mfence

    .incr_version:
        mov r9, rax
        rw_seqlock_make_version_mask r8
        and r9, r8
        mov qword [rdi], r9
        mfence
    .end:
        xor rax, rax
        ret





global xact_rw_seqlock_util_make_write_locked_update_from_uint64_t
xact_rw_seqlock_util_make_write_locked_update_from_uint64_t:
    .begin:
        mov rax, qword [rdi]
        rw_seqlock_make_version_mask r8
        rw_seqlock_make_write_lock_mask r9
        and rax, r8
        or rax, r9
        ret



global xact_rw_seqlock_try_write_lock
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_try_write_lock:
    .begin:
        push r11
        push r12
        push r13
        mov rcx, 10

    .try_loop:
        mov rax, qword [rdi]


        mov r8, rax
        rw_seqlock_make_lock_mask r13
        and r8, r13
        cmp r8, 0
        jnz .already_locked

        ; version + mask
        mov r9, rax
        rw_seqlock_make_version_mask r8
        and r9, r8
        add r9, 1

        xor r12, r12
        or r12, r9
        rw_seqlock_make_write_lock_mask r11
        or r12, r11

        lock cmpxchg qword [rdi], r12
        jz .got_lock
        jmp .already_locked

    .already_locked:
        cmp rcx, 0
        je .failure
        sub rcx, 1
        pause
        bt qword [rdi], 0
        jc .already_locked
        jmp .try_loop

    .failure:
        mov rax, RW_SEQLOCK_TRYLOCK__FAILURE
        jmp .end

    .got_lock:
        mov rax, RW_SEQLOCK_TRYLOCK__SUCCESS
        jmp .end

    .end:
        pop r13
        pop r12
        pop r11
        ret


global xact_rw_seqlock_write_lock
    ; (xact_rw_seqlock_t*) -> void
xact_rw_seqlock_write_lock:
    .begin:
        push r12
        mov r12, rdi

    .retry_loop:
        push r12
        mov rdi, r12
        call xact_rw_seqlock_try_write_lock
        pop r12
        cmp rax, RW_SEQLOCK_TRYLOCK__SUCCESS
        je .end
        pause
        jmp .retry_loop

    .end:
        xor rax, rax
        pop r12
        ret


global xact_rw_seqlock_write_unlock
    ; (xact_rw_seqlock_t*) -> void
xact_rw_seqlock_write_unlock:
    .begin:
        mov rax, qword [rdi]
        mfence

    .incr_version:
        rw_seqlock_make_version_mask r8
        mov r9, rax
        and r9, r8
        add r9, 1
        mov qword [rdi], r9
        mfence
    .end:
        xor rax, rax
        ret



global xact_rw_seqlock_is_locked_from_uint64_t
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_locked_from_uint64_t:
    .check:
        rw_seqlock_make_lock_mask r8
        and rdi, r8
        cmp rdi, 0
        jz .not_locked
        jmp .locked

    .locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__TRUE
        jmp .end

    .not_locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__FALSE
        jmp .end

    .end:
        ret


global xact_rw_seqlock_is_write_locked_from_uint64_t
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_write_locked_from_uint64_t:
    .check:
        rw_seqlock_make_write_lock_mask r8
        and rdi, r8
        cmp rdi, 0
        jz .not_locked
        jmp .locked

    .locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__TRUE
        jmp .end

    .not_locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__FALSE
        jmp .end

    .end:
        ret

global xact_rw_seqlock_is_read_locked_from_uint64_t
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_read_locked_from_uint64_t:
    .check:
        rw_seqlock_make_read_lock_mask r8
        and rdi, r8
        cmp rdi, 0
        jz .not_locked
        jmp .locked

    .locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__TRUE
        jmp .end

    .not_locked:
        mov rax, RW_SEQLOCK_IS_LOCKED__FALSE
        jmp .end

    .end:
        ret


global xact_rw_seqlock_is_write_locked
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_write_locked:
        mov rax, qword [rdi]
        mov rdi, rax
        call xact_rw_seqlock_is_write_locked_from_uint64_t
        ret

global xact_rw_seqlock_is_read_locked
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_read_locked:
        mov rax, qword [rdi]
        mov rdi, rax
        call xact_rw_seqlock_is_read_locked_from_uint64_t
        ret

global xact_rw_seqlock_is_locked
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_is_locked:
        mov rax, qword [rdi]
        mov rdi, rax
        call xact_rw_seqlock_is_locked_from_uint64_t


global xact_rw_seqlock_get_version_from_uint64_t
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_get_version_from_uint64_t:
        mov rax, rdi
        rw_seqlock_make_version_mask rdx
        and rax, rdx
        ret


global xact_rw_seqlock_get_version
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_get_version:
        mov rax, rdi
        mov rdi, qword [rax]
        call xact_rw_seqlock_get_version_from_uint64_t
        ret


global xact_rw_seqlock_get_raw_value
    ; (xact_rw_seqlock_t*) -> bool
xact_rw_seqlock_get_raw_value:
        mov rax, qword [rdi]
        mfence
        ret

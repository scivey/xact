bits 64

section .text

global xact_lockable_atomic_u64_load
xact_lockable_atomic_u64_load:
        push r11
        push r12
        push r13
        push r14

        mov r11, rdi ; target
        mov r12, rsi ; result

        mfence        
        jmp .x_begin

    .on_failure:
        cmp rax, 0
        je .on_zero_failure
        jmp .end

    .on_zero_failure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, 0x00b33f00

    .x_begin:
        xbegin .on_failure
        mov r13, qword [r11+8]
        bt r13, 0
        jnc .x_continue_1
        xabort 1 ; resource_locked

    .x_continue_1:
        mfence
        mov r13, qword [r11]
        mov qword [r12], r13
        mov rax, 0

    .x_end:
        xend
        mfence

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret




global xact_lockable_atomic_u64_store
xact_lockable_atomic_u64_store:
        push r11
        push r12
        push r13
        push r14

        mov r11, rdi ; target
        mov r12, rsi ; value
        mfence        
        jmp .x_begin

    .on_failure:
        cmp rax, 0
        je .on_zero_failure
        jmp .end

    .on_zero_failure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, 0x00b33f00

    .x_begin:
        xbegin .on_failure
        mov r13, [rdi+8]
        bt r13, 0
        jnc .x_continue_1
        xabort 1 ; resource_locked

    .x_continue_1:
        mov qword [r11], r12
        mov rax, 0
        mfence

    .x_end:
        xend
        mfence        

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret


global xact_lockable_atomic_u64_compare_exchange
xact_lockable_atomic_u64_compare_exchange:
        push r11
        push r12
        push r13
        push r14

        mov r11, rdi ; target*
        mov r12, rsi ; expected*
        mov r13, rdx ; desired
        xor rax, rax
        mfence        
        jmp .x_begin

    .on_failure:
        cmp rax, 0
        je .on_zero_failure
        jmp .end

    .on_zero_failure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, 0x00b33f00

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort 1 ; resource_locked

    .x_continue_1:
        mov r14, qword [r11]
        mov r8, qword [r12]
        mfence
        cmp r14, r8   ; compare *target with *expected
        je .x_continue_2
        xabort 2 ; precondition failed

    .x_continue_2:
        mfence
        mov rax, r8
        lock cmpxchg qword [r11], r13
        jz .x_continue_3
        xabort 2

    .x_continue_3:
        ; mov qword [r11], r13 ; load desired into *target
        mfence
        xor rax, rax

    .x_end:
        xend
        xor rax, rax
        mfence        

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret



global xact_lockable_atomic_u64_fetch_add
xact_lockable_atomic_u64_fetch_add:
        push r11
        push r12
        push r13
        push r14

        mov r11, rdi ; target*
        mov r12, rsi ; result*
        mov r13, rdx ; add_by
        jmp .x_begin

    .on_failure:
        cmp rax, 0
        je .on_zero_failure
        jmp .end

    .on_zero_failure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, 0x00b33f00

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort 1 ; resource_locked

    .x_continue_1:
        mov r14, [r11]
        add [r11], r13
        mov [r12], r14
        mov rax, 0

    .x_end:
        xend

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret



global xact_lockable_atomic_u64_init
xact_lockable_atomic_u64_init:
        mov qword [rdi], rsi ; store value
        mov qword [rdi+8], 0 ; set tag to 0
        mfence
        xor rax, rax
        ret


global xact_lockable_atomic_u64_incr
xact_lockable_atomic_u64_incr:
        push r11
        push r12
        mov r11, rdi ; target*
        jmp .x_begin

    .on_failure:
        cmp rax, 0
        je .on_zero_failure
        jmp .end

    .on_zero_failure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, 0x00b33f00

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort 1 ; resource_locked

    .x_continue_1:
        inc qword [r11]

    .x_end:
        xend

    .end:
        pop r12
        pop r11
        ret


global xact_lockable_atomic_u64_trylock
xact_lockable_atomic_u64_trylock:
    .try_lock:
        lock bts qword [rdi+8], 0
        jnc .success

    .failure:
        mov rax, 0
        jmp .end

    .success:
        mov rax, 1

    .end:
        ret


global xact_lockable_atomic_u64_unlock
xact_lockable_atomic_u64_unlock:
    .unlock:
        lock btr qword [rdi+8], 0

    .end:
        xor rax, rax
        ret


global xact_lockable_atomic_u64_lock
xact_lockable_atomic_u64_lock:
    .loop_retry:
        lock bts qword [rdi+8], 0
        jnc .lock_acquired

    .on_failure:
        pause
        bt qword [rdi+8], 0
        jc .on_failure
        jmp .loop_retry

    .lock_acquired:
        xor rax, rax

    .end:
        ret


global xact_lockable_atomic_u64_is_locked
xact_lockable_atomic_u64_is_locked:
    .check_lock:
        bt qword [rdi+8], 0
        jc .is_locked

    .not_locked:
        mov rax, 0
        jmp .end

    .is_locked:
        mov rax, 1
        jmp .end

    .end:
        ret


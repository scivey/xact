bits 64

section .text

%include "common_tsx_defs.asm"
%include "rw_seqlock_dtypes.asm"
%include "rw_seqlock_macros.asm"


%macro _lu64_push_common_registers 0
        push r11
        push r12
        push r13
        push r14
        push r15
%endmacro

%macro _lu64_pop_common_registers 0
        pop r15
        pop r14    
        pop r13
        pop r12
        pop r11
%endmacro



global xact_lockable_atomic_u64_init
xact_lockable_atomic_u64_init:
        mov qword [rdi], rsi ; store initial value
        mov qword [rdi+8], 0
        mfence
        xor rax, rax
        ret






global xact_lockable_atomic_u64_load_mvcc
xact_lockable_atomic_u64_load_mvcc:
    .begin:
        _lu64_push_common_registers

        mov r8, rdi ; target
        mov r9, rsi ; result
        rw_seqlock_make_write_lock_mask rdx
        rw_seqlock_make_version_mask rcx

    .loop_step:
        mov r10, qword [rdi+8]

        ; is write lock bit set?
        mov r11, r10
        and r11, rdx
        cmp r11, 0
        jnz .resource_locked

        mov r11, r10
        and r11, rcx
        ; r11 <- version1

        mov r12, qword [rdi]
        ; r12 <- target->value

        mov r13, qword [rdi+8]
        ; r13 <- target->slock->rawValue()

        ; is write lock bit set?
        mov r14, r13
        and r14, rdx
        cmp r14, 0
        jnz .resource_locked

        mov r14, r13
        and r14, rcx
        ; r14 <- version2

        ; if version1 != version2, we have a bad read:
        ; we can't tell if a writer had the write lock bit set,
        ; and was in the middle of some kind of change, at the
        ; moment we grabbed [rdi].
        ; to avoid a race condition, we have to retry
        cmp r11, r14    ; version1 == version2 ?
        jne .loop_step

        mov qword [r9], r12 ; *result = target->value
        xor rax, rax
        jmp .end

    .resource_locked:
        mov rax, XSTATUS_RESOURCE_LOCKED
        jmp .end

    .end:
        _lu64_pop_common_registers
        ret





global xact_lockable_atomic_u64_store_cmpxchg16b
;   (xact_lockable_atomic_u64_t*, uint64_t new_val) -> uint64_t
xact_lockable_atomic_u64_store_cmpxchg16b:
    .begin:
        _lu64_push_common_registers
        mov r11, rdi     ; target (xact_lockable_atomic_u64_t*)
        mov r12, rsi     ; new_val (uint64_t)

    .cas_loop:
        mov r13, [rdi]   ;  r13 = target->value 
        mov r14, [rdi+8] ;  r14 = target->slock->value

        mov r8, r14
        rw_seqlock_make_version_mask r9
        and r8, r9
        mov r10, r8 ; r10 <- old version
        add r8, 1   ; r8  <- new version
        ; r8 now contains new version and 00 lock bits

        ; [rdx:rax] = expected = [old_version[62]:00[2]:expected_val[64]]
        mov rdx, r10
        mov rax, r13

        ; [rcx:rbx] = desired = [new_version[62]:00[2]:desired_val[64]]
        mov rcx, r8
        mov rbx, r12

        lock cmpxchg16b [r11]
        jnz .cas_failure
        xor rax, rax
        jmp .end

    .cas_failure:
        ; on failure, [rax:rdx] contain actual values of *target

        ; if target->slock->isLocked()
        mov r10, rdx
        rw_seqlock_make_lock_mask r9
        and r10, r9
        cmp r10, 0
        jnz .resource_locked

        ; remaining case: version number changed on us, but it's not locked
        jmp .cas_loop

    .resource_locked:
        mov rax, XSTATUS_RESOURCE_LOCKED
        jmp .end

    .end:
        _lu64_pop_common_registers
        ret




global xact_lockable_atomic_u64_compare_exchange_cmpxchg16b
;   (xact_lockable_atomic_u64_t*, uint64_t *expected, uint64_t desired) -> uint64_t
xact_lockable_atomic_u64_compare_exchange_cmpxchg16b:
    .begin:
        _lu64_push_common_registers
        mov r11, rdi     ; target (xact_lockable_atomic_u64_t*)
        mov r12, rsi     ; expected_val (uint64_t*)
        mov r13, rdx     ; desired_val (uint64_t)
        mov r14, [rdi]   ;  *target

    .cas_loop:
        mov r15, [rdi+8] ;  target->slock
        mov r8, r15
        rw_seqlock_make_version_mask r9
        and r8, r9
        mov r10, r8 ; r10 <- old version
        add r8, 1   ; r8  <- new version
        ; r8 now contains new version and 00 lock bits

        ; [rdx:rax] = expected = [old_version[62]:00[2]:expected_val[64]]
        mov rdx, r10
        mov rax, [r12]

        ; [rcx:rbx] = desired = [new_version[62]:00[2]:desired_val[64]]
        mov rcx, r8
        mov rbx, r13

        lock cmpxchg16b [r11]
        jnz .cas_failure
        xor rax, rax
        jmp .end

    .cas_failure:
        ; on failure, [rax:rdx] contain actual values of *target
        mov r10, rax ; target->value
        mov r9, [r12] ; <- r9 = *expected

        ; if target->value != *expected
        cmp r10, r9
        jne .precondition_failed


        ; if target->slock->isLocked()
        mov r10, rdx
        rw_seqlock_make_lock_mask r9
        and r10, r9
        cmp r10, 0
        jnz .resource_locked

        ; remaining case: version number changed on us, but it's not locked
        jmp .cas_loop

    .resource_locked:
        mov rax, XSTATUS_RESOURCE_LOCKED
        jmp .end

    .precondition_failed:
        mov qword [r12], rax ; *expected = target->value 
        mov rax, XSTATUS_PRECONDITION_FAILED
        jmp .end

    .end:
        _lu64_pop_common_registers
        ret






global xact_lockable_atomic_u64_fetch_add_cmpxchg16b
;   (xact_lockable_atomic_u64_t*, uint64_t *result, uint64_t incrBy) -> uint64_t
xact_lockable_atomic_u64_fetch_add_cmpxchg16b:
    .begin:
        _lu64_push_common_registers
        mov r11, rdi     ; target (xact_lockable_atomic_u64_t*)
        mov r12, rsi     ; result (uint64_t*)
        mov r13, rdx     ; incrBy (uint64_t)

    .cas_loop:
        mov rsi, [rdi]   ;  rsi = prevVal = *target
        mov r15, [rdi+8] ;  target->slock
        mov r8, r15
        rw_seqlock_make_version_mask r9
        and r8, r9
        mov r10, r8 ; r10 <- old version
        add r8, 1   ; r8  <- new version
        ; r8 now contains new version and 00 lock bits

        ; [rdx:rax] = expected = [old_version[62]:00[2]:expected_val[64]]
        mov rdx, r10
        mov rax, rsi

        ; [rcx:rbx] = desired = [new_version[62]:00[2]:desired_val[64]]
        mov rcx, r8
        mov rbx, rsi ; rbx <- prevVal
        add rbx, r13 ; rbx <- prevVal + incrBy


        lock cmpxchg16b [r11]
        jnz .cas_failure
        mov qword [r12], rsi  ; *result = prevVal
        xor rax, rax
        jmp .end

    .cas_failure:
        ; on failure, [rax:rdx] contain actual values of *target
        mov r10, rax ; target->value
        mov r9, [r12] ; <- r9 = *expected

        ; first possibility: target's value really doesn't match
        ; `expected`, which is the client's responsibility to deal
        ; with - so return error code.
        ;
        ; if target->value != *expected
        cmp r10, r9
        jne .precondition_failed


        ; second possibility: the lockable_atomic_t's seqlock has 
        ; either a read- or write-lock held on it.
        ; return the error code so client can decide when/if to retry.
        ;
        ; if target->slock->isLocked()
        mov r10, rdx
        rw_seqlock_make_lock_mask r9
        and r10, r9
        cmp r10, 0
        jnz .resource_locked

        ; remaining case: the version number has changed, but it's no longer
        ; locked and the value is still what the user is expecting.
        ; this could happen if a writer increments the version while
        ; storing a value identical to what that location already had.
        ; we try again.
        jmp .cas_loop

    .resource_locked:
        mov rax, XSTATUS_RESOURCE_LOCKED
        jmp .end

    .precondition_failed:
        mov qword [r12], rax ; *expected = target->value 
        mov rax, XSTATUS_PRECONDITION_FAILED
        jmp .end

    .end:
        _lu64_pop_common_registers
        ret







global xact_lockable_atomic_u64_incr
xact_lockable_atomic_u64_incr:
        push r11
        push r12
        mov r11, rdi ; target*
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort XSTATUS_RESOURCE_LOCKED

    .x_continue_1:
        inc qword [r11]

    .x_end:
        xend

    .end:
        pop r12
        pop r11
        ret



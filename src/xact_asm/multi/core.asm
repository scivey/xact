bits 64

section .text

%include "common/tsx_defs.asm"
%include "rw_seqlock/dtypes.asm"
%include "rw_seqlock/macros.asm"
%include "multi/dtypes.asm"

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

global xact_lockable_atomic_u64_multi_load_2_mvcc
        ; using Argument = MultiTransaction::Argument
        ; (Argument *marg1, Argument *marg2) -> uint64_t
xact_lockable_atomic_u64_multi_load_2_mvcc:
    .begin:
        _lu64_push_common_registers
        mov r8, rdi ; marg1
        mov r9, rsi ; marg2
        rw_seqlock_make_write_lock_mask rdx
        rw_seqlock_make_version_mask rcx

    .loop_step:
        mov r10, multi_arg_target(r8)
        mov rbx, multi_arg_target(r9)


        ; first step: save starting values of both target's seqlocks.
        ; we need to check the versions later.
        ; first we need to make sure they aren't write-locked. 
        mov r11, qword [r10+8] ; r11 <- target1->slock->value (1st)
        mov r13, qword [rbx+8] ; r13 <- target2->slock->value (1st)

        ; is target1->slock (val1) write-locked?
        mov r12, r11
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked

        ; is target2->slock (val1) write-locked?
        mov r12, r13
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked


        ; second step: load the actual values from both targets
        mov rdi, qword [r10] ; rdi <- target1->value
        mov rsi, qword [rbx] ; rdi <- target2->value


        ; third step: get the final values of both target's seqlocks.
        ; make sure they still aren't write-locked,
        ; then make sure the versions didn't change while we were reading.
        mov r14, qword [r10+8] ; r14 <- target1->slock->value (2nd)
        mov r15, qword [rbx+8] ; r15 <- target2->slock->value (2nd)

        ; is target1->slock (val2) write-locked?
        mov r12, r14
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked

        ; is target2->slock (val2) write-locked?
        mov r12, r15
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked


        ; r12 = target1, version1
        mov r12, r11
        and r12, rcx
        ; rax = target1, version2
        mov rax, r14
        and rax, rcx
        ; if target1.version1 != target1.version2, try again
        cmp r12, rax
        jne .loop_step

        ; r12 = target2, version1
        mov r12, r13
        and r12, rcx
        ; rax = target2, version2
        mov rax, r15
        and rax, rcx
        ; if target2.version1 != target2.version2, try again
        cmp r12, rax
        jne .loop_step

    .success:        
        mov r11, multi_arg_arg1(r8) ; r11 <- arg1.result* (uint64_t*)
        mov qword [r11], rdi        ; *arg1.result = arg1.target->value

        mov r11, multi_arg_arg1(r9) ; r12 <- arg2.result* (uint64_t*)
        mov qword [r11], rsi
        xor rax, rax
        jmp .end

    .resource_locked:
        mov rax, XSTATUS_RESOURCE_LOCKED
        jmp .end

    .end:
        _lu64_pop_common_registers
        ret




global xact_lockable_atomic_u64_multi_store_2_tsx
        ; using Argument = MultiTransaction::Argument
        ; (Argument *marg1, Argument *marg2) -> uint64_t
xact_lockable_atomic_u64_multi_store_2_tsx:
    .begin:
        _lu64_push_common_registers
        mov r8, rdi ; marg1
        mov r9, rsi ; marg2
        rw_seqlock_make_lock_mask rdx
        rw_seqlock_make_version_mask rcx
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .resource_locked:
        xabort XSTATUS_RESOURCE_LOCKED

    .x_begin:
        xbegin .on_failure

    .check_locks:
        mov r10, multi_arg_target(r8)
        mov rbx, multi_arg_target(r9)

        mov r11, qword [r10+8] ; r11 <- target1->slock->value (1st)
        mov r13, qword [rbx+8] ; r13 <- target2->slock->value (1st)

        ; first step: check that neither target is locked
        mov r12, r11
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked

        mov r12, r13
        and r12, rdx
        cmp r12, 0
        jnz .resource_locked

    .do_stores:
        mov r12, multi_arg_arg1(r8)
        mov qword [r10], r12
        mov r12, multi_arg_arg1(r9)
        mov qword [rbx], r12
        xor rax, rax
        xend

    .end:
        _lu64_pop_common_registers
        ret





global xact_lockable_atomic_u64_multi_store_2_mvcc_with_locks_held
        ; using Argument = MultiTransaction::Argument
        ; (Argument *marg1, Argument *marg2) -> uint64_t
xact_lockable_atomic_u64_multi_store_2_mvcc_with_locks_held:
    .begin:
        _lu64_push_common_registers
        mov r8, rdi ; marg1
        mov r9, rsi ; marg2

    .do_store:
        mov r10, multi_arg_target(r8)
        mov r11, multi_arg_arg1(r8)
        mov qword [r10], r11

        mov r10, multi_arg_target(r9)
        mov r11, multi_arg_arg1(r9)
        mov qword [r10], r11
        xor rax, rax
    .end:
        _lu64_pop_common_registers
        ret


bits 64

section .text

%include "common_tsx_defs.asm"
%include "misc_macros.asm"
%include "generalized_cas_dtypes.asm"
%include "rw_seqlock_dtypes.asm"

; all global labels define functions following the SystemV AMD64 ABI


%macro _gcas_push_preserved 0
        push r12
        push r13
        push r14
        push r15
%endmacro

%macro _gcas_pop_preserved 0
        pop r15
        pop r14    
        pop r13
        pop r12
%endmacro


extern xact_rw_seqlock_is_locked


; arg1: register to store pointer to target->seqlock in
; arg2: expression evaluating to a pointer to xact_lockable_atomic_u64_t

%macro call_lock_check 2
        mov %1, %2   ; %1 now contains target*
        add %1, 8    ; %1 now points at target->seqlock*
        push_scratch_except_rax
        mov rdi, %1
        call xact_rw_seqlock_is_locked
        pop_scratch_except_rax
%endmacro













%macro _gencas_core_impl_prelude 0
    ; this function does the main work of the generalized CAS operation
    ; (in TSX-mode)
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
    .entry:
        _gcas_push_preserved
        mov r12, rdi ; preconditions
        mov r13, rsi ; nPrecond
        mov r14, rdx ; operations
        mov r15, rcx ; nOper

        jmp .x_begin
%endmacro



; this is a macro so that it can handle both tsx and non-tsx cases.
; it relies on the _GENCAS_XBEGIN, _GENCAS_XABORT and _GENCAS_XEND
; macros being defined.
; the tsx and locked functions define their own versions of these
; before this macro is substituted
%macro _gencas_core_impl_body 0
    .x_begin:
        _GENCAS_XBEGIN(.on_failure)
        mfence
        ; setup lock-check loop over preconditions*
        mov r8, r12  ; preconditions
        mov rcx, r13 ; nPrecond

        cmp rcx, 0 ; if no preconditions, skip lock check
        je .precondition_lock_checks_succeeded

    .check_precondition_locks_step_1:
        ; r8 currently contains PreconditionCore*
        ; dereferencing this results in pointer to xact_lockable_atomic_u64_t
        call_lock_check r9, precond_target(r8)
        cmp rax, RW_SEQLOCK_IS_LOCKED__FALSE
        jz .check_precondition_locks_step_2
        _GENCAS_XABORT(XSTATUS_RESOURCE_LOCKED)

    .check_precondition_locks_step_2:
        add r8, PRECONDITION_SIZE
        loop .check_precondition_locks_step_1

    .precondition_lock_checks_succeeded:
        mov r8, r14  ; operations
        mov rcx, r15 ; nOper

    .check_operation_locks_step_1:
        ; r8 currently contains OperationCore*
        ; dereferencing this results in pointer to xact_lockable_atomic_u64_t
        call_lock_check r9, oper_target(r8)
        cmp rax, RW_SEQLOCK_IS_LOCKED__FALSE
        jz .check_operation_locks_step_2
        _GENCAS_XABORT(XSTATUS_RESOURCE_LOCKED)

    .check_operation_locks_step_2:
        add r8, OPERATION_SIZE
        loop .check_operation_locks_step_1

    .lock_checks_succeeded:
        ; load pointer to start of preconditions into rdx,
        ; setup loop for nPrecond iterations.
        mov r8, r12
        mov rcx, r13

        cmp rcx, 0 ; if no preconditions, skip the loop
        jz .preconditions_succeeded

    .precondition_loop_step:
        mov r9, precond_type(r8)
        cmp r9, CONDITION_CODE_ALWAYS_TRUE
        jz .precondition_check_success ; 0 is the null precondition
        cmp r9, CONDITION_CODE_EQ
        jz .precondition_eq_check
        cmp r9, CONDITION_CODE_NEQ
        jz .precondition_neq_check
        cmp r9, CONDITION_CODE_LT
        jz .precondition_lt_check
        cmp r9, CONDITION_CODE_GT
        jz .precondition_gt_check
        _GENCAS_XABORT(XSTATUS_INVALID_PRECONDITION)

        .precondition_eq_check:
            mov r9, precond_target(r8)      ; r9 = precondition->target*
            mov rax, [r9]                   ; rax = *target
            cmp rax, precond_arg1(r8)       ; *target == arg1 ?
            je .precondition_check_success
            _GENCAS_XABORT(XSTATUS_PRECONDITION_FAILED)

        .precondition_neq_check:
            mov r9, precond_target(r8)
            mov rax, [r9]
            cmp rax, precond_arg1(r8)
            jnz .precondition_check_success
            _GENCAS_XABORT(XSTATUS_PRECONDITION_FAILED)

        .precondition_lt_check:
            mov r9, precond_target(r8)
            mov rax, [r9]
            cmp rax, precond_arg1(r8)
            jl .precondition_check_success
            _GENCAS_XABORT(XSTATUS_PRECONDITION_FAILED)

        .precondition_gt_check:
            mov r9, precond_target(r8)
            mov rax, [r9]
            cmp rax, precond_arg1(r8)
            jg .precondition_check_success
            _GENCAS_XABORT(XSTATUS_PRECONDITION_FAILED)

        .precondition_check_success:
            add r8, PRECONDITION_SIZE ; incr list pointer
            loop .precondition_loop_step


    .preconditions_succeeded:
        ; load pointer to start of operations into rdx,
        ; setup loop for nOper iterations.
        mov r8, r14
        mov rcx, r15

    .operation_loop_step:
        mov r9, oper_type(r8)
        cmp r9, OPERATION_CODE_NOOP
        jz .operation_end
        cmp r9, OPERATION_CODE_STORE
        jz .operation_store
        cmp r9, OPERATION_CODE_LOAD
        jz .operation_fetch_add
        cmp r9, OPERATION_CODE_FETCH_ADD
        jz .operation_load
        _GENCAS_XABORT(XSTATUS_INVALID_OPERATION)


        .operation_store:
            mov rax, oper_target(r8)   ; rax = target*
            mov r9,  oper_arg1(r8)     ; r9 = arg1
            mov [rax], r9              ; *target = arg1
            jmp .operation_end

        .operation_fetch_add:
            mov rax, oper_target(r8)   ; rax = target*
            mov r9, oper_arg1(r8)      ; r9 = addBy
            mov r10, oper_arg2(r8)     ; r10 = prevVal*
            mov r11, [rax]             ; r11 = *target
            mov [r10], r11             ; *prevVal = *target
            add [rax], r9              ; *target += addBy
            jmp .operation_end

        .operation_load:
            mov rax, oper_target(r8)   ; rax = target*
            mov r9, oper_arg1(r8)      ; r9 = destination*
            mov r10, [rax]             ; r10 = *target
            mov [r9], r10              ; *destination = *target
            jmp .operation_end

        .operation_end:
            add r8, OPERATION_SIZE ; incr list pointer
            loop .operation_loop_step

    .operations_finished:
        mov rax, XSTATUS_SUCCESS
        _GENCAS_XEND

    .end:
        _gcas_pop_preserved
        ret
%endmacro






%define _GENCAS_XBEGIN(label) xbegin label
%define _GENCAS_XABORT(status) xabort status
%define _GENCAS_XEND xend
global xact_generalized_cas_op_tsx_impl
xact_generalized_cas_op_tsx_impl:
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
    _gencas_core_impl_prelude

    .on_failure:
        handle_tsx_failure .end

    _gencas_core_impl_body






%define _GENCAS_XBEGIN(label)
%macro _GENCAS_XABORT 1
    mov rax, %1
    jmp .failed
%endmacro
%define _GENCAS_XEND xend
global xact_generalized_cas_op_with_locks_acquired
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
xact_generalized_cas_op_with_locks_acquired:
    _gencas_core_impl_prelude

    .on_failure:
        jmp .end

    _gencas_core_impl_body








global xact_generalized_cas_op_tsx_with_retries
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations,
    ;   uint64_t numRetries) -> int
    ; 
    ; this function just handles a limited number of retries - defers
    ; to xact_generalized_cas_op_tsx_impl for real work.
    ; (in TSX-mode)
    ;
xact_generalized_cas_op_tsx_with_retries:
    .start:
        _gcas_push_preserved
        mov r11, rdi ; precondition ptr
        mov r12, rsi ; nPrecond
        mov r13, rdx ; operation ptr
        mov r14, rcx ; nOper

        mov r10, r8 ; numRetries
        mov r15, r8 ; numRetries

    .make_call:
        push r10
        push r11
        _gcas_push_preserved
        mov rdi, r11 ; precond*
        mov rsi, r12 ; nPrecond
        mov rdx, r13 ; operation*
        mov rcx, r14 ; nOper
        call xact_generalized_cas_op_tsx_impl
        _gcas_pop_preserved
        pop r11
        pop r10

        cmp rax, XSTATUS_SUCCESS ; if call succeeded, we're done
        je .end

        cmp r15, 0 ; do we have any retries left?
        ; if no, we're done
        je .end
        ; otherwise, decr loop count and busy-wait one cycle
        sub r15, 1
        pause
        pause
        jmp .make_call

    .end:
        _gcas_pop_preserved
        ret



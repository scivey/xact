bits 64

section .text

%include "common/tsx_defs.asm"
%include "common/misc_macros.asm"
%include "generalized_cas/dtypes.asm"
%include "generalized_cas/macros.asm"
%include "rw_seqlock/dtypes.asm"

; all global labels define functions following the SystemV AMD64 ABI





%macro gencas_core_impl_v1_prelude 0
    ; this function does the main work of the generalized CAS operation
    ; (in TSX-mode)
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
    .entry:
        gencas_push_preserved
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
%macro gencas_core_impl_v1_body 0
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
        gencas_call_lock_check r9, precond_target(r8)
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
        gencas_call_lock_check r9, oper_target(r8)
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
        gencas_pop_preserved
        ret
%endmacro





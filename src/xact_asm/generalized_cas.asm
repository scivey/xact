bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

global xact_generalized_cas_op_tsx_impl
    ; this function does the main work of the generalized CAS operation
    ; (in TSX-mode)
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
xact_generalized_cas_op_tsx_impl:
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r12, rdi ; preconditions
        mov r13, rsi ; nPrecond
        mov r14, rdx ; operations
        mov r15, rcx ; nOper

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
        jmp .end

    .x_begin:
        xbegin .on_failure
        mfence
        ; setup lock-check loop over preconditions*
        mov r8, r12  ; preconditions
        mov rcx, r13 ; nPrecond

        cmp rcx, 0 ; if no preconditions, skip lock check
        je .precondition_lock_checks_succeeded

    .check_precondition_locks_step_1:
        mov r9, [r8]
        bt qword [r9+8], 0
        jnc .check_precondition_locks_step_2
        xabort 1 ; resource locked

    .check_precondition_locks_step_2:
        add r8, 32
        loop .check_precondition_locks_step_1

    .precondition_lock_checks_succeeded:
        mov r8, r14  ; operations
        mov rcx, r15 ; nOper

    .check_operation_locks_step_1:
        mov r9, [r8]
        bt qword [r9+8], 0
        jnc .check_operation_locks_step_2
        xabort 1 ; resource locked

    .check_operation_locks_step_2:
        add r8, 32
        loop .check_operation_locks_step_1

    .lock_checks_succeeded:
        ; load pointer to start of preconditions into rdx,
        ; setup loop for nPrecond iterations.
        mov r8, r12
        mov rcx, r13

        cmp rcx, 0 ; if no preconditions, skip the loop
        jz .preconditions_succeeded

    .precondition_loop_step:
        mov r9, [r8+8] ; load PreconditionType into r9
        cmp r9, 0
        jz .precondition_check_success ; 0 is the null precondition
        cmp r9, 1
        jz .precondition_eq_check
        cmp r9, 2
        jz .precondition_neq_check
        cmp r9, 3
        jz .precondition_lt_check
        cmp r9, 4
        jz .precondition_gt_check
        xabort 3 ; invalid precondition

        .precondition_eq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16] ; compare value of *target with precondition arg1
            je .precondition_check_success
            xabort 2 ; precondition failed

        .precondition_neq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jnz .precondition_check_success
            xabort 2 ; precondition failed

        .precondition_lt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jl .precondition_check_success
            xabort 2 ; precondition failed

        .precondition_gt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jg .precondition_check_success
            xabort 2 ; precondition failed

        .precondition_check_success:
            add r8, 32 ; incr PreconditionCore list pointer by sizeof(PreconditionCore)
            loop .precondition_loop_step


    .preconditions_succeeded:
        ; load pointer to start of operations into rdx,
        ; setup loop for nOper iterations.
        mov r8, r14
        mov rcx, r15

    .operation_loop_step:
        mov r9, [r8+8] ; load OperationType into r8
        cmp r9, 0
        jz .operation_end  ; 0 is the null operation (NOOP)
        cmp r9, 1
        jz .operation_store
        cmp r9, 2
        jz .operation_fetch_add
        cmp r9, 3
        jz .operation_load
        xabort 4 ; invalid operation

        .operation_store:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9,  [r8+16] ; load value of arg1
            mov [rax], r9     ; set *target = arg1
            jmp .operation_end

        .operation_fetch_add:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9, [r8+16]  ; load value of arg1
            mov r10, [r8+24] ; load value of arg2: a pointer to previous value of *target
            mov r11, [rax]     ; set r8 = *target
            mov [r10], r11     ; set *arg2 = *target 
            add [rax], r9     ; add arg1 to *target
            jmp .operation_end

        .operation_load:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9, [r8+16]  ; load value of arg1: a pointer to destination
            mov r10, [rax]   ; load *target
            mov [r9], r10    ; set *dest = *target
            jmp .operation_end

        .operation_end:
            add r8, 32 ; incr OperationCore list pointer by sizeof(OperationCore)
            loop .operation_loop_step

    .operations_finished:
        mov rax, 0
        xend

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret


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
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r11, rdi ; precondition ptr
        mov r12, rsi ; nPrecond
        mov r13, rdx ; operation ptr
        mov r14, rcx ; nOper

        mov r10, r8
        mov r15, r8 ; numRetries

    .make_call:
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        mov rdi, r11 ; precond*
        mov rsi, r12 ; nPrecond
        mov rdx, r13 ; operation*
        mov rcx, r14 ; nOper
        call xact_generalized_cas_op_tsx_impl
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10

        cmp rax, 0 ; if call succeeded, we're done
        je .end

        cmp r15, 0 ; do we have any retries left?
        ; if no, we're done
        je .end
        ; otherwise, decr loop count and busy-wait one cycle
        sub r15, 1
        pause
        pause
        pause
        jmp .make_call

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        ret



























global xact_generalized_cas_op_with_locks_acquired
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
xact_generalized_cas_op_with_locks_acquired:
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r12, rdi ; preconditions
        mov r13, rsi ; nPrecond
        mov r14, rdx ; operations
        mov r15, rcx ; nOper
        jmp .begin

    .failed:
        jmp .end

    .begin:
        xor rax, rax
        ; load pointer to start of preconditions into rdx,
        ; setup loop for nPrecond iterations.
        mov r8, r12
        mov rcx, r13

    .precondition_loop_step:
        mov r9, [r8+8] ; load PreconditionType into r9
        cmp r9, 0
        jz .precondition_check_success ; 0 is the null precondition
        cmp r9, 1
        jz .precondition_eq_check
        cmp r9, 2
        jz .precondition_neq_check
        cmp r9, 3
        jz .precondition_lt_check
        cmp r9, 4
        jz .precondition_gt_check
        mov rax, 3 ; invalid precondition
        jmp .failed

        .precondition_eq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16] ; compare value of *target with precondition arg1
            jz .precondition_check_success
            mov rax, 2 ; precondition failed
            jmp .failed

        .precondition_neq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jnz .precondition_check_success
            mov rax, 2 ; precondition failed
            jmp .failed

        .precondition_lt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jl .precondition_check_success
            mov rax, 2 ; precondition failed
            jmp .failed

        .precondition_gt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jg .precondition_check_success
            mov rax, 2 ; precondition failed
            jmp .failed

        .precondition_check_success:
            add r8, 32 ; incr PreconditionCore list pointer by sizeof(PreconditionCore)
            loop .precondition_loop_step


    .preconditions_succeeded:
        ; load pointer to start of operations into rdx,
        ; setup loop for nOper iterations.
        mov r8, r14
        mov rcx, r15

    .operation_loop_step:
        mov r9, [r8+8] ; load OperationType into r8
        cmp r9, 0
        jz .operation_end  ; 0 is the null operation (NOOP)
        cmp r9, 1
        jz .operation_store
        cmp r9, 2
        jz .operation_fetch_add
        cmp r9, 3
        jz .operation_load
        mov rax, 4 ; invalid operation
        jmp .failed

        .operation_store:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9,  [r8+16] ; load value of arg1
            mov [rax], r9     ; set *target = arg1
            jmp .operation_end

        .operation_fetch_add:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9, [r8+16]  ; load value of arg1
            mov r10, [r8+24] ; load value of arg2: a pointer to previous value of *target
            mov r11, [rax]     ; set r8 = *target
            mov [r10], r11     ; set *arg2 = *target 
            add [rax], r9     ; add arg1 to *target
            jmp .operation_end

        .operation_load:
            mov rax, [r8]    ; load AtomicU64 *target into rax
            mov r9, [r8+16]  ; load value of arg1: a pointer to destination
            mov r10, [rax]   ; load *target
            mov [r9], r10    ; set *dest = *target
            jmp .operation_end

        .operation_end:
            add r8, 32 ; incr OperationCore list pointer by sizeof(OperationCore)
            loop .operation_loop_step

    .operations_finished:
        mov rax, 0

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret



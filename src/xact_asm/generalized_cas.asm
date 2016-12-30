bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

global xact_generalized_cas_op
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
xact_generalized_cas_op:
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
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure

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
        xabort 0

        .precondition_eq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16] ; compare value of *target with precondition arg1
            jz .precondition_check_success
            xabort 0

        .precondition_neq_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jnz .precondition_check_success
            xabort 0

        .precondition_lt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jg .precondition_check_success
            xabort 0

        .precondition_gt_check:
            mov r9, [r8]    ; load AtomicU64 target of precondition
            mov rax, [r9]    ; load *target into rax
            cmp rax, [r8+16]  ; compare value of *target with precondition arg1
            jl .precondition_check_success
            xabort 0

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
        xabort 0

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



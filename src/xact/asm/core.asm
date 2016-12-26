bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI


global xact_asm_dcas_u64
    ; performs a DCAS operation on two 64bit pointers
    ; to uint64_t values.  Unlike CMPXCHG16b, the targets don't
    ; have to be adjacent.

    ; args : target1 (uint64_t*),
    ;        target1_expected (uint64_t*),
    ;        target1_desired (uint64_t),
    ;        target2 (uint64_t*),
    ;        target2_expected (uint64_t*),
    ;        target2_desired (uint64_t)
    ; returns: bool, but as int type: 1 for success, 0 for failure)
    xact_asm_dcas_u64:
        push r12
        push r13
        push r14
        push r15
        push rbp
        mov rbp, rsp
        sub rsp, 16 ; 16 bytes on call stack for desired values

        mov r12, rdi ; target1 ptr
        mov r13, rsi ; target1_expected ptr
        mov r14, rcx ; target2 ptr
        mov r15, r8  ; target2_expected ptr

        mov [rbp-8], rdx ; target1_desired value
        mov [rbp-16], r9 ; target2_desired value

        jmp .transaction_pre_check

    .on_transaction_failed:
        ; set target1_expected_ptr to current value of target1_ptr
        mov rdx, [r12]
        mov [r13], rdx

        ; set target2_expected_ptr to current value of target2_ptr
        mov rdx, [r14]
        mov [r15], rdx

        mov rax, 0 ; return false for failure
        jmp .on_end

    .transaction_pre_check:
        mov rax, [r12] ; load current value of target1_ptr
        mov rdx, [r13] ; load value of target1_expected_ptr
        cmp rax, rdx ; compare current *target1_ptr to *target1_expected_ptr
        jne .on_transaction_failed


        mov rax, [r14] ; load current value of target2_ptr
        mov rdx, [r15] ; load value of target2_expected_ptr
        cmp rax, rdx ; compare current *target2_ptr to *target2_expected_ptr
        jne .on_transaction_failed


    .begin_transaction:
        ; the actual transaction
        xbegin .on_transaction_failed


    .transaction_step_1:
        ; test value of target1_ptr against expected value again,
        ; this time within actual transaction block.
        mov rax, [r12] ; load current value of target1_ptr
        mov rdx, [r13] ; load value of target1_expected_ptr
        cmp rax, rdx ; compare current *target1_ptr to *target1_expected_ptr
        je .transaction_step_2
        xabort 0

    .transaction_step_2:
        ; set target1 to desired value
        mov rax, [rsp+8]
        mov [r12], rax

    .transaction_step_3:
        ; test value of target2_ptr against expected value again,
        ; this time within actual transaction block.
        mov rax, [r14] ; load current value of target2_ptr
        mov rdx, [r15] ; load value of target2_expected_ptr
        cmp rax, rdx ; compare current *target2_ptr to *target2_expected_ptr
        je .transaction_step_4
        xabort 0

    .transaction_step_4:
        ; set target2 to desired value
        mov rax, [rsp]
        mov [r14], rax
        xend
        mov rax, 1

    .on_end:
        add rsp, 16 ; make up for the stack space we stole
        pop rbp
        pop r15
        pop r14
        pop r13
        pop r12
        ret










global xact_eq_test
    xact_eq_test:
        push rbp
        sub rsp, 16
        push r12
        push r13
        mov [rbp-16], rdi
        mov [rbp-8], rsi
        mov rdx, [rbp-8]
        mov rax, [rdx]
        mov rdx, [rbp-16]
        cmp rax, rdx
        ; mov r12, rdi
        ; mov r13, [rsi]
        ; cmp r12, r13
        jne xact_eq_test_failure
        mov rax, 1
        jmp xact_eq_test_end

    xact_eq_test_failure:
        mov rax, 0

    xact_eq_test_end:
        pop r13
        pop r12
        add rsp, 16
        pop rbp
        ret






global xact_add_test
    xact_add_test:
        push rbp
        mov rbp, rsp
        sub rsp, 16
        mov [rbp-8], rdi
        mov [rbp-16], rsi
        xor rax, rax
        mov rdx, [rbp-8]
        mov rax, [rdx]
        mov rdx, [rbp-16]
        add rax, [rdx]

        add rsp, 16
        pop rbp
        ret






global xact_asm_scas_u64
    ; performs a CAS on one 64bit pointer.
    ; this doesn't really require TXS - just for testing.
    ; args : target1 (uint64_t*),
    ;        target1_expected (uint64_t*),
    ;        target1_desired (uint64_t)
    ; returns: bool, but as int type: 1 for success, 0 for failure)
    xact_asm_scas_u64:
        push r12
        push r13
        push r14

        xor rax, rax

        mov r12, rdi ; target1 ptr
        mov r13, rsi ; target1_expected ptr
        mov r14, rdx ; desired value

        jmp xact_asm_scas_u64_begin_transaction


    xact_asm_scas_u64_on_failure:
        ; set target1_expected_ptr to current value of target1_ptr
        mov rdx, qword [r12]
        mov qword [r13], rdx

        mov rax, 0 ; return false for failure
        jmp xact_asm_scas_u64_end


    xact_asm_scas_u64_begin_transaction:
        xor rax, rax
        xor rdx, rdx
        mov rax, qword [r12]   ; load current value of target1_ptr
        mov rdx, qword [r13]
        cmp rax, rdx ; compare current target1_ptr value to target1_expected_ptr
        jne xact_asm_scas_u64_on_failure

        ; the actual transaction
        xbegin xact_asm_scas_u64_on_failure
        xor rax, rax
        xor rdx, rdx
        mov rax, qword [r12]
        mov rdx, qword [r13]
        cmp rax, rdx
        je xact_asm_scas_continue_transaction
        ; jmp xact_asm_scas_continue_transaction
        xabort 1

    xact_asm_scas_continue_transaction:
        ; set target1 to desired value
        mov qword [r12], r14
        xend
        mov rax, 1


    xact_asm_scas_u64_end:
        pop r14
        pop r13
        pop r12
        ret

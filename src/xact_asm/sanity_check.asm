bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

; below, eq_test and add_test are smoke tests to sanity-check
; that abi / calling convention are what we expect

global xact_eq_test
    ; (uint64_t, uint64_t) -> bool
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
        jne .on_failure
        mov rax, 1
        jmp .end

    .on_failure:
        mov rax, 0

    .end:
        pop r13
        pop r12
        add rsp, 16
        pop rbp
        ret




global xact_add_test
    ; (uint64_t, uint64_t) -> uint64_t
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



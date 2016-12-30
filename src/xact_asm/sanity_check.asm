bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

; below, eq_test and add_test are smoke tests to sanity-check
; that abi / calling convention are what we expect

global xact_sanity_test_eq_64
    ; (uint64_t, uint64_t) -> bool
xact_sanity_test_eq_64:
        push r12
        push r13
        mov r12, rdi
        mov r13, rsi
        cmp r12, r13

        je .on_eq
        mov rax, 0
        jmp .end

    .on_eq:
        mov rax, 1

    .end:
        pop r13
        pop r12
        ret




global xact_sanity_test_add2_64
    ; (uint64_t, uint64_t) -> uint64_t
xact_sanity_test_add2_64:
        mov rax, rdi
        add rax, rsi
        ret

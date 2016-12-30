bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

global xact_tsx_test_returned_abort_value
    ; validates assumptions re: values returned by xabort
    ; (uint64_t code) -> uint64_t
    ; `code` should in {10, 20, 30, 40}
xact_tsx_test_returned_abort_value:
        push r11
        mov r11, rdi ; code
        xor rax, rax
        jmp .x_begin

    .on_failure:
        ; after xabort (imm8), imm8 should be in eax[31:24]
        xor rdx, rdx
        mov edx, eax
        shr edx, 24
        xor rax, rax
        mov rax, rdx
        jmp .end

    .x_begin:
        xbegin .on_failure

    .step_0:
        cmp r11, 10
        jne .step_1
        xabort 11

    .step_1:
        cmp r11, 20
        jne .step_2
        xabort 21

    .step_2:
        cmp r11, 30
        jne .step_3
        xabort 31

    .step_3:
        cmp r11, 40
        jne .step_4
        xabort 41

    .step_4:
        xabort 100

    .end:
        pop r11
        ret



global xact_tsx_test_abort_explicit
    ; (void) -> uin64_t
xact_tsx_test_abort_explicit:
        jmp .x_begin

    .on_failure:
        ; after xabort (imm8), imm8 should be in eax[31:24]
        jmp .end

    .x_begin:
        xbegin .on_failure
        xabort 80

    .end:
        ret

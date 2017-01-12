bits 64

%include "generalized_cas/macros.asm"
%include "common/tsx_defs.asm"

section .text
extern xact_generalized_cas_op_v1_tsx_impl


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
        gencas_push_preserved
        mov r11, rdi ; precondition ptr
        mov r12, rsi ; nPrecond
        mov r13, rdx ; operation ptr
        mov r14, rcx ; nOper

        mov r10, r8 ; numRetries
        mov r15, r8 ; numRetries

    .make_call:
        push r10
        push r11
        gencas_push_preserved
        mov rdi, r11 ; precond*
        mov rsi, r12 ; nPrecond
        mov rdx, r13 ; operation*
        mov rcx, r14 ; nOper
        call xact_generalized_cas_op_v1_tsx_impl
        gencas_pop_preserved
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
        gencas_pop_preserved
        ret



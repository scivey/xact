bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI


global xact_mfence
    ; (void) -> void
xact_mfence:
        mfence
        ret



global xact_busy_wait
    ; (size_t n_pauses) -> void
xact_busy_wait:
    mov rcx, rdi
    cmp rcx, 0
    je .end

    .pause_loop:
        pause
        loop .pause_loop

    .end:
        ret

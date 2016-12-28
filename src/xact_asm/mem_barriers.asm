bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI


global xact_mfence
    ; (void) -> void
xact_mfence:
        mfence
        ret

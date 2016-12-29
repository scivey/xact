bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI


global xact_atomic_store_u64_single
    ; (uint64_t *target, uint64_t value) -> void
xact_atomic_store_u64_single:
        mov [rdi], rsi
        mfence
        xor rax, rax
        ret


global xact_atomic_load_u64_single
    ; (uint64_t *target) -> uint64_t
xact_atomic_load_u64_single:
        mfence
        mov rax, [rdi]
        ret


global xact_atomic_cas_u64_single
    ; (uint64_t *target, uint64_t *expected, uint64_t desired) -> int
    ; returns 0 on success
    ; on failure, sets *expected to current value of target and returns 1.
xact_atomic_cas_u64_single:
        push r11
        push r12
        push r13
        mov r11, rdi ; target (uint64_t*)
        mov r12, rsi ; expected (uint64_t)
        mov r13, rdx ; desired (uint64_t)

        ; the actual CAS
        mov rax, [r12]
        lock cmpxchg [r11], r13
        jz .on_success

    .on_failure:
        mov [r12], rax
        mov rax, 1
        jmp .end

    .on_success:
        ; mov rax, 7
        mov rax, 0

    .end:
        pop r13
        pop r12
        pop r11
        ret


global xact_atomic_fetch_add_u64_single
    ; (uint64_t *target, uint64_t addBy) -> uint64_t
    ; adds `addBy` to *target; returns previous value of *target.
xact_atomic_fetch_add_u64_single:
        lock xadd [rdi], rsi
        mov rax, rsi
        ret


global xact_atomic_fetch_sub_u64_single
    ; (uint64_t *target, uint64_t addBy) -> uint64_t
    ; adds `addBy` to *target; returns previous value of *target.
    ; on failure, sets *expected to current value of target and returns 1.
xact_atomic_fetch_sub_u64_single:
        neg rsi
        lock xadd [rdi], rsi
        mov rax, rsi
        ret




global xact_atomic_conditional_fetch_add_u64_single_if_between
    ; (uint64_t *target, uint64_t *result, uint64_t addBy,
    ;    uint64_t lowerBound, uin64_t upperBound) -> int
    ;
    ; conditional fetch-add.
    ; 
    ; if `*target` is in the range [`lowerBound`, `upperBound`):
    ;     increment `*target` by `addBy`
    ;     set `*result` to previous value of `*target`
    ;     return 0
    ; else:
    ;     set *result to current value of *target
    ;     return 1

xact_atomic_conditional_fetch_add_u64_single_if_between:
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r11, rdi ; target ptr
        mov r12, rsi ; result ptr
        mov r13, rdx ; addBy
        mov r14, rcx ; lowerBound
        mov r15, r8  ; upperBound
        jmp .x_begin

    .on_failure:
        mov r8, [r11]
        mov [r12], r8 ; store current value of *target in *result
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov r8, [r11] ; load current value of target
        cmp r8, r14
        jge .x_passes_lower_bound
        xabort 0

    .x_passes_lower_bound:
        cmp r8, r15
        jl .x_passes_upper_bound
        xabort 0

    .x_passes_upper_bound:
        mov [r12], r8 ; store previous value of *target in *result

        add r8, r13
        mov [r11], r8 ; incr by `addBy` and store into *target
        mov rax, 0
        xend

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret




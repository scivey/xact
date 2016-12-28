bits 64

section .text

; all global labels define functions following the SystemV AMD64 ABI

global xact_atomic_load_u64_multi
    ; (uint64_t **atomic_targets, uint64_t *dests, size_t n) -> int
    ;
    ; `atomic_targets` is a pointer to an array of pointers,
    ; of which each element should be a pointer to a properly aligned
    ; uint64_t value. (these are the atomics)
    ;
    ; `dests` is a pointer to an array of uint64_t, which will be used
    ; to store the values of the corresponding atomic values from the
    ; first argument.
    ; (i.e. dests[i] will equal `load(atomic_targets[i]))`
    ; 
    ; n gives the number of target atomics.
    ; `dests` must also be at least `n` elements long.
    ;
    ; returns 0 on success. (currently the only error code is 1)
    ;
xact_atomic_load_u64_multi:
        push r11
        push r12
        push r13
        push r14
        mov r11, rdi ; atomics (uint64_t**)
        mov r12, rsi ; dests (uint64_t*)
        mov r13, rdx ; n (size_t)
        jmp .x_begin

    .on_failure:
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov rcx, r13

    .x_loop_step:
        mov r14, [r11]
        mov rax, [r14]
        mov [r12], rax
        add r11, 8
        add r12, 8
        loop .x_loop_step

    .x_end:
        mov rax, 0
        xend

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret



global xact_atomic_store_u64_multi
    ; (uint64_t **atomic_targets, uint64_t *sources, size_t n) -> int
    ;
    ; `atomic_targets` is a pointer to an array of pointers,
    ; of which each element should be a pointer to a properly aligned
    ; uint64_t value. (these are the atomics)
    ;
    ; `sources` is a pointer to an array of uint64_t, which will be used
    ; as the values to store into the corresponding atomics in the first
    ; argument.
    ; i.e. we'll be doing `atomic_store(atomic_targets[i], sources[i])`
    ; for i in [0, n).
    ;
    ; n gives the number of target atomics.
    ; `sources` must also be at least `n` elements long.
    ;
    ; returns 0 on success. (currently the only error code is 1)
    ;
xact_atomic_store_u64_multi:
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r11, rdi ; atomics (uint64_t**)
        mov r12, rsi ; sources (uint64_t*)
        mov r13, rdx ; n (size_t)
        jmp .x_begin

    .on_failure:
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov rcx, r13

    .x_loop_step:
        mov r15, [r11]
        mov r14, [r12]
        mov [r15], r14
        add r11, 8
        add r12, 8
        loop .x_loop_step

    .x_end:
        mov rax, 0
        xend

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret



global xact_atomic_add_u64_multi
    ; (uint64_t **atomic_targets, uint64_t amount, size_t n) -> int
    ;
    ; `atomic_targets` is a pointer to an array of pointers,
    ; of which each element should be a pointer to a properly aligned
    ; uint64_t value. (these are the atomics)
    ;
    ; `amount` is the value to be added to each of the atomics.
    ;
    ; i.e. we'll be doing `atomic_increment(atomic_targets[i], amount)`
    ; for i in [0, n).
    ;
    ; n gives the number of target atomics.
    ; `sources` must also be at least `n` elements long.
    ;
    ; returns 0 on success. (currently the only error code is 1)
    ;
xact_atomic_add_u64_multi:
        push r11
        push r12
        push r13
        push r14
        mov r11, rdi ; atomics (uint64_t**)
        mov r12, rsi ; amount (uint64_t)
        mov r13, rdx ; n (uint8_t)
        jmp .x_begin

    .on_failure:
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov rcx, r13

    .x_loop_step:
        mov r14, [r11]
        add [r14], r12
        add r11, 8
        loop .x_loop_step

    .x_end:
        mov rax, 0
        xend

    .end:
        pop r14
        pop r13
        pop r12
        pop r11
        ret


global xact_atomic_sub_u64_multi
    ; (uint64_t **atomic_targets, uint64_t amount, size_t n) -> int
    ; see notes for xact_atomic_add_u64_multi
xact_atomic_sub_u64_multi:
        neg rsi
        call xact_atomic_add_u64_multi
        ret


global xact_atomic_fetch_add_u64_multi
    ; (uint64_t **atomic_targets, uint64_t *results, uint64_t amount, size_t n) -> int
    ;
    ; `atomic_targets` is a pointer to an array of pointers,
    ; of which each element should be a pointer to a properly aligned
    ; uint64_t value. (these are the atomics)
    ;
    ; `results` is an n-length uint64_t array which will contain
    ; the *previous* (prior to adding `amount`) values of the atomic targets
    ; if the transaction succeeds.
    ; ( this follows the semantics of std::atomic<T>::fetch_add )
    ;
    ; `amount` is the value to be added to each of the atomics.
    ;
    ; n gives the number of target atomics.
    ;
    ; i.e. we'll be doing `results[i] = atomic_fetch_add(atomic_targets[i], amount)`
    ; for i in [0, n).
    ;
    ; returns 0 on success. (currently the only error code is 1)
    ;
xact_atomic_fetch_add_u64_multi:
        push r11
        push r12
        push r13
        push r14
        push r15
        mov r11, rdi ; atomics (uint64_t**)
        mov r12, rsi ; results (uint64_t*)
        mov r13, rdx ; amount (uint64_t)
        mov r14, rcx ; n
        jmp .x_begin

    .on_failure:
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov rcx, r14

    .x_loop_step:
        mov r15, [r11] ; load address of current atomic target
        mov r10, [r15] ; load current value of target
        mov [r12], r10 ; store current value of target in results[i]
        add [r15], r13 ; add `amount` to current target
        add r11, 8
        add r12, 8
        loop .x_loop_step

    .x_end:
        mov rax, 0
        xend

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret



global xact_atomic_fetch_sub_u64_multi
    ; (uint64_t **atomic_targets, uint64_t *results, uint64_t amount, size_t n) -> int
    ; see notes for xact_atomic_fetch_add_u64_multi
xact_atomic_fetch_sub_u64_multi:
        neg rdx
        call xact_atomic_fetch_add_u64_multi
        ret



global xact_atomic_cas_u64_multi
    ; (uint64_t **atomic_targets, uint64_t *expected, uint64_t *desired, size_t n)
    ;   -> int
    ;
    ; `atomic_targets` is a pointer to an `n`-length array of pointers,
    ; of which each element should be a pointer to a properly aligned
    ; uint64_t value. (these are the atomics)
    ;
    ; `expected` is an `n`-length array of the expected uint64_t values,
    ; while `desired` is an `n`-length array of our desired uint64_t values.
    ;
    ; i.e. for each i in [0, n), we're doing the equivalent of:
    ;   `compare_exchange(targets[i], expected[i], desired[i])`
    ; (except that the entire CAS sequence will succeed or fail atomically)
    ;
    ; returns 0 on success. (currently the only error code is 1)
    ;    
xact_atomic_cas_u64_multi:
        push r11
        push r12
        push r13
        push r14
        push r15

        mov r11, rdi ; atomics (uint64_t**)
        mov r12, rsi ; expected (uint64_t*)
        mov r13, rdx ; desired (uint64_t*)
        mov r14, rcx ; n elements (size_t)

        jmp .x_begin

    .on_failure:
        mov rax, 1
        jmp .end

    .x_begin:
        xbegin .on_failure
        mov rcx, r14

    .x_loop_step:
        mov r8, [r11] ; load address of current atomic 
        mov r9, [r12] ; load current expectation
        mov r10, [r8] ; load current value of current atomic

        cmp r10, r9 ; atomics[i] == expected[i] ?
        ; if (*atomics[i] == expected[i])
        je .x_loop_step_success

        ; if (*atomics[i] != expected[i])
        xabort 0

    .x_loop_step_success:
        mov r10, [r13] ; load current desired value
        mov [r8], r10 ; set *atomics[i] = desired[i]

        add r11, 8
        add r12, 8
        add r13, 8
        loop .x_loop_step

    .x_end:
        mov rax, 0
        xend

    .end:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        ret





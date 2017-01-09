bits 64

section .text

%include "common_tsx_defs.asm"
%include "rw_seqlock_dtypes.asm"
%include "rw_seqlock_macros.asm"


%macro _lu64_push_common_registers 0
        push r11
        push r12
        push r13
        push r14
        push r15
%endmacro

%macro _lu64_pop_common_registers 0
        pop r15
        pop r14    
        pop r13
        pop r12
        pop r11
%endmacro


global xact_lockable_atomic_u64_compare_exchange
xact_lockable_atomic_u64_compare_exchange:
        _lu64_push_common_registers

        mov r11, rdi ; target*
        mov r12, rsi ; expected*
        mov r13, rdx ; desired
        xor rax, rax
        mfence        
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort XSTATUS_RESOURCE_LOCKED

    .x_continue_1:
        mov r14, qword [r11]
        mov r8, qword [r12]
        mfence
        cmp r14, r8   ; compare *target with *expected
        je .x_continue_2
        xabort XSTATUS_PRECONDITION_FAILED

    .x_continue_2:
        mfence
        mov rax, r8
        lock cmpxchg qword [r11], r13
        jz .x_end
        xabort XSTATUS_PRECONDITION_FAILED

    .x_end:
        xend
        xor rax, rax
        mfence        

    .end:
        _lu64_pop_common_registers
        ret



global xact_lockable_atomic_u64_load_tsx
xact_lockable_atomic_u64_load_tsx:
        _lu64_push_common_registers

        mov r11, rdi ; target
        mov r12, rsi ; result

        mfence        
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .x_begin:
        xbegin .on_failure
        mov r13, qword [r11+8]
        bt r13, 0
        jnc .x_continue_1
        xabort XSTATUS_RESOURCE_LOCKED

    .x_continue_1:
        mfence
        mov r13, qword [r11]
        mov qword [r12], r13
        mov rax, 0

    .x_end:
        xend
        mfence

    .end:
        _lu64_pop_common_registers
        ret



global xact_lockable_atomic_u64_load_raw_value_dangerously
;   (xact_lockable_atomic_u64_t*) -> uint64_t
xact_lockable_atomic_u64_load_raw_value_dangerously:
        mov rax, qword [rdi]    ; result = *atom
        ret



global xact_lockable_atomic_u64_store_tsx
xact_lockable_atomic_u64_store_tsx:
        _lu64_push_common_registers

        mov r11, rdi ; target
        mov r12, rsi ; value
        mfence        
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .x_begin:
        xbegin .on_failure
        mov r13, [rdi+8]
        bt r13, 0
        jnc .x_continue_1
        xabort XSTATUS_RESOURCE_LOCKED

    .x_continue_1:
        mov qword [r11], r12
        mov rax, 0
        mfence

    .x_end:
        xend
        mfence        

    .end:
        _lu64_pop_common_registers
        ret




global xact_lockable_atomic_u64_fetch_add_tsx
xact_lockable_atomic_u64_fetch_add_tsx:
        _lu64_push_common_registers

        mov r11, rdi ; target*
        mov r12, rsi ; result*
        mov r13, rdx ; add_by
        jmp .x_begin

    .on_failure:
        handle_tsx_failure .end

    .x_begin:
        xbegin .on_failure
        bt qword [r11+8], 0
        jnc .x_continue_1
        xabort XSTATUS_RESOURCE_LOCKED

    .x_continue_1:
        mov r14, [r11]
        add [r11], r13
        mov [r12], r14
        mov rax, 0

    .x_end:
        xend

    .end:
        _lu64_pop_common_registers
        ret

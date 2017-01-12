extern xact_rw_seqlock_is_locked

; arg1: register to store pointer to target->seqlock in
; arg2: expression evaluating to a pointer to xact_lockable_atomic_u64_t

%macro gencas_call_lock_check 2
        mov %1, %2   ; %1 now contains target*
        add %1, 8    ; %1 now points at target->seqlock*
        push_scratch_except_rax
        mov rdi, %1
        call xact_rw_seqlock_is_locked
        pop_scratch_except_rax
%endmacro

%macro gencas_push_preserved 0
        push r12
        push r13
        push r14
        push r15
%endmacro

%macro gencas_pop_preserved 0
        pop r15
        pop r14    
        pop r13
        pop r12
%endmacro

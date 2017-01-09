
%macro push_r8_thru_11 0
    ; preserve all scratch registers
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro pop_r11_thru_8 0
    ; pop all scratch registers
    pop r11
    pop r10
    pop r9
    pop r8
%endmacro

; push all scratch registers except rax
%macro push_scratch_except_rax 0
    push rdi
    push rsi
    push rdx
    push rcx

    push r8
    push r9
    push r10
    push r11
    ; push_r8_thru_r11

%endmacro


; pop all scratch registers except rax
%macro pop_scratch_except_rax 0
    ; pop_r11_thru_8
    pop r11
    pop r10
    pop r9
    pop r8

    pop rcx
    pop rdx
    pop rsi
    pop rdi


%endmacro

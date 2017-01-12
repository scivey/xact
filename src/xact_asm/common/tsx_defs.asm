%define TSX_ZERO_FAILURE_CODE 0x00b33f00

%define XSTATUS_SUCCESS 0
%define XSTATUS_RESOURCE_LOCKED 1
%define XSTATUS_PRECONDITION_FAILED 2
%define XSTATUS_INVALID_PRECONDITION 3
%define XSTATUS_INVALID_OPERATION 4



; takes label of the function-end handler as argument
%macro handle_tsx_failure 1
        cmp rax, 0
        je %%handleZeroFailure
        jmp %1

    %%handleZeroFailure:
        ; TSX sometimes aborts with a status code of 0.
        ; we signal the caller of this case by zeroing
        ; upper RAX and setting EAX to 0x00b33f00.
        ; the higher-level API converts this to
        ; TransactionStatus::TSX_ZERO_FAILURE.
        xor rax, rax
        mov eax, TSX_ZERO_FAILURE_CODE
        jmp %1
%endmacro



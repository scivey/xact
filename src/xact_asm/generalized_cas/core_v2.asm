%include "generalized_cas/core_impl_v2.asm"

%define _GENCAS_XBEGIN(label) xbegin label
%define _GENCAS_XABORT(status) xabort status
%define _GENCAS_XEND xend
global xact_generalized_cas_op_v2_tsx_impl
xact_generalized_cas_op_v2_tsx_impl:
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
    gencas_core_impl_v2_prelude

    .on_failure:
        handle_tsx_failure .end

    gencas_core_impl_v2_body




%define _GENCAS_XBEGIN(label)
%macro _GENCAS_XABORT 1
    mov rax, %1
    jmp .failed
%endmacro
%define _GENCAS_XEND
global xact_generalized_cas_op_v2_with_locks_acquired
    ;
    ; (PreconditionCore* [8b], uint64_t numPreconditions,
    ;  OperationCore* [8b], uint64_t numOperations) -> int
    ; 
    ; PreconditionCore and OperationCore are both 32b wide.
    ;
xact_generalized_cas_op_v2_with_locks_acquired:
    gencas_core_impl_v2_prelude

    .on_failure:
        jmp .end

    gencas_core_impl_v2_body




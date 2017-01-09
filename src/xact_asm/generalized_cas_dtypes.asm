%define PRECONDITION_SIZE 32
%define PRECONDITION_TARGET_OFFSET 0
%define PRECONDITION_TYPE_OFFSET 8
%define PRECONDITION_ARG1_OFFSET 16
%define PRECONDITION_ARG2_OFFSET 24

%define OPERATION_SIZE 32
%define OPERATION_TARGET_OFFSET 0
%define OPERATION_TYPE_OFFSET 8
%define OPERATION_ARG1_OFFSET 16
%define OPERATION_ARG2_OFFSET 24


%define _mk_offset(src, offset) qword [src + offset]
%define precond_target(reg) _mk_offset(reg, PRECONDITION_TARGET_OFFSET)
%define precond_type(reg) _mk_offset(reg, PRECONDITION_TYPE_OFFSET)
%define precond_arg1(reg) _mk_offset(reg, PRECONDITION_ARG1_OFFSET)
%define precond_arg2(reg) _mk_offset(reg, PRECONDITION_ARG2_OFFSET)
%define oper_target(reg) _mk_offset(reg, OPERATION_TARGET_OFFSET)
%define oper_type(reg) _mk_offset(reg, OPERATION_TYPE_OFFSET)
%define oper_arg1(reg) _mk_offset(reg, OPERATION_ARG1_OFFSET)
%define oper_arg2(reg) _mk_offset(reg, OPERATION_ARG2_OFFSET)



%define CONDITION_CODE_ALWAYS_TRUE 0
%define CONDITION_CODE_EQ          1
%define CONDITION_CODE_NEQ         2
%define CONDITION_CODE_LT          3
%define CONDITION_CODE_GT          4


%define OPERATION_CODE_NOOP        0
%define OPERATION_CODE_STORE       1
%define OPERATION_CODE_LOAD        2
%define OPERATION_CODE_FETCH_ADD   3



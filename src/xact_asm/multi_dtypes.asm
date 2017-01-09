%define MULTI_ARGUMENT_SIZE 24
%define MULTI_ARGUMENT_TARGET_OFFSET 0
%define MULTI_ARGUMENT_ARG1_OFFSET 8
%define MULTI_ARGUMENT_ARG2_OFFSET 16

%define _mk_offset(src, offset) qword [src + offset]
%define multi_arg_target(reg) _mk_offset(reg, MULTI_ARGUMENT_TARGET_OFFSET)
%define multi_arg_arg1(reg)   _mk_offset(reg, MULTI_ARGUMENT_ARG1_OFFSET)
%define multi_arg_arg2(reg)   _mk_offset(reg, MULTI_ARGUMENT_ARG2_OFFSET)

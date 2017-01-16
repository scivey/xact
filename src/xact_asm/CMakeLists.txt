find_program(NASM NAMES nasm)

set(HERE ${CMAKE_CURRENT_SOURCE_DIR})
set(DEST ${CMAKE_BINARY_DIR})

# set(NASM_DEBUG_FLAGS "-gdwarf")
set(NASM_DEBUG_FLAGS "")

macro(add_nasm_command src_name obj_name)
    add_custom_command(
        OUTPUT ${DEST}/${obj_name}
        COMMAND ${NASM}
        ARGS -I ${HERE} -f elf64 ${NASM_DEBUG_FLAGS} -o ${DEST}/${obj_name} ${HERE}/${src_name} 
        DEPENDS ${HERE}/${src_name}
        WORKING_DIRECTORY ${HERE}
    )
endmacro()


set(ASM_SOURCES
    generalized_cas/core_v1
    generalized_cas/core_v2
    generalized_cas/retry
    lockable_atomic_u64/core
    lockable_atomic_u64/retired
    misc/util
    misc/tsx_tests
    misc/sanity_check
    multi/core
    rw_seqlock/core
)
set(BUILT_ASM_OBJ "")

foreach(asm_src ${ASM_SOURCES})
    string(REPLACE "/" "_" asm_src_out "${asm_src}")
    add_nasm_command(${asm_src}.asm xact_${asm_src_out}.o)
    LIST(APPEND BUILT_ASM_OBJ ${DEST}/xact_${asm_src_out}.o)
endforeach()

add_library(xact_asm ${BUILT_ASM_OBJ})
set_target_properties(xact_asm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(xact_asm PUBLIC ${XACT_INCLUDE_ROOT})

install(TARGETS xact_asm DESTINATION lib)
install(DIRECTORY ${XACT_INCLUDE_ROOT}/xact_asm DESTINATION include)

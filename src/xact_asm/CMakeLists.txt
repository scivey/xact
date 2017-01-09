find_program(NASM NAMES nasm)

set(HERE ${CMAKE_CURRENT_SOURCE_DIR})
set(DEST ${CMAKE_BINARY_DIR})

macro(add_nasm_command src_name obj_name)
    add_custom_command(
        OUTPUT ${DEST}/${obj_name}
        COMMAND ${NASM}
        ARGS -I ${HERE} -f elf64 -o ${DEST}/${obj_name} ${HERE}/${src_name}
        DEPENDS ${HERE}/${src_name}
        WORKING_DIRECTORY ${HERE}
    )
endmacro()


set(ASM_SOURCES
    generalized_cas
    lockable_atomic_u64
    lockable_atomic_u64_retired
    rw_seqlock
    sanity_check
    tsx_tests
    util
)
set(BUILT_ASM_OBJ "")

foreach(asm_src ${ASM_SOURCES})
    add_nasm_command(${asm_src}.asm xact_${asm_src}.o)
    LIST(APPEND BUILT_ASM_OBJ ${DEST}/xact_${asm_src}.o)
endforeach()

add_library(xact_asm ${BUILT_ASM_OBJ})
set_target_properties(xact_asm PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(xact_asm PUBLIC ${XACT_INCLUDE_ROOT})

install(TARGETS xact_asm DESTINATION lib)
install(DIRECTORY ${XACT_INCLUDE_ROOT}/xact_asm DESTINATION include)

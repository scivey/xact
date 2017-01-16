set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(COMMON_PROF_DEPS xact xact_asm)
set(COMMON_PROF_LIBS xact xact_asm ${XACT_COMMON_LIBS})

macro(add_prof prof_name)
    add_executable(${prof_name} ${ARGN})
    add_dependencies(${prof_name} ${COMMON_PROF_DEPS})
    target_link_libraries(${prof_name} ${COMMON_PROF_LIBS})
    target_include_directories(${prof_name} PUBLIC ${XACT_INCLUDE_ROOT})
endmacro()

add_prof(prof_multi ${HERE}/prof_multi.cpp)

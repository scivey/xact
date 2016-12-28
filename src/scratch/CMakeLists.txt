set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(run_scratch
    ${HERE}/main.cpp
)
add_dependencies(run_scratch xact xact_asm)

set(SCRATCH_LIBS
    xact
    xact_asm
    glog
    pthread
    atomic
    ${XACT_COMMON_LIBS}
)

target_link_libraries(run_scratch ${SCRATCH_LIBS})
target_include_directories(run_scratch PUBLIC ${XACT_INCLUDE_ROOT})

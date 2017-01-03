set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

set(COMMON_BENCH_DEPS xact xact_asm)
set(COMMON_BENCH_LIBS xact xact_asm benchmark ${XACT_COMMON_LIBS})

set(COMMON_BENCH_INCLUDE ${XACT_INCLUDE_ROOT} ${HERE})


macro(add_benchmark bench_name)
    add_executable(${bench_name} ${ARGN})
    add_dependencies(${bench_name} ${COMMON_BENCH_DEPS})
    target_link_libraries(${bench_name} ${COMMON_BENCH_LIBS})
    target_include_directories(${bench_name} PUBLIC ${COMMON_BENCH_INCLUDE})
endmacro()

add_benchmark(micro_benchmarks ${HERE}/micro_benchmarks.cpp)
add_benchmark(spinlock_comparison ${HERE}/spinlock_comparison.cpp)

cmake_minimum_required(VERSION 2.8)

project(xact)


# set(SSE_FLAGS "-march=native -msse4.2 -msse4.1 -msse3 -mssse3 -mavx -mpclmul")
set(SSE_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SSE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SSE_FLAGS}")


set(EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(GTEST_BASE ${EXTERNAL}/googletest)
set(BENCHMARK_BASE ${EXTERNAL}/benchmark)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GTEST_BASE}/googletest/include
    ${GTEST_BASE}/googlemock/include
    ${BENCHMARK_BASE}/include
)

link_directories(
    ${GTEST_BASE}/build/googlemock
    ${BENCHMARK_BASE}/build/src
)

set(COMMON_LIBS
    glog
    pthread
    atomic
)

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XACT_SRC_DIR ${SRC_ROOT}/xact)
set(XACT_SRC_ASM_DIR ${XACT_SRC_DIR}/asm)

find_program(NASM NAMES nasm)

add_custom_command(OUTPUT xact_asm_core.o COMMAND ${NASM}
                   ARGS -f elf64 -o xact_asm_core.o ${XACT_SRC_ASM_DIR}/core.asm)


add_library(xact_asm xact_asm_core.o)
set_target_properties(xact_asm PROPERTIES LINKER_LANGUAGE CXX)



FILE(GLOB XACT_LIB_SRC
    ${XACT_SRC_DIR}/*.cpp
)

add_library(xact
    ${XACT_LIB_SRC}
)
add_dependencies(xact xact_asm)

set(SCRATCH_SRC_DIR ${SRC_ROOT}/scratch)




add_executable(run_scratch
    ${SCRATCH_SRC_DIR}/main.cpp
)
add_dependencies(run_scratch xact xact_asm)

set(SCRATCH_LIBS
    xact
    xact_asm
    glog
    pthread
    atomic
)

target_link_libraries(run_scratch ${SCRATCH_LIBS})


FILE(GLOB XACT_TEST_SRC
    ${SRC_ROOT}/test/*.cpp
    ${SRC_ROOT}/test/**/*.cpp
)

add_executable(xact_test_runner
    ${XACT_TEST_SRC}
    ${SRC_ROOT}/run_tests.cpp
)

add_dependencies(xact_test_runner xact xact_asm)
target_link_libraries(xact_test_runner
    gmock
    ${SCRATCH_LIBS}
)

add_executable(benchmark_runner
    ${SRC_ROOT}/run_benchmarks.cpp
)
add_dependencies(benchmark_runner xact xact_asm)
target_link_libraries(benchmark_runner benchmark ${SCRATCH_LIBS})


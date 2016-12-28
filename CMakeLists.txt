cmake_minimum_required(VERSION 2.8)

project(xact)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Logging)
include(BuildType)
include(CompilerFlags)

set_default_build_type()
set_default_compiler_flags()
print_build_settings()

set(EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(GTEST_BASE ${EXTERNAL}/googletest)
set(BENCHMARK_BASE ${EXTERNAL}/benchmark)

set(XACT_INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(
    ${XACT_INCLUDE_ROOT}
    ${GTEST_BASE}/googletest/include
    ${GTEST_BASE}/googlemock/include
    ${BENCHMARK_BASE}/include
)

link_directories(
    ${GTEST_BASE}/build/googlemock
    ${BENCHMARK_BASE}/build/src
)

set(XACT_COMMON_LIBS
    glog
    pthread
    atomic
)

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SKIP_INSTALL_ALL true)
add_subdirectory(${SRC_ROOT}/xact_asm)
add_subdirectory(${SRC_ROOT}/xact)
add_subdirectory(${SRC_ROOT}/test)
add_subdirectory(${SRC_ROOT}/bench)
add_subdirectory(${SRC_ROOT}/scratch)


set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB XACT_UNIT_TEST_SOURCES
    ${HERE}/unit/*.cpp
    ${HERE}/unit/**/*.cpp
    ${HERE}/unit/**/**/*.cpp
)

add_executable(run_unit_tests
    ${XACT_UNIT_TEST_SOURCES}
    ${HERE}/run_tests.cpp
)

target_link_libraries(run_unit_tests
    gmock
    xact
    xact_asm
    ${XACT_COMMON_LIBS}
)

add_dependencies(run_unit_tests xact xact_asm)

target_include_directories(run_unit_tests PUBLIC
    ${HERE}/include
    ${XACT_INCLUDE_ROOT}
)

set(HERE ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB XACT_LIB_SRC
    ${HERE}/*.cpp
    ${HERE}/**/*.cpp
)

add_library(xact
    ${XACT_LIB_SRC}
)

add_dependencies(xact xact_asm)
target_link_libraries(xact xact_asm)
target_include_directories(xact PUBLIC ${XACT_INCLUDE_ROOT})

install(TARGETS xact DESTINATION lib)
install(DIRECTORY ${XACT_INCLUDE_ROOT}/xact DESTINATION include)

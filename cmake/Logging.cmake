function(log_info arg)
    MESSAGE(STATUS "${arg}")
endfunction()

function(say arg)
    log_info(${arg})
endfunction()

function(print_build_settings)
    log_info("BUILD TYPE: ${CMAKE_BUILD_TYPE}")
    log_info("* * * *")
    log_info("CXX COMPILER: ${CMAKE_CXX_COMPILER}")
    log_info("CXX FLAGS: ${CMAKE_CXX_FLAGS}")
    log_info("CXX_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
    log_info("CXX_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

    log_info("C COMPILER: ${CMAKE_C_COMPILER}")
    log_info("C FLAGS: ${CMAKE_C_FLAGS}")
    log_info("* * * *")
    log_info("OUTPUT : LIBS : '${LIBRARY_OUTPUT_PATH}'")
    log_info("OUTPUT : BINS : '${EXECUTABLE_OUTPUT_PATH}'")

endfunction()

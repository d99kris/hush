# Project
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(hush VERSION 1.0 LANGUAGES C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g \
                   -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual \
                   -Wstrict-prototypes -Wmissing-prototypes \
                   -Wno-missing-braces -Wswitch-default \
                   -Wcast-align -Wunreachable-code -Wundef -Wuninitialized")

# Target paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Faketty Utility
add_executable(faketty src/faketty.c)
install(TARGETS faketty DESTINATION bin)

# Hush Main Program
configure_file(src/hush ${CMAKE_CURRENT_BINARY_DIR}/bin/hush COPYONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/hush DESTINATION bin)

# Manual Pages
configure_file(src/faketty.1 ${CMAKE_CURRENT_BINARY_DIR}/share/man/man1/faketty.1 COPYONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/man/man1/faketty.1 DESTINATION share/man/man1)
configure_file(src/hush.1 ${CMAKE_CURRENT_BINARY_DIR}/share/man/man1/hush.1 COPYONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/man/man1/hush.1 DESTINATION share/man/man1)

# Tests
enable_testing()

configure_file(tests/test001 ${CMAKE_CURRENT_BINARY_DIR}/test001 COPYONLY)
add_test(test001 "${PROJECT_BINARY_DIR}/test001")

configure_file(tests/test002 ${CMAKE_CURRENT_BINARY_DIR}/test002 COPYONLY)
add_test(test002 "${PROJECT_BINARY_DIR}/test002")

configure_file(tests/test003 ${CMAKE_CURRENT_BINARY_DIR}/test003 COPYONLY)
add_test(test003 "${PROJECT_BINARY_DIR}/test003")


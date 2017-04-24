#project name
cmake_minimum_required (VERSION 2.6)
PROJECT(webserv C)

file(GLOB SOURCES "src/*.c")
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ansi -Wall -pthread -Wpedantic -D_GNU_SOURCE -std=c99")
endif()


add_executable(webserv ${SOURCES})
add_subdirectory(tests)

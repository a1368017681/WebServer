#project name
PROJECT(webserv C)

file(GLOB SOURCES "src/*.c")
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -D_GNU_SOURCE -std=c99")
endif()


add_executable(webserv ${SOURCES})

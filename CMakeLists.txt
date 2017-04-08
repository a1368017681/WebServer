#project name
PROJECT(webserv C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -D_GNU_SOURCE -std=c99")
endif()

add_library(WebServer socklib.c)
add_executable(webserv webserv.c)
target_link_libraries(webserv WebServer)
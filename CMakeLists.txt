#project name

# setup compiler
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# define project
cmake_minimum_required (VERSION 2.6)
PROJECT(webserv)

# set project version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSUON_PATCH 0)

option(DEPLOY_TEST "deploy test env" OFF)

# required packages
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)

# setup environment
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -std=c++0x -fPIC -Wall -Wl,--export-dynamic -Wno-deprecated")

# Definitions
add_definitions(
    -DDEBUG
)

# link libraries
link_libraries(
    ${CMAKE_THREAD_LIBS_INIT}
    jsoncpp
)

# set include dic path
include_directories(
    "/usr/include/jsoncpp"
)

# set link dic path
link_directories(
)

file(GLOB SOURCES "src/*.cpp")
set(SRC_LIST
    src/*.cpp
)
#if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ansi -Wall -pthread -Wpedantic -D_GNU_SOURCE -std=c99")
#endif()


add_executable(webserv ${SOURCES})
add_subdirectory(tests)

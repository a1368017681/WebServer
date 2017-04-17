include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test test.c ../src/priority_queue.c ../src/memory_pool.c)

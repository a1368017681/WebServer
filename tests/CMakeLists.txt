include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(pq_test pq_test.c ../src/priority_queue.c ../src/memory_pool.c)

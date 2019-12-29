cmake_minimum_required(VERSION 3.1)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_link_libraries(ThreadPool_test ${LIB_FULL_NAME})
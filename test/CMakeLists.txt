cmake_minimum_required(VERSION 3.1)

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test ${LIB_FULL_NAME})
cmake_minimum_required(VERSION 3.1)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_link_libraries(ThreadPool_test ${LIB_FULL_NAME})

add_executable(EventLoop_test EventLoop_test.cc)
target_link_libraries(EventLoop_test ${LIB_FULL_NAME})

add_executable(TcpServer_test TcpServer_test.cc)
target_link_libraries(TcpServer_test ${LIB_FULL_NAME})
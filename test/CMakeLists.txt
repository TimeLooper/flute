cmake_minimum_required(VERSION 3.1)

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test ${LIB_FULL_NAME})

add_executable(Buffer_test Buffer_test.cpp)
target_link_libraries(Buffer_test ${LIB_FULL_NAME})

add_executable(ThreadPool_test ThreadPool_test.cpp)
target_link_libraries(ThreadPool_test ${LIB_FULL_NAME})

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test ${LIB_FULL_NAME})
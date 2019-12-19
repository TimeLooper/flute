cmake_minimum_required(VERSION 3.1)

add_executable(EventLoop_test EventLoop_test.cpp)
target_link_libraries(EventLoop_test ${LIB_FULL_NAME})

add_executable(Buffer_test Buffer_test.cpp)
target_link_libraries(Buffer_test ${LIB_FULL_NAME})

add_executable(ThreadPool_test ThreadPool_test.cpp)
target_link_libraries(ThreadPool_test ${LIB_FULL_NAME})

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test ${LIB_FULL_NAME})

add_executable(EchoServer_test EchoServer_test.cpp)
target_link_libraries(EchoServer_test ${LIB_FULL_NAME})

add_executable(EchoClient_test EchoClient_test.cpp)
target_link_libraries(EchoClient_test ${LIB_FULL_NAME})
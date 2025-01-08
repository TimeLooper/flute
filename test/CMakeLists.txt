cmake_minimum_required(VERSION 3.6)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_link_libraries(ThreadPool_test ${LIB_FULL_NAME})

add_executable(EventLoop_test EventLoop_test.cc)
target_link_libraries(EventLoop_test ${LIB_FULL_NAME})

add_executable(TcpServer_test TcpServer_test.cc)
target_link_libraries(TcpServer_test ${LIB_FULL_NAME})

add_executable(TcpClient_test TcpClient_test.cc)
target_link_libraries(TcpClient_test ${LIB_FULL_NAME})

add_executable(UdpServer_test UdpServer_test.cc)
target_link_libraries(UdpServer_test ${LIB_FULL_NAME})

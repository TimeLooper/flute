cmake_minimum_required(VERSION 3.1)

project(flute)

# 设置默认为Release版本编译
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Set build type to Debug o Release (default Release)" FORCE)
endif ()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

set(LIBS_EXPORT_TYPE "shared" CACHE STRING "set library export type to shared, static or inline (default shared)")

set(CMAKE_CXX_STANDARD 11)

include(CheckTypeSize)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)

# 设置编译导出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# 头文件检测
CHECK_INCLUDE_FILE(sys/types.h FLUTE_HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/epoll.h FLUTE_HAVE_SYS_EPOLL_H)
CHECK_INCLUDE_FILE(sys/socket.h FLUTE_HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE(unistd.h FLUTE_HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(fcntl.h FLUTE_HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(sys/eventfd.h FLUTE_HAVE_SYS_EVENTFD_H)
CHECK_INCLUDE_FILE(netinet/in.h FLUTE_HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE(netinet/in6.h FLUTE_HAVE_NETINET_IN6_H)
CHECK_INCLUDE_FILE(netinet/tcp.h FLUTE_HAVE_NETINET_TCP_H)
CHECK_INCLUDE_FILE(arpa/inet.h FLUTE_HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(netdb.h FLUTE_HAVE_NETDB_H)
CHECK_INCLUDE_FILE(sys/timerfd.h FLUTE_HAVE_SYS_TIMERFD_H)
CHECK_INCLUDE_FILE(sys/uio.h FLUTE_HAVE_SYS_UIO_H)
CHECK_INCLUDE_FILE(sys/ioctl.h FLUTE_HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILE(poll.h FLUTE_HAVE_POLL_H)
CHECK_INCLUDE_FILE(sys/sendfile.h FLUTE_HAVE_SYS_SENDFILE_H)

# 依赖的方法检测
CHECK_FUNCTION_EXISTS(kqueue FLUTE_HAVE_KQUEUE)
CHECK_FUNCTION_EXISTS(epoll_create FLUTE_HAVE_EPOLL)
CHECK_FUNCTION_EXISTS(epoll_create1 FLUTE_HAVE_EPOLL_CREATE1)
CHECK_FUNCTION_EXISTS(pipe2 FLUTE_HAVE_PIPE2)
CHECK_FUNCTION_EXISTS(pipe FLUTE_HAVE_PIPE)
CHECK_FUNCTION_EXISTS(eventfd FLUTE_HAVE_EVENTFD)
CHECK_FUNCTION_EXISTS(accept4 FLUTE_HAVE_ACCEPT4)
CHECK_FUNCTION_EXISTS(timerfd_create FLUTE_HAVE_TIMERFD_CREATE)

# 检测变量符号等
CHECK_SYMBOL_EXISTS(F_SETFD fcntl.h FLUTE_HAVE_SETFD)

# 配置文件生成
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/flute-config.hpp.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/include/flute/flute-config.hpp
        NEWLINE_STYLE UNIX)

# 设置头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)

file(GLOB_RECURSE HEADERS "flute/*.hpp")
add_custom_target(headers SOURCES ${HEADERS})

set(FLUTE_SOURCES src/flute/src.cpp)

if (UNIX OR APPLE)
set(LIB_PLATFORM pthread)
add_definitions(-Wall)
endif()
if(WIN32)
set(LIB_PLATFORM ws2_32 shell32 advapi32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

set(GNUC 0)
set(CLANG 0)
set(MSVC 0)
if (("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang"))
    set(CLANG 1)
endif()
if (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR (${CLANG}))
    set(GNUC 1)
endif()
if (("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC") OR (${CLANG}))
    set(MSVC 1)
endif()

# 设置动态链接库名称
set(LIB_NAME flute)

if (${LIBS_EXPORT_TYPE} STREQUAL "shared")
set(LIB_FULL_NAME "${LIB_NAME}_shared")
set(LIB_BUILD_TYPE SHARED)
elseif(${LIBS_EXPORT_TYPE} STREQUAL "static")
set(LIB_FULL_NAME "${LIB_NAME}_static")
set(LIB_BUILD_TYPE SHARED)
endif(${LIBS_EXPORT_TYPE} STREQUAL "shared")

if ((${LIBS_EXPORT_TYPE} STREQUAL "shared") OR (${LIBS_EXPORT_TYPE} STREQUAL "static"))
# 编译动态链接库
add_library(${LIB_FULL_NAME} ${LIB_BUILD_TYPE} ${FLUTE_SOURCES})

if(${GNUC})
set_target_properties(${LIB_FULL_NAME} PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
set_target_properties(${LIB_FULL_NAME} PROPERTIES LINK_FLAGS -fvisibility=hidden)
endif()

# 设置依赖选项
target_link_libraries(${LIB_FULL_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        ${LIB_PLATFORM}
        ${LIB_LIBRARIES}
)

# 设置库属性
set_target_properties(${LIB_FULL_NAME} PROPERTIES
        OUTPUT_NAME "${LIB_NAME}"
        CLEAN_DIRECT_OUTPUT 1
)

# 设置动态链接库属性
set_target_properties(${LIB_FULL_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 设置动态链接库版本号
set_target_properties(${LIB_FULL_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)

endif()
cmake_minimum_required(VERSION 3.1)

add_executable(login login/main.cc)
target_link_libraries(login ${LIB_FULL_NAME})
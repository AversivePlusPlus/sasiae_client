cmake_minimum_required(VERSION 3.1)

set(NAME _test)
project(${NAME} CXX C ASM)

set(CMAKE_CXX_STANDARD 11)

include_directories(../include)
link_directories(../build)

find_package(Qt5Core REQUIRED)

add_executable(${NAME} main.cpp)

target_link_libraries(${NAME} Qt5::Core)
target_link_libraries(${NAME} sasiae_client)

cmake_minimum_required(VERSION 3.20)
project(cyshell C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    -fno-exceptions
    -fno-rtti
    -nostdinc++
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(cysh
    src/main.cpp
)

target_link_options(cysh PRIVATE -nodefaultlibs)
target_link_libraries(cysh PRIVATE c)

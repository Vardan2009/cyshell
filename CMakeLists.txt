cmake_minimum_required(VERSION 3.20)
project(cyshell C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    -fno-exceptions
    -fno-rtti
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(cysh
    src/lex.h
    src/lex.cpp

    src/parser.h 
    src/parser.cpp

    src/run.h
    src/run.cpp

    src/error.h 
    src/error.cpp

    src/main.cpp
)

target_compile_options(cysh PRIVATE
    $<$<CONFIG:Debug>:   -O0 -g3 -fsanitize=address,undefined>
    $<$<CONFIG:Release>: -O2 -DNDEBUG>
)

target_link_options(cysh PRIVATE
    $<$<CONFIG:Debug>: -fsanitize=address,undefined>
)

target_link_libraries(cysh PRIVATE 
    $<$<CONFIG:Debug>:asan>
    $<$<CONFIG:Debug>:ubsan>
)

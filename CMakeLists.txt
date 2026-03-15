cmake_minimum_required(VERSION 3.10)
project(cyshell)

add_executable(cysh
	src/main.c
	
	src/lex.h
	src/lex.c

	src/parser.h
	src/parser.c

	src/proc.h
	src/proc.c
)

if(UNIX AND NOT APPLE)
    link_libraries(m)
endif()

cmake_minimum_required(VERSION 3.10)

project(Mython CXX)
set(CMAKE_CXX_STANDARD 17)

set (SRCS
		main.cpp
		lexer.cpp
		runtime.cpp
		statement.cpp
		parse.cpp
		lexer_test_open.cpp
		runtime_test.cpp
		statement_test.cpp
		parse_test.cpp
)

set(HDRS
		lexer.h
		runtime.h
		statement.h
		parse.h
		test_runner_p.h
)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_executable(mython ${SRCS} ${HDRS})
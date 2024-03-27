cmake_minimum_required(VERSION 3.5.0)
project(potato-chess-uci VERSION 0.1.0)

include(CTest)
enable_testing()

# set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --debug -g -Wall -Wextra -pedantic -fsanitize=undefined")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -pedantic -fsanitize=undefined")

add_executable(potato-chess-uci bitboard.cpp book.cpp eval.cpp hash.cpp main.cpp maps.cpp move.cpp movegen.cpp perft.cpp search.cpp timeman.cpp logger.h)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

cmake_minimum_required(VERSION 3.5.0)
project(potato-chess-uci VERSION 0.1.0)

include(CTest)
enable_testing()

# set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --debug -g")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -pedantic")

add_executable(potato-chess-uci bitboard.cpp book.cpp eval.cpp hash.cpp main.cpp maps.cpp move.cpp movegen.cpp perft.cpp search.cpp timeman.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

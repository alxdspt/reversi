cmake_minimum_required(VERSION 3.26)
project(upload)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(upload
        ai.h
        board.h
        main.cpp)

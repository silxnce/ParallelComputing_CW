cmake_minimum_required(VERSION 3.31)
project(ParallelComputing_CW)

set(CMAKE_CXX_STANDARD 20)

add_executable(ParallelComputing_CW src/main.cpp
        include/core/ThreadPool.h
        src/core/ThreadPool.cpp
        include/core/InvertedIndex.h
        src/core/InvertedIndex.cpp)

cmake_minimum_required(VERSION 3.24)
project(TwinWidth)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(TwinWidth main.cpp src/graph.cpp src/modDecomp.cpp src/partitionRefinement.cpp)

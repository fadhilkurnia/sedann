cmake_minimum_required(VERSION 3.0.0)
add_library(lineagetree STATIC 
                lineagetree.cpp 
                node.cpp
                cluster.cpp)
target_include_directories(lineagetree PUBLIC ${CMAKE_HOME_DIRECTORY}/includes)

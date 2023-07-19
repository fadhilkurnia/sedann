cmake_minimum_required(VERSION 3.0.0)

# project description
project(sedann
    VERSION 0.1
    DESCRIPTION "scalable out-of-core approximate nearest neighbor (ANN) search implementation"
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/fadhilkurnia/sedann")

# #############################################################################################
# COMPILER SETUP
# #############################################################################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -Wno-unused-variable")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

message(STATUS "Build mode         : ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_STANDARD : ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_FLAGS    : ${CMAKE_CXX_FLAGS}")


# #############################################################################################
# TARGET SPECIFICATION
# #############################################################################################

set(SOURCES src/lineage_main.cpp) # specify the main target "sedann"
add_executable(sedann ${SOURCES})
# include_directories(includes)

# uncomment the following line for per target include directory
target_include_directories(sedann PUBLIC includes)

# experimentation targets
# add_executable(test_bplustree ./src/bplustree.cpp)            # this require bits/stdc++.h that is not exist in macos
# add_executable(xsedann ./src/main.cpp)                        # this require faiss
# add_executable(test_faiss_flat ./src/test_faiss_flat.cpp)     # this require faiss
# add_executable(test_faiss_graph ./src/test_faiss_graph.cpp)   # this require faiss
# add_executable(tools_get_bvecs_prefix ./src/tools_get_bvecs_prefix.cpp)

# #############################################################################################
# STATIC AND EXTERNAL LIBRARIES
# #############################################################################################

# static library: lineagetree
add_subdirectory(src/lineagetree)
target_link_libraries(sedann lineagetree)

# external libary: boost
find_package(Boost REQUIRED COMPONENTS program_options log)
if(Boost_FOUND)
    message(STATUS "   Boost_INCLUDE_DIRS : ${Boost_INCLUDE_DIRS}")
    message(STATUS "   Boost_LIBRARIES    : ${Boost_LIBRARIES}")
    message(STATUS "   Boost_VERSION      : ${Boost_VERSION}")
#    include_directories(${Boost_INCLUDE_DIRS})
    target_include_directories(sedann INTERFACE ${Boost_INCLUDE_DIRS})
    target_link_libraries(sedann ${Boost_LIBRARIES})
endif()
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

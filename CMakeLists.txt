cmake_minimum_required(VERSION 3.0.0)
project(sedann VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

# Include Boost library
find_package(Boost COMPONENTS program_options REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

# Using threads
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# using -march=native for SIMD support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

include_directories(./include)
add_executable(sedann ./src/main.cpp)
add_executable(test_bplustree ./src/bplustree.cpp)
add_executable(test_faiss_flat ./src/test_faiss_flat.cpp)

if(Boost_FOUND)
    target_link_libraries(sedann ${Boost_LIBRARIES})
endif()

# include faiss library
set(FAISS_ENABLE_GPU OFF)
set(FAISS_ENABLE_PYTHON OFF)
add_subdirectory(./external/faiss)
target_link_libraries(test_faiss_flat faiss_avx2)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

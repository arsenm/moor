cmake_minimum_required (VERSION 2.6)

project(Moor CXX)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra -pedantic")
endif()


find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIR})

add_subdirectory(moor)
add_subdirectory(test)



cmake_minimum_required(VERSION 3.11)

project(scheduler CXX)
set(version 0.1)

set(CMAKE_CXX_COMPILER /usr/bin/g++-7)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -pthread")

include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/src")
include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/support/src")

add_subdirectory(include/dmclock/src)
add_executable(scheduler scheduler_main.cc)
target_link_libraries(scheduler dmclock)

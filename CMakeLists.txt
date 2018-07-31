cmake_minimum_required(VERSION 3.11)

project(scheduler CXX)
set(version 0.1)

set(CMAKE_CXX_COMPILER /usr/bin/g++-7)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -pthread")

include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/src")
include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/support/src")

add_subdirectory(include/dmclock/src)
add_executable(push_pq push_pq_main.cc)
add_executable(pull_pq pull_pq_main.cc)
target_link_libraries(push_pq dmclock)
target_link_libraries(pull_pq dmclock)

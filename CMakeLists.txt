cmake_mimimum_required(VERSION 2.8.12)

project(scheduler CXX)
set(version 0.1)

include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/src")
include_directories("${CMAKE_SOURCE_DIR}/include/dmclock/support/src")

addexecutable(scheduler scheduler_main.cc)

cmake_minimum_required(VERSION 3.13)
Project(MedianOfLines)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(../../include)
link_directories(../../include)
add_subdirectory(../../include include)

add_executable(mrc mrc.cpp)

target_link_libraries(schedular co_task)
target_link_libraries(mrc co_task linearfunctions schedular)

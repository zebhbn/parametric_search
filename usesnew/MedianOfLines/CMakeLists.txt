cmake_minimum_required(VERSION 3.13)
Project(MinimumMeanCycle)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(../../includenew)
link_directories(../../includenew)
add_subdirectory(../../includenew includenew)

#add_library(graphGenerator GraphGenerators.hpp)

add_executable(medianOfLines medianOfLines.cpp)

target_link_libraries(Scheduler CoTask)
target_link_libraries(ComparisonResolver Scheduler CoTask)
target_link_libraries(psQuicksort ComparisonResolver Scheduler CoTask)
target_link_libraries(medianOfLines linearfunctions CoTask ComparisonResolver Scheduler)

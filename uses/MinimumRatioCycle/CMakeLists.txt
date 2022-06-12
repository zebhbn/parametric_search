cmake_minimum_required(VERSION 3.13)
Project(MinimumMeanCycle)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(../../include)
link_directories(../../include)
add_subdirectory(../../include includenew)


add_library(graphGenerator GraphGenerators.hpp)
add_library(PSFloyd PSFloyd.hpp)

add_executable(mrc mrc.cpp)

target_link_libraries(ThreadPool CoTask)
target_link_libraries(Scheduler CoTask)
target_link_libraries(MultiTScheduler CoTask ThreadPool Scheduler)
target_link_libraries(graphGenerator linearfunctions)
target_link_libraries(ComparisonResolver Scheduler CoTask)
target_link_libraries(MultiTComparisonResolver ComparisonResolver Scheduler CoTask)
target_link_libraries(DirectComparisonResolver ComparisonResolver Scheduler CoTask)
target_link_libraries(mrc linearfunctions graphGenerator CoTask ComparisonResolver Scheduler ThreadPool MultiTScheduler MultiTComparisonResolver)
target_link_libraries(PSFloyd linearfunctions graphGenerator CoTask ComparisonResolver Scheduler ThreadPool MultiTScheduler MultiTComparisonResolver)


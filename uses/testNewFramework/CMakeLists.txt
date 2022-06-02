cmake_minimum_required(VERSION 3.13)
Project(TestNewF)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(../../include)
link_directories(../../include)
add_subdirectory(../../include include)

add_executable(bob testNewF.cpp)

target_link_libraries(Scheduler co_task)
target_link_libraries(comparisonResolver Scheduler co_task)
target_link_libraries(bob linearfunctions co_task comparisonResolver schedular)

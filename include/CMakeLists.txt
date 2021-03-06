cmake_minimum_required(VERSION 3.13)
Project(PSFramework)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_library(CoTask CoTask.hpp)
add_library(ThreadPool CoTask ThreadPool.hpp ThreadPool.cpp)
add_library(ps_core PSCore.hpp)
add_library(linearfunctions PSInterfaces.hpp LinearFunction.hpp LinearFunction.cpp)
#add_library(schedular co_task schedular.hpp)
add_library(Scheduler CoTask Scheduler.hpp)
add_library(MultiTScheduler CoTask Scheduler ThreadPool MultiTScheduler.hpp)
add_library(ComparisonResolver Scheduler CoTask ComparisonResolver.hpp)
add_library(MultiTComparisonResolver ComparisonResolver Scheduler CoTask MultiTComparisonResolver.hpp)
#add_library(DirectComparisonResolver ComparisonResolver Scheduler CoTask DirectComparisonResolver.hpp)
add_library(PSQuicksort ComparisonResolver Scheduler CoTask PSQuicksort.hpp)
#add_library(multiTQuicksort PSQuicksort ComparisonResolver Scheduler CoTask MultiTQuicksort.hpp)
#add_library(imp_scheduler co_task improved_scheduler.hpp)
#add_library(threadpool co_task ThreadPool.hpp ThreadPool.cpp)
#add_library(ps_quicksort CoTask schedular ps_quicksort.hpp)
#add_library(multischedular ThreadPool.cpp ThreadPool.hpp MultiTSchedular.hpp)
#add_library(multischedular MultiTSchedular.hpp)
set_target_properties(ps_core PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(MultiTScheduler PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(PSQuicksort PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(multiTQuicksort PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(Scheduler PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(MultiTComparisonResolver PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ComparisonResolver PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(DirectComparisonResolver PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(linearfunctions PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(schedular PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(imp_scheduler PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(CoTask PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(threadpool PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(multischedular PROPERTIES LINKER_LANGUAGE CXX)
#set_target_properties(ps_quicksort PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(multischedular threadpool)

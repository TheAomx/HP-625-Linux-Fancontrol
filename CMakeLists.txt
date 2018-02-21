cmake_minimum_required(VERSION 2.6)
project(FanControl)


#add_definitions(-DDEBUG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Os")
set(SOURCE_FILES main.cpp)
add_executable(FanControl ${SOURCE_FILES})

install (TARGETS FanControl RUNTIME DESTINATION /opt/)


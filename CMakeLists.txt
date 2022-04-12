cmake_minimum_required(VERSION 3.15)
project(trip)

set(CMAKE_CXX_STANDARD 17)

link_libraries(crypto++)
link_libraries(pthread)

add_executable(main main.cpp)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -g")

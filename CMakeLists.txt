cmake_minimum_required(VERSION 3.2)

project(Bryx)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES src/*.h src/*.cpp)
add_executable(bryx ${SOURCES})

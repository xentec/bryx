cmake_minimum_required(VERSION 3.0)

project(Bryx C++)

file(GLOB_RECURSE SOURCES src/*.h src/*.cpp)
add_executable(bryx ${SOURCES})

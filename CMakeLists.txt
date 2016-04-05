cmake_minimum_required(VERSION 3.2)

project(Bryx)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-stdlib=libc++")
endif()

add_subdirectory(extern/cppformat)

list(APPEND LIBS
	cppformat
)

if(NOT WIN32)
	list(APPEND LIBS c++ c++abi m c gcc_s gcc)
endif()

file(GLOB_RECURSE SOURCES src/*.h src/*.cpp)

add_executable(bryx ${SOURCES})
target_link_libraries(bryx ${LIBS})

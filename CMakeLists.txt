cmake_minimum_required(VERSION 2.8.12)

project(Bryx)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

find_package(Threads REQUIRED)

add_subdirectory(extern/fmt)

list(APPEND LIBS
	fmt
	${CMAKE_THREAD_LIBS_INIT}
)

if(WIN32)
	list(APPEND LIBS ws2_32)
endif()

include_directories(src)
file(GLOB_RECURSE SOURCES src/*.h src/*.cpp)

add_executable(bryx ${SOURCES})
target_link_libraries(bryx ${LIBS})

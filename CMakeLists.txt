cmake_minimum_required(VERSION 2.8.12)

project(Bryx)

set(USE_LIBCXX 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if(${USE_LIBCXX})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall")
endif()

add_subdirectory(extern/fmt)

list(APPEND LIBS
	fmt
)

if(${USE_LIBCXX})
	list(APPEND LIBS c++ c++abi m c gcc_s gcc)
endif()


file(GLOB_RECURSE SOURCES src/*.h src/*.cpp)

add_executable(bryx ${SOURCES})
target_link_libraries(bryx ${LIBS})

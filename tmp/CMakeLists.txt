cmake_minimum_required (VERSION 3.8)
project (CQLDriverTmp)

include(FindPkgConfig)

# add subdirectory
add_subdirectory(../src CQLDriver)

# add target and source files
FILE(GLOB_RECURSE Files ./*.cpp)
FILE(GLOB_RECURSE PublicHeaders ../include/*.hpp)
FILE(GLOB_RECURSE InternalHeaders ../src/*.hpp)
add_executable(${PROJECT_NAME} ${Files} ${PublicHeaders} ${InternalHeaders})

# find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(SEASTAR REQUIRED seastar)
pkg_check_modules(SEASTAR_DEBUG REQUIRED seastar-debug)

# set compile options
set(CMAKE_VERBOSE_MAKEFILE TRUE)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE
	../include ../src ./)
target_compile_options(${PROJECT_NAME} PRIVATE
	-Wall -Wextra
	-Wno-unused-variable -Wno-unused-function)

# set compile options dependent on build type
if (CMAKE_BUILD_TYPE MATCHES Release OR
	CMAKE_BUILD_TYPE MATCHES RelWithDebInfo OR
	CMAKE_BUILD_TYPE MATCHES MinSizeRel)
	target_compile_options(${PROJECT_NAME} PRIVATE
		${SEASTAR_CFLAGS})
	target_link_libraries(${PROJECT_NAME} PRIVATE
		${SEASTAR_LDFLAGS} CQLDriver)
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_options(${PROJECT_NAME} PRIVATE
		${SEASTAR_DEBUG_CFLAGS})
	target_link_libraries(${PROJECT_NAME} PRIVATE
		asan ubsan ${SEASTAR_DEBUG_LDFLAGS} CQLDriver)
endif()


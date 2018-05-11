cmake_minimum_required (VERSION 2.8.11)
project (CPVFrameworkTests)

FILE(GLOB_RECURSE Files ./*.cpp)
FILE(GLOB_RECURSE PublicHeaders ../include/*.hpp)
FILE(GLOB_RECURSE InternalHeaders ../src/*.hpp)
add_executable(${PROJECT_NAME} ${Files} ${PublicHeaders} ${InternalHeaders})

# set compile options, libCPVFramework should specific as a path for modification detect
set(CMAKE_VERBOSE_MAKEFILE TRUE)
add_subdirectory(../3rd-party/googletest googletest EXCLUDE_FROM_ALL)
include_directories(../src ../tests)
get_filename_component(TOP_PATH ${CMAKE_SOURCE_DIR} PATH)
target_link_libraries(${PROJECT_NAME} gtest_main ${TOP_PATH}/bin/debug/libCPVFramework.a)

# read version number
file(READ ../VERSION VERSION_NUMBER)
string(STRIP "${VERSION_NUMBER}" VERSION_NUMBER)

# add predefined macros
add_definitions(-DHTTPD_LISTEN_IP="127.0.0.1")
add_definitions(-DHTTPD_LISTEN_PORT=8080)
add_definitions(-DCPV_FRAMEWORK_VERSION_NUMBER="${VERSION_NUMBER}")

# get flags from cpv framework pkgconfig
set(CPVFRAMEWORK_PC_PATH ${TOP_PATH}/bin/debug/cpvframework.pc)
execute_process(COMMAND pkg-config --cflags ${CPVFRAMEWORK_PC_PATH} OUTPUT_VARIABLE CPVFRAMEWORK_CFLAGS)
execute_process(COMMAND pkg-config --libs ${CPVFRAMEWORK_PC_PATH} OUTPUT_VARIABLE CPVFRAMEWORK_LDFLAGS)
string(STRIP "${CPVFRAMEWORK_CFLAGS}" CPVFRAMEWORK_CFLAGS)
string(STRIP "${CPVFRAMEWORK_LDFLAGS}" CPVFRAMEWORK_LDFLAGS)

# set compiler and link flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPVFRAMEWORK_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CPVFRAMEWORK_LDFLAGS}")


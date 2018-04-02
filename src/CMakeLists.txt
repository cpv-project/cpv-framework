cmake_minimum_required (VERSION 2.8.11)
project (CPVFramework)

FILE(GLOB_RECURSE Files ./*.cpp)
FILE(GLOB_RECURSE PublicHeaders ../include/*.hpp)
FILE(GLOB_RECURSE InternalHeaders ../src/*.hpp)
add_library(${PROJECT_NAME} STATIC ${Files} ${PublicHeaders} ${InternalHeaders})

# set compile options
set(CMAKE_VERBOSE_MAKEFILE TRUE)
target_link_libraries(${PROJECT_NAME})
get_filename_component(TOP_PATH ${CMAKE_SOURCE_DIR} PATH)

# get flags from seastar pkgconfig
if (CMAKE_BUILD_TYPE MATCHES Debug)
	set(SEASTAR_PC_PATH ${TOP_PATH}/3rd-party/seastar/build/debug/seastar.pc)
else()
	set(SEASTAR_PC_PATH ${TOP_PATH}/3rd-party/seastar/build/release/seastar.pc)
endif()
execute_process(COMMAND pkg-config --cflags ${SEASTAR_PC_PATH} OUTPUT_VARIABLE SEASTAR_CFLAGS)
execute_process(COMMAND pkg-config --libs ${SEASTAR_PC_PATH} OUTPUT_VARIABLE SEASTAR_LDFLAGS)
string(STRIP "${SEASTAR_CFLAGS}" SEASTAR_CFLAGS)
string(STRIP "${SEASTAR_LDFLAGS}" SEASTAR_LDFLAGS)
string(REPLACE "-Werror" "" SEASTAR_CFLAGS "${SEASTAR_CFLAGS}")
string(REPLACE "-Werror" "" SEASTAR_LDFLAGS "${SEASTAR_LDFLAGS}")

# set compiler and link flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-function -Wno-noexcept-type ${SEASTAR_CFLAGS} -I'${TOP_PATH}/include' -I'${TOP_PATH}/3rd-party/json/include'")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -fuse-ld=gold ${SEASTAR_LDFLAGS}")
if (CMAKE_BUILD_TYPE MATCHES Release)
	string(REPLACE "-O3" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-O2" "-O3 -march=native -mtune=native" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# generate pkgconfig file for cpv framework
file(READ ../VERSION VERSION_NUMBER)
string(STRIP "${VERSION_NUMBER}" VERSION_NUMBER)
string(TIMESTAMP BUILD_DATE)
set(PC_PATH "${CMAKE_BINARY_DIR}/cpvframework.pc")
file(REMOVE ${PC_PATH})
file(APPEND ${PC_PATH} "Name: CPVFramework\n")
file(APPEND ${PC_PATH} "URL: https://github.com/cpv-project/cpv-framework\n")
file(APPEND ${PC_PATH} "Description: C++ web framework, build date: ${BUILD_DATE}\n")
file(APPEND ${PC_PATH} "Version: ${VERSION_NUMBER}\n")
file(APPEND ${PC_PATH} "Cflags: ${CMAKE_CXX_FLAGS}\n")
file(APPEND ${PC_PATH} "Libs: ${CMAKE_SHARED_LINKER_FLAGS} -L'${CMAKE_BINARY_DIR}' -lCPVFramework\n")


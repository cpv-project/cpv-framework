cmake_minimum_required (VERSION 3.8)
project (CPVFramework)

include(FindPkgConfig)
include(GNUInstallDirs)

# add target and source files
FILE(GLOB_RECURSE Files ./*.cpp)
FILE(GLOB_RECURSE PublicHeaders ../include/*.hpp)
FILE(GLOB_RECURSE InternalHeaders ../src/*.hpp)
add_library(${PROJECT_NAME} SHARED ${Files} ${PublicHeaders} ${InternalHeaders})

# find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(SEASTAR REQUIRED seastar)
pkg_check_modules(SEASTAR_DEBUG REQUIRED seastar-debug)

# set compile options
set(CMAKE_VERBOSE_MAKEFILE TRUE)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE ../include)
target_compile_options(${PROJECT_NAME} PRIVATE
	-Wall -Wextra
	-Wno-unused-variable -Wno-unused-function)
target_link_libraries(${PROJECT_NAME} PRIVATE http_parser)

# set compile options dependent on build type
if (CMAKE_BUILD_TYPE MATCHES Release OR
	CMAKE_BUILD_TYPE MATCHES RelWithDebInfo OR
	CMAKE_BUILD_TYPE MATCHES MinSizeRel)
	target_compile_options(${PROJECT_NAME} PRIVATE
		${SEASTAR_CFLAGS} -march=native -mtune=native)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		${SEASTAR_LDFLAGS} -flto -fuse-ld=gold)
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_options(${PROJECT_NAME} PRIVATE
		${SEASTAR_DEBUG_CFLAGS})
	target_link_libraries(${PROJECT_NAME} PRIVATE
		${SEASTAR_DEBUG_LDFLAGS})
endif()

# generate pkgconfig file for cpvframework
execute_process(COMMAND
	sh ${CMAKE_SOURCE_DIR}/../debian/scripts/get_version.sh
	OUTPUT_VARIABLE VERSION_NUMBER
	OUTPUT_STRIP_TRAILING_WHITESPACE)
string(TIMESTAMP BUILD_DATE)
set(PC_PATH "${CMAKE_CURRENT_BINARY_DIR}/cpvframework.pc")
file(REMOVE ${PC_PATH})
file(APPEND ${PC_PATH} "Name: CPVFramework\n")
file(APPEND ${PC_PATH} "URL: https://github.com/cpv-project/cpv-framework\n")
file(APPEND ${PC_PATH} "Description: C++ web framework, build date: ${BUILD_DATE}\n")
file(APPEND ${PC_PATH} "Version: ${VERSION_NUMBER}\n")
file(APPEND ${PC_PATH} "Cflags:\n")
file(APPEND ${PC_PATH} "Libs: -lCPVFramework\n")

# add install rules
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ../include/CPVFramework DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${PC_PATH} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ../LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/CPVFramework)

# add predefined macros
add_definitions(-DCPV_FRAMEWORK_VERSION_NUMBER="${VERSION_NUMBER}")


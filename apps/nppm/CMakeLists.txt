#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.10)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# next line repeated found
# find_package(OpenCV REQUIRED)

# Enable C++11
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

set( name nppm-demo )
project(${name})
###################################################
#find_package(fmt)
#find_package(nlohmann_json)
###################################################
# generate an object library from sources
#add_library(nppm-dev-objs
#		OBJECT
#			../../dev/configdata.h
#			../../dev/configdata.cpp
#			../../dev/x_moildev.h
#			../../dev/moildev.cpp
#		)
# this is only needed for older compilers
# but doesn't hurt either to have it
#set_target_properties(nppm-dev-objs
#		PROPERTIES
#		POSITION_INDEPENDENT_CODE 1
#		)
#
#add_library(nppm-dev-shared
#		SHARED
#		$<TARGET_OBJECTS:nppm-dev-objs>
#		)
#set_target_properties(nppm-dev-shared
#		PROPERTIES
#		OUTPUT_NAME "nppm-dev"
#		)
#
#add_library(nppm-dev-static
#		STATIC
#		$<TARGET_OBJECTS:nppm-dev-objs>
#		)
#set_target_properties(nppm-dev-static
#		PROPERTIES
#		OUTPUT_NAME "nppm-dev"
#		)
########################################################

# Print this project name
message(STATUS "Project name: ${name}" )

#application specific @cj
find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})

# Default to the local OpenCV package on this machine, but still allow
# callers to override it from the CMake cache or command line.
if(NOT OpenCV_DIR)
    set(OpenCV_DIR "$ENV{HOME}/cv-2026/lib/cmake/opencv4" CACHE PATH "Directory containing OpenCVConfig.cmake")
else()
    set(OpenCV_DIR "${OpenCV_DIR}" CACHE PATH "Directory containing OpenCVConfig.cmake")
endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
#find_package(Boost 1.54 REQUIRED COMPONENTS filesystem)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message( STATUS "         OpenCV_DIR: ${OpenCV_DIR}" )
message( STATUS "     OpenCV_VERSION: ${OpenCV_VERSION}" )
message( STATUS "        OpenCV_LIBS: ${OpenCV_LIBS}" )
message( STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}" )

message( STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}" )
## Specify the include directories
#target_include_directories(x_mainmoil PRIVATE
#		${CMAKE_SOURCE_DIR}/jason/include
#)


#include_directories("/home/cj/Dropbox/19-22-ing/er/cj/r-208-install/i7/usr/local/include")
#link_directories("/home/cj/Dropbox/19-22-ing/er/cj/r-208-install/i7/usr/local/lib")
#find_package(dlib REQUIRED)

set(APPS
#       fish.cpp
			nppm.cpp
		cast.cpp
        recipe_legacy_loader.cpp
        recipe_resolver.cpp
        recipe_validator.cpp
        recipe_yaml.cpp
        tools.cpp
		../../dev/moildev.cpp
		../../dev/configdata.cpp
		)

add_executable(x_mainmoil main.cpp ${APPS})

#target_link_libraries(x_mainmoil ${OpenCV_LIBS})
##########################################################
# Link Library:
#target_link_libraries(x_mainmoil fmt::fmt)
#target_link_libraries(x_mainmoil nlohmann_json::nlohmann_json)
# C++ Version:
target_compile_features(x_mainmoil PUBLIC cxx_std_17)
##########################################################
#05/21/2023 set(LIBS ${OpenCV_LIBS} nppm-dev-static)
set(LIBS ${OpenCV_LIBS} ${X11_LIBRARIES})
target_link_libraries(x_mainmoil ${LIBS} )

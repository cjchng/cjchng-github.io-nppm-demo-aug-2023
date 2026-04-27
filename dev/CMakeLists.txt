# set minimum cmake version
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# project name and language
project(nppm-dev LANGUAGES CXX)

# generate an object library from sources
add_library(nppm-dev-objs
  OBJECT
        configdata.h
        configdata.cpp
        x_moildev.h
        moildev.cpp
  )

# this is only needed for older compilers
# but doesn't hurt either to have it
set_target_properties(nppm-dev-objs
  PROPERTIES
    POSITION_INDEPENDENT_CODE 1
  )

add_library(nppm-dev-shared
  SHARED
    $<TARGET_OBJECTS:nppm-dev-objs>
  )
set_target_properties(nppm-dev-shared
  PROPERTIES
    OUTPUT_NAME "nppm-dev"
  )

add_library(nppm-dev-static
  STATIC
    $<TARGET_OBJECTS:nppm-dev-objs>
  )
set_target_properties(nppm-dev-static
  PROPERTIES
    OUTPUT_NAME "nppm-dev"
  )

add_executable(dev-test dev-test.cpp)

#05/21/2023 target_link_libraries(dev-test nppm-dev-shared)
target_link_libraries(dev-test nppm-dev-static)
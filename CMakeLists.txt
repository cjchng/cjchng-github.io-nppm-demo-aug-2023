#/home/cj/CLionProjects/nppm-app/CMakeLists.txt

cmake_minimum_required(VERSION 3.5)
project(chapter3)

set(CMAKE_CXX_FLAGS "-std=c++14")

# 加入各子工程
#Next line will sideeffect module(apps/nppm), and has to comment out
add_subdirectory(dev)
add_subdirectory(apps/nppm)

#add_subdirectory(try-pybind11)
#add_subdirectory(examples)

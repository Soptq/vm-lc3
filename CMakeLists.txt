cmake_minimum_required(VERSION 3.15)
project(vm-lc3)
set(CMAKE_CXX_STANDARD 14)
set(SOURCE_FILES main.c)

add_executable(vm-lc3 ${SOURCE_FILES})
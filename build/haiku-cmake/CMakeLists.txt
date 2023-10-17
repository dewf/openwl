cmake_minimum_required(VERSION 3.0)

project(openwl)

add_library(openwl SHARED
    ../../source/openwl.h
    ../../source/haiku/openwl.cpp)

target_compile_definitions(openwl PRIVATE __HAIKU_OS__)

install(TARGETS openwl RUNTIME DESTINATION bin)

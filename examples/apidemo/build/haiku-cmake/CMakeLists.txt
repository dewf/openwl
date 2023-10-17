cmake_minimum_required(VERSION 3.0)

project(apidemo)

set(CMAKE_CXX_STANDARD 11)

link_directories("/develop/lib")
link_libraries(be translation media tracker)

add_executable(apidemo ../../source/main.h ../../source/main.cpp ../../source/haiku-beapi/haiku_specific.cpp)

target_compile_definitions(apidemo PRIVATE __HAIKU_OS__)

add_subdirectory("../../../../build/haiku-cmake/" "${CMAKE_CURRENT_BINARY_DIR}/openwl")

target_link_libraries(apidemo "openwl") # pthread


install(TARGETS apidemo RUNTIME DESTINATION bin)


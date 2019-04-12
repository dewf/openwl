cmake_minimum_required(VERSION 3.8)
project(c_client)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake") # for FindCairo.cmake

set(CMAKE_CXX_STANDARD 17)

# linux stuff here
find_package(Cairo REQUIRED)
find_package(X11 REQUIRED)

include_directories(${CAIRO_INCLUDE_DIRS} ${X11_INCLUDE_DIR}) #"../../"

set(SOURCE_FILES
        "../../source/unicodestuff.h"
        "../../source/main.cpp"
        "../../source/linux-cairo/linux_specific.cpp")

add_executable(c_client ${SOURCE_FILES})

add_subdirectory("../../../../build/clion/" "${CMAKE_CURRENT_BINARY_DIR}/openwl")

target_link_libraries(c_client "openwl" pthread ${CAIRO_LIBRARIES} ${X11_LIBRARIES}) # "cairo"


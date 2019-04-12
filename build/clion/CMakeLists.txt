cmake_minimum_required(VERSION 3.8)
project(openwl)

set(CMAKE_CXX_STANDARD 11)

Set(GCC_VISIBILITY_FLAGS "-fvisibility=hidden")
add_definitions(${GCC_VISIBILITY_FLAGS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)

find_package(GTK2 2.24 REQUIRED gtk gtkmm)
include_directories("../../source/" ${GTK2_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS})
add_definitions(${GTK2_DEFINITIONS})

set(SOURCE_FILES
        "../../source/linux-gtkmm2/openwl.cpp"
        "../../source/linux-gtkmm2/unicodestuff.h"
        "../../source/linux-gtkmm2/boost_stuff.h"
        "../../source/linux-gtkmm2/boost_stuff.cpp"
        "../../source/linux-gtkmm2/globals.cpp"
        "../../source/linux-gtkmm2/globals.h"
        "../../source/linux-gtkmm2/util.cpp"
        "../../source/linux-gtkmm2/util.h"
        "../../source/linux-gtkmm2/private_defs.cpp"
        "../../source/linux-gtkmm2/private_defs.h"
        "../../source/linux-gtkmm2/_wlWindow.cpp"
        "../../source/linux-gtkmm2/_wlWindow.h"
        "../../source/linux-gtkmm2/keystuff.cpp"
        "../../source/linux-gtkmm2/keystuff.h")

add_library(openwl SHARED ${SOURCE_FILES})
target_link_libraries(openwl ${GTK2_LIBRARIES} ${GTKMM_LIBRARIES})
set(GCC_VISIBILITY_FLAGS "-fvisibility=hidden")
add_definitions(${GCC_VISIBILITY_FLAGS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)

find_package(GTK2 2.24 REQUIRED gtk gtkmm)
include_directories(${GTK2_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
link_directories(${GTK2_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS})
add_definitions(${GTK2_DEFINITIONS})

set(SOURCE_FILES
        "linux-gtkmm2/openwl.cpp"
        "linux-gtkmm2/unicodestuff.h"
        "linux-gtkmm2/boost_stuff.h"
        "linux-gtkmm2/boost_stuff.cpp"
        "linux-gtkmm2/globals.cpp"
        "linux-gtkmm2/globals.h"
        "linux-gtkmm2/util.cpp"
        "linux-gtkmm2/util.h"
        "linux-gtkmm2/private_defs.cpp"
        "linux-gtkmm2/private_defs.h"
        "linux-gtkmm2/wlWindow.cpp"
        "linux-gtkmm2/wlWindow.h"
        "linux-gtkmm2/keystuff.cpp"
        "linux-gtkmm2/keystuff.h")

add_library(OpenWL SHARED ${SOURCE_FILES})
target_link_libraries(OpenWL ${GTK2_LIBRARIES} ${GTKMM_LIBRARIES})

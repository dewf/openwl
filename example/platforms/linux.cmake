# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake") # for FindCairo.cmake
# set(CMAKE_CXX_STANDARD 17)

# linux stuff here
# find_package(Cairo REQUIRED)
find_package(X11 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

include_directories(${CAIRO_INCLUDE_DIRS} ${X11_INCLUDE_DIR}) #"../../"

set(SOURCE_FILES
        "unicodestuff.h"
        "main.cpp"
        "linux-cairo/linux_specific.cpp")

add_executable(example ${SOURCE_FILES})

target_link_libraries(example OpenWL pthread ${CAIRO_LIBRARIES} ${X11_LIBRARIES}) # "cairo"

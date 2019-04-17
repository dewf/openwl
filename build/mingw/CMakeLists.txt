cmake_minimum_required(VERSION 3.8)
project(openwl)

set(CMAKE_CXX_STANDARD 11)

Set(GCC_VISIBILITY_FLAGS "-fvisibility=hidden")
add_definitions(${GCC_VISIBILITY_FLAGS})

add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

set(SOURCE_FILES
        ../../source/win32/dragdrop/dataobject.cpp
        ../../source/win32/dragdrop/dataobject.h
        ../../source/win32/dragdrop/dropsource.cpp
        ../../source/win32/dragdrop/dropsource.h
        ../../source/win32/dragdrop/droptarget.cpp
        ../../source/win32/dragdrop/droptarget.h
        ../../source/win32/dragdrop/enumformat.cpp
        ../../source/win32/dragdrop/enumformat.h
        ../../source/win32/dragdrop/oledragdrop.cpp
        ../../source/win32/dragdrop/oledragdrop.h
        ../../source/win32/dllmain.cpp
        ../../source/win32/globals.cpp
        ../../source/win32/globals.h
        ../../source/win32/keystuff.cpp
        ../../source/win32/keystuff.h
        ../../source/win32/MyDataObject.cpp
        ../../source/win32/MyDataObject.h
        ../../source/win32/MyDropTarget.cpp
        ../../source/win32/MyDropTarget.h
        ../../source/win32/OpenWL.cpp
        ../../source/win32/pngloader.cpp
        ../../source/win32/pngloader.h
        ../../source/win32/private_defs.cpp
        ../../source/win32/private_defs.h
        ../../source/win32/resource.h
        ../../source/win32/targetver.h
        ../../source/win32/unicodestuff.cpp
        ../../source/win32/unicodestuff.h
        ../../source/win32/wndproc.cpp
        ../../source/win32/wndproc.h)

add_library(openwl SHARED ${SOURCE_FILES})
target_link_libraries(openwl d2d1 windowscodecs boost_locale-mt icuuc)

#d2d1.lib
#windowscodecs.lib

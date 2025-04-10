add_compile_definitions(OPENWL_EXPORTS)
add_compile_definitions(UNICODE _UNICODE)

set(SOURCE_FILES
    win32/menustuff.cpp
    win32/cursor.cpp
    win32/dllmain.cpp
    win32/dragdrop/dataobject.cpp
    win32/dragdrop/dropsource.cpp
    win32/dragdrop/droptarget.cpp
    win32/dragdrop/enumformat.cpp
    win32/dragdrop/oledragdrop.cpp
    win32/globals.cpp
    win32/keystuff.cpp
    win32/dialogs.cpp
    win32/MyDataObject.cpp
    win32/MyDropTarget.cpp
    win32/openwl.cpp
    win32/pngloader.cpp
    win32/private_defs.cpp
    win32/timer.cpp
    win32/unicodestuff.cpp
    win32/win32util.cpp
    win32/window.cpp
    win32/wndproc.cpp
)

add_library(OpenWL SHARED ${SOURCE_FILES})
target_link_libraries(OpenWL d2d1 shcore windowscodecs)

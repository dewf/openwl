cmake_minimum_required(VERSION 3.8)
project(c_client)

set(CMAKE_CXX_STANDARD 11)

add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

set(SOURCE_FILES
        "../../source/unicodestuff.h"
        "../../source/main.cpp"
        "../../source/win32-gdi/win32_specific.cpp")

add_executable(c_client ${SOURCE_FILES})

add_subdirectory("../../../../build/mingw/" "${CMAKE_CURRENT_BINARY_DIR}/openwl")

add_link_options("LINKER:/subsystem:console")

add_custom_command(TARGET c_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/openwl/libopenwl.dll"
        "${CMAKE_CURRENT_BINARY_DIR}")
        #$<TARGET_FILE_DIR:MyTest>)

target_link_libraries(c_client openwl pthread gdiplus)

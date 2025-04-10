cmake_minimum_required(VERSION 3.22)
project(example)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory("../library" "${CMAKE_BINARY_DIR}/library_build")

if(WIN32)
    if(MSVC)
        message(STATUS "Configuring for Windows (MSVC)")
        include(platforms/windows.cmake)
    else()
        message(FATAL_ERROR "Unsupported Windows compiler")
    endif()
elseif(APPLE)
    message(STATUS "Configuring for macOS")
    include(platforms/macos.cmake)
elseif(UNIX)
    message(STATUS "Configuring for Linux")
    include(platforms/linux.cmake)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if(APPLE)
    # no icons, included in bundle ... supposedly
else()
    # Add your icons folder copy step after the build
    add_custom_command(TARGET example POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/_icons"
            "$<TARGET_FILE_DIR:example>/_icons"
            COMMENT "Copying _icons folder to output directory")
endif()

add_compile_definitions(UNICODE _UNICODE)

set(SOURCE_FILES
        "unicodestuff.h"
        "main.cpp"
        "win32-gdi/win32_specific.cpp")

add_executable(example ${SOURCE_FILES})
target_link_libraries(example PRIVATE OpenWL gdiplus)

# Copy DLL after build (MSVC only)
add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:OpenWL>"
        "$<TARGET_FILE_DIR:example>"
    COMMENT "Copying OpenWL.dll to output directory"
)

set(APP_SOURCES
    main.cpp
    osx-quartz/osx_specific.cpp
)

set(APP_RESOURCES
    # Images.xcassets
    # Base.lproj/MainMenu.xib
    "_icons" # Add the icons folder path relative to this CMakeLists.txt
)

add_executable(example MACOSX_BUNDLE ${APP_SOURCES})

set_target_properties(example PROPERTIES
        MACOSX_BUNDLE_RESOURCES "${APP_RESOURCES}"
    )

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum macOS version")

set_target_properties(example PROPERTIES
    OBJC_ARC_ENABLED OFF
    OBJCXX_ARC_ENABLED OFF
)

# # C++ Standard (from Xcode settings: gnu++0x -> C++11 with extensions)
# set_target_properties(example PROPERTIES
#     CXX_STANDARD 11
#     CXX_STANDARD_REQUIRED YES
#     CXX_EXTENSIONS YES
# )

# # Info.plist for the Bundle (from Xcode settings)
# # Assumes Info.plist is in the same directory as this CMakeLists.txt
# set_target_properties(example PROPERTIES
#     MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
# )
# # Note: CMake will substitute variables like ${PRODUCT_NAME} in the Info.plist

# Bundle Identifier (derived from Xcode settings)
set_target_properties(example PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.whatever.example"
)
# MACOSX_BUNDLE_ICON_FILE can be set if using a single .icns file instead of Assets.xcassets

# Ensure RPATH is set correctly to find frameworks inside the bundle
set_target_properties(example PROPERTIES
    MACOSX_RPATH TRUE
)

# --- Link Frameworks ---
find_library(AppKit_framework AppKit)
find_library(Foundation_framework Foundation)
find_library(CoreText_framework CoreText)
find_library(CoreFoundation_framework CoreFoundation)
target_link_libraries(example PRIVATE 
    OpenWL 
    ${AppKit_framework} 
    ${Foundation_framework} 
    ${CoreText_framework} 
    ${CoreFoundation_framework})

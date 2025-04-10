set(OPENWL_SOURCES
    osx-cocoa/MainThreadExecutor.mm
    osx-cocoa/WLAppDelegate.mm
    osx-cocoa/miscutil.mm
    osx-cocoa/NSCustomWindow.m
    osx-cocoa/WLWindowObject.mm
    osx-cocoa/globals.mm
    osx-cocoa/keystuff.mm
    osx-cocoa/WLContentView.mm
    osx-cocoa/openwl.mm
    osx-cocoa/dialogs.mm
    osx-cocoa/private_defs.mm
)

add_library(OpenWL SHARED ${OPENWL_SOURCES})

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.6" CACHE STRING "Minimum macOS version")

set_target_properties(OpenWL PROPERTIES OBJC_ARC_ENABLED OFF)
set_target_properties(OpenWL PROPERTIES OBJCXX_ARC_ENABLED OFF)

# # C++ Standard (from Xcode settings)
# set_target_properties(OpenWL PROPERTIES
#     CXX_STANDARD 14
#     CXX_STANDARD_REQUIRED YES
#     CXX_EXTENSIONS YES # For gnu++14
# )

set_target_properties(OpenWL PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    OBJC_VISIBILITY_PRESET hidden
    OBJCXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINED_HIDDEN YES
)
# If specific symbols need exporting, use __attribute__((visibility("default"))) in code.

set_target_properties(OpenWL PROPERTIES
    FRAMEWORK TRUE
    MACOSX_FRAMEWORK_IDENTIFIER "org.whatever.OpenWL" # From PRODUCT_BUNDLE_IDENTIFIER
    # DEFINES_MODULE TRUE # FRAMEWORK TRUE often handles this implicitly
    MACOSX_RPATH TRUE # Embed rpath for easier linking
)

set_target_properties(OpenWL PROPERTIES
    PUBLIC_HEADER "openwl.h"
)

target_include_directories(OpenWL
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        # When installed, the headers are inside the framework bundle
        $<INSTALL_INTERFACE:Headers>
    # PRIVATE
    #     ${CMAKE_CURRENT_SOURCE_DIR}/osx-cocoa
)

find_library(AppKit_framework AppKit)
find_library(Foundation_framework Foundation)
target_link_libraries(OpenWL PRIVATE ${AppKit_framework} ${Foundation_framework})

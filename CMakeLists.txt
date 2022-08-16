cmake_minimum_required(VERSION 3.21)

project(RealTime3D LANGUAGES CXX VERSION 2.0.3.1)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif ()

configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/src/config.h.in"
        "${CMAKE_CURRENT_LIST_DIR}/src/config.h"
)


# Add our cmake module path so we don't need relative paths for these
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

#include(CMakeSetCompilerOptions)
include(QtHelpers)
include(SrcHelpers)
# Load python setup
include(SetupPython)

# Default debug suffix for libraries.
set(CMAKE_DEBUG_POSTFIX "d")

# Load advanced scripts
#include(CMakeInstalling)
#include(Install)

# Add external libraries
#include(CMakeExternalLibs)

# Internal libs used
add_subdirectory(libs)

# external libraries
add_subdirectory(extern)

# main program
add_subdirectory(src)

# applications
add_subdirectory(apps)


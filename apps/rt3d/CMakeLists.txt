
# find lensfun packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(lensfun REQUIRED IMPORTED_TARGET lensfun)
pkg_get_variable(lensfun_BIN_DIR lensfun bindir)

# find OpenCV
#find_package(OpenCV REQUIRED)

# find QT
add_definitions(-DQT_NO_KEYWORDS -DQT_USE_QSTRINGBUILDER)
find_package(Qt5 COMPONENTS
        Core
        Svg
        Widgets
        GUI
        Concurrent
        MultimediaWidgets
        Axcontainer
        Axserver
        REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(rt3d WIN32 ${SRC_SRC} main.cpp)

set(install_DIR rt3d_${RealTime3D_VERSION})
set(install_PATH ${CMAKE_INSTALL_PREFIX}/${install_DIR}/rt3d${CMAKE_EXECUTABLE_SUFFIX})
cmake_path(GET install_PATH PARENT_PATH install_DIR_PATH)

cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH COMPILER_DIR)
find_file (intl8_LIB "libintl-8.dll" PATHS ${COMPILER_DIR} REQUIRED)
find_file (pcre_LIB "libpcre-1.dll" PATHS ${COMPILER_DIR} REQUIRED)
find_file(iconv_LIB "libiconv-2.dll" PATHS ${COMPILER_DIR} REQUIRED)

message("|\t Install dir: ${install_DIR}")
message("|\t Install path: ${install_PATH}")
message("|\t Install dir path: ${install_DIR_PATH}")

install(TARGETS rt3d
        DESTINATION ${install_DIR})

target_compile_features(rt3d PUBLIC cxx_std_20)
set_target_properties(rt3d PROPERTIES
        CMAKE_CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )

set(COMPILE_DEFINITIONS $<$<CONFIG:DEBUG>:DEBUG_MODE>)
add_compile_options(
        $<$<CONFIG:RELEASE>:-Ofast>
)
target_compile_definitions(rt3d PRIVATE ${COMPILE_DEFINITIONS})

set(Qt_libraries
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        Qt5::Concurrent
        Qt5::AxServer
        Qt5::AxContainer
        )
target_link_libraries(rt3d PRIVATE
        -static-libgcc
        -static-libstdc++
        PkgConfig::lensfun
        efsw
        ${Qt_libraries}
        pybind11::embed
        Python3::Python
        )

target_compile_options(rt3d PUBLIC ${lensfun_CFLAGS_OTHER} ${Python_LINK_OPTIONS})

target_include_directories(rt3d PUBLIC ${lensfun_INCLUDE_DIR})


# Add custom preprocessor definitions
if (MSVC)
    set_target_properties(rt3d PROPERTIES LINK_FLAGS " /MANIFEST:NO")
endif ()

pybind11_find_import(numpy VERSION 1.21.0 REQUIRED)

find_python_package(numpy)
find_python_package(imageio)
find_python_package(opencv-contrib-python-headless)
find_python_package(lensfunpy)

include_python_script(__init__.py rt3d)
include_python_script(dat2tiff.py rt3d)
include_python_script(image_matching_v2.py rt3d)
include_python_script(image_processing.py rt3d)
include_python_script(phase_matching_correct.py rt3d)
include_python_script(phase_correlation.py rt3d)
include_python_script(video2frames.py rt3d)
include_python_script(lens_correction.py rt3d)

post_build_DEM_generation(rt3d)
postbuild_windeployqt(rt3d)
add_custom_command(TARGET rt3d POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying lensfunDB files."
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory
        ${RealTime3D_SOURCE_DIR}/data/lensfunDB/db
        $<TARGET_FILE_DIR:rt3d>/lensfunDB/db

        COMMAND ${CMAKE_COMMAND} -E echo "Copying windows .DLL files."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Windows/System32/vcruntime140_1.dll"
        "C:/Windows/System32/vcruntime140.dll"
        "${lensfun_BIN_DIR}/liblensfun.dll"
        "${lensfun_BIN_DIR}/libglib-2.0-0.dll"
        $<TARGET_RUNTIME_DLLS:rt3d>
        $<TARGET_FILE_DIR:rt3d>
        COMMAND_EXPAND_LISTS
        )

foreach (X IN LISTS ALL_PYPACKAGE_DIRS)
    cmake_path(GET X FILENAME package_dir_name)
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/${package_dir_name})
        add_custom_command(TARGET rt3d POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Copying: ${package_dir_name}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${X}
                $<TARGET_FILE_DIR:rt3d>/${package_dir_name}
                )
    endif()
    install(DIRECTORY
        ${X}
        DESTINATION
        ${install_DIR_PATH}
        )
endforeach ()

getPyEmbedding()
foreach(X IN LISTS PYTHON_EMBEDDING)
    install(FILES ${X} DESTINATION ${install_DIR_PATH})
endforeach()



install(FILES
        $<TARGET_FILE:efsw>
        "C:/Windows/System32/vcruntime140_1.dll"
        "C:/Windows/System32/vcruntime140.dll"
        "${lensfun_BIN_DIR}/liblensfun.dll"
        "${lensfun_BIN_DIR}/libglib-2.0-0.dll"
        $<TARGET_RUNTIME_DLLS:rt3d>
        "${intl8_LIB}"
        "${pcre_LIB}"
        ${iconv_LIB}
        DESTINATION
        ${install_DIR_PATH}
        )

install(DIRECTORY ${RealTime3D_SOURCE_DIR}/data/lensfunDB/db DESTINATION ${install_DIR}/lensfunDB)
install(DIRECTORY ${RealTime3D_SOURCE_DIR}/data/aircraftDB DESTINATION ${install_DIR})



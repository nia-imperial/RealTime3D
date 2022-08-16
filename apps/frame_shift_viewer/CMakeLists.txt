cmake_minimum_required(VERSION 3.21)

project(FrameShiftViewer LANGUAGES CXX)

# find QT
add_definitions(-DQT_NO_KEYWORDS)
find_package(Qt5 COMPONENTS
        Core
        Svg
        Widgets
        GUI
        MultimediaWidgets
        REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


add_executable(frameshift_viewer main.cpp ${FRAME_SHIFT_VIEWER_SRC} ${SETTINGS_SRC})

target_compile_features(frameshift_viewer PUBLIC cxx_std_20)
set_target_properties(frameshift_viewer PROPERTIES
        CMAKE_CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )

set(COMPILE_DEFINITIONS $<$<CONFIG:DEBUG>:DEBUG_MODE>)
target_compile_definitions(frameshift_viewer PRIVATE ${COMPILE_DEFINITIONS})

set(Qt_libraries
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        )
target_link_libraries(frameshift_viewer PRIVATE
        -static-libgcc
        -static-libstdc++
        ${Qt_libraries}
        )

postbuild_windeployqt(frameshift_viewer)
add_custom_command(TARGET frameshift_viewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying windows .DLL files."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Windows/System32/vcruntime140_1.dll"
        "C:/Windows/System32/vcruntime140.dll"
        $<TARGET_RUNTIME_DLLS:frameshift_viewer>
        $<TARGET_FILE_DIR:frameshift_viewer>
        COMMAND_EXPAND_LISTS
        )





cmake_minimum_required(VERSION 3.21)

project(ImageMatching LANGUAGES CXX)

find_package(OpenMP REQUIRED)

find_package(OpenCV REQUIRED)

add_definitions(-DQT_NO_KEYWORDS)
find_package(Qt5 COMPONENTS
        Core
        Svg
        Widgets
        GUI
        REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


add_executable(imageMatch nav_main.cpp ${NAVIGATION_SRC} ${SETTINGS_SRC})
target_compile_features(imageMatch PUBLIC cxx_std_20)
set_target_properties(imageMatch PROPERTIES
        CMAKE_CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
target_link_libraries(imageMatch PRIVATE
        -static-libgcc
        -static-libstdc++
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        #        OpenMP::OpenMP_CXX
        ${OpenCV_LIBS}
        )

add_custom_command(TARGET imageMatch POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:imageMatch>
        $<TARGET_FILE_DIR:imageMatch>
        COMMAND_EXPAND_LISTS
        )
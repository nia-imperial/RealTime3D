cmake_minimum_required(VERSION 3.21)

project(Waypointer LANGUAGES CXX VERSION 1.0.0.0)

# find OpenCV
#find_package(OpenCV REQUIRED)

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


add_executable(wayptr main.cpp ${NAVIGATION_SRC} ${SETTINGS_SRC})
target_compile_features(wayptr PUBLIC cxx_std_20)
set_target_properties(wayptr PROPERTIES
        CMAKE_CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
target_link_libraries(wayptr PRIVATE
        -static-libgcc
        -static-libstdc++
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
#        ${OpenCV_LIBS}
        )

add_custom_command(TARGET wayptr POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:wayptr>
        $<TARGET_FILE_DIR:wayptr>
        COMMAND_EXPAND_LISTS
        )


#add_subdirectory(waypointer_io)
#add_subdirectory(MissionViewer)
#add_subdirectory(LayerSystem)

#target_sources(wayptr PRIVATE ${WAYPOINTER_SRC})

#set(UX_UI_ICONS_DIR_NAME ux_ui_cons)
#set(UX_UI_ICONS_DIR "${CMAKE_CURRENT_LIST_DIR}/icons/${UX_UI_ICONS_DIR_NAME}")
#set(UX_UI_SVG_DIR_NAME SVG)
#set(UX_UI_SVG_DIR "${UX_UI_ICONS_DIR}/${UX_UI_SVG_DIR_NAME}")
#set(UX_UI_SVG_FILES
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_36.svg"
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_35.svg"
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_31.svg"
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_38.svg"
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_11.svg"
#        "${UX_UI_SVG_DIR}/UX-UI-Icon_29.svg")

#add_custom_command(TARGET wayptr POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${UX_UI_ICONS_DIR}
#        $<TARGET_FILE_DIR:wayptr>

#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${UX_UI_SVG_DIR}
#        $<TARGET_FILE_DIR:wayptr>/${UX_UI_ICONS_DIR_NAME}

#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${UX_UI_SVG_FILES}
#        $<TARGET_FILE_DIR:wayptr>/${UX_UI_ICONS_DIR_NAME}/${UX_UI_SVG_DIR_NAME}
#        COMMAND_EXPAND_LISTS
#        )

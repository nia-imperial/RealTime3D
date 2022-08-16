set(NAVIGATION_SRC
        icons.qrc
        maininterface.cpp maininterface.h maininterface.ui
        layerpanel.cpp layerpanel.h
        coordinatepanel.cpp coordinatepanel.h
        Workspace/toolmode.cpp Workspace/toolmode.h
        Workspace/missionscene.cpp Workspace/missionscene.h
        Workspace/systemviewer.cpp Workspace/systemviewer.h
        Workspace/connectionline.cpp Workspace/connectionline.h
        Workspace/controlpoint.cpp Workspace/controlpoint.h
        Workspace/waypoint.cpp Workspace/waypoint.h
        Workspace/workspace.cpp Workspace/workspace.h
        flighttools.cpp flighttools.h
        waypointer_io/images.cpp waypointer_io/images.hpp
        waypointer_io/waypoints.hpp waypointer_io/waypoints.cpp
#        imageprocessing.cpp imageprocessing.h
        )
#add_subdirectory(feature_match)
add_source_list("${NAVIGATION_SRC}")
#global_list_append(NAVIGATION_SRC FEATURE_MATCH_SRC)


#list(TRANSFORM NAVIGATION_SRC PREPEND ${CMAKE_CURRENT_LIST_DIR})
#message(STATUS "navigation source files added")

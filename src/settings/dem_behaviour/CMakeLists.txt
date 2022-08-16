set(DEM_BEHAVIOUR_SRC
        dembehaviour.cpp dembehaviour.h dembehaviour.ui
        )

add_source_list("${DEM_BEHAVIOUR_SRC}")
#list(TRANSFORM DEM_BEHAVIOUR_SRC PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
#set(DEM_BEHAVIOUR_SRC "${DEM_BEHAVIOUR_SRC}" CACHE INTERNAL "")
#message(STATUS "** dem_behaviour source files added")

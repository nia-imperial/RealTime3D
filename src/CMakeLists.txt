set(SRC_SRC config.h)

#list(TRANSFORM SRC PREPEND ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(dem_generation)
add_subdirectory(device_profiles)
add_subdirectory(flight_parameters)
add_subdirectory(frame_shift_viewer)
add_subdirectory(main_window)
add_subdirectory(navigation)
add_subdirectory(settings)
add_subdirectory(utility)


add_source_list("${SRC_SRC}")
global_list_append(SRC_SRC DEM_GENERATION_SRC)
global_list_append(SRC_SRC DEVICE_PROFILES_SRC)
global_list_append(SRC_SRC FLIGHT_PARAMETERS_SRC)
global_list_append(SRC_SRC FRAME_SHIFT_VIEWER_SRC)
global_list_append(SRC_SRC MAIN_WINDOW_SRC)
global_list_append(SRC_SRC NAVIGATION_SRC)
global_list_append(SRC_SRC SETTINGS_SRC)
global_list_append(SRC_SRC UTILITY_SRC)

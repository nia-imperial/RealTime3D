set(SETTINGS_SRC
        SettingsForm.cpp SettingsForm.h
        settingsmenu.cpp settingsmenu.h settingsmenu.ui
        )

add_subdirectory(dem_behaviour)
add_subdirectory(general_settings)
add_subdirectory(path_settings)
add_subdirectory(geometric_settings)

add_source_list("${SETTINGS_SRC}")
global_list_append(SETTINGS_SRC DEM_BEHAVIOUR_SRC)
global_list_append(SETTINGS_SRC GENERAL_SETTINGS_SRC)
global_list_append(SETTINGS_SRC PATH_SETTINGS_SRC)
global_list_append(SETTINGS_SRC GEOMETRIC_SETTINGS_SRC)
set(DEVICE_PROFILES_SRC
        DeviceProfile.cpp DeviceProfile.h
        overviewprofile.cpp overviewprofile.h overviewprofile.ui
        )

add_subdirectory(aircraft)
add_subdirectory(camera)
add_subdirectory(lens)

add_source_list("${DEVICE_PROFILES_SRC}")
global_list_append(DEVICE_PROFILES_SRC AIRCRAFT_SRC)
global_list_append(DEVICE_PROFILES_SRC CAMERA_SRC)
global_list_append(DEVICE_PROFILES_SRC LENS_SRC)


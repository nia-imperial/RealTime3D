set(UTILITY_SRC
        messages.cpp messages.hpp
        GroupFileObjectDialog.hpp
        scriptlauncher.cpp scriptlauncher.h
        DirectoryWatchdog.cpp DirectoryWatchdog.hpp
        CameraWatchdog.cpp CameraWatchdog.hpp
        ../../libs/wia/wiaaut.cpp ../../libs/wia/wiaaut.h
        pyscriptcaller.h pyscriptcaller.cpp
        )

add_source_list("${UTILITY_SRC}")

#list(TRANSFORM UTILITY_SRC PREPEND ${CMAKE_CURRENT_LIST_DIR})
#message(STATUS "utility source files added")

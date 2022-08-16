# Local external projects
#add_executable(dem_images IMPORTED)
#set_property(TARGET dem_images PROPERTY
#        IMPORTED_LOCATION "DemGeneration_software_licensed/ExerciseDemGeneration.exe")



function(post_build_DEM_generation target_name)

        set(DEMexe_DIR_NAME "DemGeneration_software_licensed")
        set(DEMexe_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/${DEMexe_DIR_NAME}")
        set(DEMexe "${DEMexe_DIR}/Ekag20nt.exe"
                "${DEMexe_DIR}/Ekc.sif"
                "${DEMexe_DIR}/Ekc6420.dll"
                "${DEMexe_DIR}/ExerciseDemGeneration.exe"
                "${DEMexe_DIR}/ExerciseDemGeneration.suo"
                "${DEMexe_DIR}/libfftw3-3.dll"
                "${DEMexe_DIR}/PhaseCorrelation.dll"
                "${DEMexe_DIR}/vcomp100.dll"
                )
        message(DEBUG "${DEMexe_DIR_NAME}")
        message(DEBUG "${DEMexe_DIR}")
        message(DEBUG "${DEMexe}")

        add_custom_command(TARGET ${target_name} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target_name}>/${DEMexe_DIR_NAME})

        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Copying ${DEMexe_DIR_NAME} to $<TARGET_FILE_DIR:${target_name}> "
                COMMAND ${CMAKE_COMMAND}
                -E copy_if_different
                ${DEMexe}
                $<TARGET_FILE_DIR:${target_name}>/${DEMexe_DIR_NAME}
                COMMAND_EXPAND_LISTS
                )

        # CORRECT: absolute path used inside the generator expression
#        target_sources(${target_name} "$<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_FILE}/dbgsrc.cpp>")
        install(DIRECTORY ${DEMexe_DIR} DESTINATION ${install_DIR_PATH})
endfunction()

# git imported libraries
find_package(Git REQUIRED)
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Pybind11
FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG master
        GIT_PROGRESS ON
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/pybind11
        )

set(PYBIND11_PYTHON_VERSION 3)
set(PYBIND11_FINDPYTHON ON)
FetchContent_MakeAvailable(pybind11)
#add_subdirectory(pybind11 EXCLUDE_FROM_ALL)

# EFSW
FetchContent_Declare(
        efsw
        GIT_REPOSITORY https://github.com/SpartanJ/efsw.git
        GIT_TAG master
        GIT_PROGRESS ON
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/efsw
)
FetchContent_MakeAvailable(efsw)

#find_file(PYTHON_ZIP python39.zip PATHS python-3.9.5-embed-amd64 REQUIRED)
function(getPyEmbedding)
        file(GLOB_RECURSE PYTHON_EMBEDDING CONFIGURE_DEPENDS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/python-3.9.5-embed-amd64/*")
        set(PYTHON_EMBEDDING ${PYTHON_EMBEDDING} PARENT_SCOPE)
endfunction()
# Lensfun
#find_library(lensfun
#        NAMES liblensfun
#        HINTS "${CMAKE_CURRENT_LIST_DIR}/lensfun/lib"
#        REQUIED)
#set(lensfun_INCLUDE_DIRS)
##set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL YES)
#FetchContent_Declare(
#        lensfun
#        GIT_REPOSITORY https://github.com/lensfun/lensfun.git
#        GIT_TAG master
#        GIT_PROGRESS ON
#        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/lensfun"
#        BUILD_COMMAND ""
#        )
#
#file(READ lensfun/CMakeLists.txt  filedata)
#string(REGEX REPLACE "CMAKE_SOURCE_DIR" "CMAKE_CURRENT_SOURCE_DIR" filedata "${filedata}")
#file(WRITE lensfun/CMakeLists.txt "${filedata}")
#FetchContent_MakeAvailable(lensfun)
#add_subdirectory(${lensfun_SOURCE_DIR})


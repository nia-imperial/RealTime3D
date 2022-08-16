# find python3
set(Python3_FIND_VIRTUALENV ONLY)
set(Python3_FIND_REGISTRY NEVER)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Python3 COMPONENTS NumPy REQUIRED)

#message(DEBUG "python::Python_EXECUTABLE ${Python3_EXECUTABLE}")
#message(DEBUG "python::Python_INCLUDE_DIRS ${Python3_INCLUDE_DIRS}")
#message(DEBUG "python::Python_LINK_OPTIONS ${Python3_LINK_OPTIONS}")
#message(DEBUG "python::Python_LIBRARIES ${Python3_LIBRARIES}")
#message(DEBUG "python::Python_LIBRARY_DIRS ${Python3_LIBRARY_DIRS}")
#message(DEBUG "python::Python_RUNTIME_LIBRARY_DIRS ${Python3_RUNTIME_LIBRARY_DIRS}")
#message(DEBUG "python::Python_NumPy_INCLUDE_DIRS ${Python3_NumPy_INCLUDE_DIRS}")

cmake_path(GET Python3_NumPy_INCLUDE_DIRS PARENT_PATH python_site_packages)
cmake_path(GET python_site_packages PARENT_PATH python_site_packages)
cmake_path(GET python_site_packages PARENT_PATH python_site_packages)
message(STATUS ${python_site_packages})

set(PYTHON_SCRIPTS_DIR "${CMAKE_CURRENT_LIST_DIR}/../scripts" CACHE INTERNAL "")  # hacked path, no time for fanciness
message(DEBUG "Python scripts directory: ${PYTHON_SCRIPTS_DIR}")


function(find_python_package NAME)
    # get site-packages directory for package
    execute_process(COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/get_package_site_dir.py" ${NAME}
            OUTPUT_VARIABLE ${NAME}_PYPACKAGE)
    message(STATUS "Python package location for '${NAME}':")
    foreach (X IN LISTS ${NAME}_PYPACKAGE)
        message(STATUS "\t> ${X}")
    endforeach ()
    set(${NAME}_PYPACKAGE ${${NAME}_PYPACKAGE} PARENT_SCOPE)
    list(APPEND ALL_PYPACKAGE_DIRS ${${NAME}_PYPACKAGE})
    list(REMOVE_DUPLICATES ALL_PYPACKAGE_DIRS)
    set(ALL_PYPACKAGE_DIRS ${ALL_PYPACKAGE_DIRS} PARENT_SCOPE)

endfunction()

function(include_python_script SCRIPT_NAME TARGET_NAME)
    set(SCRIPT_LOC "${PYTHON_SCRIPTS_DIR}/${SCRIPT_NAME}")

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/compile_and_move.py
            "${SCRIPT_LOC}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/scripts"
            )

    install(CODE "execute_process(COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/compile_and_move.py
            ${SCRIPT_LOC}
            ${install_DIR_PATH}/scripts
            )")

endfunction()


function(add_source_list SOURCE_FILES_LIST )

    list(TRANSFORM SOURCE_FILES_LIST PREPEND "${CMAKE_CURRENT_LIST_DIR}/")
    cmake_path(GET CMAKE_CURRENT_LIST_DIR FILENAME dir_name)

    string(TOUPPER ${dir_name} SOURCE_LIST_NAME)
    set(SOURCE_LIST_NAME "${SOURCE_LIST_NAME}_SRC")
    set(${SOURCE_LIST_NAME} "${SOURCE_FILES_LIST}" CACHE INTERNAL "")
    set(${SOURCE_LIST_NAME} "${SOURCE_FILES_LIST}" PARENT_SCOPE)

    message(DEBUG "** ${dir_name} source files added to -> ${SOURCE_LIST_NAME}")
    foreach(X IN LISTS SOURCE_FILES_LIST)
        message(DEBUG "\t\t ${X}")
    endforeach()

endfunction()

function(global_list_append GLOBAL_NAME OTHER_LIST)
    set(GLOBAL_LIST "${${GLOBAL_NAME}}")

    list(APPEND GLOBAL_LIST ${${OTHER_LIST}})
#    message(DEBUG "${GLOBAL_NAME}")
#    foreach(X IN LISTS GLOBAL_LIST)
#        message(DEBUG "x--> ${X}")
#    endforeach()
#    message(STATUS "")
    set("${GLOBAL_NAME}" "${GLOBAL_LIST}" PARENT_SCOPE)
    set("${GLOBAL_NAME}" "${GLOBAL_LIST}" CACHE INTERNAL "")
endfunction()

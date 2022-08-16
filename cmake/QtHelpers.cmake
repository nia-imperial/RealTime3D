function(postbuild_windeployqt TARGET_NAME)

    # QT variables
    get_filename_component(QT_BIN_DIR ${QT_MOC_EXECUTABLE}/.. ABSOLUTE)
    message(STATUS "Qt bin directory: ${QT_BIN_DIR}")
    find_program(WINDEPLOY_QT windeployqt.exe PATHS ${QT_BIN_DIR} REQUIRED)
    message(STATUS "windeployqt executable: ${WINDEPLOY_QT}")

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeploy_qt."
            COMMAND ${WINDEPLOY_QT}
            --test
            --no-angle
            --no-opengl-sw
            --no-quick-import
            --no-system-d3d-compiler
            --verbose=1
            --no-translations
            --no-webkit2
            $<TARGET_FILE_DIR:${TARGET_NAME}>/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX})

    install(CODE "execute_process(COMMAND ${WINDEPLOY_QT}
            --test
            --no-system-d3d-compiler
            --no-translations
            --no-webkit2
            --no-quick-import
            --verbose=1
             ${install_PATH})")

endfunction()
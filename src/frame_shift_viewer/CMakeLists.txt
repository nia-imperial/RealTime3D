set(FRAME_SHIFT_VIEWER_SRC
        FrameShiftScene.cpp FrameShiftScene.h
        FrameShiftModule.cpp FrameShiftModule.h FrameShiftModule.ui
        RectFrame.cpp RectFrame.h
        OpenGlViewport.cpp OpenGlViewport.h
        )

add_source_list("${FRAME_SHIFT_VIEWER_SRC}")

#cmake_path(SET current_dir "${CMAKE_CURRENT_SOURCE_DIR}")
#cmake_path(GET current_dir FILENAME build_dir_filename)

#add_custom_target(copy_assets
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_CURRENT_LIST_DIR}/Reference.svg
#        ${CMAKE_CURRENT_LIST_DIR}/Secondary.svg
#        $<TARGET_FILE_DIR:rt3d>/${build_dir_filename}
#        )
#add_dependencies(rt3d copy_assets)

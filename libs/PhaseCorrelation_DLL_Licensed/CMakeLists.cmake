#message(STATUS "Linking PhaseCorrelation_DLL_Licensed")

add_library(PhaseCorrelation SHARED IMPORTED GLOBAL)
set_property(TARGET PhaseCorrelation PROPERTY
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelation.dll)
set_property(TARGET PhaseCorrelation PROPERTY
        IMPORTED_IMPLIB ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelation.lib)
set_target_properties(PhaseCorrelation PROPERTIES
        INTERFACE_LINK_LIBRARIES ${CMAKE_CURRENT_LIST_DIR}/libfftw3-3.dll)

#add_library(PhaseCorrelation ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelation.dll)
#target_link_libraries(PhaseCorrelation ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelation.lib)
set_target_properties(PhaseCorrelation PROPERTIES LINKER_LANGUAGE C)

target_sources(PhaseCorrelation INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelation.h
        ${CMAKE_CURRENT_LIST_DIR}/PhaseCorrelationGlobalDefs.h)


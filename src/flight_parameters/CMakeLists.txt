
set(FLIGHT_PARAMETERS_SRC
        Camera.cpp Camera.h
        Uav.cpp Uav.h
        FlightParameters.cpp FlightParameters.h flightParameters.ui
        ImageCalculations.hpp
        UnitConversion.hpp
        )

add_source_list("${FLIGHT_PARAMETERS_SRC}")
#list(TRANSFORM FLIGHT_PARAMETERS_SRC PREPEND ${CMAKE_CURRENT_LIST_DIR})
#message(STATUS "flight_parameters source files added")

cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )
set(CMAKE_CXX_STANDARD 14)
project( PhotonicDirector )

get_filename_component( CINDER_PATH "../../cinder_0.9.1_mac" ABSOLUTE )
get_filename_component( APP_PATH "." ABSOLUTE )


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CINDER_PATH}/proj/cmake/modules)
include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

set( SRC_FILES
	${APP_PATH}/src/ConfigManager.cpp
	${APP_PATH}/src/Effects.cpp
	${APP_PATH}/src/Light.cpp
	${APP_PATH}/src/Output.cpp
	${APP_PATH}/src/PhotonicDirectorApp.cpp
	${APP_PATH}/src/Visualizer.cpp
)

ci_make_app(
	SOURCES     ${SRC_FILES}
	CINDER_PATH ${CINDER_PATH}
	BLOCKS      DMXusbPro Cinder-ImGui OSC
)
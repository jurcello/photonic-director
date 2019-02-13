cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )
set(CMAKE_CXX_STANDARD 14)
set(CINDER_COPY_ASSETS ON)
project( PhotonicDirector )

get_filename_component( CINDER_PATH "../../cinder_0.9.1_mac" ABSOLUTE )
get_filename_component( APP_PATH "." ABSOLUTE )


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CINDER_PATH}/proj/cmake/modules)
include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

SET(VENDOR_DIR ${APP_PATH}/vendor)
add_subdirectory("${VENDOR_DIR}/poco-poco-1.9.0-release")

#include(build/conanbuildinfo.cmake)
#conan_basic_setup()

set( SRC_FILES
	${APP_PATH}/src/ConfigManager.cpp
	${APP_PATH}/src/UnityConnector.cpp
	${APP_PATH}/src/Effects.cpp
	${APP_PATH}/src/Light.cpp
	${APP_PATH}/src/LightCalibrator.cpp
	${APP_PATH}/src/LightComponent.cpp
	${APP_PATH}/src/Output.cpp
	${APP_PATH}/src/PhotonicDirectorApp.cpp
	${APP_PATH}/src/Visualizer.cpp
	${APP_PATH}/src/Utils.h
	${APP_PATH}/src/Effects/WaveEffect.cpp
	${APP_PATH}/src/Effects/LocationFollow.cpp
	${APP_PATH}/src/Effects/FlashLight.cpp
	${APP_PATH}/src/Effects/VolumeLocation.cpp
	${APP_PATH}/src/Effects/Ripples.cpp
	${APP_PATH}/src/Effects/MuteAll.cpp
	${APP_PATH}/src/Effects/Desaturate.cpp
	${APP_PATH}/src/Effects/Droplet.cpp
	${APP_PATH}/src/Effects/OriginatingWave.cpp
	${APP_PATH}/src/Effects/ConditionalVolume.cpp
	${APP_PATH}/src/Effects/CloseInLocation.cpp
	${APP_PATH}/src/Effects/PulseStatic.cpp
	${APP_PATH}/src/Effects/Preset.cpp
	${APP_PATH}/src/Effects/MidiLight.cpp
	${APP_PATH}/src/Effects/FightingInputs.cpp
	${APP_PATH}/src/Effects/Drifter.cpp
	${APP_PATH}/src/Effects/StoryTeller.cpp
	${APP_PATH}/src/Effects/ShutDown.cpp
	${APP_PATH}/src/Effects/MovingHead.cpp
)

message(STATUS "Poco components: ${Poco_COMPONENTS}")

ci_make_app(
	SOURCES     ${SRC_FILES}
	CINDER_PATH ${CINDER_PATH}
	BLOCKS      DMXusbPro Cinder-ImGui OSC midifile
	LIBRARIES 	PocoFoundation PocoNet PocoDNSSD PocoDNSSDBonjour
)


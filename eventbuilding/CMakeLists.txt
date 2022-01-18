########################################################
# cmake file for building the event building program
# @author Hector Garcia
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
########################################################

message("CPP Version: " ${CMAKE_CXX_STANDARD})

# set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_MACOSX_RPATH 1)

# project name
project(ECal_EventBuilding)

# project version
set( ${PROJECT_NAME}_VERSION_MAJOR 0 )
set( ${PROJECT_NAME}_VERSION_MINOR 0 )
set( ${PROJECT_NAME}_VERSION_PATCH 1 )

set( CMAKE_INSTALL_PREFIX "..")

### DEPENDENCIES ############################################################

find_package( Marlin 1.0 REQUIRED )
include_directories( ${Marlin_INCLUDE_DIRS} )
link_libraries( ${Marlin_LIBRARIES} )

#find_package( LCIO REQUIRED )
#include_directories( ${LCIO_INCLUDE_DIRS} )
#link_libraries( ${LCIO_LIBRARIES} )

find_package( ROOT REQUIRED )
include_directories( ${ROOT_INCLUDE_DIRS} )
link_libraries( ${ROOT_LIBRARIES} )

### LIBRARY #################################################################

# definitions to pass to the compiler
add_definitions( "-Wall -ansi -pedantic" )
add_definitions( "-Wno-long-long" )

# include directories
include_directories( ./include )

# add executable
aux_source_directory( ./src library_sources )
add_executable( ${PROJECT_NAME} ${library_sources} )
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION app )


#########################################################################
#
#  CMAKE
#
#########################################################################

###
#  Minimum Version
#  ---------------
#  The CMake version required.
###
cmake_minimum_required(VERSION 3.1)

###
#  CMake Configuration
#  -------------------
#  Configuration settings for CMake.
#
#  NOTE:
#  These settings have to be applied before the project() setting!
###
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)

###
#  Project Info
#  ------------
#  General simple information about our project.
###
project(de.mrh.mirrorspeech VERSION 1.0
                            DESCRIPTION "MRH mirror speech application"
                            LANGUAGES CXX)


#########################################################################
#
#  PATHS
#
#########################################################################

###
#  Binary Paths
#  ------------
#  The paths for our created binary file(s).
###
set(BIN_DIR_PATH "${CMAKE_SOURCE_DIR}/bin/")
file(MAKE_DIRECTORY ${BIN_DIR_PATH})


###
#  Build Paths
#  -----------
#  The paths for the cmake build.
###
set(BUILD_DIR_PATH "${CMAKE_SOURCE_DIR}/build/")
file(MAKE_DIRECTORY ${BUILD_DIR_PATH})

###
#  Source Paths
#  ------------
#  The paths to the source files to use.
#  Add OS specific source files in their own list.
###
set(SRC_DIR_PATH "${CMAKE_SOURCE_DIR}/src/")
             
set(SRC_LIST_APP "${SRC_DIR_PATH}/Revision.h"
                 "${SRC_DIR_PATH}/Main.cpp")
                 
set(SRC_LIST_MODULE "${SRC_DIR_PATH}/Module/RepeatAfterMe/RepeatAfterMe.cpp"
                    "${SRC_DIR_PATH}/Module/RepeatAfterMe/RepeatAfterMe.h")

###
#  Library Paths
#  -------------
#  The paths to the external libraries to use. 
#  Both the binary and include paths should be set here.
#  Add OS specific libraries in their own lists.
###
set(LIB_DIR_PATH "${CMAKE_SOURCE_DIR}/lib/")
set(INCLUDE_DIR_PATH "${CMAKE_SOURCE_DIR}/include/")

set(LIB_BIN_LIST "${LIB_DIR_PATH}/libmrh.a"
				 "${LIB_DIR_PATH}/libmrhcevs.a"
				 "${LIB_DIR_PATH}/libmrhab.a"
				 "${LIB_DIR_PATH}/libmrhbf.a"
				 "${LIB_DIR_PATH}/libmrhvt.a")
set(LIB_INCLUDE_LIST "${INCLUDE_DIR_PATH}/mrhshared"
					 "${INCLUDE_DIR_PATH}/libmrh"
					 "${INCLUDE_DIR_PATH}/libmrhcevs"
					 "${INCLUDE_DIR_PATH}/libmrhab"
					 "${INCLUDE_DIR_PATH}/libmrhbf"
					 "${INCLUDE_DIR_PATH}/libmrhvt")

#########################################################################
#
#  TARGET
#
#########################################################################

###
#  Target
#  ------
#  The target(s) to build.
#
#  NOTE: 
#  MRH applications are required to be named App.so!
#  They are build as shared objects.
###
add_library(MRH_App SHARED ${SRC_LIST_APP}
                           ${SRC_LIST_MODULE})
set_target_properties(MRH_App
    				  PROPERTIES
					  PREFIX ""
					  OUTPUT_NAME "App"
					  SUFFIX ".so"
    				  ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR_PATH}
    				  LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR_PATH}
    				  RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR_PATH})

###
#  Required Libraries
#  ------------------
#  Libraries required by this application.
###
target_link_libraries(MRH_App PUBLIC ${LIB_BIN_LIST})
target_include_directories(MRH_App PUBLIC ${LIB_INCLUDE_LIST})
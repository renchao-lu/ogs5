#### OGS-5 Qt Project ####

# Specify minimum CMake version
cmake_minimum_required(VERSION 2.4)

# Set build configuration types
# "RelWithDebInfo" and "MinSizeRelease" can be added here
#

IF (WIN32)
	SET ( CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE TYPE INTERNAL FORCE )
ENDIF (WIN32)

# Project name
PROJECT( OGS-5 )

# Adds useful macros and variables
INCLUDE( CMakeConfiguration/Macros.cmake )

# Do some prerequisite checks
IF (WIN32)
   IF (NOT MSVC)
      MESSAGE (FATAL_ERROR "Aborting: On Windows only the Visual Studio compiler is supported!")
   ENDIF (NOT MSVC)
ENDIF (WIN32)

INCLUDE( CMakeConfiguration/Options.cmake )

# Set additional CMake modules path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeConfiguration")

# Find Shapelib
FIND_PACKAGE( Shapelib )

# Set build directories
# Binaries are created in /bin and libraries in /lib
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

# Relative paths are used in the created projects
# May not work correcty due to the CMake docs
#SET( CMAKE_USE_RELATIVE_PATHS TRUE )

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
ELSE (MSVC)
	ADD_DEFINITIONS (-D__STDC_FORMAT_MACROS)	# for OpenSG compiling
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wextra")
ENDIF (MSVC)


# Add subdirectories with the projects
ADD_SUBDIRECTORY( Base )
ADD_SUBDIRECTORY( MathLib )
ADD_SUBDIRECTORY( GEO )
#ADD_SUBDIRECTORY( DMS )
ADD_SUBDIRECTORY( MSH )
ADD_SUBDIRECTORY( FEM )
ADD_SUBDIRECTORY( FileIO )
ADD_SUBDIRECTORY( OGSProject )
ADD_SUBDIRECTORY( OGS )

## MDL: compiling with LIBPHREEQC
IF (LIBPQC)
	ADD_SUBDIRECTORY ( PQC )
ENDIF (LIBPQC)

IF (GEM_REACT)
	ADD_SUBDIRECTORY( GEM )
ENDIF (GEM_REACT)

IF (QT4_FOUND)
	# Adds useful macros and variables
	# this is needed to correctly link the qt libraries through target_link_libraries
	INCLUDE( ${QT_USE_FILE} )
	ADD_SUBDIRECTORY( Qt )
ENDIF (QT4_FOUND)

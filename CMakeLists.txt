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

# Ask for options
OPTION (RANDOM_WALK "Do you want to add #define RANDOM_WALK ?" ON)
OPTION (PROBLEM_CLASS "Do you want to add #define PROBLEM_CLASS ?" ON)

## MDL: cmake options for LIBPHREEQC and DEBUG
OPTION (LIBPQC "Do you want to compile LIBPHREEQC?" OFF)
OPTION (LIBPQC_DEBUG "Do you want to compile LIBPHREEQC with debug flag?" OFF)

OPTION (OGS_USE_QT "Do you want to build the OGS GUI?" OFF)

IF (OGS_USE_QT)
	# By default only QtCore and QtGui modules are enabled
	# other modules must be enabled like this:
	SET( QT_USE_QTOPENGL TRUE )
	SET( QT_USE_QTSQL TRUE )
	SET( QT_USE_QTTEST TRUE )

	# Find installed Qt4 libraries and headers
	FIND_PACKAGE( Qt4 )

	IF (QTXMLPATTERNS_FOUND)
		set( QT_USE_QTXMLPATTERNS TRUE )
	ENDIF (QTXMLPATTERNS_FOUND)

	# Adds useful macros and variables
	# this is needed to correctly link the qt libraries through target_link_libraries
	INCLUDE( ${QT_USE_FILE} )
ENDIF (OGS_USE_QT)

# Create Configure.h
CONFIGURE_FILE (Base/Configure.h.in ${PROJECT_BINARY_DIR}/Base/Configure.h)
INCLUDE_DIRECTORIES( ${PROJECT_BINARY_DIR}/Base )

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
ELSE (MSVC)
	ADD_DEFINITIONS (-D__STDC_FORMAT_MACROS)	# for OpenSG compiling
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wextra")
ENDIF (MSVC)


## MDL: compiling with LIBPHREEQC
IF (LIBPQC)
	ADD_DEFINITIONS ( -DLIBPHREEQC )
	MESSAGE ( "-- Compiling built-in LIBPHREEQC" )
	IF (LIBPQC_DEBUG)
	   MESSAGE ( "-- Compiling ogs and LIBPHREEQC with debug flags" )
	   ADD_DEFINITIONS ( "-O0 -ggdb -DMDL_DEBUG" )
	ELSE (LIBPQC_DEBUG)
	   ADD_DEFINITIONS ( "-O3" )
	ENDIF (LIBPQC_DEBUG)
	ADD_SUBDIRECTORY ( PQC )
ENDIF (LIBPQC)

# Add subdirectories with the projects
ADD_SUBDIRECTORY( Base )
ADD_SUBDIRECTORY( GEO )
#ADD_SUBDIRECTORY( DMS )
ADD_SUBDIRECTORY( MSH )
ADD_SUBDIRECTORY( FEM )
#ADD_SUBDIRECTORY( GEM )
ADD_SUBDIRECTORY( FileIO )
ADD_SUBDIRECTORY( OGSProject )
ADD_SUBDIRECTORY( OGS )


IF (QT4_FOUND)
	# Adds useful macros and variables
	# this is needed to correctly link the qt libraries through target_link_libraries
	INCLUDE( ${QT_USE_FILE} )
	ADD_SUBDIRECTORY( Qt )
ENDIF (QT4_FOUND)

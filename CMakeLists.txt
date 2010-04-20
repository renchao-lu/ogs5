#### OGS-5 Qt Project ####

# Specify minimum CMake version
cmake_minimum_required(VERSION 2.6)	# or set to 2.4 for older systems (like JUGENE...)

# Set build configuration types
# "RelWithDebInfo" and "MinSizeRelease" can be added here
#

IF (WIN32)
	SET ( CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE TYPE INTERNAL FORCE )
ENDIF (WIN32)

SET (OGS_VERSION_MAJOR 5)
SET (OGS_VERSION_MINOR 0)
SET (OGS_VERSION_PATCH 0)

# Project name
PROJECT( OGS-${OGS_VERSION_MAJOR} )

# Adds useful macros and variables
INCLUDE( CMakeConfiguration/Macros.cmake )

# Adds packaging configuration
INCLUDE( CMakeConfiguration/Pack.cmake)

# Do some prerequisite checks
IF (WIN32)
   IF (NOT MSVC)
      MESSAGE (FATAL_ERROR "Aborting: On Windows only the Visual Studio compiler is supported!")
   ENDIF (NOT MSVC)
ENDIF (WIN32)

INCLUDE( CMakeConfiguration/Options.cmake )

# Set additional CMake modules path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeConfiguration")

# Find OpenMP
FIND_PACKAGE( OpenMP )

# Find Shapelib
FIND_PACKAGE( Shapelib )

IF (UNIX)
  # Find LISlib
  FIND_PACKAGE( LIS )

  # Find MKLlib
  FIND_PACKAGE( MKL )

  if(MKL)
    SET(CMAKE_CXX_FLAGS "-I${CMAKE_SOURCE_DIR}/LIB/MKL/include")
    if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    	SET(CMAKE_CXX_LINK_FLAGS "-L${CMAKE_SOURCE_DIR}/LIB/MKL/64 -llis-64 -lmkl_solver_lp64 -lmkl_intel_lp64 -lmkl_gnu_thread -lmkl_core ")
    else (CMAKE_SIZEOF_VOID_P MATCHES "8")
    	SET(CMAKE_CXX_LINK_FLAGS "-L${CMAKE_SOURCE_DIR}/LIB/MKL/32 -llis-32 -lmkl_solver -lmkl_intel -lmkl_gnu_thread -lmkl_core ")
    endif (CMAKE_SIZEOF_VOID_P MATCHES "8")
  ENDIF(MKL)
ENDIF (UNIX)

# Set build directories
# Binaries are created in /bin and libraries in /lib
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
SET( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

# Relative paths are used in the created projects
# May not work correcty due to the CMake docs
#SET( CMAKE_USE_RELATIVE_PATHS TRUE )

# Set OpenMP flags
IF (LIS AND OPENMP_FOUND)
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
ENDIF (LIS AND OPENMP_FOUND)

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
	ADD_DEFINITIONS(-D_CRT_XNONSTDC_NO_WARNINGS)
ELSE (MSVC)
	ADD_DEFINITIONS (-D__STDC_FORMAT_MACROS)	# for OpenSG compiling
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wno-deprecated -Wall -Wextra")
ENDIF (MSVC)

# Add LIB to include and link directory
include_directories( ${CMAKE_SOURCE_DIR}/LIB )
link_directories( ${CMAKE_SOURCE_DIR}/LIB )

# Add subdirectories with the projects
ADD_SUBDIRECTORY( Base )
ADD_SUBDIRECTORY( MathLib )
ADD_SUBDIRECTORY( GEO )
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

#### OGS-5 Qt Project ####

# Specify minimum CMake version
cmake_minimum_required(VERSION 2.6)

# Set build configuration types
# "RelWithDebInfo" and "MinSizeRelease" can be added here
set ( CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE TYPE INTERNAL FORCE )

# Project name
project( OGS-5 )

# Adds useful macros and variables
include( CMakeConfiguration/Macros.cmake )

# Do some prerequisite checks
if (WIN32)
	if (NOT MSVC)
		message (FATAL_ERROR "Aborting: On Windows only the Visual Studio compiler is supported!")
	endif (NOT MSVC)
endif (WIN32)

# Ask for options
option (RANDOM_WALK "Do you want to add #define RANDOM_WALK ?" ON)
option (PROBLEM_CLASS "Do you want to add #define PROBLEM_CLASS ?" ON)

# Find installed Qt4 libraries and headers
find_package( Qt4 )

# Create Configure.h
configure_file (Base/Configure.h.in ${PROJECT_BINARY_DIR}/Base/Configure.h)
include_directories( ${PROJECT_BINARY_DIR}/Base )

# Set additional CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeConfiguration")

# Find Shapelib
find_package( Shapelib REQUIRED )

# Adds useful macros and variables
# this is needed to correctly link the qt libraries through target_link_libraries
INCLUDE( ${QT_USE_FILE} )

# Set build directories
# Binaries are created in /bin and libraries in /lib
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

# Relative paths are used in the created projects
# May not work correcty due to the CMake docs
#set( CMAKE_USE_RELATIVE_PATHS TRUE )

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else (MSVC)
    add_definitions (-D__STDC_FORMAT_MACROS)	# for OpenSG compiling
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wall -Wextra")
endif (MSVC)

# Add subdirectories with the projects
add_subdirectory( Base )
add_subdirectory( GEO )
#add_subdirectory( DMS )
add_subdirectory( MSH )
add_subdirectory( FEM )
#add_subdirectory( GEM )
add_subdirectory( FileIO )
add_subdirectory( OGSProject )
add_subdirectory( OGS )

if (QT4_FOUND)
  add_subdirectory( Qt )
endif (QT4_FOUND)

# Copy ExampleData
option (COPY_EXAMPLE_DATA "If checked, every time CMake is run the data files from /ExampleData get copied to the executable directories" OFF)
if (COPY_EXAMPLE_DATA)
	message(STATUS "Copying example data...")
	if (WIN32)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/ExampleData ${PROJECT_BINARY_DIR}/bin/Debug/ExampleData )
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/bin/Debug/ExampleData/.svn )
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/ExampleData ${PROJECT_BINARY_DIR}/bin/Release/ExampleData )
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/bin/Release/ExampleData/.svn )
	else (WIN32)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/ExampleData ${PROJECT_BINARY_DIR}/bin/ExampleData )
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/bin/ExampleData/.svn )
	endif (WIN32)
endif (COPY_EXAMPLE_DATA)


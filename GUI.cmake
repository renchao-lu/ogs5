## Programmed by Lars Bilke
## Modified by:
##            WW 20.04.2010. 

# Project name
PROJECT( OGS-${OGS_VERSION_MAJOR}-GUI )

# Set additional CMake modules path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeConfiguration")

ADD_SUBDIRECTORY( Qt )

# Self-compile QVTK
option (OGS_COMPILE_QVTK "Should the QVTK window be compiled from OGS sources?" OFF)

IF(CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS (-D__STDC_FORMAT_MACROS)	# for OpenSG compiling
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# Ask for options
OPTION (RANDOM_WALK "Do you want to add #define RANDOM_WALK ?" OFF)
OPTION (PROBLEM_CLASS "Do you want to add #define PROBLEM_CLASS ?" OFF)
OPTION (NEW_EQS "Do you want to add #define NEW_EQS ?" OFF)
OPTION (BRNS OFF)
OPTION (GEM_REACT OFF)
OPTION (IPMGEMPLUGIN OFF)

OPTION (_POSIX_SOURCE OFF)
OPTION (NO_ERROR_CONTROL OFF)

## MDL: cmake options for LIBPHREEQC and DEBUG
OPTION (LIBPQC "Do you want to compile LIBPHREEQC?" OFF)
OPTION (LIBPQC_DEBUG "Do you want to compile LIBPHREEQC with debug flag?" OFF)

OPTION (CONFIG_G++ "Do you want to use the g++ config?" OFF)
OPTION (CONFIG_G++_BRNS "Do you want to use the g++_BRNS config?" OFF)
OPTION (CONFIG_G++_GEMS "Do you want to use the g++_gems config?" OFF)

if (CONFIG_G++)
	INCLUDE (configs/g++.cmake)
endif(CONFIG_G++)

if (CONFIG_G++_BRNS)
	INCLUDE (configs/g++_BRNS.cmake)
endif(CONFIG_G++_BRNS)

if (CONFIG_G++_GEMS)
	INCLUDE (configs/g++_gems.cmake)
endif(CONFIG_G++_GEMS)

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
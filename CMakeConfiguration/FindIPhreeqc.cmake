# - Try to find IPhreeqc
#
# Use IPHREEQC_DIR to point to the directory
#  E.g. cmake -DIPHREEQC_DIR=../iphreeqc-3.0.2-7614
#
# Once done this will define
#  IPHREEQC_FOUND - System has IPhreeqc
#  IPHREEQC_INCLUDE_DIRS - The IPhreeqc include directories
#  IPHREEQC_LIBRARIES - The libraries needed to use IPhreeqc
#  IPHREEQC_DEFINITIONS - Compiler switches required for using IPhreeqc

find_path(IPHREEQC_INCLUDE_DIR IPhreeqc.h
	HINTS ${IPHREEQC_DIR} ${CMAKE_SOURCE_DIR}/../Libs/iphreeqc
	PATH_SUFFIXES src )

find_library(IPHREEQC_LIBRARY NAMES iphreeqc
	HINTS ${IPHREEQC_DIR} ${CMAKE_SOURCE_DIR}/../Libs/iphreeqc
	PATH_SUFFIXES src/.libs )

set(IPHREEQC_LIBRARIES ${IPHREEQC_LIBRARY} )
set(IPHREEQC_INCLUDE_DIRS ${IPHREEQC_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set IPHREEQC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(IPhreeqc  DEFAULT_MSG
                                  IPHREEQC_LIBRARY IPHREEQC_INCLUDE_DIR)

mark_as_advanced(IPHREEQC_INCLUDE_DIR IPHREEQC_LIBRARY )
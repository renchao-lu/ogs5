
if (NOT MKL_FOUND)

	include(LibFindMacros)
	
	find_path( MKL_INCLUDE_DIR
		NAMES mkl.h
		PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/include)

	if ( UNIX )
		# Tell if the unix system is on 64-bit base
		if(CMAKE_SIZEOF_VOID_P MATCHES "8")
			find_library(MKL_LIBRARIES
				NAMES mkl_solver_lp64 mkl_intel_lp64 mkl_gnu_thread mkl_core
				PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/64 )	
		else (CMAKE_SIZEOF_VOID_P MATCHES "8")
			find_library(MKL_LIBRARIES
				NAMES mkl_solver mkl_intel mkl_gnu_thread mkl_core
				PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32 )	
		endif (CMAKE_SIZEOF_VOID_P MATCHES "8")	
	endif ( UNIX )

	# Set the include dir variables and the libraries and let libfind_process do the rest.
	# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
	if (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
		set(MKL_PROCESS_INCLUDES MKL_INCLUDE_DIR)
		set(MKL_PROCESS_LIBS MKL_LIBRARIES)
		libfind_process(MKL)
	else (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
		message (STATUS "Warning: MKL not found!")
	endif (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
	
endif (NOT MKL_FOUND)

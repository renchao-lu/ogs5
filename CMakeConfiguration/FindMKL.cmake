
if (NOT MKL_FOUND)

	include(LibFindMacros)
	
	find_path( MKL_INCLUDE_DIR
		NAMES mkl.h
		PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/include)

	if ( UNIX )
		find_library(MKL_SOLVER_LIBRARIES
			NAMES mkl_solver
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_solver.a )	
		find_library(MKL_INTEL_LIBRARIES
			NAMES mkl_intel
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_intel.a )	
		find_library(MKL_GNU_THREAD_LIBRARIES
			NAMES mkl_gnu_thread
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_gnu_thread.a )	
		find_library(MKL_CORE_LIBRARIES
			NAMES mkl_core
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_core.a )	
	else ( UNIX )			
		find_library(MKL_SOLVER_LIBRARIES
			NAMES mkl_solver
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_solver.a )	
		find_library(MKL_INTEL_LIBRARIES
			NAMES mkl_intel
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_intel.a )	
		find_library(MKL_GNU_THREAD_LIBRARIES
			NAMES mkl_gnu_thread
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_gnu_thread.a )	
		find_library(MKL_CORE_LIBRARIES
			NAMES mkl_core
			PATHS ${CMAKE_SOURCE_DIR}/LIB/MKL/32/libmkl_core.a )	
	endif ( UNIX )

	# Set the include dir variables and the libraries and let libfind_process do the rest.
	# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
	if (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
		set(MKL_PROCESS_INCLUDES MKL_INCLUDE_DIR)
		set(MKL_PROCESS_LIBS MKL_SOLVER_LIBRARIES)
		set(MKL_PROCESS_LIBS MKL_INTEL_LIBRARIES)
		set(MKL_PROCESS_LIBS MKL_GNU_THREAD_LIBRARIES)
		set(MKL_PROCESS_LIBS MKL_CORE_LIBRARIES)
		libfind_process(MKL)
	else (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
		message (STATUS "Warning: MKL not found!")
	endif (NOT MKL_LIBRARIES STREQUAL "MKL_LIBRARIES-NOTFOUND" AND NOT MKL_INCLUDE_DIR STREQUAL "MKL_INCLUDE_DIR-NOTFOUND")
	
endif (NOT MKL_FOUND)

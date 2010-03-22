## MDL: cmake options for compiling ogs with LIBPHREEQC

message (STATUS "Configuration: g++ and LIBPHREEQC")

if (UNIX)
	set (_POSIX_SOURCE ON)
	set (NO_ERROR_CONTROL ON)
	set (NEW_EQS ON)

	IF (LIBPQC_DEBUG)
   		MESSAGE (STATUS  "Compiling ogs and LIBPHREEQC with debug flags" )
		SET(CMAKE_CXX_FLAGS "-O0 -ggdb -Wall")
	ELSE (LIBPQC_DEBUG)
   		ADD_DEFINITIONS ( "-O3" )
	ENDIF (LIBPQC_DEBUG)

endif (UNIX)

set (PROBLEM_CLASS ON)
set (RANDOM_WALK ON)

set (LIBPQC ON)

##ADD_DEFINITIONS ( -DLIBPHREEQC )



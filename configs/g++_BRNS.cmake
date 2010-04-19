message (STATUS "Setting the g++_BRNS configuration")

if (WIN32)
	#message (FATAL_ERROR "ERROR: This configuration is not available on Windows!")
endif (WIN32)

if (UNIX)
	if (APPLE)
		
	else (APPLE)
		add_definitions(-pedantic)
	endif (APPLE)
	add_definitions(-fno-nonansi-builtins -DGCC)
endif (UNIX)

set (_POSIX_SOURCE ON)
set (NO_ERROR_CONTROL ON)
set (NEW_EQS ON)
set (RANDOM_WALK ON)
set (BRNS ON)
set (PROBLEM_CLASS ON)

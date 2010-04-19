message (STATUS "Setting the g++ configuration")

if (UNIX)
	if (APPLE)
		
	else (APPLE)
		add_definitions(-pedantic)
	endif (APPLE)
	add_definitions(-fno-nonansi-builtins)
endif (UNIX)

set (_POSIX_SOURCE ON)
set (NO_ERROR_CONTROL ON)
set (PROBLEM_CLASS ON)
set (RANDOM_WALK ON)

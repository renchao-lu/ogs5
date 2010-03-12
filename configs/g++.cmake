message (STATUS "Setting the g++ configuration")

if (UNIX)
	add_definitions(
		-pedantic
		-fno-nonansi-builtins
	)
endif (UNIX)

set (_POSIX_SOURCE ON)
set (NO_ERROR_CONTROL ON)
set (PROBLEM_CLASS ON)
set (RANDOM_WALK ON)

message (STATUS "Setting the g++_BRNS configuration")

if (UNIX)
	add_definitions(
		-pedantic
		-fno-nonansi-builtins
		-DGCC
	)
endif (UNIX)

set (_POSIX_SOURCE ON)
set (NO_ERROR_CONTROL ON)
set (NEW_EQS ON)
set (RANDOM_WALK ON)
set (BRNS ON)
set (PROBLEM_CLASS ON)

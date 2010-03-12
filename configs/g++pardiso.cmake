message (STATUS "Setting the g++ configuration")

if (UNIX)
	add_definitions(
		-O3 
		-fopenmp 
		-lpthread
	)
endif (UNIX)

#set (_POSIX_SOURCE ON)
set (NO_ERROR_CONTROL ON)
set (NEW_EQS ON)
set (PROBLEM_CLASS ON)
set (RANDOM_WALK ON)
#set (_OPENMP ON)
set (LIS ON)
set (MKL ON)

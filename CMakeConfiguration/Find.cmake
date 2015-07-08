############################
### Find OGS directories ###
############################

IF(DEFINED BENCHMARK_DIR)
	FIND_PATH (BENCHMARK_DIR_FOUND copy.py ${BENCHMARK_DIR})
ELSE()
	FIND_PATH (BENCHMARK_DIR_FOUND copy.py ${PROJECT_SOURCE_DIR}/../benchmarks)
ENDIF()

IF(DEFINED EXAMPLEDATA_DIR)
	FIND_PATH (EXAMPLEDATA_DIR_FOUND points.gli ${EXAMPLEDATA_DIR})
ELSE()
	FIND_PATH (EXAMPLEDATA_DIR_FOUND points.gli ${PROJECT_SOURCE_DIR}/../ExampleData)
ENDIF()

IF(DEFINED TESTDATA_DIR)
	FIND_PATH(TESTDATA_DIR_FOUND testdata.dummy ${TESTDATA_DIR})
ELSE()
	FIND_PATH(TESTDATA_DIR_FOUND testdata.dummy ${PROJECT_SOURCE_DIR}/../testdata)
ENDIF()

######################
### Find tools     ###
######################

# Find Python interpreter
FIND_PACKAGE (PythonInterp)

# Find Subversion
FIND_PACKAGE(Subversion)

# Find Git
FIND_PACKAGE(Git)

# msysGit on Windows
IF(WIN32 AND GIT_FOUND)
	FIND_PACKAGE(MsysGit)
ENDIF() # WIN32 AND GIT_FOUND

# Find dot tool from graphviz
FIND_PROGRAM(DOT_TOOL_PATH dot DOC "Dot tool from graphviz")

# Find doxygen
FIND_PACKAGE(Doxygen)

# Find gnu profiler gprof
FIND_PROGRAM(GPROF_PATH gprof DOC "GNU profiler gprof")

FIND_PACKAGE(cppcheck)

# Find Exuberant ctags or BBEdit for code completion
FIND_PROGRAM(CTAGS_TOOL_PATH ctags DOC "Exuberant ctags")
FIND_PROGRAM(BBEDIT_TOOL_PATH bbedit DOC "BBEdit Editor")
IF(BBEDIT_TOOL_PATH)
	ADD_CUSTOM_TARGET(ctags
		bbedit --maketags
		WORKING_DIRECTORY ${CMAKE_SOURCES_DIR}
		COMMENT "Creating tags..." VERBATIM
	)
	ADD_CUSTOM_COMMAND(TARGET ctags POST_BUILD
		COMMAND mv -f tags ../tags
		WORKING_DIRECTORY ${CMAKE_SOURCES_DIR}
		COMMENT "Moving tags..." VERBATIM
	)
ELSE()
	IF(CTAGS_TOOL_PATH)
		ADD_CUSTOM_TARGET(ctags
			ctags -R --fields=+iamS -f ${CMAKE_SOURCES_DIR}/../tags
			WORKING_DIRECTORY ${CMAKE_SOURCES_DIR}
			COMMENT "Creating tags..." VERBATIM
		)
	ENDIF()
ENDIF()

## Unix tools ##
# Date
FIND_PROGRAM(DATE_TOOL_PATH date PATHS ${MSYSGIT_BIN_DIR})
# Grep
FIND_PROGRAM(GREP_TOOL_PATH grep PATHS ${MSYSGIT_BIN_DIR})
# Unzip
FIND_PROGRAM(UNZIP_TOOL_PATH unzip PATHS ${MSYSGIT_BIN_DIR})

# Hide these variables for the CMake user
MARK_AS_ADVANCED(DOT_TOOL_PATH GPROF_PATH CTAGS_TOOL_PATH BBEDIT_TOOL_PATH
	UNZIP_TOOL_PATH
)
########################
### Find other stuff ###
########################

# Check if on Jenkins
IF(NOT $ENV{JENKINS_URL} STREQUAL "")
	SET(JENKINS_URL $ENV{JENKINS_URL})
	SET(JENKINS_JOB_NAME $ENV{JOB_NAME})
ENDIF()


######################
### Find libraries ###
######################
IF(OGS_FEM_PETSC OR OGS_NO_EXTERNAL_LIBS)
	RETURN()
ENDIF()

FIND_PATH (OGS_LIBS_DIR_FOUND geotiff.lib ${PROJECT_SOURCE_DIR}/../Libs/libgeotiff)

# Find precompiled libraries (for BRNS GEMS LIS)
FIND_PATH (OGS_PRECOMPILED_LIBS_DIR_FOUND BrnsDll.lib ${PROJECT_SOURCE_DIR}/../Libs/precompiled)
IF (OGS_PRECOMPILED_LIBS_DIR_FOUND)
	INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/../Libs/precompiled)
	LINK_DIRECTORIES (${PROJECT_SOURCE_DIR}/../Libs/precompiled)
ELSE (OGS_PRECOMPILED_LIBS_DIR_FOUND)
	IF (WIN32)
		IF (OGS_FEM_BRNS OR OGS_FEM_GEMS OR OGS_FEM_CHEMAPP)
			MESSAGE (FATAL_ERROR "Precompiled libraries not found! Make sure to also check out the trunk/Libs directory beneath your sources directory.")
		ENDIF (OGS_FEM_BRNS OR OGS_FEM_GEMS OR OGS_FEM_CHEMAPP)
	ELSE (WIN32)
		IF (OGS_FEM_LIS)
			MESSAGE (FATAL_ERROR "Precompiled libraries not found! Make sure to also check out the trunk/Libs directory beneath your sources directory.")
		ENDIF (OGS_FEM_LIS)
	ENDIF (WIN32)
ENDIF (OGS_PRECOMPILED_LIBS_DIR_FOUND)


## pthread ##
SET ( CMAKE_THREAD_PREFER_PTHREAD ON CACHE BOOL "" )
FIND_PACKAGE( Threads )
IF ( CMAKE_USE_PTHREADS_INIT AND NOT HAVE_PTHREADS)
	SET (HAVE_PTHREADS TRUE CACHE BOOL "Is PThreads found.")
	MESSAGE (STATUS "pthread library found." )
ENDIF ()
IF(HAVE_PTHREADS)
  ADD_DEFINITIONS(-DHAVE_PTHREADS)
ENDIF()
MARK_AS_ADVANCED(CMAKE_THREAD_PREFER_PTHREAD)

## boost (see FindBoost.cmake for more options) ##
##kg44 this configuration works for boost and petsc on a cray
OPTION(Boost_USE_STATIC_LIBS "" ON)
OPTION(Boost_USE_MULTITHREADED "" ON)
OPTION(Boost_USE_STATIC_RUNTIME "" ON)
MARK_AS_ADVANCED(Boost_USE_STATIC_LIBS Boost_USE_MULTITHREADED Boost_USE_STATIC_RUNTIME)

IF(NOT OGS_FEM_GEMS AND NOT OGS_FEM_PETSC_GEMS)
	IF(NOT OGS_DONT_USE_BOOST)
		FIND_PACKAGE( Boost 1.50.0 COMPONENTS filesystem system regex)
	ENDIF()
ELSE()
	# Boost with threads is required for GEMS
	FIND_PACKAGE(Boost 1.50.0 COMPONENTS system thread REQUIRED)
        MESSAGE(STATUS "** Boost root: ${BOOST_ROOT}")
        MESSAGE(STATUS "** Boost include: ${Boost_INCLUDE_DIR}")
        MESSAGE(STATUS "** Boost libraries: ${Boost_LIBRARY_DIRS}")
        MESSAGE(STATUS "** Boost libraries: ${Boost_LIBRARIES}")
ENDIF()

IF(OGS_FEM_MKL)
	# Find MKLlib
	FIND_PACKAGE( MKL REQUIRED )
	INCLUDE_DIRECTORIES (${MKL_INCLUDE_DIR})
ENDIF()

IF(OGS_FEM_LIS OR OGS_FEM_MKL)
	# Find LISlib
	FIND_PACKAGE( LIS REQUIRED )
	set (NEW_EQS ON)
	add_definitions(
		-o3
		-DIPMGEMPLUGIN
	)
ENDIF()

# Find OpenMP
IF(PARALLEL_USE_OPENMP)
	FIND_PACKAGE( OpenMP REQUIRED )
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
	SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
	SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgomp" )
ENDIF(PARALLEL_USE_OPENMP)

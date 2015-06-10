set ( SOURCES
	Density.cpp
	DiffAL.cpp
	DiffOH.cpp
	eqbrm.cpp
	export.cpp
	Fluid.cpp
	HKF.cpp
	IAPWS-IF97.cpp
	IO.cpp
	logk.cpp
	NR.cpp
	PDE.cpp
	PITZcalc.cpp
	PITZdata.cpp
	VLE.cpp
)

# Create the library
add_library( GCC STATIC ${SOURCES} )
include_directories( ${CMAKE_SOURCE_DIR}/GCC )

IF (WIN32)
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPMGEMPLUGIN")
ENDIF (WIN32)

IF (UNIX)
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPMGEMPLUGIN -D__unix")
ENDIF (UNIX)

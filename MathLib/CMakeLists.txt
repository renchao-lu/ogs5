# Source files
set( HEADERS
	Vector3.h
	Matrix.h
	MathTools.h
	AnalyticalGeometry.h
	LinkedTriangle.h
)

set( SOURCES
	MathTools.cpp
	AnalyticalGeometry.cpp
	LinkedTriangle.cpp
)

# Create the library
add_library( MathLib STATIC ${HEADERS} ${SOURCES} )

set_target_properties(MathLib PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
	../Base
	../GEO
	.
)

# Source files
set( HEADERS
	Configure.h.in
	DateTools.h
	StringTools.h
	MathTools.h
)

set( SOURCES
	MathTools.cpp
)

# Create the library
add_library( Base STATIC ${HEADERS} ${SOURCES} )

set_target_properties(Base PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
	../GEO
	.
)

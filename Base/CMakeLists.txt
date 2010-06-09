# Source files
set( HEADERS
	Configure.h.in
	DateTools.h
	StringTools.h
	swap.h
	FileTools.h
)

set( SOURCES
	dummy.cpp
)

# Create the library
add_library( Base STATIC ${HEADERS} ${SOURCES} )

set_target_properties(Base PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
	../GEO
	.
)

IF(OGS_BUILD_UTILITIES)
	# Create utility executable
	add_executable( correctScientificNotation CorrectScientificNotationMain.cpp )
ENDIF(OGS_BUILD_UTILITIES)
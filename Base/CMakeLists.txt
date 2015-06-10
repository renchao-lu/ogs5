set( SOURCES
	binarySearch.cpp
	break.cpp
	DateTools.cpp
	display.cpp
	FileTools.cpp
	makros.cpp
	memory.cpp
	MemWatch.cpp
	StringTools.cpp
	timer.cpp
)

# Create the library
add_library( Base STATIC ${HEADERS} ${SOURCES} )

set_target_properties(Base PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
	${CMAKE_SOURCE_DIR}/Base
)

if(OGS_BUILD_CORRECT_SCIENTIFIC)
	# Create utility executable
	add_executable( correctScientificNotation CorrectScientificNotationMain.cpp )
endif(OGS_BUILD_CORRECT_SCIENTIFIC)

# Source files
set( SOURCES
	GocadInterface.cpp	
	OGSIOVer4.cpp
	PetrelInterface.cpp	
	StationIO.cpp
)

# Header files
set( HEADERS
	GocadInterface.h
	OGSIOVer4.h
	PetrelInterface.h
	StationIO.h
)

if (Shapelib_FOUND)
	set( HEADERS ${HEADERS} SHPInterface.h )
	set (SOURCES ${SOURCES} SHPInterface.cpp )
	include_directories (${Shapelib_INCLUDE_DIR})
endif (Shapelib_FOUND)

if (QT4_FOUND)
	set( HEADERS ${HEADERS} XMLInterface.h )
	set (SOURCES ${SOURCES} XMLInterface.cpp )
endif (QT4_FOUND)

# Create the library
add_library( FileIO STATIC
	${SOURCES}
	${HEADERS}
)

include_directories(
	.
	../Base
	../GEO
	../MathLib
)

target_link_libraries( FileIO
	Base
	MathLib
)

# Source files
set( SOURCES
	GocadInterface.cpp	
	OGSIOVer4.cpp
	PetrelInterface.cpp
	SHPInterface.cpp
	XMLInterface.cpp
)

# Header files
set( HEADERS
	GocadInterface.h
	OGSIOVer4.h
	PetrelInterface.h
	SHPInterface.h
	XMLInterface.h
)

# Create the library
add_library( FileIO STATIC
	${SOURCES}
	${HEADERS}
)


include_directories(
	.
	../Base
	../GEO
	${Shapelib_INCLUDE_DIR}	
)

target_link_libraries( FileIO
	Base
)

# Source files
set( SOURCES
	GMSHInterface.cpp
	GMSInterface.cpp
	GocadInterface.cpp
	OGSIOVer4.cpp
	PetrelInterface.cpp	
	StationIO.cpp
)

# Header files
set( HEADERS
	GMSHInterface.h
	GMSInterface.h
	GocadInterface.h
	OGSIOVer4.h
	PetrelInterface.h
	StationIO.h
)

if (OGS_USE_NETCDF)
	find_package( VTK REQUIRED )
	INCLUDE( ${VTK_USE_FILE} )
	set (HEADERS NetCDFInterface.h ${HEADERS})
	set (SOURCES NetCDFInterface.cpp ${SOURCES})
endif (OGS_USE_NETCDF)

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
	../FEM
	../GEO
	../MathLib
	../MSH
)

target_link_libraries( FileIO
	Base
	MathLib
	GEO
)

if (Shapelib_FOUND)
	target_link_libraries( FileIO ${Shapelib_LIBRARIES} )
endif (Shapelib_FOUND)

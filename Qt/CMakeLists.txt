
# Visual Studio settings
if (MSVC)
	add_definitions(
		-D/MP	# For multicore compiling
	)
endif (MSVC)


# Find VTK
find_package( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

# Find libgeotiff
find_package( LibTiff REQUIRED )
find_package( LibGeoTiff REQUIRED )

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /ZI /Od /Ob0")

# Add subprojects
if (OGS_COMPILE_QVTK)
	add_subdirectory( QVTK )
endif (OGS_COMPILE_QVTK)
add_subdirectory( Base )
add_subdirectory( DataView )
add_subdirectory( StationView )
add_subdirectory( StationView/DiagramView )
add_subdirectory( StationView/StratView )
add_subdirectory( VtkVis )
add_subdirectory( VtkAct )
add_subdirectory( Gui )

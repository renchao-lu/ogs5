
# Visual Studio settings
if (MSVC)
	add_definitions(
		-D/MP	# For multicore compiling
	)
endif (MSVC)


# Find VTK
#if (NOT UNIX)
	#set ( VTK_DIR ${PROJECT_SOURCE_DIR}/../Libs/vtk/VTK/Build )
#endif (NOT UNIX)
find_package( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

# Find libgeotiff
find_package( LibTiff REQUIRED )
find_package( LibGeoTiff REQUIRED )

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /ZI /Od /Ob0")

# Add subprojects
add_subdirectory( Base )
add_subdirectory( DataView )
add_subdirectory( DiagramView )
add_subdirectory( StationView )
add_subdirectory( VtkVis )
add_subdirectory( Gui )

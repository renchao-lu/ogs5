
# By default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
set( QT_USE_QTOPENGL TRUE )
set( QT_USE_QTSQL TRUE )
set( QT_USE_QTTEST TRUE )
set( QT_USE_QTXMLPATTERNS TRUE )

# Visual Studio settings
if (MSVC)
	add_definitions(
		-D/MP
	)
endif (MSVC)


# Find VTK
if (NOT UNIX)
	set ( VTK_DIR ${PROJECT_SOURCE_DIR}/../Libs/vtk/VTK/Build )
endif (NOT UNIX)
find_package( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

# Find libgeotiff
find_package( LibTiff REQUIRED )
find_package( LibGeoTiff REQUIRED )

# Adds useful macros and variables
# this is needed to correctly link the qt libraries through target_link_libraries
INCLUDE( ${QT_USE_FILE} )

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /ZI /Od /Ob0")

# Add subprojects
add_subdirectory( Base )
add_subdirectory( DataView )
add_subdirectory( DataViewTest )
add_subdirectory( DiagramView )
add_subdirectory( StationView )
add_subdirectory( VtkVis )

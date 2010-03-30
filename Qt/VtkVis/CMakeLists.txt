# Source files
set( SOURCES
	GridAdapter.cpp
	VtkAddFilterDialog.cpp
	VtkMeshSource.cpp
	VtkPolylinesSource.cpp
	VtkPointsSource.cpp
	VtkStationSource.cpp
	VtkSurfacesSource.cpp
	VtkVisPipeline.cpp
	VtkVisPipelineItem.cpp
	VtkVisPipelineView.cpp
	VtkVisTabWidget.cpp
)

# Moc headers
set( MOC_HEADERS
	VtkAddFilterDialog.h
	VtkVisPipeline.h
	VtkVisPipelineView.h
	VtkVisTabWidget.h
)

# Header files
set( HEADERS
	GridAdapter.h
	VtkAlgorithmProperties.h
	VtkMeshSource.h
	VtkPolylinesSource.h
	VtkPointsSource.h
	VtkStationSource.h
	VtkSurfacesSource.h
	VtkVisPipelineItem.h
)

# UI files
set ( UIS
	VtkVisTabWidgetBase.ui
	VtkAddFilterDialogBase.ui
)

# Put moc files in a project folder
source_group("UI Files" REGULAR_EXPRESSION "\\w*\\.ui")
source_group("Moc Files" REGULAR_EXPRESSION "moc_.*")

# Run Qts user interface compiler uic on .ui files
qt4_wrap_ui( UI_HEADERS ${UIS} )

# Run Qts meta object compiler moc on header files
qt4_wrap_cpp( MOC_SOURCES ${MOC_HEADERS} )

include_directories(
	.
	${CMAKE_BINARY_DIR}/Qt/VtkVis
	../../Base
	../../FEM
	../../GEO
	../../MathLib
	../../MSH
	../Base
	../DataView
	../StationView
)

# Create the library
add_library( VtkVis STATIC
	${SOURCES}
	${HEADERS}
	${MOC_HEADERS}
	${MOC_SOURCES}
	${UIS}
)

target_link_libraries( VtkVis ${QT_LIBRARIES} )

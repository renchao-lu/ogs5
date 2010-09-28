# Source files
SET( SOURCES
	ColorLookupTable.cpp
	GridAdapter.cpp
	VtkAddFilterDialog.cpp
	VtkBGImageSource.cpp
	VtkColorByHeightFilter.cpp
	VtkMeshSource.cpp
	VtkOGSFilter.cpp
	VtkTextureOnSurfaceFilter.cpp
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
SET( MOC_HEADERS
	VtkAddFilterDialog.h
	VtkVisPipeline.h
	VtkVisPipelineView.h
	VtkVisTabWidget.h
)

# Header files
SET( HEADERS
	ColorLookupTable.h
	GridAdapter.h
	VtkAlgorithmProperties.h	
	VtkBGImageSource.h
	VtkColorByHeightFilter.h
	VtkMeshSource.h
	VtkOGSFilter.h
	VtkTextureOnSurfaceFilter.h
	VtkPolylinesSource.h
	VtkPointsSource.h
	VtkStationSource.h
	VtkSurfacesSource.h
	VtkVisPipelineItem.h
)

IF (OGS_USE_OPENSG)
	SET (SOURCES ${SOURCES} vtkOsgActor.cpp)
	SET (HEADERS ${HEADERS} vtkOsgActor.h)
ENDIF (OGS_USE_OPENSG)

# UI files
SET ( UIS
	VtkVisTabWidgetBase.ui
	VtkAddFilterDialogBase.ui
)

# Put moc files in a project folder
SOURCE_GROUP("UI Files" REGULAR_EXPRESSION "\\w*\\.ui")
SOURCE_GROUP("Moc Files" REGULAR_EXPRESSION "moc_.*")

# Run Qts user interface compiler uic on .ui files
QT4_WRAP_UI( UI_HEADERS ${UIS} )

# Run Qts meta object compiler moc on header files
QT4_WRAP_CPP( MOC_SOURCES ${MOC_HEADERS} )

INCLUDE_DIRECTORIES(
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
ADD_LIBRARY( VtkVis STATIC
	${SOURCES}
	${HEADERS}
	${MOC_HEADERS}
	${MOC_SOURCES}
	${UIS}
)

TARGET_LINK_LIBRARIES( VtkVis ${QT_LIBRARIES} )

IF (OGS_USE_OPENSG)
	INCLUDE_DIRECTORIES (${OpenSG_INCLUDE_DIRS})
	TARGET_LINK_LIBRARIES (VtkVis ${OpenSG_LIBRARIES})
ENDIF (OGS_USE_OPENSG)

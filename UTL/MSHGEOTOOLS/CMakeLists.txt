
INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/FEM
	${CMAKE_SOURCE_DIR}/FileIO
	${CMAKE_SOURCE_DIR}/FileIO/MeshIO
	${CMAKE_SOURCE_DIR}/GEO
	${CMAKE_SOURCE_DIR}/MathLib
	${CMAKE_SOURCE_DIR}/MSH
	${CMAKE_SOURCE_DIR}/MSHGEOTOOLS
	${CMAKE_SOURCE_DIR}/UTL/MSHGEOTOOLS
)

# Create executables
ADD_EXECUTABLE( ExtractMeshNodes mainExtractMeshNodes.cpp )
ADD_EXECUTABLE( ExtractMeshNodeIDs mainExtractMeshNodeIDs.cpp )
ADD_EXECUTABLE( ModifyMeshProperties mainModifyMeshProperties.cpp ModifyMeshProperties.cpp )
ADD_EXECUTABLE( filterMeshNodes filterMeshNodes.cpp )
ADD_EXECUTABLE( convertGLIVerticalSurfaceToPolygon mainConvertGLIVerticalSurfaceToPolygon.cpp )

SET_TARGET_PROPERTIES(ExtractMeshNodeIDs ExtractMeshNodes ModifyMeshProperties filterMeshNodes convertGLIVerticalSurfaceToPolygon
  PROPERTIES FOLDER Utilities)

TARGET_LINK_LIBRARIES( ExtractMeshNodeIDs
	FEM
	FileIO
	GEO
	MSH
	MSHGEOTOOLS
	OGSProject
)

TARGET_LINK_LIBRARIES( ExtractMeshNodes
	FEM
	FileIO
	GEO
	MSH
	MSHGEOTOOLS
	OGSProject
)


TARGET_LINK_LIBRARIES( convertGLIVerticalSurfaceToPolygon
	FEM
	FileIO
	GEO
	MSH
	MSHGEOTOOLS
	OGSProject
)


TARGET_LINK_LIBRARIES( ModifyMeshProperties
	FEM
	FileIO
	GEO
	MathLib
	MSH
	MSHGEOTOOLS
	OGSProject
)

TARGET_LINK_LIBRARIES( filterMeshNodes
	Base
	MSH
)

ADD_EXECUTABLE( testMeshSearchAlgorithms testMeshSearchAlgorithms.cpp )
TARGET_LINK_LIBRARIES( testMeshSearchAlgorithms
	Base
	FileIO
	GEO
	MSH
	MSHGEOTOOLS
)

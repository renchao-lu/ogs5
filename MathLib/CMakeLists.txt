set( SOURCES
	AnalyticalGeometry.cpp
	EarClippingTriangulation.cpp
	InterpolationAlgorithms/CubicSpline.cpp
	InterpolationAlgorithms/PiecewiseLinearInterpolation.cpp
	LinAlg/TriangularSolve.cpp
	LinkedTriangle.cpp
	MathTools.cpp
)

if(OGS_FEM_PETSC OR OGS_FEM_PETSC_GEMS)
	set( SOURCES ${SOURCES} PETSC/PETScLinearSolver.cpp)
endif()

add_library( MathLib STATIC ${SOURCES} )

set_target_properties(MathLib PROPERTIES LINKER_LANGUAGE CXX)

include_directories(
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/GEO
	${CMAKE_SOURCE_DIR}/MathLib
	${CMAKE_SOURCE_DIR}/MathLib/LinAlg
	${CMAKE_SOURCE_DIR}/MathLib/PETSC
)

target_link_libraries (
	MathLib
	Base
)


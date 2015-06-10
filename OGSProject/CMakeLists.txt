set( SOURCES
	gs_project.cpp
)

# and include additional header
include_directories(
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/FEM
	${CMAKE_SOURCE_DIR}/FileIO
	${CMAKE_SOURCE_DIR}/GEO
	${CMAKE_SOURCE_DIR}/MathLib
	${CMAKE_SOURCE_DIR}/MSH
)


# Create the library
add_library( OGSProject ${SOURCES} )

target_link_libraries( OGSProject FEM )

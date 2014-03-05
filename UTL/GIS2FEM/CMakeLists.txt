##-------------------------------------------------------
##          
##    Tool to convert GIS shape data into FEM data
##                By Wenqing Wang
##
##                     03.2010        
##-------------------------------------------------------

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/UTL/MSHGEOTOOLS
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/GEO
	${CMAKE_SOURCE_DIR}/FileIO
	${CMAKE_SOURCE_DIR}/MathLib
	${CMAKE_SOURCE_DIR}/FEM
	${CMAKE_SOURCE_DIR}/MSH
	${CMAKE_SOURCE_DIR}/MSHGEOTOOLS
)

SET(FEM_PATH ${CMAKE_SOURCE_DIR}/FEM)
SET(MSH_PATH ${CMAKE_SOURCE_DIR}/MSH)
SET(GEO_PATH ${CMAKE_SOURCE_DIR}/GEO)
SET(MATH_PATH ${CMAKE_SOURCE_DIR}/MathLib)
SET(BASE_PATH ${CMAKE_SOURCE_DIR}/Base)

ADD_DEFINITIONS(-DNONFLUID_MOMENTUM 
		-DNON_GEO
		-DNON_PROCESS
		-DNEW_EQS)

SET(FEM_SOURCES  
   ${FEM_PATH}/fem_ele.cpp 
   ${FEM_PATH}/femlib.cpp
   ${FEM_PATH}/mathlib.cpp 
   ${FEM_PATH}/matrix_class.cpp 
   ${FEM_PATH}/matrix_class.h
   ${FEM_PATH}/fem_ele.h
   ${FEM_PATH}/mathlib.h    
)
SET(MSH_SOURCES     
   ${MSH_PATH}/msh_core.cpp  
   ${MSH_PATH}/msh_edge.cpp
   ${MSH_PATH}/msh_node.cpp 
   ${MSH_PATH}/msh_elem.cpp 
   ${MSH_PATH}/msh_mesh.cpp 
   ${MSH_PATH}/MSHEnums.cpp
   ${MSH_PATH}/msh_core.h  
   ${MSH_PATH}/msh_edge.h
   ${MSH_PATH}/msh_node.h 
   ${MSH_PATH}/msh_elem.h 
   ${MSH_PATH}/msh_mesh.h 
   ${MSH_PATH}/MSHEnums.h
  )
  
  
SET(GIS2FEM_SOURCES main.cpp misc.cpp misc.h ${FEM_SOURCES} ${MSH_SOURCES} )
    
add_executable(GIS2FEM  ${GIS2FEM_SOURCES})

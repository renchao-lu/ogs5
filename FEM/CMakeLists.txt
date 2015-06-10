set( SOURCES
	BoundaryCondition.cpp
	CAP_IO.cpp
	conversion_rate.cpp
	DistributionInfo.cpp
	DUMUX.cpp
	Eclipse.cpp
	eos.cpp
	fem_ele.cpp
	fem_ele_std.cpp
	fem_ele_std1.cpp
	fem_ele_std_tes.cpp
	fem_ele_std_tneq.cpp
	fem_ele_vec.cpp
	FEMCondition.cpp
	FEMEnums.cpp
	femlib.cpp
	files0.cpp
	GeoInfo.cpp
	InitialCondition.cpp
	LinearFunctionData.cpp
	mathlib.cpp
	matrix_class.cpp
	Output.cpp
	pcs_dm.cpp
	problem.cpp
	ProcessInfo.cpp
	rf_bc_new.cpp
	rf_fct.cpp
	rf_fluid_momentum.cpp
	rf_ic_new.cpp
	rf_kinreact.cpp
	rf_mfp_new.cpp
	rf_mmp_new.cpp
	rf_msp_new.cpp
	rf_node.cpp
	rf_num_new.cpp
	rf_out_new.cpp
	rf_pcs.cpp
	rf_random_walk.cpp
	rf_react.cpp
	rf_react_cap.cpp
	rf_react_int.cpp
	rf_st_new.cpp
	rf_tim_new.cpp
	rfmat_cp.cpp
	SourceTerm.cpp
	SparseMatrixDOK.cpp
	Stiff_Bulirsch-Stoer.cpp
	tools.cpp
	vtk.cpp
)

IF(OGS_FEM_PETSC OR OGS_FEM_PETSC_GEMS)
	SET( SOURCES ${SOURCES} rf_pcs1.cpp fct_mpi.cpp)
ELSE(OGS_FEM_PETSC OR OGS_FEM_PETSC_GEMS)
	SET( SOURCES ${SOURCES} par_ddc.cpp)
	## New sparse class. WW
	IF(OGS_USE_NEW_EQS)
		SET( SOURCES ${SOURCES} equation_class.cpp )
		IF(OGS_FEM_JFNK)
			SET( SOURCES ${SOURCES} rf_pcs1.cpp )
		ENDIF(OGS_FEM_JFNK)
	ELSE()
		SET( SOURCES ${SOURCES} solver.cpp matrix_routines.cpp)
	ENDIF()
ENDIF()

IF(OGS_FEM_BRNS)
	SET( SOURCES ${SOURCES} rf_REACT_BRNS.cpp )
ENDIF(OGS_FEM_BRNS)

IF(OGS_FEM_GEMS OR OGS_FEM_PETSC_GEMS)
	SET( SOURCES ${SOURCES} rf_REACT_GEM.cpp )
ENDIF(OGS_FEM_GEMS OR OGS_FEM_PETSC_GEMS)


include_directories(
	${CMAKE_SOURCE_DIR}/Base
	${CMAKE_SOURCE_DIR}/FEM
	${CMAKE_SOURCE_DIR}/FileIO
	${CMAKE_SOURCE_DIR}/FileIO/FEMIO
	${CMAKE_SOURCE_DIR}/FileIO/MeshIO
	${CMAKE_SOURCE_DIR}/GCC
	${CMAKE_SOURCE_DIR}/GEM
	${CMAKE_SOURCE_DIR}/GEO
	${CMAKE_SOURCE_DIR}/MathLib
	${CMAKE_SOURCE_DIR}/MSH
	${CMAKE_SOURCE_DIR}/OGSProject
)

IF (LIS)
	INCLUDE_DIRECTORIES(${LIS_INCLUDE_DIR})
ENDIF (LIS)

IF (MKL)
	INCLUDE_DIRECTORIES(${MKL_INCLUDES})
ENDIF (MKL)

IF(OGS_FEM_PQC)
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ThirdParty/PQC)
ENDIF (OGS_FEM_PQC)

IF(OGS_FEM_IPQC)
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ThirdParty/IPQC/src)
ENDIF (OGS_FEM_IPQC)

# Create the library
add_library( FEM STATIC ${SOURCES} )

target_link_libraries(FEM
	Base
	FileIO
	GCC
	GEO
	MSH
	OGSProject
)

IF(OGS_FEM_PQC)
	target_link_libraries(FEM PQC)
ENDIF(OGS_FEM_PQC)

if (WIN32)
	if (OGS_FEM_GEMS)
		target_link_libraries( FEM GEM )
	endif (OGS_FEM_GEMS)
	if (LIS)
		target_link_libraries( FEM ${LIS_LIBRARIES} )
		set_target_properties( FEM PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt" )
	endif (LIS)
	if (MKL)
		target_link_libraries( FEM ${MKL_LIBRARIES} )
	endif (MKL)
	if (OGS_FEM_BRNS)
		target_link_libraries( FEM BrnsDll )
	endif (OGS_FEM_BRNS)
	if (OGS_FEM_CHEMAPP)
		target_link_libraries( FEM ca_vc_e )
	endif (OGS_FEM_CHEMAPP)
endif (WIN32)

if (UNIX)
	if (OGS_FEM_BRNS)
		target_link_libraries( FEM dl ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
	endif (OGS_FEM_BRNS)

	if (LIS)
		target_link_libraries( FEM ${LIS_LIBRARIES} )
	endif (LIS)

	if (MKL)
		target_link_libraries( FEM ${MKL_LIBRARIES} )
	endif (MKL)
endif (UNIX)

IF (OGS_FEM_CHEMAPP)
	INCLUDE_DIRECTORIES ( ${CMAKE_SOURCE_DIR}/EQL )
	TARGET_LINK_LIBRARIES ( FEM EQL )
ENDIF (OGS_FEM_CHEMAPP)


IF (OGS_FEM_CAP)
	INCLUDE_DIRECTORIES (  ${CMAKE_SOURCE_DIR}/CMP )
	TARGET_LINK_LIBRARIES (
		FEM
		GEO
		MSH
		CMP
	)
	IF (UNIX)
		TARGET_LINK_LIBRARIES (
			FEM
			ChemAppCS_64
			EChemAppS_64
		)
	ENDIF (UNIX)
ENDIF (OGS_FEM_CAP)



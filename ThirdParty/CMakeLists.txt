
if(OGS_FEM_IPQC)
	add_subdirectory(IPQC)
endif()

if(OGS_FEM_GEMS OR OGS_FEM_PETSC_GEMS)
	add_subdirectory(GEM)
endif(OGS_FEM_GEMS OR OGS_FEM_PETSC_GEMS)

if(OGS_FEM_PQC)
	add_subdirectory ( PQC )
endif(OGS_FEM_PQC)

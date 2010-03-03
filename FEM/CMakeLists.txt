# Source files

set( SOURCES
	Stiff_Bulirsch-Stoer.h
	break.h
	display.h
	eos.h
	equation_class.h
	fem_ele.h
	fem_ele_std.h
	fem_ele_vec.h
	femlib.h
	files0.h
	#intrface.h
	makros.h
	mathlib.h
	matrix.h
	matrix_class.h
	memory.h
	par_ddc.h
	pcs_dm.h
	problem.h
	prototyp.h
	rf_REACT_BRNS.h
	rf_REACT_GEM.h
	rf_bc_new.h
	rf_fct.h
	rf_fluid_momentum.h
	rf_ic_new.h
	rf_kinreact.h
	rf_mfp_new.h
	rf_mmp_new.h
	rf_msp_new.h
	rf_node.h
	rf_num_new.h
	rf_out_new.h
	rf_pcs.h
	rf_random_walk.h
	rf_react.h
	rf_st_new.h
	rf_tim_new.h
	rfmat_cp.h
	solver.h
	timer.h
	tools.h
	vtk.h
	Stiff_Bulirsch-Stoer.cpp
	break.cpp
	display.cpp
	eos.cpp
	equation_class.cpp
	fem_ele.cpp
	fem_ele_std.cpp
	fem_ele_vec.cpp
	femlib.cpp
	files0.cpp
	#intrface.cpp
	mathlib.cpp
	matrix.cpp
	matrix_class.cpp
	memory.cpp
	par_ddc.cpp
	pcs_dm.cpp
	problem.cpp
	#rf.cpp							RockFlow-main()
	rf_REACT_BRNS.cpp
	rf_REACT_GEM.cpp
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
	rf_st_new.cpp
	rf_tim_new.cpp
	rfmat_cp.cpp
	solver.cpp
	timer.cpp
	tools.cpp
	vtk.cpp
	
	../gs_project.h
	../gs_project.cpp
)

include_directories(
	.
	../Base
	../GEO
	../MSH
	../GEM
)

# Create the library
add_library( FEM STATIC ${SOURCES} )

target_link_libraries( FEM
	GEO
	MSH
)

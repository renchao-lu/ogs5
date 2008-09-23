# GeoSys - Makefile for LINUX
# 4.5.06 22/07/2007 WW
# 4.7.06 15/09/2008 WW
FEMOBJS =tools.o\
timer.o\
steam.o\
solver.o\
rfstring.o\
rfmat_cp.o\
rfiter.o\
rfadt.o\
rf_vel_new.o\
rf_tim_new.o\
rf_st_new.o\
rf_react.o\
rf_REACT_GEM.o\
rf_REACT_BRNS.o\
rf_random_walk.o\
rf_pcs.o\
rf_out_new.o\
rf_num_new.o\
rf_node.o\
rf_msp_new.o\
rf_mmp_new.o\
rf_mfp_new.o\
rf_ic_new.o\
rf_fluid_momentum.o\
rf_fct.o\
rf_db.o\
rf_bc_new.o\
rf_apl.o\
rf.o\
renumber.o\
ptrarr.o\
pcs_dm.o\
par_ddc.o\
nodes.o\
memory.o\
matrix_class.o\
matrix.o\
mathlib.o\
loop_pcs.o\
files0.o\
femlib.o\
fem_ele_vec.o\
fem_ele_std.o\
fem_ele.o\
equation_class.o\
elements.o\
eos.o\
display.o\
break.o\
../gs_project.o

ifndef NO_RANDOM_WALK
FEMOBJS += rf_random_walk.o 
endif




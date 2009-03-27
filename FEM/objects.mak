# GeoSys - Makefile for LINUX
# 4.5.06 22/07/2007 WW
# 4.7.06 15/09/2008 WW
#        15/10/2008 WW
# 4.9    17/12/2008 WW
FEMOBJS = break.o\
display.o\
elements.o\
eos.o\
equation_class.o\
fem_ele.o\
fem_ele_std.o\
fem_ele_vec.o\
femlib.o\
files0.o\
mathlib.o\
matrix.o\
matrix_class.o\
memory.o\
nodes.o\
par_ddc.o\
pcs_dm.o\
problem.o\
ptrarr.o\
rf.o\
rf_bc_new.o\
rf_db.o\
rf_fct.o\
rf_fluid_momentum.o\
rf_ic_new.o\
rf_kinreact.o\
rf_mfp_new.o\
rf_mmp_new.o\
rf_msp_new.o\
rf_node.o\
rf_num_new.o\
rf_out_new.o\
rf_pcs.o\
rf_random_walk.o\
rf_react.o\
rf_REACT_GEM.o\
rf_st_new.o\
rf_tim_new.o\
rfadt.o\
rfmat_cp.o\
rfstring.o\
solver.o\
Stiff_Bulirsch-Stoer.o\
timer.o\
tools.o\
../gs_project.o

ifndef NO_RANDOM_WALK
FEMOBJS += rf_random_walk.o 
endif




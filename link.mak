FEMOBJS =FEM/tools.o\
         FEM/timer.o\
         FEM/testvar.o\
         FEM/steam.o\
         FEM/solver.o\
         FEM/rfsystim.o\
         FEM/rfstring.o\
         FEM/rfpriref.o\
         FEM/rfod.o\
         FEM/rfmat_cp.o\
         FEM/rfiv.o\
         FEM/rfiter.o\
         FEM/rfim.o\
         FEM/rfii.o\
         FEM/rfidb.o\
         FEM/rfhgm.o\
         FEM/rfdb.o\
         FEM/rfadt.o\
         FEM/rf_vel_new.o\
         FEM/rf_tim_new.o\
         FEM/rf_st_new.o\
         FEM/rf_react.o\
         FEM/rf_pcs.o\
         FEM/rf_out_new.o\
         FEM/rf_num_new.o\
         FEM/rf_node.o\
         FEM/rf_msp_new.o\
         FEM/rf_mmp_new.o\
         FEM/rf_mg.o\
         FEM/rf_mfp_new.o\
         FEM/rf_ic_new.o\
         FEM/rf_fct.o\
         FEM/rf_db.o\
         FEM/rf_bc_new.o\
         FEM/rf_apl.o\
FEM/rf_random_walk.o\
FEM/rf_fluid_momentum.o\
FEM/rf.o\
FEM/renumber.o\
FEM/relax.o\
FEM/refine3.o\
FEM/refine2.o\
FEM/refine1.o\
FEM/ptrarr.o\
FEM/plot.o\
FEM/plains.o\
FEM/pcs_dm.o\
FEM/par_ddc.o\
FEM/nodes.o\
FEM/memory.o\
FEM/matrix_class.o\
FEM/matrix.o\
FEM/mathlib.o\
FEM/masscont.o\
FEM/loop_pcs.o\
FEM/int_mtm2.o\
FEM/int_mms.o\
FEM/int_mmp.o\
FEM/int_htm.o\
FEM/int_ge.o\
FEM/int_asm.o\
FEM/int_agm.o\
FEM/indicatr.o\
FEM/indicat2.o\
FEM/indicat1.o\
FEM/gridadap.o\
FEM/files2.o\
FEM/files1.o\
FEM/files0.o\
FEM/file_rfr.o\
FEM/femlib.o\
FEM/fem_ele_vec.o\
FEM/fem_ele_std.o\
FEM/fem_ele.o\
FEM/elements.o\
FEM/edges.o\
FEM/display.o\
FEM/cvel.o\
FEM/cgs_mtm2.o\
FEM/cgs_mpc.o\
FEM/cgs_mms.o\
FEM/cgs_mmp.o\
FEM/cgs_htm.o\
FEM/cgs_ge.o\
FEM/cgs_asm.o\
FEM/cgs_agm.o\
FEM/cel_mtm2.o\
FEM/cel_mpc.o\
FEM/cel_mms.o\
FEM/cel_mmp.o\
FEM/cel_htm.o\
FEM/cel_asm.o\
FEM/cel_agm.o\
FEM/break.o\
FEM/balance.o\
FEM/adaptiv.o\
FEM/gs_project.o

GEOOBJS = GEO/geo_dom.o GEO/geo_lin.o GEO/geo_ply.o \
          GEO/geo_sfc.o  GEO/geo_vol.o\
          GEO/geo_lib.o  GEO/geo_mathlib.o \
          GEO/geo_pnt.o  GEO/geo_strings.o 

MSHOBJS =  MSH/moveGEOtoMSH.o  MSH/msh_elements_rfi.o  MSH/msh_lin.o   MSH/msh_nodes_rfi.o\
        MSH/msh_core.o      MSH/msh_gen.o           MSH/msh_mat.o   MSH/msh_pnt.o\
        MSH/msh_edge.o      MSH/msh_gen_pris.o      MSH/msh_mesh.o  MSH/msh_quality.o\
        MSH/msh_elem.o      MSH/msh_lib.o           MSH/msh_node.o  MSH/msh_rfi.o

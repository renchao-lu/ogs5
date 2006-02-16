# Microsoft Developer Studio Generated NMAKE File, Based on rf3_cpp.dsp
!IF "$(CFG)" == ""
CFG=rf3_cpp - Win32 Debug
!MESSAGE No configuration specified. Defaulting to rf3_cpp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "rf3_cpp - Win32 Release" && "$(CFG)" != "rf3_cpp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rf3_cpp.mak" CFG="rf3_cpp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rf3_cpp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "rf3_cpp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rf3_cpp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\rf3_cpp.exe" "$(OUTDIR)\rf3_cpp.bsc"


CLEAN :
	-@erase "$(INTDIR)\adaptiv.obj"
	-@erase "$(INTDIR)\adaptiv.sbr"
	-@erase "$(INTDIR)\balance.obj"
	-@erase "$(INTDIR)\balance.sbr"
	-@erase "$(INTDIR)\break.obj"
	-@erase "$(INTDIR)\break.sbr"
	-@erase "$(INTDIR)\cel_agm.obj"
	-@erase "$(INTDIR)\cel_agm.sbr"
	-@erase "$(INTDIR)\cel_asm.obj"
	-@erase "$(INTDIR)\cel_asm.sbr"
	-@erase "$(INTDIR)\cel_atm.obj"
	-@erase "$(INTDIR)\cel_atm.sbr"
	-@erase "$(INTDIR)\cel_dm.obj"
	-@erase "$(INTDIR)\cel_dm.sbr"
	-@erase "$(INTDIR)\cel_ent.obj"
	-@erase "$(INTDIR)\cel_ent.sbr"
	-@erase "$(INTDIR)\cel_ge.obj"
	-@erase "$(INTDIR)\cel_ge.sbr"
	-@erase "$(INTDIR)\cel_gtm.obj"
	-@erase "$(INTDIR)\cel_gtm.sbr"
	-@erase "$(INTDIR)\cel_htm.obj"
	-@erase "$(INTDIR)\cel_htm.sbr"
	-@erase "$(INTDIR)\cel_mmp.obj"
	-@erase "$(INTDIR)\cel_mmp.sbr"
	-@erase "$(INTDIR)\cel_mms.obj"
	-@erase "$(INTDIR)\cel_mms.sbr"
	-@erase "$(INTDIR)\cel_mpc.obj"
	-@erase "$(INTDIR)\cel_mpc.sbr"
	-@erase "$(INTDIR)\cel_mtm.obj"
	-@erase "$(INTDIR)\cel_mtm.sbr"
	-@erase "$(INTDIR)\cel_mtm2.obj"
	-@erase "$(INTDIR)\cel_mtm2.sbr"
	-@erase "$(INTDIR)\cel_rrm.obj"
	-@erase "$(INTDIR)\cel_rrm.sbr"
	-@erase "$(INTDIR)\cel_rsm.obj"
	-@erase "$(INTDIR)\cel_rsm.sbr"
	-@erase "$(INTDIR)\cel_rtm.obj"
	-@erase "$(INTDIR)\cel_rtm.sbr"
	-@erase "$(INTDIR)\cgs_agm.obj"
	-@erase "$(INTDIR)\cgs_agm.sbr"
	-@erase "$(INTDIR)\cgs_asm.obj"
	-@erase "$(INTDIR)\cgs_asm.sbr"
	-@erase "$(INTDIR)\cgs_atm.obj"
	-@erase "$(INTDIR)\cgs_atm.sbr"
	-@erase "$(INTDIR)\cgs_dm.obj"
	-@erase "$(INTDIR)\cgs_dm.sbr"
	-@erase "$(INTDIR)\cgs_ent.obj"
	-@erase "$(INTDIR)\cgs_ent.sbr"
	-@erase "$(INTDIR)\cgs_ge.obj"
	-@erase "$(INTDIR)\cgs_ge.sbr"
	-@erase "$(INTDIR)\cgs_gtm.obj"
	-@erase "$(INTDIR)\cgs_gtm.sbr"
	-@erase "$(INTDIR)\cgs_htm.obj"
	-@erase "$(INTDIR)\cgs_htm.sbr"
	-@erase "$(INTDIR)\cgs_mmp.obj"
	-@erase "$(INTDIR)\cgs_mmp.sbr"
	-@erase "$(INTDIR)\cgs_mms.obj"
	-@erase "$(INTDIR)\cgs_mms.sbr"
	-@erase "$(INTDIR)\cgs_mpc.obj"
	-@erase "$(INTDIR)\cgs_mpc.sbr"
	-@erase "$(INTDIR)\cgs_mtm.obj"
	-@erase "$(INTDIR)\cgs_mtm.sbr"
	-@erase "$(INTDIR)\cgs_mtm2.obj"
	-@erase "$(INTDIR)\cgs_mtm2.sbr"
	-@erase "$(INTDIR)\cgs_rrm.obj"
	-@erase "$(INTDIR)\cgs_rrm.sbr"
	-@erase "$(INTDIR)\cgs_rsm.obj"
	-@erase "$(INTDIR)\cgs_rsm.sbr"
	-@erase "$(INTDIR)\cgs_rtm.obj"
	-@erase "$(INTDIR)\cgs_rtm.sbr"
	-@erase "$(INTDIR)\cvel.obj"
	-@erase "$(INTDIR)\cvel.sbr"
	-@erase "$(INTDIR)\display.obj"
	-@erase "$(INTDIR)\display.sbr"
	-@erase "$(INTDIR)\edges.obj"
	-@erase "$(INTDIR)\edges.sbr"
	-@erase "$(INTDIR)\elements.obj"
	-@erase "$(INTDIR)\elements.sbr"
	-@erase "$(INTDIR)\femlib.obj"
	-@erase "$(INTDIR)\femlib.sbr"
	-@erase "$(INTDIR)\file_rfr.obj"
	-@erase "$(INTDIR)\file_rfr.sbr"
	-@erase "$(INTDIR)\files0.obj"
	-@erase "$(INTDIR)\files0.sbr"
	-@erase "$(INTDIR)\files1.obj"
	-@erase "$(INTDIR)\files1.sbr"
	-@erase "$(INTDIR)\files2.obj"
	-@erase "$(INTDIR)\files2.sbr"
	-@erase "$(INTDIR)\geo_db.obj"
	-@erase "$(INTDIR)\geo_db.sbr"
	-@erase "$(INTDIR)\geo_dom.obj"
	-@erase "$(INTDIR)\geo_dom.sbr"
	-@erase "$(INTDIR)\geo_lib.obj"
	-@erase "$(INTDIR)\geo_lib.sbr"
	-@erase "$(INTDIR)\geo_lin.obj"
	-@erase "$(INTDIR)\geo_lin.sbr"
	-@erase "$(INTDIR)\geo_ply.obj"
	-@erase "$(INTDIR)\geo_ply.sbr"
	-@erase "$(INTDIR)\geo_pnt.obj"
	-@erase "$(INTDIR)\geo_pnt.sbr"
	-@erase "$(INTDIR)\geo_sfc.obj"
	-@erase "$(INTDIR)\geo_sfc.sbr"
	-@erase "$(INTDIR)\geo_strings.obj"
	-@erase "$(INTDIR)\geo_strings.sbr"
	-@erase "$(INTDIR)\geo_vol.obj"
	-@erase "$(INTDIR)\geo_vol.sbr"
	-@erase "$(INTDIR)\gridadap.obj"
	-@erase "$(INTDIR)\gridadap.sbr"
	-@erase "$(INTDIR)\help_mtm2.obj"
	-@erase "$(INTDIR)\help_mtm2.sbr"
	-@erase "$(INTDIR)\indicat1.obj"
	-@erase "$(INTDIR)\indicat1.sbr"
	-@erase "$(INTDIR)\indicat2.obj"
	-@erase "$(INTDIR)\indicat2.sbr"
	-@erase "$(INTDIR)\indicatr.obj"
	-@erase "$(INTDIR)\indicatr.sbr"
	-@erase "$(INTDIR)\int_agm.obj"
	-@erase "$(INTDIR)\int_agm.sbr"
	-@erase "$(INTDIR)\int_asm.obj"
	-@erase "$(INTDIR)\int_asm.sbr"
	-@erase "$(INTDIR)\int_atm.obj"
	-@erase "$(INTDIR)\int_atm.sbr"
	-@erase "$(INTDIR)\int_dm.obj"
	-@erase "$(INTDIR)\int_dm.sbr"
	-@erase "$(INTDIR)\int_ge.obj"
	-@erase "$(INTDIR)\int_ge.sbr"
	-@erase "$(INTDIR)\int_gtm.obj"
	-@erase "$(INTDIR)\int_gtm.sbr"
	-@erase "$(INTDIR)\int_htm.obj"
	-@erase "$(INTDIR)\int_htm.sbr"
	-@erase "$(INTDIR)\int_mmp.obj"
	-@erase "$(INTDIR)\int_mmp.sbr"
	-@erase "$(INTDIR)\int_mms.obj"
	-@erase "$(INTDIR)\int_mms.sbr"
	-@erase "$(INTDIR)\int_mtm.obj"
	-@erase "$(INTDIR)\int_mtm.sbr"
	-@erase "$(INTDIR)\int_mtm2.obj"
	-@erase "$(INTDIR)\int_mtm2.sbr"
	-@erase "$(INTDIR)\int_rrm.obj"
	-@erase "$(INTDIR)\int_rrm.sbr"
	-@erase "$(INTDIR)\int_rsm.obj"
	-@erase "$(INTDIR)\int_rsm.sbr"
	-@erase "$(INTDIR)\int_rtm.obj"
	-@erase "$(INTDIR)\int_rtm.sbr"
	-@erase "$(INTDIR)\intrface.obj"
	-@erase "$(INTDIR)\intrface.sbr"
	-@erase "$(INTDIR)\lag_atm.obj"
	-@erase "$(INTDIR)\lag_atm.sbr"
	-@erase "$(INTDIR)\loop00.obj"
	-@erase "$(INTDIR)\loop00.sbr"
	-@erase "$(INTDIR)\loop0010.obj"
	-@erase "$(INTDIR)\loop0010.sbr"
	-@erase "$(INTDIR)\loop0093.obj"
	-@erase "$(INTDIR)\loop0093.sbr"
	-@erase "$(INTDIR)\loop0095.obj"
	-@erase "$(INTDIR)\loop0095.sbr"
	-@erase "$(INTDIR)\loop0097.obj"
	-@erase "$(INTDIR)\loop0097.sbr"
	-@erase "$(INTDIR)\loop0099.obj"
	-@erase "$(INTDIR)\loop0099.sbr"
	-@erase "$(INTDIR)\loop02.obj"
	-@erase "$(INTDIR)\loop02.sbr"
	-@erase "$(INTDIR)\loop0297.obj"
	-@erase "$(INTDIR)\loop0297.sbr"
	-@erase "$(INTDIR)\loop0299.obj"
	-@erase "$(INTDIR)\loop0299.sbr"
	-@erase "$(INTDIR)\loop0699.obj"
	-@erase "$(INTDIR)\loop0699.sbr"
	-@erase "$(INTDIR)\loop07.obj"
	-@erase "$(INTDIR)\loop07.sbr"
	-@erase "$(INTDIR)\loop0797.obj"
	-@erase "$(INTDIR)\loop0797.sbr"
	-@erase "$(INTDIR)\loop10.obj"
	-@erase "$(INTDIR)\loop10.sbr"
	-@erase "$(INTDIR)\loop8460.obj"
	-@erase "$(INTDIR)\loop8460.sbr"
	-@erase "$(INTDIR)\loop__MK.obj"
	-@erase "$(INTDIR)\loop__MK.sbr"
	-@erase "$(INTDIR)\loop_jdj.obj"
	-@erase "$(INTDIR)\loop_jdj.sbr"
	-@erase "$(INTDIR)\loop_mb.obj"
	-@erase "$(INTDIR)\loop_mb.sbr"
	-@erase "$(INTDIR)\loop_mix.obj"
	-@erase "$(INTDIR)\loop_mix.sbr"
	-@erase "$(INTDIR)\loop_mx.obj"
	-@erase "$(INTDIR)\loop_mx.sbr"
	-@erase "$(INTDIR)\loop_pcs.obj"
	-@erase "$(INTDIR)\loop_pcs.sbr"
	-@erase "$(INTDIR)\masscont.obj"
	-@erase "$(INTDIR)\masscont.sbr"
	-@erase "$(INTDIR)\material.obj"
	-@erase "$(INTDIR)\material.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\mod_00.obj"
	-@erase "$(INTDIR)\mod_00.sbr"
	-@erase "$(INTDIR)\mod_0010.obj"
	-@erase "$(INTDIR)\mod_0010.sbr"
	-@erase "$(INTDIR)\mod_0093.obj"
	-@erase "$(INTDIR)\mod_0093.sbr"
	-@erase "$(INTDIR)\mod_0095.obj"
	-@erase "$(INTDIR)\mod_0095.sbr"
	-@erase "$(INTDIR)\mod_0097.obj"
	-@erase "$(INTDIR)\mod_0097.sbr"
	-@erase "$(INTDIR)\mod_0099.obj"
	-@erase "$(INTDIR)\mod_0099.sbr"
	-@erase "$(INTDIR)\mod_02.obj"
	-@erase "$(INTDIR)\mod_02.sbr"
	-@erase "$(INTDIR)\mod_0297.obj"
	-@erase "$(INTDIR)\mod_0297.sbr"
	-@erase "$(INTDIR)\mod_0299.obj"
	-@erase "$(INTDIR)\mod_0299.sbr"
	-@erase "$(INTDIR)\mod_0699.obj"
	-@erase "$(INTDIR)\mod_0699.sbr"
	-@erase "$(INTDIR)\mod_07.obj"
	-@erase "$(INTDIR)\mod_07.sbr"
	-@erase "$(INTDIR)\mod_0797.obj"
	-@erase "$(INTDIR)\mod_0797.sbr"
	-@erase "$(INTDIR)\mod_10.obj"
	-@erase "$(INTDIR)\mod_10.sbr"
	-@erase "$(INTDIR)\mod_8460.obj"
	-@erase "$(INTDIR)\mod_8460.sbr"
	-@erase "$(INTDIR)\mod___MK.obj"
	-@erase "$(INTDIR)\mod___MK.sbr"
	-@erase "$(INTDIR)\mod__jdj.obj"
	-@erase "$(INTDIR)\mod__jdj.sbr"
	-@erase "$(INTDIR)\mod__mb.obj"
	-@erase "$(INTDIR)\mod__mb.sbr"
	-@erase "$(INTDIR)\mod__mx.obj"
	-@erase "$(INTDIR)\mod__mx.sbr"
	-@erase "$(INTDIR)\models.obj"
	-@erase "$(INTDIR)\models.sbr"
	-@erase "$(INTDIR)\msh_nodes.obj"
	-@erase "$(INTDIR)\msh_nodes.sbr"
	-@erase "$(INTDIR)\nodes.obj"
	-@erase "$(INTDIR)\nodes.sbr"
	-@erase "$(INTDIR)\numerics.obj"
	-@erase "$(INTDIR)\numerics.sbr"
	-@erase "$(INTDIR)\pcs_dm.obj"
	-@erase "$(INTDIR)\pcs_dm.sbr"
	-@erase "$(INTDIR)\plains.obj"
	-@erase "$(INTDIR)\plains.sbr"
	-@erase "$(INTDIR)\plot.obj"
	-@erase "$(INTDIR)\plot.sbr"
	-@erase "$(INTDIR)\prisgen.obj"
	-@erase "$(INTDIR)\prisgen.sbr"
	-@erase "$(INTDIR)\pst_flow.obj"
	-@erase "$(INTDIR)\pst_flow.sbr"
	-@erase "$(INTDIR)\ptrarr.obj"
	-@erase "$(INTDIR)\ptrarr.sbr"
	-@erase "$(INTDIR)\refine1.obj"
	-@erase "$(INTDIR)\refine1.sbr"
	-@erase "$(INTDIR)\refine2.obj"
	-@erase "$(INTDIR)\refine2.sbr"
	-@erase "$(INTDIR)\refine3.obj"
	-@erase "$(INTDIR)\refine3.sbr"
	-@erase "$(INTDIR)\relax.obj"
	-@erase "$(INTDIR)\relax.sbr"
	-@erase "$(INTDIR)\renumber.obj"
	-@erase "$(INTDIR)\renumber.sbr"
	-@erase "$(INTDIR)\rf.obj"
	-@erase "$(INTDIR)\rf.sbr"
	-@erase "$(INTDIR)\rf_apl.obj"
	-@erase "$(INTDIR)\rf_apl.sbr"
	-@erase "$(INTDIR)\rf_db.obj"
	-@erase "$(INTDIR)\rf_db.sbr"
	-@erase "$(INTDIR)\rf_mg.obj"
	-@erase "$(INTDIR)\rf_mg.sbr"
	-@erase "$(INTDIR)\rf_pcs.obj"
	-@erase "$(INTDIR)\rf_pcs.sbr"
	-@erase "$(INTDIR)\rf_react.obj"
	-@erase "$(INTDIR)\rf_react.sbr"
	-@erase "$(INTDIR)\rfadt.obj"
	-@erase "$(INTDIR)\rfadt.sbr"
	-@erase "$(INTDIR)\rfbc.obj"
	-@erase "$(INTDIR)\rfbc.sbr"
	-@erase "$(INTDIR)\rfdb.obj"
	-@erase "$(INTDIR)\rfdb.sbr"
	-@erase "$(INTDIR)\rfhgm.obj"
	-@erase "$(INTDIR)\rfhgm.sbr"
	-@erase "$(INTDIR)\rfidb.obj"
	-@erase "$(INTDIR)\rfidb.sbr"
	-@erase "$(INTDIR)\rfii.obj"
	-@erase "$(INTDIR)\rfii.sbr"
	-@erase "$(INTDIR)\rfim.obj"
	-@erase "$(INTDIR)\rfim.sbr"
	-@erase "$(INTDIR)\rfinitc.obj"
	-@erase "$(INTDIR)\rfinitc.sbr"
	-@erase "$(INTDIR)\rfiter.obj"
	-@erase "$(INTDIR)\rfiter.sbr"
	-@erase "$(INTDIR)\rfiv.obj"
	-@erase "$(INTDIR)\rfiv.sbr"
	-@erase "$(INTDIR)\rfmat_cp.obj"
	-@erase "$(INTDIR)\rfmat_cp.sbr"
	-@erase "$(INTDIR)\rfmat_fp.obj"
	-@erase "$(INTDIR)\rfmat_fp.sbr"
	-@erase "$(INTDIR)\rfmat_mp.obj"
	-@erase "$(INTDIR)\rfmat_mp.sbr"
	-@erase "$(INTDIR)\rfmat_sp.obj"
	-@erase "$(INTDIR)\rfmat_sp.sbr"
	-@erase "$(INTDIR)\rfmat_tp.obj"
	-@erase "$(INTDIR)\rfmat_tp.sbr"
	-@erase "$(INTDIR)\rfmodel.obj"
	-@erase "$(INTDIR)\rfmodel.sbr"
	-@erase "$(INTDIR)\rfod.obj"
	-@erase "$(INTDIR)\rfod.sbr"
	-@erase "$(INTDIR)\rfoutput.obj"
	-@erase "$(INTDIR)\rfoutput.sbr"
	-@erase "$(INTDIR)\rfpriref.obj"
	-@erase "$(INTDIR)\rfpriref.sbr"
	-@erase "$(INTDIR)\rfsolver.obj"
	-@erase "$(INTDIR)\rfsolver.sbr"
	-@erase "$(INTDIR)\rfsousin.obj"
	-@erase "$(INTDIR)\rfsousin.sbr"
	-@erase "$(INTDIR)\rfstring.obj"
	-@erase "$(INTDIR)\rfstring.sbr"
	-@erase "$(INTDIR)\rfsystim.obj"
	-@erase "$(INTDIR)\rfsystim.sbr"
	-@erase "$(INTDIR)\rftime.obj"
	-@erase "$(INTDIR)\rftime.sbr"
	-@erase "$(INTDIR)\solver.obj"
	-@erase "$(INTDIR)\solver.sbr"
	-@erase "$(INTDIR)\steam.obj"
	-@erase "$(INTDIR)\steam.sbr"
	-@erase "$(INTDIR)\testvar.obj"
	-@erase "$(INTDIR)\testvar.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\tools.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\rf3_cpp.bsc"
	-@erase "$(OUTDIR)\rf3_cpp.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\rf3_cpp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\rf3_cpp.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\adaptiv.sbr" \
	"$(INTDIR)\balance.sbr" \
	"$(INTDIR)\break.sbr" \
	"$(INTDIR)\cel_agm.sbr" \
	"$(INTDIR)\cel_asm.sbr" \
	"$(INTDIR)\cel_atm.sbr" \
	"$(INTDIR)\cel_dm.sbr" \
	"$(INTDIR)\cel_ent.sbr" \
	"$(INTDIR)\cel_ge.sbr" \
	"$(INTDIR)\cel_gtm.sbr" \
	"$(INTDIR)\cel_htm.sbr" \
	"$(INTDIR)\cel_mmp.sbr" \
	"$(INTDIR)\cel_mms.sbr" \
	"$(INTDIR)\cel_mpc.sbr" \
	"$(INTDIR)\cel_mtm.sbr" \
	"$(INTDIR)\cel_mtm2.sbr" \
	"$(INTDIR)\cel_rrm.sbr" \
	"$(INTDIR)\cel_rsm.sbr" \
	"$(INTDIR)\cel_rtm.sbr" \
	"$(INTDIR)\cgs_agm.sbr" \
	"$(INTDIR)\cgs_asm.sbr" \
	"$(INTDIR)\cgs_atm.sbr" \
	"$(INTDIR)\cgs_dm.sbr" \
	"$(INTDIR)\cgs_ent.sbr" \
	"$(INTDIR)\cgs_ge.sbr" \
	"$(INTDIR)\cgs_gtm.sbr" \
	"$(INTDIR)\cgs_htm.sbr" \
	"$(INTDIR)\cgs_mmp.sbr" \
	"$(INTDIR)\cgs_mms.sbr" \
	"$(INTDIR)\cgs_mpc.sbr" \
	"$(INTDIR)\cgs_mtm.sbr" \
	"$(INTDIR)\cgs_mtm2.sbr" \
	"$(INTDIR)\cgs_rrm.sbr" \
	"$(INTDIR)\cgs_rsm.sbr" \
	"$(INTDIR)\cgs_rtm.sbr" \
	"$(INTDIR)\cvel.sbr" \
	"$(INTDIR)\display.sbr" \
	"$(INTDIR)\edges.sbr" \
	"$(INTDIR)\elements.sbr" \
	"$(INTDIR)\femlib.sbr" \
	"$(INTDIR)\file_rfr.sbr" \
	"$(INTDIR)\files0.sbr" \
	"$(INTDIR)\files1.sbr" \
	"$(INTDIR)\files2.sbr" \
	"$(INTDIR)\gridadap.sbr" \
	"$(INTDIR)\help_mtm2.sbr" \
	"$(INTDIR)\indicat1.sbr" \
	"$(INTDIR)\indicat2.sbr" \
	"$(INTDIR)\indicatr.sbr" \
	"$(INTDIR)\int_agm.sbr" \
	"$(INTDIR)\int_asm.sbr" \
	"$(INTDIR)\int_atm.sbr" \
	"$(INTDIR)\int_dm.sbr" \
	"$(INTDIR)\int_ge.sbr" \
	"$(INTDIR)\int_gtm.sbr" \
	"$(INTDIR)\int_htm.sbr" \
	"$(INTDIR)\int_mmp.sbr" \
	"$(INTDIR)\int_mms.sbr" \
	"$(INTDIR)\int_mtm.sbr" \
	"$(INTDIR)\int_mtm2.sbr" \
	"$(INTDIR)\int_rrm.sbr" \
	"$(INTDIR)\int_rsm.sbr" \
	"$(INTDIR)\int_rtm.sbr" \
	"$(INTDIR)\intrface.sbr" \
	"$(INTDIR)\lag_atm.sbr" \
	"$(INTDIR)\loop00.sbr" \
	"$(INTDIR)\loop0010.sbr" \
	"$(INTDIR)\loop0093.sbr" \
	"$(INTDIR)\loop0095.sbr" \
	"$(INTDIR)\loop0097.sbr" \
	"$(INTDIR)\loop0099.sbr" \
	"$(INTDIR)\loop02.sbr" \
	"$(INTDIR)\loop0297.sbr" \
	"$(INTDIR)\loop0299.sbr" \
	"$(INTDIR)\loop0699.sbr" \
	"$(INTDIR)\loop07.sbr" \
	"$(INTDIR)\loop0797.sbr" \
	"$(INTDIR)\loop10.sbr" \
	"$(INTDIR)\loop8460.sbr" \
	"$(INTDIR)\loop__MK.sbr" \
	"$(INTDIR)\loop_jdj.sbr" \
	"$(INTDIR)\loop_mb.sbr" \
	"$(INTDIR)\loop_mix.sbr" \
	"$(INTDIR)\loop_mx.sbr" \
	"$(INTDIR)\loop_pcs.sbr" \
	"$(INTDIR)\masscont.sbr" \
	"$(INTDIR)\material.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\mod_00.sbr" \
	"$(INTDIR)\mod_0010.sbr" \
	"$(INTDIR)\mod_0093.sbr" \
	"$(INTDIR)\mod_0095.sbr" \
	"$(INTDIR)\mod_0097.sbr" \
	"$(INTDIR)\mod_0099.sbr" \
	"$(INTDIR)\mod_02.sbr" \
	"$(INTDIR)\mod_0297.sbr" \
	"$(INTDIR)\mod_0299.sbr" \
	"$(INTDIR)\mod_0699.sbr" \
	"$(INTDIR)\mod_07.sbr" \
	"$(INTDIR)\mod_0797.sbr" \
	"$(INTDIR)\mod_10.sbr" \
	"$(INTDIR)\mod_8460.sbr" \
	"$(INTDIR)\mod___MK.sbr" \
	"$(INTDIR)\mod__jdj.sbr" \
	"$(INTDIR)\mod__mb.sbr" \
	"$(INTDIR)\mod__mx.sbr" \
	"$(INTDIR)\models.sbr" \
	"$(INTDIR)\nodes.sbr" \
	"$(INTDIR)\numerics.sbr" \
	"$(INTDIR)\plains.sbr" \
	"$(INTDIR)\plot.sbr" \
	"$(INTDIR)\pst_flow.sbr" \
	"$(INTDIR)\ptrarr.sbr" \
	"$(INTDIR)\refine1.sbr" \
	"$(INTDIR)\refine2.sbr" \
	"$(INTDIR)\refine3.sbr" \
	"$(INTDIR)\relax.sbr" \
	"$(INTDIR)\renumber.sbr" \
	"$(INTDIR)\rf.sbr" \
	"$(INTDIR)\rf_apl.sbr" \
	"$(INTDIR)\rf_db.sbr" \
	"$(INTDIR)\rf_mg.sbr" \
	"$(INTDIR)\rf_pcs.sbr" \
	"$(INTDIR)\rf_react.sbr" \
	"$(INTDIR)\rfadt.sbr" \
	"$(INTDIR)\rfbc.sbr" \
	"$(INTDIR)\rfdb.sbr" \
	"$(INTDIR)\rfhgm.sbr" \
	"$(INTDIR)\rfidb.sbr" \
	"$(INTDIR)\rfii.sbr" \
	"$(INTDIR)\rfim.sbr" \
	"$(INTDIR)\rfinitc.sbr" \
	"$(INTDIR)\rfiter.sbr" \
	"$(INTDIR)\rfiv.sbr" \
	"$(INTDIR)\rfmat_cp.sbr" \
	"$(INTDIR)\rfmat_fp.sbr" \
	"$(INTDIR)\rfmat_mp.sbr" \
	"$(INTDIR)\rfmat_sp.sbr" \
	"$(INTDIR)\rfmat_tp.sbr" \
	"$(INTDIR)\rfmodel.sbr" \
	"$(INTDIR)\rfod.sbr" \
	"$(INTDIR)\rfoutput.sbr" \
	"$(INTDIR)\rfpriref.sbr" \
	"$(INTDIR)\rfsolver.sbr" \
	"$(INTDIR)\rfsousin.sbr" \
	"$(INTDIR)\rfstring.sbr" \
	"$(INTDIR)\rfsystim.sbr" \
	"$(INTDIR)\rftime.sbr" \
	"$(INTDIR)\solver.sbr" \
	"$(INTDIR)\steam.sbr" \
	"$(INTDIR)\testvar.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\tools.sbr" \
	"$(INTDIR)\geo_db.sbr" \
	"$(INTDIR)\geo_dom.sbr" \
	"$(INTDIR)\geo_lin.sbr" \
	"$(INTDIR)\geo_ply.sbr" \
	"$(INTDIR)\geo_pnt.sbr" \
	"$(INTDIR)\geo_sfc.sbr" \
	"$(INTDIR)\geo_strings.sbr" \
	"$(INTDIR)\geo_vol.sbr" \
	"$(INTDIR)\msh_nodes.sbr" \
	"$(INTDIR)\prisgen.sbr" \
	"$(INTDIR)\geo_lib.sbr" \
	"$(INTDIR)\pcs_dm.sbr"

"$(OUTDIR)\rf3_cpp.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\rf3_cpp.pdb" /machine:I386 /out:"$(OUTDIR)\rf3_cpp.exe" 
LINK32_OBJS= \
	"$(INTDIR)\adaptiv.obj" \
	"$(INTDIR)\balance.obj" \
	"$(INTDIR)\break.obj" \
	"$(INTDIR)\cel_agm.obj" \
	"$(INTDIR)\cel_asm.obj" \
	"$(INTDIR)\cel_atm.obj" \
	"$(INTDIR)\cel_dm.obj" \
	"$(INTDIR)\cel_ent.obj" \
	"$(INTDIR)\cel_ge.obj" \
	"$(INTDIR)\cel_gtm.obj" \
	"$(INTDIR)\cel_htm.obj" \
	"$(INTDIR)\cel_mmp.obj" \
	"$(INTDIR)\cel_mms.obj" \
	"$(INTDIR)\cel_mpc.obj" \
	"$(INTDIR)\cel_mtm.obj" \
	"$(INTDIR)\cel_mtm2.obj" \
	"$(INTDIR)\cel_rrm.obj" \
	"$(INTDIR)\cel_rsm.obj" \
	"$(INTDIR)\cel_rtm.obj" \
	"$(INTDIR)\cgs_agm.obj" \
	"$(INTDIR)\cgs_asm.obj" \
	"$(INTDIR)\cgs_atm.obj" \
	"$(INTDIR)\cgs_dm.obj" \
	"$(INTDIR)\cgs_ent.obj" \
	"$(INTDIR)\cgs_ge.obj" \
	"$(INTDIR)\cgs_gtm.obj" \
	"$(INTDIR)\cgs_htm.obj" \
	"$(INTDIR)\cgs_mmp.obj" \
	"$(INTDIR)\cgs_mms.obj" \
	"$(INTDIR)\cgs_mpc.obj" \
	"$(INTDIR)\cgs_mtm.obj" \
	"$(INTDIR)\cgs_mtm2.obj" \
	"$(INTDIR)\cgs_rrm.obj" \
	"$(INTDIR)\cgs_rsm.obj" \
	"$(INTDIR)\cgs_rtm.obj" \
	"$(INTDIR)\cvel.obj" \
	"$(INTDIR)\display.obj" \
	"$(INTDIR)\edges.obj" \
	"$(INTDIR)\elements.obj" \
	"$(INTDIR)\femlib.obj" \
	"$(INTDIR)\file_rfr.obj" \
	"$(INTDIR)\files0.obj" \
	"$(INTDIR)\files1.obj" \
	"$(INTDIR)\files2.obj" \
	"$(INTDIR)\gridadap.obj" \
	"$(INTDIR)\help_mtm2.obj" \
	"$(INTDIR)\indicat1.obj" \
	"$(INTDIR)\indicat2.obj" \
	"$(INTDIR)\indicatr.obj" \
	"$(INTDIR)\int_agm.obj" \
	"$(INTDIR)\int_asm.obj" \
	"$(INTDIR)\int_atm.obj" \
	"$(INTDIR)\int_dm.obj" \
	"$(INTDIR)\int_ge.obj" \
	"$(INTDIR)\int_gtm.obj" \
	"$(INTDIR)\int_htm.obj" \
	"$(INTDIR)\int_mmp.obj" \
	"$(INTDIR)\int_mms.obj" \
	"$(INTDIR)\int_mtm.obj" \
	"$(INTDIR)\int_mtm2.obj" \
	"$(INTDIR)\int_rrm.obj" \
	"$(INTDIR)\int_rsm.obj" \
	"$(INTDIR)\int_rtm.obj" \
	"$(INTDIR)\intrface.obj" \
	"$(INTDIR)\lag_atm.obj" \
	"$(INTDIR)\loop00.obj" \
	"$(INTDIR)\loop0010.obj" \
	"$(INTDIR)\loop0093.obj" \
	"$(INTDIR)\loop0095.obj" \
	"$(INTDIR)\loop0097.obj" \
	"$(INTDIR)\loop0099.obj" \
	"$(INTDIR)\loop02.obj" \
	"$(INTDIR)\loop0297.obj" \
	"$(INTDIR)\loop0299.obj" \
	"$(INTDIR)\loop0699.obj" \
	"$(INTDIR)\loop07.obj" \
	"$(INTDIR)\loop0797.obj" \
	"$(INTDIR)\loop10.obj" \
	"$(INTDIR)\loop8460.obj" \
	"$(INTDIR)\loop__MK.obj" \
	"$(INTDIR)\loop_jdj.obj" \
	"$(INTDIR)\loop_mb.obj" \
	"$(INTDIR)\loop_mix.obj" \
	"$(INTDIR)\loop_mx.obj" \
	"$(INTDIR)\loop_pcs.obj" \
	"$(INTDIR)\masscont.obj" \
	"$(INTDIR)\material.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\mod_00.obj" \
	"$(INTDIR)\mod_0010.obj" \
	"$(INTDIR)\mod_0093.obj" \
	"$(INTDIR)\mod_0095.obj" \
	"$(INTDIR)\mod_0097.obj" \
	"$(INTDIR)\mod_0099.obj" \
	"$(INTDIR)\mod_02.obj" \
	"$(INTDIR)\mod_0297.obj" \
	"$(INTDIR)\mod_0299.obj" \
	"$(INTDIR)\mod_0699.obj" \
	"$(INTDIR)\mod_07.obj" \
	"$(INTDIR)\mod_0797.obj" \
	"$(INTDIR)\mod_10.obj" \
	"$(INTDIR)\mod_8460.obj" \
	"$(INTDIR)\mod___MK.obj" \
	"$(INTDIR)\mod__jdj.obj" \
	"$(INTDIR)\mod__mb.obj" \
	"$(INTDIR)\mod__mx.obj" \
	"$(INTDIR)\models.obj" \
	"$(INTDIR)\nodes.obj" \
	"$(INTDIR)\numerics.obj" \
	"$(INTDIR)\plains.obj" \
	"$(INTDIR)\plot.obj" \
	"$(INTDIR)\pst_flow.obj" \
	"$(INTDIR)\ptrarr.obj" \
	"$(INTDIR)\refine1.obj" \
	"$(INTDIR)\refine2.obj" \
	"$(INTDIR)\refine3.obj" \
	"$(INTDIR)\relax.obj" \
	"$(INTDIR)\renumber.obj" \
	"$(INTDIR)\rf.obj" \
	"$(INTDIR)\rf_apl.obj" \
	"$(INTDIR)\rf_db.obj" \
	"$(INTDIR)\rf_mg.obj" \
	"$(INTDIR)\rf_pcs.obj" \
	"$(INTDIR)\rf_react.obj" \
	"$(INTDIR)\rfadt.obj" \
	"$(INTDIR)\rfbc.obj" \
	"$(INTDIR)\rfdb.obj" \
	"$(INTDIR)\rfhgm.obj" \
	"$(INTDIR)\rfidb.obj" \
	"$(INTDIR)\rfii.obj" \
	"$(INTDIR)\rfim.obj" \
	"$(INTDIR)\rfinitc.obj" \
	"$(INTDIR)\rfiter.obj" \
	"$(INTDIR)\rfiv.obj" \
	"$(INTDIR)\rfmat_cp.obj" \
	"$(INTDIR)\rfmat_fp.obj" \
	"$(INTDIR)\rfmat_mp.obj" \
	"$(INTDIR)\rfmat_sp.obj" \
	"$(INTDIR)\rfmat_tp.obj" \
	"$(INTDIR)\rfmodel.obj" \
	"$(INTDIR)\rfod.obj" \
	"$(INTDIR)\rfoutput.obj" \
	"$(INTDIR)\rfpriref.obj" \
	"$(INTDIR)\rfsolver.obj" \
	"$(INTDIR)\rfsousin.obj" \
	"$(INTDIR)\rfstring.obj" \
	"$(INTDIR)\rfsystim.obj" \
	"$(INTDIR)\rftime.obj" \
	"$(INTDIR)\solver.obj" \
	"$(INTDIR)\steam.obj" \
	"$(INTDIR)\testvar.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\geo_db.obj" \
	"$(INTDIR)\geo_dom.obj" \
	"$(INTDIR)\geo_lin.obj" \
	"$(INTDIR)\geo_ply.obj" \
	"$(INTDIR)\geo_pnt.obj" \
	"$(INTDIR)\geo_sfc.obj" \
	"$(INTDIR)\geo_strings.obj" \
	"$(INTDIR)\geo_vol.obj" \
	"$(INTDIR)\msh_nodes.obj" \
	"$(INTDIR)\prisgen.obj" \
	"$(INTDIR)\geo_lib.obj" \
	"$(INTDIR)\pcs_dm.obj"

"$(OUTDIR)\rf3_cpp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "rf3_cpp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\rf3_cpp.exe" "$(OUTDIR)\rf3_cpp.bsc"


CLEAN :
	-@erase "$(INTDIR)\adaptiv.obj"
	-@erase "$(INTDIR)\adaptiv.sbr"
	-@erase "$(INTDIR)\balance.obj"
	-@erase "$(INTDIR)\balance.sbr"
	-@erase "$(INTDIR)\break.obj"
	-@erase "$(INTDIR)\break.sbr"
	-@erase "$(INTDIR)\cel_agm.obj"
	-@erase "$(INTDIR)\cel_agm.sbr"
	-@erase "$(INTDIR)\cel_asm.obj"
	-@erase "$(INTDIR)\cel_asm.sbr"
	-@erase "$(INTDIR)\cel_atm.obj"
	-@erase "$(INTDIR)\cel_atm.sbr"
	-@erase "$(INTDIR)\cel_dm.obj"
	-@erase "$(INTDIR)\cel_dm.sbr"
	-@erase "$(INTDIR)\cel_ent.obj"
	-@erase "$(INTDIR)\cel_ent.sbr"
	-@erase "$(INTDIR)\cel_ge.obj"
	-@erase "$(INTDIR)\cel_ge.sbr"
	-@erase "$(INTDIR)\cel_gtm.obj"
	-@erase "$(INTDIR)\cel_gtm.sbr"
	-@erase "$(INTDIR)\cel_htm.obj"
	-@erase "$(INTDIR)\cel_htm.sbr"
	-@erase "$(INTDIR)\cel_mmp.obj"
	-@erase "$(INTDIR)\cel_mmp.sbr"
	-@erase "$(INTDIR)\cel_mms.obj"
	-@erase "$(INTDIR)\cel_mms.sbr"
	-@erase "$(INTDIR)\cel_mpc.obj"
	-@erase "$(INTDIR)\cel_mpc.sbr"
	-@erase "$(INTDIR)\cel_mtm.obj"
	-@erase "$(INTDIR)\cel_mtm.sbr"
	-@erase "$(INTDIR)\cel_mtm2.obj"
	-@erase "$(INTDIR)\cel_mtm2.sbr"
	-@erase "$(INTDIR)\cel_rrm.obj"
	-@erase "$(INTDIR)\cel_rrm.sbr"
	-@erase "$(INTDIR)\cel_rsm.obj"
	-@erase "$(INTDIR)\cel_rsm.sbr"
	-@erase "$(INTDIR)\cel_rtm.obj"
	-@erase "$(INTDIR)\cel_rtm.sbr"
	-@erase "$(INTDIR)\cgs_agm.obj"
	-@erase "$(INTDIR)\cgs_agm.sbr"
	-@erase "$(INTDIR)\cgs_asm.obj"
	-@erase "$(INTDIR)\cgs_asm.sbr"
	-@erase "$(INTDIR)\cgs_atm.obj"
	-@erase "$(INTDIR)\cgs_atm.sbr"
	-@erase "$(INTDIR)\cgs_dm.obj"
	-@erase "$(INTDIR)\cgs_dm.sbr"
	-@erase "$(INTDIR)\cgs_ent.obj"
	-@erase "$(INTDIR)\cgs_ent.sbr"
	-@erase "$(INTDIR)\cgs_ge.obj"
	-@erase "$(INTDIR)\cgs_ge.sbr"
	-@erase "$(INTDIR)\cgs_gtm.obj"
	-@erase "$(INTDIR)\cgs_gtm.sbr"
	-@erase "$(INTDIR)\cgs_htm.obj"
	-@erase "$(INTDIR)\cgs_htm.sbr"
	-@erase "$(INTDIR)\cgs_mmp.obj"
	-@erase "$(INTDIR)\cgs_mmp.sbr"
	-@erase "$(INTDIR)\cgs_mms.obj"
	-@erase "$(INTDIR)\cgs_mms.sbr"
	-@erase "$(INTDIR)\cgs_mpc.obj"
	-@erase "$(INTDIR)\cgs_mpc.sbr"
	-@erase "$(INTDIR)\cgs_mtm.obj"
	-@erase "$(INTDIR)\cgs_mtm.sbr"
	-@erase "$(INTDIR)\cgs_mtm2.obj"
	-@erase "$(INTDIR)\cgs_mtm2.sbr"
	-@erase "$(INTDIR)\cgs_rrm.obj"
	-@erase "$(INTDIR)\cgs_rrm.sbr"
	-@erase "$(INTDIR)\cgs_rsm.obj"
	-@erase "$(INTDIR)\cgs_rsm.sbr"
	-@erase "$(INTDIR)\cgs_rtm.obj"
	-@erase "$(INTDIR)\cgs_rtm.sbr"
	-@erase "$(INTDIR)\cvel.obj"
	-@erase "$(INTDIR)\cvel.sbr"
	-@erase "$(INTDIR)\display.obj"
	-@erase "$(INTDIR)\display.sbr"
	-@erase "$(INTDIR)\edges.obj"
	-@erase "$(INTDIR)\edges.sbr"
	-@erase "$(INTDIR)\elements.obj"
	-@erase "$(INTDIR)\elements.sbr"
	-@erase "$(INTDIR)\femlib.obj"
	-@erase "$(INTDIR)\femlib.sbr"
	-@erase "$(INTDIR)\file_rfr.obj"
	-@erase "$(INTDIR)\file_rfr.sbr"
	-@erase "$(INTDIR)\files0.obj"
	-@erase "$(INTDIR)\files0.sbr"
	-@erase "$(INTDIR)\files1.obj"
	-@erase "$(INTDIR)\files1.sbr"
	-@erase "$(INTDIR)\files2.obj"
	-@erase "$(INTDIR)\files2.sbr"
	-@erase "$(INTDIR)\geo_db.obj"
	-@erase "$(INTDIR)\geo_db.sbr"
	-@erase "$(INTDIR)\geo_dom.obj"
	-@erase "$(INTDIR)\geo_dom.sbr"
	-@erase "$(INTDIR)\geo_lib.obj"
	-@erase "$(INTDIR)\geo_lib.sbr"
	-@erase "$(INTDIR)\geo_lin.obj"
	-@erase "$(INTDIR)\geo_lin.sbr"
	-@erase "$(INTDIR)\geo_ply.obj"
	-@erase "$(INTDIR)\geo_ply.sbr"
	-@erase "$(INTDIR)\geo_pnt.obj"
	-@erase "$(INTDIR)\geo_pnt.sbr"
	-@erase "$(INTDIR)\geo_sfc.obj"
	-@erase "$(INTDIR)\geo_sfc.sbr"
	-@erase "$(INTDIR)\geo_strings.obj"
	-@erase "$(INTDIR)\geo_strings.sbr"
	-@erase "$(INTDIR)\geo_vol.obj"
	-@erase "$(INTDIR)\geo_vol.sbr"
	-@erase "$(INTDIR)\gridadap.obj"
	-@erase "$(INTDIR)\gridadap.sbr"
	-@erase "$(INTDIR)\help_mtm2.obj"
	-@erase "$(INTDIR)\help_mtm2.sbr"
	-@erase "$(INTDIR)\indicat1.obj"
	-@erase "$(INTDIR)\indicat1.sbr"
	-@erase "$(INTDIR)\indicat2.obj"
	-@erase "$(INTDIR)\indicat2.sbr"
	-@erase "$(INTDIR)\indicatr.obj"
	-@erase "$(INTDIR)\indicatr.sbr"
	-@erase "$(INTDIR)\int_agm.obj"
	-@erase "$(INTDIR)\int_agm.sbr"
	-@erase "$(INTDIR)\int_asm.obj"
	-@erase "$(INTDIR)\int_asm.sbr"
	-@erase "$(INTDIR)\int_atm.obj"
	-@erase "$(INTDIR)\int_atm.sbr"
	-@erase "$(INTDIR)\int_dm.obj"
	-@erase "$(INTDIR)\int_dm.sbr"
	-@erase "$(INTDIR)\int_ge.obj"
	-@erase "$(INTDIR)\int_ge.sbr"
	-@erase "$(INTDIR)\int_gtm.obj"
	-@erase "$(INTDIR)\int_gtm.sbr"
	-@erase "$(INTDIR)\int_htm.obj"
	-@erase "$(INTDIR)\int_htm.sbr"
	-@erase "$(INTDIR)\int_mmp.obj"
	-@erase "$(INTDIR)\int_mmp.sbr"
	-@erase "$(INTDIR)\int_mms.obj"
	-@erase "$(INTDIR)\int_mms.sbr"
	-@erase "$(INTDIR)\int_mtm.obj"
	-@erase "$(INTDIR)\int_mtm.sbr"
	-@erase "$(INTDIR)\int_mtm2.obj"
	-@erase "$(INTDIR)\int_mtm2.sbr"
	-@erase "$(INTDIR)\int_rrm.obj"
	-@erase "$(INTDIR)\int_rrm.sbr"
	-@erase "$(INTDIR)\int_rsm.obj"
	-@erase "$(INTDIR)\int_rsm.sbr"
	-@erase "$(INTDIR)\int_rtm.obj"
	-@erase "$(INTDIR)\int_rtm.sbr"
	-@erase "$(INTDIR)\intrface.obj"
	-@erase "$(INTDIR)\intrface.sbr"
	-@erase "$(INTDIR)\lag_atm.obj"
	-@erase "$(INTDIR)\lag_atm.sbr"
	-@erase "$(INTDIR)\loop00.obj"
	-@erase "$(INTDIR)\loop00.sbr"
	-@erase "$(INTDIR)\loop0010.obj"
	-@erase "$(INTDIR)\loop0010.sbr"
	-@erase "$(INTDIR)\loop0093.obj"
	-@erase "$(INTDIR)\loop0093.sbr"
	-@erase "$(INTDIR)\loop0095.obj"
	-@erase "$(INTDIR)\loop0095.sbr"
	-@erase "$(INTDIR)\loop0097.obj"
	-@erase "$(INTDIR)\loop0097.sbr"
	-@erase "$(INTDIR)\loop0099.obj"
	-@erase "$(INTDIR)\loop0099.sbr"
	-@erase "$(INTDIR)\loop02.obj"
	-@erase "$(INTDIR)\loop02.sbr"
	-@erase "$(INTDIR)\loop0297.obj"
	-@erase "$(INTDIR)\loop0297.sbr"
	-@erase "$(INTDIR)\loop0299.obj"
	-@erase "$(INTDIR)\loop0299.sbr"
	-@erase "$(INTDIR)\loop0699.obj"
	-@erase "$(INTDIR)\loop0699.sbr"
	-@erase "$(INTDIR)\loop07.obj"
	-@erase "$(INTDIR)\loop07.sbr"
	-@erase "$(INTDIR)\loop0797.obj"
	-@erase "$(INTDIR)\loop0797.sbr"
	-@erase "$(INTDIR)\loop10.obj"
	-@erase "$(INTDIR)\loop10.sbr"
	-@erase "$(INTDIR)\loop8460.obj"
	-@erase "$(INTDIR)\loop8460.sbr"
	-@erase "$(INTDIR)\loop__MK.obj"
	-@erase "$(INTDIR)\loop__MK.sbr"
	-@erase "$(INTDIR)\loop_jdj.obj"
	-@erase "$(INTDIR)\loop_jdj.sbr"
	-@erase "$(INTDIR)\loop_mb.obj"
	-@erase "$(INTDIR)\loop_mb.sbr"
	-@erase "$(INTDIR)\loop_mix.obj"
	-@erase "$(INTDIR)\loop_mix.sbr"
	-@erase "$(INTDIR)\loop_mx.obj"
	-@erase "$(INTDIR)\loop_mx.sbr"
	-@erase "$(INTDIR)\loop_pcs.obj"
	-@erase "$(INTDIR)\loop_pcs.sbr"
	-@erase "$(INTDIR)\masscont.obj"
	-@erase "$(INTDIR)\masscont.sbr"
	-@erase "$(INTDIR)\material.obj"
	-@erase "$(INTDIR)\material.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\mod_00.obj"
	-@erase "$(INTDIR)\mod_00.sbr"
	-@erase "$(INTDIR)\mod_0010.obj"
	-@erase "$(INTDIR)\mod_0010.sbr"
	-@erase "$(INTDIR)\mod_0093.obj"
	-@erase "$(INTDIR)\mod_0093.sbr"
	-@erase "$(INTDIR)\mod_0095.obj"
	-@erase "$(INTDIR)\mod_0095.sbr"
	-@erase "$(INTDIR)\mod_0097.obj"
	-@erase "$(INTDIR)\mod_0097.sbr"
	-@erase "$(INTDIR)\mod_0099.obj"
	-@erase "$(INTDIR)\mod_0099.sbr"
	-@erase "$(INTDIR)\mod_02.obj"
	-@erase "$(INTDIR)\mod_02.sbr"
	-@erase "$(INTDIR)\mod_0297.obj"
	-@erase "$(INTDIR)\mod_0297.sbr"
	-@erase "$(INTDIR)\mod_0299.obj"
	-@erase "$(INTDIR)\mod_0299.sbr"
	-@erase "$(INTDIR)\mod_0699.obj"
	-@erase "$(INTDIR)\mod_0699.sbr"
	-@erase "$(INTDIR)\mod_07.obj"
	-@erase "$(INTDIR)\mod_07.sbr"
	-@erase "$(INTDIR)\mod_0797.obj"
	-@erase "$(INTDIR)\mod_0797.sbr"
	-@erase "$(INTDIR)\mod_10.obj"
	-@erase "$(INTDIR)\mod_10.sbr"
	-@erase "$(INTDIR)\mod_8460.obj"
	-@erase "$(INTDIR)\mod_8460.sbr"
	-@erase "$(INTDIR)\mod___MK.obj"
	-@erase "$(INTDIR)\mod___MK.sbr"
	-@erase "$(INTDIR)\mod__jdj.obj"
	-@erase "$(INTDIR)\mod__jdj.sbr"
	-@erase "$(INTDIR)\mod__mb.obj"
	-@erase "$(INTDIR)\mod__mb.sbr"
	-@erase "$(INTDIR)\mod__mx.obj"
	-@erase "$(INTDIR)\mod__mx.sbr"
	-@erase "$(INTDIR)\models.obj"
	-@erase "$(INTDIR)\models.sbr"
	-@erase "$(INTDIR)\msh_nodes.obj"
	-@erase "$(INTDIR)\msh_nodes.sbr"
	-@erase "$(INTDIR)\nodes.obj"
	-@erase "$(INTDIR)\nodes.sbr"
	-@erase "$(INTDIR)\numerics.obj"
	-@erase "$(INTDIR)\numerics.sbr"
	-@erase "$(INTDIR)\pcs_dm.obj"
	-@erase "$(INTDIR)\pcs_dm.sbr"
	-@erase "$(INTDIR)\plains.obj"
	-@erase "$(INTDIR)\plains.sbr"
	-@erase "$(INTDIR)\plot.obj"
	-@erase "$(INTDIR)\plot.sbr"
	-@erase "$(INTDIR)\prisgen.obj"
	-@erase "$(INTDIR)\prisgen.sbr"
	-@erase "$(INTDIR)\pst_flow.obj"
	-@erase "$(INTDIR)\pst_flow.sbr"
	-@erase "$(INTDIR)\ptrarr.obj"
	-@erase "$(INTDIR)\ptrarr.sbr"
	-@erase "$(INTDIR)\refine1.obj"
	-@erase "$(INTDIR)\refine1.sbr"
	-@erase "$(INTDIR)\refine2.obj"
	-@erase "$(INTDIR)\refine2.sbr"
	-@erase "$(INTDIR)\refine3.obj"
	-@erase "$(INTDIR)\refine3.sbr"
	-@erase "$(INTDIR)\relax.obj"
	-@erase "$(INTDIR)\relax.sbr"
	-@erase "$(INTDIR)\renumber.obj"
	-@erase "$(INTDIR)\renumber.sbr"
	-@erase "$(INTDIR)\rf.obj"
	-@erase "$(INTDIR)\rf.sbr"
	-@erase "$(INTDIR)\rf_apl.obj"
	-@erase "$(INTDIR)\rf_apl.sbr"
	-@erase "$(INTDIR)\rf_db.obj"
	-@erase "$(INTDIR)\rf_db.sbr"
	-@erase "$(INTDIR)\rf_mg.obj"
	-@erase "$(INTDIR)\rf_mg.sbr"
	-@erase "$(INTDIR)\rf_pcs.obj"
	-@erase "$(INTDIR)\rf_pcs.sbr"
	-@erase "$(INTDIR)\rf_react.obj"
	-@erase "$(INTDIR)\rf_react.sbr"
	-@erase "$(INTDIR)\rfadt.obj"
	-@erase "$(INTDIR)\rfadt.sbr"
	-@erase "$(INTDIR)\rfbc.obj"
	-@erase "$(INTDIR)\rfbc.sbr"
	-@erase "$(INTDIR)\rfdb.obj"
	-@erase "$(INTDIR)\rfdb.sbr"
	-@erase "$(INTDIR)\rfhgm.obj"
	-@erase "$(INTDIR)\rfhgm.sbr"
	-@erase "$(INTDIR)\rfidb.obj"
	-@erase "$(INTDIR)\rfidb.sbr"
	-@erase "$(INTDIR)\rfii.obj"
	-@erase "$(INTDIR)\rfii.sbr"
	-@erase "$(INTDIR)\rfim.obj"
	-@erase "$(INTDIR)\rfim.sbr"
	-@erase "$(INTDIR)\rfinitc.obj"
	-@erase "$(INTDIR)\rfinitc.sbr"
	-@erase "$(INTDIR)\rfiter.obj"
	-@erase "$(INTDIR)\rfiter.sbr"
	-@erase "$(INTDIR)\rfiv.obj"
	-@erase "$(INTDIR)\rfiv.sbr"
	-@erase "$(INTDIR)\rfmat_cp.obj"
	-@erase "$(INTDIR)\rfmat_cp.sbr"
	-@erase "$(INTDIR)\rfmat_fp.obj"
	-@erase "$(INTDIR)\rfmat_fp.sbr"
	-@erase "$(INTDIR)\rfmat_mp.obj"
	-@erase "$(INTDIR)\rfmat_mp.sbr"
	-@erase "$(INTDIR)\rfmat_sp.obj"
	-@erase "$(INTDIR)\rfmat_sp.sbr"
	-@erase "$(INTDIR)\rfmat_tp.obj"
	-@erase "$(INTDIR)\rfmat_tp.sbr"
	-@erase "$(INTDIR)\rfmodel.obj"
	-@erase "$(INTDIR)\rfmodel.sbr"
	-@erase "$(INTDIR)\rfod.obj"
	-@erase "$(INTDIR)\rfod.sbr"
	-@erase "$(INTDIR)\rfoutput.obj"
	-@erase "$(INTDIR)\rfoutput.sbr"
	-@erase "$(INTDIR)\rfpriref.obj"
	-@erase "$(INTDIR)\rfpriref.sbr"
	-@erase "$(INTDIR)\rfsolver.obj"
	-@erase "$(INTDIR)\rfsolver.sbr"
	-@erase "$(INTDIR)\rfsousin.obj"
	-@erase "$(INTDIR)\rfsousin.sbr"
	-@erase "$(INTDIR)\rfstring.obj"
	-@erase "$(INTDIR)\rfstring.sbr"
	-@erase "$(INTDIR)\rfsystim.obj"
	-@erase "$(INTDIR)\rfsystim.sbr"
	-@erase "$(INTDIR)\rftime.obj"
	-@erase "$(INTDIR)\rftime.sbr"
	-@erase "$(INTDIR)\solver.obj"
	-@erase "$(INTDIR)\solver.sbr"
	-@erase "$(INTDIR)\steam.obj"
	-@erase "$(INTDIR)\steam.sbr"
	-@erase "$(INTDIR)\testvar.obj"
	-@erase "$(INTDIR)\testvar.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\tools.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\rf3_cpp.bsc"
	-@erase "$(OUTDIR)\rf3_cpp.exe"
	-@erase "$(OUTDIR)\rf3_cpp.ilk"
	-@erase "$(OUTDIR)\rf3_cpp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\rf3_cpp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\rf3_cpp.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\adaptiv.sbr" \
	"$(INTDIR)\balance.sbr" \
	"$(INTDIR)\break.sbr" \
	"$(INTDIR)\cel_agm.sbr" \
	"$(INTDIR)\cel_asm.sbr" \
	"$(INTDIR)\cel_atm.sbr" \
	"$(INTDIR)\cel_dm.sbr" \
	"$(INTDIR)\cel_ent.sbr" \
	"$(INTDIR)\cel_ge.sbr" \
	"$(INTDIR)\cel_gtm.sbr" \
	"$(INTDIR)\cel_htm.sbr" \
	"$(INTDIR)\cel_mmp.sbr" \
	"$(INTDIR)\cel_mms.sbr" \
	"$(INTDIR)\cel_mpc.sbr" \
	"$(INTDIR)\cel_mtm.sbr" \
	"$(INTDIR)\cel_mtm2.sbr" \
	"$(INTDIR)\cel_rrm.sbr" \
	"$(INTDIR)\cel_rsm.sbr" \
	"$(INTDIR)\cel_rtm.sbr" \
	"$(INTDIR)\cgs_agm.sbr" \
	"$(INTDIR)\cgs_asm.sbr" \
	"$(INTDIR)\cgs_atm.sbr" \
	"$(INTDIR)\cgs_dm.sbr" \
	"$(INTDIR)\cgs_ent.sbr" \
	"$(INTDIR)\cgs_ge.sbr" \
	"$(INTDIR)\cgs_gtm.sbr" \
	"$(INTDIR)\cgs_htm.sbr" \
	"$(INTDIR)\cgs_mmp.sbr" \
	"$(INTDIR)\cgs_mms.sbr" \
	"$(INTDIR)\cgs_mpc.sbr" \
	"$(INTDIR)\cgs_mtm.sbr" \
	"$(INTDIR)\cgs_mtm2.sbr" \
	"$(INTDIR)\cgs_rrm.sbr" \
	"$(INTDIR)\cgs_rsm.sbr" \
	"$(INTDIR)\cgs_rtm.sbr" \
	"$(INTDIR)\cvel.sbr" \
	"$(INTDIR)\display.sbr" \
	"$(INTDIR)\edges.sbr" \
	"$(INTDIR)\elements.sbr" \
	"$(INTDIR)\femlib.sbr" \
	"$(INTDIR)\file_rfr.sbr" \
	"$(INTDIR)\files0.sbr" \
	"$(INTDIR)\files1.sbr" \
	"$(INTDIR)\files2.sbr" \
	"$(INTDIR)\gridadap.sbr" \
	"$(INTDIR)\help_mtm2.sbr" \
	"$(INTDIR)\indicat1.sbr" \
	"$(INTDIR)\indicat2.sbr" \
	"$(INTDIR)\indicatr.sbr" \
	"$(INTDIR)\int_agm.sbr" \
	"$(INTDIR)\int_asm.sbr" \
	"$(INTDIR)\int_atm.sbr" \
	"$(INTDIR)\int_dm.sbr" \
	"$(INTDIR)\int_ge.sbr" \
	"$(INTDIR)\int_gtm.sbr" \
	"$(INTDIR)\int_htm.sbr" \
	"$(INTDIR)\int_mmp.sbr" \
	"$(INTDIR)\int_mms.sbr" \
	"$(INTDIR)\int_mtm.sbr" \
	"$(INTDIR)\int_mtm2.sbr" \
	"$(INTDIR)\int_rrm.sbr" \
	"$(INTDIR)\int_rsm.sbr" \
	"$(INTDIR)\int_rtm.sbr" \
	"$(INTDIR)\intrface.sbr" \
	"$(INTDIR)\lag_atm.sbr" \
	"$(INTDIR)\loop00.sbr" \
	"$(INTDIR)\loop0010.sbr" \
	"$(INTDIR)\loop0093.sbr" \
	"$(INTDIR)\loop0095.sbr" \
	"$(INTDIR)\loop0097.sbr" \
	"$(INTDIR)\loop0099.sbr" \
	"$(INTDIR)\loop02.sbr" \
	"$(INTDIR)\loop0297.sbr" \
	"$(INTDIR)\loop0299.sbr" \
	"$(INTDIR)\loop0699.sbr" \
	"$(INTDIR)\loop07.sbr" \
	"$(INTDIR)\loop0797.sbr" \
	"$(INTDIR)\loop10.sbr" \
	"$(INTDIR)\loop8460.sbr" \
	"$(INTDIR)\loop__MK.sbr" \
	"$(INTDIR)\loop_jdj.sbr" \
	"$(INTDIR)\loop_mb.sbr" \
	"$(INTDIR)\loop_mix.sbr" \
	"$(INTDIR)\loop_mx.sbr" \
	"$(INTDIR)\loop_pcs.sbr" \
	"$(INTDIR)\masscont.sbr" \
	"$(INTDIR)\material.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\mod_00.sbr" \
	"$(INTDIR)\mod_0010.sbr" \
	"$(INTDIR)\mod_0093.sbr" \
	"$(INTDIR)\mod_0095.sbr" \
	"$(INTDIR)\mod_0097.sbr" \
	"$(INTDIR)\mod_0099.sbr" \
	"$(INTDIR)\mod_02.sbr" \
	"$(INTDIR)\mod_0297.sbr" \
	"$(INTDIR)\mod_0299.sbr" \
	"$(INTDIR)\mod_0699.sbr" \
	"$(INTDIR)\mod_07.sbr" \
	"$(INTDIR)\mod_0797.sbr" \
	"$(INTDIR)\mod_10.sbr" \
	"$(INTDIR)\mod_8460.sbr" \
	"$(INTDIR)\mod___MK.sbr" \
	"$(INTDIR)\mod__jdj.sbr" \
	"$(INTDIR)\mod__mb.sbr" \
	"$(INTDIR)\mod__mx.sbr" \
	"$(INTDIR)\models.sbr" \
	"$(INTDIR)\nodes.sbr" \
	"$(INTDIR)\numerics.sbr" \
	"$(INTDIR)\plains.sbr" \
	"$(INTDIR)\plot.sbr" \
	"$(INTDIR)\pst_flow.sbr" \
	"$(INTDIR)\ptrarr.sbr" \
	"$(INTDIR)\refine1.sbr" \
	"$(INTDIR)\refine2.sbr" \
	"$(INTDIR)\refine3.sbr" \
	"$(INTDIR)\relax.sbr" \
	"$(INTDIR)\renumber.sbr" \
	"$(INTDIR)\rf.sbr" \
	"$(INTDIR)\rf_apl.sbr" \
	"$(INTDIR)\rf_db.sbr" \
	"$(INTDIR)\rf_mg.sbr" \
	"$(INTDIR)\rf_pcs.sbr" \
	"$(INTDIR)\rf_react.sbr" \
	"$(INTDIR)\rfadt.sbr" \
	"$(INTDIR)\rfbc.sbr" \
	"$(INTDIR)\rfdb.sbr" \
	"$(INTDIR)\rfhgm.sbr" \
	"$(INTDIR)\rfidb.sbr" \
	"$(INTDIR)\rfii.sbr" \
	"$(INTDIR)\rfim.sbr" \
	"$(INTDIR)\rfinitc.sbr" \
	"$(INTDIR)\rfiter.sbr" \
	"$(INTDIR)\rfiv.sbr" \
	"$(INTDIR)\rfmat_cp.sbr" \
	"$(INTDIR)\rfmat_fp.sbr" \
	"$(INTDIR)\rfmat_mp.sbr" \
	"$(INTDIR)\rfmat_sp.sbr" \
	"$(INTDIR)\rfmat_tp.sbr" \
	"$(INTDIR)\rfmodel.sbr" \
	"$(INTDIR)\rfod.sbr" \
	"$(INTDIR)\rfoutput.sbr" \
	"$(INTDIR)\rfpriref.sbr" \
	"$(INTDIR)\rfsolver.sbr" \
	"$(INTDIR)\rfsousin.sbr" \
	"$(INTDIR)\rfstring.sbr" \
	"$(INTDIR)\rfsystim.sbr" \
	"$(INTDIR)\rftime.sbr" \
	"$(INTDIR)\solver.sbr" \
	"$(INTDIR)\steam.sbr" \
	"$(INTDIR)\testvar.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\tools.sbr" \
	"$(INTDIR)\geo_db.sbr" \
	"$(INTDIR)\geo_dom.sbr" \
	"$(INTDIR)\geo_lin.sbr" \
	"$(INTDIR)\geo_ply.sbr" \
	"$(INTDIR)\geo_pnt.sbr" \
	"$(INTDIR)\geo_sfc.sbr" \
	"$(INTDIR)\geo_strings.sbr" \
	"$(INTDIR)\geo_vol.sbr" \
	"$(INTDIR)\msh_nodes.sbr" \
	"$(INTDIR)\prisgen.sbr" \
	"$(INTDIR)\geo_lib.sbr" \
	"$(INTDIR)\pcs_dm.sbr"

"$(OUTDIR)\rf3_cpp.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\rf3_cpp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\rf3_cpp.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\adaptiv.obj" \
	"$(INTDIR)\balance.obj" \
	"$(INTDIR)\break.obj" \
	"$(INTDIR)\cel_agm.obj" \
	"$(INTDIR)\cel_asm.obj" \
	"$(INTDIR)\cel_atm.obj" \
	"$(INTDIR)\cel_dm.obj" \
	"$(INTDIR)\cel_ent.obj" \
	"$(INTDIR)\cel_ge.obj" \
	"$(INTDIR)\cel_gtm.obj" \
	"$(INTDIR)\cel_htm.obj" \
	"$(INTDIR)\cel_mmp.obj" \
	"$(INTDIR)\cel_mms.obj" \
	"$(INTDIR)\cel_mpc.obj" \
	"$(INTDIR)\cel_mtm.obj" \
	"$(INTDIR)\cel_mtm2.obj" \
	"$(INTDIR)\cel_rrm.obj" \
	"$(INTDIR)\cel_rsm.obj" \
	"$(INTDIR)\cel_rtm.obj" \
	"$(INTDIR)\cgs_agm.obj" \
	"$(INTDIR)\cgs_asm.obj" \
	"$(INTDIR)\cgs_atm.obj" \
	"$(INTDIR)\cgs_dm.obj" \
	"$(INTDIR)\cgs_ent.obj" \
	"$(INTDIR)\cgs_ge.obj" \
	"$(INTDIR)\cgs_gtm.obj" \
	"$(INTDIR)\cgs_htm.obj" \
	"$(INTDIR)\cgs_mmp.obj" \
	"$(INTDIR)\cgs_mms.obj" \
	"$(INTDIR)\cgs_mpc.obj" \
	"$(INTDIR)\cgs_mtm.obj" \
	"$(INTDIR)\cgs_mtm2.obj" \
	"$(INTDIR)\cgs_rrm.obj" \
	"$(INTDIR)\cgs_rsm.obj" \
	"$(INTDIR)\cgs_rtm.obj" \
	"$(INTDIR)\cvel.obj" \
	"$(INTDIR)\display.obj" \
	"$(INTDIR)\edges.obj" \
	"$(INTDIR)\elements.obj" \
	"$(INTDIR)\femlib.obj" \
	"$(INTDIR)\file_rfr.obj" \
	"$(INTDIR)\files0.obj" \
	"$(INTDIR)\files1.obj" \
	"$(INTDIR)\files2.obj" \
	"$(INTDIR)\gridadap.obj" \
	"$(INTDIR)\help_mtm2.obj" \
	"$(INTDIR)\indicat1.obj" \
	"$(INTDIR)\indicat2.obj" \
	"$(INTDIR)\indicatr.obj" \
	"$(INTDIR)\int_agm.obj" \
	"$(INTDIR)\int_asm.obj" \
	"$(INTDIR)\int_atm.obj" \
	"$(INTDIR)\int_dm.obj" \
	"$(INTDIR)\int_ge.obj" \
	"$(INTDIR)\int_gtm.obj" \
	"$(INTDIR)\int_htm.obj" \
	"$(INTDIR)\int_mmp.obj" \
	"$(INTDIR)\int_mms.obj" \
	"$(INTDIR)\int_mtm.obj" \
	"$(INTDIR)\int_mtm2.obj" \
	"$(INTDIR)\int_rrm.obj" \
	"$(INTDIR)\int_rsm.obj" \
	"$(INTDIR)\int_rtm.obj" \
	"$(INTDIR)\intrface.obj" \
	"$(INTDIR)\lag_atm.obj" \
	"$(INTDIR)\loop00.obj" \
	"$(INTDIR)\loop0010.obj" \
	"$(INTDIR)\loop0093.obj" \
	"$(INTDIR)\loop0095.obj" \
	"$(INTDIR)\loop0097.obj" \
	"$(INTDIR)\loop0099.obj" \
	"$(INTDIR)\loop02.obj" \
	"$(INTDIR)\loop0297.obj" \
	"$(INTDIR)\loop0299.obj" \
	"$(INTDIR)\loop0699.obj" \
	"$(INTDIR)\loop07.obj" \
	"$(INTDIR)\loop0797.obj" \
	"$(INTDIR)\loop10.obj" \
	"$(INTDIR)\loop8460.obj" \
	"$(INTDIR)\loop__MK.obj" \
	"$(INTDIR)\loop_jdj.obj" \
	"$(INTDIR)\loop_mb.obj" \
	"$(INTDIR)\loop_mix.obj" \
	"$(INTDIR)\loop_mx.obj" \
	"$(INTDIR)\loop_pcs.obj" \
	"$(INTDIR)\masscont.obj" \
	"$(INTDIR)\material.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\mod_00.obj" \
	"$(INTDIR)\mod_0010.obj" \
	"$(INTDIR)\mod_0093.obj" \
	"$(INTDIR)\mod_0095.obj" \
	"$(INTDIR)\mod_0097.obj" \
	"$(INTDIR)\mod_0099.obj" \
	"$(INTDIR)\mod_02.obj" \
	"$(INTDIR)\mod_0297.obj" \
	"$(INTDIR)\mod_0299.obj" \
	"$(INTDIR)\mod_0699.obj" \
	"$(INTDIR)\mod_07.obj" \
	"$(INTDIR)\mod_0797.obj" \
	"$(INTDIR)\mod_10.obj" \
	"$(INTDIR)\mod_8460.obj" \
	"$(INTDIR)\mod___MK.obj" \
	"$(INTDIR)\mod__jdj.obj" \
	"$(INTDIR)\mod__mb.obj" \
	"$(INTDIR)\mod__mx.obj" \
	"$(INTDIR)\models.obj" \
	"$(INTDIR)\nodes.obj" \
	"$(INTDIR)\numerics.obj" \
	"$(INTDIR)\plains.obj" \
	"$(INTDIR)\plot.obj" \
	"$(INTDIR)\pst_flow.obj" \
	"$(INTDIR)\ptrarr.obj" \
	"$(INTDIR)\refine1.obj" \
	"$(INTDIR)\refine2.obj" \
	"$(INTDIR)\refine3.obj" \
	"$(INTDIR)\relax.obj" \
	"$(INTDIR)\renumber.obj" \
	"$(INTDIR)\rf.obj" \
	"$(INTDIR)\rf_apl.obj" \
	"$(INTDIR)\rf_db.obj" \
	"$(INTDIR)\rf_mg.obj" \
	"$(INTDIR)\rf_pcs.obj" \
	"$(INTDIR)\rf_react.obj" \
	"$(INTDIR)\rfadt.obj" \
	"$(INTDIR)\rfbc.obj" \
	"$(INTDIR)\rfdb.obj" \
	"$(INTDIR)\rfhgm.obj" \
	"$(INTDIR)\rfidb.obj" \
	"$(INTDIR)\rfii.obj" \
	"$(INTDIR)\rfim.obj" \
	"$(INTDIR)\rfinitc.obj" \
	"$(INTDIR)\rfiter.obj" \
	"$(INTDIR)\rfiv.obj" \
	"$(INTDIR)\rfmat_cp.obj" \
	"$(INTDIR)\rfmat_fp.obj" \
	"$(INTDIR)\rfmat_mp.obj" \
	"$(INTDIR)\rfmat_sp.obj" \
	"$(INTDIR)\rfmat_tp.obj" \
	"$(INTDIR)\rfmodel.obj" \
	"$(INTDIR)\rfod.obj" \
	"$(INTDIR)\rfoutput.obj" \
	"$(INTDIR)\rfpriref.obj" \
	"$(INTDIR)\rfsolver.obj" \
	"$(INTDIR)\rfsousin.obj" \
	"$(INTDIR)\rfstring.obj" \
	"$(INTDIR)\rfsystim.obj" \
	"$(INTDIR)\rftime.obj" \
	"$(INTDIR)\solver.obj" \
	"$(INTDIR)\steam.obj" \
	"$(INTDIR)\testvar.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\geo_db.obj" \
	"$(INTDIR)\geo_dom.obj" \
	"$(INTDIR)\geo_lin.obj" \
	"$(INTDIR)\geo_ply.obj" \
	"$(INTDIR)\geo_pnt.obj" \
	"$(INTDIR)\geo_sfc.obj" \
	"$(INTDIR)\geo_strings.obj" \
	"$(INTDIR)\geo_vol.obj" \
	"$(INTDIR)\msh_nodes.obj" \
	"$(INTDIR)\prisgen.obj" \
	"$(INTDIR)\geo_lib.obj" \
	"$(INTDIR)\pcs_dm.obj"

"$(OUTDIR)\rf3_cpp.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("rf3_cpp.dep")
!INCLUDE "rf3_cpp.dep"
!ELSE 
!MESSAGE Warning: cannot find "rf3_cpp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "rf3_cpp - Win32 Release" || "$(CFG)" == "rf3_cpp - Win32 Debug"
SOURCE=.\adaptiv.cpp

"$(INTDIR)\adaptiv.obj"	"$(INTDIR)\adaptiv.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\balance.cpp

"$(INTDIR)\balance.obj"	"$(INTDIR)\balance.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\break.cpp

"$(INTDIR)\break.obj"	"$(INTDIR)\break.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_agm.cpp

"$(INTDIR)\cel_agm.obj"	"$(INTDIR)\cel_agm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_asm.cpp

"$(INTDIR)\cel_asm.obj"	"$(INTDIR)\cel_asm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_atm.cpp

"$(INTDIR)\cel_atm.obj"	"$(INTDIR)\cel_atm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_dm.cpp

"$(INTDIR)\cel_dm.obj"	"$(INTDIR)\cel_dm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_ent.cpp

"$(INTDIR)\cel_ent.obj"	"$(INTDIR)\cel_ent.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_ge.cpp

"$(INTDIR)\cel_ge.obj"	"$(INTDIR)\cel_ge.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_gtm.cpp

"$(INTDIR)\cel_gtm.obj"	"$(INTDIR)\cel_gtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_htm.cpp

"$(INTDIR)\cel_htm.obj"	"$(INTDIR)\cel_htm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_mmp.cpp

"$(INTDIR)\cel_mmp.obj"	"$(INTDIR)\cel_mmp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_mms.cpp

"$(INTDIR)\cel_mms.obj"	"$(INTDIR)\cel_mms.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_mpc.cpp

"$(INTDIR)\cel_mpc.obj"	"$(INTDIR)\cel_mpc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_mtm.cpp

"$(INTDIR)\cel_mtm.obj"	"$(INTDIR)\cel_mtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_mtm2.cpp

"$(INTDIR)\cel_mtm2.obj"	"$(INTDIR)\cel_mtm2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_rrm.cpp

"$(INTDIR)\cel_rrm.obj"	"$(INTDIR)\cel_rrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_rsm.cpp

"$(INTDIR)\cel_rsm.obj"	"$(INTDIR)\cel_rsm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cel_rtm.cpp

"$(INTDIR)\cel_rtm.obj"	"$(INTDIR)\cel_rtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_agm.cpp

"$(INTDIR)\cgs_agm.obj"	"$(INTDIR)\cgs_agm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_asm.cpp

"$(INTDIR)\cgs_asm.obj"	"$(INTDIR)\cgs_asm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_atm.cpp

"$(INTDIR)\cgs_atm.obj"	"$(INTDIR)\cgs_atm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_dm.cpp

"$(INTDIR)\cgs_dm.obj"	"$(INTDIR)\cgs_dm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_ent.cpp

"$(INTDIR)\cgs_ent.obj"	"$(INTDIR)\cgs_ent.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_ge.cpp

"$(INTDIR)\cgs_ge.obj"	"$(INTDIR)\cgs_ge.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_gtm.cpp

"$(INTDIR)\cgs_gtm.obj"	"$(INTDIR)\cgs_gtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_htm.cpp

"$(INTDIR)\cgs_htm.obj"	"$(INTDIR)\cgs_htm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_mmp.cpp

"$(INTDIR)\cgs_mmp.obj"	"$(INTDIR)\cgs_mmp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_mms.cpp

"$(INTDIR)\cgs_mms.obj"	"$(INTDIR)\cgs_mms.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_mpc.cpp

"$(INTDIR)\cgs_mpc.obj"	"$(INTDIR)\cgs_mpc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_mtm.cpp

"$(INTDIR)\cgs_mtm.obj"	"$(INTDIR)\cgs_mtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_mtm2.cpp

"$(INTDIR)\cgs_mtm2.obj"	"$(INTDIR)\cgs_mtm2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_rrm.cpp

"$(INTDIR)\cgs_rrm.obj"	"$(INTDIR)\cgs_rrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_rsm.cpp

"$(INTDIR)\cgs_rsm.obj"	"$(INTDIR)\cgs_rsm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cgs_rtm.cpp

"$(INTDIR)\cgs_rtm.obj"	"$(INTDIR)\cgs_rtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cvel.cpp

"$(INTDIR)\cvel.obj"	"$(INTDIR)\cvel.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\display.cpp

"$(INTDIR)\display.obj"	"$(INTDIR)\display.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\edges.cpp

"$(INTDIR)\edges.obj"	"$(INTDIR)\edges.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\elements.cpp

"$(INTDIR)\elements.obj"	"$(INTDIR)\elements.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\femlib.cpp

"$(INTDIR)\femlib.obj"	"$(INTDIR)\femlib.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\file_rfr.cpp

"$(INTDIR)\file_rfr.obj"	"$(INTDIR)\file_rfr.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\files0.cpp

"$(INTDIR)\files0.obj"	"$(INTDIR)\files0.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\files1.cpp

"$(INTDIR)\files1.obj"	"$(INTDIR)\files1.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\files2.cpp

"$(INTDIR)\files2.obj"	"$(INTDIR)\files2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gridadap.cpp

"$(INTDIR)\gridadap.obj"	"$(INTDIR)\gridadap.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\help_mtm2.cpp

"$(INTDIR)\help_mtm2.obj"	"$(INTDIR)\help_mtm2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\indicat1.cpp

"$(INTDIR)\indicat1.obj"	"$(INTDIR)\indicat1.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\indicat2.cpp

"$(INTDIR)\indicat2.obj"	"$(INTDIR)\indicat2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\indicatr.cpp

"$(INTDIR)\indicatr.obj"	"$(INTDIR)\indicatr.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_agm.cpp

"$(INTDIR)\int_agm.obj"	"$(INTDIR)\int_agm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_asm.cpp

"$(INTDIR)\int_asm.obj"	"$(INTDIR)\int_asm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_atm.cpp

"$(INTDIR)\int_atm.obj"	"$(INTDIR)\int_atm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_dm.cpp

"$(INTDIR)\int_dm.obj"	"$(INTDIR)\int_dm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_ge.cpp

"$(INTDIR)\int_ge.obj"	"$(INTDIR)\int_ge.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_gtm.cpp

"$(INTDIR)\int_gtm.obj"	"$(INTDIR)\int_gtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_htm.cpp

"$(INTDIR)\int_htm.obj"	"$(INTDIR)\int_htm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_mmp.cpp

"$(INTDIR)\int_mmp.obj"	"$(INTDIR)\int_mmp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_mms.cpp

"$(INTDIR)\int_mms.obj"	"$(INTDIR)\int_mms.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_mtm.cpp

"$(INTDIR)\int_mtm.obj"	"$(INTDIR)\int_mtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_mtm2.cpp

"$(INTDIR)\int_mtm2.obj"	"$(INTDIR)\int_mtm2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_rrm.cpp

"$(INTDIR)\int_rrm.obj"	"$(INTDIR)\int_rrm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_rsm.cpp

"$(INTDIR)\int_rsm.obj"	"$(INTDIR)\int_rsm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\int_rtm.cpp

"$(INTDIR)\int_rtm.obj"	"$(INTDIR)\int_rtm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\intrface.cpp

"$(INTDIR)\intrface.obj"	"$(INTDIR)\intrface.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lag_atm.cpp

"$(INTDIR)\lag_atm.obj"	"$(INTDIR)\lag_atm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop00.cpp

"$(INTDIR)\loop00.obj"	"$(INTDIR)\loop00.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0010.cpp

"$(INTDIR)\loop0010.obj"	"$(INTDIR)\loop0010.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0093.cpp

"$(INTDIR)\loop0093.obj"	"$(INTDIR)\loop0093.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0095.cpp

"$(INTDIR)\loop0095.obj"	"$(INTDIR)\loop0095.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0097.cpp

"$(INTDIR)\loop0097.obj"	"$(INTDIR)\loop0097.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0099.cpp

"$(INTDIR)\loop0099.obj"	"$(INTDIR)\loop0099.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop02.cpp

"$(INTDIR)\loop02.obj"	"$(INTDIR)\loop02.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0297.cpp

"$(INTDIR)\loop0297.obj"	"$(INTDIR)\loop0297.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0299.cpp

"$(INTDIR)\loop0299.obj"	"$(INTDIR)\loop0299.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0699.cpp

"$(INTDIR)\loop0699.obj"	"$(INTDIR)\loop0699.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop07.cpp

"$(INTDIR)\loop07.obj"	"$(INTDIR)\loop07.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop0797.cpp

"$(INTDIR)\loop0797.obj"	"$(INTDIR)\loop0797.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop10.cpp

"$(INTDIR)\loop10.obj"	"$(INTDIR)\loop10.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop8460.cpp

"$(INTDIR)\loop8460.obj"	"$(INTDIR)\loop8460.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop__MK.cpp

"$(INTDIR)\loop__MK.obj"	"$(INTDIR)\loop__MK.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop_jdj.cpp

"$(INTDIR)\loop_jdj.obj"	"$(INTDIR)\loop_jdj.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop_mb.cpp

"$(INTDIR)\loop_mb.obj"	"$(INTDIR)\loop_mb.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop_mix.cpp

"$(INTDIR)\loop_mix.obj"	"$(INTDIR)\loop_mix.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop_mx.cpp

"$(INTDIR)\loop_mx.obj"	"$(INTDIR)\loop_mx.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\loop_pcs.cpp

"$(INTDIR)\loop_pcs.obj"	"$(INTDIR)\loop_pcs.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\masscont.cpp

"$(INTDIR)\masscont.obj"	"$(INTDIR)\masscont.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\material.cpp

"$(INTDIR)\material.obj"	"$(INTDIR)\material.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mathlib.cpp

"$(INTDIR)\mathlib.obj"	"$(INTDIR)\mathlib.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\matrix.cpp

"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\memory.cpp

"$(INTDIR)\memory.obj"	"$(INTDIR)\memory.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_00.cpp

"$(INTDIR)\mod_00.obj"	"$(INTDIR)\mod_00.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0010.cpp

"$(INTDIR)\mod_0010.obj"	"$(INTDIR)\mod_0010.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0093.cpp

"$(INTDIR)\mod_0093.obj"	"$(INTDIR)\mod_0093.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0095.cpp

"$(INTDIR)\mod_0095.obj"	"$(INTDIR)\mod_0095.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0097.cpp

"$(INTDIR)\mod_0097.obj"	"$(INTDIR)\mod_0097.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0099.cpp

"$(INTDIR)\mod_0099.obj"	"$(INTDIR)\mod_0099.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_02.cpp

"$(INTDIR)\mod_02.obj"	"$(INTDIR)\mod_02.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0297.cpp

"$(INTDIR)\mod_0297.obj"	"$(INTDIR)\mod_0297.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0299.cpp

"$(INTDIR)\mod_0299.obj"	"$(INTDIR)\mod_0299.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0699.cpp

"$(INTDIR)\mod_0699.obj"	"$(INTDIR)\mod_0699.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_07.cpp

"$(INTDIR)\mod_07.obj"	"$(INTDIR)\mod_07.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_0797.cpp

"$(INTDIR)\mod_0797.obj"	"$(INTDIR)\mod_0797.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_10.cpp

"$(INTDIR)\mod_10.obj"	"$(INTDIR)\mod_10.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_8460.cpp

"$(INTDIR)\mod_8460.obj"	"$(INTDIR)\mod_8460.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod___MK.cpp

"$(INTDIR)\mod___MK.obj"	"$(INTDIR)\mod___MK.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod__jdj.cpp

"$(INTDIR)\mod__jdj.obj"	"$(INTDIR)\mod__jdj.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod__mb.cpp

"$(INTDIR)\mod__mb.obj"	"$(INTDIR)\mod__mb.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod__mx.cpp

"$(INTDIR)\mod__mx.obj"	"$(INTDIR)\mod__mx.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\models.cpp

"$(INTDIR)\models.obj"	"$(INTDIR)\models.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\nodes.cpp

"$(INTDIR)\nodes.obj"	"$(INTDIR)\nodes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\numerics.cpp

"$(INTDIR)\numerics.obj"	"$(INTDIR)\numerics.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pcs_dm.cpp

"$(INTDIR)\pcs_dm.obj"	"$(INTDIR)\pcs_dm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plains.cpp

"$(INTDIR)\plains.obj"	"$(INTDIR)\plains.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\plot.cpp

"$(INTDIR)\plot.obj"	"$(INTDIR)\plot.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pst_flow.cpp

"$(INTDIR)\pst_flow.obj"	"$(INTDIR)\pst_flow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ptrarr.cpp

"$(INTDIR)\ptrarr.obj"	"$(INTDIR)\ptrarr.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\refine1.cpp

"$(INTDIR)\refine1.obj"	"$(INTDIR)\refine1.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\refine2.cpp

"$(INTDIR)\refine2.obj"	"$(INTDIR)\refine2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\refine3.cpp

"$(INTDIR)\refine3.obj"	"$(INTDIR)\refine3.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\relax.cpp

"$(INTDIR)\relax.obj"	"$(INTDIR)\relax.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\renumber.cpp

"$(INTDIR)\renumber.obj"	"$(INTDIR)\renumber.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf.cpp

"$(INTDIR)\rf.obj"	"$(INTDIR)\rf.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf_apl.cpp

"$(INTDIR)\rf_apl.obj"	"$(INTDIR)\rf_apl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf_db.cpp

"$(INTDIR)\rf_db.obj"	"$(INTDIR)\rf_db.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf_mg.cpp

"$(INTDIR)\rf_mg.obj"	"$(INTDIR)\rf_mg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf_pcs.cpp

"$(INTDIR)\rf_pcs.obj"	"$(INTDIR)\rf_pcs.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rf_react.cpp

"$(INTDIR)\rf_react.obj"	"$(INTDIR)\rf_react.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfadt.cpp

"$(INTDIR)\rfadt.obj"	"$(INTDIR)\rfadt.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfbc.cpp

"$(INTDIR)\rfbc.obj"	"$(INTDIR)\rfbc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfdb.cpp

"$(INTDIR)\rfdb.obj"	"$(INTDIR)\rfdb.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfhgm.cpp

"$(INTDIR)\rfhgm.obj"	"$(INTDIR)\rfhgm.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfidb.cpp

"$(INTDIR)\rfidb.obj"	"$(INTDIR)\rfidb.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfii.cpp

"$(INTDIR)\rfii.obj"	"$(INTDIR)\rfii.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfim.cpp

"$(INTDIR)\rfim.obj"	"$(INTDIR)\rfim.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfinitc.cpp

"$(INTDIR)\rfinitc.obj"	"$(INTDIR)\rfinitc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfiter.cpp

"$(INTDIR)\rfiter.obj"	"$(INTDIR)\rfiter.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfiv.cpp

"$(INTDIR)\rfiv.obj"	"$(INTDIR)\rfiv.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmat_cp.cpp

"$(INTDIR)\rfmat_cp.obj"	"$(INTDIR)\rfmat_cp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmat_fp.cpp

"$(INTDIR)\rfmat_fp.obj"	"$(INTDIR)\rfmat_fp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmat_mp.cpp

"$(INTDIR)\rfmat_mp.obj"	"$(INTDIR)\rfmat_mp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmat_sp.cpp

"$(INTDIR)\rfmat_sp.obj"	"$(INTDIR)\rfmat_sp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmat_tp.cpp

"$(INTDIR)\rfmat_tp.obj"	"$(INTDIR)\rfmat_tp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfmodel.cpp

"$(INTDIR)\rfmodel.obj"	"$(INTDIR)\rfmodel.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfod.cpp

"$(INTDIR)\rfod.obj"	"$(INTDIR)\rfod.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfoutput.cpp

"$(INTDIR)\rfoutput.obj"	"$(INTDIR)\rfoutput.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfpriref.cpp

"$(INTDIR)\rfpriref.obj"	"$(INTDIR)\rfpriref.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfsolver.cpp

"$(INTDIR)\rfsolver.obj"	"$(INTDIR)\rfsolver.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfsousin.cpp

"$(INTDIR)\rfsousin.obj"	"$(INTDIR)\rfsousin.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfstring.cpp

"$(INTDIR)\rfstring.obj"	"$(INTDIR)\rfstring.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rfsystim.cpp

"$(INTDIR)\rfsystim.obj"	"$(INTDIR)\rfsystim.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rftime.cpp

"$(INTDIR)\rftime.obj"	"$(INTDIR)\rftime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\solver.cpp

"$(INTDIR)\solver.obj"	"$(INTDIR)\solver.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\steam.cpp

"$(INTDIR)\steam.obj"	"$(INTDIR)\steam.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\testvar.cpp

"$(INTDIR)\testvar.obj"	"$(INTDIR)\testvar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\timer.cpp

"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tools.cpp

"$(INTDIR)\tools.obj"	"$(INTDIR)\tools.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\GEO\geo_db.cpp

"$(INTDIR)\geo_db.obj"	"$(INTDIR)\geo_db.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_dom.cpp

"$(INTDIR)\geo_dom.obj"	"$(INTDIR)\geo_dom.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Geo\geo_lib.cpp

"$(INTDIR)\geo_lib.obj"	"$(INTDIR)\geo_lib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_lin.cpp

"$(INTDIR)\geo_lin.obj"	"$(INTDIR)\geo_lin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_ply.cpp

"$(INTDIR)\geo_ply.obj"	"$(INTDIR)\geo_ply.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_pnt.cpp

"$(INTDIR)\geo_pnt.obj"	"$(INTDIR)\geo_pnt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_sfc.cpp

"$(INTDIR)\geo_sfc.obj"	"$(INTDIR)\geo_sfc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_strings.cpp

"$(INTDIR)\geo_strings.obj"	"$(INTDIR)\geo_strings.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GEO\geo_vol.cpp

"$(INTDIR)\geo_vol.obj"	"$(INTDIR)\geo_vol.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MSH\msh_nodes.cpp

"$(INTDIR)\msh_nodes.obj"	"$(INTDIR)\msh_nodes.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MSH\prisgen.cpp

"$(INTDIR)\prisgen.obj"	"$(INTDIR)\prisgen.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


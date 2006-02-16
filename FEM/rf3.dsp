# Microsoft Developer Studio Project File - Name="rf3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rf3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rf3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rf3.mak" CFG="rf3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rf3 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "rf3 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rf3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "rf3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "rf3___Wi"
# PROP BASE Intermediate_Dir "rf3___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "debug"
# PROP Intermediate_Dir "debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "rf3 - Win32 Release"
# Name "rf3 - Win32 Debug"
# Begin Source File

SOURCE=.\adaptiv.c
# End Source File
# Begin Source File

SOURCE=.\adaptiv.h
# End Source File
# Begin Source File

SOURCE=.\balance.c
# End Source File
# Begin Source File

SOURCE=.\balance.h
# End Source File
# Begin Source File

SOURCE=.\break.c
# End Source File
# Begin Source File

SOURCE=.\break.h
# End Source File
# Begin Source File

SOURCE=.\cel_agm.c
# End Source File
# Begin Source File

SOURCE=.\cel_agm.h
# End Source File
# Begin Source File

SOURCE=.\cel_asm.c
# End Source File
# Begin Source File

SOURCE=.\cel_asm.h
# End Source File
# Begin Source File

SOURCE=.\cel_atm.c
# End Source File
# Begin Source File

SOURCE=.\cel_atm.h
# End Source File
# Begin Source File

SOURCE=.\cel_dm.c
# End Source File
# Begin Source File

SOURCE=.\cel_dm.h
# End Source File
# Begin Source File

SOURCE=.\cel_ent.c
# End Source File
# Begin Source File

SOURCE=.\cel_ent.h
# End Source File
# Begin Source File

SOURCE=.\cel_ge.c
# End Source File
# Begin Source File

SOURCE=.\cel_ge.h
# End Source File
# Begin Source File

SOURCE=.\cel_gtm.c
# End Source File
# Begin Source File

SOURCE=.\cel_gtm.h
# End Source File
# Begin Source File

SOURCE=.\cel_htm.c
# End Source File
# Begin Source File

SOURCE=.\cel_htm.h
# End Source File
# Begin Source File

SOURCE=.\cel_mmp.c
# End Source File
# Begin Source File

SOURCE=.\cel_mmp.h
# End Source File
# Begin Source File

SOURCE=.\cel_mms.c
# End Source File
# Begin Source File

SOURCE=.\cel_mms.h
# End Source File
# Begin Source File

SOURCE=.\cel_mpc.c
# End Source File
# Begin Source File

SOURCE=.\cel_mpc.h
# End Source File
# Begin Source File

SOURCE=.\cel_mtm.c
# End Source File
# Begin Source File

SOURCE=.\cel_mtm.h
# End Source File
# Begin Source File

SOURCE=.\cel_rrm.c
# End Source File
# Begin Source File

SOURCE=.\cel_rrm.h
# End Source File
# Begin Source File

SOURCE=.\cel_rsm.c
# End Source File
# Begin Source File

SOURCE=.\cel_rsm.h
# End Source File
# Begin Source File

SOURCE=.\cel_rtm.c
# End Source File
# Begin Source File

SOURCE=.\cel_rtm.h
# End Source File
# Begin Source File

SOURCE=.\cel_tbc.c
# End Source File
# Begin Source File

SOURCE=.\cel_tbc.h
# End Source File
# Begin Source File

SOURCE=.\cgs_agm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_agm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_asm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_asm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_atm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_atm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_dm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_dm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_ent.c
# End Source File
# Begin Source File

SOURCE=.\cgs_ge.c
# End Source File
# Begin Source File

SOURCE=.\cgs_ge.h
# End Source File
# Begin Source File

SOURCE=.\cgs_gtm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_gtm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_htm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_htm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mmp.c
# End Source File
# Begin Source File

SOURCE=.\cgs_mmp.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mms.c
# End Source File
# Begin Source File

SOURCE=.\cgs_mms.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mpc.c
# End Source File
# Begin Source File

SOURCE=.\cgs_mtm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_mtm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_rrm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_rrm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_rsm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_rsm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_rtm.c
# End Source File
# Begin Source File

SOURCE=.\cgs_rtm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_tbc.c
# End Source File
# Begin Source File

SOURCE=.\cgs_tbc.h
# End Source File
# Begin Source File

SOURCE=.\cvel.c
# End Source File
# Begin Source File

SOURCE=.\cvel.h
# End Source File
# Begin Source File

SOURCE=.\display.c
# End Source File
# Begin Source File

SOURCE=.\display.h
# End Source File
# Begin Source File

SOURCE=.\edges.c
# End Source File
# Begin Source File

SOURCE=.\edges.h
# End Source File
# Begin Source File

SOURCE=.\elements.c
# End Source File
# Begin Source File

SOURCE=.\elements.h
# End Source File
# Begin Source File

SOURCE=.\femlib.c
# End Source File
# Begin Source File

SOURCE=.\femlib.h
# End Source File
# Begin Source File

SOURCE=.\file_rfr.c
# End Source File
# Begin Source File

SOURCE=.\file_rfr.h
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\files0.c
# End Source File
# Begin Source File

SOURCE=.\files1.c
# End Source File
# Begin Source File

SOURCE=.\files2.c
# End Source File
# Begin Source File

SOURCE=.\globdat.h
# End Source File
# Begin Source File

SOURCE=.\gridadap.c
# End Source File
# Begin Source File

SOURCE=.\gridadap.h
# End Source File
# Begin Source File

SOURCE=.\indicat1.c
# End Source File
# Begin Source File

SOURCE=.\indicat2.c
# End Source File
# Begin Source File

SOURCE=.\indicatr.c
# End Source File
# Begin Source File

SOURCE=.\indicatr.h
# End Source File
# Begin Source File

SOURCE=.\int_agm.c
# End Source File
# Begin Source File

SOURCE=.\int_agm.h
# End Source File
# Begin Source File

SOURCE=.\int_asm.c
# End Source File
# Begin Source File

SOURCE=.\int_asm.h
# End Source File
# Begin Source File

SOURCE=.\int_atm.c
# End Source File
# Begin Source File

SOURCE=.\int_atm.h
# End Source File
# Begin Source File

SOURCE=.\int_dm.c
# End Source File
# Begin Source File

SOURCE=.\int_dm.h
# End Source File
# Begin Source File

SOURCE=.\int_ge.c
# End Source File
# Begin Source File

SOURCE=.\int_ge.h
# End Source File
# Begin Source File

SOURCE=.\int_gtm.c
# End Source File
# Begin Source File

SOURCE=.\int_gtm.h
# End Source File
# Begin Source File

SOURCE=.\int_htm.c
# End Source File
# Begin Source File

SOURCE=.\int_htm.h
# End Source File
# Begin Source File

SOURCE=.\int_mmp.c
# End Source File
# Begin Source File

SOURCE=.\int_mmp.h
# End Source File
# Begin Source File

SOURCE=.\int_mms.c
# End Source File
# Begin Source File

SOURCE=.\int_mms.h
# End Source File
# Begin Source File

SOURCE=.\int_mtm.c
# End Source File
# Begin Source File

SOURCE=.\int_mtm.h
# End Source File
# Begin Source File

SOURCE=.\int_rrm.c
# End Source File
# Begin Source File

SOURCE=.\int_rrm.h
# End Source File
# Begin Source File

SOURCE=.\int_rsm.c
# End Source File
# Begin Source File

SOURCE=.\int_rsm.h
# End Source File
# Begin Source File

SOURCE=.\int_rtm.c
# End Source File
# Begin Source File

SOURCE=.\int_rtm.h
# End Source File
# Begin Source File

SOURCE=.\int_tbc.c
# End Source File
# Begin Source File

SOURCE=.\int_tbc.h
# End Source File
# Begin Source File

SOURCE=.\intrface.c
# End Source File
# Begin Source File

SOURCE=.\intrface.h
# End Source File
# Begin Source File

SOURCE=.\lag_atm.c
# End Source File
# Begin Source File

SOURCE=.\lag_atm.h
# End Source File
# Begin Source File

SOURCE=.\loop00.c
# End Source File
# Begin Source File

SOURCE=.\loop00.h
# End Source File
# Begin Source File

SOURCE=.\loop0010.c
# End Source File
# Begin Source File

SOURCE=.\loop0010.h
# End Source File
# Begin Source File

SOURCE=.\loop0093.c
# End Source File
# Begin Source File

SOURCE=.\loop0093.h
# End Source File
# Begin Source File

SOURCE=.\loop0095.c
# End Source File
# Begin Source File

SOURCE=.\loop0095.h
# End Source File
# Begin Source File

SOURCE=.\loop0096.c
# End Source File
# Begin Source File

SOURCE=.\loop0096.h
# End Source File
# Begin Source File

SOURCE=.\loop0097.c
# End Source File
# Begin Source File

SOURCE=.\loop0097.h
# End Source File
# Begin Source File

SOURCE=.\loop0099.c
# End Source File
# Begin Source File

SOURCE=.\loop0099.h
# End Source File
# Begin Source File

SOURCE=.\loop02.c
# End Source File
# Begin Source File

SOURCE=.\loop02.h
# End Source File
# Begin Source File

SOURCE=.\loop0297.c
# End Source File
# Begin Source File

SOURCE=.\loop0297.h
# End Source File
# Begin Source File

SOURCE=.\loop0299.c
# End Source File
# Begin Source File

SOURCE=.\loop0299.h
# End Source File
# Begin Source File

SOURCE=.\loop0699.c
# End Source File
# Begin Source File

SOURCE=.\loop0699.h
# End Source File
# Begin Source File

SOURCE=.\loop07.c
# End Source File
# Begin Source File

SOURCE=.\loop07.h
# End Source File
# Begin Source File

SOURCE=.\loop0797.c
# End Source File
# Begin Source File

SOURCE=.\loop0797.h
# End Source File
# Begin Source File

SOURCE=.\loop10.c
# End Source File
# Begin Source File

SOURCE=.\loop10.h
# End Source File
# Begin Source File

SOURCE=.\loop8460.c
# End Source File
# Begin Source File

SOURCE=.\loop8460.h
# End Source File
# Begin Source File

SOURCE=.\loop__MK.c
# End Source File
# Begin Source File

SOURCE=.\loop__MK.h
# End Source File
# Begin Source File

SOURCE=.\loop_jdj.c
# End Source File
# Begin Source File

SOURCE=.\loop_JDJ.h
# End Source File
# Begin Source File

SOURCE=.\loop_mb.c
# End Source File
# Begin Source File

SOURCE=.\loop_mb.h
# End Source File
# Begin Source File

SOURCE=.\loop_mx.c
# End Source File
# Begin Source File

SOURCE=.\loop_mx.h
# End Source File
# Begin Source File

SOURCE=.\loop_ww.c
# End Source File
# Begin Source File

SOURCE=.\loop_ww.h
# End Source File
# Begin Source File

SOURCE=.\makros.h
# End Source File
# Begin Source File

SOURCE=.\masscont.c
# End Source File
# Begin Source File

SOURCE=.\masscont.h
# End Source File
# Begin Source File

SOURCE=.\material.c
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\matrix.c
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\mod_00.c
# End Source File
# Begin Source File

SOURCE=.\mod_00.h
# End Source File
# Begin Source File

SOURCE=.\mod_0010.c
# End Source File
# Begin Source File

SOURCE=.\mod_0010.h
# End Source File
# Begin Source File

SOURCE=.\mod_0093.c
# End Source File
# Begin Source File

SOURCE=.\mod_0093.h
# End Source File
# Begin Source File

SOURCE=.\mod_0095.c
# End Source File
# Begin Source File

SOURCE=.\mod_0095.h
# End Source File
# Begin Source File

SOURCE=.\mod_0096.c
# End Source File
# Begin Source File

SOURCE=.\mod_0096.h
# End Source File
# Begin Source File

SOURCE=.\mod_0097.c
# End Source File
# Begin Source File

SOURCE=.\mod_0097.h
# End Source File
# Begin Source File

SOURCE=.\mod_0099.c
# End Source File
# Begin Source File

SOURCE=.\mod_0099.h
# End Source File
# Begin Source File

SOURCE=.\mod_02.c
# End Source File
# Begin Source File

SOURCE=.\mod_02.h
# End Source File
# Begin Source File

SOURCE=.\mod_0297.c
# End Source File
# Begin Source File

SOURCE=.\mod_0297.h
# End Source File
# Begin Source File

SOURCE=.\mod_0299.c
# End Source File
# Begin Source File

SOURCE=.\mod_0299.h
# End Source File
# Begin Source File

SOURCE=.\mod_0699.c
# End Source File
# Begin Source File

SOURCE=.\mod_0699.h
# End Source File
# Begin Source File

SOURCE=.\mod_07.c
# End Source File
# Begin Source File

SOURCE=.\mod_07.h
# End Source File
# Begin Source File

SOURCE=.\mod_0797.c
# End Source File
# Begin Source File

SOURCE=.\mod_0797.h
# End Source File
# Begin Source File

SOURCE=.\mod_10.c
# End Source File
# Begin Source File

SOURCE=.\mod_10.h
# End Source File
# Begin Source File

SOURCE=.\mod_8460.c
# End Source File
# Begin Source File

SOURCE=.\mod_8460.h
# End Source File
# Begin Source File

SOURCE=.\mod___MK.c
# End Source File
# Begin Source File

SOURCE=.\mod___MK.h
# End Source File
# Begin Source File

SOURCE=.\mod__jdj.c
# End Source File
# Begin Source File

SOURCE=.\mod__JDJ.h
# End Source File
# Begin Source File

SOURCE=.\mod__mb.c
# End Source File
# Begin Source File

SOURCE=.\mod__mb.h
# End Source File
# Begin Source File

SOURCE=.\mod__mx.c
# End Source File
# Begin Source File

SOURCE=.\mod__mx.h
# End Source File
# Begin Source File

SOURCE=.\mod__ww.c
# End Source File
# Begin Source File

SOURCE=.\mod__ww.h
# End Source File
# Begin Source File

SOURCE=.\models.c
# End Source File
# Begin Source File

SOURCE=.\models.h
# End Source File
# Begin Source File

SOURCE=.\nodes.c
# End Source File
# Begin Source File

SOURCE=.\nodes.h
# End Source File
# Begin Source File

SOURCE=.\numerics.c
# End Source File
# Begin Source File

SOURCE=.\numerics.h
# End Source File
# Begin Source File

SOURCE=.\plains.c
# End Source File
# Begin Source File

SOURCE=.\plains.h
# End Source File
# Begin Source File

SOURCE=.\plot.c
# End Source File
# Begin Source File

SOURCE=.\plot.h
# End Source File
# Begin Source File

SOURCE=.\prototyp.h
# End Source File
# Begin Source File

SOURCE=.\pst_flow.c
# End Source File
# Begin Source File

SOURCE=.\pst_flow.h
# End Source File
# Begin Source File

SOURCE=.\ptrarr.c
# End Source File
# Begin Source File

SOURCE=.\ptrarr.h
# End Source File
# Begin Source File

SOURCE=.\refine.h
# End Source File
# Begin Source File

SOURCE=.\refine1.c
# End Source File
# Begin Source File

SOURCE=.\refine2.c
# End Source File
# Begin Source File

SOURCE=.\refine3.c
# End Source File
# Begin Source File

SOURCE=.\relax.c
# End Source File
# Begin Source File

SOURCE=.\relax.h
# End Source File
# Begin Source File

SOURCE=.\renumber.c
# End Source File
# Begin Source File

SOURCE=.\renumber.h
# End Source File
# Begin Source File

SOURCE=.\rf.c
# End Source File
# Begin Source File

SOURCE=.\rf_apl.c
# End Source File
# Begin Source File

SOURCE=.\rf_apl.h
# End Source File
# Begin Source File

SOURCE=.\rf_attach.c
# End Source File
# Begin Source File

SOURCE=.\rf_attach.h
# End Source File
# Begin Source File

SOURCE=.\rf_geo.c
# End Source File
# Begin Source File

SOURCE=.\rf_geo.h
# End Source File
# Begin Source File

SOURCE=.\rf_mg.c
# End Source File
# Begin Source File

SOURCE=.\rf_mg.h
# End Source File
# Begin Source File

SOURCE=.\rf_ply.c
# End Source File
# Begin Source File

SOURCE=.\rf_ply.h
# End Source File
# Begin Source File

SOURCE=.\rfadt.c
# End Source File
# Begin Source File

SOURCE=.\rfadt.h
# End Source File
# Begin Source File

SOURCE=.\rfbc.c
# End Source File
# Begin Source File

SOURCE=.\rfbc.h
# End Source File
# Begin Source File

SOURCE=.\rfdb.c
# End Source File
# Begin Source File

SOURCE=.\rfdb.h
# End Source File
# Begin Source File

SOURCE=.\rfhgm.c
# End Source File
# Begin Source File

SOURCE=.\rfhgm.h
# End Source File
# Begin Source File

SOURCE=.\rfidb.c
# End Source File
# Begin Source File

SOURCE=.\rfidb.h
# End Source File
# Begin Source File

SOURCE=.\rfii.c
# End Source File
# Begin Source File

SOURCE=.\rfii.h
# End Source File
# Begin Source File

SOURCE=.\rfim.c
# End Source File
# Begin Source File

SOURCE=.\rfim.h
# End Source File
# Begin Source File

SOURCE=.\rfinitc.c
# End Source File
# Begin Source File

SOURCE=.\rfinitc.h
# End Source File
# Begin Source File

SOURCE=.\rfiter.c
# End Source File
# Begin Source File

SOURCE=.\rfiter.h
# End Source File
# Begin Source File

SOURCE=.\rfiv.c
# End Source File
# Begin Source File

SOURCE=.\rfiv.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_cp.c
# End Source File
# Begin Source File

SOURCE=.\rfmat_cp.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_fp.c
# End Source File
# Begin Source File

SOURCE=.\rfmat_fp.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_mp.c
# End Source File
# Begin Source File

SOURCE=.\rfmat_mp.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_sp.c
# End Source File
# Begin Source File

SOURCE=.\rfmat_sp.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_tp.c
# End Source File
# Begin Source File

SOURCE=.\rfmat_tp.h
# End Source File
# Begin Source File

SOURCE=.\rfmodel.c
# End Source File
# Begin Source File

SOURCE=.\rfmodel.h
# End Source File
# Begin Source File

SOURCE=.\rfod.c
# End Source File
# Begin Source File

SOURCE=.\rfod.h
# End Source File
# Begin Source File

SOURCE=.\rfoutput.c
# End Source File
# Begin Source File

SOURCE=.\rfoutput.h
# End Source File
# Begin Source File

SOURCE=.\rfpriref.c
# End Source File
# Begin Source File

SOURCE=.\rfpriref.h
# End Source File
# Begin Source File

SOURCE=.\rfsolver.c
# End Source File
# Begin Source File

SOURCE=.\rfsolver.h
# End Source File
# Begin Source File

SOURCE=.\rfsousin.c
# End Source File
# Begin Source File

SOURCE=.\rfsousin.h
# End Source File
# Begin Source File

SOURCE=.\rfstring.c
# End Source File
# Begin Source File

SOURCE=.\rfstring.h
# End Source File
# Begin Source File

SOURCE=.\rfsystim.c
# End Source File
# Begin Source File

SOURCE=.\rfsystim.h
# End Source File
# Begin Source File

SOURCE=.\rftime.c
# End Source File
# Begin Source File

SOURCE=.\rftime.h
# End Source File
# Begin Source File

SOURCE=.\solver.c
# End Source File
# Begin Source File

SOURCE=.\solver.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\steam.c
# End Source File
# Begin Source File

SOURCE=.\steam67.h
# End Source File
# Begin Source File

SOURCE=.\testvar.c
# End Source File
# Begin Source File

SOURCE=.\testvar.h
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\tools.c
# End Source File
# Begin Source File

SOURCE=.\tools.h
# End Source File
# Begin Source File

SOURCE=.\txtinout.c
# End Source File
# Begin Source File

SOURCE=.\txtinout.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Target
# End Project

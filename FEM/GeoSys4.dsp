# Microsoft Developer Studio Project File - Name="GeoSys4" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=GeoSys4 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GeoSys4.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GeoSys4.mak" CFG="GeoSys4 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GeoSys4 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "GeoSys4 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GeoSys4 - Win32 Release"

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
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "GeoSys4 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GeoSys4 - Win32 Release"
# Name "GeoSys4 - Win32 Debug"
# Begin Group "FEM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\adaptiv.cpp
# End Source File
# Begin Source File

SOURCE=.\adaptiv.h
# End Source File
# Begin Source File

SOURCE=.\balance.cpp
# End Source File
# Begin Source File

SOURCE=.\balance.h
# End Source File
# Begin Source File

SOURCE=.\break.cpp
# End Source File
# Begin Source File

SOURCE=.\break.h
# End Source File
# Begin Source File

SOURCE=.\cel_agm.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_agm.h
# End Source File
# Begin Source File

SOURCE=.\cel_asm.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_asm.h
# End Source File
# Begin Source File

SOURCE=.\cel_dm.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_dm.h
# End Source File
# Begin Source File

SOURCE=.\cel_htm.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_htm.h
# End Source File
# Begin Source File

SOURCE=.\cel_mmp.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_mmp.h
# End Source File
# Begin Source File

SOURCE=.\cel_mms.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_mms.h
# End Source File
# Begin Source File

SOURCE=.\cel_mpc.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_mpc.h
# End Source File
# Begin Source File

SOURCE=.\cel_mtm2.cpp
# End Source File
# Begin Source File

SOURCE=.\cel_mtm2.h
# End Source File
# Begin Source File

SOURCE=.\cgs_agm.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_agm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_asm.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_asm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_dm.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_dm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_ge.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_ge.h
# End Source File
# Begin Source File

SOURCE=.\cgs_htm.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_htm.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mmp.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_mmp.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mms.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_mms.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mpc.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_mpc.h
# End Source File
# Begin Source File

SOURCE=.\cgs_mtm2.cpp
# End Source File
# Begin Source File

SOURCE=.\cgs_mtm2.h
# End Source File
# Begin Source File

SOURCE=.\cvel.cpp
# End Source File
# Begin Source File

SOURCE=.\cvel.h
# End Source File
# Begin Source File

SOURCE=.\display.cpp
# End Source File
# Begin Source File

SOURCE=.\display.h
# End Source File
# Begin Source File

SOURCE=.\edges.cpp
# End Source File
# Begin Source File

SOURCE=.\edges.h
# End Source File
# Begin Source File

SOURCE=.\element_new.cpp
# End Source File
# Begin Source File

SOURCE=.\element_new.h
# End Source File
# Begin Source File

SOURCE=.\elements.cpp
# End Source File
# Begin Source File

SOURCE=.\elements.h
# End Source File
# Begin Source File

SOURCE=.\femlib.cpp
# End Source File
# Begin Source File

SOURCE=.\femlib.h
# End Source File
# Begin Source File

SOURCE=.\file_rfr.cpp
# End Source File
# Begin Source File

SOURCE=.\file_rfr.h
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\files0.cpp
# End Source File
# Begin Source File

SOURCE=.\files1.cpp
# End Source File
# Begin Source File

SOURCE=.\files2.cpp
# End Source File
# Begin Source File

SOURCE=.\gridadap.cpp
# End Source File
# Begin Source File

SOURCE=.\gridadap.h
# End Source File
# Begin Source File

SOURCE=.\indicat1.cpp
# End Source File
# Begin Source File

SOURCE=.\indicat2.cpp
# End Source File
# Begin Source File

SOURCE=.\indicatr.cpp
# End Source File
# Begin Source File

SOURCE=.\indicatr.h
# End Source File
# Begin Source File

SOURCE=.\int_agm.cpp
# End Source File
# Begin Source File

SOURCE=.\int_agm.h
# End Source File
# Begin Source File

SOURCE=.\int_asm.cpp
# End Source File
# Begin Source File

SOURCE=.\int_asm.h
# End Source File
# Begin Source File

SOURCE=.\int_atm.cpp
# End Source File
# Begin Source File

SOURCE=.\int_atm.h
# End Source File
# Begin Source File

SOURCE=.\int_ge.cpp
# End Source File
# Begin Source File

SOURCE=.\int_ge.h
# End Source File
# Begin Source File

SOURCE=.\int_htm.cpp
# End Source File
# Begin Source File

SOURCE=.\int_htm.h
# End Source File
# Begin Source File

SOURCE=.\int_mmp.cpp
# End Source File
# Begin Source File

SOURCE=.\int_mmp.h
# End Source File
# Begin Source File

SOURCE=.\int_mms.cpp
# End Source File
# Begin Source File

SOURCE=.\int_mms.h
# End Source File
# Begin Source File

SOURCE=.\int_mtm2.cpp
# End Source File
# Begin Source File

SOURCE=.\int_mtm2.h
# End Source File
# Begin Source File

SOURCE=.\intrface.cpp
# End Source File
# Begin Source File

SOURCE=.\intrface.h
# End Source File
# Begin Source File

SOURCE=.\loop_pcs.cpp
# End Source File
# Begin Source File

SOURCE=.\loop_pcs.h
# End Source File
# Begin Source File

SOURCE=.\makros.h
# End Source File
# Begin Source File

SOURCE=.\masscont.cpp
# End Source File
# Begin Source File

SOURCE=.\masscont.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.cpp
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\matrix_class.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix_class.h
# End Source File
# Begin Source File

SOURCE=.\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\nodes.h
# End Source File
# Begin Source File

SOURCE=.\par_ddc.cpp
# End Source File
# Begin Source File

SOURCE=.\par_ddc.h
# End Source File
# Begin Source File

SOURCE=.\pcs_dm.cpp
# End Source File
# Begin Source File

SOURCE=.\pcs_dm.h
# End Source File
# Begin Source File

SOURCE=.\plains.cpp
# End Source File
# Begin Source File

SOURCE=.\plains.h
# End Source File
# Begin Source File

SOURCE=.\plot.cpp
# End Source File
# Begin Source File

SOURCE=.\plot.h
# End Source File
# Begin Source File

SOURCE=.\prototyp.h
# End Source File
# Begin Source File

SOURCE=.\ptrarr.cpp
# End Source File
# Begin Source File

SOURCE=.\ptrarr.h
# End Source File
# Begin Source File

SOURCE=.\refine.h
# End Source File
# Begin Source File

SOURCE=.\refine1.cpp
# End Source File
# Begin Source File

SOURCE=.\refine2.cpp
# End Source File
# Begin Source File

SOURCE=.\refine3.cpp
# End Source File
# Begin Source File

SOURCE=.\relax.cpp
# End Source File
# Begin Source File

SOURCE=.\relax.h
# End Source File
# Begin Source File

SOURCE=.\renumber.cpp
# End Source File
# Begin Source File

SOURCE=.\renumber.h
# End Source File
# Begin Source File

SOURCE=.\rf.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_apl.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_apl.h
# End Source File
# Begin Source File

SOURCE=.\rf_bc_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_bc_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_db.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_db.h
# End Source File
# Begin Source File

SOURCE=.\rf_fct.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_fct.h
# End Source File
# Begin Source File

SOURCE=.\rf_ic_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_ic_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_mfp_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_mfp_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_mg.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_mg.h
# End Source File
# Begin Source File

SOURCE=.\rf_mmp_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_mmp_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_msp_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_msp_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_node.h
# End Source File
# Begin Source File

SOURCE=.\rf_num_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_num_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_out_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_out_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_pcs.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_pcs.h
# End Source File
# Begin Source File

SOURCE=.\rf_react.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_react.h
# End Source File
# Begin Source File

SOURCE=.\rf_st_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_st_new.h
# End Source File
# Begin Source File

SOURCE=.\rf_tim_new.cpp
# End Source File
# Begin Source File

SOURCE=.\rf_tim_new.h
# End Source File
# Begin Source File

SOURCE=.\rfadt.cpp
# End Source File
# Begin Source File

SOURCE=.\rfadt.h
# End Source File
# Begin Source File

SOURCE=.\rfdb.cpp
# End Source File
# Begin Source File

SOURCE=.\rfdb.h
# End Source File
# Begin Source File

SOURCE=.\rfhgm.cpp
# End Source File
# Begin Source File

SOURCE=.\rfhgm.h
# End Source File
# Begin Source File

SOURCE=.\rfidb.cpp
# End Source File
# Begin Source File

SOURCE=.\rfidb.h
# End Source File
# Begin Source File

SOURCE=.\rfii.cpp
# End Source File
# Begin Source File

SOURCE=.\rfii.h
# End Source File
# Begin Source File

SOURCE=.\rfim.cpp
# End Source File
# Begin Source File

SOURCE=.\rfim.h
# End Source File
# Begin Source File

SOURCE=.\rfiter.cpp
# End Source File
# Begin Source File

SOURCE=.\rfiter.h
# End Source File
# Begin Source File

SOURCE=.\rfiv.cpp
# End Source File
# Begin Source File

SOURCE=.\rfiv.h
# End Source File
# Begin Source File

SOURCE=.\rfmat_cp.cpp
# End Source File
# Begin Source File

SOURCE=.\rfmat_cp.h
# End Source File
# Begin Source File

SOURCE=.\rfod.cpp
# End Source File
# Begin Source File

SOURCE=.\rfod.h
# End Source File
# Begin Source File

SOURCE=.\rfpriref.cpp
# End Source File
# Begin Source File

SOURCE=.\rfpriref.h
# End Source File
# Begin Source File

SOURCE=.\rfstring.cpp
# End Source File
# Begin Source File

SOURCE=.\rfstring.h
# End Source File
# Begin Source File

SOURCE=.\rfsystim.cpp
# End Source File
# Begin Source File

SOURCE=.\rfsystim.h
# End Source File
# Begin Source File

SOURCE=.\solver.cpp
# End Source File
# Begin Source File

SOURCE=.\solver.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\steam.cpp
# End Source File
# Begin Source File

SOURCE=.\steam67.h
# End Source File
# Begin Source File

SOURCE=.\testvar.cpp
# End Source File
# Begin Source File

SOURCE=.\testvar.h
# End Source File
# Begin Source File

SOURCE=.\timer.cpp
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\tools.cpp
# End Source File
# Begin Source File

SOURCE=.\tools.h
# End Source File
# End Group
# Begin Group "GEO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Geo\geo_db.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_db.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_dom.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_dom.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_lib.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_lib.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_lin.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_lin.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_ply.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_ply.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_pnt.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_pnt.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_sfc.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_sfc.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_strings.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_strings.h
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_vol.cpp
# End Source File
# Begin Source File

SOURCE=..\Geo\geo_vol.h
# End Source File
# End Group
# Begin Group "MSH"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Msh\msh_nodes.cpp
# End Source File
# Begin Source File

SOURCE=..\Msh\msh_nodes.h
# End Source File
# Begin Source File

SOURCE=..\Msh\mshlib.cpp
# End Source File
# Begin Source File

SOURCE=..\Msh\mshlib.h
# End Source File
# Begin Source File

SOURCE=..\Msh\prisgen.cpp
# End Source File
# Begin Source File

SOURCE=..\Msh\prisgen.h
# End Source File
# End Group
# End Target
# End Project

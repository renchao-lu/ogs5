// GeoSysDoc.cpp : implementation of the CGeoSysDoc class
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include "mainfrm.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include "GeoSysListView.h"
#include "GSForm3DLeft.h"
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#include "MainFrm.h" // Progress bar
// C++ STL
#include <string>
#include <iostream>
using namespace std;
// Shared 
#include "makros.h"
#define FILE_EXTENSION_LENGTH 3
#include "tools.h" //SB todo
extern string GetLineFromFile1(ifstream*);
// GEOLib
#include "geo_lib.h"
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
#include "geo_strings.h"
// MSHLib
#include "nodes.h"
#include "msh_lib.h"
#include "femlib.h"
#include "fem_ele.h"
// FEMLib
#include "rf_tim_new.h"
#include "rf_out_new.h"
#include "rf_bc_new.h"
#include "rf_st_new.h"
#include "rf_ic_new.h"
#include "rf_mmp_new.h"
#include "rf_mfp_new.h"
#include "rf_msp_new.h"
#include "rfmat_cp.h"
#include "pcs_dm.h"
#ifdef PROBLEM_CLASS //16.12.2008. WW
#include "problem.h"
Problem *aproblem = NULL;
#else
#include "rf_apl.h"
#include "loop_pcs.h"
#endif


extern bool RFDOpen(string file_name_base);
#include "rf_fct.h"
#include "par_ddc.h"
#include "rf_react.h" //OK
// GeoSys-LIB
#include "shp.h"
#include "dlg_shp.h"
#include "dlg_GHDB.h"//FS
#include ".\geosysdoc.h"
#include "dlg_database.h"//CC 12/2008
// Dialogs
#include "gs_newproject.h"
#include "gs_project.h"
#include "sim_dlg.h"
#include "gs_graphics.h"
#include "feflow_dlg.h" //OK
#include "rf_kinreact.h" //OK

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TK
bool m_SHP = false;
int p=0; //CGeoSysDoc::OnNewDocument
string ext_file_name;   //WW

/////////////////////////////////////////////////////////////////////////////
// CGeoSysDoc

IMPLEMENT_DYNCREATE(CGeoSysDoc, CDocument)

BEGIN_MESSAGE_MAP(CGeoSysDoc, CDocument)
    // Add GSP member
    ON_COMMAND(ID_ADD_GEO, OnAddGEO)
    ON_COMMAND(ID_ADD_MSH, OnAddMSH)
    ON_COMMAND(ID_ADD_FEM, OnAddFEM)
    // Remove GSP member
    ON_COMMAND(ID_REMOVE_GEO, OnRemoveGEO)
    ON_COMMAND(ID_REMOVE_MSH, OnRemoveMSH)
    // Save GSP member
    ON_COMMAND(ID_EXPORT_MSHFILE, OnSaveMSH)
    ON_COMMAND(ID_EXPORT_FEMFILE, OnSaveFEM)
    // Save GSP member as
    ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveAs)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_SAVEAS_GEO, OnSaveAsGEO)
    ON_COMMAND(ID_SAVEAS_MSH, OnSaveAsMSH)
    ON_COMMAND(ID_SAVEAS_FEM, OnSaveAsFEM)
    // Import data
	ON_COMMAND(ID_IMPORT_SHP, OnImportSHP)
    ON_COMMAND(ID_IMPORT_RFO, OnImportRFO)
    ON_COMMAND(ID_IMPORT_BMP, OnImportBMP) //OK
    ON_COMMAND(ID_IMPORT_EXCEL, OnImportEXCEL_PNT) //OK
    ON_COMMAND(ID_IMPORT_EXCEL_FCT,OnImportEXCEL_FCT) //OK
    ON_COMMAND(ID_DEM_ASC, OnImportASC) //CC
	ON_COMMAND(ID_IMPORT_SHP_NEW, OnImportSHPNew) //OK
	ON_COMMAND(ID_IMPORT_GHDBDATABASE, OnImportGHDB)//CC
	ON_COMMAND(ID_IMPORT_FLAC, OnImportFLAC) //OK
    ON_COMMAND(ID_IMPORT_FLAC_MESH, OnImportFLACMesh) //MR
    ON_COMMAND(ID_IMPORT_FEFLOW, OnImportFEFLOW) //OK
    // Export data data
    ON_COMMAND(ID_EXPORT_TEC, OnExportTecFile)
    // Simulator
	ON_COMMAND(ID_SIMULATOR_FORWARD, OnSimulatorForward)
    ON_COMMAND(ID_REMOVE_FEM, OnRemoveFEM)
    ON_UPDATE_COMMAND_UI(ID_ADD_GEO, OnUpdateAddGEO)
    ON_UPDATE_COMMAND_UI(ID_ADD_MSH, OnUpdateAddMSH)
    ON_UPDATE_COMMAND_UI(ID_ADD_FEM, OnUpdateAddFEM)
    ON_UPDATE_COMMAND_UI(ID_REMOVE_GEO, OnUpdateRemoveGEO)
    ON_UPDATE_COMMAND_UI(ID_REMOVE_MSH, OnUpdateRemoveMSH)
    ON_UPDATE_COMMAND_UI(ID_REMOVE_FEM, OnUpdateRemoveFEM)
    ON_COMMAND(ID_SIMULATOR_CHECKSTATUS, OnSimulatorCheckStatus)
    ON_UPDATE_COMMAND_UI(ID_SIMULATOR_FORWARD, OnUpdateSimulatorForward)
    ON_COMMAND(ID_IMPORT_GMS, OnImportGMS)
	ON_COMMAND(ID_GHDBVIEW_CREATE, &CGeoSysDoc::OnGhdbviewCreate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeoSysDoc construction/destruction

CGeoSysDoc::CGeoSysDoc()
{
  m_strFileNameRFD = "";
  m_bDataGEO = FALSE;
  m_bDataMSH = FALSE;
  m_bDataFEM = FALSE;
  m_strGSPFileBase = "new";
  m_dXmin = 0.0;
  m_dXmax = 1.0;
  m_dYmin = 0.0;
  m_dYmax = 1.0;
  m_bDataRFO = false;
  m_bDataPCS = false;
  m_bReady2Run = false;
  m_nodes_elements = NULL; //OK
#ifdef PROBLEM_CLASS //16.12.2008. WW
  problem_win = NULL;
#endif
}

CGeoSysDoc::~CGeoSysDoc()
{
  //if (m_strFileNameBase != "new") ID_FILE_CLOSE;

  //Haibing reclaim the memory -----
  long i;
  for (i=0; i<(long)material_groups_vector.size();i++)delete material_groups_vector[i];
  material_groups_vector.clear();
  for (i=0; i<(long)gsp_vector.size();i++)delete gsp_vector[i];
  gsp_vector.clear();
  /*
  list<CBoundaryConditionsGroup*>::iterator ii;
  for (ii=bc_group_list.begin();ii != bc_group_list.end();ii++)delete *ii;
  bc_group_list.clear();*/
  /*list<CSourceTermGroup*>::iterator jj;
  for (jj=st_group_list.begin();jj != st_group_list.end();jj++)delete *jj;
  st_group_list.clear();*/


  list<CMediumPropertiesGroup*>::iterator kk;
  for (kk=mmp_group_list.begin();kk != mmp_group_list.end();kk++)delete *kk;
  mmp_group_list.clear();

  //OnRemoveFEM(); can not be called here, otherwise the files will be modified.
#ifdef PROBLEM_CLASS //16.12.2008. WW
  delete problem_win;
  problem_win = NULL;
#else 
  GEOLIB_Clear_GeoLib_Data();

  PCSDelete();
  NUMDelete();
  TIMDelete();
  OUTDelete();
  ICDelete();
  BCDelete();
  STDelete();
  MFPDelete();
  MSPDelete();
  MMPDelete();
  MCPDelete();
  PCSDestroyAllProcesses();//Do not put this before PCSDelete, it will cause crash
#endif
  //--------------------------------------------------
}

void CGeoSysDoc::InitDocument() //CC 
{
  m_sizeDoc = CSize(800,900);
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysDoc OnNewDocument
/**************************************************************************
GeoSysGUI-Method:
Task: 
Programing:
??/2004 TK Implementation
01/2005 OK GSP
06/2005 OK modeless control panel
**************************************************************************/
BOOL CGeoSysDoc::OnNewDocument()
{
  int OGLView_OnOff = 0; /*0=OFF / 1=ON*/ 
  int GeoSysView_OnOff = 0; /*0=OFF / 1=ON*/ 
  CFrameWnd			*pWndSecondChild ;
  CMultiDocTemplate	*pDocTemplate ;
  p++; 
  if (!CDocument::OnNewDocument())
	return FALSE;
  else
  {
	CGS_newproject m_newproject;
	m_newproject.m_strProjectName = "";
	m_newproject.m_strFolderPath = "C:\\";
    if(m_newproject.DoModal()==IDOK)
	{
      //------------------------------------------------------------------
	  // Get Project Path and Base
      m_strGSPFilePath = m_newproject.m_strFolderPath;
      m_strGSPFileBase = m_newproject.m_strProjectName;
	  m_strGSPFilePathBaseExt = m_strGSPFilePath + '\\' + m_strGSPFileBase + ".gsp";
	  m_strGSPFilePathBase = m_strGSPFilePath + '\\' + m_strGSPFileBase; // path+base OK4104
      g_gsp_path = m_strGSPFilePath; //OK
      g_gsp_base = m_strGSPFileBase; //OK
      //------------------------------------------------------------------
      // Create GSP directory if necessary
      CreateDirectory(m_strGSPFilePath,NULL); 
      OnFileSave();
      OGLView_OnOff = 1;
      //------------------------------------------------------------------
      // Create GSP file
/*OK
      FILE* CheckFile;
      CheckFile = fopen(m_strGSPFileNamePathBaseType, "r");
	  if (CheckFile == NULL)
	  {
		//OK OnCreateProjectFile(m_strFileNamePathBaseExt);
        CreateDirectory(m_strGSPFileNamePath,NULL); //OK only path
		OGLView_OnOff = GeoSysView_OnOff = 1;
	  }
	  else
	  {
	    AfxMessageBox("The Project Already Exists ! \n\n Try Another Project Name");
		OGLView_OnOff = GeoSysView_OnOff = 0;
		fclose(CheckFile);
	  }
*/
      //------------------------------------------------------------------
      // Control panel //OK
/*OK4216
      theApp->g_graphics_modeless_dlg = new CGraphics;
      theApp->g_graphics_modeless_dlg->Create(IDD_DIALOG_GRAPHICS);
      theApp->g_graphics_modeless_dlg->ShowWindow(SW_SHOW);
*/
      //------------------------------------------------------------------
	  // Create secondary child frame with view and document. 'theApp' is defined at 
	  // the bottom of the app's class' header file, so that we can easily refer to 
	  // the application object.
	  //CC
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	  pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_SECONDVIEW);
      if (GeoSysView_OnOff == 0) 
        pDocTemplate = NULL; //Deaktiviert 2D View bei "New"
	  if (pDocTemplate != NULL) //Aktiviert 2D View bei "New"
	  {
	    // Note: First child frame does not exist at this point, so we cannot refer to it.
		pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
		ASSERT(pWndSecondChild != NULL);
		// Third parameter (make window visible) defaults to TRUE.
		pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
	  }
	  // TODO: add reinitialization code here
	  // (SDI documents will reuse this document)
	  //OKchanged return FALSE; //Deaktiviert OpenGL View bei "New"
	  if (OGLView_OnOff == 1) 
        return TRUE;  //Aktiviert OpenGL View bei "New"
	  else 
        return FALSE;
	}
	else
	{
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
	  pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_SECONDVIEW); //OK
	  pDocTemplate = NULL;//Deaktiviert 2D View bei "New"
	  if (pDocTemplate != NULL)//Aktiviert 2D View bei "New"
	  { 
	    pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
	    ASSERT(pWndSecondChild != NULL);
	    pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
	  }
	  return FALSE;  //(De)Aktiviert OpenGL View bei "New"
	}
  }
}

////////////////////////////////////////////////////////////////////////////
// CGeoSysDoc serialization
/**************************************************************************
GeoSys-Method: Serialize
Task:
Programing:
11/2003 OK GEO,MSH,FEM open functions
03/2004 TK ExecuteProjectFile
01/2005 OK GSPRead/GSPReadWIN
01/2005 OK Serialize: File-Save
06/2005 OK modeless Control Panel
12/2008 WW Replace the creation of PCS with the instance of the new class
**************************************************************************/
void CGeoSysDoc::Serialize(CArchive& ar)
{
  char a = '\\';
  int pos;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //----------------------------------------------------------------------
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  mainframe->OnDrawFromToolbar();
  //----------------------------------------------------------------------
  // Save ================================================================
  if (ar.IsStoring())
  {
    m_strGSPFilePathBaseExt = ar.m_strFileName;
    pos = m_strGSPFilePathBaseExt.ReverseFind(a);
    CString m_strGSPFileNamePath = m_strGSPFilePathBaseExt.Left(pos+1);
    g_gsp_path = m_strGSPFileNamePath;
	pos = m_strGSPFilePathBaseExt.GetLength()-(pos+1);
	CString m_strGSPFileNameBase = m_strGSPFilePathBaseExt.Right(pos);
    pos = m_strGSPFileNameBase.ReverseFind('.');
    m_strGSPFileNameBase = m_strGSPFileNameBase.Left(pos);
    g_gsp_base = m_strGSPFileNameBase;
    //--------------------------------------------------------------------
    // Write GSP file
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Write GSP file");
    GSPWriteWIN(ar);
    //--------------------------------------------------------------------
    // Write GSP data
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Write GSP data");
    GSPWriteData();
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Data output completed");
  }
  // Open ================================================================
  else
  {
    //--------------------------------------------------------------------
    // File handling
    m_strGSPFilePathBaseExt = ar.m_strFileName;
    pos = m_strGSPFilePathBaseExt.ReverseFind(a);
    m_strGSPFilePath = m_strGSPFilePathBaseExt.Left(pos+1);
	pos = m_strGSPFilePathBaseExt.GetLength()-(pos+1);
	m_strGSPFileBase = m_strGSPFilePathBaseExt.Right(pos);
    pos = m_strGSPFileBase.ReverseFind('.');
    m_strGSPFileBase = m_strGSPFileBase.Left(pos);
    m_strGSPFileExt = m_strGSPFilePathBaseExt.Right(FILE_EXTENSION_LENGTH+1);
	m_strGSPFilePathBase = m_strGSPFilePath + m_strGSPFileBase;
    // GSP
    g_gsp_path = m_strGSPFilePath;
    g_gsp_base = m_strGSPFileBase;
    // Instructions to statusbar
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"");
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read data");
    //--------------------------------------------------------------------
    // GSP project
	if(m_strGSPFileExt==".gsp")
    {
      // Read GSP file
      GSPReadWIN(ar);
      if(m_bDataMSH)
      {
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Config MSH data");
        CompleteMesh(); //WW
        GetHeterogeneousFields(); //OK/MB
        MSHTestMATGroups(); //OK Test MSH-MMP
        DDCCreate(); //OK
        ConfigSolverProperties();
        //ConfigTopology(); // max_dim for solver, elements to nodes relationships
      }
      // Create PCS data
      if(pcs_created)
      {
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Create PCS data");
#ifdef PROBLEM_CLASS //16.12.2008. WW
        problem_win = new Problem();
        aproblem = problem_win;
#else
        PCSCreate(); //WW
#endif
        //PCSCreateNew(); //OK Create PCS
		//REACTInit(); //OK Initialization of REACT structure for rate exchange between MTM2 and Reactions
        //PCSRestart(); //SB
        m_bDataPCS = true;
        m_bDataFEM = true;
      }
      //------------------------------------------------------------------
      // Create Views for existing data
/*OK open on request
      CFrameWnd			*pWndSecondChild ;
      CMultiDocTemplate	*pDocTemplate ;
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
      //..................................................................
      if(m_bDataGEO){
	    pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_GEO_VIEW);
	    if (pDocTemplate != NULL)
	    {
		  pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
		  ASSERT(pWndSecondChild != NULL);
		  pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
	    }
        mainframe->m_bIsGEOViewOpen = true; //OK
      }
      //..................................................................
      if(m_bDataMSH){
	    pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_MSH_VIEW);
	    if (pDocTemplate != NULL)
	    {
		  pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
		  ASSERT(pWndSecondChild != NULL);
		  pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
	    }
        mainframe->m_bIsMSHViewOpen = true; //OK
      }
      //..................................................................
      if(m_bDataPCS){
	    pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_SECONDVIEW);
	    if (pDocTemplate != NULL)
	    {
		  pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
		  ASSERT(pWndSecondChild != NULL);
		  pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
	    }
      }
*/
      //------------------------------------------------------------------
	}
    else{
      AfxMessageBox("Select GSP file, please.");
    }
    //--------------------------------------------------------------------
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Data input finished");
    //------------------------------------------------------------------
    // Control panel //OK
/*OK
    CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
    theApp->CreateControlPanel();
    theApp->g_graphics_modeless_dlg->DestroyWindow();
    theApp->g_graphics_modeless_dlg->m_bIsControlPanelOpen = false; //TK
*/
  }
}

/**************************************************************************
GeoSys-Method:
Task:
Programing:
01/2005 OK 
04/2005 OK GSP ASCII version
10/2005 OK DOMRead
02/2006 WW
**************************************************************************/
void CGeoSysDoc::GSPReadWIN(CArchive& ar)
{
  char a = '\\';
  int pos;
  CGSProject* m_gsp = NULL;
  CWnd*pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_strLine;
  bool m_bIsASCII = false;
  char line[MAX_ZEILE];
  string line_string;
  ifstream gsp_file;
  //----------------------------------------------------------------------
  CString m_strFileNamePathBaseExt = ar.m_strFileName;
  pos = m_strFileNamePathBaseExt.ReverseFind(a);
  CString m_strGSPFileNamePath = m_strFileNamePathBaseExt.Left(pos+1);
  g_gsp_path = m_strGSPFileNamePath;
  //----------------------------------------------------------------------
//  ar.ReadString(m_strLine);
  string gsp_file_name = m_strFileNamePathBaseExt;
  gsp_file.open(gsp_file_name.data(),ios::in);
  if(!gsp_file.good()) 
    return;
  gsp_file.seekg(0L,ios::beg);
  gsp_file.getline(line,MAX_ZEILE);
  if(line[0]=='#')
    m_bIsASCII = true;
  if(m_bIsASCII){
  }
  else{
    ar >> m_strLine;
  }
  //----------------------------------------------------------------------
  while(1){
    if(m_bIsASCII){
      gsp_file.getline(line,MAX_ZEILE);
      m_strLine = line;
      m_strLine = m_strLine.Trim(); //NW
    }
    else
      ar >> m_strLine;
    if(m_strLine.Find("#STOP")==0) //OK
      return;
    pos = m_strLine.ReverseFind('.');
    CString m_strGSPFileNameBase = m_strLine.Left(pos);
	pos = m_strLine.GetLength()-(pos+1);
	CString m_strGSPFileNameType = m_strLine.Right(pos);
    m_gsp = new CGSProject;
    m_gsp->path = (string)m_strGSPFileNamePath;
	m_gsp->base = (string)m_strGSPFileNameBase;
	m_gsp->type = (string)m_strGSPFileNameType;
	gsp_vector.push_back(m_gsp);
    string gsp_member_path_base = m_gsp->path + m_gsp->base;
    ext_file_name = gsp_member_path_base;     //WW
    //....................................................................
    if(m_strGSPFileNameType=="gli"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read GEO data");
      GEOLIB_Read_GeoLib(gsp_member_path_base);
      m_bDataGEO = TRUE;
    }
    //....................................................................
    if(m_strGSPFileNameType=="rfi"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read RFI data");
      FEMRead(gsp_member_path_base);   // PCH This is added for temperary purpose
      PCTRead(gsp_member_path_base);   // PCH This is added for temperary purpose
      m_bDataMSH = TRUE;
    }
    //....................................................................
    if(m_strGSPFileNameType=="msh"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read MSH data");
      FEMRead(gsp_member_path_base);
	  PCTRead(gsp_member_path_base);   // PCH This is added for temperary purpose
	 // CompleteMesh(); //WW  
      m_bDataMSH = TRUE;
    }
    //....................................................................
    if(m_strGSPFileNameType=="pcs"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read PCS data");
      PCSRead(gsp_member_path_base);
      pcs_created = true;
    }
    //....................................................................
    if(m_strGSPFileNameType=="num"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read NUM data");
      NUMRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="tim"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read TIM data");
      TIMRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="out"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read OUT data");
      OUTRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="ic"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read IC data");
      ICRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="bc"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read BC data");
      BCRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="st"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read ST data");
      STRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="mfp"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read MFP data");
      MFPRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="msp"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read MSP data");
      MSPRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="mmp"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read MMP data");
      MMPRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="mcp"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read MCP data");
      CPRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="rfd"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read RFD data");
      CURRead(gsp_member_path_base); //WWRFDOpen(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="fct"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read FCT data");
      FCTRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="ddc"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read DOM data");
      DOMRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="krc"){  //WW
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read KRC data");
       KRRead(gsp_member_path_base);
    }
    //....................................................................
    if(m_strGSPFileNameType=="pct"){
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read PCT data");
    }
    //....................................................................
  }
}

/**************************************************************************
GeoSys-Method:
Task:
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::GSPWriteWIN(CArchive& ar)
{
  int i;
  CGSProject* m_gsp = NULL;
  CString m_strLine;
  CWnd*pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  //----------------------------------------------------------------------
  m_strLine = "#PROJECT_MEMBER";
  ar << m_strLine;
  //----------------------------------------------------------------------
  // Check GSP member
  m_gsp = GSPGetMember("pcs");
  if(m_gsp){//OK41+
    string base_bc = m_gsp->base + "." + "bc";
    int bc_list_size = (int)bc_list.size();
    if(bc_list_size>0)
      GSPAddMember(base_bc);
    string base_mmp = m_gsp->base + "." + "mmp";
    int mmp_vector_size = (int)mmp_vector.size();
    if(mmp_vector_size>0)
      GSPAddMember(base_mmp);
  }
  //----------------------------------------------------------------------
  int gsp_vector_size =(int)gsp_vector.size();
  for(i=0;i<gsp_vector_size;i++){
    m_gsp = gsp_vector[i];
    m_strLine  = m_gsp->base.data();
    m_strLine += ".";
    m_strLine += m_gsp->type.data();
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strLine);
    ar << m_strLine;
  }
  m_strLine = "#STOP";
  ar << m_strLine;
  //----------------------------------------------------------------------
  // Write GSP ASCII file
  //----------------------------------------------------------------------
  // File handling
  CString gsp_file_name_ar = ar.m_strFileName;
  int pos = gsp_file_name_ar.ReverseFind('.');
  CString m_strARFilePathBase = gsp_file_name_ar.Left(pos);
  string gsp_file_name = (string)m_strARFilePathBase;
  gsp_file_name += "_ascii.gsp";
  fstream gsp_file (gsp_file_name.data(),ios::trunc|ios::out);
  gsp_file.clear();
  //......................................................................
  gsp_file << "#PROJECT_MEMBER" << endl;
  for(i=0;i<gsp_vector_size;i++)
  {
    m_gsp = gsp_vector[i];
    m_strLine  = m_gsp->base.data();
    m_strLine += ".";
    m_strLine += m_gsp->type.data();
    gsp_file << m_strLine << endl;
  }
  gsp_file << "#STOP";
  //----------------------------------------------------------------------
}

/////////////////////////////////////////////////////////////////////////////
// CGeoSysDoc diagnostics

#ifdef _DEBUG
void CGeoSysDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGeoSysDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

//###########################################################################
// Add Project Member
/**************************************************************************
GeoSys-Method: OnAddGSPMember
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
/*
void CGeoSysDoc::OnAddGSPMember()
{
  FILE *gsp_member_file_orig = NULL;
  FILE *gsp_member_file_copy = NULL;
  CString m_strFileNamePlusExtension;
  // File dialog
  CFileDialog fileDlg(TRUE, "*", NULL, OFN_ENABLESIZING," GEO Files (*.*)|*.*|| ");
  if(fileDlg.DoModal()==IDOK){   
    gsp_member_file_orig = fopen(fileDlg.GetPathName(),"rt"); 
    if(gsp_member_file_orig){
      m_strFileNamePlusExtension = m_strFileNameBase + m_strFileNameExtension;
      GSPAddMember((string)m_strFileNamePlusExtension);
      // Copy file to project folder
	  gsp_member_file_copy = fopen(m_strFileNameBase + m_strFileNameExtension,"w+t");
 	  while (!feof(gsp_member_file_copy)){
	    fgets(input_text,MAX_ZEILE,gsp_member_file_orig);
		FilePrintString(gsp_member_file_copy,input_text); 
	  }
	  fclose(gsp_member_file_copy);
      // GEO member
      if(m_strFileNameExtension=="gli"){
	    GEOLIB_Clear_GeoLib_Data (); //CC->GEODestroy()
		GEOLIB_Read_GeoLib((string)m_strFileNameBase); //CC->GEORead()
      }
      // MSH member
      else if(m_strFileNameExtension=="rfi"){
        MSHOpen((string)m_strFileNameBase);
      }
      // PCS members
      else if(m_strFileNameExtension=="pcs"){
      }
	}
  }
}
*/
/**************************************************************************
GeoSys-Method: OnAddGEO
Task: 
Programing:
11/2003 OK Implementation
        TK
01/2005 TK/OK
01/2005 OK Copy reqired PLY and TIN files
**************************************************************************/
void CGeoSysDoc::OnAddGEO()
{
  CWnd*pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Add GEO data");
  char input_text[1024];
  string input_line;
  string execute_line;
  string file_orig;
  string file_copy;
  FILE* fp_copy = NULL;
  CString m_strMessage;
  CString m_strPathOrig;
  int pos;
  char a = '\\';
  // File dialog
  CFileDialog fileDlg(TRUE, "gli", NULL, OFN_ENABLESIZING," GEO Files (*.gli)|*.gli|| ");
  if(fileDlg.DoModal()==IDOK){
    if (fileDlg.GetFileExt() == "gli")
    {
      m_bDataGEO = TRUE;
      pos = fileDlg.GetPathName().ReverseFind(a);
      m_strPathOrig = fileDlg.GetPathName().Left(pos+1);
      FILE *gsp_member_file_orig = NULL;
      FILE *gsp_member_file_copy = NULL;
      gsp_member_file_orig = fopen(fileDlg.GetPathName(),"rt"); 
      if(gsp_member_file_orig)
      {
       if(m_strPathOrig.Compare(m_strGSPFilePath)!=0){ //OK
        // Copy file to project folder
	    gsp_member_file_copy = fopen(m_strGSPFilePathBase + ".gli","w+t");
 	    while (!feof(gsp_member_file_orig)){
	      fgets(input_text,1024,gsp_member_file_orig);
	      FilePrintString(gsp_member_file_copy,input_text); 
          // Copy PLY and TIN files
          input_line = input_text;
          if(input_line.find(".ply")!=string::npos){
            pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Copy PLY files");
            pos = (int)input_line.find_first_of('.');
            string file_base = input_line.substr(0,pos);
            file_orig = (string)m_strPathOrig    + file_base + ".ply";
            remove_white_space(&file_orig);
            file_copy = (string)m_strGSPFilePath + a + file_base + ".ply";
            remove_white_space(&file_copy);
            execute_line = "copy " + file_orig + " " + file_copy;
            system(execute_line.c_str());
            fp_copy = fopen(file_copy.c_str(),"rt"); 
            if(!fp_copy){
              m_strMessage = "File copy failed: ";
              m_strMessage += file_orig.data();
              AfxMessageBox(m_strMessage);
            }
          }
          if(input_line.find(".tin")!=string::npos){
            pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Copy TIN files");
            pos = (int)input_line.find_first_of('.');
            string file_base_tin = input_line.substr(0,pos);
            file_orig = (string)m_strPathOrig    + file_base_tin + ".tin";
            remove_white_space(&file_orig);
            file_copy = (string)m_strGSPFilePath + file_base_tin + ".tin";
            remove_white_space(&file_copy);
            execute_line = "copy " + file_orig + " " + file_copy;
            system(execute_line.c_str());
            if(!fp_copy){
              m_strMessage = "File copy failed: ";
              m_strMessage += file_orig.data();
              AfxMessageBox(m_strMessage);
            }
          }
	    }
	    fclose(gsp_member_file_copy);
       }
	    fclose(gsp_member_file_orig);
        //Adds a member to the gsp vector
        GSPAddMember((string)m_strGSPFileBase + ".gli");
        // GEO member
	    GEOLIB_Clear_GeoLib_Data (); //CC->GEODestroy()
        pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Read GEO file");
	    GEOLIB_Read_GeoLib((string)m_strGSPFilePathBase); //CC->GEORead()
      }	
      // Serialize: Write GSP file
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Write GSP file");
      OnFileSave();
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Ready");
    }
   CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
   m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
   UpdateAllViews(NULL);
  }
}

void CGeoSysDoc::OnUpdateAddGEO(CCmdUI *pCmdUI)
{
  if(m_bDataGEO)
    pCmdUI->SetCheck(TRUE);
  else
    pCmdUI->SetCheck(FALSE);
}

/**************************************************************************
GeoSys-Method: OnAddMSH
Task: 
Programing:
11/2003 OK Implementation
01/2005 OK MSHDestroy
12/2008 NW Support .MSH
**************************************************************************/
void CGeoSysDoc::OnAddMSH()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Add MSH data");
  char input_text[MAX_ZEILE];
  // File dialog
  CFileDialog fileDlg(TRUE, "msh", NULL, OFN_ENABLESIZING," MSH Files (*.msh)|*.msh|RFI Files (*.rfi)|*.rfi|| "); //NW
  if(fileDlg.DoModal()==IDOK){
    if(fileDlg.GetFileExt()=="rfi"){
      m_bDataMSH = TRUE;
      FILE *gsp_member_file_orig = NULL;
      FILE *gsp_member_file_copy = NULL;
      gsp_member_file_orig = fopen(fileDlg.GetPathName(),"rt"); 
      if(gsp_member_file_orig){
        // Copy file to project folder
        gsp_member_file_copy = fopen(m_strGSPFilePathBase + ".rfi","w+t");
 	    while (!feof(gsp_member_file_orig)){
	      fgets(input_text,1024,gsp_member_file_orig);
		  FilePrintString(gsp_member_file_copy,input_text); 
	    }
	    fclose(gsp_member_file_orig);
	    fclose(gsp_member_file_copy);
        // Add MSH member to GSP vector
        GSPAddMember((string)m_strGSPFileBase + ".rfi");
        // Read MSH file
        MSHDestroy();
        MSHOpen((string)m_strGSPFilePathBase);
      }
      // Serialize: Write GSP file
      OnFileSave();
    } else if (fileDlg.GetFileExt()=="msh") {
      m_bDataMSH = TRUE;
      FILE *gsp_member_file_orig = NULL;
      FILE *gsp_member_file_copy = NULL;
      gsp_member_file_orig = fopen(fileDlg.GetPathName(),"rt"); 
      if(gsp_member_file_orig){
        // Copy file to project folder
        gsp_member_file_copy = fopen(m_strGSPFilePathBase + ".msh","w+t");
 	    while (!feof(gsp_member_file_orig)){
	      fgets(input_text,1024,gsp_member_file_orig);
		  FilePrintString(gsp_member_file_copy,input_text); 
	    }
	    fclose(gsp_member_file_orig);
	    fclose(gsp_member_file_copy);
        // Add MSH member to GSP vector
        GSPAddMember((string)m_strGSPFileBase + ".msh");
        // Read MSH file
        FEMRead((string)m_strGSPFilePathBase);
      }
      // Serialize: Write GSP file
      OnFileSave();
    }
  }
  // Config GSP data
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Config MSH data");
 //16.12.2008. WW  RFPre_Model();
  start_new_elems = ElListSize();
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Config MSH data");
  CompleteMesh();
//  ConfigTopology(); // max_dim for solver, elements to nodes relationships
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Ready");
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  UpdateAllViews(NULL);
}

void CGeoSysDoc::OnUpdateAddMSH(CCmdUI *pCmdUI)
{
  if(!m_bDataGEO)
    pCmdUI->Enable(FALSE);
  if(m_bDataMSH)
    pCmdUI->SetCheck(TRUE);
  else
    pCmdUI->SetCheck(FALSE);
}

/**************************************************************************
GeoSys-Method: OnAddFEM
01/2005 OK Implementation
07/2007 OK Add MODEL
**************************************************************************/
void CGeoSysDoc::OnAddFEM()
{
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Add GSP data");
  CFileDialog fileDlg(TRUE,"gsp",NULL,OFN_ENABLESIZING," GSP Files (*.gsp)|*.gsp||");
  if(fileDlg.DoModal()==IDOK)
  {
    if(fileDlg.GetFileExt()=="gsp")
    {
      m_bDataFEM = TRUE;
      CString m_strPCSFilePathBaseExt = fileDlg.GetPathName();
      int pos = m_strPCSFilePathBaseExt.ReverseFind('.');
      CString m_strPCSFilePathBase = m_strPCSFilePathBaseExt.Left(pos);
      //..................................................................
      if(!PCSRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No PCS file");
      else
      {
        GSPAddMember((string)m_strGSPFileBase + ".pcs");
        m_strPCSTypeName = pcs_vector[pcs_vector.size()-1]->pcs_type_name.data();
      }
      //..................................................................
      if(!NUMRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No NUM file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".num");
      //..................................................................
      if(!TIMRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No TIM file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".tim");
      //..................................................................
      if(!OUTRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No OUT file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".out");
      //..................................................................
      if(!ICRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No IC file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".ic");
      //..................................................................
      if(!BCRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No BC file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".bc");
      //..................................................................
      if(!STRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No ST file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".st");
      //..................................................................
      if(!MFPRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No MFP file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".mfp");
      //..................................................................
      if(!MSPRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No MSP file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".msp");
      //..................................................................
      if(!MMPRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No MMP file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".mmp");
      //..................................................................
      if(!CPRead((string)m_strPCSFilePathBase))
        AfxMessageBox("No MCP file");
      else
        GSPAddMember((string)m_strGSPFileBase + ".mcp");
      //..................................................................
    }
    // Serialize: Write GSP file
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Write GSP file");
    //OK OnFileSave();
  }
  // Create PCS data
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Create PCS data");
  //LOPPreTimeLoop_PCS(); // TK
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Ready");
}

void CGeoSysDoc::OnUpdateAddFEM(CCmdUI *pCmdUI)
{
  if(!m_bDataGEO||!m_bDataMSH)
    pCmdUI->Enable(FALSE);
  if(m_bDataFEM)
    pCmdUI->SetCheck(TRUE);
  else
    pCmdUI->SetCheck(FALSE);
}

//###########################################################################
// Remove Project Member
/**************************************************************************
GeoSys-Method: RemoveMemberfromProject(CString FileNameAndPath)
Task: Liest ?ergbenen MemberType und l?cht die entsprechenden
      Eintr?e aus dem Projektfile
	  L?cht ebenfalls das entsprechende File im Projektverzeichnis
      CString MemberExtension: "gli" "rfi" "rfd"
Programing:
01/2004 TK project related file handling
01/2005 OK GSP
**************************************************************************/
void CGeoSysDoc::GSPRemoveMemberFromProject(CString ext)
{
  // Remove from GSP vector
  GSPRemoveMember((string)ext);
  // Delete File
  CString File = m_strGSPFilePathBase + "." + ext;
  FILE *memberfile = NULL;
  memberfile = fopen(File,"w+t"); 
  fclose(memberfile);
  remove(File);
/*OK
  if(memberfile){
    string execute_line = "delete ";
    execute_line += File;
    system(execute_line.c_str());
  }
*/
/*OK
  int j = unlink (File);
  if (j==0) 
    AfxMessageBox("MEMBER DESTROYED! \n \n "+ File);
  if (j!=0) 
    AfxMessageBox("Can't remove file! \n \n "+ File);
*/
  // Serialize: Write new GSP file
  OnFileSave();
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
12/2003 TK project related file handling
01/2005 TK/OK
**************************************************************************/
void CGeoSysDoc::OnRemoveGEO()
{
  m_bDataGEO = FALSE;
  // Destroy data
  GEOLIB_Clear_GeoLib_Data();
  // Remove GSP member
  GSPRemoveMemberFromProject("gli");
  // Update Views
/*
  POSITION pos = GetFirstViewPosition();
    while(pos!=NULL) {
      CView* pView = GetNextView(pos);
      pView->UpdateWindow();
    }
    SetModifiedFlag(1);
*/ 
  DestroyOGLViewData();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  UpdateAllViews(NULL);
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnUpdateRemoveGEO(CCmdUI *pCmdUI)
{
  if(!m_bDataGEO){
    pCmdUI->SetCheck(FALSE);
    pCmdUI->Enable(FALSE);
  }
}

/**************************************************************************
GeoSys-Method: OnRemoveMSH
Task: 
Programing:
01/2004 TK project related file handling
01/2005 OK MSHDestroy
12/2008 NW Support .msh
**************************************************************************/
void CGeoSysDoc::OnRemoveMSH()
{
  m_bDataMSH = FALSE;
  if (GSPGetMember("rfi")!=NULL) {
    // Destroy MSH data
    MSHDestroy();
    // Remove MSH member
    GSPRemoveMemberFromProject("rfi");
  } else if (GSPGetMember("msh")!=NULL) {
    // Destroy MSH data
    FEMDeleteAll();
    //for (int i=0; i<(int)fem_msh_vector.size(); i++) {
    //  delete fem_msh_vector[i];
    //}
    //fem_msh_vector.clear();
    // Remove MSH member
    GSPRemoveMemberFromProject("msh");
  }
  // Update Views
  DestroyOGLViewData();
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  m_frame->m_something_changed = 1;
  UpdateAllViews(NULL);
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnUpdateRemoveMSH(CCmdUI *pCmdUI)
{
  if(!m_bDataMSH){
    pCmdUI->SetCheck(FALSE);
    pCmdUI->Enable(FALSE);
  }
}

/**************************************************************************
GeoSysGUI-Method:
Task: 
Programing:
02/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnRemoveFEM()
{
  m_bDataFEM = FALSE;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  // Destroy FEM data and remove GSP member
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove PCS");
  PCSDelete();
  GSPRemoveMemberFromProject("pcs");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove NUM");
  NUMDelete();
  GSPRemoveMemberFromProject("num");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove TIM");
  TIMDelete();
  GSPRemoveMemberFromProject("tim");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove OUT");
  OUTDelete();
  GSPRemoveMemberFromProject("out");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove IC");
  ICDelete();
  GSPRemoveMemberFromProject("ic");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove BC");
  BCDelete();
  GSPRemoveMemberFromProject("bc");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove ST");
  STDelete();
  GSPRemoveMemberFromProject("st");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove MFP");
  MFPDelete();
  GSPRemoveMemberFromProject("mfp");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove MSP");
  MSPDelete();
  GSPRemoveMemberFromProject("msp");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove MMP");
  MMPDelete();
  GSPRemoveMemberFromProject("mmp");
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Remove MCP");
  MCPDelete();
  GSPRemoveMemberFromProject("cp");
  // Update Views
  UpdateAllViews(NULL,0L,NULL);
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnUpdateRemoveFEM(CCmdUI *pCmdUI)
{
  if(!m_bDataFEM){
    pCmdUI->SetCheck(FALSE);
    pCmdUI->Enable(FALSE);
  }
}

//###########################################################################
// Save Project Member

/**************************************************************************
GeoSys-Method: 
Task: 
Programing:
11/2003 TK Implementation
12/2003 OK Surfaces + #STOP
**************************************************************************/
void CGeoSysDoc::OnSaveGEO()
{
  CFileDialog fileDlg(FALSE,"gli",NULL,OFN_ENABLESIZING,"GLI Files (*.gli)|*.gli|| ");
  if (fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
	//OK41 GEOWrite((string)m_strFileTitle); //OK
  }
}

/**************************************************************************
GeoSys-Method: OnSaveMSH
Task: 
Programing:
11/2003 TK Implementation
01/2005 OK V4+GSP
**************************************************************************/
void CGeoSysDoc::OnSaveMSH()
{
  CFileDialog fileDlg(FALSE,"rfi",NULL,OFN_ENABLESIZING,"RFI Files (*.rfi)|*.rfi||");
  if(fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
   	char filename[254];
	strcpy(filename,m_strFileTitle);
	DATWriteRFIFile(filename);
  }
}

/**************************************************************************
GeoSys-Method: OnSaveFEM
Task: 
Programing:
11/2003 TK Implementation
02/2004 OK new Write methods
01/2005 OK V4+GSP
**************************************************************************/
void CGeoSysDoc::OnSaveFEM()
{
  CFileDialog fileDlg(FALSE,"pcs",NULL,OFN_ENABLESIZING,"PCS Files (*.pcs)|*.pcs|| ");
  if (fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
    PCSWrite((string)m_strFileTitle);
    NUMWrite((string)m_strFileTitle);
    OUTWrite((string)m_strFileTitle);
    //TIMWrite((string)m_strFileTitle);
    ICWrite((string)m_strFileTitle);
    BCWrite((string)m_strFileTitle);
    STWrite((string)m_strFileTitle);
    MFPWrite((string)m_strFileTitle);
    //MSPWrite((string)m_strFileTitle); //WW todo
    MMPWrite((string)m_strFileTitle);
    CPWrite((string)m_strFileTitle,0); //SB todo
  }
}

//###########################################################################
// Save Project Member as (change file name)

/**************************************************************************
GeoSys-Method: OnSaveAsGEO
Task: 
Programing:
11/2003 TK Implementation
12/2003 OK Surfaces + #STOP
01/2005 OK V4
**************************************************************************/
void CGeoSysDoc::OnSaveAsGEO()
{
  CFileDialog fileDlg(FALSE,"gli",NULL,OFN_ENABLESIZING,"GLI Files (*.gli)|*.gli|| ");
  if (fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
	//OK41 GEOWrite((string)m_strFileTitle); //OK
  }
}

/**************************************************************************
GeoSys-Method: OnSaveAsMSH
Task: 
Programing:
11/2003 TK Implementation
01/2005 OK V4
**************************************************************************/
void CGeoSysDoc::OnSaveAsMSH()
{
  CFileDialog fileDlg(FALSE,"rfi",NULL,OFN_ENABLESIZING,"RFI Files (*.rfi)|*.rfi||");
  if(fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
   	char filename[254];
	strcpy(filename,m_strFileTitle);
	DATWriteRFIFile(filename);
  }
}

/**************************************************************************
GeoSys-Method: OnSaveAsFEM
Task: 
Programing:
11/2003 TK Implementation
01/2005 OK V4
**************************************************************************/
void CGeoSysDoc::OnSaveAsFEM()
{
  CFileDialog fileDlg(FALSE,"pcs",NULL,OFN_ENABLESIZING,"PCS Files (*.pcs)|*.pcs|| ");
  if (fileDlg.DoModal()==IDOK) 
  {
	CString m_strFileTitle = fileDlg.GetFileTitle();
    PCSWrite((string)m_strFileTitle);
    NUMWrite((string)m_strFileTitle);
    OUTWrite((string)m_strFileTitle);
    TIMWrite((string)m_strFileTitle);
    ICWrite((string)m_strFileTitle);
    BCWrite((string)m_strFileTitle);
    STWrite((string)m_strFileTitle);
    MFPWrite((string)m_strFileTitle);
    //MSPWrite((string)m_strFileTitle); //WW todo
    MMPWrite((string)m_strFileTitle);
    CPWrite((string)m_strFileTitle,0); //SB todo
  }
}

/**************************************************************************
GeoSys-Method: OnFileSaveAs
Task: 
Programing:
11/2003 OK Implementation
01/2005 OK/TK V4
**************************************************************************/
void CGeoSysDoc::OnFileSaveAs()
{
  SetPathName(m_strGSPFilePathBaseExt,TRUE);
  CDocument::OnFileSaveAs();
  CString new_name = GetPathName( );
}

/**************************************************************************
GeoSys-Method: OnFileSave
Task: 
Programing:
01/2005 TK
**************************************************************************/
void CGeoSysDoc::OnFileSave()
{
  SetPathName(m_strGSPFilePathBaseExt,TRUE);
  CDocument::OnFileSave();
}

//###########################################################################
// Import
/**************************************************************************
GeoSys-Method: OnImportSHP
Task: 
Programing:
12/2003 CC Implementation
01/2005 OK GSP: Create GEO
01/2005 CC Modification diaplay shape file direct after importing
**************************************************************************/
void CGeoSysDoc::OnImportSHP()
{
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  m_SHP = true;
  CFileDialog fileDlg(TRUE,"shp",NULL,OFN_ENABLESIZING,"SHAPE Files (*.shp)|*.shp||");
  if (fileDlg.DoModal()==IDOK) {
    // Test
    CString SHPBaseType = fileDlg.GetFileName(); // base_type
    if(SHPBaseType.IsEmpty())
      return; 
    // Determine path plus base name
    CString SHPPathBaseType = fileDlg.GetPathName();
    int pos = SHPPathBaseType.ReverseFind('.');
    if(pos<1)
      return; 
    CString SHPPathBase = SHPPathBaseType.Left(pos);
	//file type cast
	char* shp_name = NULL;
	int datlen = SHPPathBase.GetLength();
    shp_name = (char *) Malloc(datlen);
	strcpy(shp_name,SHPPathBase);
	SHPReadFile(shp_name); //CCToDo: string argument
    //--------------------------------------------------------------------
    // GSP: Create GEO
//CCToDo please check, whether SHP import was sucessful.
    pos = SHPBaseType.ReverseFind('.');
    CString SHPBase = SHPBaseType.Left(pos);
    string geo_base_type = g_gsp_base + ".gli";
    GSPAddMember(geo_base_type);
    //....................................................................
    // Min/Max //OK
    GEOCalcPointMinMaxCoordinates();//CC 10/05
    if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()!= NULL){
      theApp->g_graphics_modeless_dlg->m_dXmin = pnt_x_min;
      theApp->g_graphics_modeless_dlg->m_dXmax = pnt_x_max;
      theApp->g_graphics_modeless_dlg->m_dYmin = pnt_y_min;
      theApp->g_graphics_modeless_dlg->m_dYmax = pnt_y_max;
      // Shrink a bit
      theApp->g_graphics_modeless_dlg->m_dXmin -= 0.05*(pnt_x_max-pnt_x_min);
      theApp->g_graphics_modeless_dlg->m_dXmax += 0.05*(pnt_x_max-pnt_x_min);
      theApp->g_graphics_modeless_dlg->m_dYmin -= 0.05*(pnt_y_max-pnt_y_min);
      theApp->g_graphics_modeless_dlg->m_dYmax += 0.05*(pnt_y_max-pnt_y_min);
      theApp->g_graphics_modeless_dlg->OnInitDialog();
    }
    //....................................................................
    // Open GEOView
    m_frame->OnViewGEOCreate();
    m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
    UpdateAllViews(NULL);
  }
}

/**************************************************************************
GeoSys-Method: OnImportRFO
Task: 
Programing:
06/2004 CC Implementation
**************************************************************************/
void CGeoSysDoc::OnImportRFO()
{
  CFileDialog fileDlg(TRUE, "rfo", NULL, OFN_ENABLESIZING," Output files (*.rfo)|*.rfo|| ");
  if (fileDlg.DoModal()==IDOK) {
    CString file_name = fileDlg.GetFileName();
    CString path_name = fileDlg.GetPathName();
    if(file_name.IsEmpty())
      file_name = "new";
    int pos = path_name.ReverseFind('.');
    if(pos<1)
      m_strGSPFileBase = file_name;
    else
      m_strGSPFileBase = path_name.Left(pos);
    ///OK read RFI
    MSHOpen((string)m_strGSPFileBase);
//OK41    AddMemberToProject(m_strFileNameBase + RF_INPUT_EXTENSION);
    ///OK FEMNodesElements *m = NULL;
	m_nodes_elements = new FEMNodesElements();
	m_nodes_elements->FEMReadRFOFile((string)m_strGSPFileBase);
	///OK delete m;
//OK41    AddMemberToProject(m_strFileNameBase + RF_OUTPUT_EXTENSION);
    m_bDataRFO = true;
  }
}

/**************************************************************************
GeoSys-Method: OnImportBMP
Task: 
Programing:
07/2004 OK Implementation
06/2005 OK Bugfix: m_strGSPFileBase = path_name.Left(pos);
07/2005 OK copy BMP file to project folder
**************************************************************************/
void CGeoSysDoc::OnImportBMP()
{
  CString path_name;
  CString m_strInfo;
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
  //----------------------------------------------------------------------
  // File handling
  CFileDialog fileDlg(TRUE, "bmp", NULL, OFN_ENABLESIZING," BMP graphic files (*.bmp)|*.bmp|| ");
  if (fileDlg.DoModal()==IDOK) {
    CString file_name = fileDlg.GetFileName();
    path_name = fileDlg.GetPathName();
    //
    HBITMAP hBitmap = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(),
                                            path_name, IMAGE_BITMAP, 0, 0,
                                            LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBitmap){
      if (m_bmpBitmap.DeleteObject()) // letztes Bitmap l?chen
        m_bmpBitmap.Detach();         // letztes Bitmap l?en
      // Aktuell geladenes Bitmap mit Bitmap-Objekt verbinden
      m_bmpBitmap.Attach(hBitmap);
      GSPAddMember((string)m_strGSPFileBase + ".bmp"); //OK
    }
    if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()){
      theApp->g_graphics_modeless_dlg->m_bDisplayBMP = TRUE;
      theApp->g_graphics_modeless_dlg->OnInitDialog();
    }
    else{
     AfxMessageBox("no Control Panel active");
     return;
    }
  }
  //----------------------------------------------------------------------
  // Create GEOView
  CMultiDocTemplate *pDocTemplate ;
  CFrameWnd *pWndSecondChild ;
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd()){
    if(theApp->g_graphics_modeless_dlg->m_bDisplayBMP){
      pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_GEO_VIEW);
      if(pDocTemplate!=NULL&&(!mainframe->m_bIsGEOViewOpen)) //OK
	  {
	    pWndSecondChild = pDocTemplate->CreateNewFrame(this, NULL);	
	    ASSERT(pWndSecondChild != NULL);
	    pDocTemplate->InitialUpdateFrame(pWndSecondChild, this);
        mainframe->m_bIsGEOViewOpen = true; //OK
	  }
    }
  }
  //----------------------------------------------------------------------
  // Copy BMP to project folder
  string execute_line;
  string file_orig = (string)path_name;
  string file_copy = (string)m_strGSPFilePathBase + ".bmp";
  execute_line = "copy " + file_orig + " " + file_copy;
  //OK m_strInfo = execute_line.c_str();
  //OK AfxMessageBox(m_strInfo);
  system(execute_line.c_str());
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
10/2004 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnImportEXCEL_PNT()
{
  CFileDialog fileDlg(TRUE,"csv", NULL, OFN_ENABLESIZING," EXCEL files (*.csv)|*.csv|| ");
  if (fileDlg.DoModal()==IDOK) {
    CString file_name = fileDlg.GetFileName();
    CString path_name = fileDlg.GetPathName();
    int pos = path_name.ReverseFind('.');
    CString m_strEXCELFileBase = path_name.Left(pos);
    GEOReadPointProperties((string)m_strEXCELFileBase);//CC
  }
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
03/2005 OK Implementation
04/2005 OK GSPAddMember
**************************************************************************/
void CGeoSysDoc::OnImportEXCEL_FCT()
{
  CGSProject* m_gsp = NULL;
  CFileDialog fileDlg(TRUE,"csv", NULL, OFN_ENABLESIZING," EXCEL files (*.csv)|*.csv|| ");
  if (fileDlg.DoModal()==IDOK) {
    CString file_name = fileDlg.GetFileName();
    CString path_name = fileDlg.GetPathName();
    int pos = path_name.ReverseFind('.');
    CString m_strEXCELFileBase = path_name.Left(pos);
    m_gsp = GSPGetMember("gli");
    if(m_gsp)
      FCTReadTIMData((string)m_strEXCELFileBase);
      string fct_base_type = m_gsp->base + ".fct";
      GSPAddMember(fct_base_type);
      FCTWrite(m_gsp->base);
  }
}

/**************************************************************************
GeoSys-Method: OnExportTecFile
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnExportTecFile()
{
  // File dialog
  CFileDialog fileDlg(FALSE,"tec",NULL,OFN_ENABLESIZING,"TEC Files (*.tec)|*.tec||");
  if (fileDlg.DoModal()==IDOK){
    CString m_strFileNameBaseTec = fileDlg.GetFileTitle();
    //OK 41 MSHWriteTecplot((string)m_strFileNameBaseTec);
  }
}

//###########################################################################
// Simulator

/**************************************************************************
GeoSysGUI-Method:
Task: 
Programing:
01/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnSimulatorCheckStatus()
{  
  m_bReady2Run = false;
  CSimulator m_sim_dlg;
  m_sim_dlg.DoModal();
  m_bReady2Run = m_sim_dlg.m_bSimulatorReady;
}

/**************************************************************************
GeoSys-Method: OnSimulatorForward
Task: 
Programing:
12/2003 OK Implementation
08/2004 OK PCS2 based on ExecuteRFTimeLoop
09/2004 OK/RN Progress bar
12/2008 WW New simulation class
**************************************************************************/
void CGeoSysDoc::OnSimulatorForward()
{
  short no_steps, no_all_steps;
  //WW double dt_sum = 0.0;
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  CStatusBar* pStatus = &m_frame->m_wndStatusBar;
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  //========================================================================
  // Tests
  if((!NODListExists()||!ELEListExists())&&!GSPGetMember("msh")){//OK
    AfxMessageBox("No MSH data !");
    return;
  }
  if(pcs_vector.size()==0) {
    AfxMessageBox("No PCS data ! Create in -> Simulator -> Processes");
    return;
  }
  //........................................................................
  if(!PCSCheck()) //OK
  {
    AfxMessageBox("Incorrect MOD data !");
    return; // TK
  }
  //========================================================================
#ifdef PROBLEM_CLASS    //16.12.2008. WW
  aproblem->Euler_TimeDiscretize();
#else
  // PreTimeLoop
   // Create PCS processes
  //PCSCreate();
 

  //------------------------------------------------------------------------
  // Reactions
  //CB before the first time step
  // 1) set the id variable flow_pcs_type for Saturation and velocity calculation
  //    in mass transport element matrices
  // 2) in case of Twophaseflow calculate NAPL- and the corresponding 
  //    Water phase Saturation from the NAPL concentrations
  if(MASS_TRANSPORT_Process) // if(MASS_TRANSPORT_Process&&NAPL_Dissolution) //CB Todo
  {
    SetFlowProcessType();
    CRFProcess *m_pcs = NULL;
    if (m_pcs = PCSGet("TWO_PHASE_FLOW"))     
      CalcInitialNAPLDens(m_pcs);
  }

  PCSRestart(); //SB
  KRConfig();
  KBlobConfig();
  KBlobCheck();

  //------------------------------------------------------------------------
  // Clock time  
  CreateClockTime();

  //========================================================================
  // Calculate Jakobian und Element-Volumen
  CalcElementsGeometry();
  //========================================================================
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else{
    cout << "Error in ExecuteRFTimeLoop: no time discretization data !" << endl;
  }
  int no_time_steps = (int)m_tim->time_step_vector.size();
  //========================================================================
  // Status bar
  //------------------------------------------------------------------------
  // Progress bar 1: Simulation time
  CProgressCtrl m_ProgressBar;
  RECT MyRect;
  m_frame->m_wndStatusBar.GetItemRect(1,&MyRect);
  MyRect.left = 800;
  int time_step_counter = 0;
  int i;
  m_tim->time_current = m_tim->time_start;
  for(i=0;i<no_time_steps;i++)
  //OK while(m_tim->time_current<m_tim->time_end)
  {
    dt = m_tim->CalcTimeStep();
    m_tim->time_current += dt;
    time_step_counter++;
    m_tim->step_current++;
    if(m_tim->time_current>m_tim->time_end)
      break;
  }
  if(m_ProgressBar.m_hWnd==NULL){
    m_ProgressBar.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
	m_ProgressBar.SetRange(0,(short)time_step_counter-1);
	m_ProgressBar.SetStep(1);
  }
  //------------------------------------------------------------------------
  // Progress bar 2: Simulation step
  CProgressCtrl m_PBSimulationStep;
  RECT PBRect;
  m_frame->m_wndStatusBar.GetItemRect(2,&PBRect);
  PBRect.left = 800;
  no_all_steps = 10000;
  if(m_PBSimulationStep.m_hWnd==NULL){
    m_PBSimulationStep.Create(WS_VISIBLE|PBS_SMOOTH,MyRect,&m_frame->m_wndStatusBar,1);
	m_PBSimulationStep.SetRange(0,no_all_steps);
	m_PBSimulationStep.SetStep(1);
  }
  //======================================================================
  m_tim->step_current = 0;
  m_tim->time_current = m_tim->time_start;
  OUTData(0.,0);
  //
  while(m_tim->time_current < m_tim->time_end) {
    //----------------------------------------------------------------------
    // Time step calculation
    dt = m_tim->CalcTimeStep();
    m_tim->time_current += dt;
    aktueller_zeitschritt++; // ToDo
    aktuelle_zeit = m_tim->time_current; // ToDo
    cout << endl;
    cout << "*********************************************" << endl;
    cout << "TIME step " << m_tim->step_current+1 << ": " << m_tim->time_current << endl;
    //----------------------------------------------------------------------
    // Status bar
    CString m_str;
    if (pStatus) {
      m_str.Format("t=%e",m_tim->time_current);
      pStatus->SetPaneText(1,m_str,1);
    }
//WW -> DM calc secondary variables
  //----------------------------------------------------------------------
  // Draw stress path calculation
  COutput *m_out = NULL;
  CGLPoint *m_pnt = NULL;
  int no_out = (int)out_vector.size();
  int j;
  int no_pcs_names;
  long msh_node;
  double* stressInv;
  stressInv = new double[3];
  for(i=0;i<no_out;i++){
    m_out = out_vector[i];
    no_pcs_names = (int)m_out->nod_value_vector.size();
    for(j=0;j<no_pcs_names;j++){
      if(m_out->nod_value_vector[j].find("LOAD_PATH_ANALYSIS")!=string::npos){
        m_pnt = GEOGetPointByName(m_out->geo_name);//CC
        msh_node = GetNodeNumberClose(m_pnt->x,m_pnt->y,m_pnt->z);
        //OK CalStressInvariants(msh_node,stressInv);
        m_out->out_line_vector.push_back(stressInv);
      }
    }
  }
//
    //----------------------------------------------------------------------
    // Time step excution 
    m_str.Format("Execute time step: t=%e",m_tim->time_current);
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_str);
    //dt_sum += dt;
    LOPTimeLoop_PCS(); //WW (dt_sum);
    IsSynCron();   //WW
    //----------------------------------------------------------------------
    // Data output
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Data output");
    OUTData(m_tim->time_current,aktueller_zeitschritt);
    //......................................................................
    // GUI
    POSITION pos = GetFirstViewPosition();
    while(pos!=NULL) {
      CView* pView = GetNextView(pos);
      pView->UpdateWindow();
    }
	// 04.2008 HS: Update the min and max values------------------------------------
	CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
	CRFProcess* m_pcs = NULL;
	if(pcs_vector.size()==0)
	return;
	m_pcs = PCSGet((string)mainframe->m_pcs_name);
	if(!m_pcs)
	{
	AfxMessageBox("CGSPropertyRightResults::GetPcsMinmax() - no PCS data");
	return;
	}
	double value, m_pcs_min_r, m_pcs_max_r;
	m_pcs_min_r = 1.e+19;
	m_pcs_max_r = -1.e+19;
	int nidx = m_pcs->GetNodeValueIndex((string)mainframe->m_variable_name);
	for(long j=0;j<(long)m_pcs->nod_val_vector.size();j++)
	{
	value = m_pcs->GetNodeValue(j,nidx);
	if(value<m_pcs_min_r) m_pcs_min_r = value;
	if(value>m_pcs_max_r) m_pcs_max_r = value;
	}  
	mainframe->m_pcs_min = m_pcs_min_r;
	mainframe->m_pcs_max = m_pcs_max_r;
	mainframe->m_something_changed = 1;
	//OKmainframe = NULL;
	//OKm_pcs = NULL;
	//OKdelete mainframe, m_pcs;
	// -----------------------------------------------------------------------------
    SetModifiedFlag(1);
    UpdateAllViews(NULL,0L,NULL);
    //......................................................................
    // update current time step number
    m_tim->step_current++;
    if(m_tim->step_current==no_time_steps){
      break;
    }
    //----------------------------------------------------------------------
    no_steps = (short)(dt/m_tim->time_end*no_all_steps);
    m_ProgressBar.StepIt();
    for(i=0;i<no_steps;i++)
    {
      m_PBSimulationStep.StepIt();
    }
  }
  m_ProgressBar.DestroyWindow();
  m_PBSimulationStep.DestroyWindow();
#endif
  //======================================================================
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)"Simulation finished");
}

/**************************************************************************
GeoSys-Method: 
Task: 
Programing:
12/2003 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnUpdateSimulatorForward(CCmdUI *pCmdUI)
{
/*
  if(m_bReady2Run)
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
*/
  pCmdUI->Enable(TRUE);
}

/**************************************************************************
GeoSys-Method: GetProjectFileName
Task: Gibt den Filenamen- Pfad+Name aber ohne Extension - zur?k // bisher nicht verwendet
Programing:
04/2004 TK Implementation
**************************************************************************/
string GetProjectFileName()
{
	int i=0;
    vector<CGSProject*>project_vector = gsp_vector;
	long size = (long)gsp_vector.size();
	CString Test;
	for (i=0; i<size; i++)
	{
	   Test = gsp_vector[i]->ProjectName;
	   Test = gsp_vector[i]->member;
	   Test = gsp_vector[i]->memberextension;
	}
  
    if (i>1)
	{
	CString ProjectName = gsp_vector[0]->ProjectName;
	string projectname = ProjectName;
	return projectname;
	}
	else return "Error";
}
/**************************************************************************
GeoSys-Method: OnImportASC
Task: 
Programing:
02/2005 CC Implementation
04/2005 OK Remove compiler warnings
**************************************************************************/
void CGeoSysDoc::OnImportASC()
{
  FILE *f;
  char *s;  /* gelesene Zeile */
  int i, j;
  int ncols = 0;
  int nrows = 0;
  char charbuff[41];
  double **H=NULL;
  char c_string_pnt[MAX_ZEILE];
  double MinX = 1.0e+10; 
  double MinY = 1.0e+10; 
  double MaxX = -1.0e+10; 
  double MaxY = -1.0e+10; 
  double dx =  1.0e+10;
  double dy =  1.0e+10;
  //------------------
  CFileDialog fileDlg(TRUE,"asc",NULL,OFN_ENABLESIZING,"Raster Files (*.asc)|*.asc||");
  if (fileDlg.DoModal()==IDOK) {

    CString ASCName = fileDlg.GetPathName(); // base_type

      /* Open grid file */
  if ((f = fopen(ASCName,"rb"))==NULL)  {
      DisplayErrorMsg("Fehler: Cannot open .asc file. It may not exist !");
      abort();
  }
  s = (char *) Malloc(MAX_ZEILE);

       ncols = 0;
       nrows = 0;
       
	   double x0, y0, z0;

       fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %ld  ", charbuff, &ncols);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d ", charbuff, &nrows);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &x0);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &y0);

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &dx);
	   dy = dx;

	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &z0);

       MinX = x0;
       MaxX = x0+dx*ncols;
       MinY = y0;
       MaxY = y0+dy*nrows;

       // Allocate memory for grid and the specified surface
       H = new double*[nrows];
	   for(i=0; i<nrows; i++)  H[i] = new double[ncols];
	   for(i=nrows-1; i>=0; i--) 
	   {          
    	  for(j=0; j<ncols; j++)
		  {
             CGLPoint *gl_point = NULL;
             long number_of_points = (long)gli_points_vector.size();//CC
             sprintf(c_string_pnt, "%ld",number_of_points);
             CString m_Pointname = "POINT";
             m_Pointname += c_string_pnt;
  // Create new point
             strcpy(c_string_pnt,m_Pointname);
             gl_point = new CGLPoint();
             gl_point->name = (string)c_string_pnt;
             gl_point->highlighted = false; //CC
             gli_points_vector.push_back(gl_point);
             gl_point->x=double(MinX+dx/2+dx*j);
             gl_point->y=double(MaxY-dy/2-dy*(nrows-i-1));
             gl_point->id=number_of_points;
             fscanf(f,"%lf", &H[i][j]);
             gl_point->z=H[i][j];
		  } 
	   } 
  }
  delete H;
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
bool GMSReadSOL(string file_name)
{
  char buffer[MAX_ZEILE];
  string line;
  std::stringstream in;
  string sub_line;
  long no_vertexes = 0;
  long i;
  double xyz[3]; 
  CFEMesh* m_msh = NULL;
  CNode* m_nod = NULL;
  long no_triangles = 0;
  CElem* m_ele = NULL;
  CString m_strSubLine;
  char a = '\"';
#ifdef MFC
  CString m_strInfo = "Read GMS data: ";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
#endif
  //----------------------------------------------------------------------
#ifdef MFC
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  ifstream sol_file;
  sol_file.open(file_name.c_str());
  if(!sol_file.good()){
    return false;
  }
  sol_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  sol_file.getline(buffer,MAX_ZEILE); // SOLID
  //----------------------------------------------------------------------
  while (!sol_file.eof()) {
    sol_file.getline(buffer,MAX_ZEILE); // BEGS
    line = buffer;
    if(line.find("#STOP")!=string::npos)
      return true;
    //....................................................................
    if(line.find("BEGS")!=string::npos) { // new block
      m_msh = new CFEMesh();
      sol_file.getline(buffer,MAX_ZEILE); // HIDDEN
      sol_file.getline(buffer,MAX_ZEILE); // ID 23589
      sol_file.getline(buffer,MAX_ZEILE); // MAT 1
      in.str(GetLineFromFile1(&sol_file)); // SNAM "material_1 16"
      in >> sub_line;
      in >> sub_line;
      m_strSubLine = sub_line.c_str();
      m_strSubLine.Replace(a,'_');
      m_msh->pcs_name = (string)m_strSubLine;
#ifdef MFC
      m_strInfo = "Read GMS-SOL data: ";
      m_strInfo += m_msh->pcs_name.c_str();
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
      in.clear();
      in.str(GetLineFromFile1(&sol_file)); // VERT 31496
      in >> sub_line;
      in >> no_vertexes;
      in.clear();
      for(i=0;i<no_vertexes;i++){
        m_nod = new CNode(i);
        sol_file>>xyz[0]>>xyz[1]>>xyz[2]>>ws;
        m_nod->SetCoordinates(xyz);     
        m_msh->nod_vector.push_back(m_nod);
      }
      in.str(GetLineFromFile1(&sol_file)); // TRI 62928
      in >> sub_line; 
      //..................................................................
      // Evaluate ele_type
      if(sub_line.compare("TRI")==0){
        m_msh->ele_type = 4;
      }
      //..................................................................
      in >> no_triangles;
      in.clear();
      for(i=0;i<no_triangles;i++){
        m_ele = new CElem();
        m_ele->SetElementType(m_msh->ele_type); //WWOK
        m_ele->Read(sol_file,4);
        m_msh->ele_vector.push_back(m_ele);
      }
      in.str(GetLineFromFile1(&sol_file)); // LAYERS 1 2
      in.clear();
      in.str(GetLineFromFile1(&sol_file)); // TOPCELLBIAS 0 0.800000
      in.clear();
      in.str(GetLineFromFile1(&sol_file)); // MINCELLTHICK 0.1
      in.clear();
      in.str(GetLineFromFile1(&sol_file)); // ENDS
      in.clear();
      m_msh->ConstructGrid(); //WWOK
      fem_msh_vector.push_back(m_msh);
    } // block
    //....................................................................
  } // eof
  //----------------------------------------------------------------------
#ifdef MFC
  m_strInfo = "Reading GMS data finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  return true;
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
bool GMSReadTIN(string file_name)
{
  char buffer[MAX_ZEILE];
  string line;
  std::stringstream in;
  string sub_line;
  CFEMesh* m_msh = NULL;
  CString m_strSubLine;
  ios::pos_type position;
#ifdef MFC
  CString m_strInfo = "Read GMS-TIN data: ";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
#endif
  //----------------------------------------------------------------------
#ifdef MFC
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  ifstream tin_file;
  tin_file.open(file_name.c_str());
  if(!tin_file.good()){
    return false;
  }
  tin_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  tin_file.getline(buffer,MAX_ZEILE); // TIN
  //----------------------------------------------------------------------
  while (!tin_file.eof()) {
    tin_file.getline(buffer,MAX_ZEILE); // BEGT
    line = buffer;
    if(line.find("#STOP")!=string::npos)
      return true;
    //====================================================================
    if(line.find("BEGT")!=string::npos) { // new block
      m_msh = new CFEMesh();
      position = m_msh->GMSReadTIN(&tin_file);
      m_msh->ConstructGrid(); //WWOK
      fem_msh_vector.push_back(m_msh);
    } // block
    //====================================================================
  } // eof
  //----------------------------------------------------------------------
#ifdef MFC
  m_strInfo = "Reading GMS data finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  return true;
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
07/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnImportGMS()
{
  //----------------------------------------------------------------------
  CFileDialog fileDlg(TRUE,"dat",NULL,OFN_ENABLESIZING,
                 "GMS-SOL files (*.sol)|*.sol|GMS-TIN files (*.tin)|*.tin|");
  if (fileDlg.DoModal()==IDOK) {
    CString file_name = fileDlg.GetPathName();
    CString m_strFileExtension = file_name.Right(FILE_EXTENSION_LENGTH);
    if(m_strFileExtension.Compare("sol")==0){
      GMSReadSOL((string)file_name);
    }
    else if(m_strFileExtension.Compare("tin")==0){
      GMSReadTIN((string)file_name);
    }
  }
  CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
  //----------------------------------------------------------------------
  if(theApp->g_graphics_modeless_dlg->GetSafeHwnd())
  {
    theApp->g_graphics_modeless_dlg->OnInitDialog();
  }
  //----------------------------------------------------------------------
  if((int)fem_msh_vector.size()>0)
    GSPAddMember((string)m_strGSPFileBase + ".msh");
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-Method:
Task: 
Programing:
??/2005 TK Implementation
**************************************************************************/
void CGeoSysDoc::OnOpenOGL()
{
      // Create Views for existing data
      CFrameWnd			*OGLChild ;
      CMultiDocTemplate	*pDocTemplate ;
      CGeoSysApp* theApp = (CGeoSysApp*)AfxGetApp();
      //..................................................................
	    pDocTemplate = theApp->GetDocTemplate(DOCTEMPLATE_OGLSPLIT_VIEW);
	    if (pDocTemplate != NULL)
	    {
		  OGLChild = pDocTemplate->CreateNewFrame(this, NULL);	
                    //
          OGLChild->SetWindowPos(&CWnd::wndTopMost,0,0,200,200,SWP_NOOWNERZORDER);
          OGLChild->MoveWindow(0,0,200,200,1);

		  ASSERT(OGLChild  != NULL);
		  pDocTemplate->InitialUpdateFrame(OGLChild , this);
	    }
}

//###########################################################################
// Import
/**************************************************************************
GeoSys-Method: OnImportSHP
Task: 
Programing:
10/2005 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnImportSHPNew()
{
  CDialogSHP m_dlg_shp;
  m_dlg_shp.m_strGSPFilePathBase = m_strGSPFilePathBase;
  m_dlg_shp.m_strGSPFileBase = m_strGSPFileBase;
  m_dlg_shp.DoModal();
  //....................................................................
  // Open GEOView
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  //m_frame->OnViewGEOCreate();
  m_frame->m_rebuild_formtree = 1;//TK - left tree in form view
  UpdateAllViews(NULL);
}

/**************************************************************************
GeoSys-Method: OnImportGHDB
Task: 
Programing:
12/2008 CC Implementation
**************************************************************************/
void CGeoSysDoc::OnImportGHDB()
{
  CDialogDatabase m_dlg_shp;
  m_dlg_shp.m_strGSPFilePathBase = m_strGSPFilePathBase;
  m_dlg_shp.m_strGSPFileBase = m_strGSPFileBase;
  m_dlg_shp.DoModal();
  //....................................................................
  // Open GEOView
  CMainFrame* m_frame = (CMainFrame*)AfxGetMainWnd();
  m_frame->m_rebuild_formtree = 1;
  UpdateAllViews(NULL);
}
/**************************************************************************
GeoSys-Method: RearrangeFLACNodes
Task: The node orders of FLAC3D zones and GS/RF elements are different from
      each other; this method reorders the from FLAC3D imported nodes for
      GS/RF.
Programing:
03/2008 MR Implementation, based on FLACReadMAT
**************************************************************************/
bool RearrangeFLACNodes(CElem* m_ele, string str_e)
{
  vec<long>node_indices(8);
  if (str_e.compare("T4")*str_e.compare("Tetra")==0)      //MR
  {                                                       //MR
    m_ele->SetElementType(5);   // needed only for FLACReadMAT
    m_ele->Config();            // needed only for FLACReadMAT
  }                                                       //MR
  else if (str_e.compare("W6")*str_e.compare("Wedge")==0)
  {
    m_ele->GetNodeIndeces(node_indices);
    m_ele->SetElementType(6);   // needed only for FLACReadMAT
    m_ele->Config();            // needed only for FLACReadMAT
    m_ele->SetNodeIndex(0,node_indices[0]);
    m_ele->SetNodeIndex(1,node_indices[1]);
    m_ele->SetNodeIndex(2,node_indices[3]);
    m_ele->SetNodeIndex(3,node_indices[2]);
    m_ele->SetNodeIndex(4,node_indices[4]);
    m_ele->SetNodeIndex(5,node_indices[5]);
  }
  else if (str_e.compare("B8")*str_e.compare("Brick")==0)
  {
    m_ele->GetNodeIndeces(node_indices);
    m_ele->SetNodeIndex(0,node_indices[0]);
    m_ele->SetNodeIndex(1,node_indices[1]);
    m_ele->SetNodeIndex(2,node_indices[4]);
    m_ele->SetNodeIndex(3,node_indices[2]);
    m_ele->SetNodeIndex(4,node_indices[3]);
    m_ele->SetNodeIndex(5,node_indices[6]);
    m_ele->SetNodeIndex(6,node_indices[7]);
    m_ele->SetNodeIndex(7,node_indices[5]);
  }
  else return false;  // m_ele contains no valid element type name
  return true;
}

/**************************************************************************
GeoSys-Method: ReassignFLACElementNodes
Task: Because of possible gaps in the FLAC3D gridpoint list (due to e.g.
      merged gridpoints) which have been closed in GS/RF during the
      reading process, the corner node numbers of the elements have to be
      reassigned. That's what this method does.
Programing:
07/2008 MR Implementation
**************************************************************************/
bool ReassignFLACElementNodes(CElem* m_ele, vector<long> nodmap)
{
  int i, j = 0;
  j = m_ele->GetElementType();
  if ((j-3)*(j-5)*(j-6) != 0) return false;  // m_ele of invalid geometry type

  vec<long>node_indices(8);
  m_ele->GetNodeIndeces(node_indices);
  j = (4*j*j - 38*j + 102) / 3;   // 3 (hex) -> 8;  5 (tet) -> 4;  6 (pri) -> 6
  for (i=0; i<j; i++) {
    m_ele->SetNodeIndex(i,nodmap[node_indices[i]]);
  }

  return true;
}

/**************************************************************************
GeoSys-Method:
Task:
Programing:
12/2007 OK Implementation
**************************************************************************/
bool FLACReadNOD(string file_name, CFEMesh* m_msh)
{
  //char buffer[MAX_ZEILE];  //MR: obsolete
  string line;
  std::stringstream in;
  string sub_line;
  double x,y,z;
  CNode* m_nod = NULL;
  //CString m_strSubLine;    //MR: obsolete
#ifdef MFC
  CString m_strInfo = "Read FLAC nod data";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
#endif
  //----------------------------------------------------------------------
#ifdef MFC
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  // File handling
  ifstream flac_file;
  flac_file.open(file_name.c_str());
  if(!flac_file.good()){
    return false;
  }
  flac_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file (start reading after dashed line)
  //----------------------------------------------------------------------
  if (!flac_file.eof()) {line = GetLineFromFile1(&flac_file);}        //MR
  while (!flac_file.eof() && line.find("------ -----")==string::npos) //MR
    {line = GetLineFromFile1(&flac_file);}                            //MR
  //----------------------------------------------------------------------
  while (!flac_file.eof())
  {
//  1 (  1.0270e+000,  2.0427e-001, -1.1150e+003)
    line = GetLineFromFile1(&flac_file);
    if(line.find("log off")!=string::npos)
      return true;
    in.str(line);
    in >> sub_line >> sub_line >> x >> sub_line >> y >> sub_line >> z >>ws;
    in.clear();
    m_nod = new CNode((long)m_msh->nod_vector.size(),x,y,z);
    //m_nod->SetCoordinates(xyz);
    m_msh->nod_vector.push_back(m_nod);
    //....................................................................
  } // eof
  //----------------------------------------------------------------------
  return false;
}

/**************************************************************************
GeoSys-Method:
Task:
Programing:
12/2007 OK Implementation
**************************************************************************/
bool FLACReadELE(string file_name, CFEMesh* m_msh)
{
  //char buffer[MAX_ZEILE];  //MR: obsolete
  string line;
  std::stringstream in;
  string sub_line;
  long i;
  CElem* m_ele = NULL;
  //CString m_strSubLine;    //MR: obsolete
#ifdef MFC
  CString m_strInfo = "Read FLAC data: ";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
#endif
  //----------------------------------------------------------------------
#ifdef MFC
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  // File handling
  ifstream flac_file;
  flac_file.open(file_name.c_str());
  if(!flac_file.good()){
    return false;
  }
  flac_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file (start reading after dashed line)
  //----------------------------------------------------------------------
  if (!flac_file.eof()) {line = GetLineFromFile1(&flac_file);}        //MR
  while (!flac_file.eof() && line.find("------ -----")==string::npos) //MR
    {line = GetLineFromFile1(&flac_file);}                            //MR
  //----------------------------------------------------------------------
//CElem::Read(istream& is, int fileType)
  while (!flac_file.eof())
  {
//      1      1      2      3      4      5      6      4      6
    line = GetLineFromFile1(&flac_file);
    if(line.find("log off")!=string::npos)
      return true;
    in.str(line);
    in >> i;
    m_ele = new CElem(i-1);
    m_ele->Read(in,5);
    in.clear();
    m_msh->ele_vector.push_back(m_ele);
    //....................................................................
  } // eof
  //----------------------------------------------------------------------
  return false;
}
/**************************************************************************
GeoSys-Method:
01/2008 WW Implemented bases on FLACReadELE(string file_name, CFEMesh* m_msh)
02/2008 OK Element types
**************************************************************************/
bool FLACReadMAT(string file_name, CFEMesh* m_msh)
{
  //char buffer[MAX_ZEILE];    //MR: obsolete
  string line;
  std::stringstream in;
  string sub_line, str_e;
  long i;
  //CString m_strSubLine;      //MR: obsolete
  CElem* m_ele = NULL;
  //vec<CNode*>e_nodes0(8);    //MR: obsolete
  //vec<CNode*>e_nodes1(8);    //MR: obsolete
  //vec<long>node_indeces(8);  //MR: obsolete
#ifdef MFC
  CString m_strInfo = "Read FLAC data: ";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
#endif
  //----------------------------------------------------------------------
#ifdef MFC
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  // File handling
  ifstream flac_file;
  flac_file.open(file_name.c_str());
  if(!flac_file.good()){
    return false;
  }
  flac_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file (start reading after dashed line)
  //----------------------------------------------------------------------
  if (!flac_file.eof()) {line = GetLineFromFile1(&flac_file);}        //MR
  while (!flac_file.eof() && line.find("------ -----")==string::npos) //MR
    {line = GetLineFromFile1(&flac_file);}                            //MR
  //----------------------------------------------------------------------
  vector<string> orig_mat;
  int imat;
  //int ii = 0, imat=-1;   //MR: obsolete
  //bool done = false;     //MR: obsolete
  while (!flac_file.eof())
  {
    line = GetLineFromFile1(&flac_file);
    if(line.find("log off")!=string::npos)
      return true;
    in.str(line);
    //....................................................................
    in >> i;
    m_ele = m_msh->ele_vector[i-1];
    in >> str_e; // Type
    //....................................................................
    // ELE type
    RearrangeFLACNodes(m_ele,str_e);  //MR
    /*
    if(str_e.compare("Wedge")==0)
    {
      m_ele->GetNodeIndeces(node_indeces);
      m_ele->SetElementType(6);
      m_ele->Config();
      m_ele->SetNodeIndex(0,node_indeces[0]);
      m_ele->SetNodeIndex(1,node_indeces[1]);
      m_ele->SetNodeIndex(2,node_indeces[3]);
      m_ele->SetNodeIndex(3,node_indeces[2]);
      m_ele->SetNodeIndex(4,node_indeces[4]);
      m_ele->SetNodeIndex(5,node_indeces[5]);
    }
    if(str_e.compare("Brick")==0)
    {
      m_ele->GetNodeIndeces(node_indeces);
      m_ele->SetNodeIndex(0,node_indeces[0]);
      m_ele->SetNodeIndex(1,node_indeces[1]);
      m_ele->SetNodeIndex(2,node_indeces[4]);
      m_ele->SetNodeIndex(3,node_indeces[2]);
      m_ele->SetNodeIndex(4,node_indeces[3]);
      m_ele->SetNodeIndex(5,node_indeces[6]);
      m_ele->SetNodeIndex(6,node_indeces[7]);
      m_ele->SetNodeIndex(7,node_indeces[5]);
    }
    */
    //....................................................................
    // MAT type
    in >> str_e; // Model
    in >> str_e; // Group

    /*
    done = false;
    for(ii=0; ii<(int)orig_mat.size(); ii++)
    {
      if(orig_mat[ii].find(str_e)!=string::npos)
      {
         imat = ii;
         done = true;
         break;
      }
    }
    if(!done)
    {
       imat++;
       orig_mat.push_back(str_e);
    }
    in.clear();
    m_msh->ele_vector[i-1]->SetPatchIndex(imat);
    */

    /* 
       Fehler:
       - sieben verschiedene Materialien (0 bis 6) im Vektor orig_mat
       - nächstes Element ist von Material 3, also imat = 3
       - nächstes Element ist neu, aber imat = 4 statt 7!

       //MR: korrigiert und etwas knapper formuliert:
    */

    //--------------------------------------------------------------------
    // search vector of previously found material groups for str_e:
    imat = 0;
    while (imat < (int)orig_mat.size())
    {
      if (orig_mat[imat].find(str_e)!=string::npos)
        break;      // index of str_e found: => lock index
      imat++;       //            otherwise: => increase it
    }
    if (imat >= (int)orig_mat.size())  // i.e. str_e not in vector
      orig_mat.push_back(str_e);
    in.clear();
    m_msh->ele_vector[i-1]->SetPatchIndex(imat);
    //....................................................................
  } // eof
  //----------------------------------------------------------------------
  return false;
}

/**************************************************************************
GeoSys-Method: OnImportFLAC
12/2007 OK Implementation
01/2008 WW Finishing
**************************************************************************/
void CGeoSysDoc::OnImportFLAC()
{
  CFEMesh* m_msh = NULL;
  m_msh = new CFEMesh();
  CString file_name;
  CString m_strFileExtension;
  // Clean MSH // WW
  for(long i=0; i<(long)fem_msh_vector.size(); i++ )
  {
    delete fem_msh_vector[i];
    fem_msh_vector[i] = NULL;
  }
  fem_msh_vector.clear();
  //----------------------------------------------------------------------
  // NOD data
  AfxMessageBox("Read FLAC Geometry File");
  CFileDialog fileDlg1(TRUE,"txt",NULL,OFN_ENABLESIZING,"FLAC NOD files (*.txt)|*.txt|");
  if(fileDlg1.DoModal()==IDOK)
  {
    file_name = fileDlg1.GetPathName();
    m_strFileExtension = file_name.Right(FILE_EXTENSION_LENGTH);
    FLACReadNOD((string)file_name,m_msh);
    m_msh->InitialNodesNumber(); // Only for output. WW
  }
  //----------------------------------------------------------------------
  // ELE data
  AfxMessageBox("Read FLAC Element File");
  CFileDialog fileDlg2(TRUE,"txt",NULL,OFN_ENABLESIZING,"FLAC ELE files (*.txt)|*.txt|");
  if(fileDlg2.DoModal()==IDOK)
  {
    file_name = fileDlg2.GetPathName();
    m_strFileExtension = file_name.Right(FILE_EXTENSION_LENGTH);
    FLACReadELE((string)file_name,m_msh);
  }
  //----------------------------------------------------------------------
  // MAT data
  AfxMessageBox("Read FLAC Material Group File");
  CFileDialog fileDlg3(TRUE,"txt",NULL,OFN_ENABLESIZING,"FLAC MAT files (*.txt)|*.txt|");
  if(fileDlg3.DoModal()==IDOK)
  {
    file_name = fileDlg3.GetPathName();  //WW
    m_strFileExtension = file_name.Right(FILE_EXTENSION_LENGTH);
    FLACReadMAT((string)file_name,m_msh);
  }
  //----------------------------------------------------------------------
  //m_msh->ConstructGrid();
  fem_msh_vector.push_back(m_msh);
  //----------------------------------------------------------------------
  if((int)fem_msh_vector.size()>0)
    GSPAddMember((string)m_strGSPFileBase + ".msh");
  //----------------------------------------------------------------------
  // Write TEC
  MSHWriteTecplot();
  //----------------------------------------------------------------------
}

/**************************************************************************
GeoSys-Method: OnImportFLACMesh
Task: import a single .flac3d file instead of three separate files for
      NOD, ELE and MAT data
Programing:
03/2008 MR Implementation, based on OnImportFLAC and FLACReadXXX
**************************************************************************/
void CGeoSysDoc::OnImportFLACMesh()
{
  CFEMesh* m_msh = NULL;
  CString m_file_name;
  string line, sub_line, str_e;
  std::stringstream in, in2;
  double x,y,z;
  int i = 0;  // value irrelevant
  long j, k = 0, map_index = 0;
  bool split_ele;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  CElem* m_ele2 = NULL;
  vector <string> orig_mat;
  vector <long> nodmap, elemap;  // maps FLAC gridpoint/zone no. to (start of) RF NOD/ELE no.
#ifdef MFC
  CString m_strInfo = "Read FLAC mesh data";
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
#endif
  //----------------------------------------------------------------------
  // Clean mesh vector and create a new mesh
  //----------------------------------------------------------------------
  for(j=0; j<(long)fem_msh_vector.size(); j++)
  {
    delete fem_msh_vector[j];
    fem_msh_vector[j] = NULL;
  }
  fem_msh_vector.clear();
  m_msh = new CFEMesh();
  //----------------------------------------------------------------------
  // Get filename via user interface, open file and get ready for reading
  //----------------------------------------------------------------------
  CFileDialog fileDlg1(TRUE,"txt",NULL,OFN_ENABLESIZING,"FLAC mesh data file (*.Flac3D)|*.flac3d|");
  if(fileDlg1.DoModal()==IDOK)
    m_file_name = fileDlg1.GetPathName();
  ifstream flac_file;
  string file_name = (string)m_file_name;
  flac_file.open(file_name.c_str());
  if(!flac_file.good()){
    // Abbruchmeldung einbauen!
    return;
  }
  flac_file.seekg(0L,ios::beg);  // spool to begin
  if (!flac_file.eof()) {line = GetLineFromFile1(&flac_file);}
  while (!flac_file.eof() && line.find("* GRIDPOINTS")==string::npos) {
    line = GetLineFromFile1(&flac_file);
  }
  //----------------------------------------------------------------------
  // Read NOD data
  //----------------------------------------------------------------------
  while (!flac_file.eof())
  {
    line = GetLineFromFile1(&flac_file);
    if (line.find("* ZONES")!=string::npos)
      break;
    in.str(line);
    in >> sub_line >> k >> x >> y >> z;
    in.clear();
    for (; map_index<k-1; map_index++)  // not in FLAC defined (e.g.
      nodmap.push_back(-1);             // merged) gridpoints map to -1

    j = (long)m_msh->nod_vector.size();
    if (map_index == k-1) {
      nodmap.push_back(j);  // FLAC gridpoint no. (= map_index) maps to
      map_index++;          // RF node no. j (= new index of nod_vector)
    }
    m_nod = new CNode(j,x,y,z);
    //m_nod->SetCoordinates(xyz);
    m_msh->nod_vector.push_back(m_nod);
  }
  m_msh->InitialNodesNumber();  // Only for output. WW
  //----------------------------------------------------------------------
  // Read ELE data
  //----------------------------------------------------------------------
  map_index = 0;
  while (!flac_file.eof())
  {
    line = GetLineFromFile1(&flac_file);
    if (line.find("* GROUPS")!=string::npos)
      break;
    in.str(line);
    in >> sub_line >> str_e >> k;
    for (; map_index<k-1; map_index++)  // not in FLAC defined (e.g.
      elemap.push_back(-1);             // excavated) zones map to -1
    j = (long)m_msh->ele_vector.size();
    if (map_index == k-1) {
      elemap.push_back(j);  // FLAC zone no. (= map_index) maps to
      map_index++;          // RF element no. j (= new index of ele_vector)
    }
    m_ele = new CElem(j);
    split_ele = false;
    if (str_e.compare("B8")==0)         // Brick ~> geo_type = 3 (hex)
      i = 53;
    else if (str_e.compare("T4")==0)    // Tetrahedron ~> geo_type = 5 (tet)
      i = 55;
    else if (str_e.compare("W6")==0)    // Wedge ~> geo_type = 6 (pri)
      i = 56;
    else if (str_e.compare("P5")==0) {  // Pyramid ~> geo_type = 5 (tet)
      split_ele = true; str_e = "T4";   //    (split into two Tetrahedrons)
      i = 55;
    }
    else
      i = 5;
    m_ele->Read(in,i);
    in.clear();
    RearrangeFLACNodes(m_ele,str_e);          // corner nodes of an element are numbered in different ways here & there
    ReassignFLACElementNodes(m_ele,nodmap);   // nodes might have been renumbered while being read (gaps due to merged nodes in FLAC)
    m_msh->ele_vector.push_back(m_ele);
    if (split_ele) {   // add a second tetrahedral element
      elemap.push_back(j+2);
      map_index++;   // (value will once again be used below, don't change until then!)
      in2.str(line);
      in2 >> sub_line >> sub_line >> sub_line >> sub_line;
      m_ele2 = new CElem(j+1);
      m_ele2->Read(in2,i);
      in2.clear();
      RearrangeFLACNodes(m_ele2,str_e);
      m_msh->ele_vector.push_back(m_ele2);
    }
  }
  //----------------------------------------------------------------------
  // Read MAT data
  //----------------------------------------------------------------------
  if (!flac_file.eof()) {line = GetLineFromFile1(&flac_file);}
  while (!flac_file.eof() && line.find("ZGROUP")==string::npos) {line = GetLineFromFile1(&flac_file);}
  while (true)
  {
    in.str(line);
    while (line.find("ZGROUP")!=string::npos) {  // material group keyword
      in >> sub_line >> str_e;
      i = 0;
      while (i < (int)orig_mat.size())
      {
        if (orig_mat[i].find(str_e)!=string::npos)
          break;      // index of str_e found: => lock index
        i++;          //            otherwise: => increase it
      }
      if (i >= (int)orig_mat.size())  // i.e. str_e not in vector
        orig_mat.push_back(str_e);
      // material group index determined, now go on with next line
      line = GetLineFromFile1(&flac_file);
      in.clear();
      in.str(line);
    }
    //while (in.gcount() > 0)
    j = 1;
    while (j > 0)  // as long as there are element IDs left
    {
      j = 0;
      in >> j;
      if (j > 0) {
        m_msh->ele_vector[elemap[j-1]]->SetPatchIndex(i);
        // if FLAC zone has been splitted, also set material group index
        // of the next RF element to i:
        if (map_index>j && elemap[j]-elemap[j-1]==2)
          m_msh->ele_vector[elemap[j]-1]->SetPatchIndex(i);
      }
    }
    in.clear();
    if (flac_file.eof())
      break;
    else
      line = GetLineFromFile1(&flac_file);
  }
  //----------------------------------------------------------------------
  // Store mesh
  //----------------------------------------------------------------------
  //m_msh->ConstructGrid();
  fem_msh_vector.push_back(m_msh);
  if((int)fem_msh_vector.size()>0)
    GSPAddMember((string)m_strGSPFileBase + ".msh");
  MSHWriteTecplot();
}
void CGeoSysDoc::OnGhdbviewCreate()
{
	// TODO: Add your command handler code here
	CDialogGHDB_Connect* GHDBdiag = new CDialogGHDB_Connect();
  
  if (GHDBdiag->DoModal()==IDOK) {
  }
}

/**************************************************************************
GeoSys-GUI-Method: OnImportFEFLOW
10/2008 OK Implementation
**************************************************************************/
void CGeoSysDoc::OnImportFEFLOW()
{
  // Message to status bar
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_strInfo;
  // Message to status bar
  m_strInfo = "Import FEFLOW data - start";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
  // Dialog
  CDialogFEFLOW m_dlg; // creating an instance of CDialogFEFLOW
  m_dlg.DoModal(); // showing the dialog
  // Message to status bar
  m_strInfo = "Import FEFLOW data - finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
}
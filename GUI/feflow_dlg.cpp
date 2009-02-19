// feflow_dlg.cpp : implementation file
//
#include "stdafx.h"
#include "GeoSys.h"
#include "feflow_dlg.h"
#include "afxpriv.h" // For WM_SETMESSAGESTRING

extern string GetLineFromFile1(ifstream*);
bool FEFLOWReadBoundaryConditions(string);
bool FEFLOWReadNodes(string,CFEMesh*);
bool FEFLOWReadElements(string,CFEMesh*);
bool FEFLOWReadSourceSinkTerms(string);
bool FEFLOWReadNodesFEM(string,CFEMesh*);
// CDialogFEFLOW dialog

IMPLEMENT_DYNAMIC(CDialogFEFLOW, CDialog)

CDialogFEFLOW::CDialogFEFLOW(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogFEFLOW::IDD, pParent)
{
}

CDialogFEFLOW::~CDialogFEFLOW()
{
}

void CDialogFEFLOW::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDialogFEFLOW, CDialog)
    ON_BN_CLICKED(IDC_FILE_FEFLOW_READ_NOD, &CDialogFEFLOW::OnBnClickedReadNodes)
    ON_BN_CLICKED(IDC_FILE_FEFLOW_READ_ELE, &CDialogFEFLOW::OnBnClickedReadElements)
    ON_BN_CLICKED(IDC_FILE_FEFLOW_READ_BC, &CDialogFEFLOW::OnBnClickedReadBoundaryConditions)
    ON_BN_CLICKED(IDC_FILE_FEFLOW_READ_ST, &CDialogFEFLOW::OnBnClickedReadSourceSinkTerms)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_NOD, &CDialogFEFLOW::OnBnClickedCreateNOD)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_ELE, &CDialogFEFLOW::OnBnClickedCreateELE)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_BC, &CDialogFEFLOW::OnBnClickedCreateBC)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_ST, &CDialogFEFLOW::OnBnClickedCreateST)
END_MESSAGE_MAP()

/**************************************************************************
GeoSys-GUI-Method:
10/2008 OK Implementation
**************************************************************************/
BOOL CDialogFEFLOW::OnInitDialog() 
{
  CDialog::OnInitDialog();
  // de/active elements
  if(fem_msh_vector.size()>0)
  {
    GetDlgItem(IDC_FILE_FEFLOW_READ_NOD)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CREATE_NOD)->EnableWindow(FALSE);
    GetDlgItem(IDC_FILE_FEFLOW_READ_ELE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_CREATE_ELE)->EnableWindow(FALSE);
  }
  GetDlgItem(IDC_BUTTON_CREATE_BC)->EnableWindow(FALSE);
  GetDlgItem(IDC_BUTTON_CREATE_ST)->EnableWindow(FALSE);

  UpdateData(FALSE); // noetig, wenn Eingabefelder nach OnInitDialog() initialisiert werden
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

// CDialogFEFLOW message handlers


/**************************************************************************
GUI-Method:
10/2008 OK Implementation
02/2009 OK FEM files
**************************************************************************/
void CDialogFEFLOW::OnBnClickedReadNodes()
{
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_strInfo;
  // Homeworks#1(MK): copy the functions from MK to finish the node data import
  // MSH create
  m_msh = new CFEMesh();
  m_msh->nod_vector.clear();
  // FEFLOW node files
  CFileDialog fileDlg(TRUE,"fem",NULL,OFN_ENABLESIZING,"FEM files (*.fem)|*.fem| DAT files (*.dat)|*.dat|");
  if (fileDlg.DoModal()==IDOK) 
  {
    CString file_name = fileDlg.GetPathName();
    CString m_strFileExtension = file_name.Right(3);
    if(m_strFileExtension.Compare("dat")==0)
      FEFLOWReadNodes((string)file_name,m_msh);  
    else if (m_strFileExtension.Compare("fem")==0)
      FEFLOWReadNodesFEM((string)file_name,m_msh);  
  }
/*
  // test output of NOD
  fstream msh_file;
  msh_file.open("nodes.msh",ios::trunc|ios::out);
  if(!msh_file.good()) return;
  msh_file.setf(ios::scientific,ios::floatfield);
  msh_file.precision(12);
  m_msh->InitialNodesNumber();
  m_strInfo = "Import FEFLOW data: write nodes start";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
  m_msh->Write(&msh_file);
  msh_file << "#STOP" << endl;
  msh_file.close();
  m_strInfo = "Import FEFLOW data: write nodes finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
*/
  //......................................................................
  // m_msh->ConstructGrid();
  fem_msh_vector.push_back(m_msh);
  //----------------------------------------------------------------------
/*
  MSHCalcMinMaxMidCoordinates();
  CGLPoint *m_pnt = NULL;
  m_pnt = new CGLPoint();
  m_pnt->id = (long)gli_points_vector.size();
  m_pnt->x = msh_x_min;
  m_pnt->y = msh_y_min;
  gli_points_vector.push_back(m_pnt); 
  m_pnt = new CGLPoint();
  m_pnt->id = (long)gli_points_vector.size();
  m_pnt->x = msh_x_min;
  m_pnt->y = msh_y_max;
  gli_points_vector.push_back(m_pnt);
  m_pnt = new CGLPoint();
  m_pnt->id = (long)gli_points_vector.size();
  m_pnt->x = msh_x_max;
  m_pnt->y = msh_y_max;
  gli_points_vector.push_back(m_pnt); 
  m_pnt = new CGLPoint();
  m_pnt->id = (long)gli_points_vector.size();
  m_pnt->x = msh_x_max;
  m_pnt->y = msh_y_min;
  gli_points_vector.push_back(m_pnt);
*/
  GEOWrite("feflow");
}

void CDialogFEFLOW::OnBnClickedReadElements()
{
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_strInfo;
  // Homeworks#2(MK): copy the functions from MK to finish the element data import
  // test
  if(!m_msh)
  {
    AfxMessageBox("Error: no MSH data: Please import node data first");
    return;
  }
  // FEFLOW mesh file
  CFileDialog fileDlg(TRUE,"fem",NULL,OFN_ENABLESIZING,"FEFLOW files (*.´fem)|*.fem|");
  if(fileDlg.DoModal()==IDOK) 
  {
    CString file_name = fileDlg.GetPathName();
    CString m_strFileExtension = file_name.Right(3);
    m_strInfo = "Import FEFLOW data: elements start";
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
    FEFLOWReadElements((string)file_name,m_msh);
    m_strInfo = "Import FEFLOW data: elements finished";
    pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
  }
  // if succesfull
  //fem_msh_vector.push_back(m_msh);
  // test output
  fstream msh_file;
  msh_file.open("elements.msh",ios::trunc|ios::out);
  if(!msh_file.good()) return;
  msh_file.setf(ios::scientific,ios::floatfield);
  msh_file.precision(12);
  m_strInfo = "Import FEFLOW data: write elements start";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
  m_msh->InitialNodesNumber();
  m_msh->Write(&msh_file);
  msh_file << "#STOP";
  msh_file.close();
  m_strInfo = "Import FEFLOW data: write elements finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
}

void CDialogFEFLOW::OnBnClickedReadBoundaryConditions()
{
  // Homeworks#3(OK):
  CFileDialog fileDlg(TRUE,"dat",NULL,OFN_ENABLESIZING,"FEFLOW files (*.dat)|*.dat|");
  if (fileDlg.DoModal()==IDOK) 
  {
    CString file_name = fileDlg.GetPathName();
    CString m_strFileExtension = file_name.Right(3);
    FEFLOWReadBoundaryConditions((string)file_name);
    GetDlgItem(IDC_BUTTON_CREATE_BC)->EnableWindow(TRUE); // activate element
  }
  GetDlgItem(IDC_FILE_FEFLOW_READ_BC)->EnableWindow(FALSE); // deactivate element
}

void CDialogFEFLOW::OnBnClickedReadSourceSinkTerms()
{
  CFileDialog fileDlg(TRUE,"dat",NULL,OFN_ENABLESIZING,"FEFLOW files (*.dat)|*.dat|");
  if (fileDlg.DoModal()==IDOK) 
  {
    CString file_name = fileDlg.GetPathName();
    CString m_strFileExtension = file_name.Right(3);
    FEFLOWReadSourceSinkTerms((string)file_name);
    GetDlgItem(IDC_BUTTON_CREATE_ST)->EnableWindow(TRUE); // activate element
  }
  GetDlgItem(IDC_FILE_FEFLOW_READ_ST)->EnableWindow(FALSE); // deactivate element
}

void CDialogFEFLOW::OnBnClickedCreateNOD()
{
  // Homeworks#4(MK):
  // m_msh->ConstructGrid();
}

void CDialogFEFLOW::OnBnClickedCreateELE()
{
  // Homeworks#5(MK):
  // fem_msh_vector.push_back(m_msh);
}

/**************************************************************************
GeoSys-Method: Read from FEFLOW DAT files
10/2008 MK/JOD Implementation
10/2008 OK 2nd Implementation
**************************************************************************/
bool FEFLOWReadNodes(string file_name, CFEMesh* m_msh)
{
  char buffer[MAX_ZEILE];
  string line_string;
  std::stringstream line_stream;
  double xyz[3]; 
  CNode* m_nod = NULL;
  //CString m_strSubLine;
  int idummy;
  long ldummy;
  long i = 0;
  //----------------------------------------------------------------------
  // File handling
  ifstream feflow_file;
  feflow_file.open(file_name.c_str());
  if(!feflow_file.good())
  {
    return false;
  }
  feflow_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  //    NODE  SLICE                   X                   Y               Z                   F
  feflow_file.getline(buffer,MAX_ZEILE); 
  //m_strSubLine = buffer; 
  //while(!m_strSubLine.IsEmpty()) 
  while(!feflow_file.eof()) 
  {
    //  1      1   217347.2919203297   658768.5953254319    481.42573174        0.0121619608
    line_string = GetLineFromFile1(&feflow_file);
    line_stream.str(line_string);
    if(line_string.size()<1) break;
    line_stream >> idummy >> ldummy >> xyz[0] >> xyz[1] >> xyz[2] >> ws;
    m_nod = new CNode(i);
    m_nod->SetCoordinates(xyz);    
    m_nod->SetIndex(i); 
    i +=1;
    m_msh->nod_vector.push_back(m_nod);
    line_stream.clear();
  } // is line empty
  feflow_file.close();
  //----------------------------------------------------------------------
  return true;
}

/**************************************************************************
GeoSys-Method:
10/2008 MK/JOD Implementation
10/2008 OK 2nd Implementation
**************************************************************************/
bool FEFLOWReadElements(string file_name,CFEMesh* m_msh) 
{
  char buffer[MAX_ZEILE];
  string line_string;
  std::stringstream line_stream;
  long i;
  CElem* m_ele = NULL;
  CString m_strSubLine;
  long no_nodes,no_elements=0;
  //long no_elements_per_layer=0;
  int idummy, no_layer=0;
  //----------------------------------------------------------------------
  // File handling
  ifstream feflow_file;
  feflow_file.open(file_name.c_str());
  if(!feflow_file.good())
  {
    return false;
  }
  feflow_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  feflow_file.getline(buffer,MAX_ZEILE); // SOLID
  //getline(*feflow_file,line);
  //----------------------------------------------------------------------
  while(!feflow_file.eof()) 
  {
    feflow_file.getline(buffer,MAX_ZEILE);
    line_string = buffer;
    //....................................................................
    // CLASS
    if(line_string.find("CLASS")!=string::npos) 
    {
      // 8    1    0    3   27    0    8    8    1
      line_stream.str(GetLineFromFile1(&feflow_file));
      line_stream >> idummy >> idummy >> idummy >> idummy >> no_layer;
	  line_stream.clear();
    }
    //....................................................................
    if(line_string.find("DIMENS")!=string::npos) 
    {
      // DIMENS
      // 39465  61808      6      1   2000      0      0      0      0     -1      0      0      1      0      0      0      0
      line_stream.str(GetLineFromFile1(&feflow_file));
      line_stream >> no_nodes >>no_elements >> m_msh->ele_type >> ws;
	  line_stream.clear();
    }
    //....................................................................
    // SCALE
    // 4.950495e+001,5.000000e+004,1.000000e+000,1.000000e+002,1.522132e+004,1.658530e+004
    if(line_string.find("NODE")!=string::npos) 
    {
      // NODE
      for(i=0;i<no_elements;i++) 
      {
        m_ele = new CElem();
        m_ele->SetElementType(m_msh->ele_type);
        //feflow_file.getline(buffer,MAX_ZEILE);
        //line_string = buffer;
        //line_stream.str(line_string);
        //line_stream.str(GetLineFromFile1(&feflow_file));
        //m_ele->Read(line_stream,m_msh->ele_type);
        m_ele->Read(feflow_file,m_msh->ele_type);
        m_msh->ele_vector.push_back(m_ele);
        //delete [] m_ele;
      }
      return true;
    } 
  } // eof
  return true;
}

/**************************************************************************
FEFLOW-Method: 
10/2008 OK Implementation
**************************************************************************/
bool FEFLOWReadBoundaryConditions(string file_name)
{
  //char buffer[MAX_ZEILE];
  string line;
  std::stringstream in;
  int i,tim_type;
  long l;
  double x,y,z,f;
  string bc_type;
  CGLPoint* m_pnt = NULL;
  //----------------------------------------------------------------------
  // File handling
  ofstream out_file;
  //out_file.open("out_test.txt");
  ifstream file;
  file.open(file_name.c_str());
  if(!file.good())
  {
    return false;
  }
  file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  //NODE  SLICE                   X                   Y               Z                   F       BCKIND STEADY
  line = GetLineFromFile1(&file);
  while(!file.eof()) 
  {
    //file.getline(buffer,MAX_ZEILE); 
    // 836      1   235554.6425086915   638255.2217882699     13.34303538        0.0000000000         WELL 1 
    line = GetLineFromFile1(&file);
    if(line.length()<1) continue;
    in.str(line);
    //....................................................................
    in >> l >> i >> x >> y >> z >> f >> bc_type >> tim_type;
    //out_file << l << " " << i << " " << x << " " << y << " " << z << " " << f << " " << bc_type << " " << tim_type << endl;
    //....................................................................
    // Create PNT
    //....................................................................
    m_pnt = new CGLPoint();
    m_pnt->id = (long)gli_points_vector.size();
    m_pnt->node = l;
    m_pnt->x = x;
    m_pnt->y = y;
    m_pnt->z = z;
    m_pnt->type = 0; // GEO
    if(bc_type.compare("DIRICHLET")==0) m_pnt->type = 1; // BC
    m_pnt->value = f;
    gli_points_vector.push_back(m_pnt); 
    //....................................................................
    in.clear();
  }
  file.close();
  out_file.close();
  GEOWrite("bc_pnt");
  return true;
}

//    if(line.find("log off")!=string::npos) 
//      return true;

/**************************************************************************
GUI-Method: 
10/2008 OK Implementation
**************************************************************************/
void CDialogFEFLOW::OnBnClickedCreateBC()
{
  // Homeworks#6(OK):
  CGLPoint* m_pnt = NULL;
  //----------------------------------------------------------------------
  // Create BC from PNT: see BCRead
  //----------------------------------------------------------------------
  CBoundaryCondition *m_bc = NULL;
  for(long l=0;l<(long)gli_points_vector.size();l++)
  {
    m_pnt =  gli_points_vector[l];
    if(m_pnt->type==1)
    {
      m_bc = new CBoundaryCondition();
      m_bc->pcs_type_name = "GROUNDWATER_FLOW";
      m_bc->pcs_pv_name = "HEAD";
      m_bc->geo_type_name = "POINT";
      m_bc->geo_name = m_pnt->name; //?->CC
      //m_bc->msh_type_name = "NODE";
      //m_bc->msh_node_number = m_pnt->node;
      //m_bc->dis_type_name = m_bc->msh_type_name; // redundant
      m_bc->dis_type_name = "CONSTANT";
      m_bc->geo_node_value = m_pnt->value;
      bc_list.push_back(m_bc);
    }
    m_pnt->type=0; // reset (repeated use)
  }
  //----------------------------------------------------------------------
  // Create PCS->bc_node from BC: see CRFProcess::Create()
  //----------------------------------------------------------------------
  //CBoundaryConditions::OnBnClickedButtonCreateGroup()
  CRFProcess* m_pcs = PCSGet(m_bc->pcs_type_name);
  if(!m_pcs) 
  {
    AfxMessageBox("No PCS data");
    return;
  }
  //else BCGroupDelete(m_bc->pcs_type_name,(string)m_strPVName);
  CBoundaryConditionNode *m_node_value = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) 
  {
    m_bc = *p_bc;
    if(m_bc->msh_type_name.compare("NODE")==0) 
    {
      m_node_value = new CBoundaryConditionNode;
      m_node_value->geo_node_number = m_bc->msh_node_number;
      m_node_value->msh_node_number = m_bc->msh_node_number; //OK redundant
      m_node_value->node_value = m_bc->geo_node_value;
      m_node_value->pcs_pv_name = m_bc->pcs_type_name; 
      m_pcs->bc_node.push_back(m_bc);
      m_pcs->bc_node_value.push_back(m_node_value);
    }
    ++p_bc;
  } // list
  // test
  m_pcs->WriteBC();
  // de/activate elements
  GetDlgItem(IDC_FILE_FEFLOW_READ_BC)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_CREATE_BC)->EnableWindow(FALSE);
}

/**************************************************************************
FEFLOW-Method: 
10/2008 OK Implementation
**************************************************************************/
bool FEFLOWReadSourceSinkTerms(string file_name)
{
  string line;
  std::stringstream in;
  int i,tim_type;
  long l;
  double x,y,z,f;
  string bc_type;
  CGLPoint* m_pnt = NULL;
  //----------------------------------------------------------------------
  // File handling
  ifstream file;
  file.open(file_name.c_str());
  if(!file.good())
  {
    return false;
  }
  file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  // Read data from file
  //NODE  SLICE                   X                   Y               Z                   F       BCKIND STEADY
  line = GetLineFromFile1(&file);
  while(!file.eof()) 
  {
    //file.getline(buffer,MAX_ZEILE); 
    // 836      1   235554.6425086915   638255.2217882699     13.34303538        0.0000000000         WELL 1 
    line = GetLineFromFile1(&file);
    if(line.length()<1) continue;
    in.str(line);
    //....................................................................
    in >> l >> i >> x >> y >> z >> f >> bc_type >> tim_type;
    //out_file << l << " " << i << " " << x << " " << y << " " << z << " " << f << " " << bc_type << " " << tim_type << endl;
    if(bc_type.compare("WELL")==0)
    {
      m_pnt = new CGLPoint();
      m_pnt->id = (long)gli_points_vector.size();
      m_pnt->node = l;
      m_pnt->x = x;
      m_pnt->y = y;
      m_pnt->z = z;
      m_pnt->type = 2; // ST
      m_pnt->value = f;
      gli_points_vector.push_back(m_pnt); 
    //....................................................................
    }
    in.clear();
  }
  file.close();
  GEOWrite("st_pnt");
  return true;
}

/**************************************************************************
GUI-Method: 
10/2008 OK Implementation
**************************************************************************/
void CDialogFEFLOW::OnBnClickedCreateST()
{
  CGLPoint* m_pnt = NULL;
  //----------------------------------------------------------------------
  // Create ST from PNT: see STRead
  //----------------------------------------------------------------------
  CSourceTerm *m_st = NULL;
  for(long l=0;l<(long)gli_points_vector.size();l++)
  {
    m_pnt =  gli_points_vector[l];
    if(m_pnt->type==2)
    {
      m_st = new CSourceTerm();
      m_st->pcs_type_name = "GROUNDWATER_FLOW";
      m_st->pcs_pv_name = "HEAD";
      m_st->geo_type_name = "POINT";
      m_st->geo_name = m_pnt->name; //?->CC
      m_st->dis_type_name = "CONSTANT";
      m_st->geo_node_value = m_pnt->value;
      st_vector.push_back(m_st);
    }
    m_pnt->type=0; // reset (repeated use)
  }
/*
  //----------------------------------------------------------------------
  // Create PCS->bc_node from BC: see CRFProcess::Create()
  //----------------------------------------------------------------------
  //CBoundaryConditions::OnBnClickedButtonCreateGroup()
  CRFProcess* m_pcs = PCSGet(m_bc->pcs_type_name);
  if(!m_pcs) 
  {
    AfxMessageBox("No PCS data");
    return;
  }
  //else BCGroupDelete(m_bc->pcs_type_name,(string)m_strPVName);
  CBoundaryConditionNode *m_node_value = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) 
  {
    m_bc = *p_bc;
    if(m_bc->msh_type_name.compare("NODE")==0) 
    {
      m_node_value = new CBoundaryConditionNode;
      m_node_value->geo_node_number = m_bc->msh_node_number;
      m_node_value->msh_node_number = m_bc->msh_node_number; //OK redundant
      m_node_value->node_value = m_bc->geo_node_value;
      m_node_value->pcs_pv_name = m_bc->pcs_type_name; 
      m_pcs->bc_node.push_back(m_bc);
      m_pcs->bc_node_value.push_back(m_node_value);
    }
    ++p_bc;
  } // list
  // test
  m_pcs->WriteBC();
*/
  // de/activate elements
  GetDlgItem(IDC_FILE_FEFLOW_READ_BC)->EnableWindow(TRUE);
  GetDlgItem(IDC_BUTTON_CREATE_BC)->EnableWindow(FALSE);
}

/**************************************************************************
GeoSys-Method:
02/2009 OK Implementation (slow bit safe version)
**************************************************************************/
bool FEFLOWReadNodesFEM(string file_name, CFEMesh* m_msh)
{
  string line_string;
  std::stringstream line_stream;
  double x[12],z; 
  CNode* m_nod = NULL;
  long i,n,nn,n0;
  long no_nodes=0, no_elements=0, no_nodes_per_layer, no_nodes_per_layer_1;
  int j, idummy, no_layer=0, i_read=0, l;
  char a;
  CWnd *pWin = ((CWinApp*)AfxGetApp())->m_pMainWnd;
  CString m_strInfo;
  CGLPoint* m_pnt;
/*
  fstream file;
  file.open("test.txt",ios::trunc|ios::out);
  if(!file.good()) return false;
  file.setf(ios::scientific,ios::floatfield);
  file.precision(12);
*/
  //----------------------------------------------------------------------
  // File handling
  ifstream feflow_file;
  feflow_file.open(file_name.c_str());
  if(!feflow_file.good())
  {
    return false;
  }
  feflow_file.seekg(0L,ios::beg); // spool to begin
  //----------------------------------------------------------------------
  while(!feflow_file.eof()) 
  {
    line_string = GetLineFromFile1(&feflow_file);
    //....................................................................
    // CLASS
    if(line_string.find("CLASS")!=string::npos) 
    {
      // 8    1    0    3   27    0    8    8    1
      line_stream.str(GetLineFromFile1(&feflow_file));
      line_stream >> idummy >> idummy >> idummy >> idummy >> no_layer;
	  line_stream.clear();
    }
    //....................................................................
    // DIMENS
    if(line_string.find("DIMENS")!=string::npos) 
    {
      // DIMENS
      // 39465  61808      6      1   2000      0      0      0      0     -1      0      0      1      0      0      0      0
      //254744 489591      6      1   2000      2      4      0      0      1      0      0      1      0      0      0      0
      line_stream.str(GetLineFromFile1(&feflow_file));
      line_stream >> no_nodes >> no_elements >> m_msh->ele_type >> ws;
	  line_stream.clear();
      // Create nodes
//long no_nodes_per_layer = no_nodes / (no_layer+1);
//      for(i=0;i<no_nodes_per_layer*2;i++)
      for(i=0;i<no_nodes;i++)
      {
        m_nod = new CNode(i);
        m_msh->nod_vector.push_back(m_nod);
      }
    }
    //....................................................................
    // COOR
    if(line_string.compare("COOR")==0)
    {
      m_strInfo = "Import FEFLOW data: COOR";
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
      no_nodes_per_layer = no_nodes / (no_layer+1);// 9098;
      i_read = no_nodes_per_layer / 12 + 1;
      no_nodes_per_layer_1 = no_layer*no_nodes_per_layer;
//no_layer = 1;
      // x
      for(i=0;i<i_read;i++)
      {
        line_string = GetLineFromFile1(&feflow_file);
        line_stream.clear();
        line_stream.str(line_string);
        for(j=0;j<12;j++)
        {
          line_stream >> x[j] >> a;
          for(l=0;l<no_layer+1;l++)
          {
            n = i*12 + l*no_nodes_per_layer + j;
            nn = i*12 + no_nodes_per_layer_1 + j;
            if(nn>=no_nodes)
              break;
            m_nod = m_msh->nod_vector[n];
            m_nod->SetX(x[j]); 
          }
        }
        line_stream.clear();
      }
      // y
      for(i=0;i<i_read;i++)
      {
        line_string = GetLineFromFile1(&feflow_file);
        line_stream.clear();
        line_stream.str(line_string);
        for(j=0;j<12;j++)
        {
          line_stream >> x[j] >> a;
          for(l=0;l<no_layer+1;l++)
          {
            n = i*12 + l*no_nodes_per_layer + j;
            nn = i*12 + no_nodes_per_layer_1 + j;
            if(nn>=no_nodes)
              break;
            m_nod = m_msh->nod_vector[n];
            m_nod->SetY(x[j]); 
          }
        }
        line_stream.clear();
      }
    }
    //....................................................................
    // ELEV_I
    if(line_string.compare("ELEV_I")==0)
    {
      m_strInfo = "Import FEFLOW data: ELEV_I";
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
//no_layer = 27;
      no_nodes_per_layer = no_nodes / (no_layer+1);// 9098;
//no_layer = 1;
      for(l=0;l<no_layer+1;l++)
      {
        for(i=0;i<no_nodes_per_layer;i++)
        {
          line_stream.clear();
          line_string = GetLineFromFile1(&feflow_file);
          line_stream.str(line_string);
          line_stream >> z >> n0;
          line_stream.clear();
          //n = i+l*no_nodes_per_layer;
          n = n0-1 + l*no_nodes_per_layer;
          m_nod = m_msh->nod_vector[n];
          m_nod->SetZ(z); 
        }
        line_string = GetLineFromFile1(&feflow_file);
        line_stream.str(line_string);
        line_stream >> j;
        //file << j << endl;
        line_stream.clear();
      }
    }
    //....................................................................
    // EXTENTS
    if(line_string.compare("EXTENTS")==0)
    {
      m_strInfo = "FEFLOW import: EXTENTS";
      pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
      //0.00000000000000e+000,0.00000000000000e+000,7.99587000000000e+003,5.29027651306167e+003,
      line_string = GetLineFromFile1(&feflow_file);
      line_stream.clear();
      line_stream.str(line_string);
      line_stream >> x[0] >> x[1] >> x[2] >> x[3];
      //3.54101621147850e+003,3.26511822506216e+003,4.38757239488187e+003,3.81153176162252e+003,
      line_string = GetLineFromFile1(&feflow_file);
      line_stream.clear();
      line_stream.str(line_string);
      line_stream >> x[4] >> x[5] >> x[6] >> x[7];
      //
      m_pnt = new CGLPoint();
      m_pnt->id = (long)gli_points_vector.size();
      m_pnt->x = x[2];
      m_pnt->y = x[4];
      m_pnt->z = x[6];
      gli_points_vector.push_back(m_pnt);
      m_pnt = new CGLPoint();
      m_pnt->id = (long)gli_points_vector.size();
      m_pnt->x = x[3];
      m_pnt->y = x[5];
      m_pnt->z = x[7];
      gli_points_vector.push_back(m_pnt);
    }
    //....................................................................
  } // is line empty
  feflow_file.close();
  //----------------------------------------------------------------------
  m_strInfo = "Import FEFLOW data: NOD finished";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_strInfo);
  //file.close();
  return true;
}


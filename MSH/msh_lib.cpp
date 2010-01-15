/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C
#include "math.h"
// C++
#include <string>
#include <vector>
using namespace std;
// GEOLib
#include "geo_lib.h"
#include "geo_strings.h"
// MSHLib
#include "msh_lib.h"
#include "msh_nodes_rfi.h"
// PCSLib
#include "mathlib.h"
#include "nodes.h"
#include "elements.h"

#ifdef RFW_FRACTURE
#include"rf_mmp_new.h"
#endif
#ifdef USE_TOKENBUF
#include "tokenbuf.h"
#endif

// WW extern void RFConfigRenumber(void);
#ifndef NEW_EQS //WW. 07.11.2008
extern void ConfigRenumberProperties(void);
#endif
extern int ReadRFIFile(string g_strFileNameBase);
#include "rf_pcs.h"
#include "gs_project.h"

vector<Mesh_Group::CFEMesh*>fem_msh_vector;

#define FEM_FILE_EXTENSION ".msh"

double msh_x_min,msh_x_max; //OK
double msh_y_min,msh_y_max; //OK
double msh_z_min,msh_z_max; //OK
double msh_x_mid,msh_y_mid,msh_z_mid; //OK

bool msh_file_binary = false;
#define MSH_SIZE 1e5

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void MSHDelete(string m_msh_name)
{
  CFEMesh* m_fem_msh = NULL;
  int fem_msh_vector_size = (int)fem_msh_vector.size();
  for(int i=0;i<fem_msh_vector_size;i++){
    m_fem_msh = fem_msh_vector[i];
    if(m_fem_msh->pcs_name.compare(m_msh_name)==0){
      if(m_fem_msh) delete m_fem_msh;
      fem_msh_vector.erase((fem_msh_vector.begin()+i));
    }
  }
}

/**************************************************************************
GeoSys-Method: MSHOpen
Task: 
Programing:
11/2003 OK Implementation
**************************************************************************/
void MSHOpen(string file_name_base)
{
  if(!NODListExists())
    CreateNodeList();
  if(!ELEListExists())
    ElCreateElementList();
//OK  CreateEdgeList();
//OK  CreatePlainList();
  ReadRFIFile(file_name_base);
  //WW ConfigRenumberProperties();
//WW   RFConfigRenumber();
  ELEConfigJacobianMatrix();
  //OK3909  CalcElementsGeometry();
}


/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
05/2005 TK modified
05/2006 TK modified
**************************************************************************/
void FEMDeleteAll()
{
  for(int i=0;i<(int)fem_msh_vector.size();i++){
      delete fem_msh_vector[i];
      fem_msh_vector[i]=NULL;
  }
  fem_msh_vector.clear();
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
08/2005 WW Topology construction and rfi compatible
10/2005 OK BINARY
**************************************************************************/
bool FEMRead(string file_base_name)
{
 
  //----------------------------------------------------------------------
  FEMDeleteAll();  
  //----------------------------------------------------------------------
  CFEMesh *m_fem_msh = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //========================================================================
  // File handling
  msh_file_binary = false;
  string msh_file_name_bin = file_base_name + "_binary" + FEM_FILE_EXTENSION;
  string msh_file_name_ascii = file_base_name + FEM_FILE_EXTENSION;
  ifstream msh_file_bin;
  ifstream msh_file_ascii;
#ifdef USE_TOKENBUF
  TokenBuf *tokenbuf;
#endif

  msh_file_bin.open(msh_file_name_bin.c_str(),ios::binary|ios::in);
  if(msh_file_bin.good()){ 
    msh_file_binary = true;
  }
/*
  //......................................................................
  fem_msh_file_name = file_base_name + FEM_FILE_EXTENSION;
  ifstream fem_msh_file;
  //......................................................................
  fem_msh_file.open(fem_msh_file_name.data(),ios::in);
  if (!fem_msh_file.good()) 
    return false;
  fem_msh_file.getline(line,MAX_ZEILE);
  line_string = line;
  if(line_string.find("BINARY")!=string::npos)
    msh_file_binary = true;
  else
    msh_file_binary = false;
  fem_msh_file.close();
*/
  //----------------------------------------------------------------------
  cout << "MSHRead: ";
  if(msh_file_binary){
    cout << "BINARY file" << endl;
    if (!msh_file_bin.good()) 
      return false;
  }
  else{
    cout << "ASCII file" << endl;
    msh_file_ascii.open(msh_file_name_ascii.data(),ios::in);
    if (!msh_file_ascii.good()){
#ifdef MFC
      AfxMessageBox("Opening MSH file failed");
#endif
      return false;
    }
  }
  //----------------------------------------------------------------------
  // RFI - WW
if(!msh_file_binary){
  bool rfiMesh = true;
  getline(msh_file_ascii,line_string); // The first line
  if(line_string.find("#FEM_MSH")!=string::npos)
    rfiMesh = false;
  if(line_string.find("GeoSys-MSH")!=string::npos) //OK
    rfiMesh = false;
  msh_file_ascii.seekg(0L,ios::beg);
  
#ifdef USE_TOKENBUF
  tokenbuf = new TokenBuf(msh_file_ascii, 10485760);
#endif

  if (rfiMesh) 
  {
#ifdef TRACE
    std::cout << "RFI MESH" << std::endl;
#endif
     m_fem_msh = new CFEMesh();
     Read_RFI(msh_file_ascii, m_fem_msh);
     fem_msh_vector.push_back(m_fem_msh);
     msh_file_ascii.close();
     return true;
  }
}
  //========================================================================
  // Keyword loop
  //----------------------------------------------------------------------
  if(msh_file_binary){
    while(!msh_file_bin.eof()){
      char keyword_char[9];
      msh_file_bin.read((char*)(&keyword_char),sizeof(keyword_char));
      line_string = keyword_char;
      if(line_string.find("#STOP")!=string::npos)
        return true;
      //..................................................................
      if(line_string.find("#FEM_MSH")!=string::npos){ // keyword found
        m_fem_msh = new CFEMesh();
        position = m_fem_msh->ReadBIN(&msh_file_bin);
        fem_msh_vector.push_back(m_fem_msh);
        msh_file_bin.seekg(position,ios::beg);
      } // keyword found
    } // eof
    msh_file_bin.close();
  }
  //----------------------------------------------------------------------
  else{
#ifdef USE_TOKENBUF
    while(!tokenbuf->done()) {
      tokenbuf->get_non_empty_line(line, MAX_ZEILE);
      line_string = line;
      if(line_string.find("#STOP") != string::npos)
        return true;

      if(line_string.find("#FEM_MSH") != string::npos) { // mesh
        m_fem_msh = new CFEMesh();
        m_fem_msh->Read(tokenbuf);
        fem_msh_vector.push_back(m_fem_msh);
      }
    }
    delete tokenbuf;
#else
    while(!msh_file_ascii.eof()){
      msh_file_ascii.getline(line,MAX_ZEILE);
      line_string = line;
      if(line_string.find("#STOP")!=string::npos)
        return true;
      //..................................................................
      if(line_string.find("#FEM_MSH")!=string::npos) { // keyword found
        m_fem_msh = new CFEMesh();
        position = m_fem_msh->Read(&msh_file_ascii);
        fem_msh_vector.push_back(m_fem_msh);
        msh_file_ascii.seekg(position,ios::beg);
      } // keyword found
    } // eof
#endif

    msh_file_ascii.close();
  }

  //========================================================================
  return true;
}
/**************************************************************************
MSHLib-Method: Read rfi file () 
Task:
Programing:
08/2005 WW Re-implememtation
**************************************************************************/
void Read_RFI(istream& msh_file,CFEMesh* m_msh)
{
  long id;
  long i=0;
  int NumNodes=0;
  int NumElements=0;
  int End = 1;
  double x,y,z;
  string strbuffer;

  CNode* node = NULL;
  CElem* elem = NULL;
  //----------------------------------------------------------------------
  while (End) 
  {
    getline(msh_file, strbuffer);// The first line
    msh_file>>i>>NumNodes>>NumElements>>ws;
    //....................................................................
	// Node data
    for(i=0;i<NumNodes;i++){
      msh_file>>id>>x>>y>>z>>ws;
      node = new CNode(id,x,y,z);
	  m_msh->nod_vector.push_back(node);
    }
    for(i=0;i<NumElements; i++)
	{
      elem = new CElem(i);
	  elem->Read(msh_file, 1);
      m_msh->ele_vector.push_back(elem);
	}
    End =0;
  }
}


/**************************************************************************
MSHLib-Method: 
02/2006 WW Implementation
**************************************************************************/
void CompleteMesh()
{
  for(int i=0;i<(int)fem_msh_vector.size(); i++)
  {
    fem_msh_vector[i]->ConstructGrid();
    fem_msh_vector[i]->FillTransformMatrix();
  }      
}

/**************************************************************************
FEMLib-Method:
Task: Master write functionn
Programing:
03/2005 OK Implementation
10/2005 OK BINARY
last modification:
**************************************************************************/
void MSHWrite(string file_base_name)
{
  int i;
  CFEMesh* m_fem_msh = NULL;
  string sub_line;
  string line_string;
  msh_file_binary = false;
  //----------------------------------------------------------------------
  // File handling
  string fem_msh_file_name = file_base_name + FEM_FILE_EXTENSION;
  fstream fem_msh_file;
  string msh_file_test_name = file_base_name + "_test" + FEM_FILE_EXTENSION;
  fstream msh_file_test;
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_fem_msh = fem_msh_vector[i];
    if(m_fem_msh->ele_vector.size()>MSH_SIZE)
      //msh_file_binary = true;
      msh_file_binary = false; //TK 21.12.05 default false / TODO!!!
  }
  if(msh_file_binary){
    fem_msh_file_name = file_base_name + "_binary" + FEM_FILE_EXTENSION;
    fem_msh_file.open(fem_msh_file_name.c_str(),ios::binary|ios::out);
    //msh_file_test.open(msh_file_test_name.c_str(),ios::trunc|ios::out);OK
    //msh_file_test.open(fem_msh_file_name.c_str(),ios::trunc|ios::out);//TK
    if(!fem_msh_file.good()) return;
  }
  else{
    fem_msh_file.open(fem_msh_file_name.c_str(),ios::trunc|ios::out);
    if(!fem_msh_file.good()) return;
    fem_msh_file.setf(ios::scientific,ios::floatfield);
    fem_msh_file.precision(12);
  }
  //----------------------------------------------------------------------
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_fem_msh = fem_msh_vector[i];
    if(msh_file_binary)
      m_fem_msh->WriteBIN(&fem_msh_file,&msh_file_test);
    else
      m_fem_msh->Write(&fem_msh_file);
  }
  //----------------------------------------------------------------------
  if(msh_file_binary){
    char binary_char[6] = "#STOP";
    fem_msh_file.write((char*)(&binary_char),sizeof(binary_char));
  }
  else{
    fem_msh_file << "#STOP";
  }
  fem_msh_file.close();
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
05/2005 OK
last modification:
**************************************************************************/
void MSHTopology()
{
  CFEMesh* m_msh = NULL;
  //----------------------------------------------------------------------
  // Soil elements
  m_msh = FEMGet("RICHARDS_FLOW");
  if(m_msh)
    m_msh->SetNOD2ELETopology();
  //----------------------------------------------------------------------
  // Groundwater elements
  m_msh = FEMGet("GROUNDWATER_FLOW");
  if(m_msh)
    m_msh->SetELE2NODTopology();
}



/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
last modification:
**************************************************************************/
CFEMesh* FEMGet(const string &msh_name)
{
  int no_msh = (int)fem_msh_vector.size();
  // If there is only one msh file available, use it for all process. WW
  if(no_msh==1) return fem_msh_vector[0]; //WW
  CFEMesh* m_msh = NULL;
  for(int i=0;i<no_msh;i++){
    m_msh = fem_msh_vector[i];
    if(m_msh->pcs_name.compare(msh_name)==0)
      return m_msh;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
**************************************************************************/
void MSHCalcMinMaxMidCoordinates()
{
  double m_dXMin1 = 1.e+19;
  double m_dXMax1 = -1.e+19;
  double m_dYMin1 = 1.e+19;
  double m_dYMax1 = -1.e+19;
  double m_dZMin1 = 1.e+19;
  double m_dZMax1 = -1.e+19;
  double value;
  CFEMesh* m_msh = NULL;
  //----------------------------------------------------------------------
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
    for(long i=0;i<(long)m_msh->nod_vector.size();i++) 
    {
      value = m_msh->nod_vector[i]->X();
      if(value<m_dXMin1) m_dXMin1 = value;
      if(value>m_dXMax1) m_dXMax1 = value;
      value = m_msh->nod_vector[i]->Y();
      if(value<m_dYMin1) m_dYMin1 = value;
      if(value>m_dYMax1) m_dYMax1 = value;
      value = m_msh->nod_vector[i]->Z();
      if(value<m_dZMin1) m_dZMin1 = value;
      if(value>m_dZMax1) m_dZMax1 = value;
      //..................................................................
      // Shrink a bit
      msh_x_min = m_dXMin1 - 0.05*(m_dXMax1-m_dXMin1);
      msh_x_max = m_dXMax1 + 0.05*(m_dXMax1-m_dXMin1);
      msh_y_min = m_dYMin1 - 0.05*(m_dYMax1-m_dYMin1);
      msh_y_max = m_dYMax1 + 0.05*(m_dYMax1-m_dYMin1);
      msh_z_min = m_dZMin1 - 0.05*(m_dZMax1-m_dZMin1);
      msh_z_max = m_dZMax1 + 0.05*(m_dZMax1-m_dZMin1);
    }
  }
  //----------------------------------------------------------------------
  msh_x_mid = 0.5*(msh_x_min+msh_x_max);
  msh_y_mid = 0.5*(msh_y_min+msh_y_max);
  msh_z_mid = 0.5*(msh_z_min+msh_z_max);
  //----------------------------------------------------------------------
}


/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
01/2005 OK File handling
09/2005 OK MSH ToDo
last modification: 
**************************************************************************/
void MSHWriteVOL2TEC(string m_msh_name)
{
  long i,j;
  CGLVolume *m_vol = NULL;
  vector<CGLVolume*>::const_iterator p_vol;
  string name("VOLUMES");
  vector<Surface*>::const_iterator p_sfc;
  string delimiter(", ");
  double x,y,z;
  CGLPoint m_point;
  ios::pos_type position;
  int vol_number = -1;
  Surface* m_sfc = NULL;
  //--------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet(m_msh_name);
  if(!m_msh)
    return;
  long no_nodes = (long)m_msh->nod_vector.size();
  long ep_layer = (long)m_msh->ele_vector.size() / m_msh->no_msh_layer;
  //--------------------------------------------------------------------
  // File handling
  string tec_path;
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp)
    tec_path = m_gsp->path; 
  //======================================================================
  p_vol = volume_vector.begin();
  while(p_vol!=volume_vector.end()) {
    m_vol = *p_vol;
    if(m_vol->layer==0){ //OK
      p_vol++;
      continue;
    }
    p_sfc = m_vol->surface_vector.begin();
    m_sfc = *p_sfc;
    if(!m_sfc)
      return;
    //--------------------------------------------------------------------
    long jb = (m_vol->layer-1)*ep_layer;
    long je = jb + ep_layer;
    vol_number++;
    //--------------------------------------------------------------------
    // file handling
    string vol_file_name = tec_path + "VOL_" + m_vol->name + TEC_FILE_EXTENSION;
    fstream vol_file (vol_file_name.data(),ios::trunc|ios::out);
    vol_file.setf(ios::scientific,ios::floatfield);
	vol_file.precision(12);
    if (!vol_file.good()) return;
    vol_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    vol_file << "VARIABLES = X,Y,Z,VOL" << endl;
    //--------------------------------------------------------------------
    long no_mat_elements = 0;
    CElem* m_ele = NULL;
    vec<long>node_indeces(6);
    for(i=jb;i<je;i++){
      m_ele = m_msh->ele_vector[i];
	  if(m_ele->GetElementType()==6){
        m_ele->GetNodeIndeces(node_indeces);
        //nodes = m_msh->ele_vector[i]->nodes;
        x=0.0; y=0.0; z=0.0;
        for(j=0;j<6;j++) {
          x += m_msh->nod_vector[node_indeces[j]]->X();
          y += m_msh->nod_vector[node_indeces[j]]->Y();
          z += m_msh->nod_vector[node_indeces[j]]->Z();
        }
        x /= double(6);
        y /= double(6);
        z /= double(6);
        m_point.x = x;
        m_point.y = y;
        m_point.z = z;
        if(m_sfc->PointInSurface(&m_point)){
          no_mat_elements++; 
        }
      }
    }
#ifdef MFC
    if(no_mat_elements<1){
      CString m_str = "Warning: no ELE data for VOL ";
      m_str += m_vol->name.data();
      AfxMessageBox(m_str);
    }
#endif
    //--------------------------------------------------------------------
    position = vol_file.tellg();
    vol_file << "ZONE T = " << m_vol->name << ", " \
             << "N = " << no_nodes << ", " \
             << "E = " << no_mat_elements << ", " \
             << "F = FEPOINT" << ", " << "ET = BRICK" << endl;
    for(i=0;i<no_nodes;i++) {
      vol_file \
        << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << " " << vol_number << endl;
    }
    for(long i=jb;i<je;i++){
      m_ele = m_msh->ele_vector[i];
	  if(m_ele->GetElementType()==6){
        m_ele->GetNodeIndeces(node_indeces);
        x=0.0; y=0.0; z=0.0;
        for(j=0;j<6;j++) {
          x += m_msh->nod_vector[node_indeces[j]]->X();
          y += m_msh->nod_vector[node_indeces[j]]->Y();
          z += m_msh->nod_vector[node_indeces[j]]->Z();
        }
        x /= double(6);
        y /= double(6);
        z /= double(6);
        m_point.x = x;
        m_point.y = y;
        m_point.z = z;
        if(m_sfc->PointInSurface(&m_point)){
          vol_file \
            << node_indeces[0]+1 << " " << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " << node_indeces[2]+1 << " " \
            << node_indeces[3]+1 << " " << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " << node_indeces[5]+1 << endl;
        }
      }
    }
    ++p_vol;
  //======================================================================
  }
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2005 OK Implementation
11/2005 OK OO-ELE
**************************************************************************/
void MSHWriteTecplot()
{
  int ele_type  = -1;
  long no_nodes;
  long no_elements;
  string delimiter(", ");
  long i;
  CElem* m_ele = NULL;
  vec<long>node_indeces(8);
  //----------------------------------------------------------------------
  // File handling
  string file_path = "MSH";
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(m_gsp)
    file_path = m_gsp->path + "MSH";
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
    no_nodes = (long)m_msh->nod_vector.size();
    no_elements = (long)m_msh->ele_vector.size();
    // Test ele_type
    if(no_elements>0)
    {
      m_ele = m_msh->ele_vector[0];
      ele_type = m_ele->GetElementType();
    }
    // File handling
    string msh_file_name = file_path + "_" + m_msh->pcs_name + TEC_FILE_EXTENSION;
    fstream msh_file (msh_file_name.data(),ios::trunc|ios::out);
    msh_file.setf(ios::scientific,ios::floatfield);
    msh_file.precision(12);
    if (!msh_file.good()) return;
    msh_file.seekg(0L,ios::beg);
    msh_file << "VARIABLES = X,Y,Z" << endl;
    msh_file << "ZONE T = " << m_msh->pcs_name << delimiter \
             << "N = " << no_nodes << delimiter \
             << "E = " << no_elements << delimiter;
    msh_file << "F = FEPOINT" << delimiter;
    switch(ele_type)
    {
      //..................................................................
      case 1:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[1]+1 << " " << node_indeces[0]+1 << endl;
        }
        break;
      //..................................................................
      case 2:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 3:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << " " \
            << node_indeces[4]+1 << " " << node_indeces[5]+1 << " " \
            << node_indeces[6]+1 << " " << node_indeces[7]+1 << endl;
        }
        break;
      //..................................................................
      case 4:
        msh_file << "ET = TRIANGLE" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << endl;
        }
        break;
      //..................................................................
      case 5:
        msh_file << "ET = TETRAHEDRON" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 6:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) 
        {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          if(m_ele->GetElementType()==6)
          {
            msh_file \
              << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
              << node_indeces[2]+1 << " " << node_indeces[2]+1 << " " \
              << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " \
              << node_indeces[5]+1 << " " << node_indeces[5]+1 << endl;
          }
          if(m_ele->GetElementType()==3)
          {
            msh_file \
              << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
              << node_indeces[2]+1 << " " << node_indeces[3]+1 << " " \
              << node_indeces[4]+1 << " " << node_indeces[5]+1 << " " \
              << node_indeces[6]+1 << " " << node_indeces[7]+1 << endl;
          }
        }
        break;
    }
  }
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2005 OK Implementation
11/2005 OK OO-ELE
**************************************************************************/
void MSHLayerWriteTecplot()
{
  int ele_type  = -1;
  long no_nodes;
  long no_elements;
  string delimiter(", ");
  long i;
  CElem* m_ele = NULL;
  vec<long>node_indeces(8);
  int k;
  string no_layer_str;
  char no_layer_char[3];
  //----------------------------------------------------------------------
  // File handling
  string file_path = "MSH";
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(m_gsp)
    file_path = m_gsp->path;
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
for(k=0;k<m_msh->no_msh_layer;k++){
    sprintf(no_layer_char,"%i",k+1);
    no_layer_str = no_layer_char;
    no_nodes = (long)m_msh->nod_vector.size()/(m_msh->no_msh_layer+1);
    no_elements = (long)m_msh->ele_vector.size()/m_msh->no_msh_layer;
    // Test ele_type
    if(no_elements>0){
      m_ele = m_msh->ele_vector[0];
      ele_type = m_ele->GetElementType();
    }
    // File handling
    string msh_file_name = file_path + "MSH_LAYER" + no_layer_str + "_" + m_msh->pcs_name + TEC_FILE_EXTENSION;
    fstream msh_file (msh_file_name.data(),ios::trunc|ios::out);
    msh_file.setf(ios::scientific,ios::floatfield);
    msh_file.precision(12);
    if (!msh_file.good()) return;
    msh_file.seekg(0L,ios::beg);
    msh_file << "VARIABLES = X,Y,Z" << endl;
    msh_file << "ZONE T = " << m_msh->pcs_name << delimiter \
             << "N = " << (long)m_msh->nod_vector.size() << delimiter \
             << "E = " << no_elements << delimiter;
    msh_file << "F = FEPOINT" << delimiter;
    switch(ele_type){
      //..................................................................
      case 1:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[1]+1 << " " << node_indeces[0]+1 << endl;
        }
        break;
      //..................................................................
      case 2:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 3:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << " " \
            << node_indeces[4]+1 << " " << node_indeces[5]+1 << " " \
            << node_indeces[6]+1 << " " << node_indeces[7]+1 << endl;
        }
        break;
      //..................................................................
      case 4:
        msh_file << "ET = TRIANGLE" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << endl;
        }
        break;
      //..................................................................
      case 5:
        msh_file << "ET = TETRAHEDRON" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 6:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[2]+1 << " " \
            << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " \
            << node_indeces[5]+1 << " " << node_indeces[5]+1 << endl;
        }
        break;
    }
} // layer
  }
}

/**************************************************************************
MSHLib-Method: 
12/2005 OK Implementation
07/2007 OK PCS
**************************************************************************/
CFEMesh* MSHGet(const string &geo_name)
{
  CFEMesh* m_msh = NULL;
  for(int i=0;i<(int)fem_msh_vector.size();i++)
  {
    m_msh = fem_msh_vector[i];
    if(m_msh->geo_name.compare(geo_name)==0)
    {
      return m_msh;
    }
    if(m_msh->pcs_name.compare(geo_name)==0)
    {
      return m_msh;
    }
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* MSHGet(const string &pcs_type_name,const string &geo_name)
{
  CFEMesh* m_msh = NULL;
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    if((m_msh->pcs_name.compare(pcs_type_name)==0)&&\
       (m_msh->geo_name.compare(geo_name)==0)){
      return m_msh;
    }
  }
  return NULL;
}

/**************************************************************************
PCSLib-Method:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* MSHGetGEO(string geo_name)
{
  int no_msh = (int)fem_msh_vector.size();
  // If there is only one msh file available, use it for all process. WW
  if(no_msh==1) 
    return fem_msh_vector[0]; //WW
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int i=0;i<no_msh;i++){
    m_msh = fem_msh_vector[i];
    if(m_msh->geo_name.compare(geo_name)==0)
      return m_msh;
  }
  //----------------------------------------------------------------------
  return NULL;
}
#ifdef RFW_FRACTURE
/**************************************************************************************
 ROCKFLOW - Funktion: ELEGetCommonNodes
 Aufgabe:
   Get the nodes shared by 2 elements
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: const CElem* elem1 and elem2  :  element pointers
   R: vec<CNode*> nodes  :  a vector containing pointers to the matching (i.e. common) nodes
 Ergebnis:
   Returns the value true if elements are actually neighbors(i.e. have nodes in common),
   otherwise returns false
 Programmaenderungen:
   05/2005 RFW Implementierung
   05/2006 RFW Änderung
***************************************************************************************/
bool MSHGetCommonNodes(CElem* elem1, CElem* elem2, vector<CNode*>& nodes)
{
nodes.clear();
bool neighbor = false;
vec<CNode*> nodes1, nodes2; 
int numnodes1, numnodes2;
if(elem1!=NULL && elem2!=NULL)//RFW 19/05/2005
{
    elem1->GetNodes(nodes1);
    elem2->GetNodes(nodes2);
    numnodes1 = elem1->GetVertexNumber();
    numnodes2 = elem2->GetVertexNumber();

    for(int i=0; i!=numnodes1; ++i)
    {
    for(int j=0; j!=numnodes2; ++j)
    {
        if(nodes1[i] == nodes2[j])
            nodes.push_back(nodes1[i]);
    }
    }
    if(nodes.size() == 0)
    return neighbor;
    else{
    neighbor = true;
    return neighbor;
    }
}
else
    return neighbor;
}

/*************************************************************************
 ROCKFLOW - Funktion: MSHSetFractureElements
 Aufgabe:
   This function gives each fracture element a weight and an aperture direction.  It
   does this by using polylines that define the top and bottom of the 2D fracture.  For
   the function to work properly, the points defining the polyline must be evenly spaced.
   Function should only be called once, after configuration.
 Formalparameter: 
 Ergebnis:
 Programmaenderungen:
   11/2005 RFW Implementierung
***************************************************************************/
void MSHSetFractureElements(void)
{
                //** TEST *************
cout << "Location 0 MSHSetFractureElements\n";
                //** TEST *************
int group, frac_exists=0;
CElem *elem = NULL;
CGLPolyline *frac_top = NULL, *frac_bot = NULL;
CMediumProperties *m_mmp = NULL;
CFEMesh* m_msh = NULL;
group = -1; //WW
vector<long> bound_elements;
vector<double> node_x, node_y; 
double x, y, tri[6], dx_avg, dy_avg, d_max, *gravity_center;
//The following vector is a strange beast.  The first index indicates the fracture being considered, the second
//index indicates the segment of the fracture being considered, and the final index contains the element numbers
//of those elements making up the given fracture segment.
vector<vector<vector<int> > > segment_elements;
vector<vector<bool> > segment_on_boundary;
vector<vector<double> > segment_dx;
vector<vector<double> > segment_dy;
vector<int> fractures;

//finding the material group for the fractures, there should only be 1
for(int i=0; i<(int)mmp_vector.size(); ++i)
{
    m_mmp = mmp_vector[i];
    //for(int j=0; j<(int)m_mmp->relative_permeability_function.size(); ++j)
    //{
        if (m_mmp->frac_num != 0) 
        {
            group = i;
            frac_exists = 1;
        }
    //}
}

if(frac_exists)
{
    m_mmp = mmp_vector[group];
    //frac_segments.resize(m_mmp->frac_num);
    segment_elements.resize(m_mmp->frac_num);
    segment_on_boundary.resize(m_mmp->frac_num);
    segment_dx.resize(m_mmp->frac_num);
    segment_dy.resize(m_mmp->frac_num);

    //grabbing the nodes of the boundary polyline, will be used later---------------------START
    CGLPolyline *bound_ply = NULL;
    vector<long> poly_nodes;

    bound_ply = GEOGetPLYByName("BOUNDARY");
    m_msh = fem_msh_vector[0]; //this isn't great


    if(bound_ply)
    {
        m_msh->GetNODOnPLY(bound_ply, poly_nodes);
    }
    else
    {
        cout <<"Error 1 in ELESetBoundaryElements, no BOUNDARY polyline.\n";
        abort();
    }
    //grabbing the nodes of the boundary polyline-------------------------------------------------END

    for(long j=0; j<m_mmp->frac_num; ++j) //loop1, over all fractures
    {
        string polyname_top = m_mmp->frac_names[j] + "_top";
        string polyname_bot = m_mmp->frac_names[j] + "_bot";
        frac_top = GEOGetPLYByName(polyname_top); 
        frac_bot = GEOGetPLYByName(polyname_bot); 
        //frac_segments[j].resize((long)frac_top->point_vector.size()-1);
        segment_elements[j].resize((long)frac_top->point_vector.size()-1);
        segment_on_boundary[j].resize((long)frac_top->point_vector.size()-1);
        segment_dx[j].resize((long)frac_top->point_vector.size()-1);
        segment_dy[j].resize((long)frac_top->point_vector.size()-1);

        if(!frac_top || !frac_bot)
        {
            cout << "Error 1 in MSHSetFractureElements: fracture polyline "<<m_mmp->frac_names[j]<<" not found.\n";
            abort();
        }
        for(long k=0; k<((long)frac_top->point_vector.size()-1); ++k) //loop2, over fracture segments
        {                     
            node_x.resize(4); //of course, these are not really nodes, but I'm using code from ELEWhatElemIsPointIn
            node_y.resize(4);
            //4 points defining the fracture segment
            node_x[0]=frac_bot->point_vector[k]->x;        node_y[0]=frac_bot->point_vector[k]->y;
            node_x[1]=frac_bot->point_vector[k+1]->x;    node_y[1]=frac_bot->point_vector[k+1]->y;
            node_x[2]=frac_top->point_vector[k+1]->x;    node_y[2]=frac_top->point_vector[k+1]->y;
            node_x[3]=frac_top->point_vector[k]->x;        node_y[3]=frac_top->point_vector[k]->y;

            //2 trangular subareas defining the fracture segment
            double Area1, Area2;
            Area1 = ( (node_x[1]*node_y[2] - node_x[2]*node_y[1])
                + (node_x[2]*node_y[0] - node_x[0]*node_y[2])
                + (node_x[0]*node_y[1] - node_x[1]*node_y[0]) )/2;
            Area2 = ( (node_x[3]*node_y[2] - node_x[2]*node_y[3])
                + (node_x[2]*node_y[0] - node_x[0]*node_y[2])
                + (node_x[0]*node_y[3] - node_x[3]*node_y[0]) )/2;

            for (int l = 0; l < (int)m_msh->ele_vector.size(); l++)//loop 3, all elements
            {
                elem = m_msh->ele_vector[l];
                if(elem->GetPatchIndex()==group) //this is simply to make things go faster
                {
                    
                    gravity_center = elem->GetGravityCenter();
                    x = gravity_center[0]; //x = ELEGetEleMidPoint(l,0);
                    y = gravity_center[1]; //y = ELEGetEleMidPoint(l,1);
                               
                    //calculate triangular coordinates for both triangles making up fracture segment
                    //first triangle
                    tri[0] = ( (node_x[1]*node_y[2]-node_x[2]*node_y[1]) + (node_y[1]-node_y[2])*x + (node_x[2]-node_x[1])*y )/(2*Area1);
                    tri[1] = ( (node_x[2]*node_y[0]-node_x[0]*node_y[2])+ (node_y[2]-node_y[0])*x + (node_x[0]-node_x[2])*y )/(2*Area1);
                    tri[2] = ( (node_x[0]*node_y[1]-node_x[1]*node_y[0]) + (node_y[0]-node_y[1])*x + (node_x[1]-node_x[0])*y )/(2*Area1);
                    //second triangle
                    tri[3] = ( (node_x[3]*node_y[2]-node_x[2]*node_y[3]) + (node_y[3]-node_y[2])*x + (node_x[2]-node_x[3])*y )/(2*Area2);
                    tri[4] = ( (node_x[2]*node_y[0]-node_x[0]*node_y[2])+ (node_y[2]-node_y[0])*x + (node_x[0]-node_x[2])*y )/(2*Area2);
                    tri[5] = ( (node_x[0]*node_y[3]-node_x[3]*node_y[0]) + (node_y[0]-node_y[3])*x + (node_x[3]-node_x[0])*y )/(2*Area2);
                    
                    if((tri[0]>=-0.00000001 && tri[1]>=-0.00000001 && tri[2]>=-0.00000001)
                    || (tri[3]>=-0.00000001 && tri[4]>=-0.00000001 && tri[5]>=-0.00000001))
                    {     
                      elem->SetFracNum(j);
                       
                        //what is the appropriate search dirction for aperture searches?
                        dx_avg = ( (node_x[1]-node_x[0]) + (node_x[2]-node_x[3]) )/2;
                        dy_avg = ( (node_y[1]-node_y[0]) + (node_y[2]-node_y[3]) )/2;
                        d_max = max(abs(dx_avg), abs(dy_avg));
                        elem->SetFracDx(-1*dy_avg/d_max);
                        elem->SetFracDy(dx_avg/d_max);

                        //increment the number of elements in the fracture segment, for weight calculations
                        //frac_segments[j][k] += 1;
                        segment_elements[j][k].push_back( l );

                        //check if element is on boundary, if so, the segment_on_boundary gets a value of true, otherwise false
                        vector<CNode*> match_nodes; 
                        vec<CNode*> elem_nodes; 
                        elem->GetNodes(elem_nodes); 
                        int elem_num_nodes = elem->GetVertexNumber();
                    
                        for(long m=0; m!=(long)poly_nodes.size(); ++m)
                        {
                            for(int n=0; n!=elem_num_nodes; ++n)
                            {
                                if(elem_nodes[n] == m_msh->nod_vector[poly_nodes[m]])  //NOT SURE THIS WILL WORK!, probably will
                                {
                                    match_nodes.push_back(elem_nodes[n]);
                                }
                            }
                        }
                        if(match_nodes.size()==2)
                        {
                            segment_on_boundary[j][k] = true;
                            dx_avg = match_nodes[0]->X() - match_nodes[1]->X();
                            dy_avg = match_nodes[0]->Y() - match_nodes[1]->Y();
                            d_max = max(abs(dx_avg), abs(dy_avg));
                            segment_dx[j][k] = dx_avg/d_max; //no longer the inverese of the average slope as above, rather the orientation of the boundary
                            segment_dy[j][k] = dy_avg/d_max;
                        }                       

                    }
                }//if GroupNumber
            }//loop 3, all elements
        }//loop2, over fracture segments
    }//loop1, over all fractures



    for(long j=0; j<m_mmp->frac_num; ++j)//loop4, over all fractures
    {
        double Weight=0, seg_length=0;
        vector<double> point_x, point_y;
        point_x.resize(2); point_y.resize(2);
        string polyname_top = m_mmp->frac_names[j] + "_top";
        string polyname_bot = m_mmp->frac_names[j] + "_bot";
        frac_top = GEOGetPLYByName(polyname_top); 
        frac_bot = GEOGetPLYByName(polyname_bot);

        //calculating polyline length
	    double top_poly_length = frac_top->CalcPolylineLength(); 

        for(long k=0; k<((long)frac_top->point_vector.size()-1); ++k) //loop5, over fracture segments
        {
            //calculating segment length
            point_x[0]=frac_top->point_vector[k+1]->x;    point_y[0]=frac_top->point_vector[k+1]->y;
            point_x[1]=frac_top->point_vector[k]->x;        point_y[1]=frac_top->point_vector[k]->y;
            seg_length = sqrt(   pow( (point_x[1]-point_x[0]), 2 ) + pow( (point_y[1]-point_y[0]), 2 )   );

            for(long l=0; l<(long)segment_elements[j][k].size(); ++l)//loop6, over elements in segment
            {
                //the weight that each element gets is the total weight of 1, divided by the number of segments
                //in which the fracture is divided, and further divided by the number of elements in the given segment
//WW                long test1 = ((long)frac_top->point_vector.size()-1);
//WW                long test2 = (long)segment_elements[j][k].size();
                Weight = seg_length / top_poly_length / (double)segment_elements[j][k].size();
                
                elem = m_msh->ele_vector[segment_elements[j][k][l]];
                //elem->in_frac = true; // RFW 18/11/2005
                elem->SetFrac(Weight);
                //if element is part of boundary segment, reassign it's search directions appropriately
                if(segment_on_boundary[j][k] == true)
                {
                  elem->SetFracDx(segment_dx[j][k]);
                  elem->SetFracDy(segment_dy[j][k]);
                }

            }//loop6, over elements in segment
        }//loop5, over fracture segments
    }//loop4, over all fractures
}//if frac_exists

}

/*************************************************************************
 ROCKFLOW - Funktion: MSHResetFractureElements
 Aufgabe:
  Resets the aperture calculation for the next time, this means that CalculateFracAperture
  function will not be called multiple times each timestep
 Formalparameter: 
 Ergebnis:
 Programmaenderungen:
   11/2005 RFW Implementierung
***************************************************************************/
void MSHResetFractureElements(void)
{
CElem *elem = NULL;
CFEMesh* m_msh = NULL;
m_msh = fem_msh_vector[0]; //this isn't great

for (int i = 0; i < (int)m_msh->ele_vector.size(); i++)//loop 3, all elements
{
     elem = m_msh->ele_vector[i];
     elem->ApertureIsNotSet();
     elem->PermeabilityIsNotSet();
}

for(int i = 0; i<(int)mmp_vector.size(); i++)
{
  mmp_vector[i]->fracs_set=0;
}

}

/**************************************************************************
GeoSys-FEM-Method: 
Task: Find what element a set of given coords is in.  Works for triangles and quadrilatera in a 2d system.
Programming: 
      RFW 04.2005 index is an initial guess of an element close to
      the one containing the given point.
**************************************************************************/
long MSHWhatElemIsPointIn(double x, double y, long index)
{
    CElem* elem = NULL;  //PointElementNow
    vec<CElem*> neighbors(10);
    CFEMesh* m_msh = NULL;
    m_msh = fem_msh_vector[0];  // Something must be done later on here.
    //need a pointer for node
    bool inside = false, in_domain = false;
    int num_face, num_nodes;
    double Area1, Area2, tri[6];
    vector<double> node_x, node_y; 
    long return_value=0, count=0;//perhaps return_value shouldn't be initialized
    vector<long> index_guess;
    index_guess.push_back(index);
    long number_of_elem = (long)m_msh->ele_vector.size();

    //----------------------------------------------point in model domain?-------------------------------------------------------START
    //boundary polyline defining the model boundaries, used to check if point is inside or outside model domain
    //this is not a very elegant solution, needs to be improved.  Current assumption, quadratisch model domain.
    CGLPolyline *bound_ply = NULL;
    vector<double> bnode_x, bnode_y;
    bnode_x.resize(4);  bnode_y.resize(4); //of course, these are not really nodes, rather points
    bound_ply = GEOGetPLYByName("BOUNDARY");
    bnode_x[0]=bound_ply->point_vector[0]->x;       bnode_y[0]=bound_ply->point_vector[0]->y;
    bnode_x[1]=bound_ply->point_vector[1]->x;        bnode_y[1]=bound_ply->point_vector[1]->y;
    bnode_x[2]=bound_ply->point_vector[2]->x;        bnode_y[2]=bound_ply->point_vector[2]->y;
    bnode_x[3]=bound_ply->point_vector[3]->x;        bnode_y[3]=bound_ply->point_vector[3]->y;    
   
    Area1 = ( (bnode_x[1]*bnode_y[2] - bnode_x[2]*bnode_y[1])
        + (bnode_x[2]*bnode_y[0] - bnode_x[0]*bnode_y[2])
        + (bnode_x[0]*bnode_y[1] - bnode_x[1]*bnode_y[0]) )/2;
    Area2 = ( (bnode_x[3]*bnode_y[2] - bnode_x[2]*bnode_y[3])
        + (bnode_x[2]*bnode_y[0] - bnode_x[0]*bnode_y[2])
        + (bnode_x[0]*bnode_y[3] - bnode_x[3]*bnode_y[0]) )/2;
    //calculate triangular coordinates for both triangles making up the element
    //first triangle
    tri[0] = ( (bnode_x[1]*bnode_y[2]-bnode_x[2]*bnode_y[1]) + (bnode_y[1]-bnode_y[2])*x + (bnode_x[2]-bnode_x[1])*y )/(2*Area1);
    tri[1] = ( (bnode_x[2]*bnode_y[0]-bnode_x[0]*bnode_y[2])+ (bnode_y[2]-bnode_y[0])*x + (bnode_x[0]-bnode_x[2])*y )/(2*Area1);
    tri[2] = ( (bnode_x[0]*bnode_y[1]-bnode_x[1]*bnode_y[0]) + (bnode_y[0]-bnode_y[1])*x + (bnode_x[1]-bnode_x[0])*y )/(2*Area1);
    //second triangle
    tri[3] = ( (bnode_x[3]*bnode_y[2]-bnode_x[2]*bnode_y[3]) + (bnode_y[3]-bnode_y[2])*x + (bnode_x[2]-bnode_x[3])*y )/(2*Area2);
    tri[4] = ( (bnode_x[2]*bnode_y[0]-bnode_x[0]*bnode_y[2])+ (bnode_y[2]-bnode_y[0])*x + (bnode_x[0]-bnode_x[2])*y )/(2*Area2);
    tri[5] = ( (bnode_x[0]*bnode_y[3]-bnode_x[3]*bnode_y[0]) + (bnode_y[0]-bnode_y[3])*x + (bnode_x[3]-bnode_x[0])*y )/(2*Area2);
    
    if((tri[0]>=-0.0000001 && tri[1]>=-0.0000001 && tri[2]>=-0.0000001)
        || (tri[3]>=-0.0000001 && tri[4]>=-0.0000001 && tri[5]>=-0.0000001)) //perhaps this should be +ve??
    {
        in_domain = true;
    }
    else //point is not in model domain
    {
        return_value = -100;
    }
        
    //----------------------------------------------point in model domain?---------------------------------------------------------END

    //----------------------------------------------which element is point in?----------------------------------------------------START
    while (!inside && count < (number_of_elem+2) && in_domain)
    {
    count++;
    CNode* node;
    if ((long)index_guess.size() > number_of_elem) //check to avoid infinite loops
    {
        cout<<"Error in ELEWhatElemIsPointIn, point not in any element";
        return_value = -100;
        break;
    }
    for (long i=0; i!=(long)index_guess.size(); ++i)
    {
        elem = m_msh->ele_vector[index_guess[i]];
        num_nodes = elem->GetVertexNumber();
        node_x.resize(num_nodes);
        node_y.resize(num_nodes);
        for(int j=0; j!=num_nodes; ++j)
        {
            node = elem->GetNode(j);
			node_x[j] = node->X_displaced();
			node_y[j] = node->Y_displaced();
        }
        //----------------------------------------------triangles------------------------------------------------------------
        if(num_nodes==3)
        {
           //calculate area of triangle
           Area1 = ( (node_x[1]*node_y[2] - node_x[2]*node_y[1])
               + (node_x[2]*node_y[0] - node_x[0]*node_y[2])
               + (node_x[0]*node_y[1] - node_x[1]*node_y[0]) )/2;
          //calculate triangular coordinates
           tri[0] = ( (node_x[1]*node_y[2]-node_x[2]*node_y[1]) + (node_y[1]-node_y[2])*x + (node_x[2]-node_x[1])*y )/(2*Area1);
           tri[1] = ( (node_x[2]*node_y[0]-node_x[0]*node_y[2])+ (node_y[2]-node_y[0])*x + (node_x[0]-node_x[2])*y )/(2*Area1);
           tri[2] = ( (node_x[0]*node_y[1]-node_x[1]*node_y[0]) + (node_y[0]-node_y[1])*x + (node_x[1]-node_x[0])*y )/(2*Area1);

          if(tri[0]>=-0.0000001 && tri[1]>=-0.0000001 && tri[2]>=-0.0000001 ) //perhaps this should be +ve??
          {
              inside = true;
              return_value = index_guess[i];
              break;
          }
        }
        //----------------------------------------------quadrilaterals------------------------------------------------------------
        else if(num_nodes == 4)
        {
           Area1 = ( (node_x[1]*node_y[2] - node_x[2]*node_y[1])
               + (node_x[2]*node_y[0] - node_x[0]*node_y[2])
               + (node_x[0]*node_y[1] - node_x[1]*node_y[0]) )/2;
           Area2 = ( (node_x[3]*node_y[2] - node_x[2]*node_y[3])
               + (node_x[2]*node_y[0] - node_x[0]*node_y[2])
               + (node_x[0]*node_y[3] - node_x[3]*node_y[0]) )/2;
           //calculate triangular coordinates for both triangles making up the element
           //first triangle
           tri[0] = ( (node_x[1]*node_y[2]-node_x[2]*node_y[1]) + (node_y[1]-node_y[2])*x + (node_x[2]-node_x[1])*y )/(2*Area1);
           tri[1] = ( (node_x[2]*node_y[0]-node_x[0]*node_y[2])+ (node_y[2]-node_y[0])*x + (node_x[0]-node_x[2])*y )/(2*Area1);
           tri[2] = ( (node_x[0]*node_y[1]-node_x[1]*node_y[0]) + (node_y[0]-node_y[1])*x + (node_x[1]-node_x[0])*y )/(2*Area1);
          //second triangle
           tri[3] = ( (node_x[3]*node_y[2]-node_x[2]*node_y[3]) + (node_y[3]-node_y[2])*x + (node_x[2]-node_x[3])*y )/(2*Area2);
           tri[4] = ( (node_x[2]*node_y[0]-node_x[0]*node_y[2])+ (node_y[2]-node_y[0])*x + (node_x[0]-node_x[2])*y )/(2*Area2);
           tri[5] = ( (node_x[0]*node_y[3]-node_x[3]*node_y[0]) + (node_y[0]-node_y[3])*x + (node_x[3]-node_x[0])*y )/(2*Area2);
           
           if((tri[0]>=-0.0000001 && tri[1]>=-0.0000001 && tri[2]>=-0.0000001)
               || (tri[3]>=-0.0000001 && tri[4]>=-0.0000001 && tri[5]>=-0.0000001)) //perhaps this should be +ve??
          {
              inside = true;
              return_value = index_guess[i];
              break;
          }
        }
    } //end of for loop over i
        
    if(!inside) //Add the neighboring elements to the search
    {
        bool already_there;
	    vector<long> index_guess_old;
        index_guess_old.resize(index_guess.size());
        //for (long j=0; j!=(long)index_guess.size(); ++j)
        //    index_guess_old[j] = index_guess[j];
        index_guess_old = index_guess; //not quite sure this works
         //CLEAR OLD VALUES FROM INDEX_GUESS AT THIS POINT? 
	    for (long j=0; j!=(long)index_guess_old.size(); ++j)
        {

            elem = m_msh->ele_vector[index_guess_old[j]];
            elem->GetNeighbors(neighbors);
            num_face = elem->GetFacesNumber();
            
            for(long k =0; k!=num_face; ++k)
            {
                if(neighbors[k]->GetIndex() >= 0){
                already_there = false;
            	for(long l=0; l!=(long)index_guess.size(); ++l)
            	{
                    if(index_guess[l] == neighbors[k]->GetIndex())
        			already_there = true;
	            }
        		if(!already_there)
                    index_guess.push_back(neighbors[k]->GetIndex());
                }
    		}
    		   
        }                
    } // end of if !inside
   
    } //end of while
    //----------------------------------------------which element is point in?------------------------------------------------------END

    if(count<=number_of_elem)
        return return_value;
    else
    {
        cout<<"Error 1 in ELEWhatElemIsPointIn. Starting element: "<<index<<"\n";
        abort();
    }
}

#endif

/**************************************************************************
MSHLib-Method: 
07/2007 OK Implementation
**************************************************************************/
bool CompleteMesh(string pcs_name)
{
  bool succeed = false;
  for(int i=0;i<(int)fem_msh_vector.size(); i++)
  {
    if(fem_msh_vector[i]->pcs_name.compare(pcs_name)==0)
    {
      fem_msh_vector[i]->ConstructGrid();
      fem_msh_vector[i]->FillTransformMatrix();
      succeed = true;
    }
  }   
  return succeed;
}
#ifdef MFC //WW
/**************************************************************************
MSHLib-Method: 
07/2007 OK Implementation
**************************************************************************/
void MSHConfig()
{
  CompleteMesh(); //WW
  MSHTestMATGroups(); //OK Test MSH-MMP
  ConfigSolverProperties();
  //ConfigTopology(); // max_dim for solver, elements to nodes relationships
}
#endif

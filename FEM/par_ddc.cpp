/**************************************************************************
PARLib - Object:
Task: 
Programing:
07/2004 OK Implementation
07/2004 OK Version 1 untill 3.9.17OK6
07/2004 OK Version 2 from   3.9.17OK7
last modified:
**************************************************************************/

#include "stdafx.h" /* MFC */

#include <math.h>
// C++ STL
#include <iostream>
using namespace std;

#include "par_ddc.h"

// FEM-Makros
#include "makros.h"
#include "cel_asm.h" // to removed
extern void MakeElementEntryEQS_ASM(long,double*,double*,CPARDomain*,CRFProcess*);
#include "elements.h"
#include "matrix.h"
#include "geo_strings.h"
#include "rf_num_new.h"
#include "gs_project.h"

vector<CPARDomain*>dom_vector;

/**************************************************************************
STRLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
bool KeywordFound(string line)
{
  string hash("#");
  if(line.find(hash)!=string::npos)
    return true;
  else
    return false;
}

/**************************************************************************
STRLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
bool SubKeywordFound(string line)
{
  string dollar("$");
  if(line.find(dollar)!=string::npos)
    return true;
  else
    return false;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
07/2004 OK Version 1 untill 3.9.17OK6
07/2004 OK Version 2 from   3.9.17OK7
10/2005 OK cout
**************************************************************************/
void DOMRead(string file_base_name)
{
  CPARDomain *m_dom = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  string ddc_file_name;
  ios::pos_type position;
  //----------------------------------------------------------------------
  // File handling
  ddc_file_name = file_base_name + DDC_FILE_EXTENSION;
  ifstream ddc_file (ddc_file_name.data(),ios::in);
  if (!ddc_file.good()) return;
  ddc_file.seekg(0L,ios::beg);
  //----------------------------------------------------------------------
  cout << "DOMRead: ";
  //----------------------------------------------------------------------
  // Keyword loop
  while (!ddc_file.eof()) {
    ddc_file.getline(line,MAX_ZEILE);
    line_string = line;
    //----------------------------------------------------------------------
    if(line_string.find("#DOMAIN")!=string::npos) { // keyword found
      m_dom = new CPARDomain();
      position = m_dom->Read(&ddc_file);
      dom_vector.push_back(m_dom);
      ddc_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  //----------------------------------------------------------------------
  cout << (int)dom_vector.size() << " domains" << endl;
  //----------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
void DOMCreate()
{
  CPARDomain *m_dom = NULL;
  int i;
  int no_domains =(int)dom_vector.size();
  //----------------------------------------------------------------------
  // Create domain nodes
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
    m_dom->m_msh = fem_msh_vector[0]; //OK:ToDo
  }
  //----------------------------------------------------------------------
  // Create domain nodes
cout << "Create domain nodes" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
cout << "Domain:" << m_dom->ID << endl;
    m_dom->CreateNodes();
  }
  //----------------------------------------------------------------------
  // Create domain elements
cout << "Create domain elements" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
cout << "Domain:" << m_dom->ID << endl;
    m_dom->CreateElements();
  }
  //----------------------------------------------------------------------
  // Create domain EQS
cout << "Create domain EQS" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
cout << "Domain:" << m_dom->ID << endl;
    m_dom->CreateEQS();
  }
  //----------------------------------------------------------------------
}


//////////////////////////////////////////////////////////////////////////
// CPARDomain

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
CPARDomain::CPARDomain(void)
{
  ID =(int)dom_vector.size();
}

CPARDomain::~CPARDomain(void)
{
  elements.clear();
  nodes.clear();
  nodes_inner.clear();
  nodes_halo.clear();
}

/**************************************************************************
FEMLib-Method: 
Task: ST read function
Programing:
07/2004 OK Implementation
07/2004 OK Version 1 untill 3.9.17OK6
07/2004 OK Version 2 from   3.9.17OK7
**************************************************************************/
ios::pos_type CPARDomain::Read(ifstream *ddc_file)
{
  char line[MAX_ZEILE];
  string sub_line;
  string sub_string;
  string cut_string;
  string line_string;
  string delimiter(" ");
  string delimiter_type(";");
  bool new_subkeyword = false;
  string dollar("$");
  bool new_keyword = false;
  string hash("#");
  ios::pos_type position;
  long i;
  CElem* m_ele = NULL;
  //======================================================================
  while (!new_keyword) {
    position = ddc_file->tellg();
    ddc_file->getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    //....................................................................
    if(line_string.find("$ELEMENTS")!=string::npos) { // subkeyword found
      while ((!new_keyword)&&(!new_subkeyword)) {
        position = ddc_file->tellg();
        ddc_file->getline(line,MAX_ZEILE);
        line_string = line;
        if(line_string.find(hash)!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
        i = strtol(line,NULL,0);
        m_ele = new CElem(); //OK
        //OK m_ele->global_number = i;
        elements.push_back(m_ele);
      }
    }
    //....................................................................
    if(line_string.find("$NODES_INNER")!=string::npos) { // subkeyword found
      while (!new_keyword) {
        position = ddc_file->tellg();
        ddc_file->getline(line,MAX_ZEILE);
        line_string = line;
        if(line_string.find(hash)!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
        i = strtol(line,NULL,0);
        nodes_inner.push_back(i);
      }
    }
    //....................................................................
    if(line_string.find("$NODES_INNER")!=string::npos) { // subkeyword found
      while (!new_keyword) {
        position = ddc_file->tellg();
        ddc_file->getline(line,MAX_ZEILE);
        line_string = line;
        if(line_string.find(hash)!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
        i = strtol(line,NULL,0);
        nodes_inner.push_back(i);
      }
    }
    //....................................................................
    if(line_string.find("$NODES_BORDER")!=string::npos) { // subkeyword found
      while (!new_keyword) {
        position = ddc_file->tellg();
        ddc_file->getline(line,MAX_ZEILE);
        line_string = line;
        if(line_string.find(hash)!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find(dollar)!=string::npos) {
          new_subkeyword = true;
          break;
        }
        i = strtol(line,NULL,0);
        nodes_halo.push_back(i);
      }
    }
    //....................................................................
  }
  //======================================================================
  return position;
}

/**************************************************************************
PARLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::CreateNodes()
{
  long j,k;
  long no_nodes_halo, no_nodes_inner;
  //----------------------------------------------------------------------
  no_nodes_inner = (long)nodes_inner.size();
  for(j=0;j<no_nodes_inner;j++){
    k = nodes_inner[j];
    nodes.push_back(k);
//cout << nodes[j] << endl;
  }
//cout << "---" << endl;
  no_nodes_halo = (long)nodes_halo.size();
  for(j=0;j<no_nodes_halo;j++){
    k = nodes_halo[j];
    nodes.push_back(k);
//cout << nodes[no_nodes_inner+j] << endl;
  }
  //----------------------------------------------------------------------
}

/**************************************************************************
PARLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::CreateElements()
{
#ifdef PARALLEL
  //----------------------------------------------------------------------
  if(!m_msh)
    return;
  //----------------------------------------------------------------------
  long i;
  int j;
  Mesh_Group::CElem* m_ele = NULL;
  Mesh_Group::CNode* m_nod = NULL;
  //----------------------------------------------------------------------
  for(i=0;i<(long)elements.size();i++){
    if(elements[i]>(long)m_msh->ele_vector.size()){
      cout << "Warning: no ELE data" << '\n';
      continue;
    }
    m_ele = m_msh->ele_vector[elements[i]];
cout << i << " " << elements[i] << ": ";
    for(j=0;j<m_ele->GetNodesNumber(false);j++){
      m_nod = m_ele->GetNode(j);
      m_ele->domain_nodes[j] = GetDOMNode(m_nod->GetEquationIndex());
cout << m_nod->GetEquationIndex() << "->"  << m_ele->domain_nodes[j] << " ";
    }
cout << endl;
  }
  //----------------------------------------------------------------------
#endif
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
long CPARDomain::GetDOMNode(long global_node)
{
  long i;
  long no_nodes = (long)nodes.size();
  for(i=0;i<no_nodes;i++){
    if(nodes[i]==global_node)
      return i;
  }
  return -1;
}

/**************************************************************************
PARLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::CreateEQS()
{
  long no_nodes = (long)nodes.size();
  eqs = CreateLinearSolver(0,no_nodes);
  //InitializeLinearSolver(m_dom->eqs,NULL,NULL,NULL,m_dom->lsp_name);
  InitLinearSolver(eqs);
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::CalcElementMatrices(CRFProcess* m_pcs)
{
  m_pcs = m_pcs;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::AssembleMatrix(CRFProcess* m_pcs)
{
  long i;
  //----------------------------------------------------------------------
  SetZeroLinearSolver(eqs);
//MXDumpGLS("AssembleMatrix1.txt",1,eqs->b,eqs->x);
  //----------------------------------------------------------------------
  long no_elements = (long)elements.size();
  for(i=0;i<no_elements;i++){
    // virtual function PCSAssembleMatrix(i)
    //MakeElementEntryEQS_ASM(elements[i]->global_number,eqs->b,NULL,this);
    MakeElementEntryEQS_ASM(i,eqs->b,NULL,this,m_pcs);
//MXDumpGLS("AssembleMatrix1.txt",1,eqs->b,eqs->x);
  }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::SolveEQS(void)
{
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
**************************************************************************/
void CPARDomain::WriteMatrix()
{
  //----------------------------------------------------------------------
  // File handling
  string dom_file_name("egs.txt");
  fstream dom_file(dom_file_name.data(),ios::out);
  dom_file.setf(ios::scientific,ios::floatfield);
  dom_file.precision(3);
  if (!dom_file.good()) return;
  dom_file.seekg(0L,ios::beg); // rewind?
  //----------------------------------------------------------------------
  // File handling
  long i,j;
  long no_nodes = (long)nodes.size();
  for(i=0;i<no_nodes;i++){
    for(j=0;j<no_nodes;j++){
      dom_file << MXGet(i,j) << " " ;
    }
    dom_file << endl;
  }
  dom_file.close();
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
bool NodeExists(long node,vector<long>node_vector)
{
  long i;
  long no_nodes = (long)node_vector.size();
  for(i=0;i<no_nodes;i++){
    if(node==node_vector[i])
      return true;
  }
  return false;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
01/2005 OK Implementation
09/2005 OK MSH
last modification:
**************************************************************************/
void CPARDomain::WriteTecplot(string msh_name)
{
  long i;
  string element_type;
  //----------------------------------------------------------------------
  // GSP
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(!m_gsp){
#ifdef MFC
    AfxMessageBox("Error: No MSH member");
#endif
    return;
  }
  //--------------------------------------------------------------------
  // file handling
  char name[10];
  sprintf(name,"%i",ID);
  string dom_name = "DOMAIN";
  dom_name += name;
  string dom_file_name = m_gsp->path + dom_name + TEC_FILE_EXTENSION;
  fstream dom_file (dom_file_name.data(),ios::trunc|ios::out);
  dom_file.setf(ios::scientific,ios::floatfield);
  dom_file.precision(12);
  //--------------------------------------------------------------------
  // MSH
  CFEMesh* m_msh = NULL;
  CNode* m_nod = NULL;
  CElem* m_ele = NULL;
  m_msh = FEMGet(msh_name);
  if(!m_msh)
    return;
  //--------------------------------------------------------------------
  if (!dom_file.good()) return;
  dom_file.seekg(0L,ios::beg);
  //--------------------------------------------------------------------
  for(i=0;i<(long)elements.size();i++){
    m_ele = m_msh->ele_vector[elements[i]->GetIndex()]; //OK
    if(!m_ele)
      continue;
    switch(m_ele->GetElementType()){
      case 1:
        element_type = "ET = QUADRILATERAL";
        break;
      case 2:
        element_type = "ET = QUADRILATERAL";
        break;
      case 3:
        element_type = "ET = BRICK";
        break;
      case 4:
        element_type = "ET = TRIANGLE";
        break;
      case 5:
        element_type = "ET = TETRAHEDRON";
        break;
      case 6:
        element_type = "ET = BRICK";
      break;
    }
  }
  //--------------------------------------------------------------------
  dom_file << "VARIABLES = X,Y,Z,DOM" << endl;
  long no_nodes = (long)m_msh->nod_vector.size();
  dom_file << "ZONE T = " << dom_name << ", " \
           << "N = " << no_nodes << ", " \
           << "E = " << (long)elements.size() << ", " \
           << "F = FEPOINT" << ", " << element_type << endl;
  //......................................................................
  for(i=0;i<no_nodes;i++) {
    m_nod = m_msh->nod_vector[i];
    dom_file \
      << m_nod->X() << " " << m_nod->Y() << " " << m_nod->Z() << " " \
      << ID << endl;
  }
  //......................................................................
  for(i=0;i<(long)elements.size();i++){
    m_ele = m_msh->ele_vector[elements[i]->GetIndex()]; //OK global_index
    if(!m_ele)
      continue;
    switch(m_ele->GetElementType()){
        case 1:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[0]+1 << endl;
            element_type = "ET = QUADRILATERAL";
            break;
          case 2:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[2]+1 << " " << m_ele->nodes_index[3]+1 << endl;
            element_type = "ET = QUADRILATERAL";
            break;
          case 3:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[2]+1 << " " << m_ele->nodes_index[3]+1 << " " \
              << m_ele->nodes_index[4]+1 << " " << m_ele->nodes_index[5]+1 << " " << m_ele->nodes_index[6]+1 << " " << m_ele->nodes_index[7]+1 << endl;
            element_type = "ET = BRICK";
            break;
          case 4:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[2]+1 << endl;
            element_type = "ET = TRIANGLE";
            break;
          case 5:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[2]+1 << " " << m_ele->nodes_index[3]+1 << endl;
            element_type = "ET = TETRAHEDRON";
            break;
          case 6:
            dom_file \
              << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[0]+1 << " " << m_ele->nodes_index[1]+1 << " " << m_ele->nodes_index[2]+1 << " " \
              << m_ele->nodes_index[3]+1 << " " << m_ele->nodes_index[3]+1 << " " << m_ele->nodes_index[4]+1 << " " << m_ele->nodes_index[5]+1 << endl;
            element_type = "ET = BRICK";
          break;
      }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
10/2005 OK Implementation
last modification:
**************************************************************************/
void DOMWriteTecplot(string msh_name)
{
  CPARDomain *m_dom = NULL;
  for(int i=0;i<(int)dom_vector.size();i++){
    m_dom = dom_vector[i];
    m_dom->WriteTecplot(msh_name);
  }
}


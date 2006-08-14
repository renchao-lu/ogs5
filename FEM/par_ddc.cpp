/**************************************************************************
PARLib - Object:
Task: 
Programing:
07/2004 OK Implementation
07/2004 OK Version 1 untill 3.9.17OK6
07/2004 OK Version 2 from   3.9.17OK7
07/2006 WW Local topology, High order nodes
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
vector<double> node_connected_doms; //This will be removed after sparse class is finished WW

//---- MPI Parallel --------------
#ifdef USE_MPI //WW
int size;
int myrank;
char t_fname[3];
double time_ele_paral;
#endif
//---- MPI Parallel --------------



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
  //----------------------------------------------------------------------
  cout << "DOMRead: ";
  //----------------------------------------------------------------------
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
  if(!ddc_file.good())
  {
    cout << "no DDC file" << endl;
    return;
  }
  ddc_file.seekg(0L,ios::beg);
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
05/2006 WW Fix bugs and add the case of DOF>1 
07/2006 WW Find nodes of all neighbors of each node 
**************************************************************************/
void DOMCreate(CRFProcess *m_pcs)
{
  int no_domains = (int)dom_vector.size();
  if(no_domains==0)
    return;
  CPARDomain *m_dom = NULL;
  bool quadr = false; //WW  
  int i;
  long j; 
  if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
    quadr = true;
  //----------------------------------------------------------------------
  // Create domain nodes
  cout << "->Create DOM" << endl;
  /* // Comment by WW
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
    m_dom->m_msh = fem_msh_vector[0]; //OK:ToDo
  }
  */

  //----------------------------------------------------------------------
  // Create domain nodes
  cout << "Create domain nodes" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
    m_dom->m_msh = m_pcs->m_msh;
    cout << "    Domain:" << m_dom->ID << endl;
    m_dom->CreateNodes();
  }
  //----------------------------------------------------------------------
  // Create domain elements
  cout << "  Create domain elements" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
    cout << "    Domain:" << m_dom->ID << endl;
    m_dom->CreateElements(quadr);
  }

  //Average of nodal Neumann BCs contributed by nodes from different domains
  long nsize = m_pcs->m_msh->GetNodesNumber(true);
  node_connected_doms.resize(nsize);
  for(j=0; j<nsize; j++)
    node_connected_doms[j] = 0.0;
  for(i=0;i<(int)dom_vector.size();i++)
  {
     m_dom = dom_vector[i];
     for(j=0;j<(long)m_dom->nodes.size();j++)
       node_connected_doms[m_dom->nodes[j]] += 1.0;
  }
  // Find nodes of all neighbors of each node. // WW
  FindNodesOnInterface(m_pcs->m_msh, quadr);
  // Local topology. WW
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
	m_dom->NodeConnectedNodes();
  }

  //----------------------------------------------------------------------
  // Create domain EQS
  cout << "  Create domain EQS" << endl;
  for(i=0;i<no_domains;i++){
    m_dom = dom_vector[i];
    cout << "    Domain:" << m_dom->ID << endl;
    m_dom->CreateEQS(m_pcs);
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
  for(int i=0; i<4; i++) //WW
    shift[i] = 0;
  quadratic = false; //WW
}

CPARDomain::~CPARDomain(void)
{
  elements.clear();
  nodes.clear();
  nodes_inner.clear();
  nodes_halo.clear();
  // WW
  for(long i=0; i< (long)element_nodes_dom.size(); i++)
  {
     delete element_nodes_dom[i];
     element_nodes_dom[i] = NULL; 
  }	  
  for(long i=0; i< (long)node_conneted_nodes.size(); i++)
  {
     delete node_conneted_nodes[i];
     node_conneted_nodes[i] = NULL; 
  }	  
  //
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
//  CElem* m_ele = NULL;
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
        elements.push_back(i);
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
  nnodes_dom = no_nodes_halo+no_nodes_inner; //WW
  nnodesHQ_dom = nnodes_dom;

  //----------------------------------------------------------------------
}

/**************************************************************************
PARLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
05/2006 WW Fix bugs and add the case of DOF>1 
**************************************************************************/
void CPARDomain::CreateElements(const bool quadr)
{
  //----------------------------------------------------------------------
  if(!m_msh)
    return;
  //----------------------------------------------------------------------
  long i,k;
  int j, nNodes, nNodesHQ;
  long *elem_nodes=NULL;
  bool done = false;
  Mesh_Group::CElem* m_ele = NULL;
  Mesh_Group::CNode* m_nod = NULL;

  //----------------------------------------------------------------------
  for(i=0;i<(long)elements.size();i++){
    if(elements[i]>(long)m_msh->ele_vector.size()){
      cout << "Warning: no ELE data" << '\n';
      continue;
    }
    m_ele = m_msh->ele_vector[elements[i]];
    nNodes = m_ele->GetNodesNumber(false); //WW
    nNodesHQ = m_ele->GetNodesNumber(quadr);
    // cout << i << " " << elements[i] << ": ";
    elem_nodes = new long[nNodesHQ]; //WW
    element_nodes_dom.push_back(elem_nodes); //WW
    for(j=0;j<nNodes;j++)
    {
      m_nod = m_ele->GetNode(j);
      for(k=0; k<(long)nodes.size(); k++)
      {
        if(nodes[k]==m_nod->GetIndex())
        {
           elem_nodes[j] = k;
           break;
        }
      }
    }
    //------------------WW 
    if(!quadr) continue; 
    for(j=nNodes;j<nNodesHQ;j++)
    {
      done = false;
      m_nod = m_ele->GetNode(j);
      for(k=nnodes_dom; k<(long)nodes.size(); k++)
      {
        if(nodes[k]==m_nod->GetIndex())
        {
           elem_nodes[j] = k;
           done = true;
           break;
        }
      }
      if(!done)
      {
          elem_nodes[j] = (long)nodes.size();         
          nodes.push_back(m_nod->GetIndex());
      }     
    }
    nnodesHQ_dom = (long) nodes.size();  
    //------------------WW 
    // cout << endl;
  }
  // 
  //----------------------------------------------------------------------
}


/**************************************************************************
MSHLib-Method:
Task:
Programing:
06/2006 WW Implementation
**************************************************************************/
void CPARDomain::NodeConnectedNodes()
{
  int k, i_buff;
  long i, j, long_buff;
  CNode* m_nod = NULL;
  vector<long> nodes2node;
  //----------------------------------------------------------------------
  for(i=0;i<(long)nodes.size();i++)
  {
     m_nod = m_msh->nod_vector[nodes[i]];
	 nodes2node.clear();
     for(k=0; k<(int)m_nod->connected_nodes.size(); k++)
	 { 
         long_buff = m_nod->connected_nodes[k];
         for(j=0;j<(long)nodes.size();j++)
		 {
            if(long_buff==nodes[j])
               nodes2node.push_back(j);
		 }         
     }
     i_buff = (int)nodes2node.size();
     long  *nodes_to_node = new long[i_buff];
     for(k=0; k<i_buff; k++)
        nodes_to_node[k] = nodes2node[k];
     node_conneted_nodes.push_back(nodes_to_node);
	 num_nodes2_node.push_back(i_buff);
  }
  //----------------------------------------------------------------------
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
05/2006 WW Fix bugs and add the case of DOF>1 
**************************************************************************/
void CPARDomain::CreateEQS(CRFProcess *m_pcs)
{
  long no_nodes = (long)nodes.size();
  int dof = m_pcs->GetPrimaryVNumber(); 

  if(m_pcs->type==4||m_pcs->type==41)
  {
     for(int i=0; i<m_pcs->GetPrimaryVNumber(); i++)
        shift[i] = i*nnodesHQ_dom;
     
  }
  if(m_pcs->type==4)
  {
     eqs = CreateLinearSolverDim(m_pcs->m_num->ls_storage_method,dof,dof*nnodesHQ_dom);
 //    InitializeLinearSolver(eqs,m_num);
  }
  else if(m_pcs->type==41)    
  {
     eqs = CreateLinearSolverDim(m_pcs->m_num->ls_storage_method,dof,dof*nnodesHQ_dom+nnodes_dom);  

  }
  else
    eqs = CreateLinearSolver(m_pcs->m_num->ls_storage_method, no_nodes); //WW.
//  eqs = CreateLinearSolver(0,no_nodes);
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
    m_ele = m_msh->ele_vector[elements[i]];
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
    m_ele = m_msh->ele_vector[elements[i]];
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

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2006 WW Implementation
last modification:
**************************************************************************/
void FindNodesOnInterface(CFEMesh *m_msh, bool quadr)
{
  // int k;
  long i, j, nnodes_gl, g_index, nnodes_l;
  long l_buff = 0, l_buff1 = 0;

  long *elem_nodes=NULL;
  //
  Mesh_Group::CElem* m_ele = NULL;
  Mesh_Group::CNode* m_nod = NULL;
  //
  CPARDomain *m_dom = NULL;
  vector<long> boundary_nodes;
  vector<long> boundary_nodes_HQ;
  vector<long> inner_nodes_HQ;
  vector<long> inner_nodes;
  vector<long> long_buffer;
  vector<long> bc_buffer;
  vector<long> dom_bc_buffer;
  vector<long> dom_bc_bufferHQ;


  nnodes_gl = m_msh->GetNodesNumber(true);
  nnodes_l = m_msh->GetNodesNumber(false);

  bc_buffer.resize(nnodes_gl);
  for(i=0;i<nnodes_gl;i++)
  {
     if(node_connected_doms[i]>1.0)
     {
        // Mapp BC entry-->global node array
        bc_buffer[i] = (long)long_buffer.size();   
        long_buffer.push_back(m_msh->nod_vector[i]->GetIndex());
#ifdef USE_MPI
        if(m_msh->nod_vector[i]->GetIndex()<m_msh-> GetNodesNumber(false))
           overlapped_entry_size = (long)long_buffer.size();
#endif
     }
     else 
         bc_buffer[i] = -1; 
  }
  
#ifdef USE_MPI
  overlapped_entry_sizeHQ = (long)long_buffer.size();
  overlapped_entry = new long[overlapped_entry_sizeHQ];
  for(i=0;i<overlapped_entry_sizeHQ;i++)
      overlapped_entry[i] = long_buffer[i];
#endif
 
  // Sort
#ifndef USE_MPI
   for(int k=0;k<(int)dom_vector.size();k++)
   {
     int myrank = k;
#endif
     m_dom = dom_vector[myrank];
     boundary_nodes.clear();     
     inner_nodes.clear();  
     boundary_nodes_HQ.clear();     
     inner_nodes_HQ.clear();  
     long_buffer.clear();
     long_buffer.resize((long)m_dom->nodes.size());
     dom_bc_buffer.clear();
     dom_bc_bufferHQ.clear();

     for(i=0;i<(long)m_dom->nodes.size();i++)
     {
         g_index = m_dom->nodes[i];
         if(node_connected_doms[ g_index]>1.0)
         {
            if(g_index>=nnodes_l)
            { 
               boundary_nodes_HQ.push_back(i);
               dom_bc_bufferHQ.push_back(bc_buffer[g_index]);
               long_buffer[i] = -(long)boundary_nodes_HQ.size()-nnodes_gl;    
            }
            else
            { 
               boundary_nodes.push_back(i);
               dom_bc_buffer.push_back(bc_buffer[g_index]);
               long_buffer[i] = -(long)boundary_nodes.size();    
            }
         }
         else
         {
            if(g_index>=nnodes_l)
            { 
               long_buffer[i] = (long)inner_nodes_HQ.size()+nnodes_gl;            
               inner_nodes_HQ.push_back(i);
            }
            else
            { 
               long_buffer[i] = (long)inner_nodes.size();            
               inner_nodes.push_back(i);
            }
         }
     }
     //
     m_dom->num_inner_nodes = (long)inner_nodes.size();
     m_dom->num_inner_nodesHQ = (long)inner_nodes_HQ.size();
     m_dom->num_boundary_nodes = (long)boundary_nodes.size();
     m_dom->num_boundary_nodesHQ = (long)boundary_nodes_HQ.size();
     // Sort for high order nodes
 

     m_dom->nodes_inner.clear();
     m_dom->nodes_halo.clear();
     for(i=0;i<m_dom->num_inner_nodes;i++)
        m_dom->nodes_inner.push_back(m_dom->nodes[inner_nodes[i]]);
     for(i=0;i<(long)inner_nodes_HQ.size();i++)
        m_dom->nodes_inner.push_back(m_dom->nodes[inner_nodes_HQ[i]]);
     //
     for(i=0;i<m_dom->num_boundary_nodes;i++)
         m_dom->nodes_halo.push_back(m_dom->nodes[boundary_nodes[i]]);
     for(i=0;i<(long)boundary_nodes_HQ.size();i++)
         m_dom->nodes_halo.push_back(m_dom->nodes[boundary_nodes_HQ[i]]);
     //     
     m_dom->nodes.clear();
     m_dom->nodes.resize(m_dom->nnodesHQ_dom);
     // First interior nodes, then interface nodes
     j=0;
     for(i=0;i<m_dom->num_inner_nodes;i++)
	 {
        m_dom->nodes[i] = m_dom->nodes_inner[i];
 //       m_dom->nodes_inner[i] = i;        
	 }
     j += m_dom->num_inner_nodes;
     for(i=0;i<m_dom->num_boundary_nodes;i++)
	 {
        m_dom->nodes[i+j] = m_dom->nodes_halo[i];
  //      m_dom->nodes_halo[i] = i+j; 
	 }
     j += m_dom->num_boundary_nodes;
     for(i=0;i<(long)inner_nodes_HQ.size();i++)
     {
        m_dom->nodes[i+j] = m_dom->nodes_inner[i+m_dom->num_inner_nodes];
   //     m_dom->nodes_inner[i+m_dom->num_inner_nodes] = i+j;
     }
     j += (long)inner_nodes_HQ.size();
     for(i=0;i<(long)boundary_nodes_HQ.size();i++)
	 {
         m_dom->nodes[i+j] = m_dom->nodes_halo[i+m_dom->num_boundary_nodes];
   //      m_dom->nodes_halo[i+m_dom->num_boundary_nodes] = i+j;
	 }
     
     for(i=0;i<(long)m_dom->nodes.size();i++)
     {
        l_buff = long_buffer[i];
        if(l_buff<0)  //interface nodes
        {
           if(-l_buff-nnodes_gl>0) //HQ nodes
             l_buff1 = m_dom->nnodes_dom+(long)inner_nodes_HQ.size()-l_buff-nnodes_gl-1;
		   else 
             l_buff1 = (long)inner_nodes.size()-l_buff-1;
//             l_buff1 = m_dom->num_inner_nodesHQ-l_buff-1;
        }
		else
        {
           if(l_buff-nnodes_gl>=0) //HQ nodes
             l_buff1 = m_dom->nnodes_dom +l_buff-nnodes_gl;
		   else 
             l_buff1 = l_buff;
            
        }
        long_buffer[i] = l_buff1;
     }

     m_dom->nodes_inner.clear();
     m_dom->nodes_halo.clear();  
     // Mapping the local index to global BC array, overlapped_entry.
     for(i=0;i<m_dom->num_boundary_nodes;i++)
         m_dom->nodes_halo.push_back(dom_bc_buffer[i]);
     for(i=0;i<(long)boundary_nodes_HQ.size();i++)
         m_dom->nodes_halo.push_back(dom_bc_bufferHQ[i]);
    
     //----------------------------------------------------------------------
     for(i=0;i<(long)m_dom->elements.size();i++)
     {
         m_ele = m_msh->ele_vector[m_dom->elements[i]];
         elem_nodes = m_dom->element_nodes_dom[i]; 
         for(j=0;j<m_ele->GetNodesNumber(quadr);j++)
         {
             l_buff = elem_nodes[j];
             elem_nodes[j]=long_buffer[l_buff];
         }
     }

#ifndef USE_MPI
   }
#endif  
}
//

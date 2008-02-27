/**************************************************************************
ROCKFLOW - Object: Process PCS
Programing:
02/2003 OK Implementation
  /2003 WW CRFProcessDeformation
11/2003 OK re-organized
07/2004 OK PCS2
02/2005 WW/OK Element Assemblier and output 
12/2007 WW Classes of sparse matrix (jagged diagonal storage) and linear solver 
           and parellelisation of them 
02/2008 PCH OpenMP parallelization for Lis matrix solver
**************************************************************************/

/*--------------------- MPI Parallel  -------------------*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
#include <mpi.h>
#endif
/*--------------------- MPI Parallel  -------------------*/

/*--------------------- OpenMP Parallel ------------------*/
#if defined(LIS)
#include "lis.h"
#include <omp.h>
#endif
/*--------------------- OpenMP Parallel ------------------*/

// MFC
#include "stdafx.h" 
#ifdef MFC
#include "afxpriv.h" // For WM_SETMESSAGESTRING
#endif
#include "makros.h"
// C
#include <malloc.h>
// C++
#include <iostream>
#include <iomanip>  //WW
//#include <algorithm> // header of transform. WW
// GEOLib
#include "geo_ply.h"
/*------------------------------------------------------------------------*/
/* MshLib */
#include "msh_nodes_rfi.h" //MB
#include "msh_elem.h"
#include "msh_lib.h"
/*-----------------------------------------------------------------------*/
/* Objects */
#include "rf_pcs.h"
#include "pcs_dm.h"
#include "files.h"    // FIL
#include "solver.h"    // ConfigRenumberProperties
#include "rf_st_new.h" // ST
#include "rf_bc_new.h" // ST
#include "rf_mmp_new.h" // MAT
#include "rf_ic_new.h"  // IC
#include "rf_tim_new.h"  // IC
#include "rfiter.h"   // ITE
#include "elements.h" // ELE
#include "fem_ele_std.h" // ELE
#include "msh_lib.h" // ELE
#include "nodes.h" 
#include "rf_tim_new.h"
#include "rf_out_new.h"
#include "rfmat_cp.h"
#include "rf_mfp_new.h" // MFP
#include "rf_num_new.h"
#include "gs_project.h"
#include "rf_fct.h"
#include "femlib.h"
/*-----------------------------------------------------------------------*/
/* Tools */
#include "matrix.h"
#include "mathlib.h"
#include "geo_strings.h"
#include "par_ddc.h"
#include "tools.h"
#include "rf_pcs.h"
#include "rfstring.h"
#ifdef GEM_REACT
// GEMS chemical solver
#include "rf_REACT_GEM.h"
REACT_GEM *m_vec_GEM;
#endif

// New EQS
#ifdef NEW_EQS
#include "equation_class.h"   
#endif

/*-------------------- ITPACKV    ---------------------------*/
extern void transM2toM6(void);
/*-------------------- ITPACKV    ---------------------------*/
/*-------------------- JAD    ---------------------------*/
extern void transM2toM5(void);
/*-------------------- JAD    ---------------------------*/
/*-----------------------------------------------------------------------*/
/* LOP */
#include "rf_apl.h" // Loop...
#include "loop_pcs.h"
extern VoidFuncVoid LOPCalcSecondaryVariables_USER;
//------------------------------------------------------------------------
// PCS
VoidXFuncVoidX PCSDestroyELEMatrices[PCS_NUMBER_MAX];
//------------------------------------------------------------------------
// Globals, to be checked
int pcs_no_fluid_phases = 0;
int pcs_no_components = 0;
bool pcs_monolithic_flow = false;
int dm_pcs_number = 0;
int pcs_deformation = 0;
int dm_number_of_primary_nvals = 2; 
bool show_onces_adp = true;
bool show_onces_mod = true;
bool show_onces_mod_flow = true;
bool show_onces_density = true;
int memory_opt = 0;
int problem_2d_plane_dm; 
int anz_nval = 0;
int anz_nval0 = 0; //WW
//
int size_eval=0; //WW

NvalInfo *nval_data = NULL;
int anz_eval = 0;
EvalInfo *eval_data = NULL;
string project_title("New project"); //OK41

//--------------------------------------------------------
// Coupling Flag. WW
bool T_Process = false;
bool H_Process = false;
bool M_Process = false;
bool RD_Process = false;
bool MH_Process = false; // MH monolithic scheme
bool MASS_TRANSPORT_Process = false;
bool FLUID_MOMENTUM_Process = false;
bool RANDOM_WALK_Process = false;
bool pcs_created = false;

namespace process{class CRFProcessDeformation;}
using process::CRFProcessDeformation;
using Mesh_Group::CNode;
using Mesh_Group::CElem;
using FiniteElement::ElementValue;
using Math_Group::vec;

#define noCHECK_EQS
#define noCHECK_ST_GROUP
#define noCHECK_BC_GROUP
//----------------------------------------------------------

/*************************************************************************
PCS2 - File structure
// Construction/destruction
CRFProcess::CRFProcess(void)
CRFProcess::~CRFProcess(void)
void CRFProcess::Create()
void PCSDestroyAllProcesses(void)
void PCSRead(string file_base_name)
ios::pos_type CRFProcess::Read(ifstream *pcs_file)
// Access to PCS
CRFProcess *CRFProcess::Get(string name)
CRFProcess *CRFProcess::GetProcessByFunctionName(char *name)
CRFProcess *CRFProcess::GetProcessByNumber(int number)
// Configuration 1 - NOD
void CRFProcess::Config(void)
void CRFProcess::ConfigNODValues(void)
void CRFProcess::CreateNODValues(void)
// Configuration 2 - ELE
void CRFProcess::ConfigELEValues(void)
void CRFProcess::CreateELEValues(void)
void CRFProcess::CreateELEGPValues(void)
// Configuration 3 - ELE matrices
void CRFProcess::CreateELEMatricesPointer(void)
??? void PCSConfigELEMatricesXXX(int pcs_type_number)
// Execution
double CRFProcess::Execute()
void CRFProcess::InitEQS()
void CRFProcess::CalculateElementMatrices(void) 
void CRFProcess::DomainDecomposition()
void CRFProcess::AssembleSystemMatrixNew(void) 
void CRFProcess::IncorporateBoundaryConditions(const double Scaling)
void CRFProcess::IncorporateSourceTerms(const double Scaling)
int CRFProcess::ExecuteLinearSolver(void)
// Specials
void PCSRestart()
void RelocateDeformationProcess(CRFProcess *m_pcs)
void CRFProcess::CreateFDMProcess()
void CRFProcess::PCSMoveNOD(void) 
string PCSProblemType()
// ReMove site
*************************************************************************/

//////////////////////////////////////////////////////////////////////////
// PCS vector
//////////////////////////////////////////////////////////////////////////
// It is better to have space between data type and data name. WW
vector<LINEAR_SOLVER *> PCS_Solver; //WW
vector<CRFProcess*> pcs_vector;
vector<double*> ele_val_vector; //PCH
vector<string> ele_val_name_vector; // PCH
template <class T> T *resize(T *array, size_t old_size, size_t new_size);
//////////////////////////////////////////////////////////////////////////
// Construction / destruction
//////////////////////////////////////////////////////////////////////////

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2003 OK Implementation
02/2005 WW Local elment assembly (all protected members)
last modified:
**************************************************************************/
CRFProcess::CRFProcess(void) 
           :fem(NULL), Write_Matrix(false), matrix_file(NULL), 
            Memory_Type(0), WriteSourceNBC_RHS(0) //WW  //WW
{
  TempArry = NULL;
//SB:GS4  pcs_component_number=0; //SB: counter for transport components
  pcs_component_number = pcs_no_components -1;
  //----------------------------------------------------------------------
  // NUM
  pcs_num_name[0] = NULL;
  pcs_num_name[1] = NULL;
  pcs_nonlinear_iterations = 1;
  pcs_nonlinear_iteration_tolerance = 1.0e8;
  pcs_coupling_iterations = 1;
  pcs_sol_name = NULL;
  m_num = NULL;
  cpl_type_name = "PARTITIONED"; //OK
  num_type_name = "FEM"; //OK
  eqs = NULL; //WW
  dof = 1; //WW
  //----------------------------------------------------------------------
  // ELE
  ConfigELEMatrices = NULL;
  pcs_number_of_evals = 0;
  NumDeactivated_SubDomains = 0;
  Deactivated_SubDomain = NULL;
  //----------------------------------------------------------------------
  // 
  mobile_nodes_flag = -1;
  //----------------------------------------------------------------------
  // USER
  PCSSetIC_USER = NULL;
  //----------------------------------------------------------------------
  // TIM
  tim_type_name = "TRANSIENT"; //OK
  time_unit_factor = 1.0;
  timebuffer = 1.0e-5;  //WW
  //pcs_type_name.empty();
  //----------------------------------------------------------------------
  // CPL
  for(int i=0; i<10; i++)
    Shift[i] = 0;
  selected = true; //OK
  // MSH OK
  m_msh = NULL;
  // Reload solutions
  reload=-1;
  pcs_nval_data = NULL;
  pcs_eval_data = NULL;
  non_linear = false; //OK/CMCD
  cal_integration_point_value = false; //WW
  continuum = 0;
  //adaption = false; JOD removed
  compute_domain_face_normal = false; //WW
  //
  additioanl2ndvar_print = -1; //WW
  //----------------------------------------------------------------------
  m_bCheck = false; //OK
  m_bCheckOBJ = false; //OK
  m_bCheckNOD = false; //OK
  m_bCheckELE = false; //OK
  m_bCheckEQS = false; //OK
  //
  write_boundary_condition = false; //15.01.2008. WW
#ifdef USE_MPI //WW
  cpu_time_assembly = 0;
#endif
  // New equation and solver WW
#ifdef NEW_EQS
  eqs_new = NULL;  
  configured_in_nonlinearloop = false;
#endif

}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2003 OK Implementation
01/2005 WW Local assemblier as a member
last modified:
**************************************************************************/
CRFProcess::~CRFProcess(void)
{
  long i;
  //----------------------------------------------------------------------
  // Finite element
  if(fem) delete fem; //WW
  fem = NULL;
  //----------------------------------------------------------------------
  // ELE: Element matrices
  ElementMatrix *eleMatrix = NULL;
  ElementValue* gp_ele = NULL;
  if(Ele_Matrices.size()>0)
  {
    for (i=0;i<(long)Ele_Matrices.size();i++)
    {
      eleMatrix = Ele_Matrices[i];
      delete eleMatrix;
      eleMatrix = NULL;
    }
    Ele_Matrices.clear();
  }
  //----------------------------------------------------------------------
  // ELE: Element Gauss point values
  if(ele_gp_value.size()>0)
  {
    for(i=0;i<(long)ele_gp_value.size();i++)
    {
      gp_ele = ele_gp_value[i];
      delete gp_ele;
      gp_ele = NULL;
    }
    ele_gp_value.clear();
  }
  //----------------------------------------------------------------------
  // OUT: Matrix output
  if(matrix_file)
  {
    matrix_file->close();
    delete matrix_file;
  }     
  //----------------------------------------------------------------------
  // NOD: Release memory of node values
  for(i=0;i<(int)nod_val_vector.size();i++)
  {
    delete [] nod_val_vector[i];  // Add []. WW
    nod_val_vector[i] = NULL;
  }
  nod_val_vector.clear();
  //----------------------------------------------------------------------
  // ST:
  CNodeValue* m_nod_val = NULL;
  for(i=0;i<(int)st_node_value.size();i++)
  {
    m_nod_val = st_node_value[i];
    //OK delete st_node_value[i];
    //OK st_node_value[i] = NULL;
    if(m_nod_val->check_me) //OK
    {
      m_nod_val->check_me = false;
      delete m_nod_val;
      m_nod_val = NULL;
    }
  }
  st_node_value.clear();
  //----------------------------------------------------------------------
  for(i=0; i<(int)bc_node_value.size(); i++)
  {
     delete bc_node_value[i];
     bc_node_value[i] = NULL;    
  }
  bc_node_value.clear();
  //----------------------------------------------------------------------
  //pcs_type_name.clear();
  //----------------------------------------------------------------------
  // CON
  continuum_vector.clear();
  //Haibing 13112006------------------------------------------------------
  for(i=0;i<(long)ele_val_vector.size();i++)
    delete[] ele_val_vector[i];
  ele_val_vector.clear();
  //----------------------------------------------------------------------
  if(Deactivated_SubDomain) //05.09.2007 WW
  {
     delete [] Deactivated_SubDomain;
     Deactivated_SubDomain = NULL;
  }
}

/**************************************************************************
FEMLib-Method:
Task:    Gauss point values for CFEMSH  
Programing:
08/2005 WW Implementation
**************************************************************************/
void CRFProcess::AllocateMemGPoint()
{
  if(pcs_type_name.find("FLOW")==0) return;
  long i; 
  CElem* elem = NULL;
  ElementValue* ele_GP=NULL;
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
  { 
     elem = m_msh->ele_vector[i];
     ele_GP = new ElementValue(this, elem);
     ele_gp_value.push_back(ele_GP);
  }

}

/**************************************************************************
FEMLib-Method:
Task:    This function is a part of the monolithic scheme
         and it is used to assign pcs name to IC, ST, BC, TIM and OUT. object 
Programing:
07/2005 WW Implementation
**************************************************************************/
void CRFProcess::SetOBJNames()
{
  int i; 
  //----------------------------------------------------------------------
  // IC
  CInitialCondition *m_ic = NULL;
  for(i=0;i<(long)ic_vector.size();i++){
    m_ic = ic_vector[i];
	m_ic->pcs_type_name = pcs_type_name;
  }
  //----------------------------------------------------------------------
  // BC
  CBoundaryCondition *m_bc = NULL;
  list<CBoundaryCondition*>::const_iterator p_bc = bc_list.begin();
  while(p_bc!=bc_list.end()) {
    m_bc = *p_bc;
	m_bc->pcs_type_name = pcs_type_name;
    ++p_bc;
  }
  //----------------------------------------------------------------------
  // ST
  CSourceTerm* m_st = NULL;
  for(i=0;i<(int)st_vector.size();i++){
    m_st = st_vector[i];
	m_st->pcs_type_name = pcs_type_name;
  }
  //----------------------------------------------------------------------
/*
  // TIM
  CTimeDiscretization *m_tim = NULL; 
  for(i=0;i<(int)time_vector.size();i++){
    m_tim = time_vector[i];
	m_tim->pcs_type_name = pcs_type_name;
  }
*/
  //----------------------------------------------------------------------
  // TIM
  for(i=0;i<(int)time_vector.size();i++){
    Tim = time_vector[i];
	Tim->pcs_type_name = pcs_type_name;
  }
  //----------------------------------------------------------------------
  // OUT
// OK4216
  COutput* m_out = NULL;
  for(i=0;i<(int)out_vector.size();i++){
    m_out = out_vector[i];
	  m_out->pcs_type_name = pcs_type_name;
    //m_out->pcs_pv_name = pcs_primary_function_name[0];//CMCD
    //string temp = pcs_primary_function_name[0];
  }
}

/**************************************************************************
PCSLib-Method:
10/2002 OK Implementation
04/2004 WW Modification for 3D problems 
02/2005 WW New fem calculator
04/2005 OK MSHCreateNOD2ELERelations
07/2005 WW Geometry element objects
02/2006 WW Removed memory leaking
01/2006 YD MMP for each PCS
04/2006 WW Unique linear solver for all processes if they share the same mesh
06/2006 WW Rearrange incorporation of BC and ST. Set BC and ST for domain decomposition  
last modified:
CREATE
**************************************************************************/
void CRFProcess::Create()
{
  int i=0;
  //WW  int phase;
  CRFProcess *m_pcs = NULL; //
  //----------------------------------------------------------------------
  // Element matrix output. WW
  if(Write_Matrix)
  {
     cout << "->Write Matrix" << '\n';
#if defined(USE_MPI)
     char stro[32];  
     sprintf(stro, "%d",myrank); 
     string m_file_name = FileName +"_"+pcs_type_name+(string)stro+"_element_matrix.txt";
#else
     string m_file_name = FileName +"_"+pcs_type_name+"_element_matrix.txt";
#endif
     matrix_file = new fstream(m_file_name.c_str(),ios::trunc|ios::out);
     if (!matrix_file->good())
       cout << "Warning in GlobalAssembly: Matrix files are not found" << endl;
  }
  //----------------------------------------------------------------------------
  if(m_msh) //OK->MB please shift to Config()
  {
    if(pcs_type_name.compare("GROUNDWATER_FLOW")==0)
      MSHDefineMobile(this);
  }
  //----------------------------------------------------------------------------
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable
  //----------------------------------------------------------------------------
  // MMP - create mmp groups for each process   //YD
  cout << "->Create MMP" << '\n';
  CMediumPropertiesGroup *m_mmp_group = NULL;
  int continua = 1; //WW
  if(RD_Process) continua = 2;
  for(i=0;i<continua;i++)
  {
    m_mmp_group = MMPGetGroup(pcs_type_name);
    if(!m_mmp_group) {
      m_mmp_group = new CMediumPropertiesGroup();
      m_mmp_group->pcs_type_name = pcs_type_name; 
      m_mmp_group->Set(this);
      mmp_group_list.push_back(m_mmp_group);
    }
  }
  //----------------------------------------------------------------------------
  // NUM_NEW
  cout << "->Create NUM" << '\n';
  int no_numerics = (int)num_vector.size();
  CNumerics *m_num_tmp = NULL;
  if(pcs_type_name.compare("RANDOM_WALK"))	// PCH RWPT does not need this.
  {
	for(i=0;i<no_numerics;i++){
		m_num_tmp = num_vector[i];

		if((pcs_type_name.compare(m_num_tmp->pcs_type_name)==0) 
		||(m_num_tmp->pcs_type_name.compare(pcs_primary_function_name[0])==0))
		{
			m_num = m_num_tmp;
			break;
		}
	}
  }
  if(!m_num){
    cout << "Warning in CRFProcess::Create() - no NUM data" << endl;
    m_num_tmp = new CNumerics(pcs_type_name); //OK
    m_num = m_num_tmp;
  }
  else{
    pcs_nonlinear_iterations = m_num->nls_max_iterations;
    pcs_nonlinear_iteration_tolerance = m_num->nls_error_tolerance;
    if(pcs_nonlinear_iterations>1) //WW
      non_linear = true; 
  }
  //----------------------------------------------------------------------------
  // EQS - create equation system
//WW CreateEQS();
  cout << "->Create EQS" << '\n';
#ifdef NEW_EQS
  for(i=0; i<(int)fem_msh_vector.size(); i++)
  {
    if(m_msh==fem_msh_vector[i])
      break;    
  }
  if(type==4||type==41)
    eqs_new = EQS_Vector[2*i+1];
  else
    eqs_new = EQS_Vector[2*i];  
#else  
  //WW  phase=1;
  // create EQS
  if(type==4)
  {
     eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF,DOF*m_msh->GetNodesNumber(true));
     InitializeLinearSolver(eqs,m_num);
     PCS_Solver.push_back(eqs); //WW
  }
  else if(type==41)    
  {
      if(num_type_name.find("EXCAVATION")!=string::npos)
         eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF-1,DOF*m_msh->GetNodesNumber(true));
      else
         eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF, 
		    (DOF-1)*m_msh->GetNodesNumber(true)+m_msh->GetNodesNumber(false));  
      InitializeLinearSolver(eqs,m_num);
      PCS_Solver.push_back(eqs); //WW
  }
  else
  {
    /////////////////////////////////////////////////////////////////////
    // If there is a solver exsiting. WW 
    m_pcs = NULL; 
    for(i=0; i<(int)pcs_vector.size(); i++)
	{
       m_pcs = pcs_vector[i];
       if(m_pcs&&m_pcs->eqs)
	   {
          if(m_pcs->pcs_type_name.find("DEFORMATION")==string::npos)
            break;
	   }
	}
    // If unique mesh
	if(m_pcs&&m_pcs->eqs&&(fem_msh_vector.size()==1))
       eqs = m_pcs->eqs;
    /////////////////////////////////////////////////////////////////////
	else
	{
       if(m_msh)
         eqs = CreateLinearSolver(m_num->ls_storage_method,m_msh->GetNodesNumber(false)*DOF);
       else
         eqs = CreateLinearSolver(m_num->ls_storage_method,NodeListLength);
       InitializeLinearSolver(eqs,m_num);
       PCS_Solver.push_back(eqs); 
	}
  }
#endif // If NEW_EQS
     // Set solver properties: EQS<->SOL
     // Internen Speicher allokieren
     // Speicher initialisieren

  //----------------------------------------------------------------------------
  // Time unit factor //WW
  cout << "->Create TIM" << '\n';
  //CTimeDiscretization* Tim = TIMGet(pcs_type_name);
    Tim = TIMGet(pcs_type_name);
  if(Tim){
    if(Tim->time_unit.find("MINUTE")!=string::npos) time_unit_factor=60.0;
    else if(Tim->time_unit.find("HOUR")!=string::npos) time_unit_factor=3600.0;
    else if(Tim->time_unit.find("DAY")!=string::npos) time_unit_factor=86400.0;
    else if(Tim->time_unit.find("MONTH")!=string::npos) time_unit_factor=2592000.0;
    else if(Tim->time_unit.find("YEAR")!=string::npos) time_unit_factor=31536000;
  }
  //----------------------------------------------------------------------------
  //
  if(type==4||type==41) m_msh->SwitchOnQuadraticNodes(true); 
  else  m_msh->SwitchOnQuadraticNodes(false); 
  //
  if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos) //WW
  {
     setBC_danymic_problems();
     setST_danymic_problems();
  }
  else
  {
     // BC - create BC groups for each process
     cout << "->Create BC" << '\n';
     CBoundaryConditionsGroup *m_bc_group = NULL;
     for(i=0;i<DOF;i++){
       //OKm_bc_group = BCGetGroup(pcs_type_name,pcs_primary_function_name[i]);
       //OKif(!m_bc_group){
         BCGroupDelete(pcs_type_name,pcs_primary_function_name[i]);
         m_bc_group = new CBoundaryConditionsGroup();
         m_bc_group->pcs_type_name = pcs_type_name; //OK
         m_bc_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
         m_bc_group->Set(this,Shift[i]);
         bc_group_list.push_back(m_bc_group); //Useless, to be removed. WW
       //OK}
     } 
     if((long)bc_node_value.size()<1) //WW
        cout << "Warning: no boundary conditions specified for " << pcs_type_name << endl;
     //----------------------------------------------------------------------------
     // ST - create ST groups for each process
     cout << "->Create ST" << '\n';
     CSourceTermGroup *m_st_group = NULL;

     if(WriteSourceNBC_RHS==2) // Read from file
        ReadRHS_of_ST_NeumannBC();
     else // WW
     {  // Calculate directly
       for(i=0;i<DOF;i++)
       {
         //OK m_st_group = m_st_group->Get(pcs_primary_function_name[i]);
         m_st_group = STGetGroup(pcs_type_name,pcs_primary_function_name[i]);
         if(!m_st_group) {
           m_st_group = new CSourceTermGroup();
           m_st_group->pcs_type_name = pcs_type_name; //OK
           m_st_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
           m_st_group->Set(this,Shift[i]);
           st_group_list.push_back(m_st_group); //Useless, to be removed. WW
         }
       }
       if(WriteSourceNBC_RHS==1)// WW
          WriteRHS_of_ST_NeumannBC();
     }
  }
  // Write BC/ST nodes for vsualization.WW
  if(write_boundary_condition)
    WriteBC();
  //----------------------------------------------------------------------------
  // ELE - config and create element values
  cout << "->Config ELE values" << '\n';
  AllocateMemGPoint();
  //----------------------------------------------------------------------------
  // ELE - config element matrices
  //-------------------------------------------------------
  //----------------------------------------------------------------------------
  // NOD - config and create node values
  cout << "->Config NOD values" << '\n';
  double* nod_values = NULL;
  double* ele_values = NULL;    // PCH
  long j;
//  size_t size;
 
  number_of_nvals = 2*DOF + pcs_number_of_secondary_nvals;
  for(i=0;i<pcs_number_of_primary_nvals;i++)
  {
    nod_val_name_vector.push_back(pcs_primary_function_name[i]); // new time
    nod_val_name_vector.push_back(pcs_primary_function_name[i]); // old time //need this MB!
  }
  for(i=0;i<pcs_number_of_secondary_nvals;i++)
    nod_val_name_vector.push_back(pcs_secondary_function_name[i]); // new time
  //
  long m_msh_nod_vector_size = m_msh->NodesNumber_Quadratic;
  for(j=0;j<m_msh_nod_vector_size;j++)
  {
     nod_values =  new double[number_of_nvals];
     for(i=0;i<number_of_nvals;i++) nod_values[i] = 0.0;
        nod_val_vector.push_back(nod_values);
  } 
  // Create element values - PCH
  int number_of_evals = 2*pcs_number_of_evals;  //PCH, increase memory
  if(number_of_evals>0) // WW added this "if" condition
  {
     for(i=0;i<pcs_number_of_evals;i++)
     {
       ele_val_name_vector.push_back(pcs_eval_name[i]); // new time
       ele_val_name_vector.push_back(pcs_eval_name[i]); // old time
     }
     long m_msh_ele_vector_size = (long)m_msh->ele_vector.size();
     if(ele_val_vector.size()==0)
     {
       for(j=0;j<m_msh_ele_vector_size;j++)
       {
         ele_values =  new double[number_of_evals];
         size_eval += number_of_evals; //WW
         for(i=0;i<number_of_evals;i++) 
           ele_values[i] = 0.0;
           ele_val_vector.push_back(ele_values);
         }
      } 
      else
      {
        for(j=0;j<m_msh_ele_vector_size;j++){
          ele_values = ele_val_vector[j];
/* //Comment by WW
#ifndef SX
#ifdef GCC
            size = malloc_usable_size( ele_values )/sizeof(double); 
#elif HORIZON
	    //KG44: malloc_usable_size and _msize are not available
#else 
            size= _msize( ele_values )/sizeof(double);
#endif
#endif
*/
          ele_values = resize(ele_values, size_eval, size_eval+ number_of_evals);
          size_eval += number_of_evals; 
          ele_val_vector[j] = ele_values;
        }
     }
  }
  // 
  if(reload>=2&&type!=4&&type!=41) 
     ReadSolution(); //WW
  //----------------------------------------------------------------------------
  // IC
  cout << "->Assign IC" << '\n';
  //
  SetIC();
  //
  if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos) //WW
     setIC_danymic_problems();

  //----------------------------------------------------------------------------
  // Keep all local matrices in the memory
  if(type != 55) //Not for fluid momentum. WW
  {
    if(Memory_Type!=0)  
      AllocateLocalMatrixMemory();
    if(type==4||type==41)
    {
      // Set initialization function
      CRFProcessDeformation *dm_pcs = (CRFProcessDeformation *) this;
      dm_pcs->Initialization(); 
    }
    else  // Initialize FEM calculator
    {
       int Axisymm = 1; // ani-axisymmetry
       if(m_msh->isAxisymmetry()) Axisymm = -1; // Axisymmetry is true
       fem = new CFiniteElementStd(this, Axisymm*m_msh->GetCoordinateFlag()); 
       fem->SetGaussPointNumber(m_num->ele_gauss_points);
    }
  }
  //----------------------------------------------------------------------
  // Initialize the system equations 
  if(PCSSetIC_USER)
    PCSSetIC_USER(pcs_type_number);
  //
  CalcSecondaryVariables(true); //WW
  //
  if(compute_domain_face_normal) //WW
     m_msh->FaceNormal();        
}
/**************************************************************************
FEMLib-Method:
Task: Write the contribution of ST or Neumann BC to RHS to a file after
      integration
Programing:
12/2005 WW 
03/2006 WW Write as acsi
04/2006 WW 
last modified:
**************************************************************************/
inline void CRFProcess::WriteRHS_of_ST_NeumannBC()
{
     string m_file_name = FileName +"_"+pcs_type_name+"_ST_RHS.asc";
     ofstream os(m_file_name.c_str(), ios::trunc|ios::out);     	
     if (!os.good())
     {
         cout << "Failure to open file: "<<m_file_name << endl;
         abort();
     }

	/*
    CSourceTermGroup *m_st_group = NULL;
    list<CSourceTermGroup*>::const_iterator p_st_group = st_group_list.begin();
    while(p_st_group!=st_group_list.end()) {
      m_st_group = *p_st_group;
	  m_st_group->Write(os);
      ++p_st_group;
    }
	*/
    os<<"$PCS_TYPE  "<<endl;
    //string s_buffer;
    //s_buffer = pcs_type_name;
    //transform(s_buffer.begin(), s_buffer.end(), s_buffer.begin(), toupper);
    os<<pcs_type_name.c_str()<<endl;
    os<<"geo_node_number  ";
    os<<"msh_node_number  ";
	os<<"CurveIndex ";
    os<<"node_value ";
    /*
    // This is for river flow
    // This writing will be valid for river flow when some 
    // of its parameters being moved from CSourceTerm to here
    os<<"node_distype ";
    os<<"node_area ";
    os<<"node_parameterA ";
    os<<"node_parameterB ";
    os<<"node_parameterC ";
    os<<"node_parameterD ";
    os<<"node_parameterE ";
    os<<"conditional ";
    */
    os<<endl;
    os.setf(ios::scientific,ios::floatfield);
  	os.precision(14);
    os<<(long)st_node_value.size()<<endl;
    for(long i=0; i<(long)st_node_value.size(); i++)
      st_node_value[i]->Write(os); 
    os.close();
}

/**************************************************************************
FEMLib-Method:
Task: Write the contribution of ST or Neumann BC to RHS to a file after
      integration
Programing:
03/2006 WW 
last modified: 04/2006
**************************************************************************/
inline void CRFProcess::ReadRHS_of_ST_NeumannBC()
{
    long Size;
    string m_file_name = FileName +"_"+pcs_type_name+"_ST_RHS.asc";
    ifstream is(m_file_name.c_str(), ios::in);     
    if (!is.good())
    {
        cout << "File "<<m_file_name<<" is not found" << endl;
        abort();
    }

    string s_buffer;
    getline(is, s_buffer);
	getline(is, s_buffer);
	getline(is, s_buffer);
    is>>Size>>ws;
    st_node_value.clear();
    for(long i=0; i<Size; i++)
	{
       CNodeValue *cnodev = new CNodeValue();
	   cnodev->Read(is);
       st_node_value.push_back(cnodev); 
	}
    is.close();
}
/**************************************************************************
FEMLib-Method:
Task: Write the solution
Programing:
04/2006 WW 
last modified:
**************************************************************************/
void CRFProcess:: WriteSolution()
{
    if(reload==2||reload<0) return;	 
    string m_file_name = FileName +"_"+pcs_type_name+"_primary_value.asc";
    ofstream os(m_file_name.c_str(), ios::trunc|ios::out);     	
    if (!os.good())
    {
       cout << "Failure to open file: "<<m_file_name << endl;
       abort();
    }
    //
    long i;
    int j;
    int idx[20];
	for(j=0; j<pcs_number_of_primary_nvals; j++) 
	{
      idx[j] = GetNodeValueIndex(pcs_primary_function_name[j]);
      idx[j+pcs_number_of_primary_nvals] = idx[j]+1;
	}
	for(i=0; i<m_msh->GetNodesNumber(false); i++)
    {
       for(j=0; j<2*pcs_number_of_primary_nvals; j++)
         os<<GetNodeValue(i,idx[j]) <<"  ";
       os<<endl;
    }  
    os.close();
}

/**************************************************************************
FEMLib-Method:
Task: Write the solution
Programing:
04/2006 WW 
last modified:
**************************************************************************/
void CRFProcess:: ReadSolution()
{
    string m_file_name = FileName +"_"+pcs_type_name+"_primary_value.asc";
    ifstream is(m_file_name.c_str(), ios::in);     	
    if (!is.good())
    {
       cout << "Failure to open file: "<<m_file_name << endl;
       abort();
    }
    //
    long i;
    int j;
    int idx[20];
    double val[20];
	for(j=0; j<pcs_number_of_primary_nvals; j++) 
	{
       idx[j] = GetNodeValueIndex(pcs_primary_function_name[j]);
       idx[j+pcs_number_of_primary_nvals] = idx[j]+1;
	}
	for(i=0; i<m_msh->GetNodesNumber(false); i++)
    {
       for(j=0; j<2*pcs_number_of_primary_nvals; j++)
         is>>val[j];
       is>>ws;
       for(j=0; j<2*pcs_number_of_primary_nvals; j++)
         SetNodeValue(i,idx[j], val[j]);
    }  
    is.close();
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 WW Set coupling data
last modified:
**************************************************************************/
void CRFProcess::setIC_danymic_problems()
{
  char *function_name[7];
  int i, j, nv;
  nv = 0; 
  if(max_dim==1) // 2D
  {
    nv = 5;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "VELOCITY_DM_X";
    function_name[3] = "VELOCITY_DM_Y";
    function_name[4] = "PRESSURE1"; 
  }
  else //3D
  {
    nv =7;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "DISPLACEMENT_Z1";
    function_name[3] = "VELOCITY_DM_X";
    function_name[4] = "VELOCITY_DM_Y";
    function_name[5] = "VELOCITY_DM_Z";
    function_name[6] = "PRESSURE1"; 
  }

  CInitialCondition *m_ic = NULL;
  long no_ics =(long)ic_vector.size();
  int nidx;
  for(i=0;i<nv;i++){
    nidx = GetNodeValueIndex(function_name[i]);
    for(j=0;j<no_ics;j++){
      m_ic = ic_vector[j];
      if(m_ic->pcs_pv_name.compare(function_name[i])==0)
        m_ic->Set(nidx);
    }
  }

}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 WW Set coupling data
last modified:
**************************************************************************/
void CRFProcess::setST_danymic_problems()
{
  char *function_name[7];
  int i, nv;
  nv = 0; 
  if(max_dim==1) // 2D
  {
    nv = 5;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "VELOCITY_DM_X";
    function_name[3] = "VELOCITY_DM_Y";
    function_name[4] = "PRESSURE1"; 
  }
  else //3D
  {
    nv =7;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "DISPLACEMENT_Z1";
    function_name[3] = "VELOCITY_DM_X";
    function_name[4] = "VELOCITY_DM_Y";
    function_name[5] = "VELOCITY_DM_Z";
    function_name[6] = "PRESSURE1"; 
  }

  // ST - create ST groups for each process
  CSourceTermGroup *m_st_group = NULL;
  for(i=0;i<nv;i++)
  {
     m_st_group = STGetGroup(pcs_type_name,function_name[i]);
     if(!m_st_group) {
        m_st_group = new CSourceTermGroup();
        m_st_group->pcs_type_name = pcs_type_name; 
        m_st_group->pcs_pv_name = function_name[i]; 
        m_st_group->Set(this,Shift[i], function_name[i]);
        st_group_list.push_back(m_st_group); //Useless, to be removed. WW
     }
  }


}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 WW Set coupling data
last modified:
**************************************************************************/
void CRFProcess::setBC_danymic_problems()
{
  char *function_name[7];
  int i, nv;
  nv = 0; 
  if(max_dim==1) // 2D
  {
    nv = 5;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "VELOCITY_DM_X";
    function_name[3] = "VELOCITY_DM_Y";
    function_name[4] = "PRESSURE1"; 
  }
  else //3D
  {
    nv =7;
    function_name[0] = "DISPLACEMENT_X1";
    function_name[1] = "DISPLACEMENT_Y1";
    function_name[2] = "DISPLACEMENT_Z1";
    function_name[3] = "VELOCITY_DM_X";
    function_name[4] = "VELOCITY_DM_Y";
    function_name[5] = "VELOCITY_DM_Z";
    function_name[6] = "PRESSURE1"; 
  }

  cout << "->Create BC" << '\n';
  CBoundaryConditionsGroup *m_bc_group = NULL;
  for(i=0;i<nv;i++){
    BCGroupDelete(pcs_type_name,function_name[i]);
    m_bc_group = new CBoundaryConditionsGroup();
    m_bc_group->pcs_type_name = pcs_type_name; //OK
    m_bc_group->pcs_pv_name = function_name[i]; //OK
    m_bc_group->Set(this,Shift[i],function_name[i]);
    bc_group_list.push_back(m_bc_group); //Useless, to be removed. WW
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
02/2005 WW Set coupling data
last modified:
**************************************************************************/
void CRFProcess::ConfigureCouplingForLocalAssemblier()
{
   bool Dyn = false;
   if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos)
     Dyn = true;
   if(fem)
	   fem->ConfigureCoupling(this, Shift, Dyn);
}
/**************************************************************************
FEMLib-Method:
06/2003 OK Implementation
        WW 2nd version, PCS_Solver
**************************************************************************/
void PCSDestroyAllProcesses(void)
{
  CRFProcess *m_process = NULL;
  long i;
  int j;
  //----------------------------------------------------------------------
#ifdef NEW_EQS //WW
 #if defined(USE_MPI)
  for(j=0;j<(int)EQS_Vector.size();j+=2) //WW
 #else 
  for(j=0;j<(int)EQS_Vector.size();j++) //WW
 #endif
  {
     if(EQS_Vector[j])  delete EQS_Vector[j];
     EQS_Vector[j] = NULL;
 #if defined(USE_MPI)
     EQS_Vector[j+1] = NULL; 
 #endif
  }
#else
  // SOLver
  LINEAR_SOLVER *eqs;
  for(j=0;j<(int)PCS_Solver.size();j++)
  {
    eqs = PCS_Solver[j]; 
    if(eqs->unknown_vector_indeces)
       eqs->unknown_vector_indeces = \
        (int*) Free(eqs->unknown_vector_indeces);
    if(eqs->unknown_node_numbers)
      eqs->unknown_node_numbers = \
        (long*) Free(eqs->unknown_node_numbers);
    if(eqs->unknown_update_methods)
       eqs->unknown_update_methods = \
        (int*) Free(eqs->unknown_update_methods); 
     eqs = DestroyLinearSolver(eqs);
  }
#endif
  //----------------------------------------------------------------------
  // PCS
  for(j=0;j<(int)pcs_vector.size();j++){
    m_process = pcs_vector[j];
#ifdef USE_MPI //WW
    //  if(myrank==0)
      m_process->Print_CPU_time_byAssembly();
#endif

    if(m_process->pcs_nval_data)
      m_process->pcs_nval_data = (PCS_NVAL_DATA *) Free(m_process->pcs_nval_data);
    if(m_process->pcs_eval_data)
       m_process->pcs_eval_data = (PCS_EVAL_DATA *) Free(m_process->pcs_eval_data);
#ifdef PCS_NOD
    for (i=0;i<NodeListSize();i++) {
      k = GetNode(i);
      k->values[m_process->pcs_number] = (double *)Free(k->values[m_process->pcs_number]);
	}
#endif
    if(m_process->TempArry)   //MX
      m_process->TempArry = (double *) Free(m_process->TempArry);
	delete(m_process);
  }
  //----------------------------------------------------------------------
  // MSH
  for(i=0;i<(long)fem_msh_vector.size();i++)
  {
      if(fem_msh_vector[i]) delete fem_msh_vector[i];
      fem_msh_vector[i] = NULL; 
  }
  fem_msh_vector.clear();
  //----------------------------------------------------------------------
  
  // DOM WW
#if defined(USE_MPI)
  //if(myrank==0)
  dom_vector[myrank]->PrintEQS_CPUtime(); //WW
#endif
  for(i=0;i<(long)dom_vector.size();i++)
  {
    if(dom_vector[i]) delete dom_vector[i];
    dom_vector[i] = NULL; 
  }
  dom_vector.clear();
  
  
  //----------------------------------------------------------------------
  // ELE
  for(i=0;i<(long)ele_val_vector.size();i++)
    delete ele_val_vector[i];
  ele_val_vector.clear();
  //----------------------------------------------------------------------
  // IC ICDelete()
  for(i=0;i<(long)ic_vector.size();i++)
    delete ic_vector[i];
  ic_vector.clear();
  //----------------------------------------------------------------------
  MSPDelete(); //WW
  BCDelete();  //WW
  STDelete();  //WW
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2004 OK Implementation
08/2004 WW Read the deformation process
           Check the comment key '//' in .pcs
last modified:
**************************************************************************/
bool PCSRead(string file_base_name)
{
  //----------------------------------------------------------------------
//OK  PCSDelete();  
  //----------------------------------------------------------------------
  CRFProcess *m_pcs = NULL;
  char line[MAX_ZEILE];
  int indexCh1a, indexCh2a;
  //basic_string <char>::size_type indexCh1a, indexCh2a;
  //  static const basic_string <char>::size_type npos = -1;
  string CommentK("//");
  string line_string;
  string pcs_file_name;
  ios::pos_type position;
  //========================================================================
  // File handling
  pcs_file_name = file_base_name + PCS_FILE_EXTENSION;
  ifstream pcs_file (pcs_file_name.data(),ios::in);
  if (!pcs_file.good()){
    cout << "Warning: no PCS data *.pcs file is missing" << endl;
    return false;
  }
  // rewind the file
  pcs_file.clear();
  pcs_file.seekg(0,ios::beg); 
  //========================================================================
  // Keyword loop
  cout << "PCSRead" << endl;
  while (!pcs_file.eof()) {
    pcs_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      break;
    indexCh1a = (int)line_string.find_first_of(CommentK.c_str()); 
    indexCh2a = (int)line_string.find("#PROCESS");
    //----------------------------------------------------------------------
    if(indexCh2a>indexCh1a&&(indexCh1a==-1)) { // keyword found
      m_pcs = new CRFProcess();
      m_pcs->file_name_base = file_base_name; //OK
      position = m_pcs->Read(&pcs_file);
      //..................................................................
      // pcs_type_name_vector
      int pcs_type_name_vector_size = (int)m_pcs->pcs_type_name_vector.size();
      if(pcs_type_name_vector_size>1){
        string pname = m_pcs->pcs_type_name_vector[0]+m_pcs->pcs_type_name_vector[1];
        m_pcs->pcs_type_name_vector.pop_back();
        if(pname.find("FLOW")!=string::npos&&pname.find("DEFORMATION")!=string::npos)
		{
            m_pcs->pcs_type_name = "DEFORMATION_FLOW";
			MH_Process = true; // MH monolithic scheme
            if(pname.find("DYNAMIC")!=string::npos)
               m_pcs->pcs_type_name_vector[0] = "DYNAMIC";
		}         
      }
      //..................................................................
      m_pcs->pcs_number =(int)pcs_vector.size();
      //RelocateDeformationProcess(m_pcs);
	  if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos) 
      {
         pcs_vector.push_back(m_pcs->CopyPCStoDM_PCS());
         pcs_vector[(int)pcs_vector.size()-1]->pcs_number = (int)pcs_vector.size();
         delete m_pcs;
      }
      //..................................................................
      else
        pcs_vector.push_back(m_pcs);

      pcs_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  ///OK LOPPreTimeLoop_PCS();
  return true;
}
/**************************************************************************
FEMLib-Method: 
Task: Copy data to dm_pcs from PCS read function
Programing:
06/2007 OK/WW Implementation
**************************************************************************/
CRFProcess* CRFProcess::CopyPCStoDM_PCS()
{
   CRFProcessDeformation *dm_pcs = NULL;
   string pcs_name_dm = pcs_type_name; 
   string num_type_name_dm = num_type_name;
   string pname = pcs_type_name_vector[0].data();
   bool m_output = false;
   int rhs_out = WriteSourceNBC_RHS;
   int r_load = reload;
   int m_memory = 0;
   int i = 0;
   int m_inactive = NumDeactivated_SubDomains;
   int Inctive_SubDomain[20];
   pcs_deformation = -1;
   for(i=0; i<m_inactive; i++)
      Inctive_SubDomain[i] = Deactivated_SubDomain[i];

   m_output = Write_Matrix;
   bool write_bc_st = write_boundary_condition; //WW
   m_memory = Memory_Type;
   // Numerics
   if(num_type_name.compare("STRONG_DISCONTINUITY")==0) 
      enhanced_strain_dm=1;

   dm_pcs = new CRFProcessDeformation();

   dm_pcs->pcs_type_name = pcs_name_dm;
   dm_pcs->pcs_type_name_vector.push_back(pname);
   dm_pcs->Write_Matrix=m_output;
   dm_pcs->WriteSourceNBC_RHS = rhs_out;
   dm_pcs->num_type_name = num_type_name_dm;
   dm_pcs-> Memory_Type = m_memory;
   dm_pcs->NumDeactivated_SubDomains = m_inactive;
   dm_pcs->reload = r_load;
   dm_pcs->write_boundary_condition =  write_bc_st; //WW
   if(!dm_pcs->Deactivated_SubDomain)
     dm_pcs->Deactivated_SubDomain = new int[m_inactive];
   for(i=0; i<m_inactive; i++)
      dm_pcs->Deactivated_SubDomain[i] = Inctive_SubDomain[i];
   pcs_deformation = 1;
   //
   return dynamic_cast<CRFProcess *> (dm_pcs);
}

/**************************************************************************
FEMLib-Method: 
Task: PCS read function
Programing:
06/2004 OK Implementation
08/2004 WW Read deformation process
11/2004 OK file streaming
12/2005 OK MSH_TYPE
01/2006 OK GEO_TYPE
**************************************************************************/
ios::pos_type CRFProcess::Read(ifstream *pcs_file)
{
  char line[MAX_ZEILE];
  string line_string;
  string CommentK("//");
  string hash("#");
  bool new_keyword = false;
  bool new_subkeyword = false;
  ios::pos_type position;
  ios::pos_type position_subkeyword;
  std::stringstream line_stream;
  saturation_switch = false; // JOD for Richards
  //----------------------------------------------------------------------
  while (!new_keyword) {
    position = pcs_file->tellg();
    pcs_file->getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    //....................................................................
    // WW Comment line
    if(line_string.find_first_of(CommentK.c_str())!=string::npos) return position;
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
      while((!new_keyword)||(!new_subkeyword)||(!pcs_file->eof())){
        position = pcs_file->tellg();
        line_string = GetLineFromFile1(pcs_file);
        if(line_string.find("#")!=string::npos){
          return position;
        }
        if(line_string.find("$")!=string::npos){
          new_subkeyword = true;
          break;
        }
        line_stream.str(line_string);
        line_stream >> pcs_type_name;
        line_stream.clear();
        if(pcs_type_name.find("FLOW")!=string::npos){ 
          pcs_no_fluid_phases++;
          H_Process = true;
        }
        if(pcs_type_name.compare("FLUID_FLOW")==0){ 
          pcs_type_name = "LIQUID_FLOW";
        }
  	    if(pcs_type_name.find("DEFORMATION")!=string::npos){
          M_Process = true;
	      //if only "DEFORMATION", do not create solver for FLUID process. WW
          if(pcs_no_fluid_phases<1) 
            pcs_no_fluid_phases = 1;
	    }
	    if(pcs_type_name.compare("MASS_TRANSPORT")==0)
        { 
           H_Process = true;
		   MASS_TRANSPORT_Process = true;
           pcs_no_components++;
        }
        if(pcs_type_name.find("HEAT")!=string::npos)
           T_Process = true;
        pcs_type_name_vector.push_back(pcs_type_name);
		if(pcs_type_name.compare("FLUID_MOMENTUM")==0) {
			FLUID_MOMENTUM_Process = true;
			pcs_type_name = "FLUID_MOMENTUM";
		}
        if(pcs_type_name.compare("RANDOM_WALK")==0) {
			RANDOM_WALK_Process = true;
			pcs_type_name = "RANDOM_WALK";
		}
      }
/*
     *pcs_file >> pcs_type_name;
      if(pcs_type_name.find("FLOW")!=string::npos){ 
        pcs_no_fluid_phases++;
      }
      if(pcs_type_name.compare("FLUID_FLOW")==0){ 
        pcs_type_name = "LIQUID_FLOW";
      }
	  if(pcs_type_name.find("DEFORMATION")!=string::npos)
	  {
	    //if only "DEFORMATION", do not create solver for FLUID process. WW
        if(pcs_no_fluid_phases<1) 
          pcs_no_fluid_phases = 1;
	  }
	  if(pcs_type_name.compare("MASS_TRANSPORT")==0) 
        pcs_no_components++;
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
*/
    }
    //....................................................................
    if(line_string.find("$NUM_TYPE")!=string::npos) { // subkeyword found
     *pcs_file >> num_type_name;
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //....................................................................
    if(line_string.find("$CPL_TYPE")!=string::npos) { // subkeyword found
     *pcs_file >> cpl_type_name;
      if(cpl_type_name.compare("MONOLITHIC")==0){
        pcs_monolithic_flow = true;
        pcs_deformation = 11;
      }
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //....................................................................
    if(line_string.find("$TIM_TYPE")!=string::npos) { // subkeyword found
     *pcs_file >> tim_type_name;
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //....................................................................
    if(line_string.find("$PRIMARY_VARIABLE")!=string::npos) { // subkeyword found
     *pcs_file >> primary_variable_name;
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //....................................................................
    if(line_string.find("$ELEMENT_MATRIX_OUTPUT")!=string::npos) { // subkeyword found
      *pcs_file >> Write_Matrix; //WW
      pcs_file->ignore(MAX_ZEILE,'\n');
      continue;
    }
    //....................................................................
    if(line_string.find("$BOUNDARY_CONDITION_OUTPUT")!=string::npos)  //WW
    {
      write_boundary_condition = true;
      continue;
    }
    //....................................................................
    if(line_string.find("$ST_RHS")!=string::npos) { // subkeyword found
		*pcs_file >> WriteSourceNBC_RHS; //WW
        pcs_file->ignore(MAX_ZEILE,'\n');
        continue;
    }
    //....................................................................
    if(line_string.find("$MEMORY_TYPE")!=string::npos) { // subkeyword found
       *pcs_file >> Memory_Type;  //WW
       pcs_file->ignore(MAX_ZEILE,'\n');
       continue;
    }
    //....................................................................
    if(line_string.find("$RELOAD")!=string::npos) { // subkeyword found
       *pcs_file >> reload;  //WW
       pcs_file->ignore(MAX_ZEILE,'\n');
       continue;
    }
    if(line_string.find("$DEACTIVATED_SUBDOMAIN")!=string::npos) { // subkeyword found
        *pcs_file >> NumDeactivated_SubDomains>>ws; //WW
        Deactivated_SubDomain = new int[NumDeactivated_SubDomains];
        for(int i=0; i<NumDeactivated_SubDomains; i++)
           *pcs_file >> Deactivated_SubDomain[i]>>ws;
        continue;
    }
    //....................................................................
    if(line_string.find("$MSH_TYPE")!=string::npos) { // subkeyword found
        *pcs_file >> msh_type_name >> ws;
      continue;
    }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { //OK
      *pcs_file >> geo_type >> geo_type_name >> ws;
      continue;
    }
    //....................................................................
    if(line_string.find("$MEDIUM_TYPE")!=string::npos) { // subkeyword found
      while ((!new_keyword)||(!new_subkeyword)||(!pcs_file->eof())) {
        position_subkeyword = pcs_file->tellg();
       *pcs_file >> line_string;
        if(line_string.size()==0) 
          break;
        if(line_string.find("#")!=string::npos) {
          new_keyword = true;
          break;
        }
        if(line_string.find("$")!=string::npos) {
          new_subkeyword = true;
          break;
        }
        if(line_string.find("CONTINUUM")!=string::npos) {
          *pcs_file >> line_string;
          double w_m = strtod(line_string.data(),NULL); //WW
          continuum_vector.push_back(w_m);
          continuum_vector.push_back(1.0-w_m);  //WW
          break;          //WW
        }
        pcs_file->ignore(MAX_ZEILE,'\n');
      }
      continue;
    }
	if(line_string.find("$SATURATION_SWITCH")!=string::npos) { //OK
      saturation_switch = true;;
      continue;
    }
    //....................................................................
    //....................................................................
  }
  //----------------------------------------------------------------------
  return position;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2004 OK Implementation
08/2004 WW Read the deformation process
           Check the comment key '//' in .pcs
last modified:
**************************************************************************/
void PCSWrite(string file_base_name)
{
  //----------------------------------------------------------------------
  // File handling
  string pcs_file_name = file_base_name + PCS_FILE_EXTENSION;
  fstream pcs_file (pcs_file_name.data(),ios::trunc|ios::out);
  // rewind the file
  pcs_file.clear();
  //----------------------------------------------------------------------
  // PCS loop
  cout << "PCSWrite" << endl;
  int no_processes = (int)pcs_vector.size();
  CRFProcess* m_pcs = NULL;
  int i;
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    pcs_file << "#PROCESS" << endl;
    m_pcs->Write(&pcs_file);
  }
  //----------------------------------------------------------------------
  pcs_file << "#STOP" << endl;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2004 OK Implementation
12/2005 OK MSH_TYPE
last modified:
**************************************************************************/
void CRFProcess::Write(fstream *pcs_file)
{
  //......................................................................
 *pcs_file << " $PCS_TYPE" << endl;
 *pcs_file << "  " << pcs_type_name << endl;
  //......................................................................
 *pcs_file << " $NUM_TYPE" << endl;
 *pcs_file << "  " << num_type_name << endl;
  //......................................................................
 *pcs_file << " $CPL_TYPE" << endl;
 *pcs_file << "  " << cpl_type_name << endl;
  //......................................................................
 *pcs_file << " $TIM_TYPE" << endl;
 *pcs_file << "  " << tim_type_name << endl;
  //......................................................................
  if(msh_type_name.size()>0){
   *pcs_file << " $MSH_TYPE" << endl;
   *pcs_file << "  " << msh_type_name << endl;
  }
  //......................................................................
 //*pcs_file << " $PRIMARY_VARIABLE" << endl;
 //*pcs_file << "  " << primary_variable_name << endl;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
12/2004 OK Implementation
last modified:
**************************************************************************/
CRFProcess* PCSGet(string pcs_type_name)
{
  CRFProcess *m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare(pcs_type_name)==0){
      return m_pcs;
    }
  }
  return NULL;
}
//////////////////////////////////////////////////////////////////////////
// Access
//////////////////////////////////////////////////////////////////////////


// OK->SB please try Get function
CRFProcess *CRFProcess::GetProcessByFunctionName(char *name)
{
  CRFProcess *m_process = NULL;
  /* Tests */
  if (!name) return m_process;
  int i;
  int no_processes =(int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
    m_process = pcs_vector[i];
    if ( strcmp(StrUp(m_process->pcs_primary_function_name[0]),StrUp(name))==0 ) {
      break;
    }
  }
  return m_process;
}

//SB: new 3912
CRFProcess *CRFProcess::GetProcessByNumber(int number)
{
  CRFProcess *m_process = NULL;
  /* Tests */
  if (number < 1) return m_process;
  int i;
  int no_processes =(int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
    m_process = pcs_vector[i];
    if ( m_process->pcs_number == number ) {
      break;
    }
  }
  return m_process;
}

//////////////////////////////////////////////////////////////////////////
// Configuration
//////////////////////////////////////////////////////////////////////////

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2003 OK Implementation
08/2003 WW Modified to fit monolithic scheme 
02/2005 OK Unsaturated flow (Richards model)
02/2005 MB string
05/2005 WW/DL Dymanic problem
01/2006 YD Dual Richards
OKToDo switch to char
get rid of type
**************************************************************************/
void CRFProcess::Config(void)
{
  //----------------------------------------------------------------------
  // Set mesh pointer to corresponding mesh
  m_msh = FEMGet(pcs_type_name);
  if(!m_msh)
    cout << "Error in CRFProcess::Config - no MSH data" << endl;
  CheckMarkedElement();	 //WW
  //......................................................................
  if((int)continuum_vector.size()== 0)  // YD
    continuum_vector.push_back(1.0);
  //......................................................................
  //  CRFProcessDeformation *dm_pcs;
  //  dm_pcs = NULL;
  if(pcs_type_name.compare("LIQUID_FLOW")==0)  {
    type = 1; 
    ConfigLiquidFlow();
  }
  if(pcs_type_name.compare("GROUNDWATER_FLOW")==0)  {
    type = 1; 
    ConfigGroundwaterFlow();
  }
  if(pcs_type_name.compare("RICHARDS_FLOW")==0)  {
    if((int)continuum_vector.size()>1){
      RD_Process = true;
      type = 22;
    }
    else
       type = 14;
    ConfigUnsaturatedFlow();
  }
  if(pcs_type_name.compare("OVERLAND_FLOW")==0)  {
    type = 66;
    max_dim = 1;
    ConfigGroundwaterFlow();
  }
  if(pcs_type_name.compare("AIR_FLOW")==0)  { //OK
    type = 5;
    ConfigGasFlow();
  }
  if(pcs_type_name.compare("TWO_PHASE_FLOW")==0)  {
    type = 12;
    ConfigMultiphaseFlow();
  }
  if(pcs_type_name.compare("COMPONENTAL_FLOW")==0)  {
    type = 11;
    ConfigNonIsothermalFlow();
  }
  if(pcs_type_name.compare("HEAT_TRANSPORT")==0)  {
    type = 3;
    ConfigHeatTransport();
  }
  if(pcs_type_name.compare("MASS_TRANSPORT")==0)  {
    type = 2;
    ConfigMassTransport();
  }
  if(pcs_type_name.find("DEFORMATION")!=string::npos)
    ConfigDeformation();
  if(pcs_type_name.find("FLUID_MOMENTUM")!=string::npos) 
  {
     type = 55; //WW
     ConfigFluidMomentum();
  }  
  if(pcs_type_name.find("RANDOM_WALK")!=string::npos)
  {
    type = 55; //WW
	ConfigRandomWalk();
  }
  if(pcs_type_name.find("MULTI_PHASE_FLOW")!=string::npos) //24.02.2007 WW
  {
     type = 1212;
     ConfigMultiPhaseFlow();
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 OK Implementation
        WW Splitted for processes
last modified:
02/2005 MB Pressure version for LIQUID Flow
**************************************************************************/
void CRFProcess::ConfigLiquidFlow()
{
  pcs_num_name[0] = "PRESSURE0";
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_PRESSURE1";
  // NOD values
  pcs_number_of_primary_nvals = 1;
  pcs_number_of_secondary_nvals = 0;
  pcs_primary_function_name[0] = "PRESSURE1";
  pcs_primary_function_unit[0] = "Pa";
  // ELE values
  pcs_number_of_evals = 5;
  pcs_eval_name[0] = "VOLUME";
  pcs_eval_unit[0] = "m3";
  pcs_eval_name[1] = "VELOCITY1_X";
  pcs_eval_unit[1] = "m/s";
  pcs_eval_name[2] = "VELOCITY1_Y";
  pcs_eval_unit[2] = "m/s";
  pcs_eval_name[3] = "VELOCITY1_Z";
  pcs_eval_unit[3] = "m/s";
    pcs_eval_name[4] = "POROSITY";  //MX, test for n=n(c), 04.2005
  pcs_eval_unit[4] = "-";
  //----------------------------------------------------------------------
  // Secondary variables
  pcs_number_of_secondary_nvals = 0; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "HEAD";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  //WW
  // Output material parameters
  //WW
  COutput *m_out = NULL;
  for(int i=0;i<(int)out_vector.size();i++)
  {
     m_out = out_vector[i];
     for(int k=0; k<(int)m_out->nod_value_vector.size(); k++)
     {
        if(m_out->nod_value_vector[k].find("PERMEABILITY_X1")!=string::npos)
        {
           additioanl2ndvar_print = 1;
           break; 
        }   
     }
     if(additioanl2ndvar_print == 1)
        break;    
  }
  for(int i=0;i<(int)out_vector.size();i++)
  {
     m_out = out_vector[i];
     for(int k=0; k<(int)m_out->nod_value_vector.size(); k++)
     {
        if(m_out->nod_value_vector[k].find("POROSITY")!=string::npos) 
        { 
          if(additioanl2ndvar_print>0)
            additioanl2ndvar_print = 2;
          else
            additioanl2ndvar_print = 3; 
        } 
        if(additioanl2ndvar_print>1)
          break;    
     }
     if(additioanl2ndvar_print>1)
        break;    
  }

  if(additioanl2ndvar_print>0) //WW
  {
    if(additioanl2ndvar_print<3)
    {
      pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_X1";
      pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
      pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
      pcs_number_of_secondary_nvals++;     
      pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_Y1";
      pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
      pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
      pcs_number_of_secondary_nvals++;     
      if(max_dim==2) // 3D
      {
        pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_Z1";
        pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
        pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
        pcs_number_of_secondary_nvals++;
      }
    }
    if(additioanl2ndvar_print>1) //WW
    {
       pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "POROSITY";
       pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-";
       pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
       pcs_number_of_secondary_nvals++;          
    }             
  }

}


/**************************************************************************
FEMLib-Method: 
03/2003 OK Implementation
        WW Splitted for processes
02/2005 MB head version for GroundwaterFlow
08/2006 OK FLUX
**************************************************************************/
void CRFProcess::ConfigGroundwaterFlow()
{
  pcs_num_name[0] = "HEAD";
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_HEAD";
  // NOD values
  pcs_number_of_primary_nvals = 1;
  pcs_primary_function_name[0] = "HEAD";
  pcs_primary_function_unit[0] = "m";
  // ELE values
  pcs_number_of_evals = 6;
  pcs_eval_name[0] = "VOLUME";
  pcs_eval_unit[0] = "m3";
  pcs_eval_name[1] = "VELOCITY1_X";
  pcs_eval_unit[1] = "m/s";
  pcs_eval_name[2] = "VELOCITY1_Y";
  pcs_eval_unit[2] = "m/s";
  pcs_eval_name[3] = "VELOCITY1_Z";
  pcs_eval_unit[3] = "m/s";
  pcs_eval_name[4] = "PERMEABILITY";
  pcs_eval_unit[4] = "m^2";
  pcs_eval_name[5] = "POROSITY";
  pcs_eval_unit[5] = "-";
  //----------------------------------------------------------------------
  // Secondary variables
  pcs_number_of_secondary_nvals = 4;
  pcs_secondary_function_name[0] = "FLUX";
  pcs_secondary_function_unit[0] = "m3/s";
  pcs_secondary_function_timelevel[0] = 1;
  pcs_secondary_function_name[1] = "WDEPTH";
  pcs_secondary_function_unit[1] = "m";
  pcs_secondary_function_timelevel[1] = 1;
  pcs_secondary_function_name[2] = "COUPLING"; //JOD 
  pcs_secondary_function_unit[2] = "m/s";
  pcs_secondary_function_timelevel[2] = 0;
  pcs_secondary_function_name[3] = "COUPLING"; //JOD
  pcs_secondary_function_unit[3] = "m/s";
  pcs_secondary_function_timelevel[3] = 1;
  //----------------------------------------------------------------------
  if(m_msh)
    m_msh->DefineMobileNodes(this);
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
10/2004 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::ConfigGasFlow()
{
  //----------------------------------------------------------------------
  // Primary variables - NOD values
  pcs_number_of_primary_nvals = 1;
  pcs_number_of_secondary_nvals = 0;
  pcs_primary_function_name[0] = "PRESSURE1";
  pcs_primary_function_unit[0] = "Pa";
  //----------------------------------------------------------------------
  // Secondary variables - NOD values
  pcs_number_of_secondary_nvals = 1;
  pcs_secondary_function_name[0] = "NOD_MASS_FLUX";
  pcs_secondary_function_unit[0] = "kg/s";
  //----------------------------------------------------------------------
  // ELE values
  pcs_number_of_evals = 3;
  pcs_eval_name[0] = "VELOCITY1_X";
  pcs_eval_unit[0] = "m/s";
  pcs_eval_name[1] = "VELOCITY1_Y";
  pcs_eval_unit[1] = "m/s";
  pcs_eval_name[2] = "VELOCITY1_Z";
  pcs_eval_unit[2] = "m/s";
  //----------------------------------------------------------------------
  // NUM
  pcs_num_name[0] = "PRESSURE0";
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_PRESSURE1";
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 OK Implementation
        WW Splitted for processes
last modified:
**************************************************************************/
void CRFProcess::ConfigMultiphaseFlow()
{           
  switch(pcs_type_number){
    case 0:
      pcs_num_name[0]= "PRESSURE0";
      pcs_sol_name   = "LINEAR_SOLVER_PROPERTIES_PRESSURE1";
      break;
    case 1:
      pcs_num_name[0]= "SATURATION0";
      pcs_sol_name   = "LINEAR_SOLVER_PROPERTIES_SATURATION1";
      break;
  }
  //----------------------------------------------------------------------
  // NOD Primary variables
  pcs_number_of_primary_nvals=1;
  switch(pcs_type_number){
    case 0:
      pcs_primary_function_name[0] = "PRESSURE1";
      pcs_primary_function_unit[0] = "Pa";
      break;
    case 1:
      pcs_primary_function_name[0] = "SATURATION2";
      pcs_primary_function_unit[0] = "m3/m3";
      break;
  }
  //----------------------------------------------------------------------
  // NOD Secondary variables
  pcs_number_of_secondary_nvals = 6;
  switch(pcs_type_number){
    case 0:
      pcs_secondary_function_name[0]      = "PRESSURE2";
      pcs_secondary_function_unit[0]      = "Pa";
      pcs_secondary_function_timelevel[0] = 0;
      pcs_secondary_function_name[1]      = "PRESSURE2";
      pcs_secondary_function_unit[1]      = "Pa";
      pcs_secondary_function_timelevel[1] = 1;
      pcs_secondary_function_name[2]      = "PRESSURE_CAP";
      pcs_secondary_function_unit[2]      = "Pa";
      pcs_secondary_function_timelevel[2] = 0;
      pcs_secondary_function_name[3]      = "FLUX";
      pcs_secondary_function_unit[3]      = "m3/s";
      pcs_secondary_function_timelevel[3] = 0;
      pcs_secondary_function_name[4]      = "DENSITY1";
      pcs_secondary_function_unit[4]      = "kg/m3";
      pcs_secondary_function_timelevel[4] = 1;
      pcs_secondary_function_name[5]      = "VISCOSITY1";
      pcs_secondary_function_unit[5]      = "Pa s";
      pcs_secondary_function_timelevel[5] = 1;
      break;
    case 1:
      pcs_secondary_function_name[0]      = "SATURATION1";
      pcs_secondary_function_timelevel[0] = 0;
      pcs_secondary_function_unit[0]      = "m3/m3";
      pcs_secondary_function_name[1]      = "SATURATION1";
      pcs_secondary_function_timelevel[1] = 1;
      pcs_secondary_function_unit[1]      = "m3/m3";
      pcs_secondary_function_name[2]      = "PRESSURE_CAP";
      pcs_secondary_function_unit[2]      = "Pa";
      pcs_secondary_function_timelevel[2] = 1;
      pcs_secondary_function_name[3]      = "FLUX";
      pcs_secondary_function_unit[3]      = "m3/s";
      pcs_secondary_function_timelevel[3] = 1;
      pcs_secondary_function_name[4]      = "DENSITY2";
      pcs_secondary_function_unit[4]      = "kg/m3";
      pcs_secondary_function_timelevel[4] = 1;
      pcs_secondary_function_name[5]      = "VISCOSITY2";
      pcs_secondary_function_unit[5]      = "Pa s";
      pcs_secondary_function_timelevel[5] = 1;
      break;
  }
  //----------------------------------------------------------------------
  // ELE values
  pcs_number_of_evals = 7;
  switch(pcs_type_number){
    case 0:
      pcs_eval_name[0] = "VELOCITY1_X";
      pcs_eval_unit[0] = "m/s";
	  pcs_eval_name[1] = "VELOCITY1_Y";
      pcs_eval_unit[1] = "m/s";
	  pcs_eval_name[2] = "VELOCITY1_Z";
      pcs_eval_unit[2] = "m/s";
	  pcs_eval_name[3] = "POROSITY1"; //MX 03.2005
      pcs_eval_unit[3] = "-";
	  pcs_eval_name[4] = "POROSITY1_IL"; //MX 03.2005
      pcs_eval_unit[4] = "-";
	  pcs_eval_name[5] = "PERMEABILITY1"; //MX 03.2005
      pcs_eval_unit[5] = "-";
	  pcs_eval_name[6] = "POROSITY1_SW"; //MX 03.2005
      pcs_eval_unit[6] = "-";
      break;
    case 1:
  	  pcs_eval_name[0] = "VELOCITY2_X";
      pcs_eval_unit[0] = "m/s";
	  pcs_eval_name[1] = "VELOCITY2_Y";
      pcs_eval_unit[1] = "m/s";
	  pcs_eval_name[2] = "VELOCITY2_Z";
      pcs_eval_unit[2] = "m/s";
	  pcs_eval_name[3] = "POROSITY";   //MX 03.2005
      pcs_eval_unit[3] = "-";
	  pcs_eval_name[4] = "POROSITY_IL"; //MX 03.2005
      pcs_eval_unit[4] = "-";
	  pcs_eval_name[5] = "PERMEABILITY"; //MX 03.2005
      pcs_eval_unit[5] = "-";
	  pcs_eval_name[6] = "POROSITY_SW"; //MX 03.2005
      pcs_eval_unit[6] = "-";
      break;
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 OK Implementation
        WW Splitted for processes
last modified:
**************************************************************************/
void CRFProcess::ConfigNonIsothermalFlow()
{
  //----------------------------------------------------------------------
  // Primary variables
  pcs_number_of_primary_nvals=1;
  switch(pcs_type_number){
    case 0:
      pcs_num_name[0]= "PRESSURE0";
      pcs_sol_name = "LINEAR_SOLVER_PROPERTIES_PRESSURE1";
      pcs_primary_function_name[0] = "PRESSURE1";
      pcs_primary_function_unit[0] = "Pa";
      break;
    case 1:
      pcs_num_name[0]= "SATURATION0";
      pcs_sol_name = "LINEAR_SOLVER_PROPERTIES_SATURATION1";
      pcs_primary_function_name[0] = "SATURATION2";
      pcs_primary_function_unit[0] = "m3/m3";
      break;
  }
  //----------------------------------------------------------------------
  // Secondary variables
  pcs_number_of_secondary_nvals = 6;
  switch(pcs_type_number){
    case 0:
      pcs_secondary_function_name[0] = "PRESSURE2";
      pcs_secondary_function_timelevel[0] = 0;
      pcs_secondary_function_unit[0] = "Pa";
      pcs_secondary_function_name[1] = "PRESSURE2";
      pcs_secondary_function_timelevel[1] = 1;
      pcs_secondary_function_unit[1] = "Pa";
      pcs_secondary_function_name[2] = "MASS_FRACTION1";
      pcs_secondary_function_timelevel[2] = 0;
      pcs_secondary_function_unit[2] = "kg/kg";
      pcs_secondary_function_name[3] = "MASS_FRACTION1";
      pcs_secondary_function_timelevel[3] = 1;
      pcs_secondary_function_unit[3] = "kg/kg";
      pcs_secondary_function_name[4] = "PRESSURE_CAP";
      pcs_secondary_function_timelevel[4] = 0;
      pcs_secondary_function_unit[4] = "Pa";
      pcs_secondary_function_name[5] = "DENSITY1";
      pcs_secondary_function_timelevel[5] = 1;
      pcs_secondary_function_unit[5] = "kg/m3";
      break;
    case 1:
      pcs_secondary_function_name[0] = "SATURATION1";
      pcs_secondary_function_timelevel[0] = 0;
      pcs_secondary_function_unit[0] = "m3/m3";
      pcs_secondary_function_name[1] = "SATURATION1";
      pcs_secondary_function_timelevel[1] = 1;
      pcs_secondary_function_unit[1] = "m3/m3";
      pcs_secondary_function_name[2] = "MASS_FRACTION2";
      pcs_secondary_function_timelevel[2] = 0;
      pcs_secondary_function_unit[2] = "kg/kg";
      pcs_secondary_function_name[3] = "MASS_FRACTION2";
      pcs_secondary_function_timelevel[3] = 1;
      pcs_secondary_function_unit[3] = "kg/kg";
      pcs_secondary_function_name[4] = "PRESSURE_CAP";
      pcs_secondary_function_timelevel[4] = 1;
      pcs_secondary_function_unit[4] = "Pa";
      pcs_secondary_function_name[5] = "DENSITY2";
      pcs_secondary_function_timelevel[5] = 1;
      pcs_secondary_function_unit[5] = "kg/m3";
      break;
  }
  // Node
  pcs_number_of_primary_nvals=1;
  // ELE values
  pcs_number_of_evals = 14;
  pcs_eval_name[0]  = "COMP_FLUX";
  pcs_eval_name[1]  = "POROSITY";
  pcs_eval_name[2]  = "PERMEABILITY";
  pcs_eval_name[3]  = "VELOCITY1_X";
  pcs_eval_name[4]  = "VELOCITY1_Y";
  pcs_eval_name[5]  = "VELOCITY1_Z";
  pcs_eval_name[6]  = "VELOCITY2_X";
  pcs_eval_name[7]  = "VELOCITY2_Y";
  pcs_eval_name[8]  = "VELOCITY2_Z";
  pcs_eval_name[9]  = "POROSITY_IL";
  pcs_eval_name[10] = "VoidRatio";
  pcs_eval_name[11] = "PorosityChange";
  pcs_eval_name[12] = "n_sw_Rate";
  pcs_eval_name[13]  = "POROSITY_SW";
  pcs_eval_unit[0]  = "kg/s";
  pcs_eval_unit[1]  = "m3/m3";
  pcs_eval_unit[2]  = "m2";
  pcs_eval_unit[3]  = "m/s";
  pcs_eval_unit[4]  = "m/s";
  pcs_eval_unit[5]  = "m/s";
  pcs_eval_unit[6]  = "m/s";
  pcs_eval_unit[7]  = "m/s";
  pcs_eval_unit[8]  = "m/s";
  pcs_eval_unit[9]  = "-";
  pcs_eval_unit[10] = "-";
  pcs_eval_unit[11] = "-";
  pcs_eval_unit[12] = "-";
  pcs_eval_unit[13] = "-";
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 OK Implementation
        WW Splitted for processes
last modified:
**************************************************************************/
//void CRFProcess::ConfigNonIsothermalFlowRichards()

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2004 SB Implementation
        WW Splitted for processes
01/2006 OK Tests    
08/2006 OK FLUX    
**************************************************************************/
void CRFProcess::ConfigMassTransport()
{
  long comp=1;
  /* count transport processes */
  pcs_component_number++;
  comp = pcs_component_number;
  // 1 NOD values
  // 1.1 primary variables
  pcs_number_of_primary_nvals = 1;
  pcs_primary_function_name[0]= new char[80];
//  sprintf(pcs_primary_function_name[0], "%s%li","CONCENTRATION",comp);
  //----------------------------------------------------------------------
  // Tests
  if((int)cp_vec.size()<pcs_component_number+1){
    cout << "Error in CRFProcess::ConfigMassTransport - not enough MCP data" << endl;
#ifdef MFC
    AfxMessageBox("Error in CRFProcess::ConfigMassTransport - not enough MCP data");
#endif
    return;
  }
  //----------------------------------------------------------------------
  sprintf(pcs_primary_function_name[0], "%s", cp_vec[pcs_component_number]->compname.c_str());
  pcs_primary_function_unit[0] = "kg/m3";  //SB
/* SB: Eintrag component name in Ausgabestruktur */ //SB:todo : just one phase todo:name
/*
    	pcs_primary_function_name[0] = GetTracerCompName(0,this->pcs_component_number-1);
		name_initial_condition_tracer_component = pcs_primary_function_name[0];
		pcs_ic_name_mass = pcs_primary_function_name[0]; 
*/
  // 1.2 secondary variables
  pcs_number_of_secondary_nvals = 2; //SB3909
  pcs_secondary_function_name[0]= new char[80];
  sprintf(pcs_secondary_function_name[0], "%s%li","MASS_FLUX_",comp);
//      pcs_secondary_function_name[0] = "MASS_FLUX1";
  pcs_secondary_function_unit[0] = "kg/m3/s";
  pcs_secondary_function_timelevel[0] = 0;
  pcs_secondary_function_name[1]= new char[80];
  sprintf(pcs_secondary_function_name[1], "%s%li","MASS_FLUX_",comp);
  pcs_secondary_function_unit[1] = "kg/m3/s";
  pcs_secondary_function_timelevel[1] = 1;
//OK  LOPCalcSecondaryVariables_USER = MTM2CalcSecondaryVariables;  //SB:todo
  // 2 ELE values
  pcs_number_of_evals = 0;
//	  pcs_eval_name[0] = "Darcy velocity";
#ifdef REACTION_ELEMENT
  pcs_number_of_evals = 1;
  pcs_eval_name[0] = pcs_primary_function_name[0];
      pcs_eval_unit[0] = "mol/kgH2O";
#endif
  // 3 ELE matrices
  // NUM
  pcs_num_name[0] = "CONCENTRATION0";
  /* SB: immer solver properties der ersten Komponente nehmen */
  pcs_sol_name = "LINEAR_SOLVER_PROPERTIES_CONCENTRATION1"; //SB ??
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 OK Implementation
        WW Splitted for processes
last modified:
**************************************************************************/
void CRFProcess::ConfigHeatTransport()
{
  pcs_num_name[0] = "TEMPERATURE0";
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_TEMPERATURE1";
//WW  ConfigELEMatrices = PCSConfigELEMatricesHTM;
  // NOD
  if((int)continuum_vector.size() == 1)
  {
  pcs_number_of_primary_nvals = 1;
  pcs_primary_function_name[0] = "TEMPERATURE1";
  pcs_primary_function_unit[0] = "K";
  pcs_number_of_secondary_nvals = 0;
#ifdef REACTION_ELEMENT
  pcs_number_of_evals = 1;  //MX
  pcs_eval_name[0] = "TEMPERATURE1";
  pcs_eval_unit[0] = "K";
#endif
  }
  if((int)continuum_vector.size() == 2)
  {
  pcs_number_of_primary_nvals = 2;
  pcs_primary_function_name[0] = "TEMPERATURE1";
  pcs_primary_function_unit[0] = "K";
  pcs_number_of_primary_nvals = 2;
  pcs_primary_function_name[1] = "TEMPERATURE2";
  pcs_primary_function_unit[1] = "K";
  pcs_number_of_secondary_nvals = 0;
  } 
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
03/2003 WW Implementation
last modified:
**************************************************************************/
void CRFProcess::ConfigDeformation()
{
  int i;
  CNumerics* num=NULL;
  // Generate high order nodes for all elements.
  m_msh->GenerateHighOrderNodes(); //WW
  type = 4;
  if(  pcs_type_name.find("DEFORMATION")!=string::npos
     &&pcs_type_name.find("FLOW")!=string::npos)
  {
     type = 41;
     cpl_type_name="MONOLITHIC";
     pcs_deformation=11;
  }
  pcs_coupling_iterations = 10;
  
  for(i=0;i<(int)num_vector.size();i++){
    num = num_vector[i];
    if(num->pcs_type_name.find("DEFORMATION")!=string::npos)
    {
       num->pcs_type_name = pcs_type_name;
	   if(num->nls_method==1)  // Newton-Raphson
	   {
           pcs_deformation = 101;
           if(type==41) pcs_deformation = 110;
	   }
       break;
     }
  }

  // Prepare for restart
  //RFConfigRenumber();
 
  // Geometry dimension
  problem_dimension_dm=m_msh->GetCoordinateFlag()/10;
  problem_2d_plane_dm=1;
 
  // NUM        
  pcs_sol_name = "LINEAR_SOLVER_PROPERTIES_DISPLACEMENT1";
  pcs_num_name[0] = "DISPLACEMENT0";
  pcs_num_name[1] = "PRESSURE0";
  if(pcs_type_name_vector[0].find("DYNAMIC")!=string::npos)
    VariableDynamics();
  else
    VariableStaticProblem();
  
  // Coupling
  for(int i=0; i<GetPrimaryVNumber(); i++)
     Shift[i] = i*m_msh->GetNodesNumber(true);
  // OBJ names are set to PCS name
  if(type==41) SetOBJNames(); //OK->WW please put to Config()
}



/**************************************************************************
FEMLib-Method: Static problems
Task: 
Programing:
05/2005 WW Implementation
last modified:
**************************************************************************/
void CRFProcess::VariableStaticProblem()
{
  //----------------------------------------------------------------------
  // NOD Primary functions
  pcs_number_of_primary_nvals = 2; //OK distinguish 2/3D problems, problem_dimension_dm;
  dm_number_of_primary_nvals = 2; 
  pcs_primary_function_name[0] = "DISPLACEMENT_X1";
  pcs_primary_function_name[1] = "DISPLACEMENT_Y1";
  pcs_primary_function_unit[0] = "m";
  pcs_primary_function_unit[1] = "m";
  if(max_dim==2)
  {
     pcs_number_of_primary_nvals = 3;
     dm_number_of_primary_nvals = 3; 
     pcs_primary_function_name[2] = "DISPLACEMENT_Z1";
     pcs_primary_function_unit[2] = "m";
  }
  if(type==41){ //Monolithic scheme
    pcs_primary_function_name[pcs_number_of_primary_nvals] = "PRESSURE1";
    pcs_primary_function_unit[pcs_number_of_primary_nvals] = "Pa";
    pcs_number_of_primary_nvals++;
  }	  
  //----------------------------------------------------------------------
  // NOD Secondary functions
  pcs_number_of_secondary_nvals = 0;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XX"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_YY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_ZZ"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XX"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_YY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_ZZ";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_PLS"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
//  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "POROPRESSURE0"; 
//  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
//  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
//  pcs_number_of_secondary_nvals++;
  
  if(max_dim==2) // 3D
  {  
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_YZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "--"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_YZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "--"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
  }

}
/**************************************************************************
FEMLib-Method: Dynamic problems
Task: 
Programing:
05/2005 WW/LD Implementation
last modified:
**************************************************************************/
void CRFProcess::VariableDynamics()
{
  //----------------------------------------------------------------------
  // NOD Primary functions
  pcs_number_of_primary_nvals = 2; 
  dm_number_of_primary_nvals = 2; 
  pcs_primary_function_name[0] = "ACCELERATION_X1";
  pcs_primary_function_name[1] = "ACCELERATION_Y1";
  pcs_primary_function_unit[0] = "m/s^2";
  pcs_primary_function_unit[1] = "m/s^2";
  if(max_dim==2)
  {
     pcs_number_of_primary_nvals = 3;
     dm_number_of_primary_nvals = 3; 
     pcs_primary_function_name[2] = "ACCELERATION_Z1";
     pcs_primary_function_unit[2] = "m/s^2";
  }
  pcs_primary_function_name[pcs_number_of_primary_nvals] = "PRESSURE_RATE1";
  pcs_primary_function_unit[pcs_number_of_primary_nvals] = "Pa/s";
  pcs_number_of_primary_nvals++;

  //----------------------------------------------------------------------
  // NOD Secondary functions
  pcs_number_of_secondary_nvals = 0;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XX"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_YY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_ZZ"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
//  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "POROPRESSURE0"; 
//  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
//  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
//  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XX"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_YY"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_ZZ";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_PLS"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "DISPLACEMENT_X1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "DISPLACEMENT_Y1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_DM_X";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_DM_Y";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE1"; 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
  pcs_number_of_secondary_nvals++;
  // 3D
  if(max_dim==2) // 3D
  {  
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_XZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRESS_YZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_XZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STRAIN_YZ"; 
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "DISPLACEMENT_Z1";
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
     pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_DM_Z";
     pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s"; 
     pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1; 
     pcs_number_of_secondary_nvals++;
  }
}


/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2005 OK Implementation
02/2006 OK FLUX
**************************************************************************/
void CRFProcess::ConfigUnsaturatedFlow()
{
 if((int)continuum_vector.size() == 1){
  // 1.1 primary variables
  pcs_number_of_primary_nvals = 1;
  pcs_primary_function_name[0] = "PRESSURE1";
  pcs_primary_function_unit[0] = "Pa";
   // 1.2 secondary variables
  //OK LOPCalcSecondaryVariables_USER = MMPCalcSecondaryVariablesRichards; // p_c and S^l
  pcs_number_of_secondary_nvals = 0;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE_CAP1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "FLUX"; //MB
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "FLUX"; //MB
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
//TEST
#define DECOVALEX
#ifdef DECOVALEX
  // DECOVALEX Test
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE_I";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++;
#endif
 /* if(adaption) //WW, JOD removed
  {
    pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "STORAGE_P"; 
    pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa";
    pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
    pcs_number_of_secondary_nvals++;
  }*/
  // Nodal velocity. WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "COUPLING"; //JOD 
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "COUPLING"; //JOD
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++;
  }
  else if((int)continuum_vector.size() == 2){
  dof = 2; //WW
  // 1.1 primary variables
  pcs_number_of_primary_nvals = 2;     //YD
  pcs_primary_function_name[0] = "PRESSURE1";
  pcs_primary_function_unit[0] = "Pa";
  pcs_primary_function_name[1] = "PRESSURE2";
  pcs_primary_function_unit[1] = "Pa";
   // 1.2 secondary variables
  //OK LOPCalcSecondaryVariables_USER = MMPCalcSecondaryVariablesRichards; // p_c and S^l
  pcs_number_of_secondary_nvals = 0;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE_CAP1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE_CAP2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "Pa";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++; //WW
  // Nodal velocity. WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; //WW
 }

  // Output material parameters
  //WW
  COutput *m_out = NULL;
  for(int i=0;i<(int)out_vector.size();i++)
  {
     m_out = out_vector[i];
     for(int k=0; k<(int)m_out->nod_value_vector.size(); k++)
     {
        if(m_out->nod_value_vector[k].find("PERMEABILITY_X1")!=string::npos)
        {
           additioanl2ndvar_print = 1;
           break; 
        }   
     }
     if(additioanl2ndvar_print == 1)
        break;    
  }
  for(int i=0;i<(int)out_vector.size();i++)
  {
     m_out = out_vector[i];
     for(int k=0; k<(int)m_out->nod_value_vector.size(); k++)
     {
        if(m_out->nod_value_vector[k].find("POROSITY")!=string::npos) 
        { 
          if(additioanl2ndvar_print>0)
            additioanl2ndvar_print = 2;
          else
            additioanl2ndvar_print = 3; 
        } 
        if(additioanl2ndvar_print>1)
          break;    
     }
     if(additioanl2ndvar_print>1)
        break;    
  }

  if(additioanl2ndvar_print>0) //WW
  {
    if(additioanl2ndvar_print<3)
    {
      pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_X1";
      pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
      pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
      pcs_number_of_secondary_nvals++;     
      pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_Y1";
      pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
      pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
      pcs_number_of_secondary_nvals++;     
      if(max_dim==2) // 3D
      {
        pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PERMEABILITY_Z1";
        pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "1/m^2";
        pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
        pcs_number_of_secondary_nvals++;
      }
    }
    if(additioanl2ndvar_print>1) //WW
    {
       pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "POROSITY";
       pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "-";
       pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
       pcs_number_of_secondary_nvals++;          
    }             
  }

  // 2 ELE values
  pcs_number_of_evals = 8; 
  pcs_eval_name[0] = "VELOCITY1_X";
  pcs_eval_unit[0]  = "m/s";
  pcs_eval_name[1] = "VELOCITY1_Y";
  pcs_eval_unit[1]  = "m/s";
  pcs_eval_name[2] = "VELOCITY1_Z";
  pcs_eval_unit[2]  = "m/s";
  pcs_eval_name[3] = "POROSITY";   //MX 11.2005
  pcs_eval_unit[3] = "-";
  pcs_eval_name[4] = "POROSITY_IL"; //MX 11.2005
  pcs_eval_unit[4] = "-";
  pcs_eval_name[5] = "PERMEABILITY"; //MX 11.2005
  pcs_eval_unit[5] = "-";
  pcs_eval_name[6] = "n_sw"; //MX 11.2005
  pcs_eval_unit[6] = "-";
  pcs_eval_name[7] = "n_sw_rate"; //MX 11.2005
  pcs_eval_unit[7] = "-";

}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
05/2005 PCH Implementation
last modified:
**************************************************************************/
void CRFProcess::ConfigFluidMomentum()
{
  //pcs_num_name[0] = "VELOCITY1_X";
  // Nothing added in terms of matrix solver.
  // Just linear solver is good enough.
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_PRESSURE1"; 
  // NOD values
  pcs_number_of_primary_nvals = 3;
  pcs_primary_function_name[0] = "VELOCITY1_X";
  pcs_primary_function_unit[0] = "m/s";
  pcs_primary_function_name[1] = "VELOCITY1_Y";
  pcs_primary_function_unit[1] = "m/s";
  pcs_primary_function_name[2] = "VELOCITY1_Z";
  pcs_primary_function_unit[2] = "m/s";

  // I'm adding this to initialize for Fluid Momentum process
  pcs_number_of_secondary_nvals = 0;
  pcs_number_of_evals = 3;
 
  pcs_eval_name[0] = "VELOCITY1_X";
  pcs_eval_unit[0]  = "m/s";
  pcs_eval_name[1] = "VELOCITY1_Y";
  pcs_eval_unit[1]  = "m/s";
  pcs_eval_name[2] = "VELOCITY1_Z";
  pcs_eval_unit[2]  = "m/s";
}
/**************************************************************************/
void CRFProcess::ConfigRandomWalk()
{
  // Nothing added in terms of matrix solver.
  // Just linear solver is good enough.
  pcs_sol_name    = "LINEAR_SOLVER_PROPERTIES_PRESSURE1"; 
  
  // NOD values
  pcs_number_of_primary_nvals = 0;
  pcs_number_of_secondary_nvals = 0;

  // 2 ELE values
  pcs_number_of_evals = 1;  
  pcs_eval_name[0] = "CONCENTRATION0";
  pcs_eval_unit[0]  = "kg/m3";
}



/**************************************************************************
FEMLib-Method: For non-isothermal multi-phase flow
Task: 
Programing:
02/2007 WW Implementation
**************************************************************************/
void CRFProcess::ConfigMultiPhaseFlow()
{
  dof = 2; 
  // 1.1 primary variables
  pcs_number_of_primary_nvals = 2;     
  pcs_primary_function_name[0] = "PRESSURE1"; 
  pcs_primary_function_unit[0] = "Pa";
  pcs_primary_function_name[1] = "PRESSURE2";
  pcs_primary_function_unit[1] = "Pa";
   // 1.2 secondary variables
  pcs_number_of_secondary_nvals = 0;
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 0;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "SATURATION1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m3/m3";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  // Nodal velocity. 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z1";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_X2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Y2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "VELOCITY_Z2";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
  pcs_secondary_function_name[pcs_number_of_secondary_nvals] = "PRESSURE_W";
  pcs_secondary_function_unit[pcs_number_of_secondary_nvals] = "m/s";
  pcs_secondary_function_timelevel[pcs_number_of_secondary_nvals] = 1;
  pcs_number_of_secondary_nvals++; 
}

//////////////////////////////////////////////////////////////////////////
// Configuration NOD
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
ROCKFLOW - Function: 
Task: Config node values 
Programming: 02/2003 OK Implementation
04/2004   WW   Modification for 3D problems 
last modified:
**************************************************************************/
void CRFProcess::ConfigNODValues1(void)
{ 
  int i;
  int pcs_nval=0;
  const int DOF = GetPrimaryVNumber(); 
  anz_nval0 = anz_nval;
  number_of_nvals = 2*DOF + pcs_number_of_secondary_nvals;
  //NVAL
  pcs_nval_data = (PCS_NVAL_DATA *) \
                  Malloc(number_of_nvals*sizeof(PCS_NVAL_DATA));
  /*----------------------------------------------------------------*/
  for(i=0;i<DOF;i++) {
    /* Primary variable - old time */
    //NVAL pcs_nval_data[pcs_nval] = (PCS_NVAL_DATA *) Malloc(sizeof(PCS_NVAL_DATA));
    strcpy(pcs_nval_data[pcs_nval].name,pcs_primary_function_name[pcs_nval-i]);
    //Change name for the previous time level
    /*
    char *ch = strchr(pcs_nval_data[pcs_nval].name, '1');
    if( ch != NULL )
    {
      int pos = ch-pcs_nval_data[pcs_nval].name; 
      pcs_nval_data[pcs_nval].name[pos]='0'; 
    }
    else
      strcat(pcs_nval_data[pcs_nval].name, "0");   */
    //-------------------------------------------------------------------------------   
    strcpy(pcs_nval_data[pcs_nval].einheit,pcs_primary_function_unit[pcs_nval-i]);
    pcs_nval_data[pcs_nval].timelevel = 0;
    pcs_nval_data[pcs_nval].speichern = 0;  //WW 
    pcs_nval_data[pcs_nval].laden = 0;
    pcs_nval_data[pcs_nval].restart = 1;
    pcs_nval_data[pcs_nval].adapt_interpol = 1;
    pcs_nval_data[pcs_nval].vorgabe = 0.0;
#ifdef PCS_NOD
	pcs_nval_data[pcs_nval].nval_index = pcs_nval;
#else
	pcs_nval_data[pcs_nval].nval_index = anz_nval + pcs_nval;
#endif
    pcs_nval++;
    /* Primary variable - new time */
    //NVAL pcs_nval_data[pcs_nval] = (PCS_NVAL_DATA *) Malloc(sizeof(PCS_NVAL_DATA));
    strcpy(pcs_nval_data[pcs_nval].name,pcs_primary_function_name[pcs_nval-i-1]);
    strcpy(pcs_nval_data[pcs_nval].einheit,pcs_primary_function_unit[pcs_nval-i-1]);
    pcs_nval_data[pcs_nval].timelevel = 1;
    pcs_nval_data[pcs_nval].speichern = 1;


    pcs_nval_data[pcs_nval].laden = 0;
    pcs_nval_data[pcs_nval].restart = 1;
    pcs_nval_data[pcs_nval].adapt_interpol = 1;
    pcs_nval_data[pcs_nval].vorgabe = 0.0;
#ifdef PCS_NOD
	pcs_nval_data[pcs_nval].nval_index = pcs_nval;
#else
	pcs_nval_data[pcs_nval].nval_index = anz_nval + pcs_nval;
#endif
    pcs_nval++;
  }

  /*----------------------------------------------------------------*/
  /* Secondary variables */
 for(i=0;i<pcs_number_of_secondary_nvals;i++) {
  //NVAL pcs_nval_data[pcs_nval] = (PCS_NVAL_DATA *) Malloc(sizeof(PCS_NVAL_DATA));
  strcpy(pcs_nval_data[pcs_nval].name,pcs_secondary_function_name[i]);
  strcpy(pcs_nval_data[pcs_nval].einheit,pcs_secondary_function_unit[i]);
//  pcs_nval_data[i+2]->timelevel = 1; // always at new time level
  pcs_nval_data[pcs_nval].timelevel = pcs_secondary_function_timelevel[i];
if(pcs_nval_data[pcs_nval].timelevel==1)
  pcs_nval_data[pcs_nval].speichern = 1;
else
  pcs_nval_data[pcs_nval].speichern = 0;
  pcs_nval_data[pcs_nval].laden = 0;
  pcs_nval_data[pcs_nval].restart = 1;
  pcs_nval_data[pcs_nval].adapt_interpol = 1;
  pcs_nval_data[pcs_nval].vorgabe = 0.0;
#ifdef PCS_NOD
  pcs_nval_data[pcs_nval].nval_index = pcs_nval;
#else
  pcs_nval_data[pcs_nval].nval_index = anz_nval + pcs_nval;
#endif
  if(type==4||type==41){ 
    if(dm_number_of_primary_nvals==2||(dm_number_of_primary_nvals==3&&this->type==41)){
      //Block:
      //STRESS_ZX, STRESS_YZ, STRAIN_ZX, STRAIN_ZY and LUMPED_STRESS 
      if(i==3||i==4||i==9||i==10||i==13)
        pcs_nval_data[pcs_nval].speichern = 0; 
      if(!problem_2d_plane_dm){
        //  Block STRESS_ZZ and STRAIN_ZZ
        if(i==5||i==11) pcs_nval_data[pcs_nval].speichern = 0;
      } 
       //if(!pcs_plasticity)
       // {   
       //   if(i==12) pcs_nval_data[pcs_nval].speichern = 0;  //STRAIN_PLS 
       // }      
    }
  }
  pcs_nval++;
 }
 pcs_nval_data = pcs_nval_data;
}

/*************************************************************************
ROCKFLOW - Function: CRFProcess::PCSConfigNODValues
Task: Config node values 
Programming: 02/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::ConfigNODValues2(void)
{
  int i;

  number_of_nvals = 2*GetPrimaryVNumber() + pcs_number_of_secondary_nvals;
  for(i=0;i<number_of_nvals;i++) {
    ModelsAddNodeValInfoStructure(pcs_nval_data[i].name,\
                                  pcs_nval_data[i].einheit,\
                                  pcs_nval_data[i].speichern,\
                                  pcs_nval_data[i].laden,\
                                  pcs_nval_data[i].restart,\
                                  pcs_nval_data[i].adapt_interpol,\
                                  pcs_nval_data[i].vorgabe);
  }
}

/*************************************************************************
ROCKFLOW - Function: 
Task: Create
Programming: 03/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::CreateNODValues(void)
{
  long i;
  for (i=0;i<NodeListSize();i++) {
    if (GetNode(i) != NULL) {
      CreateModelNodeData(i);
#ifdef PCS_NOD
      k = GetNode(i);
      k->values[pcs_number] = (double *)Malloc(number_of_nvals*sizeof(double));
#endif
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
int PCSGetNODValueIndex(string name,int timelevel)
{
  // PCS primary variables
  int pcs_vector_size =(int)pcs_vector.size();
  int i,j;
  CRFProcess *m_pcs = NULL;
  if(pcs_vector_size>0){
    for(i=0;i<pcs_vector_size;i++){
      m_pcs = pcs_vector[i];
      for(j=0;j<m_pcs->number_of_nvals;j++){
        if((name.compare(m_pcs->pcs_nval_data[j].name)==0) && \
           (m_pcs->pcs_nval_data[j].timelevel==timelevel))
           return m_pcs->pcs_nval_data[j].nval_index;
      }
    }
  }
  cout << "Error in PCSGetNODValueIndex: " << name << endl;
  return -1;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
07/2004 OK Implementation
**************************************************************************/
double PCSGetNODValue(long node,char* name,int timelevel)
{
  int nidx;
  nidx = PCSGetNODValueIndex(name,timelevel);
  return GetNodeVal(node,nidx);
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2004 OK Implementation
**************************************************************************/
void PCSSetNODValue(long node,string v_name,double value,int timelevel)
{
  SetNodeVal(node,PCSGetNODValueIndex(v_name,timelevel),value);
}

//////////////////////////////////////////////////////////////////////////
// Configuration ELE
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
ROCKFLOW - Function:
Task: Config element values 
Programming: 02/2003 OK Implementation
last modified:
  06/2004  WW  
**************************************************************************/
void CRFProcess::ConfigELEValues1(void)
{
  int i;
  if(pcs_number_of_evals)
    pcs_eval_data = (PCS_EVAL_DATA *) Malloc(pcs_number_of_evals*sizeof(PCS_EVAL_DATA));
  for(i=0;i<pcs_number_of_evals;i++) {
    //pcs_eval_data[i] = (PCS_EVAL_DATA *) Malloc(sizeof(PCS_EVAL_DATA));
    strcpy(pcs_eval_data[i].name,pcs_eval_name[i]);
    strcpy(pcs_eval_data[i].einheit,pcs_eval_unit[i]);
    pcs_eval_data[i].speichern = 1;
    pcs_eval_data[i].laden = 0;
    pcs_eval_data[i].restart = 1;
    pcs_eval_data[i].adapt_interpol = 1;
    pcs_eval_data[i].vorgabe = 0.0;
    pcs_eval_data[i].index = anz_eval + i;
    pcs_eval_data[i].eval_index = anz_eval + i; //SB
  }
}

/*************************************************************************
ROCKFLOW - Function:
Task: Config element values 
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::ConfigELEValues2(void)
{
  int i;
  for(i=0;i<pcs_number_of_evals;i++) {
    ModelsAddElementValInfoStructure(pcs_eval_data[i].name,\
                                     pcs_eval_data[i].einheit,\
                                     pcs_eval_data[i].speichern,\
                                     pcs_eval_data[i].laden,\
                                     pcs_eval_data[i].restart,\
                                     pcs_eval_data[i].adapt_interpol,\
                                     pcs_eval_data[i].vorgabe);
  }
}

/*************************************************************************
ROCKFLOW - Function: 
Task: Create
Programming: 03/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::CreateELEValues(void)
{
  long i;
  for (i=0;i<ElListSize();i++) {
    if (ElGetElement(i) != NULL) {
      ELECreateElementData(i);
    }
  }
}

/*************************************************************************
ROCKFLOW - Function:
Task: Create
Programming: 03/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::CreateELEGPValues(void)
{
  long i;
  for (i=0;i<ElListSize();i++) {
    if (ElGetElement(i) != NULL) {
      ELECreateGPValues(i);
    }
  }
}

/*************************************************************************
ROCKFLOW - Function: CRFProcess::PCSGetELEValueIndex
Task: Provide index for element values
Programming: 08/2003 SB Implementation
last modified:
**************************************************************************/
int PCSGetELEValueIndex(char *name)
{
  int i;
  CRFProcess *m_process = NULL;
  int j;
  int no_processes =(int)pcs_vector.size();
  for(j=0;j<no_processes;j++){
    m_process = pcs_vector[j];
    for(i=0;i<m_process->pcs_number_of_evals;i++) {
      if(strcmp(m_process->pcs_eval_data[i].name,name)==0)
           return m_process->pcs_eval_data[i].eval_index;
    }
  }
  printf("PCSGetELEValueIndex Alert\n");
  printf("%s \n",name);
  return -1;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2004 OK Implementation
**************************************************************************/
double PCSGetELEValue(long index,double*gp,double theta,string nod_fct_name)
{
  int nn;
  int nidx0,nidx1;
  double e_value;
  double nod_fct0,nod_fct1;
  int no_nodes;
  long* element_nodes = NULL;
  //----------------------------------------------------------------------
  nidx0 = PCSGetNODValueIndex(nod_fct_name,0);
  nidx1 = PCSGetNODValueIndex(nod_fct_name,1);
  //----------------------------------------------------------------------
  nod_fct0 = 0.0;
  nod_fct1 = 0.0;
  if(gp==NULL){ // Element average value
    no_nodes = ElNumberOfNodes[ElGetElementType(index)-1];
    element_nodes = ElGetElementNodes(index);
    for(nn=0;nn<no_nodes;nn++){
      nod_fct0 += GetNodeVal(element_nodes[nn],nidx0);
      nod_fct1 += GetNodeVal(element_nodes[nn],nidx1);
    }
    nod_fct0 /= no_nodes;
    nod_fct1 /= no_nodes;
  }
  else{ // Gauss point values
    nod_fct0 = InterpolValue(index,nidx0,gp[0],gp[1],gp[2]);
    nod_fct1 = InterpolValue(index,nidx1,gp[0],gp[1],gp[2]);
  }
  //----------------------------------------------------------------------
  e_value = (1.-theta)*nod_fct0 + theta*nod_fct1;
  return e_value;
}

//////////////////////////////////////////////////////////////////////////
// Configuration ELE matrices
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
ROCKFLOW - Function:
Task: Create
Programming: 03/2003 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::CreateELEMatricesPointer(void)
{
  long i;
  for (i=0;i<ElListSize();i++) {
    if (ElGetElement(i) != NULL) {
      ELECreateElementMatricesPointer(i,pcs_number);
    }
  }
}

/*************************************************************************
ROCKFLOW - Function: PCSConfigELEMatricesMPC
Task: Config element matrices: data access functions
Programming: 04/2003 OK Implementation
last modified:
todo: direct access possible !
**************************************************************************/

/*************************************************************************
ROCKFLOW - Function: CRFProcess::PCSConfigELEMatricesMTM
Task: Config elementa matrix pointer
Programming:	02/2003 OK Implementation
last modified:	08/2003 SB  Adapted to MTM2
**************************************************************************/

/*************************************************************************
ROCKFLOW - Function: PCSConfigELEMatricesHTM
Task: Config element matrices: data access functions
Programming: 04/2003 OK Implementation
last modified:
todo: direct access possible !
**************************************************************************/
/*
void PCSConfigELEMatricesHTM(int pcs_type_number)
{
  // Matrices pointer, allgemein
  InitInternElementData = HTMCreateELEMatricesPointer;
  ELEDestroyElementMatrices = HTMDestroyELEMatricesPointer;
  PCSDestroyELEMatrices[pcs_type_number] = HTMDestroyELEMatricesPointer;
}
*/
/**************************************************************************
FEMLib-Method: 
Task:  Activate or deactivate elements specified in .pcs file
Programing:
05/2005 WW Implementation
**************************************************************************/
void CRFProcess::CheckMarkedElement()
{
  int i, j;
  long l;  
  bool done;
  CElem *elem = NULL;
  CNode *node = NULL;
  for (l = 0; l < (long)m_msh->ele_vector.size(); l++)
  {
     elem = m_msh->ele_vector[l];
     done = false;
     for(i=0; i<NumDeactivated_SubDomains; i++)
     {
        if(elem->GetPatchIndex()== Deactivated_SubDomain[i]) 
        {
           elem->MarkingAll(false);
           done = true;
           break;
        } 
     } 
     if(done)
       continue;
     else 
       elem->MarkingAll(true);

  }
  for (l = 0; l < (long)m_msh->nod_vector.size(); l++)
  {
    while(m_msh->nod_vector[l]->connected_elements.size())
      m_msh->nod_vector[l]->connected_elements.pop_back();
  }
  for (l = 0; l < (long)m_msh->ele_vector.size(); l++)
  {
     elem = m_msh->ele_vector[l];
     if(!elem->GetMark()) continue;
     for(i=0; i<elem->GetNodesNumber(m_msh->getOrder()); i++)
     {
        done = false;
        node = elem->GetNode(i);
        for(j=0; j<(int)node->connected_elements.size(); j++)
        {
          if(l==node->connected_elements[j])
          {
             done = true;
             break;
          }
        }
        if(!done)  
           node->connected_elements.push_back(l);
     }

  }  //
}


//////////////////////////////////////////////////////////////////////////
// PCS Execution
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:
Programming: 
02/2003 OK Implementation
04/2003 OK Storing time step results
08/2004 OK PCS2
01/2005 WW new ELE concept
03/2005 OK MultiMSH concept 
06/2005 MB NEWTON error calculation
05/2007 WW DOF>1, unerrelaxation for Picard, and removing the old or spurious stuff
12/2007 WW Classes of sparse matrix (jagged diagonal storage) and linear solver 
           and parellelisation of them 
last modified:
**************************************************************************/
double CRFProcess::Execute()
{
  int nidx1;
  double pcs_error;
  double relax; //WW
  int i;
  long j, k=0, g_nnodes = 0; //07.01.07 WW

  //----------------------------------------------------------------------
#ifdef USE_MPI //WW
  long global_eqs_dim = pcs_number_of_primary_nvals*m_msh->GetNodesNumber(false);
  CPARDomain *dom = dom_vector[myrank];
  if(myrank==0)
#endif
   cout << "    ->Process " << pcs_number << ": " << pcs_type_name << endl;
  //----------------------------------------------------------------------
  // 0 Initialize equations
#ifdef NEW_EQS //WW
  if(!configured_in_nonlinearloop)
 #if defined(USE_MPI)
    dom->ConfigEQS(m_num, global_eqs_dim);
 #else  
    // Also allocate temporary memory for linear solver. WW
    eqs_new->ConfigNumerics(m_num);
  eqs_new->Initialize(); 
 #endif

#else
  // System matrix
  SetLinearSolverType(eqs, m_num); //WW
  SetZeroLinearSolver(eqs);
#endif
  //



  /*


    
    //TEST_MPI
 string test = "rank";
 char stro[1028];  
 sprintf(stro, "%d",myrank);
 string test1 = test+(string)stro+"Assemble.txt";
    ofstream Dum(test1.c_str(), ios::out); // WW
    dom->eqs->Write(Dum);   Dum.close();
    MPI_Finalize();
    exit(0);


  */







  // Solution vector
  //......................................................................
  relax = 0.0;
  if(m_num->nls_relaxation<DBL_MIN)
    relax = 0.0;
  else
    relax = 1.0-m_num->nls_relaxation; //WW
  if(NumDeactivated_SubDomains>0)
    CheckMarkedElement();
  m_msh->SwitchOnQuadraticNodes(false);
  for(i=0;i<pcs_number_of_primary_nvals;i++)
  {
    nidx1 = GetNodeValueIndex(pcs_primary_function_name[i]) + 1; //new time
    g_nnodes = m_msh->GetNodesNumber(false); //WW 
    for(j=0;j<g_nnodes;j++) //WW
#ifdef NEW_EQS
      // New EQS  WW
      eqs_new->x[j+i*g_nnodes] = GetNodeValue(m_msh->Eqs2Global_NodeIndex[j],nidx1);
#else
      eqs->x[j+i*g_nnodes] = GetNodeValue(m_msh->Eqs2Global_NodeIndex[j],nidx1);
#endif 
  } 
  /*---------------------------------------------------------------------*/
  /* 1 Calc element matrices */
if((aktueller_zeitschritt==1)||(tim_type_name.compare("TRANSIENT")==0)){
#ifdef USE_MPI //WW
  if(myrank==0)
#endif
  cout << "      Calculate element matrices" << endl;
#ifdef MFC
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  CString m_str_time;
  m_str_time.Format("Time step: t=%e sec",aktuelle_zeit);
  CString m_str_pcs = pcs_type_name.c_str();
  CString m_str = m_str_time + ", " + m_str_pcs + ", Calculate element matrices";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_str);
#endif
}
  /*---------------------------------------------------------------------*/
  /* 2 Assemble EQS */
#ifdef USE_MPI //WW
  if(myrank==0)
#endif
  cout << "      Assemble equation system" << endl;
#ifdef MFC
  CWnd *pWin = ((CWinApp *) AfxGetApp())->m_pMainWnd;
  CString m_str_time;
  m_str_time.Format("Time step: t=%e sec",aktuelle_zeit);
  CString m_str_pcs = pcs_type_name.c_str();
  CString m_str = m_str_time + ", " + m_str_pcs + ", Assemble equation system";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_str);
#endif
  // 
  // Assembly
#ifdef USE_MPI //WW
  clock_t cpu_time=0;  //WW
  cpu_time = -clock();
#endif
  //
  GlobalAssembly();
#ifdef USE_MPI
  cpu_time += clock();
  cpu_time_assembly += cpu_time;
#endif
  //
  //----------------------------------------------------------------------
  /* 5 Solve EQS */
//orig  cout << "      Solve equation system" << endl;
  // cout << "Solve equation system myrank =" <<myrank<<"of size="<<size<<endl;
#ifdef MFC
  m_str_time.Format("Time step: t=%e sec",aktuelle_zeit);
  m_str_pcs = pcs_type_name.c_str();
  m_str = m_str_time + ", " + m_str_pcs + ", Solve equation system";
  pWin->SendMessage(WM_SETMESSAGESTRING,0,(LPARAM)(LPCSTR)m_str);
#endif
#ifdef CHECK_EQS
  string eqs_name = pcs_type_name + "_EQS.txt";
  MXDumpGLS((char*)eqs_name.c_str(),1,eqs->b,eqs->x);
#endif
//.....................................................................
   // Execute linnear solver
   //
#ifdef NEW_EQS //WW
#if defined(USE_MPI)
   dom->eqs->Solver(eqs_new->x, global_eqs_dim); //21.12.2007
#else
   eqs_new->Solver(); //27.11.2007
#endif
#else
   ExecuteLinearSolver();
#endif
   //
//PCSDumpModelNodeValues();
  //----------------------------------------------------------------------
  // Error calculation 
  //----------------------------------------------------------------------
  // NEWTON
  if(m_num->nls_method_name.find("NEWTON")!=string::npos) {
    double Val;
    pcs_error = 0.0;

    // MSH data
    nidx1 = GetNodeValueIndex("HEAD")+1;
    //for(long nn=0;nn<NodeListSize(); nn++)  {
    for(long nn=0;nn<(long)m_msh->nod_vector.size();nn++)
    {
#ifdef NEW_EQS
       Val = GetNodeValue(nn,nidx1)+ eqs_new->x[nn];
#else 
       Val = GetNodeValue(nn,nidx1)+ eqs->x[nn];
#endif
	   pcs_error = max(pcs_error, fabs(Val - GetNodeValue(nn,nidx1)));
	   SetNodeValue(nn,nidx1, Val);
    }
#ifdef USE_MPI //WW
    if(myrank==0)
#endif 
	cout << "      PCS error: " << pcs_error << endl; //OK7
  } // NEWTON
  //----------------------------------------------------------------------
  else{ //PICARD
    //......................................................................
    pcs_error = CalcIterationNODError(1); //OK4105//WW4117    
#ifdef USE_MPI //WW
    if(myrank==0)
#endif 
       cout << "      PCS error: " << pcs_error << endl;
    //--------------------------------------------------------------------
    // 7 Store solution vector in model node values table
    //....................................................................
    for(i=0; i<pcs_number_of_primary_nvals; i++)
    {  
      nidx1 = GetNodeValueIndex(pcs_primary_function_name[i])+1;
      g_nnodes =m_msh->GetNodesNumber(false);  //DOF>1, WW
      for(j=0;j<g_nnodes;j++)
      {
         k = m_msh->Eqs2Global_NodeIndex[j];
#ifdef NEW_EQS
         SetNodeValue(k,nidx1,relax*GetNodeValue(k,nidx1)+(1.0-relax)*eqs_new->x[j+i*g_nnodes]);
#else
         SetNodeValue(k,nidx1,relax*GetNodeValue(k,nidx1)+(1.0-relax)*eqs->x[j+i*g_nnodes]);
#endif
      } 
    }
    //....................................................................
  } // END PICARD
  //----------------------------------------------------------------------
#ifdef NEW_EQS //WW
  if(!configured_in_nonlinearloop)
 #if defined(USE_MPI)
    dom->eqs->Clean();
 #else
    // Also allocate temporary memory for linear solver. WW
    eqs_new->Clean();
 #endif
#endif
  //----------------------------------------------------------------------
  return pcs_error;
}

/*************************************************************************
ROCKFLOW - Function:
Task: Initialize the equation system
Programming: 
 05/2003 OK/WW Implementation
 08/2003 WW    Changes for momolithic coupled equations    
last modified: 
**************************************************************************/
void CRFProcess::InitEQS()
{
    int i;
    eqs->unknown_vector_indeces = \
      (int*) Malloc(pcs_number_of_primary_nvals*sizeof(int));
    eqs->unknown_node_numbers = \
      (long*) Malloc(pcs_number_of_primary_nvals*sizeof(long));
    eqs->unknown_update_methods = \
       (int*) Malloc(pcs_number_of_primary_nvals*sizeof(long));
    
    for(i=0;i<pcs_number_of_primary_nvals;i++) {
      eqs->unknown_vector_indeces[i] = GetNodeValueIndex(pcs_primary_function_name[i])+1;
	  eqs->unknown_node_numbers[i] = m_msh->GetNodesNumber(true);
      eqs->unknown_update_methods[i] = 2; //For Newton-Raphson method. WW
      if(type==41)  // Monolithic scheme for p-u coupled equation
      {
         if(i==pcs_number_of_primary_nvals-1)
            eqs->unknown_node_numbers[i] = m_msh->GetNodesNumber(false);
         eqs->unknown_update_methods[i] = 0;
      }
    }
}

/*************************************************************************
ROCKFLOW - Function:
Task: Calculate element matrices
Programming: 
05/2003 OK Implementation
09/2005 OK gas flow removed
last modified:
**************************************************************************/
void CRFProcess::CalculateElementMatrices(void) 
{
  switch(this->type) {
    case 1: //SM
      break;
    case 2: //MTM2
      break;
    case 3: //HTM
      break;
    case 5: // Gas flow
      break;
    case 11:
      break;
    case 12: //MMP
      break;
    case 13: //MPC
      break;
    case 66: //OF
      break;
    default:
      DisplayMsgLn("CalculateElementMatrices: no CalculateElementMatrices specified");
      abort();
  }
}

/*************************************************************************
GeoSys-Function:
Task: Assemble the global system equation
Programming: 
01/2005 WW/OK Implementation
04/2005 OK MSH
07/2005 WW Change due to the geometry element objects applied
10/2005 OK DDC
11/2005 YD time step control
01/2006 OK/TK Tests
12/2007 WW Spase matrix class and condensation sequential and parallel loop
last modified:
**************************************************************************/
void CRFProcess::GlobalAssembly()
{
  //----------------------------------------------------------------------
  // Tests
  if(!Tim)
    Tim = TIMGet(pcs_type_name);
  if(!Tim)
  {
    cout << "Error in CRFProcess::GlobalAssembly() - no TIM data" << endl;
    return;
  }
  //......................................................................
  if(!fem)
  {
    // Which process needs this?
    // Only one instance of CFiniteElementStd is required for each process
    // Use "new" in such way will cause memery problem.
	// Please move this declaration to pcs configuration. 	  WW     
    if(m_msh)
      fem = new CFiniteElementStd(this,m_msh->GetCoordinateFlag()); 
  }
  //----------------------------------------------------------------------
  long i;
  CElem* elem = NULL;
  bool Check2D3D;
  Check2D3D = false;
  if(type == 66) //Overland flow
    Check2D3D = true;
  //----------------------------------------------------------------------
  // DDC
  if(dom_vector.size()>0){
    cout << "      Domain Decomposition" << '\n';
    CPARDomain* m_dom = NULL;
    int j = 0;
#if defined(USE_MPI) //WW
    j = myrank;
#else
    for(j=0;j<(int)dom_vector.size();j++)
    {
#endif
      m_dom = dom_vector[j];
      //
#ifdef NEW_EQS
      m_dom->InitialEQS(this); 
#else
      SetLinearSolver(m_dom->eqs);
      SetZeroLinearSolver(m_dom->eqs);
#endif
      for(i=0;i<(long)m_dom->elements.size();i++)
      {
        elem = m_msh->ele_vector[m_dom->elements[i]];
        if(elem->GetMark())
        {
          elem->SetOrder(false);
          fem->SetElementNodesDomain(m_dom->element_nodes_dom[i]); //WW  
          fem->ConfigElement(elem,Check2D3D);
          fem->m_dom = m_dom; //OK
          fem->Assembly();
        } 
      }
      // m_dom->WriteMatrix();
      //MXDumpGLS("rf_pcs.txt",1,m_dom->eqs->b,m_dom->eqs->x);
      //ofstream Dum("rf_pcs.txt", ios::out); // WW
      //m_dom->eqs->Write(Dum);
      //Dum.close();

      IncorporateSourceTerms(j);
      IncorporateBoundaryConditions(j);   

      /* 
      //TEST   
      string test = "rank";
      char stro[64];  
      sprintf(stro, "%d",j);
      string test1 = test+(string)stro+"Assemble.txt";
      ofstream Dum(test1.c_str(), ios::out);
      m_dom->eqs->Write(Dum);
      Dum.close(); 
      exit(0);
      */



#ifndef USE_MPI   
    }
    //....................................................................
    // Assemble global system
    DDCAssembleGlobalMatrix();
#endif
//	
//		MXDumpGLS("rf_pcs.txt",1,eqs->b,eqs->x); //abort();
  }
  //----------------------------------------------------------------------
  // STD
  else
  {   
    for(int ii=0;ii<(int)continuum_vector.size();ii++)   //YDTEST. Changed to DOF 15.02.2007 WW
    {
      continuum = ii;
      for(i=0;i<(long)m_msh->ele_vector.size();i++)
      {
        elem = m_msh->ele_vector[i];
        if (elem->GetMark()) // Marked for use
        {
           elem->SetOrder(false);
           fem->ConfigElement(elem,Check2D3D);
           fem->Assembly();
//-----------------------NEUMANN CONTROL---------
           if(Tim->time_control_name.compare("NEUMANN")==0)
           {
	         Tim->time_step_length_neumann = MMin(Tim->time_step_length_neumann,timebuffer);
             Tim->time_step_length_neumann *= 0.5*elem->GetVolume()*elem->GetVolume();
	         if(Tim->time_step_length_neumann < MKleinsteZahl)
               Tim->time_step_length_neumann = 1.0e-5;
	       }
//------------------------------   
         } 
       }
    }
	//
//  MXDumpGLS("rf_pcs1.txt",1,eqs->b,eqs->x); //abort();
    //eqs_new->Write();

    IncorporateSourceTerms();
#ifdef GEM_REACT
    if ( pcs_type_name.compare("MASS_TRANSPORT") == 0 && aktueller_zeitschritt > 1 && this->m_num->cpl_iterations > 1)
    IncorporateSourceTerms_GEMS();    
#endif
    SetCPL(); //OK
    IncorporateBoundaryConditions();
    //
    // 
    // ofstream Dum("rf_pcs.txt", ios::out); // WW
    // eqs_new->Write(Dum);   Dum.close();
    //
    //   MXDumpGLS("rf_pcs1.txt",1,eqs->b,eqs->x); abort();
  }
  //----------------------------------------------------------------------
}

/*************************************************************************
GeoSys-Function:
Task: Calculate integration point velocity
Programming: 
08/2005 WW Implementation
last modified:
**************************************************************************/
void CRFProcess::CalIntegrationPointValue()
{
  long i;
  CElem* elem = NULL;
  cal_integration_point_value = false;
  continuum = 0; //15.02.2007/
//  cal_integration_point_value = true;
// Currently, extropolation only valid for liquid and Richards flow.
  if(pcs_type_name.find("LIQUID")!=string::npos
     ||pcs_type_name.find("RICHARD")!=string::npos
     ||pcs_type_name.find("MULTI_PHASE_FLOW")!=string::npos
     ||pcs_type_name.find("GROUNDWATER_FLOW")!=string::npos )
     cal_integration_point_value = true;
  if(!cal_integration_point_value)
     return;
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
  {
    elem = m_msh->ele_vector[i];
    if (elem->GetMark()) // Marked for use
    {
      fem->ConfigElement(elem);
      // fem->m_dom = NULL; // To be used for parallization 
      fem->Cal_Velocity();
    } 
  }  
}


/*************************************************************************
ROCKFLOW - Function: AllocateLocalMatrixMemory
Task: As the function name
Programming: 
01/2005 WW/OK Implementation
06/2005 OK MSH
last modified:
**************************************************************************/
void CRFProcess::AllocateLocalMatrixMemory()
{
  long i;
  //----------------------------------------------------------------------
  int up_type = 0;
  if(!M_Process) up_type=0; 
  if(H_Process&&M_Process)
  { 
    if(type!=4&&type!=41)
      up_type=1;
    else
    {
      if(type==4)
        up_type = 3;
      if(type==41)  up_type = 4;
    }
  } 
  if(!H_Process) up_type = 2;
  //----------------------------------------------------------------------
  ElementMatrix *eleMatrix = NULL;
  CElem* elem = NULL;
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
  {
    elem = m_msh->ele_vector[i];
    if (elem->GetMark()) // Marked for use
    {
      eleMatrix = new ElementMatrix();
      eleMatrix->AllocateMemory(elem,up_type);
      Ele_Matrices.push_back(eleMatrix);
    }
  }
}

/*************************************************************************
FEMLib function
Task: Assemble global system matrix
Programming: 
05/2003 OK Implementation
??/???? WW Moved from AssembleSystemMatrixNew
05/2006 WW Modified to enable dealing with the case of DOF>1
06/2006 WW Take the advantege of sparse matrix to enhance simulation
10/2007 WW Change for the new classes of sparse matrix and linear solver
**************************************************************************/
void CRFProcess::DDCAssembleGlobalMatrix()
{
  int ii,jj, dof;
  long i,j,j0,ig,jg, ncol;
  CPARDomain *m_dom = NULL;
  long *nodes2node = NULL; //WW
  double *rhs = NULL, *rhs_dom = NULL;
  double a_ij;
  //double b_i=0.0;
  int no_domains =(int)dom_vector.size();
  long no_dom_nodes;
  dof = pcs_number_of_primary_nvals; //WW
  ncol = 0; //WW
#ifndef USE_MPI
  int k;
  for(k=0;k<no_domains;k++){
    m_dom = dom_vector[k];
#else
    m_dom = dom_vector[myrank];
#endif   
    // RHS
#if defined(NEW_EQS)
    rhs = eqs_new->b;
    if(type==4) 
      rhs_dom = m_dom->eqsH->b;
    else
      rhs_dom = m_dom->eqs->b;
#else
    rhs = eqs->b;
    rhs_dom = m_dom->eqs->b;
#endif

    no_dom_nodes = m_dom->nnodes_dom; //WW
    if(type==4||type==41)
        no_dom_nodes = m_dom->nnodesHQ_dom; //WW
    if(type==41) dof--;
    for(i=0;i<no_dom_nodes;i++){
      //------------------------------------------ 
      // Use the feature of sparse matrix of FEM
      //WW 
      ig = m_dom->nodes[i];  
      ncol = m_dom->num_nodes2_node[i]; 
      nodes2node =  m_dom->node_conneted_nodes[i];
      for(j0=0;j0<ncol;j0++){
        j=nodes2node[j0];  
        if(j>=no_dom_nodes) continue;
        jg = m_dom->nodes[j];
        //------------------------------------------ 
        // DOF loop ---------------------------WW
        for(ii=0; ii<dof; ii++) 
        { 
          for(jj=0; jj<dof; jj++)
          {
            // get domain system matrix
#ifdef  NEW_EQS  //WW
            if(type==4)
              a_ij = (*m_dom->eqsH->A)(i+no_dom_nodes*ii,j+no_dom_nodes*jj);
            else 
              a_ij = (*m_dom->eqs->A)(i+no_dom_nodes*ii,j+no_dom_nodes*jj);
            (*eqs_new->A)(ig+Shift[ii],jg+Shift[jj]) += a_ij;
#else
            SetLinearSolver(m_dom->eqs);
            a_ij = MXGet(i+no_dom_nodes*ii,j+no_dom_nodes*jj);
            // set global system matrix
            SetLinearSolver(eqs);
            MXInc(ig+Shift[ii],jg+Shift[jj],a_ij);
#endif
          }
        }
        // DOF loop ---------------------------WW
      }
      // set global RHS vector //OK
      for(ii=0; ii<dof; ii++) //WW
         rhs[ig+Shift[ii]] += rhs_dom[i+no_dom_nodes*ii];
    }

    // Mono HM------------------------------------WW
    if(type!=41)
#ifndef USE_MPI
    continue;
#else
    return;
#endif   
    no_dom_nodes = m_dom->nnodes_dom; 
    long  no_dom_nodesHQ = m_dom->nnodesHQ_dom;
    double a_ji = 0.0;
    for(i=0;i<no_dom_nodes;i++){
      ig = m_dom->nodes[i]; //WW
      ncol = m_dom->num_nodes2_node[i]; 
      nodes2node =  m_dom->node_conneted_nodes[i];
      for(j0=0;j0<ncol;j0++){
        j=nodes2node[j0];  
        jg = m_dom->nodes[j];
        for(ii=0; ii<dof; ii++) //ww
        { 
#if defined(NEW_EQS)
            // dom to global. WW
            a_ij = (*m_dom->eqsH->A)(i+no_dom_nodesHQ*dof,j+no_dom_nodesHQ*ii);
            a_ji = (*m_dom->eqsH->A)(j+no_dom_nodesHQ*ii, i+no_dom_nodesHQ*dof);
            (*eqs_new->A)(ig+Shift[ii],jg+Shift[problem_dimension_dm]) += a_ij; 
            (*eqs_new->A)(jg+Shift[problem_dimension_dm],ig+Shift[ii]) += a_ji; 
#else 
            // get domain system matrix
            SetLinearSolver(m_dom->eqs);
            a_ij = MXGet(i+no_dom_nodesHQ*dof,j+no_dom_nodesHQ*ii);
            a_ji = MXGet(j+no_dom_nodesHQ*ii, i+no_dom_nodesHQ*dof);
            // set global system matrix
            SetLinearSolver(eqs);
            MXInc(ig+Shift[ii],jg+Shift[problem_dimension_dm],a_ij);
            MXInc(jg+Shift[problem_dimension_dm],ig+Shift[ii],a_ji);
#endif
        }
      }
    }    
    for(i=0;i<no_dom_nodes;i++)
    {
      ig = m_dom->nodes[i];
      ncol = m_dom->num_nodes2_node[i]; 
      nodes2node =  m_dom->node_conneted_nodes[i];
      for(j0=0;j0<ncol;j0++){
        j=nodes2node[j0];  
        jg = m_dom->nodes[j];
        if(jg>=no_dom_nodes) continue;
        // get domain system matrix
#if defined(NEW_EQS)
        // dom to global. WW
        a_ij = (*m_dom->eqsH->A)(i+no_dom_nodesHQ*dof,j+no_dom_nodesHQ*dof);
        (*eqs_new->A)(ig+Shift[problem_dimension_dm],jg+Shift[problem_dimension_dm])
           += a_ij;
#else 
        SetLinearSolver(m_dom->eqs);
        a_ij = MXGet(i+no_dom_nodesHQ*dof,j+no_dom_nodesHQ*dof);
        // set global system matrix
        SetLinearSolver(eqs);
        MXInc(ig+Shift[problem_dimension_dm],jg+Shift[problem_dimension_dm],a_ij);
#endif
      }
      //
      rhs[ig+Shift[problem_dimension_dm]] += rhs_dom[i+no_dom_nodesHQ*dof];
    }    
    // Mono HM------------------------------------WW  
#ifndef USE_MPI
  }
#endif


}

/*************************************************************************
ROCKFLOW - Function:
Task: Assemble system matrix
Programming: 05/2003 OK Implementation
ToDo: Prototyp function
last modified:
**************************************************************************/
void CRFProcess::AssembleSystemMatrixNew(void) 
{
  switch(type) {
    case 1:
      //MakeGS_ASM_NEW(eqs->b,eqs->x,ddummy);
      // SMAssembleMatrix(eqs->b,eqs->x,ddummy,this);
      break;
    case 2: //MTM2
      break;
    case 3: //HTM
      break;
    case 5: // Gas flow
      break;
    case 11:
      break;
    case 12:
      break;
    case 13:
      break;
    case 66:
      //MakeGS_ASM_NEW(eqs->b,eqs->x,ddummy);
      // SMAssembleMatrix(eqs->b,eqs->x,ddummy,this);
      break;
    default:
      DisplayMsgLn("CalculateElementMatrices: no CalculateElementMatrices specified");
      abort();
  }
#ifdef PARALLEL
  DDCAssembleGlobalMatrix();
#else
  IncorporateSourceTerms();
  IncorporateBoundaryConditions();
#endif
//SetLinearSolver(eqs);
//MXDumpGLS("global_matrix_dd.txt",1,eqs->b,eqs->x);
}
/**************************************************************************
FEMLib-Method: CRFProcess::IncorporateBoundaryConditions
Task: set PCS boundary conditions
Programing:
05/2006 WW Implementation
**************************************************************************/
void CRFProcess::SetBoundaryConditionSubDomain()
{
  int k;
  long i,j;
  CPARDomain *m_dom = NULL;
  CBoundaryConditionNode *m_bc_nv = NULL;
  CNodeValue *m_st_nv = NULL;
  //
  for(k=0;k<(int)dom_vector.size();k++)
  {
      m_dom = dom_vector[k];
      // BC
      for(i=0; i<(long)bc_node_value.size(); i++)
	  {
         m_bc_nv = bc_node_value[i];
		 for(j=0; j<(long)m_dom->nodes.size(); j++)
		 {
             if(m_bc_nv->geo_node_number==m_dom->nodes[j])
			 {
                bc_node_value_in_dom.push_back(i);
                bc_local_index_in_dom.push_back(j);
                break;
			 } 
         }
	  }
      rank_bc_node_value_in_dom.push_back((long)bc_node_value_in_dom.size());
      // ST
      for(i=0; i<(long)st_node_value.size(); i++)
	  {
         m_st_nv = st_node_value[i];
         for(j=0; j<(long)m_dom->nodes.size(); j++)
		 {
             if(m_st_nv->geo_node_number==m_dom->nodes[j])
			 {
                st_node_value_in_dom.push_back(i);
                st_local_index_in_dom.push_back(j);
                break;
			 } 
         }
	  }
      rank_st_node_value_in_dom.push_back((long)st_node_value_in_dom.size());
  }  
  long Size = (long)st_node_value.size();
  long l_index;
  for(i=0; i<Size; i++)
  {
     l_index = st_node_value[i]->geo_node_number;
	 st_node_value[i]->node_value /= (long)node_connected_doms[l_index];
  }

}

/**************************************************************************
FEMLib-Method: CRFProcess::IncorporateBoundaryConditions
Task: set PCS boundary conditions
Programing:
02/2004 OK Implementation
????    WW  and time curve
04/2005 OK MSH
05/2005 OK conditional BCs
04/2006 WW Changes due to the geometry object applied
04/2006 OK Conditions by PCS coupling OK
05/2006 WW Re-implement
05/2006 WW DDC
10/2007 WW Changes for the new classes of sparse matrix and linear solver
last modification:
**************************************************************************/
void CRFProcess::IncorporateBoundaryConditions(const int rank)
{
  static long i;
  static double bc_value, fac = 1.0, time_fac = 1.0;
  long bc_msh_node=-1;
  long bc_eqs_index, shift;
  int interp_method=0;
  int curve, valid=0;
  int ii, idx0, idx1; 
  CBoundaryConditionNode* m_bc_node; //WW 
  CBoundaryCondition* m_bc; //WW
  CPARDomain *m_dom = NULL;
  CFunction* m_fct = NULL; //OK
  double *eqs_rhs = NULL;
  bool is_valid = false; //OK
#ifdef NEW_EQS
  Linear_EQS *eqs_p = NULL;  
#endif
  //------------------------------------------------------------WW
  // WW 
  double Scaling = 1.0; 
  if(type==4||type==41) fac = Scaling;

  long begin = 0;
  long end = 0;
  long gindex=0;
  if(rank==-1)
  {
     begin = 0;
     end = (long)bc_node_value.size();
#ifdef NEW_EQS       //WW
     eqs_p = eqs_new;
     eqs_rhs = eqs_new->b; //27.11.2007 WW   
#else
     eqs_rhs = eqs->b;
#endif
  } 
  else
  {
     m_dom = dom_vector[rank];
#ifdef NEW_EQS
     eqs_p = m_dom->eqs;     
     if(type == 4 ) //WW
     {
       eqs_p = m_dom->eqsH;     
       eqs_rhs = m_dom->eqsH->b;
     }
     else
       eqs_rhs = m_dom->eqs->b; 
#else
     eqs_rhs = m_dom->eqs->b; 
#endif
     if(rank==0) 
        begin = 0;
     else 
        begin = rank_bc_node_value_in_dom[rank-1];
        end = rank_bc_node_value_in_dom[rank];
  }

  for(i=begin;i<end;i++) {
     gindex = i;
     if(rank>-1)
       gindex = bc_node_value_in_dom[i];
     m_bc_node = bc_node_value[gindex];
     m_bc = bc_node[gindex];
     shift = m_bc_node->msh_node_number-m_bc_node->geo_node_number;     
     //
     if(rank>-1)
     {
        bc_msh_node = bc_local_index_in_dom[i];
        int dim_space = 0; 
        if(m_msh->NodesNumber_Linear==m_msh->NodesNumber_Quadratic)
           dim_space = 0;  
        else
        {
          if(shift%m_msh->NodesNumber_Quadratic==0)
            dim_space = shift/m_msh->NodesNumber_Quadratic; 
          else
            dim_space = m_msh->msh_max_dim; 
        }
        shift = m_dom->shift[dim_space];
      } 
      else
        bc_msh_node = m_bc_node->geo_node_number;
  //------------------------------------------------------------WW
     if(m_msh) //OK
//	    if(!m_msh->nod_vector[bc_msh_node]->GetMark()) //WW
//          continue;
      time_fac = 1.0;
      if(bc_msh_node>=0){
        //................................................................
        // Time dependencies - CURVE
        curve =  m_bc_node->CurveIndex;
        if(curve>0){
          time_fac = GetCurveValue(curve,interp_method,aktuelle_zeit,&valid);
          if(!valid) continue;
	    }
	    else
          time_fac = 1.0;
        //................................................................
        // Time dependencies - FCT
        if(m_bc_node->fct_name.length()>0){
          m_fct = FCTGet(m_bc_node->fct_name);
          if(m_fct){
            time_fac = m_fct->GetValue(aktuelle_zeit,&is_valid);
            //if(!valid) continue;
          }
          else
            cout << "Warning in CRFProcess::IncorporateBoundaryConditions - no FCT data" << endl;
        }
        //................................................................
        // Conditions
        if(m_bc_node->conditional)
        {  
           int idx_1 = -1;    //28.2.2007 WW       
           for(ii=0;ii<dof;ii++)  //28.2.2007 WW    
           {
              if(m_bc->pcs_pv_name.find(pcs_primary_function_name[ii]) != string::npos)
              {                     
                 idx_1 = GetNodeValueIndex(pcs_primary_function_name[ii])+1;
                 break;
              }
           }           
           bc_value = time_fac*fac* GetNodeValue(m_bc_node->msh_node_number_subst, idx_1);
                      //WW  bc_value = time_fac*fac* GetNodeVal(bc_msh_node+1,GetNODValueIndex(pcs_primary_function_name[0])+1); // YD-----TEST---
        }
        else
          bc_value = time_fac*fac*m_bc_node->node_value; // time_fac*fac*PCSGetNODValue(bc_msh_node,"PRESSURE1",0);
	    
		if(m_bc->periodic) // JOD
		   bc_value *= sin( 2 * 3.14159 * aktuelle_zeit / m_bc->periode_time_length  + m_bc->periode_phase_shift);
        //----------------------------------------------------------------
        // MSH
        if(m_msh){//OK  
          if(rank>-1)
             bc_eqs_index = bc_msh_node;
          else
             bc_eqs_index = m_msh->nod_vector[bc_msh_node]->GetEquationIndex();  //WW#
          //..............................................................
          // NEWTON WW
        if(m_num->nls_method_name.find("NEWTON")!=string::npos
           ||	type==4||type==41	 ) {  //Solution is in the manner of increment ! 
            idx0 = GetNodeValueIndex(m_bc->pcs_pv_name.c_str());
            if(type==4||type==41)  
            {
              idx1 = idx0+1;
              bc_value -=  GetNodeValue(m_bc_node->geo_node_number, idx0)
                         + GetNodeValue(m_bc_node->geo_node_number, idx1);               
            }
            else
              bc_value = bc_value - GetNodeValue(m_bc_node->geo_node_number, idx0);
         }
       }
        //----------------------------------------------------------------
        // RFI
        else {
          bc_eqs_index = GetNodeIndex(bc_msh_node);
          //..............................................................
          // NEWTON
          if(m_num->nls_method_name.find("NEWTON")!=string::npos){  //Solution vector is the increment !
            idx0 = PCSGetNODValueIndex(m_bc->pcs_pv_name.c_str(),0);
            if(type==4||type==41)  
            {
              idx1 = PCSGetNODValueIndex(m_bc->pcs_pv_name.c_str(),1);
              bc_value -= GetNodeVal(bc_eqs_index, idx0) + GetNodeVal(bc_eqs_index, idx1);               
            }
            else
              bc_value = bc_value - GetNodeVal(bc_eqs_index, idx0);
            }
        }
        //----------------------------------------------------------------
        bc_eqs_index += shift;
	
        //YD dual 
        if(dof>1) //WW
        { 
          for(ii=0;ii<dof;ii++)
          {
            if(m_bc->pcs_pv_name.find(pcs_primary_function_name[ii]) != string::npos)
            {    
               //YD/WW
               //WW   bc_eqs_index += ii*(long)eqs->dim/dof;   //YD dual //DOF>1 WW 
               bc_eqs_index += fem->NodeShift[ii];   //DOF>1 WW 

               break;
            }
          }
        }
#ifdef NEW_EQS       //WW
        eqs_p->SetKnownX_i(bc_eqs_index, bc_value);
#else
        MXRandbed(bc_eqs_index,bc_value,eqs_rhs);
#endif
      }
  }
    //-----------------------------------------------------------------------
  /* irreg. Zeilen/Spalten regularisieren */
/*
  else if (GetNodeState(NodeNumber[i]) == -2 || GetNodeState(NodeNumber[i]) == -4) { // irreg.Knoten
    if (GetRFControlGridAdapt())
      if (AdaptGetMethodIrrNodes() == 1) {
        MXSet(i, i, MKleinsteZahl);
        rechts[i] = 0.0;
      }
    }
  }
*/

}

/**************************************************************************
FEMLib-Method:
Task: PCS source terms into EQS
Programing:
04/2004 OK Implementation
08/2004 WW Extension for monolithic PCS and time curve
last modification:
02/2005 MB River Condition and CriticalDepth
05/2005 WW Dynamic problems
07/2005 WW Changes due to the geometry object applied
03/2006 WW Re-arrange
04/2006 OK CPL
05/2006 WW DDC
08/2006 YD FCT use
**************************************************************************/
void CRFProcess::IncorporateSourceTerms(const int rank)
{
  double value = 0, fac = 1.0, time_fac;
  int interp_method=0;
  int curve, valid=0;
  long msh_node, shift;
  long bc_eqs_index=-1;
  int  ii, EleType;
  double q_face=0.0; 
  CElem* elem = NULL;
  CElem* face = NULL;
  ElementValue* gp_ele =NULL;
  CPARDomain *m_dom = NULL;
  double *eqs_rhs=NULL;
  double vel[3];   
  bool is_valid; //YD
  CFunction* m_fct = NULL; //YD
  long i; //, group_vector_length; 
  fac = 1.0;
  double Scaling = 1.0;
  if(type==4) fac = Scaling;

  CNodeValue *cnodev = NULL;
  CSourceTerm *m_st = NULL;
  //
  long begin = 0;
  long end = 0;
  long gindex=0;
  if(rank==-1)
  {
	 begin = 0;
	 end = (long)st_node_value.size();
#ifdef NEW_EQS       //WW
     eqs_rhs = eqs_new->b; //27.11.2007 WW   
#else
     eqs_rhs = eqs->b;
#endif 
  } 
  else
  {
     m_dom = dom_vector[rank];
#ifdef NEW_EQS
     if(type == 4 )
       eqs_rhs = m_dom->eqsH->b;
     else
       eqs_rhs = m_dom->eqs->b;  
#else
     eqs_rhs = m_dom->eqs->b; 
#endif
     if(rank==0) 
        begin = 0;
	 else 
        begin = rank_st_node_value_in_dom[rank-1];
	 end = rank_st_node_value_in_dom[rank];
  }
  
  for(i=begin;i<end;i++) 
  {
    gindex = i;
    if(rank>-1)
      gindex = st_node_value_in_dom[i];

	cnodev = st_node_value[gindex]; 
    shift = cnodev->msh_node_number-cnodev->geo_node_number;
    if(rank>-1)
	{
       msh_node = st_local_index_in_dom[i];
       int dim_space = 0; 
	   if(m_msh->NodesNumber_Linear==m_msh->NodesNumber_Quadratic)
          dim_space = 0;  
	   else
       {
          if(shift%m_msh->NodesNumber_Quadratic==0)
            dim_space = shift/m_msh->NodesNumber_Quadratic; 
		  else
            dim_space = m_msh->msh_max_dim; 
       }
	   shift = m_dom->shift[dim_space];
	} 
	else
	{ 
       msh_node = cnodev->msh_node_number;
       msh_node -= shift;
	} 
    value = cnodev->node_value;
    //--------------------------------------------------------------------
    // Tests
    if(msh_node<0) continue; 
    m_st = NULL;
    if(st_node.size()>0&&(long)st_node.size()>i)
	{
      m_st = st_node[gindex]; 
      //--------------------------------------------------------------------
      // CPL
      //if(m_st->pcs_type_name_cond.size()>0) continue; // this is a CPL source term, JOD removed
      //--------------------------------------------------------------------
      // system dependent YD
      if(cnodev->node_distype==7)
      {      
        long no_st_ele = (long)m_st->element_st_vector.size();
        for(long i_st=0;i_st<no_st_ele;i_st++)
        {
          long ele_index = m_st->element_st_vector[i_st];
          elem = m_msh->ele_vector[ele_index];
          if (elem->GetMark())
          { 
            fem->ConfigElement(elem);
            fem->Cal_Velocity(); 
          }
          gp_ele = ele_gp_value[ele_index];
          gp_ele->GetEleVelocity(vel);
          EleType = elem->GetElementType();
          if(EleType==1)   //Line
            cnodev->node_value += vel[0]; 
          if(EleType==4||EleType==2) //Traingle & Qua
          {   
            for(long i_face=0;i_face < (long)m_msh->face_vector.size();i_face++)
            {
              face = m_msh->face_vector[i_face];
              if(m_st->element_st_vector[i_st] == face->GetOwner()->GetIndex())
                q_face = PointProduction(vel,m_msh->face_normal[i_face])*face->GetVolume();   //   
                     //for(i_node) 
            }
            cnodev->node_value =+ q_face/2;
          }
          // cout<<"  value  "<<m_st_group->group_vector[i]->node_value<<endl;
        }
      }
      //--------------------------------------------------------------------
      // MB
      //if(m_st->conditional && !m_st->river)
      //{

        GetNODValue(value, cnodev, m_st);


    }	// st_node.size()>0&&(long)st_node.size()>i
   //----------------------------------------------------------------------------------------
   //--------------------------------------------------------------------
   // Please do not move the this section
   curve = cnodev->CurveIndex;
   if(curve>0) 
   {
      time_fac = GetCurveValue(curve,interp_method,aktuelle_zeit,&valid);
      if(!valid)  
      {
          cout<<"\n!!! Time dependent curve is not found. Results are not guaranteed "<<endl;
          cout<<" in void CRFProcess::IncorporateSourceTerms(const double Scaling)"<<endl;
          time_fac = 1.0;
       }
    }
    else time_fac = 1.0;

    // Time dependencies - FCT    //YD
    if(m_st) //WW
	{
      if(m_msh&&m_msh->geo_name.find("LOCAL")!=string::npos) //WW/YD //OK
      {
        if(m_st->fct_name.length()>0)
        {
          m_fct = FCTGet(pcs_number);
          if(m_fct)
             time_fac = m_fct->GetValue(aktuelle_zeit,&is_valid);
           else
             cout << "Warning in CRFProcess::IncorporateSourceTerms - no FCT data" << endl;
         }
       }
       else 
      {
        if(m_st->fct_name.length()>0)
        {
           m_fct = FCTGet(m_st->fct_name);
           if(m_fct)
           {
             time_fac = m_fct->GetValue(aktuelle_zeit,&is_valid);
           }
           else
           {
             cout << "Warning in CRFProcess::IncorporateSourceTerms - no FCT data" << endl;
           }
         }
	  }
    }
    //----------------------------------------------------------------------------------------
    value *= time_fac*fac; 
    //------------------------------------------------------------------
    // EQS->RHS
    if(m_msh) //WW
    {
      if(rank>-1)
        bc_eqs_index = msh_node+shift;   
      else
        bc_eqs_index = m_msh->nod_vector[msh_node]->GetEquationIndex()+shift;   
    } 
    else 
      bc_eqs_index = GetNodeIndex(msh_node)+shift;
    if(dof>1) // 07.2.2007 WW
    { 
      for(ii=0;ii<dof;ii++)
      {
         if(m_st->pcs_pv_name.find(pcs_primary_function_name[ii]) != string::npos)
	     {
            //WW            bc_eqs_index += ii*(long)eqs->dim/dof;   //dual  YDTEST
            bc_eqs_index += fem->NodeShift[ii];   //dual  YDTEST
            break;
	     }
      }
    }
    eqs_rhs[bc_eqs_index] += value;
  }
  //====================================================================
}
#ifndef NEW_EQS //WW
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2004 OK Implementation
11/2004 OK NUM
07/2006 WW Parallel BiCGStab
last modification:
**************************************************************************/
int CRFProcess::ExecuteLinearSolver(void)
{
  long iter_count;
  long iter_sum = 0;
  //WW  int found = 0;
  //-----------------------------------------------------------------------
  // Set EQS
  //cout << "Before SetLinearSolver(eqs) myrank = "<< myrank<< '\n'; 
  SetLinearSolver(eqs);
  //--------------------------------------------------------------------
  // NUM
  //WW  found = 1;
  cg_maxiter        = m_num->ls_max_iterations; //OK lsp->maxiter;
  cg_eps            = m_num->ls_error_tolerance; //OK lsp->eps;
  //cg_repeat = lsp->repeat;
  vorkond           = m_num->ls_precond; //OK lsp->precond;
  linear_error_type = m_num->ls_error_method; //OK lsp->criterium;

  //  cout << "Before eqs->LinearSolver(eqs) myrank = "<< myrank<< '\n'; 

#ifdef USE_MPI 
  //WW
  long dim_eqs = 0; 
  if(type==41||type==4) // DOF >1 
  {
     dom_vector[myrank]->quadratic = true;
     if(type==4)
        dim_eqs = pcs_number_of_primary_nvals*m_msh->GetNodesNumber(true);
     else if(type==41)
        dim_eqs = pcs_number_of_primary_nvals*m_msh->GetNodesNumber(true)
                 +m_msh->GetNodesNumber(false);
  } 
  else
  {
      dom_vector[myrank]->quadratic = false; 
      dim_eqs = m_msh->GetNodesNumber(false);
  }
  iter_count =  SpBICGSTAB_Parallel(dom_vector[myrank], eqs->x, dim_eqs);
#else  
  iter_count = eqs->LinearSolver(eqs->b,eqs->x,eqs->dim);
#endif

  eqs->master_iter = iter_count;
  if(iter_count>=cg_maxiter) {
    cout << "Warning in CRFProcess::ExecuteLinearSolver() - Maximum iteration number reached" << endl;
    return -1;
  }
  iter_sum += iter_count;
  //-----------------------------------------------------------------------
  // Clean results ?
/*
  for (i=0;i<eqs->dim;i++)
    if (fabs(eqs->x[i])<MKleinsteZahl)
      eqs->x[i] = 0.0;
*/
  //-----------------------------------------------------------------------
  return iter_sum;
}
#endif 
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
06/2005 PCH Overriding
last modification:
**************************************************************************/
int CRFProcess::ExecuteLinearSolver(LINEAR_SOLVER *eqs)
{
  long iter_count;
  long iter_sum = 0;
  int found = 0;
  //-----------------------------------------------------------------------
  // Set EQS
  SetLinearSolver(eqs);
  //--------------------------------------------------------------------
  // NUM
  found = 1;
  cg_maxiter        = m_num->ls_max_iterations; //OK lsp->maxiter;
  cg_eps            = m_num->ls_error_tolerance; //OK lsp->eps;
  //cg_repeat = lsp->repeat;
  vorkond           = m_num->ls_precond; //OK lsp->precond;
  linear_error_type = m_num->ls_error_method; //OK lsp->criterium;

  iter_count = eqs->LinearSolver(eqs->b,eqs->x,eqs->dim);
  eqs->master_iter = iter_count;
  if(iter_count>=cg_maxiter) {
    cout << "Warning in CRFProcess::ExecuteLinearSolver() - Maximum iteration number reached" << endl;
    return -1;
  }
  iter_sum += iter_count;
  //-----------------------------------------------------------------------
  return iter_sum;
}

//WW
int CRFProcess::GetNODValueIndex(string name,int timelevel)
{
  for(int j=0;j<number_of_nvals;j++){
     if((name.compare(pcs_nval_data[j].name)==0) && \
         (pcs_nval_data[j].timelevel==timelevel))
        return pcs_nval_data[j].nval_index;
   }
  cout << "Error in PCSGetNODValueIndex: " << name << endl;
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Specials
///////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------------
ROCKFLOW - Function: PCSRestart
Task: Insert process to list
Programming: 
06/2003 OK Implementation
11/2004 OK file_name_base
last modified:
-------------------------------------------------------------------------*/
void PCSRestart()
{
  int j;
  CRFProcess *m_pcs = NULL;
//  int timelevel;
  int nidx0,nidx1;
  int i;
  int no_processes =(int)pcs_vector.size();
  if(no_processes==0)
    return; //OK41
  int ok = 0;
  //----------------------------------------------------------------------
  string file_name_base = pcs_vector[0]->file_name_base;
//OK  ok = ReadRFRRestartData(file_name_base);
  if(ok==0){
    cout << "RFR: no restart data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    for(j=0;j<m_pcs->GetPrimaryVNumber();j++) {
      // timelevel=0;
      nidx0 = m_pcs->GetNodeValueIndex(m_pcs->GetPrimaryVName(j));
      // timelevel= 1;
      nidx1 = nidx0+1;
      CopyNodeVals(nidx1,nidx0);
    }
  }
}

/**************************************************************************
FEMLib-Method: 
Task: Relocate Deformation process
Programing:
09/2004 WW Implementation
**************************************************************************/
void RelocateDeformationProcess(CRFProcess *m_pcs)
{
   string pcs_name_dm = m_pcs->pcs_type_name; 
   string num_type_name_dm; 
   // Numerics
   if(m_pcs->num_type_name.compare("STRONG_DISCONTINUITY")==0) 
   {
       num_type_name_dm = m_pcs->num_type_name;
       enhanced_strain_dm=1;
   }
   delete m_pcs;
   m_pcs = dynamic_cast<CRFProcess *> (new CRFProcessDeformation());
   m_pcs->pcs_type_name = pcs_name_dm;
   if(enhanced_strain_dm==1) m_pcs->num_type_name = num_type_name_dm;
   pcs_deformation = 1;
}

/*************************************************************************
ROCKFLOW - Function: CreateFDMProcess
Task:
Programming: 
06/2004 OK Implementation
06/2004 OK 1-D FDM
last modified:
**************************************************************************/
void CRFProcess::CreateFDMProcess()
{
  long i;
  //----------------------------------------------------------------------
  // create FDM nodes and relationships
  start_new_elems = ElListSize();
  ConstructElemsToNodesList();
  //============================================================================
  CFDMNode *m_FDM_node;
/*
  long *elems1d;
  int num_elems1d;
  long *nod_neighbors;
  long *ele_neighbors;
  Knoten *node = NULL;
  long *element_nodes;
  for(i=0;i<NodeListLength;i++){
    node = GetNode(i);
    if(node->anz1d>0){
      elems1d = GetNode1DElems(i,&num_elems1d);
      nod_neighbors = new long[num_elems1d];
      ele_neighbors = elems1d;
      l=0;
      for(j=0;j<num_elems1d;j++){
        element_nodes = ElGetElementNodes(elems1d[j]);
        for(k=0;k<ElGetElementNodesNumber(elems1d[j]);k++) {
          if(element_nodes[k]!=i){
            nod_neighbors[l]=element_nodes[k];
            l++;
          }
        }
      }
      m_FDM_node = new CFDMNode;
      m_FDM_node->node = i;
      m_FDM_node->nod_neighbors = nod_neighbors;
      m_FDM_node->ele_neighbors = ele_neighbors;
      m_FDM_node->no_neighbors = num_elems1d;
      FDM_node_vector.push_back(m_FDM_node);
    }
  }
*/
  //----------------------------------------------------------------------------
  long *nodes_sorted;
  CGLPolyline *m_polyline = NULL;
  m_polyline = GEOGetPLYByName("RIVER");//CC
if(m_polyline){
  //ToDo nodes_sorted = m_polyline->GetMHSNodesSorted(nodes_unsorted,&no_nodes);
  long no_nodes = 0;
  long *nodes_unsorted = NULL;
   // encapsulate sort function
///OK  nodes_unsorted = m_polyline->MSHGetNodesClose(&no_nodes);
  double pt1[3],pt2[3];
  pt1[0] = m_polyline->point_vector[0]->x;
  pt1[1] = m_polyline->point_vector[0]->y;
  pt1[2] = m_polyline->point_vector[0]->z;
  double *node_distances = NULL;
  node_distances = new double[no_nodes];
  for(i=0;i<no_nodes;i++) {
    pt2[0] = GetNodeX(nodes_unsorted[i]);
    pt2[1] = GetNodeY(nodes_unsorted[i]);
    pt2[2] = GetNodeZ(nodes_unsorted[i]);
    node_distances[i] = MCalcDistancePointToPoint(pt1,pt2);
  }
  nodes_sorted = TOLSortNodes1(nodes_unsorted,node_distances,no_nodes);
  delete [] node_distances;
  //delete [] nodes_unsorted;
  for(i=0;i<no_nodes;i++){
    m_FDM_node = new CFDMNode;
    m_FDM_node->node = nodes_sorted[i];
    FDM_node_vector.push_back(m_FDM_node);
  }
  //delete [] nodes_sorted;

}
  WriteFDMNodes("river");
  //----------------------------------------------------------------------------
  // PCS
  Config();
  //----------------------------------------------------------------------------
  // TIM
  int timelevel;
  //----------------------------------------------------------------------------
  // NUM 
  //----------------------------------------------------------------------------
  // SOL
  //----------------------------------------------------------------------------
  // BC
   // Set BC names to BC list
/*OK
  CBoundaryConditionsGroup *m_bc_group = NULL;
  for(i=0;i<pcs_number_of_primary_nvals;i++) {
    strcpy(c_string,pcs_primary_function_name[i]);
    if(!BCGroupExists(c_string))
       SetBoundaryConditions(c_string);
    m_bc_group = new CBoundaryConditionsGroup();
    m_bc_group->Set(pcs_primary_function_name[i]);
    bc_group_list.push_back(m_bc_group);
  }
*/
  //----------------------------------------------------------------------------
  // ST
  CSourceTermGroup *m_st_group = NULL;
  for(i=0;i<pcs_number_of_primary_nvals;i++){
    m_st_group = m_st_group->Get(pcs_primary_function_name[i]);
    if(!m_st_group) {
      m_st_group = new CSourceTermGroup();
      m_st_group->Set(this, 0); //OK
      st_group_list.push_back(m_st_group);
    }
  }
  //----------------------------------------------------------------------------
  // MAT
  //----------------------------------------------------------------------------
  // EQS
  //----------------------------------------------------------------------------
  // Set source terms: EQS<->ST (eqs->b)
  // Set boundary conditions: EQS<->BC
  //----------------------------------------------------------------------------
  // MOD -> Kernel
  //----------------------------------------------------------------------------
  /* NOD - model node values */
  ConfigNODValues1(); 
  ConfigNODValues2();
  CreateNODValues();
  //----------------------------------------------------------------------------
  /* IC<->NOD */    
  int no_timelevel = 2; // pcs variable
  int nidx;
  timelevel=0;
  for(i=0;i<pcs_number_of_primary_nvals;i++) {
    nidx = pcs_nval_data[no_timelevel*i+timelevel].nval_index;
///OK    ICSetNODValues(pcs_primary_function_name[i],nidx);
    CopyNodeVals(nidx,nidx+1);
  }
///OK  SMSaintVenantHeadFTCS1D(this);
}

void CRFProcess::DestroyFDMProcess()
{
}

/*************************************************************************
ROCKFLOW - Function: CRFProcess::PCSMoveNOD
Task: 
Programming: 
08/2004 MB/OK Implementation
last modified:
**************************************************************************/
void CRFProcess::PCSMoveNOD(void) 
{
  
  switch(this->type) {
    case 1:
      MSHMoveNODUcFlow(this);
      break;
    default:
      DisplayMsgLn("PCSMoveNOD: no valid process");
      abort();
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
10/2004 OK 2nd version
**************************************************************************/
string PCSProblemType()
{
  string pcs_problem_type;
  int i;
  CRFProcess* m_pcs = NULL;
  int no_processes =(int)pcs_vector.size();
  //----------------------------------------------------------------------
  // H process
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      case 'L':
        pcs_problem_type = "LIQUID_FLOW";
        break;
     // case 'U':
      //  pcs_problem_type = "UNCONFINED_FLOW";
      //  break;
      case 'O':
        pcs_problem_type = "OVERLAND_FLOW";
        break;
      case 'G':
        pcs_problem_type = "GROUNDWATER_FLOW";
        break;
      case 'T':
        pcs_problem_type = "TWO_PHASE_FLOW";
        break;
      case 'C':
        pcs_problem_type = "COMPONENTAL_FLOW";
        break;
	  case 'R':  //MX test 04.2005
        pcs_problem_type = "RICHARDS_FLOW";
        break;
    }
  }
  //----------------------------------------------------------------------
  // M process
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      case 'D':
        if(pcs_problem_type.empty())
          pcs_problem_type = "DEFORMATION";
        else
          pcs_problem_type += "+DEFORMATION";
        break;
    }
  }
  //----------------------------------------------------------------------
  // T process
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      case 'H':
        if(pcs_problem_type.empty())
          pcs_problem_type = "HEAT_TRANSPORT";
        else
          pcs_problem_type += "+HEAT_TRANSPORT";
        break;
    }
  }
  //----------------------------------------------------------------------
  // CB process
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      case 'M':
        if(pcs_problem_type.empty())
          pcs_problem_type = "MASS_TRANSPORT";
        else
          pcs_problem_type += "+MASS_TRANSPORT";
        break;
    }
  }
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  // FM process
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[0]){
      case 'F':
        if(pcs_problem_type.empty())
          pcs_problem_type = "FLUID_MOMENTUM";
        else
          pcs_problem_type += "+FLUID_MOMENTUM";
        break;
    }
  }
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    switch(m_pcs->pcs_type_name[7]){    // pcs_type_name[7] should be 'W' because 'R' is reserved for Richard Flow.
      case 'W':
        if(pcs_problem_type.empty())
          pcs_problem_type = "RANDOM_WALK";
        else
          pcs_problem_type += "+RANDOM_WALK";
        break;
    }
  }
  return pcs_problem_type;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2004 OK Implementation
**************************************************************************/
void CRFProcess::CalcELEMassFluxes(void)
{
  int i;
  double e_value = -1.0;
  string e_value_name;
  double geo_factor, density;
  double velocity = 0.0;
  int e_idx;
  int phase = 0;
  long e;
  CMediumProperties* m_mmp = NULL;
  CFluidProperties* m_mfp = NULL;
  m_mfp = mfp_vector[phase]; //OK ToDo
  //======================================================================
  for(e=0;e<ElementListLength;e++){
    m_mmp = mmp_vector[ElGetElementGroupNumber(e)];
    geo_factor = m_mmp->geo_area;
    density = m_mfp->Density();
    for(i=0;i<pcs_number_of_evals;i++){
      e_value_name = pcs_eval_data[i].name; 
      e_idx = PCSGetELEValueIndex(pcs_eval_data[i].name);
      if(e_value_name.find("MASS_FLUX1_X")!=string::npos){
        velocity = ElGetElementVal(e,PCSGetELEValueIndex("VELOCITY1_X"));
        e_value = geo_factor * density * velocity;
        ElSetElementVal(e,e_idx,e_value);
      }
      if(e_value_name.find("MASS_FLUX1_Y")!=string::npos){
        velocity = ElGetElementVal(e,PCSGetELEValueIndex("VELOCITY1_Y"));
        e_value = geo_factor * density * velocity;
        ElSetElementVal(e,e_idx,e_value);
      }
      if(e_value_name.find("MASS_FLUX1_Z")!=string::npos){
        velocity = ElGetElementVal(e,PCSGetELEValueIndex("VELOCITY1_Z"));
        e_value = geo_factor * density * velocity;
        ElSetElementVal(e,e_idx,e_value);
      }
    }
  }
  //======================================================================
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2005 OK Implementation
02/2006 YD Dual Richards
02/2007 WW General function for all unsaturated flow
**************************************************************************/
void CRFProcess::CalcSecondaryVariables(const bool initial)
{
  char pcsT;
  pcsT = pcs_type_name[0];
  if(type==1212) pcsT = 'V'; //WW
  switch(pcsT){
    case 'L':
      break;
    case 'U':
      break;
    case 'G':
      break;
    case 'T':
      break;
    case 'C':
      break;
    case 'R': // Richards flow
	  if(pcs_type_name[1] == 'I')	// PCH To make a distinction with RANDOM WALK.
        CalcSecondaryVariablesUnsaturatedFlow(initial); // WW
      break;
    case 'D':
      break;
    case 'V':
      CalcSecondaryVariablesUnsaturatedFlow(initial); //WW
      break;
  }
}

//////////////////////////////////////////////////////////////////////////
// ReMove site
//////////////////////////////////////////////////////////////////////////

/*****************************************************/


/**************************************************************************
 ROCKFLOW - Funktion: DumpModelNodeValues

 Aufgabe:
 Schreiben von Modell-Knoten-Daten  

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -

 Ergebnis:
   - void -

 Programmaenderungen:
   09/2001   OK   Erste Version
   09/2002   OK   Spaltenvorgabe

**************************************************************************/
void DumpModelNodeValues(int columns)
{
  long i;
  int j;
  int nv=columns;

  DisplayMsgLn(" ");

  printf(" %6s ","Nodes");
  for (j=0;j<nv+1;j++) {
    if (nval_data[j].name) 
      printf(" %13s ",nval_data[j].name);
  }
  DisplayMsgLn(" ");

  for (i=0;i<NodeListLength;i++) {
    printf(" %6ld ",i);
    for (j=0;j<nv+1;j++) {
      printf(" %13.5g ",GetNodeVal(i,j));
    }
    DisplayMsgLn(" ");
  }
}

void CRFProcess::PCSDumpModelNodeValues(void)
{
  DumpModelNodeValues(pcs_number_of_primary_nvals+pcs_number_of_secondary_nvals);
}

/*************************************************************************
ROCKFLOW - Function: GetCompNamehelp
Task: Namepatch, until primary function names are finally sorted out //SB:todo
Programming:	08/2003 SB Implementation
last modified:
superseded by GetPFNamebyCPName() but left here, as not all files are already in the new concept
**************************************************************************/
/* SB: namepatch
	Repariert kurzfristig die Ausgabe
	input: datafield_n[j].name
	wenn dar Name "CONCENTRATIONx" ist, wird er durch den enstprechenden Komponentennamen ersetzt, sonst bleibts */

char *GetCompNamehelp(char *inname){
  int comp; //WW, phase;
 char * outname, help[MAX_ZEILE];
 CRFProcess *m_pcs=NULL;
 outname = inname;
 //WW phase = 0;
 for(comp=0;comp<GetRFProcessNumComponents();comp++) {
	sprintf(help,"%s%d","CONCENTRATION",comp+1);
	/*  help has to be a part of inname (strstr) and also have the same length (strcmp) */
	if(strstr(inname, help) && (strcmp(inname,help) == 0)){
		m_pcs = m_pcs->GetProcessByFunctionName(help);
		if(m_pcs == NULL) break;
//		outname = GetTracerCompName(phase,m_pcs->GetProcessComponentNumber()-1);
//		outname = cp_vec[m_pcs->GetProcessComponentNumber()-1]->compname;
		outname = (char *) cp_vec[m_pcs->GetProcessComponentNumber()-1]->compname.data();
		return outname;
	};
 };
 return outname;
} //SB:namepatch

/*************************************************************************
ROCKFLOW - Function: GetCPNamebyPFName(string )
Task: Replaces CP Name by Primary function name for output input
Input:	component property name
Output: primary function name
Programming:	10/2004 SB Implementation
**************************************************************************/
string GetPFNamebyCPName(string inname){
 int i, j, k;
 int pcs_vector_size = (int) pcs_vector.size();
 string outname;
 char  help[MAX_ZEILE];
 CRFProcess *m_pcs=NULL;
 outname = "dummy";
 if(pcs_vector_size>0)
 for(i=0;i<pcs_vector_size;i++){
	m_pcs = pcs_vector[i];
	if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT") == 0){ // if this is mass transport
		j = m_pcs->GetProcessComponentNumber();
		k = cp_vec[j]->transport_phase;
		outname = cp_vec[m_pcs->GetProcessComponentNumber()]->compname;
		if(outname == inname){ //right process found
			sprintf(help,"%s%d","CONCENTRATION",j);
			outname = help;
			return outname;}
	}
 }
// for(i=0;comp<GetRFProcessNumComponents();i++) {
//	sprintf(help,"%s%d","CONCENTRATION",i);
	/*  help has to be a part of inname (strstr) and also have the same length (strcmp) */
//	if(strstr(inname, help) && (strcmp(inname,help) == 0)){
//		m_pcs = m_pcs->GetProcessByFunctionName(help);
//		if(m_pcs == NULL) break;
//		outname = cp_vec[m_pcs->GetProcessComponentNumber()-1]->compname;
//		outname = (char *) cp_vec[m_pcs->GetProcessComponentNumber()-1]->compname.data();
//		if(outname.compare(inname) == 0)
//			return outname;
//	};
// };
// Inname is not from a mass transport process, therefore return inname
 return inname;
} //SB:namepatch

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
??/2004 MX Implementation
**************************************************************************/
 void CRFProcess::PCSSetTempArry(void){    
    int i;
    TempArry = (double *) Malloc(ElListSize() * sizeof(double));

    for (i=0;i<ElListSize();i++) {
      if (ElGetElement(i)!=NULL)
        TempArry[i] = 0.0;
      }
}

/*  SB: eingef?t */
/**************************************************************************
   ROCKFLOW - Funktion: PCSGetNODConcentration

   Aufgabe:
   Returns concentration at node of requested component

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
		index: Index des Knotens, dessen Wert geholt werden soll
		component: Komponente des Prozesses
		timelevel: alte/neue Zeitebene
   Ergebnis:
		value: Knotenwert

   Programmaenderungen:
   08/2003     SB         Erste Version

**************************************************************************/
double PCSGetNODConcentration(long index, long component, long timelevel)
{
  char name[MAX_ZEILE];
  sprintf(name,"%s%i","CONCENTRATION", (int)component); //SB:todo global name
  return PCSGetNODValue(index,name,timelevel);
}

void PCSSetNODConcentration(long index, long component, long timelevel, double value)
{
  char name[MAX_ZEILE];
  sprintf(name,"%s%i","CONCENTRATION", (int)component);
  SetNodeVal(index, PCSGetNODValueIndex(name,timelevel),value);
}

//========================================================================
//OK former model functions
int GetRFControlGridAdapt(void)
{
//OK  return (get_rfcp_adaptive_mesh_refinement_flag(rfcp));
  if(show_onces_adp)
    cout << "GetRFControlGridAdapt - to be removed" << endl;
  show_onces_adp = false;
  return 0;
}

int GetRFControlModel(void)
{
  if(show_onces_mod)
    cout << "GetRFControlModel - to be removed" << endl;
  show_onces_mod = false;
  return -1;
}

int GetRFProcessChemicalModel(void)
{
  cout << "GetRFProcessChemicalModel - to be removed" << endl;
  return 0;
}

int GetRFProcessFlowModel(void)
{
  if(show_onces_mod_flow)
    cout << "GetRFProcessFlowModel - to be removed" << endl;
  show_onces_mod_flow = false;
  return 0;
}

int GetRFProcessHeatReactModel(void)
{
  cout << "GetRFProcessHeatReactModel - to be removed" << endl;
  return 0;
}

int GetRFProcessNumPhases(void)
{
  //DisplayMsgLn("GetRFProcessNumPhases - to be removed");
  int no_phases = (int)mfp_vector.size();
  return no_phases;
}

int GetRFProcessProcessing(char* rfpp_type)
{
  //----------------------------------------------------------------------
  int i;
  bool pcs_flow = false;
  bool pcs_deformation = false;
  CRFProcess* m_pcs = NULL;
  int no_processes = (int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
      pcs_deformation = true;
    if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
      pcs_flow = true;
  }
  //----------------------------------------------------------------------
  if(strcmp(rfpp_type,"SD")==0){
    if(pcs_flow&&pcs_deformation)
      return 1;
  }
  else
    cout << "GetRFProcessProcessing - to be removed" << endl;
  return 0;
}

int GetRFProcessProcessingAndActivation(char*)
{
  cout << "GetRFProcessProcessingAndActivation - to be removed" << endl;
  return 0;
}

long GetRFProcessNumComponents(void)
{
  //DisplayMsgLn("GetRFProcessNumComponents - to be removed");
  int no_components = (int)cp_vec.size();
  return no_components;
}

int GetRFControlModex(void)
{
  cout << "GetRFControlModex - to be removed" << endl;
  return 0;
}

int GetRFProcessDensityFlow(void)
{
  if(show_onces_density)
    cout << "GetRFProcessDensityFlow - to be removed" << endl;
  show_onces_density = false;
  return 0;
}

int GetRFProcessNumContinua(void)
{
  cout << "GetRFProcessNumContinua - to be removed" << endl;
  return 0;
}

int GetRFProcessNumElectricFields(void)
{
  cout << "GetRFProcessNumElectricFields - to be removed" << endl;
  return 0;
}

int GetRFProcessNumTemperatures(void)
{
  cout << "GetRFProcessNumTemperatures - to be removed" << endl;
  return -1;
}

int GetRFProcessSimulation(void)
{
  cout << "GetRFProcessSimulation - to be removed" << endl;
  return -1;
}

/**************************************************************************
   ROCKFLOW - Funktion: ModelsAddNodeValInfoStructure

   Aufgabe:
   Fuellt die Knotendaten-Infostruktur mit den zugehoerigen Modelldaten.
   Wird vom Modell der Reihe nach fuer jede Knotengroesse aufgerufen.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E:char *name         :Name der Knotengroesse fuer Ergebnisdatei
   E:char *einheit      :Name der phys. Einheit fuer Ergebnisdatei
   E:int speichern      :Werte sollen gespeichert werden (0/1)
   E:int laden          :Werte sollen geladen werden falls vorhanden (0/1)
   E:int restart        :Werte sollen bei Restart geladen werden (0/1)
   E:int adapt_interpol :Werte sollen beim verfeinern auf Kinder interpoliert (0/1)
   E:double vorgabe     :Vorgabe falls keine Restartdaten oder Anfangsbedingungen vorhanden sind

   Ergebnis:
   Knotenindex der gerade vergeben wurde

   Programmaenderungen:
   09/2000   CT    Erste Version
   
**************************************************************************/
int ModelsAddNodeValInfoStructure(char *name, char *einheit, int speichern, int laden, int restart, int adapt_interpol, double vorgabe)
{
  anz_nval++;
  nval_data = (NvalInfo *) Realloc(nval_data, anz_nval * sizeof(NvalInfo));

  nval_data[anz_nval - 1].name = NULL;
  nval_data[anz_nval - 1].einheit = NULL;

  if (name)
    {
      nval_data[anz_nval - 1].name = (char *) Malloc(((int)strlen(name) + 1) * sizeof(char));
      strcpy(nval_data[anz_nval - 1].name, name);
    }
  if (einheit)
    {
      nval_data[anz_nval - 1].einheit = (char *) Malloc(((int)strlen(einheit) + 1) * sizeof(char));
      strcpy(nval_data[anz_nval - 1].einheit, einheit);
    }

  nval_data[anz_nval - 1].speichern = speichern;
  nval_data[anz_nval - 1].laden = laden;
  nval_data[anz_nval - 1].restart = restart;
  nval_data[anz_nval - 1].adapt_interpol = adapt_interpol;
  nval_data[anz_nval - 1].vorgabe = vorgabe;

  return anz_nval - 1;
}
/**************************************************************************
   ROCKFLOW - Funktion: ModelsAddElementValInfoStructure

   Aufgabe:
   Fuellt die Elementdaten-Infostruktur mit den zugehoerigen Modelldaten.
   Wird vom Modell der Reihe nach fuer jede Elementgroesse aufgerufen.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E:char *name         :Name der Elementgroesse fuer Ergebnisdatei
   E:char *einheit      :Name der phys. Einheit fuer Ergebnisdatei
   E:int speichern      :Werte sollen gespeichert werden (0/1)
   E:int laden          :Werte sollen geladen werden falls vorhanden (0/1)
   E:int restart        :Werte sollen bei Restart geladen werden (0/1)
   E:int adapt_interpol :Werte sollen beim verfeinern auf Kinder interpoliert (0/1)
   E:double vorgabe     :Vorgabe falls keine Restartdaten oder Anfangsbedingungen vorhanden sind

   Ergebnis:
   Elementindex der gerade vergeben wurde

   Programmaenderungen:
   09/2000   CT    Erste Version
   
**************************************************************************/
int ModelsAddElementValInfoStructure(char *name, char *einheit, int speichern, int laden, int restart, int adapt_interpol, double vorgabe)
{
  anz_eval++;
  eval_data = (EvalInfo *) Realloc(eval_data, anz_eval * sizeof(EvalInfo));

  eval_data[anz_eval - 1].name = NULL;
  eval_data[anz_eval - 1].einheit = NULL;

  if (name)
    {
      eval_data[anz_eval - 1].name = (char *) Malloc(((int)strlen(name) + 1) * sizeof(char));
      strcpy(eval_data[anz_eval - 1].name, name);
    }
  if (einheit)
    {
      eval_data[anz_eval - 1].einheit = (char *) Malloc(((int)strlen(einheit) + 1) * sizeof(char));
      strcpy(eval_data[anz_eval - 1].einheit, einheit);
    }

  eval_data[anz_eval - 1].speichern = speichern;
  eval_data[anz_eval - 1].laden = laden;
  eval_data[anz_eval - 1].restart = restart;
  eval_data[anz_eval - 1].adapt_interpol = adapt_interpol;
  eval_data[anz_eval - 1].vorgabe = vorgabe;

  return anz_eval - 1;
}


/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void PCSDelete()
{
  for(int i=0;i<(int)pcs_vector.size();i++){
    delete pcs_vector[i];
  }
  pcs_vector.clear();
  pcs_no_components = 0;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::SetNodeValue(long n,int nidx,double value)
{
#ifdef gDEBUG
  if(nidx<0)
  {
    cout<<" Fatal error in  CRFProcess::SetNodeValue() "<<endl;
    abort();
  }
#endif
  nod_val_vector[n][nidx] = value;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
09/2005 PCH Implementation
last modified:
**************************************************************************/
void CRFProcess::SetElementValue(long n,int nidx,double value)
{
#ifdef gDEBUG
  if(nidx<0)
  {
    cout<<" Fatal error in  CRFProcess::SetElementValue() "<<endl;
    abort();
  }
#endif
  ele_val_vector[n][nidx] = value;
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
last modified:
**************************************************************************/
double CRFProcess::GetNodeValue(long n,int nidx)
{
  double value;
#ifdef gDEBUG
  if(nidx<0)
  {
    cout<<" Fatal error in  CRFProcess::GetNodeValue() "<<endl;
    abort();
  }
#endif
  value = nod_val_vector[n][nidx];
  return value;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
09/2005 PCH Implementation
last modified:
**************************************************************************/
double CRFProcess::GetElementValue(long n,int nidx)
{
  double value;
#ifdef gDEBUG
  if(nidx<0)
  {
    cout<<" Fatal error in CRFProcess::GetElementValue() "<<endl;
    abort();
  }
#endif
  value = ele_val_vector[n][nidx];
  return value;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
01/2006 OK Test
**************************************************************************/
int CRFProcess::GetNodeValueIndex(string var_name)
{
  int i;
  int nidx = -2;
  string help;
  for(i=0;i<(int)nod_val_name_vector.size();i++){
//    if(nod_val_name_vector[i].compare(var_name)==0){
    help = nod_val_name_vector[i];
    if(help.compare(var_name)==0){
      nidx = i;
#ifdef gDEBUG
     if(nidx<0)
     {
        cout<<" Fatal error in CRFProcess::GetNodeValueIndex() "<<endl;
        abort();
     }
#endif
      return nidx;
    }
  }

  // Suppress the following error message when Fluid Momentum process is on.
  CRFProcess* m_pcs = PCSGet("FLUID_MOMENTUM");
  if(m_pcs) ;	// Don't print any error message.
  else
	cout << "Error in CRFProcess::GetNodeValueIndex - " << pcs_type_name << ", " << var_name << ", NIDX = " << nidx << endl;
  //abort();
  return nidx;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
09/2005 PCH Implementation
last modified:
**************************************************************************/
int CRFProcess::GetElementValueIndex(string var_name)
{
  int i;
  int nidx = -1;
  for(i=0;i<(int)ele_val_name_vector.size();i++){
    if(ele_val_name_vector[i].compare(var_name)==0){
      nidx = i;
#ifdef gDEBUG
     if(nidx<0)
     {
        cout<<" Fatal error in CRFProcess:::GetElementValueIndex() "<<endl;
        abort();
     }
#endif
      return nidx;
    }
  }
  return nidx;
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
05/2005 OK pcs_pv_name, 
12/2005 OK RESTART
07/2006 OK/MX MSH
**************************************************************************/
void CRFProcess::SetIC()
{
  int nidx,nidx1;
  CInitialCondition* m_ic = NULL;
  for(int i=0;i<pcs_number_of_primary_nvals;i++){
    nidx = GetNodeValueIndex(pcs_primary_function_name[i]);
    nidx1 = nidx+1; //WW
    for(int j=0;j<(int)ic_vector.size();j++)
    {
      m_ic = ic_vector[j];
      m_ic->m_msh = m_msh; //OK/MX
      if(!(m_ic->pcs_type_name.compare(pcs_type_name)==0))
        continue;
      m_ic->m_pcs = this;
      if(m_ic->pcs_pv_name.compare(pcs_primary_function_name[i])==0){
        m_ic->Set(nidx);
        m_ic->Set(nidx+1);
      }
    }
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
last modified:
**************************************************************************/
void CRFProcess::SetNODValues()
{
  for(long i=0;i<(long)m_msh->nod_vector.size();i++){
//    SetNODValue(i,GetNODValueIndex(pcs_type_name),eqs->x[i]);
//    SetNODValue(i,GetNODValueIndex(pcs_type_name)+1,eqs->x[i]);
    SetNodeValue(m_msh->Eqs2Global_NodeIndex[i],0,eqs->x[i]); //WW
    SetNodeValue(m_msh->Eqs2Global_NodeIndex[i],1,eqs->x[i]); //WW
  }
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
last modification:
**************************************************************************/
/* // Comment by WW
void CRFProcess::CheckBCGroup()
{
  long j;
  CBoundaryConditionNode* m_node = NULL;
  //----------------------------------------------------------------------
  // File handling
  //OK CGSProject* m_gsp = NULL;
  //OK m_gsp = GSPGetMember("pcs");
  //OK string pcs_bc_group_file_name = m_gsp->path + "_CheckBCGroup" + ".txt";
  string pcs_bc_group_file_name = pcs_type_name + "_BCGroup.txt";
  fstream pcs_bc_group_file (pcs_bc_group_file_name.data(),ios::trunc|ios::out);
  pcs_bc_group_file.setf(ios::scientific,ios::floatfield);
  if (!pcs_bc_group_file.good()) 
    return;
  pcs_bc_group_file.seekg(0L,ios::beg);
  //----------------------------------------------------------------------
  CBoundaryConditionsGroup *m_bc_group = NULL;
  list<CBoundaryConditionsGroup*>::const_iterator p_bc_group = bc_group_list.begin();
  while(p_bc_group!=bc_group_list.end()) {
    m_bc_group = *p_bc_group;
    for(int i=0;i<pcs_number_of_primary_nvals;i++){
      if((m_bc_group->pcs_type_name.compare(pcs_type_name)==0)\
       &&(m_bc_group->pcs_pv_name.compare(pcs_primary_function_name[i])==0)){
        //m_bc_group->Check();
        pcs_bc_group_file << m_bc_group->pcs_type_name << endl;
        pcs_bc_group_file << m_bc_group->pcs_pv_name << endl;
        pcs_bc_group_file << "geo_node_number" << "," << "msh_node_number" << "," << "node_value" << endl;
        for(j=0;j<(long)m_bc_group->group_vector.size();j++){
          m_node = m_bc_group->group_vector[j];
          pcs_bc_group_file << " " << m_node->geo_node_number << " " << m_node->msh_node_number << " ";
          pcs_bc_group_file << m_node->node_value << endl;
        }
      }
    }
    ++p_bc_group;
  }
}
*/
/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
last modification:
**************************************************************************/
/*
void CRFProcess::CheckSTGroup()
{
  long j;
  CNodeValue* m_node = NULL;
  //----------------------------------------------------------------------
  // File handling
  //OK CGSProject* m_gsp = NULL;
  //OK m_gsp = GSPGetMember("pcs");
  //OK string pcs_st_group_file_name = m_gsp->path + "_CheckSTGroup" + ".txt";
  string pcs_st_group_file_name = pcs_type_name + "_STGroup.txt";
  fstream pcs_st_group_file (pcs_st_group_file_name.data(),ios::trunc|ios::out);
  pcs_st_group_file.setf(ios::scientific,ios::floatfield);
  if (!pcs_st_group_file.good()) 
    return;
  pcs_st_group_file.seekg(0L,ios::beg);
  //----------------------------------------------------------------------
  CSourceTermGroup *m_st_group = NULL;
  list<CSourceTermGroup*>::const_iterator p_st_group = st_group_list.begin();
  while(p_st_group!=st_group_list.end()) {
    m_st_group = *p_st_group;
    for(int i=0;i<pcs_number_of_primary_nvals;i++){
      if((m_st_group->pcs_type_name.compare(pcs_type_name)==0)\
       &&(m_st_group->pcs_pv_name.compare(pcs_primary_function_name[i])==0)){
        //m_bc_group->Check();
        pcs_st_group_file << m_st_group->pcs_type_name << endl;
        pcs_st_group_file << m_st_group->pcs_pv_name << endl;
        pcs_st_group_file << "geo_node_number" << "," << "msh_node_number" << "," << "node_value" << endl;
        for(j=0;j<(long)m_st_group->group_vector.size();j++){
          m_node = m_st_group->group_vector[j];
          pcs_st_group_file << " " << m_node->geo_node_number << " " << m_node->msh_node_number << " ";
          pcs_st_group_file << m_node->node_value << endl;
        }
      }
    }
    ++p_st_group;
  }
}
*/

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
05/2005 OK Implementation
07/2005 WW Change for geometry element object
**************************************************************************/
/*
double CRFProcess::GetELEValue(long index,double*gp,double theta,string nod_fct_name)
{
  int nidx0,nidx1;
  double unit[3];
  double e_value;
  double nod_fct0,nod_fct1;
  unit[0] = unit[1] = unit[2] = 0.0;
  //----------------------------------------------------------------------
  nidx0 = GetNODValueIndex(nod_fct_name); // PCSGetNODValueIndex(nod_fct_name,0);
  nidx1 = GetNODValueIndex(nod_fct_name)+1; // PCSGetNODValueIndex(nod_fct_name,1);
  //----------------------------------------------------------------------
  nod_fct0 = 0.0;
  nod_fct1 = 0.0;
  if(gp==NULL){ // Element average value . WW
    if(m_msh->ele_vector[index]->GetElementType()==4)
      unit[0] = unit[1] = 1.0/3.0;
    else if(m_msh->ele_vector[index]->GetElementType()==5)
      unit[0] = unit[1] = unit[2] = 0.25;
  }
  else
  {
     for(int i=0; i<3; i++) unit[i] = gp[i];
  }

  nod_fct0 = InterpolateNODValue(index,nidx0,unit);
  nod_fct1 = InterpolateNODValue(index,nidx1,unit);
  //----------------------------------------------------------------------
  e_value = (1.-theta)*nod_fct0 + theta*nod_fct1;
  return e_value;
}
*/
/**************************************************************************
FEMLib-Method:
Task:
Programing:
05/2005 OK Implementation
07/2005 WW Re-Implementation
last modification:
**************************************************************************/
/*
double CRFProcess::InterpolateNODValue(long number,int ndx,double*gp)
{
  int i;
  double nval[20];
  CElem* elem = NULL;
  elem = m_msh->ele_vector[number];
  fem->ConfigElement(elem);
  fem->setUnitCoordinates(gp);
  fem->ComputeShapefct(1); // Linear
  for(i=0;i<elem->GetVertexNumber();i++)
     nval[i] = GetNodeValueIndex(elem->nodes_index[i],ndx); // GetNodeVal(nodes[i], ndx);
  return fem->interpolate(nval, 1);
}
*/

/**************************************************************************
FEMLib-Method:
Task: Ermittelt den Fehler bei Iterationen
 new_iteration     : Vektor des neuen Iterationsschritts
 old_iteration_ndx : Knotenindex fuer Werte des alten Iterationsschritts
 reference_ndx     : Knotenindex fuer Werte des alten Zeitschritts (als Referenz)
 method            : Methode der Fehlerermittlung
Programing:
01/2005 OK NUM implementation
05/2005 OK MSH
08/2005 WW Re-implememtation based on NUMCalcIterationError
01/2007 WW For DOF>1
11/2007 WW Changes for the new classes of sparse and linear solver
last modification:
**************************************************************************/
double CRFProcess::CalcIterationNODError(int method)
{

    static long i, k, g_nnodes;
    static double error, change, max_c, min_c;
    //-----------------------------------------------------WW
    int nidx1; 
    int ii;
    g_nnodes = m_msh->GetNodesNumber(false); //WW 
    //-----------------------------------------------------WW
    double *eqs_x = NULL; // 11.2007. WW
#ifdef NEW_EQS
    eqs_x = eqs_new->x;
#else
    eqs_x = eqs->x; 
#endif 
    error = 0.;
    change = 0.;

    max_c = 0.;
    min_c = 1.e99;

    switch (method) {
       default:
       case 0:
          return 0.;
       // Maximum error
       case 1:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++) //WW
         {
            nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
            for (i = 0l; i < g_nnodes; i++) //WW
            {
               k = m_msh->Eqs2Global_NodeIndex[i];
               error = max(error, fabs(GetNodeValue(k, nidx1) - eqs_x[i+ii*g_nnodes]));
	        }
         }
         return error;
       case 2:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++)//WW
         {
            nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
            for (i = 0l; i < g_nnodes; i++) //WW
            {
               k = m_msh->Eqs2Global_NodeIndex[i];
               error = max(error, fabs(eqs_x[i+ii*g_nnodes] - GetNodeValue(k, nidx1))
                       /(fabs(eqs_x[i+ii*g_nnodes]) + fabs(GetNodeValue(k, nidx1))+ MKleinsteZahl) );
            }
		 }
		 return error;
       case 3:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++)//WW
         {
             nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
             for (i = 0l; i < g_nnodes; i++) { //WW
               k = m_msh->Eqs2Global_NodeIndex[i];
               error = max(error, fabs(eqs_x[i+ii*g_nnodes] - GetNodeValue(k, nidx1)));
               max_c = max(max(max_c, fabs(fabs(eqs_x[i+ii*g_nnodes]))),fabs(GetNodeValue(k, nidx1)));
            }
         }
         return error / (max_c + MKleinsteZahl);
       case 4:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++)//WW
         {
            nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
            for (i = 0l; i < g_nnodes; i++) { //WW
              k = m_msh->Eqs2Global_NodeIndex[i];
              error = max(error, fabs(eqs_x[i+ii*g_nnodes] - GetNodeValue(k, nidx1)));
              min_c = min(min_c, fabs(eqs_x[i+ii*g_nnodes]));
              max_c = max(max_c, fabs(eqs_x[i+ii*g_nnodes]));
            }
         }
         return error / (max_c - min_c + MKleinsteZahl);
       case 5:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++)//WW
         {
            nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
            for (i = 0l; i < g_nnodes; i++) { //WW
              k = m_msh->Eqs2Global_NodeIndex[i];
              error = max(error, fabs(eqs_x[i+ii*g_nnodes] -  GetNodeValue(k, nidx1)) 
  				/ (fabs(eqs_x[i+ii*g_nnodes] - GetNodeValue(k, nidx1-1)) + MKleinsteZahl));
            }
		 }
         return error;
       case 6:
         for(ii=0;ii<pcs_number_of_primary_nvals;ii++)//WW
         {
            nidx1 = GetNodeValueIndex(pcs_primary_function_name[ii]) + 1; //new time
            for (i = 0l; i < g_nnodes; i++) { //WW
               k = m_msh->Eqs2Global_NodeIndex[i];
               error = max(error, fabs(eqs_x[i+ii*g_nnodes] -  GetNodeValue(k, nidx1)));
               change = max(change, fabs(eqs_x[i+ii*g_nnodes] - GetNodeValue(k, nidx1-1)));
            }
         }
         return error / (change + MKleinsteZahl);
	}
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
05/2005 OK Implementation
04/2006 YD Add contiuum coupling OK???Why ere
04/2007 WW Remove the spurious stuff
**************************************************************************/
double CRFProcess::ExecuteNonLinear()
{
  double nonlinear_iteration_error=0.0;
#ifdef NEW_EQS
  configured_in_nonlinearloop = true;
  // Also allocate temporary memory for linear solver. WW
  //
#if defined(USE_MPI)
  CPARDomain *dom = dom_vector[myrank];
  dom->ConfigEQS(m_num, pcs_number_of_primary_nvals
                        *m_msh->GetNodesNumber(false));
#else  
  eqs_new->ConfigNumerics(m_num);
#endif
  //
#endif
  //..................................................................
  for(iter=0;iter<pcs_nonlinear_iterations;iter++)
  {
    cout << "    PCS non-linear iteration: " << iter << "/"   
         << pcs_nonlinear_iterations << endl;
    nonlinear_iteration_error = Execute();
    if(mobile_nodes_flag ==1)
      PCSMoveNOD();
    if(!Tim) //OK
      continue;
    if(nonlinear_iteration_error < pcs_nonlinear_iteration_tolerance)
    {
      Tim->repeat = false; //OK/YD
      Tim->nonlinear_iteration_error = nonlinear_iteration_error; //OK/YD
      break;
    }
    else
      Tim->repeat = true; //OK/YD
  }
  // 8 Calculate secondary variables
  CalcSecondaryVariables(); // Moved here from Execute() WW
  // Release temporary memory of linear solver. WW
#ifdef NEW_EQS  //WW
 #if defined(USE_MPI)
  dom->eqs->Clean();
 #else 
  eqs_new->Clean(); // Release buffer momery WW
 #endif
  configured_in_nonlinearloop = false;  
#endif
  return nonlinear_iteration_error; 
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
02/2007 WW Implementation
**************************************************************************/
void CRFProcess::Extropolation_GaussValue() 
{
  int k, NS;
  long i = 0;
  int idx[3];
 // const long LowOrderNodes= m_msh->GetNodesNumber(false);
  Mesh_Group::CElem* elem = NULL;

  // 
  NS =m_msh->GetCoordinateFlag()/10;
  idx[0] = GetNodeValueIndex("VELOCITY_X1");
  idx[1] = GetNodeValueIndex("VELOCITY_Y1");
  idx[2] = GetNodeValueIndex("VELOCITY_Z1");
  for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++)
  {
     for(k=0; k<NS; k++)
       SetNodeValue(i, idx[k], 0.0);
  }
  if(type==1212)  // Multi-phase flow
  {
     idx[0] = GetNodeValueIndex("VELOCITY_X2");
     idx[1] = GetNodeValueIndex("VELOCITY_Y2");
     idx[2] = GetNodeValueIndex("VELOCITY_Z2");
     for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++)
     {
        for(k=0; k<NS; k++)
           SetNodeValue(i, idx[k], 0.0);
     }
  }
  //
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
  {
     elem = m_msh->ele_vector[i];
     if (elem->GetMark()) // Marked for use
     {
         fem->ConfigElement(elem);
         for(k=0; k<NS; k++)
           fem->ExtropolateGauss(this, k);
      }
   }
}

/**************************************************************************
FEMLib-Method: 
Task:    Calculate the material values at Gauss points and extropolate them
         to element node
Programing:
04/2007 WW Implementation
**************************************************************************/
void CRFProcess::Extropolation_MatValue() 
{
  int k, NS;
  long i = 0;
  int idx[3], idxp;
  if(pcs_type_name.find("FLOW")==string::npos)
    return;
  if(additioanl2ndvar_print<0)
    return;
 // const long LowOrderNodes= m_msh->GetNodesNumber(false);
  Mesh_Group::CElem* elem = NULL;

  // 
  NS =m_msh->GetCoordinateFlag()/10;
  // 
  if((additioanl2ndvar_print>0)&&(additioanl2ndvar_print<3))
  {
     idx[0] = GetNodeValueIndex("PERMEABILITY_X1");
     idx[1] = GetNodeValueIndex("PERMEABILITY_Y1");
     if(NS>2)
       idx[2] = GetNodeValueIndex("PERMEABILITY_Z1");
     for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++)
     {
        for(k=0; k<NS; k++)
           SetNodeValue(i, idx[k], 0.0);
     }
  }
  if(additioanl2ndvar_print>1)
  {
     idxp = GetNodeValueIndex("POROSITY");
     for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++)
       SetNodeValue(i, idxp, 0.0);
  }
  //
  continuum = 0;
  if(continuum_vector.size()==2)
    continuum = 1;
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)
  {
     elem = m_msh->ele_vector[i];
     if (elem->GetMark()) // Marked for use
     {
         fem->ConfigElement(elem);
         fem->CalcNodeMatParatemer();
      }
   }
}


/**************************************************************************
FEMLib-Method: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void PCSDelete(string m_pcs_type_name)
{
  CRFProcess* m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare(m_pcs_type_name)==0){
      delete m_pcs;
      pcs_vector.erase(pcs_vector.begin()+i);
    }
  }
}

/**************************************************************************
   GeoSys - Function: Reallocation

   Aufgabe:
        Reallocte memory by new operator
   09/2005   WW    Erste Version
   
**************************************************************************/
template <class T> T *resize(T *array, size_t old_size, size_t new_size)
{
   T *temp = new T[new_size];
   for(size_t i=0; i<old_size; i++) temp[i] = array[i];
   for(size_t i=old_size; i<new_size; i++) temp[i] = 0;
   delete [] array;
   array = temp; 
   return temp;
} 
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2005 MB Implementation
**************************************************************************/
void CRFProcess::CalcFluxesForCoupling(void)
{
  int i,j;
  double flux;
  long n_index;
  long NodeIndex_GW;
  long NodeIndex_OLF;
  int no_nodes = m_msh->no_msh_layer +1;
  long no_richards_problems = (long)(m_msh->ele_vector.size()/m_msh->no_msh_layer);
  long IndexBottomNode;
  long IndexTopNode;
  int NoOfGWNodes = 0;
  double AverageZ_GW = 0.0;
  double AverageZ_OLF =  0.0;
  double AverageH_GW = 0.0;
  double AverageH_OLF =  0.0;
  double dh;
  int idxFLUX;
  int idxHead_GW;
  int idxHead_OLF;
  Mesh_Group::CElem* m_ele_GW = NULL;
  Mesh_Group::CElem* m_ele_OLF = NULL;
  CRFProcess*m_pcs_GW = NULL;
  CRFProcess*m_pcs_OLF = NULL;
  //Get processes
  m_pcs_GW = PCSGet("GROUNDWATER_FLOW");
  if(!m_pcs_GW) //OK
  {
    cout << "Fatal error: no GROUNDWATER_FLOW process" << endl;
    return;
  }
  m_pcs_OLF  = PCSGet("OVERLAND_FLOW");
  if(!m_pcs_OLF) //OK
  {
    cout << "Fatal error: no OVERLAND_FLOW process" << endl;
    return;
  }
  //Get meshes
  CFEMesh* m_msh_GW = m_pcs_GW->m_msh;
  CFEMesh* m_msh_OLF = m_pcs_OLF->m_msh;
  //Get indeces
  idxHead_GW  = m_pcs_GW->GetNodeValueIndex("HEAD")+1;
  idxHead_OLF  = m_pcs_OLF->GetNodeValueIndex("HEAD")+1;
  idxFLUX  = GetNodeValueIndex("FLUX")+1;

  for(i=0;i<no_richards_problems;i++){
    IndexBottomNode = ((i+1) * no_nodes)-1;

    // ToDo safe somewhere else so that this has to be done only once
    //-----------------------------------------------------------------
    // Get Nearest GW and OLF Element
    CGLPoint* m_pnt = NULL;
    long EleNumber;
    m_pnt = new CGLPoint;
	
    m_pnt->x = m_msh->nod_vector[IndexBottomNode]->X();
    m_pnt->y = m_msh->nod_vector[IndexBottomNode]->Y();
    m_pnt->z = m_msh->nod_vector[IndexBottomNode]->Z();

    EleNumber = m_msh_GW->GetNearestELEOnPNT(m_pnt);
    delete m_pnt;
    //GW and OLF use the same Numbering !!!
    m_ele_GW = m_msh_GW->ele_vector[EleNumber];
    m_ele_OLF = m_msh_OLF->ele_vector[EleNumber];

    //-----------------------------------------------------------------
    // Get Average values for element //ToDo encapsulate //WW: CElement::elemnt_averag??e         
    NoOfGWNodes = m_ele_OLF->GetNodesNumber(m_msh_GW->getOrder());
    for(j=0; j<NoOfGWNodes; j++){
      NodeIndex_GW = m_ele_GW->GetNodeIndex(j);
      NodeIndex_OLF = m_ele_OLF->GetNodeIndex(j);

      AverageZ_GW += m_pcs_GW->GetNodeValue(NodeIndex_GW,idxHead_GW);
      AverageZ_OLF += m_msh_OLF->nod_vector[NodeIndex_OLF]->Z();
    }
    AverageZ_GW = AverageZ_GW / NoOfGWNodes;
    AverageZ_OLF = AverageZ_OLF / NoOfGWNodes;

    //-----------------------------------------------------------------
    // UsatZone exists -> Flux from this        
    if(AverageZ_GW < AverageZ_OLF){
      n_index = m_msh->Eqs2Global_NodeIndex[IndexBottomNode];
      if(m_msh->nod_vector[IndexBottomNode]->GetMark()){
        flux = eqs->b[IndexBottomNode];
        //FLUXES IN NEW VERSION WITH VELOCITIES !!!!!
        //WAIT FOR SEBASTIANS MASS TRANSPORT IN USAT ZONE !!!!!
        //TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST 
        flux = 0.00001;
        //flux = 1;
        SetNodeValue(n_index, idxFLUX, flux); 
      }
    }

    //-----------------------------------------------------------------
    // No UsatZone -> Calculate Flux from leakage terms
    if(AverageZ_GW >= AverageZ_OLF){
      //SetRichardsNodesToFullySaturated??
      IndexTopNode = i * no_nodes;              // Top Node of Richard Column -> Flux for OLF
      IndexBottomNode = ((i+1) * no_nodes)-1;   // Bottom Node of Richards Column -> Flux for GW
      //-----------------------------------------------------------------
      // Get Average values for element //ToDo encapsulate       
      for(j=0; j<NoOfGWNodes; j++){
        NodeIndex_GW = m_ele_GW->GetNodeIndex(j);
        NodeIndex_OLF = m_ele_OLF->GetNodeIndex(j);
        AverageH_GW += m_pcs_GW->GetNodeValue(NodeIndex_GW,idxHead_GW);
        AverageH_OLF += m_pcs_OLF->GetNodeValue(NodeIndex_OLF,idxHead_OLF);
      }
      AverageH_GW = AverageH_GW / NoOfGWNodes;
      AverageH_OLF = AverageH_OLF / NoOfGWNodes;
      //Calculate the vertical leakage
      dh = AverageH_GW - AverageH_OLF;
      // get kf fully saturated of uppermost element ?
      // or user defined value: entry resistance / leakage factor ?
      //flux = dh * 0.001;
      flux = dh * 1.;

      //1. Add reacharge value to GW flow -> Add to flux off IndexBottomNode
      //Achtung nur zum Testen Source für GW flow durchgehend !!!!!!
      //SetNodeValue(IndexBottomNode, idxFLUX, -flux);  //H_OLF  > H_GW -> + flux_GW
      SetNodeValue(IndexBottomNode, idxFLUX, 0.00001);
      
      //2. Add reacharge value to OLF -> Add to flux off IndexTopNode
      SetNodeValue(IndexTopNode, idxFLUX, flux);      //H_OLF  > H_GW -> - flux_OLF
      //3. Set flag to set reacharge to Usat to zero ???

    }
  }
}
/**************************************************************************
FEMLib-Method:
Task: Ermittelt den Fehler bei Kopplungs Iterationen
Programing:
11/2005 MB implementation
**************************************************************************/
double CRFProcess::CalcCouplingNODError()
{
  static long i, k;
  static double error, change, max_c, min_c;
  int ndx0 = GetNodeValueIndex(m_num->cpl_variable);
  int ndx1 = GetNodeValueIndex(m_num->cpl_variable)+1;

  error = 0.;
  change = 0.;

  max_c = 0.;
  min_c = 1.e99;

  for (i = 0l; i < eqs->dim; i++) {
    k = m_msh->Eqs2Global_NodeIndex[i];
    if(m_msh->nod_vector[k]->GetMark())
      error = max(error, fabs(GetNodeValue(k, ndx1) - GetNodeValue(k, ndx0)));
	}
  cout <<  "  Coupling error: " << error << endl;
        
  return error;

}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
11/2005 MB implementation
**************************************************************************/
void CRFProcess::CopyCouplingNODValues()
{
  int nidx0 = -1;
  int nidx1 = -1;
  long l;
  
  //Carefull if cpl_variable = primary variable -> need extra coulumn in NodeValueTable !      
  nidx0 = GetNodeValueIndex(m_num->cpl_variable);
  nidx1 = nidx0+1;
  
  for(l=0;l<(long)m_msh->GetNodesNumber(false);l++){
    SetNodeValue(l,nidx0,GetNodeValue(l,nidx1));
  }
  if(pcs_type_name.find("RICHARDS")!=string::npos) //WW
  {
      nidx0 = GetNodeValueIndex("SATURATION1");
      nidx1 = nidx0+1;
     for(l=0;l<(long)m_msh->GetNodesNumber(false);l++)
        SetNodeValue(l,nidx0,GetNodeValue(l,nidx1));
   } 
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
11/2005 MB implementation
02/2006 WW Modified for the cases of high order element and saturation
**************************************************************************/
void CRFProcess::CopyTimestepNODValues()
{
  int nidx0 = -1;
  int nidx1 = -1;
  int j;
  long l;
  
  bool Quadr = false;  //WW
  if(type==4||type==41) Quadr = true;

  for(j=0;j<pcs_number_of_primary_nvals;j++)
  {
    nidx0 = GetNodeValueIndex(pcs_primary_function_name[j]);
    nidx1 = GetNodeValueIndex(pcs_primary_function_name[j])+1;
    for(l=0;l<(long)m_msh->GetNodesNumber(Quadr);l++)
      SetNodeValue(l,nidx0,GetNodeValue(l,nidx1));
    //WW 
    if(pcs_type_name.find("RICHARDS")!=string::npos
       ||type==1212) //Multiphase. WW
    {
       if(j==1&&type==1212)  // Multiphase. WW
         continue;
       if(j==0) nidx0 = GetNodeValueIndex("SATURATION1");
       else if(j==1) nidx0 = GetNodeValueIndex("SATURATION2");
       nidx1 = nidx0+1;
       for(l=0;l<(long)m_msh->GetNodesNumber(false);l++)
         SetNodeValue(l,nidx0,GetNodeValue(l,nidx1));
    }    
  }
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
12/2005 SB Implementation
last modified:
**************************************************************************/
CRFProcess* PCSGet(string pcs_type_name, string comp_name)
{
  CRFProcess *m_pcs = NULL;
  int i;
  int no_processes = (int)pcs_vector.size();
  string testname;
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare(pcs_type_name)==0){
      testname = m_pcs->pcs_primary_function_name[0];
      if(testname.compare(comp_name)==0){
//        cout << " Found in PCSGetbyTypeandCompName for PCSType/Compname " << pcs_type_name << ", " << comp_name;
//        cout << " Process number " << m_pcs->pcs_number << ", compnumber " << m_pcs->pcs_component_number << endl;
        return m_pcs;
      }
    }
  }
  return NULL;
}

/**************************************************************************
PCSLib-Method:
12/2005 OK Implementation
**************************************************************************/
CRFProcess* PCSGet(string var_name,bool bdummy)
{
  int j;
  string pcs_var_name;
  CRFProcess *m_pcs = NULL;
  bdummy = bdummy; //WW
  for(int i=0;i<(int)pcs_vector.size();i++){
    m_pcs = pcs_vector[i];
    for(j=0;j<m_pcs->GetPrimaryVNumber();j++){
      pcs_var_name = m_pcs->pcs_primary_function_name[j];
      if(pcs_var_name.compare(var_name)==0){
        return m_pcs;
      }
    }
    for(j=0;j<m_pcs->GetSecondaryVNumber();j++){
      pcs_var_name = m_pcs->pcs_secondary_function_name[j];
      if(pcs_var_name.compare(var_name)==0){
        return m_pcs;
      }
    }
  }
  return NULL;
}
/**************************************************************************
PCSLib-Method:
05/2006 CMCD Implementation
**************************************************************************/
CRFProcess* PCSGetFluxProcess()
{
  CRFProcess *m_pcs=NULL;
  int i;
  bool found = false;
  string help;
  int no_processes = (int) pcs_vector.size();
  for (i=0; i<no_processes; i++){
    help = pcs_vector[i]->pcs_type_name;
    if (pcs_vector[i]->pcs_type_name=="LIQUID_FLOW") {
       m_pcs = pcs_vector[i];
       found = true;
       }
    if (pcs_vector[i]->pcs_type_name=="GROUNDWATER_FLOW"){
       m_pcs = pcs_vector[i];
       found = true;
       }
    if (found) return m_pcs;
  }
  return NULL;
}
/*************************************************************************
GeoSys-FEM Function:
Task: 
For Richards, dual-Richards and multi-phase flow.
Programming: 
02/2007 WW Implementation
**************************************************************************/
void CRFProcess::CalcSecondaryVariablesUnsaturatedFlow(const bool initial)
{
  int ii;
  long i;
  double p_cap;
  int idxp,idxcp,idxS; 
  //----------------------------------------------
  vector<string> secondSNames; 
  vector<string> secondCPNames; 
  secondSNames.push_back("SATURATION1"); 
  if(type==1212)  // Multiphase flow 
    secondCPNames.push_back("PRESSURE1"); 
  else
    secondCPNames.push_back("PRESSURE_CAP1"); 
  if(continuum_vector.size()==2)
  {
    secondSNames.push_back("SATURATION2"); 
    secondCPNames.push_back("PRESSURE_CAP2");       
  }   

  //
  CElem* elem =NULL;
  CFiniteElementStd* fem = GetAssembler();
  //----------------------------------------------------------------------
  for(ii=0;ii<(int)secondSNames.size();ii++) 
  {
     idxS = GetNodeValueIndex(secondSNames[ii].c_str())+1;
     if(type==1212)  // Multiphase flow
     {
       idxcp = GetNodeValueIndex(pcs_primary_function_name[ii])+1;
       idxp = GetNodeValueIndex("PRESSURE_W");
     }
     else
     {
       idxp = GetNodeValueIndex(pcs_primary_function_name[ii])+1;
       idxcp = GetNodeValueIndex(secondCPNames[ii].c_str());
     }
     //----------------------------------------------------------------------
     // Capillary pressure
     for(i=0;i<(long)m_msh->GetNodesNumber(false);i++)
     {
        // Copy the new saturation to the old level
        // 
        if(type==1212)  // Multiphase flow 
        {
           // p_w = p_g-p_c
           p_cap = GetNodeValue(i,idxcp+2)-GetNodeValue(i,idxcp);  
           SetNodeValue(i,idxp,p_cap);
        }
        else
        {
           p_cap = -GetNodeValue(i,idxp);
           SetNodeValue(i,idxcp,p_cap);
        }
        SetNodeValue(i,idxS, 0.0);
     }
  }
  // Cal S
  for(ii=0;ii<(int)secondSNames.size();ii++)  
  {
    continuum = ii;
    for(i=0;i<(long)m_msh->ele_vector.size();i++)
    {
       elem = m_msh->ele_vector[i];
       if (elem->GetMark()) // Marked for use
       {
          elem->SetOrder(false);
          fem->ConfigElement(elem,false);
          fem->CalcSatution();
       }
    }
  }
  //
  if(!initial)
    return;  
  //----------
  for(ii=0;ii<(int)secondSNames.size();ii++)  
  {
    idxS = GetNodeValueIndex(secondSNames[ii].c_str())+1;
    for(i=0;i<(long)m_msh->GetNodesNumber(false);i++)
      SetNodeValue(i,idxS-1,  GetNodeValue(i, idxS));     
  }   
}


/*************************************************************************
GeoSys-FEM Function:
Task: Calculates saturation for richards flow,
      alternative to CRFProcess::CalcSecondaryVariablesRichards
      uses pressure of only one node
	  evoked by switch case in pcs-file: SATURATION_SWITCH = true
Programming: 
06/2007 JOD Implementation
**************************************************************************/
void CRFProcess::CalcSaturationRichards(int timelevel, bool update)
{
  int j;
  long i;
  long group;
  double p_cap, saturation;
  double volume_sum;
  long  elemsCnode, elem_index;
  static double Node_Cap[8];
  int idxp,idxcp,idxS,idx_tS=-1; 
  int number_continuum;
  double total_S;
  int i_pv,i_s,i_e;

  CMediumProperties* m_mmp = NULL;
  CElem* elem =NULL;
//WW  CFiniteElementStd* fem = GetAssembler();
  number_continuum = (int)continuum_vector.size();
  //----------------------------------------------------------------------
  if(continuum_ic){  //Create IC: for both continua
     i_s = 0;
     i_e = number_continuum;
  }
  else{
	 i_s = continuum;
	 i_e = continuum+1;
  }
//----------------------------------------------------------------------
  for(i_pv=i_s;i_pv<i_e;i_pv++)
  {
     idxp = GetNodeValueIndex(pcs_primary_function_name[i_pv])+ timelevel;
     idxS = GetNodeValueIndex(pcs_secondary_function_name[i_pv*number_continuum])+ timelevel;
	 idxcp = GetNodeValueIndex(pcs_secondary_function_name[i_pv*number_continuum+number_continuum*2])+ timelevel;
	 if((int)continuum_vector.size()>1)
		 idx_tS = GetNodeValueIndex("TOTAL_SATURATION")+ timelevel;
  //----------------------------------------------------------------------
 
  for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++)
  {  	 
  // Capillary pressure
   
      p_cap = -GetNodeValue(i,idxp);
      if(timelevel==1&&update) 
		  SetNodeValue(i,idxcp-1,GetNodeValue(i,idxcp));
      SetNodeValue(i,idxcp,p_cap);
	  if(timelevel==1&&update) 
		  SetNodeValue(i,idxS-1,GetNodeValue(i,idxS));
  //----------------------------------------------------------------------
  // Liquid saturation
  	  if((int)continuum_vector.size()>1)
          SetNodeValue(i,idx_tS, 0.0);  
  // 

	 
        saturation = 0., volume_sum  = 0.;
        elemsCnode = (long)m_msh->nod_vector[i]->connected_elements.size();

	    for (j = 0; j < elemsCnode; j++) {
           elem_index = m_msh->nod_vector[i]->connected_elements[j];
		   elem = m_msh->ele_vector[elem_index];
		   group = elem->GetPatchIndex();
		   m_mmp = mmp_vector[group];
		   volume_sum += elem->volume;
           saturation += m_mmp->SaturationCapillaryPressureFunction(p_cap,(int)mfp_vector.size()-1) * elem->volume; 
	    }
	  saturation /= volume_sum;
	  SetNodeValue(i,idxS, saturation);
  
  }

}
  //----------

  if(continuum > 0){
	for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++){
      total_S = 0;
	  for(j = 0; j< (int)continuum_vector.size(); j++){
        idxS = GetNodeValueIndex(pcs_secondary_function_name[j*number_continuum])+ timelevel;
	    total_S += GetNodeValue(i, idxS)*continuum_vector[j];
      }
      SetNodeValue(i,idx_tS, total_S);
	  }
  }


}

/**************************************************************************
   GeoSys - Function: Get mean element value for element index from secondary node values
				      of process pcs_name and for variable var_name; old and new timelevel
    01/2006   SB    Implementation
**************************************************************************/
double PCSGetEleMeanNodeSecondary(long index, string pcs_name, string var_name, int timelevel){

double val = 1.0; // As this returns saturation, default is fully saturated = 1.0;
int idx, j;
long enode;
CRFProcess *m_pcs = NULL;

// Get process by process name
m_pcs = PCSGet(pcs_name); 
if(m_pcs){
    // Get index of secondary node value
    idx = m_pcs->GetNodeValueIndex(var_name)+timelevel; 
    // Get element with index index
	CElem* elem =NULL; 
	elem = m_pcs->m_msh->ele_vector[index];
	val = 0.0;
	for(j=0; j<elem->GetVertexNumber(); j++){ // average all adjoining nodes
		enode = elem->GetNodeIndex(j);
        val +=  m_pcs->GetNodeValue(enode,idx);
	}
	val = val/((double)elem->GetVertexNumber());
}
return val;
}

/*************************************************************************
GeoSys-FEM Function:
01/2006 OK Implementation
**************************************************************************/
void CRFProcess::SetNODFlux()
{
  long i;
  //----------------------------------------------------------------------
  int nidx;
  nidx = GetNodeValueIndex("FLUX");
  if(nidx<0)
    return;
  double m_val;
  for(i=0;i<(long)m_msh->nod_vector.size();i++)
  {
    m_val = eqs->b[i]; //? m_nod->eqs_index
    SetNodeValue(i,nidx,m_val);
  }
  //----------------------------------------------------------------------
}

/*************************************************************************
GeoSys-FEM Function:
01/2006 OK Implementation
**************************************************************************/
void CRFProcess::AssembleParabolicEquationRHSVector()
{
  long i;
  //----------------------------------------------------------------------
  // Init
/*	PCH & WW
  for(i=0;i<eqs->dim;i++)
  {
    eqs->b[i] = 0.0;
  }
  //----------------------------------------------------------------------
  CElem* m_ele = NULL;
  for(i=0;i<(long)m_msh->ele_vector.size();i++)
  {
    m_ele = m_msh->ele_vector[i];
    if(m_ele->GetMark()) // Marked for use
    {
      fem->ConfigElement(m_ele,false);
      fem->AssembleParabolicEquationRHSVector();
      //fem->AssembleParabolicEquationLHSMatrix();
    } 
  }
  */
  //----------------------------------------------------------------------
}

/*************************************************************************
GeoSys-FEM Function:
06/2006 YD Implementation
02/2008 JOD removed
Reload primary variable
**************************************************************************/
/*void CRFProcess::PrimaryVariableReload()
{
  char pcsT;
  pcsT = pcs_type_name[0];
  switch(pcsT){
    case 'L':
      break;
    case 'U':
      break;
    case 'G':
      break;
    case 'T':
      break;
    case 'C':
      break;
    case 'R': // Richards flow
      PrimaryVariableReloadRichards();
      break;
  }
}*/
/*************************************************************************
GeoSys-FEM Function:
06/2006 YD Implementation
02/2008 JOD removed
Reload primary variable of Richards Flow
**************************************************************************/
/*void CRFProcess::PrimaryVariableReloadRichards()
{
  int i;
  int idxp,idx_storage;
  double storage_p;

  idxp = GetNodeValueIndex(pcs_primary_function_name[0]);
  idx_storage = GetNodeValueIndex("STORAGE_P");
  for(i=0;i<(long)m_msh->GetNodesNumber(false);i++){
    storage_p = GetNodeValue(i,idx_storage);
    SetNodeValue(i,idxp,storage_p);
    SetNodeValue(i,idxp+1,storage_p);
  }     
  CalcSecondaryVariables(0);
  CalcSecondaryVariables(1);
}*/

/*************************************************************************
GeoSys-FEM Function:
06/2006 YD Implementation
02/2008 JOD removed
Reload primary variable of Richards Flow
**************************************************************************/
/*void CRFProcess::PrimaryVariableStorageRichards()
{
  int i;
  int idxp,idx_storage;
  double pressure;

  idxp = GetNodeValueIndex(pcs_primary_function_name[0])+1;
  idx_storage = GetNodeValueIndex("STORAGE_P");
  for(i=0;i<(long)m_msh->GetNodesNumber(false);i++){
    pressure = GetNodeValue(i,idxp);
    SetNodeValue(i,idx_storage,pressure);
    SetNodeValue(i,idx_storage+1,pressure);
  }     
}*/
/**************************************************************************
FEMLib-Method: 
11/2005 MB Implementation
03/2006 OK 2nd version (binary coupling)
**************************************************************************/
void CRFProcess::SetCPL()
{
  int i;
  double value=0.0;
  //----------------------------------------------------------------------
  // Nothing to do
  if((cpl_type_name.size()==0)||(cpl_type_name.compare("PARTITIONED")==0))
    return;
  //----------------------------------------------------------------------
  // PCS CPL
  CRFProcess* m_pcs_cpl = PCSGet(cpl_type_name);
  if(!m_pcs_cpl)
  {
    cout << "Fatal error in CRFProcess::SetCPL: no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  // MSH data for PCS CPL
  CFEMesh* m_msh_cpl = m_pcs_cpl->m_msh;
  if(!m_msh_cpl)
  {
    cout << "Fatal error in CRFProcess::SetCPL: no MSH data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  // GEO data for PCS CPL
  Surface* m_sfc = GEOGetSFCByName(m_msh_cpl->geo_name);
  if(!m_sfc)
  {
    cout << "Fatal error in CRFProcess::SetCPL: no GEO data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  //......................................................................
  // MSH nodes of PCS CPL
  cout << "      ->CPL: " << cpl_type_name << ": ";
  vector<long>cpl_msh_nodes_vector;
  m_msh_cpl->GetNODOnSFC(m_sfc,cpl_msh_nodes_vector);
  if((int)cpl_msh_nodes_vector.size()==0)
  {
    cout << "Warning in CRFProcess::SetCPL: no MSH nodes found" << endl;
  }
  cout << "CPL nodes = " << (int)cpl_msh_nodes_vector.size() << endl;
  //.....................................................................-
  // MSH nodes of PCS
  cout << "      ->CPL: " << pcs_type_name << ": ";
  vector<long>msh_nodes_vector;
  m_msh->GetNODOnSFC(m_sfc,msh_nodes_vector);
  if((int)msh_nodes_vector.size()==0)
  {
    cout << "Warning in CRFProcess::SetCPL: no MSH nodes found" << endl;
  }
  cout << "CPL nodes = " << (int)msh_nodes_vector.size() << endl;
  //----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
if(m_msh_cpl->pcs_name.compare("RICHARDS_FLOW")==0)
{
  m_msh->SetNODPatchAreas();
  int nidx = GetNodeValueIndex("WDEPTH");
  long st_node_number;
  double st_node_value = 0.0;
  CNode* m_nod = NULL;
  for(i=0;i<(int)msh_nodes_vector.size();i++)
  {
    value = -2.314e-02;
    st_node_number = msh_nodes_vector[i];
    m_nod = m_msh->nod_vector[st_node_number];
    st_node_value = GetNodeValue(st_node_number,nidx);
    st_node_value /= m_nod->patch_area;
    value *= st_node_value;
    //cout << "CPL value = " << value << endl;
    eqs->b[st_node_number] += value;
  }
}
//////////////////////////////////////////////////////////////////////////
if(pcs_type_name.compare("RICHARDS_FLOW")==0&&m_msh_cpl->pcs_name.compare("OVERLAND_FLOW")==0) // ToDo
{
  long msh_node_number;
  long cpl_msh_nod_number;
  long cpl_msh_ele_number;
  value = 0.0;
  cout << "CPL value = " << value << endl;
/*
  CGLPoint* m_pnt = GEOGetPointByName("POINT18");
  msh_node_number = m_msh->GetNODOnPNT(m_pnt);
  eqs->b[msh_node_number] += value;
*/
  // PCS-CON
  CRFProcess* m_pcs_cond = PCSGet(cpl_type_name);
  //int nidx = m_pcs_cond->GetNodeValueIndex(m_pcs_cond->pcs_primary_function_name[0]);
  int cpl_nidx = m_pcs_cond->GetNodeValueIndex("WDEPTH");
  //----------------------------------------------------------------------
  // ELE of PCS_CPL related to NOD of PCS
//  CFEMesh* m_msh_this = MSHGet("RICHARDS_FLOW_LOCAL");
  CElem* m_ele_cnd = NULL;
  //----------------------------------------------------------------------
//  CSourceTermGroup *m_st_group = NULL;
//  CSourceTerm *m_st = NULL;
//  m_st_group = STGetGroup(pcs_type_name,pcs_primary_function_name[0]);
  //----------------------------------------------------------------------
  double cpl_ele_val = 0.0;
  int j;
  CNodeValue *cnodev = NULL;
//  for(i=0;i<(int)m_st_group->group_vector.size();i++)
//  ofstream st_out_file("st_out_file.txt",ios::app);
  for(i=0;i<(int)st_node_value.size();i++)
  {
    cnodev = st_node_value[i]; 
    // MSH-PCS
    //m_nod = m_msh_this->nod_vector[m_st_group->group_vector[i]->msh_node_number];
    msh_node_number = cnodev->msh_node_number; //m_st_group->group_vector[i]->msh_node_number; //0
    // MSH-PCS-CPL
    cpl_msh_nod_number = msh_node_number;
    cpl_msh_ele_number = pcs_number; //OK:TODO
    m_ele_cnd = m_pcs_cond->m_msh->ele_vector[cpl_msh_ele_number];
    for(j=0;j<m_ele_cnd->GetNodesNumber(false);j++)
    {
      cpl_ele_val += m_pcs_cond->GetNodeValue(m_ele_cnd->nodes_index[j],cpl_nidx);
    }
    cpl_ele_val /= m_ele_cnd->GetNodesNumber(false);
    // VAL-CON
    value = 2.314e-02 * cpl_ele_val * 1e-2;
//    st_out_file << value << endl;
    // EQS-RHS
    eqs->b[msh_node_number] += value;
  }
  //----------------------------------------------------------------------
/*
  CNodeValue* m_node_value = NULL;
  m_node_value = new CNodeValue();
  m_node_value->msh_node_number = msh_node_number;
  m_node_value->geo_node_number = m_pnt->id;
  m_node_value->node_value = value;
  CSourceTermGroup *m_st_group = NULL;
  m_st_group = STGetGroup(pcs_type_name,pcs_primary_function_name[0]);
  m_st_group->group_vector.push_back(m_node_value);
  m_st_group->st_group_vector.push_back(m_st); //OK
*/
}
//////////////////////////////////////////////////////////////////////////
if(pcs_type_name.compare("GROUNDWATER_FLOW")==0&&m_msh_cpl->pcs_name.compare("OVERLAND_FLOW")==0) // ToDo
{
  long ie = (long)msh_nodes_vector.size()/(m_msh->no_msh_layer+1);
  long of_node_number,gf_node_number;
  double of_node_value,gf_node_value;
//  CNode* m_nod = NULL;
  int of_nidx = GetNodeValueIndex("WDEPTH");

  for(i=0;i<ie;i++)
  {
    of_node_number = msh_nodes_vector[i]; //ToDo
    of_node_value = m_pcs_cpl->GetNodeValue(of_node_number,of_nidx);
    //m_nod = m_msh->nod_vector[gf_node_number];
    //st_node_value /= m_nod->patch_area;
    gf_node_value = of_node_value*2e-11;
if(gf_node_value>1e-13)
    cout << "CPL value = " << gf_node_value << endl;
    gf_node_number = msh_nodes_vector[i];
    eqs->b[gf_node_number] += gf_node_value;
  }
}
//////////////////////////////////////////////////////////////////////////
  int idx = GetNodeValueIndex("FLUX")+1;
  //for(i=0;i<(int)msh_nodes_vector.size();i++)
  for(i=0;i<1;i++)
  {
    SetNodeValue(msh_nodes_vector[i],idx,value);
  }
}

/**************************************************************************
PCSLib-Method: 
04/2006 OK Implementation
**************************************************************************/
void CRFProcess::CreateBCGroup()
{
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable
  cout << "->Create BC" << '\n';
  CBoundaryConditionsGroup *m_bc_group = NULL;
  for(int i=0;i<DOF;i++){
    BCGroupDelete(pcs_type_name,pcs_primary_function_name[i]);
    m_bc_group = new CBoundaryConditionsGroup();
    m_bc_group->pcs_type_name = pcs_type_name; //OK
    m_bc_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
    m_bc_group->Set(this,Shift[i]);
    bc_group_list.push_back(m_bc_group);
  } 
}

/**************************************************************************
PCSLib-Method: 
04/2006 OK Implementation
**************************************************************************/
void CRFProcess::CreateSTGroup()
{
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable

  cout << "->Create ST" << '\n';
  CSourceTermGroup *m_st_group = NULL;
  // WW
  ifstream *iSourceNBC_RHS_file = NULL;
  ofstream *oSourceNBC_RHS_file = NULL;
  if(WriteSourceNBC_RHS==1)
  {
     string m_file_name = FileName +"_"+pcs_type_name+"_source_Neumann_RHS.bin";
     oSourceNBC_RHS_file = new ofstream(m_file_name.c_str(), ios::binary);     
  }
  else if(WriteSourceNBC_RHS==2)
  {
     string m_file_name = FileName +"_"+pcs_type_name+"_source_Neumann_RHS.bin";
     iSourceNBC_RHS_file = new ifstream(m_file_name.c_str(), ios::binary);     
     if (!iSourceNBC_RHS_file->good())
       cout << "_source_Neumann_RHS file is not found" << endl;
  }

  for(int i=0;i<DOF;i++)
  {
    //OK m_st_group = m_st_group->Get(pcs_primary_function_name[i]);
    m_st_group = STGetGroup(pcs_type_name,pcs_primary_function_name[i]);
    if(!m_st_group) {
      m_st_group = new CSourceTermGroup();
      m_st_group->pcs_type_name = pcs_type_name; //OK
      m_st_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
//      if(iSourceNBC_RHS_file)  // Read from data. WW
//        m_st_group->Read(*iSourceNBC_RHS_file);
//	  else
        m_st_group->Set(this,Shift[i]);
      st_group_list.push_back(m_st_group);
    }
  }
  if(oSourceNBC_RHS_file)// WW
//    WriteRHS_of_ST_NeumannBC(*oSourceNBC_RHS_file);

  if(iSourceNBC_RHS_file) //WW
  {
    iSourceNBC_RHS_file->close();
     delete iSourceNBC_RHS_file;
     iSourceNBC_RHS_file = NULL; 
  }
  if(oSourceNBC_RHS_file) //WW
  {
     oSourceNBC_RHS_file->close();
     delete oSourceNBC_RHS_file;
     oSourceNBC_RHS_file = NULL; 
  }
}

/**************************************************************************
PCSLib-Method: 
08/2006 OK Implementation
**************************************************************************/
void CRFProcess::CalcELEFluxes(CGLPoint*m_pnt)
{
  m_pnt = NULL;
}

/**************************************************************************
PCSLib-Method: 
08/2006 OK Implementation
**************************************************************************/
double CRFProcess::CalcELEFluxes(CGLPolyline*m_ply)
{
  long i;
/*BUGFIX_4402_OK_2
  int f_eidx[3];
  f_eidx[0] = GetElementValueIndex("FLUX_X");
  f_eidx[1] = GetElementValueIndex("FLUX_Y");
  f_eidx[2] = GetElementValueIndex("FLUX_Z");
  for(i=0;i<3;i++)
  {
    if(f_eidx[i]<0)
    {
      //cout << "Fatal error in CRFProcess::CalcELEFluxes(CGLPolyline*m_ply) - abort"; abort();
      cout << "Warning in CRFProcess::CalcELEFluxes(CGLPolyline*m_ply) - return" << endl; 
      return 0.0;
    }
  }
*/
  double f[3];
  int v_eidx[3];
  CRFProcess* m_pcs_flow = NULL;
  if(pcs_type_name.find("FLOW")!=string::npos)
  {
    m_pcs_flow = this;
  }
  else
  {
    m_pcs_flow = PCSGet("GROUNDWATER_FLOW");
  }
  v_eidx[0] = m_pcs_flow->GetElementValueIndex("VELOCITY1_X");
  v_eidx[1] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Y");
  v_eidx[2] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Z");
  for(i=0;i<3;i++)
  {
    if(v_eidx[i]<0)
    {
      //cout << "Fatal error in CRFProcess::CalcELEFluxes(CGLPolyline*m_ply) - abort";
      cout << i << v_eidx[i] << "Velocity output is not specified"<<endl;
      return 0.0;
      //  abort();
    }
  }
  double v[3];
  //----------------------------------------------------------------------
  // Get elements at GEO
  vector<long>ele_vector_at_geo;
  m_msh->GetELEOnPLY(m_ply,ele_vector_at_geo);
  //----------------------------------------------------------------------
  CElem* m_ele = NULL;
  CEdge* m_edg = NULL;
  vec<CEdge*>ele_edges_vector(15);
  int j;
  double edg_normal_vector[3];
  double vn;
  double edg_length=0.0;
  double vn_vec[3];  
  double edge_vector[3];
  double f_n_sum = 0.0;
  double C_ele = 0.0;
  vec<long>element_nodes(20);
  int nidx;
  //======================================================================
  for(i=0;i<(long)ele_vector_at_geo.size();i++)
  {
    m_ele = m_msh->ele_vector[ele_vector_at_geo[i]];
    m_ele->SetNormalVector();
    m_ele->GetNodeIndeces(element_nodes);
/*BUGFIX_4402_OK_3
    for(j=0;j<3;j++)
    {
      SetElementValue(m_ele->GetIndex(),f_eidx[j],0.0);
    }
*/
    //--------------------------------------------------------------------
    // velocity vector
    for(j=0;j<3;j++)
    {
      v[j] = m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[j]);
    }
    // edge projection // edge marked
    m_ele->GetEdges(ele_edges_vector);
    for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
    {
      m_edg = ele_edges_vector[j];
      if(m_edg->GetMark())
      {
        m_edg->SetNormalVector(m_ele->normal_vector,edg_normal_vector);
        edg_length = m_edg->Length();
        m_edg->GetEdgeVector(edge_vector);
      }
    }
    vn = MSkalarprodukt(v,edg_normal_vector,3);
    for(j=0;j<3;j++)
    {
      vn_vec[j] = vn * edg_normal_vector[j];
    }
    //--------------------------------------------------------------------
    switch(pcs_type_name[0])
    {
      //..................................................................
      case 'L': // Liquid flow
        break;
      case 'G': // Groundwater flow
        // Volume flux = v_n * l^e * z^e
        for(j=0;j<3;j++)
        {
          f[j] = vn_vec[j] * edg_length * m_ele->GetFluxArea();
        }
        break;
      case 'T': // Two-phase flow
        break;
      case 'C': // Componental flow
        break;
      case 'H': // heat transport
        break;
      case 'M': // Mass transport
        // Mass flux = v_n * l^e * z^e * C^e
        C_ele = 0.0;
        for(j=0;j<m_ele->GetNodesNumber(false);j++)
        {
          nidx = GetNodeValueIndex(pcs_primary_function_name[0])+1;
          C_ele =+ GetNodeValue(element_nodes[j],nidx);
        }
        C_ele /=(double)m_ele->GetNodesNumber(false);
        for(j=0;j<3;j++)
        {
          f[j] = vn_vec[j] * edg_length * m_ele->GetFluxArea() * C_ele;
        }
        break;
      case 'O': // Overland flow
        break;
      case 'R': //OK4104 Richards flow
        break;
      case 'A': // Air (gas) flow
        break;
	  case 'F':	// Fluid Momentum Process
        break;
    }
    //--------------------------------------------------------------------
    // set
/*BUGFIX_4402_OK_4
    for(j=0;j<3;j++)
    {
      SetElementValue(m_ele->GetIndex(),f_eidx[j],f[j]);
    }
    // overall flux across polyline
*/
    f_n_sum += MBtrgVec(f,3);
  }
  //======================================================================
  ele_vector_at_geo.clear();
  //----------------------------------------------------------------------
  return f_n_sum;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2004 OK Implementation
08/2006 OK new
**************************************************************************/
void CRFProcess::CalcELEVelocities(void)
{
  long i;
  //----------------------------------------------------------------------
  int eidx[3];
  eidx[0] = GetElementValueIndex("VELOCITY1_X");
  eidx[1] = GetElementValueIndex("VELOCITY1_Y");
  eidx[2] = GetElementValueIndex("VELOCITY1_Z");
  for(i=0;i<3;i++)
  {
    if(eidx[i]<0)
    {
      cout << "Fatal error in CRFProcess::CalcELEVelocities - abort"; //abort();	// PCH commented abort() out for FM.
    }
  }
  //----------------------------------------------------------------------
  CElem* m_ele = NULL;
  FiniteElement::ElementValue* gp_ele = NULL;
  double vx,vy,vz;
  for(i=0l;i<(long)m_msh->ele_vector.size();i++){
    m_ele = m_msh->ele_vector[i];
    gp_ele = ele_gp_value[i];
    vx = gp_ele->Velocity(0,0);
    SetElementValue(i,eidx[0],vx);
    SetElementValue(i,eidx[0]+1,vx);
    vy = gp_ele->Velocity(1,0);
    SetElementValue(i,eidx[1],vy);
    SetElementValue(i,eidx[1]+1,vy);
    vz = gp_ele->Velocity(2,0);
    SetElementValue(i,eidx[2],vz);
    SetElementValue(i,eidx[2]+1,vz);
  }
  //----------------------------------------------------------------------
}

/*************************************************************************
GeoSys-FEM Function:
08/2006 OK Implementation
**************************************************************************/
void CRFProcess::AssembleParabolicEquationRHSVector(CNode*m_nod)//(vector<long>&ele_number_vector)
{
  //cout << "CRFProcess::AssembleParabolicEquationRHSVector" << endl;
  int i;
  long ldummy;
  double ddummy;
  //----------------------------------------------------------------------
  // Init
  for(i=0;i<(int)m_nod->connected_elements.size();i++)
  {
    eqs->b[m_nod->connected_elements[i]] = 0.0;
  }
  //----------------------------------------------------------------------
  CElem* m_ele = NULL;
  CEdge* m_edg = NULL;
  double edg_normal_vector[3];
  double edge_mid_point[3];
  vec<CEdge*>ele_edges_vector(15);
  int j;
  double aux_vector[3];
  double* gravity_center = NULL;
  double check_sign;
  //----------------------------------------------------------------------
  // Element velocity
  int v_eidx[3];
//kg44  v_eidx[0] = GetElementValueIndex("VELOCITY1_X");
//kg44  v_eidx[1] = GetElementValueIndex("VELOCITY1_Y");
//kg44  v_eidx[2] = GetElementValueIndex("VELOCITY1_Z");
  CRFProcess* m_pcs_flow = NULL;
  if(pcs_type_name.find("FLOW")!=string::npos)
  {
    m_pcs_flow = this;
  }
  else
  {
    m_pcs_flow = PCSGet("GROUNDWATER_FLOW");
  }
  v_eidx[0] = m_pcs_flow->GetElementValueIndex("VELOCITY1_X");
  v_eidx[1] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Y");
  v_eidx[2] = m_pcs_flow->GetElementValueIndex("VELOCITY1_Z");
  for(i=0;i<3;i++)
  {
    if(v_eidx[i]<0)
    {
      cout << v_eidx[i] << i << " Warning in CRFProcess::AssembleParabolicEquationRHSVector - no PCS-VEL data" << endl;
      return;
    }
  }
  double v[3];
  //======================================================================
  // Topology
  for(i=0;i<(int)m_nod->connected_elements.size();i++)
  {
    m_ele = m_msh->ele_vector[m_nod->connected_elements[i]];
    m_ele->SetNormalVector(); //OK_BUGFIX
    v[0] = m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[0]);
    v[1] = m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[1]);
    v[2] = m_pcs_flow->GetElementValue(m_ele->GetIndex(),v_eidx[2]);
    m_ele->SetMark(false);
    switch(m_ele->GetElementType())
    {
      //------------------------------------------------------------------
      // line elements
      case 1:
         v[1] = GetElementValue(m_ele->GetIndex(),v_eidx[0]);
         v[0] = GetElementValue(m_ele->GetIndex(),v_eidx[1]);
        if(m_nod->connected_elements.size()==1)
        {
          m_ele->SetMark(true);
          break;
        }
        gravity_center = m_ele->GetGravityCenter();
        aux_vector[0] = gravity_center[0] - m_nod->X();
        aux_vector[1] = gravity_center[1] - m_nod->Y();
        aux_vector[2] = gravity_center[2] - m_nod->Z();
        check_sign = MSkalarprodukt(v,aux_vector,3);
        if(check_sign<0.0)
          m_ele->SetMark(true);
        break;
      //------------------------------------------------------------------
      // tri elements
      case 4:
        m_ele->GetEdges(ele_edges_vector);
        for(j=0;j<(int)m_ele->GetEdgesNumber();j++)
        {
          m_edg = ele_edges_vector[j];
          if(m_edg->GetMark())
          {
            m_edg->SetNormalVector(m_ele->normal_vector,edg_normal_vector);
            break;
/*
            m_edg->GetEdgeMidPoint(edge_mid_point);
           gravity_center = m_ele->GetGravityCenter();
           aux_vector[0] = gravity_center[0] - edge_mid_point[0];
           aux_vector[1] = gravity_center[1] - edge_mid_point[1];
           aux_vector[2] = gravity_center[2] - edge_mid_point[2];
           check_sign = MSkalarprodukt(edg_normal_vector,aux_vector,3);
           if(check_sign<0.0) break;
*/
          }
        }
        if(m_edg->GetMark()) break;
      //----------------------------------------------------------------
      // ToDo
      default:
        cout << "Warning in CRFProcess::AssembleParabolicEquationRHSVector - not implemented for this element type" << endl;
        break;
    } // switch
  }
  //======================================================================
  for(i=0;i<(int)m_nod->connected_elements.size();i++)
  {
    m_ele = m_msh->ele_vector[m_nod->connected_elements[i]];
    switch(m_ele->GetElementType())
    {
      //------------------------------------------------------------------
      // line elements
      case 1:
        if(m_ele->GetMark())
        {
          cout << m_ele->GetIndex() << endl;
ldummy = m_nod->GetIndex();
ddummy = eqs->b[m_nod->GetIndex()];
          fem->ConfigElement(m_ele,false);
          fem->AssembleParabolicEquationRHSVector();
ddummy = eqs->b[m_nod->GetIndex()];
        } 
        break;
      //------------------------------------------------------------------
      // tri elements
      case 4:
        m_edg->GetEdgeMidPoint(edge_mid_point);
        gravity_center = m_ele->GetGravityCenter();
        aux_vector[0] = gravity_center[0] - edge_mid_point[0];
        aux_vector[1] = gravity_center[1] - edge_mid_point[1];
        aux_vector[2] = gravity_center[2] - edge_mid_point[2];
        check_sign = MSkalarprodukt(edg_normal_vector,aux_vector,3);
        if(check_sign<0.0) continue;
        {
          //cout << m_ele->GetIndex() << endl;
          fem->ConfigElement(m_ele,false);
          fem->AssembleParabolicEquationRHSVector();
        } 
        break;
      //----------------------------------------------------------------
      // ToDo
      default:
        cout << "Warning in CRFProcess::AssembleParabolicEquationRHSVector - not implemented for this element type" << endl;
        break;
    } // switch
  }
  //======================================================================
}

/**************************************************************************
PCSLib-Method:
08/2006 OK Implementation
compare with CMCDs PCSGetFluxProcess
**************************************************************************/
CRFProcess* PCSGetFlow()
{
  CRFProcess *m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++)
  {
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
    {
      return m_pcs;
    }
  }
  return NULL;
}

/**************************************************************************
PCSLib-Method:
based on MMPCalcSecondaryVariables
01/2007 OK Implementation
**************************************************************************/

/**************************************************************************
PCSLib-Method:
based on MMPCalcSecondaryVariables
01/2007 OK Implementation
**************************************************************************/
void CRFProcess::SetBC()
{
  CBoundaryCondition *m_bc = NULL;
  CBoundaryConditionNode *m_node = NULL;
  int nidx = GetNodeValueIndex(pcs_primary_function_name[0]);
  for(long i=0;i<(long)bc_node_value.size();i++){
      m_node = bc_node_value[i];
      m_bc = bc_node[i];
      SetNodeValue(m_node->msh_node_number,nidx,m_node->node_value); // old time
      SetNodeValue(m_node->msh_node_number,nidx+1,m_node->node_value); // new time
  }
}

/**************************************************************************
PCSLib-Method:
based on WriteSolution by WW
01/2007 OK Implementation
**************************************************************************/
void CRFProcess::WriteAllVariables()
{
  string m_file_name = FileName +"_"+pcs_type_name + "_" + pcs_primary_function_name[0] + ".asc";
  ofstream os(m_file_name.c_str(), ios::trunc|ios::out);     	
  if (!os.good())
  {
    cout << "Failure to open file: "<<m_file_name << endl;
    abort();
  }
  //
  int j;
  int idx[20];
  for(j=0;j<pcs_number_of_primary_nvals;j++) 
  {
    os << pcs_primary_function_name[j] << " ";
    idx[j] = GetNodeValueIndex(pcs_primary_function_name[j]);
    os << pcs_primary_function_name[j] << " ";
    idx[j+pcs_number_of_primary_nvals] = idx[j]+1;
  }
  for(j=0;j<pcs_number_of_secondary_nvals;j++) 
  {
    os << pcs_secondary_function_name[j] << " ";
    idx[2*pcs_number_of_primary_nvals+j] = GetNodeValueIndex(pcs_secondary_function_name[j]);
  }
  os << endl;
  long i;
  for(i=0;i<m_msh->GetNodesNumber(false);i++)
  {
    for(j=0;j<2*pcs_number_of_primary_nvals;j++)
      os<<GetNodeValue(i,idx[j]) <<"  ";
    for(j=0;j<pcs_number_of_secondary_nvals;j++)
      os<<GetNodeValue(i,idx[2*pcs_number_of_primary_nvals+j]) <<"  ";
    os<<endl;
  }  
  os.close();
}

/**************************************************************************
PCSLib-Method:
based on MMPCalcSecondaryVariables
01/2007 OK Implementation
**************************************************************************/
void MMPCalcSecondaryVariablesNew(CRFProcess*m_pcs)
{
  long i;
  double p_cap;
  double s_liquid,s_gas;
  //----------------------------------------------------------------------
  int ndx_density_phase;
  int ndx_viscosity_phase;
  ndx_density_phase = -1; //WW
  ndx_viscosity_phase = -1; //WW
  //----------------------------------------------------------------------
  m_pcs->SetBC();
  //======================================================================
  switch(m_pcs->pcs_type_number)
  {
    case 0:
      //..................................................................
      ndx_density_phase = m_pcs->GetNodeValueIndex("DENSITY1");
      ndx_viscosity_phase = m_pcs->GetNodeValueIndex("VISCOSITY1");
      break;

    case 1:
      //..................................................................
      // Initial conditions
      // Calc secondary variable saturation S^g=1-S^l
      int ndx_sl_old = m_pcs->GetNodeValueIndex("SATURATION2");
      int ndx_sg_old = m_pcs->GetNodeValueIndex("SATURATION1");
      int ndx_sg_new = ndx_sg_old+1;
      for(i=0;i<(long)m_pcs->m_msh->nod_vector.size();i++) 
      {
        s_liquid = m_pcs->GetNodeValue(i,ndx_sl_old);
        s_gas = MRange(0.0,1.0-s_liquid,1.0);
        m_pcs->SetNodeValue(i,ndx_sg_old,s_gas);
        m_pcs->SetNodeValue(i,ndx_sg_new,s_gas);
      }
      //......................................................................
      ndx_density_phase = m_pcs->GetNodeValueIndex("DENSITY2");
      ndx_viscosity_phase = m_pcs->GetNodeValueIndex("VISCOSITY2");
      break;
  }
  //======================================================================
  //----------------------------------------------------------------------
  // Capillary pressure - p_c (S)
  int ndx_p_cap = m_pcs->GetNodeValueIndex("PRESSURE_CAP");
  CMediumProperties *m_mmp = NULL;
  for(i=0;i<(long)m_pcs->m_msh->nod_vector.size();i++) 
  {
    m_mmp = mmp_vector[0];
    m_mmp->mode = 1;
    p_cap = 0.0; //m_mmp->CapillaryPressure(i,NULL,theta,phase); //MATCalcNodeCapillaryPressure(phase,index,i,theta);
    m_mmp->mode = 0;
    m_pcs->SetNodeValue(i,ndx_p_cap,p_cap);
  }
  //----------------------------------------------------------------------
  // Fluid properties
  double density;
  double viscosity;
  CFluidProperties* m_mfp = NULL;
  m_mfp = mfp_vector[m_pcs->pcs_type_number];
  m_mfp->mode = 1;
  for(i=0;i<(long)m_pcs->m_msh->nod_vector.size();i++) 
  {
    density = m_mfp->Density();
    m_pcs->SetNodeValue(i,ndx_density_phase,density);
    viscosity = m_mfp->Viscosity();
    m_pcs->SetNodeValue(i,ndx_viscosity_phase,viscosity);
  }
  m_mfp->mode = 0;
  //----------------------------------------------------------------------
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool CRFProcess::OBJRelations()
{
  bool succeed = true;
  //----------------------------------------------------------------------------
  cout << "OBJ->PCS relations" << '\n';
  //----------------------------------------------------------------------------
  // NUM
  cout << " - NUM->PCS" << '\n';
  m_num = NUMGet(pcs_type_name);
  if(!m_num)
  {
    cout << "Warning in CRFProcess::Create() - no NUM data - default" << endl;
    succeed = false;
  }
  else
  {
    pcs_nonlinear_iterations = m_num->nls_max_iterations;
    pcs_nonlinear_iteration_tolerance = m_num->nls_error_tolerance;
    num_type_name = "NEW"; //OK
  }
  //----------------------------------------------------------------------------
  // TIM
  cout << " - TIM->PCS" << '\n';
  Tim = TIMGet(pcs_type_name);
  if(Tim)
  {
    // Time unit factor //WW OK: -> TIM
    if(Tim->time_unit.find("MINUTE")!=string::npos) time_unit_factor=60.0;
    else if(Tim->time_unit.find("HOUR")!=string::npos) time_unit_factor=3600.0;
    else if(Tim->time_unit.find("DAY")!=string::npos) time_unit_factor=86400.0;
    else if(Tim->time_unit.find("MONTH")!=string::npos) time_unit_factor=2592000.0;
    else if(Tim->time_unit.find("YEAR")!=string::npos) time_unit_factor=31536000;
  }
  else
  {
    cout << "Warning in CRFProcess::Create() - no TIM data - default" << endl;
  }
  //----------------------------------------------------------------------------
  // OUT
  //----------------------------------------------------------------------------
  // MSH
  if((int)fem_msh_vector.size()==1)
    m_msh = fem_msh_vector[0];
  else 
    m_msh = MSHGet(pcs_type_name);
  if(!m_msh)
  {
    cout << "Warning in CRFProcess::Create() - no MSH data" << endl;
    succeed = false;
  }
  else
    m_msh->m_bCheckMSH = true;
  //----------------------------------------------------------------------------
  return succeed;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool CRFProcess::NODRelations()
{
  int i;
  bool succeed = true;
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable
  //----------------------------------------------------------------------------
  cout << "NOD->PCS relations" << '\n';
  //----------------------------------------------------------------------
  // BC
  cout << " - BC->PCS" << '\n';
  if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos) //WW
  {
    setBC_danymic_problems();
  }
  else
  {
    //.....................................................................
    // create BC groups for each process
    CBoundaryConditionsGroup *m_bc_group = NULL;
    for(i=0;i<DOF;i++)
    {
      BCGroupDelete(pcs_type_name,pcs_primary_function_name[i]);
      m_bc_group = new CBoundaryConditionsGroup();
      m_bc_group->pcs_type_name = pcs_type_name; //OK
      m_bc_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
      m_bc_group->Set(this,Shift[i]);
    }
  }
  //----------------------------------------------------------------------
  // ST
  if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos) //WW
  {
    setST_danymic_problems();
  }
  //......................................................................
  CSourceTermGroup *m_st_group = NULL;
  if(WriteSourceNBC_RHS==2) // Read from file
    ReadRHS_of_ST_NeumannBC();
  else // WW
  {  // Calculate directly
    for(i=0;i<DOF;i++)
    {
      m_st_group = STGetGroup(pcs_type_name,pcs_primary_function_name[i]);
      if(!m_st_group) 
      {
        m_st_group = new CSourceTermGroup();
        m_st_group->pcs_type_name = pcs_type_name; //OK
        m_st_group->pcs_pv_name = pcs_primary_function_name[i]; //OK
        m_st_group->Set(this,Shift[i]);
      }
    }
    if(WriteSourceNBC_RHS==1)// WW
      WriteRHS_of_ST_NeumannBC();
  }
  //----------------------------------------------------------------------
  // NOD values
  cout << "->Config NOD values" << '\n';
  //......................................................................
  // Names
  nod_val_name_vector.clear();
  //
  for(i=0;i<pcs_number_of_primary_nvals;i++)
  {
    nod_val_name_vector.push_back(pcs_primary_function_name[i]); // new time
    nod_val_name_vector.push_back(pcs_primary_function_name[i]); // old time //need this MB!
  }
  for(i=0;i<pcs_number_of_secondary_nvals;i++)
    nod_val_name_vector.push_back(pcs_secondary_function_name[i]); // new time
  if((int)nod_val_name_vector.size()!=(2*pcs_number_of_primary_nvals+pcs_number_of_secondary_nvals))
    succeed = false;
  //......................................................................
  // Values
  double* nod_values = NULL;
  for(i=0;i<(int)nod_val_vector.size();i++)
  {
    delete nod_val_vector[i];
    nod_val_vector[i] = NULL;
  }
  nod_val_vector.clear();
  //
  //OK m_msh->NodesNumber_Quadratic;
  number_of_nvals = 2*DOF + pcs_number_of_secondary_nvals;
  for(long j=0;j<(long)m_msh->nod_vector.size();j++)
  {
    nod_values =  new double[number_of_nvals];
    for(i=0;i<number_of_nvals;i++) nod_values[i] = 0.0;
      nod_val_vector.push_back(nod_values);
  }
  if((long)nod_val_vector.size()!=(long)m_msh->nod_vector.size())
    succeed = false;
  //-----------------------------------------------------------------------
  // IC
  //-----------------------------------------------------------------------
  cout << "->Assign IC" << '\n';
  //.......................................................................
  if(reload==2&&type!=4&&type!=41) 
    ReadSolution(); //WW
  //.......................................................................
  SetIC();
  //.......................................................................
  if(pcs_type_name_vector.size()&&pcs_type_name_vector[0].find("DYNAMIC")!=string::npos) //WW
    setIC_danymic_problems();
  //-----------------------------------------------------------------------
  return succeed;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool CRFProcess::ELERelations()
{
  int i;
  long j;
  bool succeed = true;
  //----------------------------------------------------------------------
  //OK->MB please shift to Config()
  if(pcs_type_name.compare("GROUNDWATER_FLOW")==0)
    MSHDefineMobile(this);
  //
  if(type==4||type==41) m_msh->SwitchOnQuadraticNodes(true); 
  else  m_msh->SwitchOnQuadraticNodes(false); 
  CheckMarkedElement();
  //m_msh->RenumberNodesForGlobalAssembly();
  //----------------------------------------------------------------------
  // ELE - GP values
  AllocateMemGPoint();
  //if((long)ele_gp_value.size()!=(long)m_msh->ele_vector.size())
    //succeed = false;
  //----------------------------------------------------------------------
  // ELE values
  double* ele_values = NULL;    // PCH
  int number_of_evals = 2*pcs_number_of_evals;  //PCH, increase memory
  if(number_of_evals>0) // WW added this "if" condition
  {
    //....................................................................
    // Names
    for(i=0;i<pcs_number_of_evals;i++)
    {
      ele_val_name_vector.push_back(pcs_eval_name[i]); // new time
      ele_val_name_vector.push_back(pcs_eval_name[i]); // old time
    }
    if(ele_val_name_vector.size()==0)
      succeed = false;
    //....................................................................
    // Values
    long m_msh_ele_vector_size = (long)m_msh->ele_vector.size();
    if(ele_val_vector.size()==0)
    {
      for(j=0;j<m_msh_ele_vector_size;j++)
      {
        ele_values =  new double[number_of_evals];
        size_eval += number_of_evals; //WW
        for(i=0;i<number_of_evals;i++) 
          ele_values[i] = 0.0;
        ele_val_vector.push_back(ele_values);
      }
    } 
    else
    {
      for(j=0;j<m_msh_ele_vector_size;j++)
      {
        ele_values = ele_val_vector[j];
/* //Comment by WW
#ifndef SX
#ifdef GCC
            size = malloc_usable_size( ele_values )/sizeof(double); 
#elif HORIZON
	    //KG44: malloc_usable_size and _msize are not available
#else 
            size= _msize( ele_values )/sizeof(double);
#endif
#endif
*/
        ele_values = resize(ele_values, size_eval, size_eval+ number_of_evals);
        size_eval += number_of_evals; 
        ele_val_vector[j] = ele_values;
      }
    }
    if((long)ele_val_vector.size()!=(long)m_msh->ele_vector.size())
      succeed = false;
  }
  //----------------------------------------------------------------------------
  // ELE matrices
  if(Memory_Type!=0)
  {
    AllocateLocalMatrixMemory();
    if((long)Ele_Matrices.size()!=(long)m_msh->ele_vector.size())
      succeed = false;
  }
  //----------------------------------------------------------------------
  // Element matrix output. WW
  if(Write_Matrix)
  {
    cout << "->Write Matrix" << '\n';
    string m_file_name = FileName +"_"+pcs_type_name+"_element_matrix.txt";
    matrix_file = new fstream(m_file_name.c_str(),ios::trunc|ios::out);
    if(!matrix_file->good())
      cout << "Warning in GlobalAssembly: Matrix files are not found" << endl;
  }
  //----------------------------------------------------------------------------
  // FEM
  if(type==4||type==41)
  {
    // Set initialization function
    CRFProcessDeformation *dm_pcs = (CRFProcessDeformation *) this;
    dm_pcs->Initialization(); 
    if(!dm_pcs->fem_dm)
      succeed = false;
  }
  else  // Initialize FEM calculator
  {
    int Axisymm = 1; // ani-axisymmetry
    if(m_msh->isAxisymmetry()) Axisymm = -1; // Axisymmetry is true
    fem = new CFiniteElementStd(this, Axisymm*m_msh->GetCoordinateFlag()); 
    fem->SetGaussPointNumber(m_num->ele_gauss_points);
    if(!fem)
      succeed = false;
  }
  //----------------------------------------------------------------------------
  return succeed;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool CRFProcess::CreateEQS()
{
  if(!m_num) return false; //OK46
  bool succeed = true;
  //----------------------------------------------------------------------------
  if(eqs)
    return false;
  //----------------------------------------------------------------------------
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable
  //----------------------------------------------------------------------------
  // EQS - create equation system
  cout << "->Create EQS" << '\n';
  //----------------------------------------------------------------------------
  if(type==4)
  {
    eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF,DOF*m_msh->GetNodesNumber(true));
    InitializeLinearSolver(eqs,m_num);
    PCS_Solver.push_back(eqs); //WW
  }
  //----------------------------------------------------------------------------
  else if(type==41)    
  {
    if(num_type_name.find("EXCAVATION")!=string::npos)
      eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF-1,DOF*m_msh->GetNodesNumber(true));
    else
      eqs = CreateLinearSolverDim(m_num->ls_storage_method,DOF, 
		    (DOF-1)*m_msh->GetNodesNumber(true)+m_msh->GetNodesNumber(false));  
    InitializeLinearSolver(eqs,m_num);
    PCS_Solver.push_back(eqs); //WW
  }
  //----------------------------------------------------------------------------
  else
  { 
/*
    // If there is a solver exsiting. WW 
    CRFProcess* m_pcs = NULL; 
    for(int i=0; i<(int)pcs_vector.size(); i++)
	{
      m_pcs = pcs_vector[i];
      if(m_pcs&&m_pcs->eqs)
	  {
        if(m_pcs->pcs_type_name.find("DEFORMATION")==string::npos)
          break;
	  }
	}
    // If unique mesh
	if(m_pcs&&m_pcs->eqs&&(fem_msh_vector.size()==1))
      eqs = m_pcs->eqs;
    //
	else
	{
*/
      eqs = CreateLinearSolver(m_num->ls_storage_method,m_msh->GetNodesNumber(false));
      InitializeLinearSolver(eqs,m_num);
      PCS_Solver.push_back(eqs); 
	//}
  }
  //----------------------------------------------------------------------------
  strcpy(eqs->pcs_type_name,pcs_type_name.data());
  //----------------------------------------------------------------------------
  if((int)PCS_Solver.size()==0)
    succeed = false;
  //----------------------------------------------------------------------------
  return succeed;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void PCSCreateNew()
{
  int i;
  CRFProcess* m_pcs = NULL;
  //----------------------------------------------------------------------
  for(i=0;i<(int)pcs_vector.size();i++)
  {
    m_pcs = pcs_vector[i];
    m_pcs->CreateNew();
  //----------------------------------------------------------------------
  }
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::CreateNew()
{
  pcs_type_number = (int)pcs_vector.size();
  Config();
  m_bCheckOBJ = OBJRelations();
  m_bCheckEQS = CreateEQS();
  m_bCheckNOD = NODRelations();
  m_bCheckELE = ELERelations();
  MMP2PCSRelation(this);
  ConfigureCouplingForLocalAssemblier();
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool CRFProcess::Check()
{
#ifdef MFC
  CString m_strMessage(pcs_type_name.data());
  m_strMessage += " -> ";
  //-----------------------------------------------------------------------
  // MSH
  if(!m_msh)
  {
    m_strMessage += "Error: no MSH data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // TIM
  CTimeDiscretization* m_tim = TIMGet(pcs_type_name);
  if(!m_tim)
  {
    m_strMessage += "Error: no TIM data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // NUM
  CNumerics* m_num = NUMGet(pcs_type_name);
  if(!m_num)
  {
    AfxMessageBox("Error: no NUM data");
    m_strMessage += "Error: no TIM data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // OUT
  COutput* m_out = OUTGet(pcs_type_name);
  if(!m_out)
  {
    m_strMessage += "Warning: no OUT data";
    AfxMessageBox(m_strMessage);
  }
  //-----------------------------------------------------------------------
  // IC
  CInitialCondition* m_ic = ICGet(pcs_type_name);
  if(!m_ic)
  {
    m_strMessage += "Warning: no IC data";
    AfxMessageBox(m_strMessage);
  }
  //-----------------------------------------------------------------------
  // BC
  CBoundaryCondition* m_bc = BCGet(pcs_type_name);
  if(!m_bc)
  {
    m_strMessage += "Error: no BC data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // ST
  CSourceTerm* m_st = STGet(pcs_type_name);
  if(!m_st)
  {
    m_strMessage += "Warning: no ST data";
    AfxMessageBox(m_strMessage);
  }
  //-----------------------------------------------------------------------
  // MFP
  if((pcs_type_name.find("FLOW")!=string::npos)&&((int)mfp_vector.size()==0))
  {
    m_strMessage += "Error: no MFP data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // MSP
  if((pcs_type_name.find("DEFORMATION")!=string::npos)&&((int)msp_vector.size()==0))
  {
    m_strMessage += "Error: no MSP data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  if((pcs_type_name.find("HEAT")!=string::npos)&&((int)msp_vector.size()==0))
  {
    m_strMessage += "Error: no MSP data";
    AfxMessageBox(m_strMessage);
    return false;
  }
  //-----------------------------------------------------------------------
  // MCP
  if((pcs_type_name.find("MASS")!=string::npos)&&((int)msp_vector.size()==0))
  {
    m_strMessage += "Error: no MCP data";
    AfxMessageBox(m_strMessage);
    return false;
  }
#endif
  //-----------------------------------------------------------------------
  // MMP
  MSHTestMATGroups();
  //-----------------------------------------------------------------------
  return true;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
bool PCSCheck()
{
  if((int)pcs_vector.size()==0)
    return false;
  CRFProcess* m_pcs = NULL;
  for(int i=0;i<(int)pcs_vector.size();i++)
  {
    m_pcs = pcs_vector[i];
    //if(m_pcs->m_bCheck)
      if(!m_pcs->Check())
        return false;
  }
  return true;
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void EQSDelete()
{
  LINEAR_SOLVER *eqs = NULL;
  CRFProcess* m_pcs = NULL;
  //----------------------------------------------------------------------
  for(int i=0;i<(int)PCS_Solver.size();i++)
  {
    eqs = PCS_Solver[i]; 
    m_pcs = PCSGet(eqs->pcs_type_name);
    if(eqs->unknown_vector_indeces)
       eqs->unknown_vector_indeces = \
        (int*) Free(eqs->unknown_vector_indeces);
    if(eqs->unknown_node_numbers)
      eqs->unknown_node_numbers = \
        (long*) Free(eqs->unknown_node_numbers);
    if(eqs->unknown_update_methods)
       eqs->unknown_update_methods = \
        (int*) Free(eqs->unknown_update_methods); 
    eqs = DestroyLinearSolver(eqs);
    if(m_pcs)
      m_pcs->eqs = NULL;
    PCS_Solver.erase((PCS_Solver.begin()+i));
  }
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::NODRelationsDelete()
{
  int i;
  int DOF = GetPrimaryVNumber(); //OK should be PCS member variable
  //----------------------------------------------------------------------
  // BC
  for(i=0;i<DOF;i++)
  {
    BCGroupDelete(pcs_type_name,pcs_primary_function_name[i]);
  }
  //......................................................................
  for(i=0;i<(int)bc_node_value.size(); i++)
  {
     delete bc_node_value[i];
     bc_node_value[i] = NULL;    
  }
  bc_node_value.clear();
  //----------------------------------------------------------------------
  // ST
  for(i=0;i<DOF;i++)
  {
    STGroupDelete(pcs_type_name,pcs_primary_function_name[i]);
  }
  //......................................................................
  CNodeValue* m_nod_val = NULL;
  for(i=0;i<(int)st_node_value.size();i++)
  {
    m_nod_val = st_node_value[i];
    //OK delete st_node_value[i];
    //OK st_node_value[i] = NULL;
    if(m_nod_val->check_me) //OK
    {
      m_nod_val->check_me = false;
      delete m_nod_val;
      m_nod_val = NULL;
    }
  }
  st_node_value.clear();
  //----------------------------------------------------------------------
  // NOD values
  nod_val_name_vector.clear();
  for(i=0;i<(int)nod_val_vector.size();i++)
  {
    delete nod_val_vector[i];
    nod_val_vector[i] = NULL;
  }
  nod_val_vector.clear();
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::ELERelationsDelete()
{
  long i;
  //----------------------------------------------------------------------
  // FEM element
  if(fem) delete fem; //WW
  fem = NULL;
  //----------------------------------------------------------------------
  // ELE matrices
  ElementMatrix *eleMatrix = NULL;
  ElementValue* gp_ele = NULL;
  if(Ele_Matrices.size()>0)
  {
    for (i=0;i<(long)Ele_Matrices.size();i++)
    {
      eleMatrix = Ele_Matrices[i];
      delete eleMatrix;
      eleMatrix = NULL;
    }
    Ele_Matrices.clear();
  }
  //----------------------------------------------------------------------
  // ELE - GP values
  if(ele_gp_value.size()>0)
  {
    for(i=0;i<(long)ele_gp_value.size();i++)
    {
      gp_ele = ele_gp_value[i];
      delete gp_ele;
      gp_ele = NULL;
    }
    ele_gp_value.clear();
  }
  //----------------------------------------------------------------------
  // ELE values
  ele_val_name_vector.clear();
  for(i=0;i<(long)ele_val_vector.size();i++)
  {
    delete ele_val_vector[i];
    //delete[] ele_val_vector[i];
    ele_val_vector[i] = NULL;
  }
  ele_val_vector.clear();
  //----------------------------------------------------------------------
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::OBJRelationsDelete()
{
  //----------------------------------------------------------------------------
  cout << "OBJ->PCS relations delete" << '\n';
  //----------------------------------------------------------------------------
  m_num = NULL;
  Tim =  NULL;
  m_msh = NULL;
  //----------------------------------------------------------------------------
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::Delete()
{
  //----------------------------------------------------------------------------
  cout << "PCS  delete" << '\n';
  //----------------------------------------------------------------------------
  ELERelationsDelete();
  NODRelationsDelete();
  EQSDelete();
  OBJRelationsDelete();
  //MMP2PCSRelation(this);
  //ConfigureCouplingForLocalAssemblier();
  //----------------------------------------------------------------------------
}

/**************************************************************************
PCSLib-Method:
07/2007 OK Implementation
**************************************************************************/
void CRFProcess::EQSDelete()
{
  LINEAR_SOLVER *eqs = NULL;
  //----------------------------------------------------------------------
  for(int i=0;i<(int)PCS_Solver.size();i++)
  {
    eqs = PCS_Solver[i]; 
    if(pcs_type_name.compare(eqs->pcs_type_name)==0)
    {
      if(eqs->unknown_vector_indeces)
         eqs->unknown_vector_indeces = \
          (int*) Free(eqs->unknown_vector_indeces);
      if(eqs->unknown_node_numbers)
        eqs->unknown_node_numbers = \
          (long*) Free(eqs->unknown_node_numbers);
      if(eqs->unknown_update_methods)
         eqs->unknown_update_methods = \
          (int*) Free(eqs->unknown_update_methods); 
      eqs = DestroyLinearSolver(eqs);
      eqs = NULL;
    }
  }
  //----------------------------------------------------------------------
  for(int i=0;i<(int)PCS_Solver.size();i++)
  {
    eqs = PCS_Solver[i]; 
    if(pcs_type_name.compare(eqs->pcs_type_name)==0)
      PCS_Solver.erase((PCS_Solver.begin()+i));
  }
}

/*
void CRFProcess::CreateYD()
{
  //----------------------------------------------------------------------------
  // MMP - create mmp groups for each process   //YD
  cout << "->Create MMP" << '\n';
  CMediumPropertiesGroup *m_mmp_group = NULL;
  for(i=0;i<DOF;i++)
  {
    m_mmp_group = MMPGetGroup(pcs_type_name);
    if(!m_mmp_group) {
      m_mmp_group = new CMediumPropertiesGroup();
      m_mmp_group->pcs_type_name = pcs_type_name; 
      m_mmp_group->Set(this);
      mmp_group_list.push_back(m_mmp_group);
    }
  }

  if(pcs_type_name.find("RICHARD")!=string::npos)    //YD
    continuum_ic = true;
  int time_level = 0;
  CalcSecondaryVariables(time_level);
  time_level = 1;
  CalcSecondaryVariables(time_level);
  if(pcs_type_name.find("RICHARD")!=string::npos)    //YD
      continuum_ic = false;
  if(compute_domain_face_normal) //WW
     m_msh->FaceNormal();        
}
*/

/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:
Programming: 
09/2007 WW Implementation
**************************************************************************/

// New solvers WW
#ifdef NEW_EQS    //1.09.2007 WW

void CreateEQS_LinearSolver()
{ 
  int i, dof_nonDM=1;
  int dof_DM=1;
  CRFProcess *m_pcs = NULL;
  CFEMesh *a_msh = NULL;
  SparseTable *sp = NULL;
  SparseTable *spH = NULL;
  Linear_EQS *eqs = NULL;
  Linear_EQS *eqsH = NULL;
  int dof = 1;
  //
  for(i=0;i<(int)pcs_vector.size();i++)
  {
    m_pcs = pcs_vector[i];
    if(m_pcs->type==22) // Monolithic TH2
    {
      dof_nonDM =  m_pcs->GetPrimaryVNumber();
      dof = dof_nonDM;
    }  
    if(m_pcs->type==4||m_pcs->type==41)  // Deformation
    {
      dof_DM = m_pcs->GetPrimaryVNumber(); 
      dof = dof_DM;  
    }
    else // Monolithic scheme for the process with linear elements
    {
      if(dof_nonDM < m_pcs->GetPrimaryVNumber())
        dof_nonDM = m_pcs->GetPrimaryVNumber();
    }          
  }
  //
  for(i=0; i<(int)fem_msh_vector.size(); i++)
  {
    a_msh = fem_msh_vector[i];
#if defined(USE_MPI) 
    eqs = new Linear_EQS(a_msh->GetNodesNumber(true)*dof); 
    EQS_Vector.push_back(eqs);
    EQS_Vector.push_back(eqs);
#else  
    a_msh->CreateSparseTable();
    // sparse pattern with linear elements exists
    sp = a_msh->GetSparseTable();
    // sparse pattern with quadratic elements exists
    spH = a_msh->GetSparseTable(true);
    //
    if(sp) 
      eqs = new Linear_EQS(*sp, dof_nonDM);
    if(spH) 
      eqsH = new Linear_EQS(*spH, dof_DM);
    EQS_Vector.push_back(eqs);
    EQS_Vector.push_back(eqsH);
#endif
  }
} 
#endif

/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:
Programming: 
09/2007 WW Implementation
**************************************************************************/
#include <iomanip>
void CRFProcess::DumpEqs(string file_name)
{
   int ii;
   long i, j, k, m;
   fstream eqs_out;
   CNode *node;
   eqs_out.open(file_name.c_str(), ios::out );
   eqs_out.setf(ios::scientific, ios::floatfield);
   setw(14);
   eqs_out.precision(14);
   // 
   long nnode = eqs->dim/eqs->unknown_vector_dimension; 
   for(i=0; i<eqs->dim; i++)
   {
      m = i%nnode;
      node = m_msh->nod_vector[m];
      for(ii=0; ii<eqs->unknown_vector_dimension; ii++)
      { 
        for(j=0; j<(long)node->connected_nodes.size(); j++)
        {
           k = node->connected_nodes[j];
           k = ii*nnode+k;
           if(k>=eqs->dim)	 continue;
           eqs_out<<i<<"  "<<k<<"  "<<MXGet(i,k)<<endl;
        } 
      }    
      eqs_out<<	i<<"  "<<	eqs->b[i]<<"  "<<eqs->x[i]<<endl;
   }
   eqs_out.close();
}



/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:
Programming: 
01/2008 WW Implementation
**************************************************************************/
void CRFProcess::WriteBC()
{
  long i = 0;
  long size_bc = (long)bc_node_value.size();
  long size_st = (long)st_node_value.size();
  //
  if(size_bc==0&&size_st==0)
    return; 
  //
  string m_file_name = FileName +"_"+pcs_type_name+"_BC_ST.asc";
  ofstream os(m_file_name.c_str(), ios::trunc|ios::out);     	
  if (!os.good())
  {
    cout << "Failure to open file: "<<m_file_name << endl;
    abort();
  }
  os.setf(ios::scientific,ios::floatfield);
  os.precision(12);
  CNode *anode = NULL;
  long nindex = 0;
  if(size_bc>0)
  {
     os<<"#Dirchelet BC  (from "<<m_file_name<<".bc file) "<<endl;
     os<<"#Total BC nodes  " <<size_bc<<endl;
     os<<"#Node index, name, x, y, z,   value: "<<endl;
     for(i=0; i<size_bc; i++)
     {
        nindex = bc_node_value[i]->geo_node_number;
        anode = m_msh->nod_vector[nindex];
        os<<nindex<<"  "
          <<bc_node_value[i]->pcs_pv_name<<" "
          <<setw(14)<<anode->X()<<" "
          <<setw(14)<<anode->Y()<<" "
          <<setw(14)<<anode->Z()<<" "
          <<setw(14)<<bc_node_value[i]->node_value<<endl;
     }
  } 
  if(size_st>0)
  {
     os<<"#Source term or Neumann BC  (from "<<m_file_name<<".st file) "<<endl;
     os<<"#Total ST nodes  " <<size_st<<endl;
     os<<"#Node index, x, y, z, name    value: "<<endl;
     for(i=0; i<size_st; i++)
     {
        nindex = st_node_value[i]->geo_node_number;
        anode = m_msh->nod_vector[nindex];
        os<<nindex<<"  "
          <<st_node[i]->pcs_pv_name<<" "
          <<setw(14)<<anode->X()<<" "
          <<setw(14)<<anode->Y()<<" "
          <<setw(14)<<anode->Z()<<" "
          <<setw(14)<<st_node_value[i]->node_value<<endl;
     }
  } 
  os<<"#STOP"<<endl; 
  os.close();
}


#ifdef NEW_EQS //WW
/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:  //For fluid momentum, WW 
Programming: 
01/2008 WW Implementation
**************************************************************************/
void CRFProcess::EQSInitialize()
 {eqs_new->Initialize();} 

/*************************************************************************
ROCKFLOW - Function: CRFProcess::
Task:  //For fluid momentum, 
Programming: 
02/2008 PCH Implementation
**************************************************************************/
void CRFProcess::EQSSolver(double* x)
 {
	 eqs_new->Solver();

	 for(int i=0; i < m_msh->nod_vector.size(); ++i)
		 x[i] = eqs_new->X(i);
 } 
#endif

#ifdef GEM_REACT
void CRFProcess::IncorporateSourceTerms_GEMS(void)
{
    // Initialization
    long it;       // iterator
    long N_Nodes;  // Number of Nodes
    int nDC;       // Number of mass transport components.
    int i =0 ;         // index of the component

    // Get a vector pointing to the REACT_GEM class
    if (m_vec_GEM)
    {
        // Get needed informations.----------------------------
        // Number of Nodes
        N_Nodes = (long) m_msh->GetNodesNumber(false);
        // Number of DC
        nDC = m_vec_GEM->nDC;
        // Identify which PCS it is and its sequence in mcp.
        i = this->pcs_component_number;
        // ----------------------------------------------------

        // Loop over all the nodes-----------------------------
        for ( it=0 ; it <  N_Nodes/*Number of Nodes*/; it++ )
        {
            // Adding the rate of concentration change to the right hand side of the equation.
            eqs->b[it] -= m_vec_GEM->m_xDC_Chem_delta[it*nDC+i] / dt ; 
        }
        // ----------------------------------------------------
    }
}
#endif


/*=======================================================================
//Class Problem: Handle the data and their functions for time stepping 

                 and coupled processes within each time step, and finally 
                 solve a problem. 
Design and implementation:  WW
Start time:  09.07.2008 
Modification:
            12.2008 WW Incorporate the changes from previous versions. 
========================================================================*/
#include "Configure.h"

#include "problem.h"
#if defined(PROBLEM_CLASS)

#if defined(USE_MPI_REGSOIL)
#include <mpi.h>
#include "par_ddc.h"
#endif
#include <iostream>
#include <sstream>
//WW
//
/*------------------------------------------------------------------------*/
/* Pre-processor definitions */
#include "makros.h"
/*------------------------------------------------------------------------*/
// MSHLib
#include "msh_node.h"
#include "msh_lib.h"
/*------------------------------------------------------------------------*/
// Data file
extern int ReadData(char*); //OK411
/* PCS */
#include "pcs_dm.h"
#include "rf_pcs.h" 
//16.12.2008.WW #include "rf_apl.h"
#include "rf_react.h"
#include "par_ddc.h"
#include "rf_st_new.h"
#include "rf_tim_new.h"
#include "rfmat_cp.h"
//#include "rf_vel_new.h"
#include "rf_fluid_momentum.h"	
#include "rf_random_walk.h"     
// Finite element
#include "fem_ele_std.h"
#include "rf_bc_new.h"
#include "rf_out_new.h"
#include "tools.h"
#include "files0.h" // GetLineFromFile1
//
#ifdef CHEMAPP
  #include "./EQL/eqlink.h"
#endif
#ifdef UDE_REACT
#include "rf_REACT_ude.h"
#endif
#ifdef GEM_REACT
#include "rf_REACT_GEM.h"
#endif
#ifdef BRNS
// BRNS dll link; HB 02.11.2007
#include "rf_REACT_BRNS.h"
#endif
#include "rf_kinreact.h"

namespace process{class CRFProcessDeformation;}
using process::CRFProcessDeformation;
using namespace std;

//NW: moved the following variables from rf.cpp to avoid linker errors
string FileName; //WW
string FilePath; //23.02.2009. WW

/**************************************************************************
GeoSys - Function: Constructor
Task: 
Programing:
 07/2008 WW Set it as an constructor of class problem based on the 
            PreTimeloop
Modification:
***************************************************************************/
Problem::Problem(char* filename):print_result(false)
{
  int i;

  print_result = true; //OK
  if(filename!=NULL)
    {
      // Read data
      ReadData(filename);
      DOMRead(filename);
    }
#ifndef NEW_EQS
  ConfigSolverProperties();  //_new. 19.10.2008. WW
#endif
  for(i=0;i<(int)pcs_vector.size();i++)
  {
    hasAnyProcessDeactivatedSubdomains = (pcs_vector[i]->NumDeactivated_SubDomains > 0);
    if (hasAnyProcessDeactivatedSubdomains) break;
  }
  //----------------------------------------------------------------------
  // Create ST
  //OK STCreateFromPNT();
  //----------------------------------------------------------------------
  GetHeterogeneousFields(); //OK/MB
  //----------------------------------------------------------------------
  // Test MSH-MMP //OK
  int g_max_mmp_groups = MSHSetMaxMMPGroups();
  if(g_max_mmp_groups>(int)mmp_vector.size()){
    cout << "Error: not enough MMP data";
    print_result = false; //OK
    return;
  }
  //----------------------------------------------------------------------
  // Create PCS processes
  PCSCreate();
  if (!PCSCheck()) //OK4910 reactivated
  {  
    print_result = false; //OK
    return;
  }
  //......................................................................
//#ifdef RESET_4410
//  //if(pcs_vector[0]->pcs_type_name.compare("TWO_PHASE_FLOW")==0) //OK
//  if(total_processes[3])  // 3: TWO_PHASE_FLOW. 12.12.2008. WW     
//    PCSCalcSecondaryVariables(); //OK
//#endif
  //......................................................................
  //09.07.2008 WW
  SetActiveProcesses(); 
  //OK if (!Check()) return; //OK
  //----------------------------------------------------------------------
  // REACTIONS 
  //CB before the first time step
  //if(MASS_TRANSPORT_Process) // if(MASS_TRANSPORT_Process&&NAPL_Dissolution) //CB Todo
  if(transport_processes.size()>0) //12.12.2008. WW   
  {
    // 1) set the id variable flow_pcs_type for Saturation and velocity calculation
    //    in mass transport element matrices
    SetFlowProcessType();
    // 2) in case of Twophaseflow calculate NAPL- and the corresponding 
    //    Water phase Saturation from the NAPL concentrations
    //if(pcs_vector[0]->pcs_type_name.compare("TWO_PHASE_FLOW")==0) //OK
    //WW CRFProcess *m_pcs = NULL;
    //WW if (m_pcs = PCSGet("TWO_PHASE_FLOW"))     
    // CB: this fct will set the initial NAPL Density in case of NAPL-Dissolution
    //     should this fct be executed in all cases?? --> CHP
    if(total_processes[3])  // 3: TWO_PHASE_FLOW. 12.12.2008. WW     
	     PCSCalcSecondaryVariables(); 
  }
  //----------------------------------------------------------------------
  // REACTIONS 
  // Initialization of REACT structure for rate exchange between MTM2 and Reactions
  
  
  //-------------------------------------------------- 
  // HB, for the GEM chemical reaction engine 05.2007
  //--------------------------------------------------
#ifdef GEM_REACT
  m_vec_GEM = new REACT_GEM(); 
  GEMRead( FileName , m_vec_GEM );
  
  string path = "";  // to get the path of the file;
  path = FileName;   // first get full path and project name;
  int pos, npos;
  pos = 0;
  npos = (int)path.size();
  
  // Get path
#ifdef _WIN32
  pos = (int)path.rfind("\\");// HS keep this on windows
#else
  pos = (int)path.rfind("/"); // HS keep this on linux
#endif // _WIN32
  if( pos < npos )
    path = path.substr(0, pos+1);
  
  // now start initialization of GEMS
  if ( m_vec_GEM->Init_Nodes(/*FileName*/ path ) == 0) 
    {
      
      if (m_vec_GEM->Init_RUN() == 0) m_vec_GEM->initialized_flag = 1;
    }
  
#else // GEM_REACT
  //---------------------------------------------------
  REACT *rc = NULL; //SB
  //  rc->TestPHREEQC(); // Test if *.pqc file is present
  rc = rc->GetREACT();
  if(rc) //OK
    {  
      if(rc->flag_pqc){
	if(cp_vec.size()>0)
	  { //OK
#ifdef REACTION_ELEMENT
	    rc->CreateREACT();//SB
	    rc->InitREACT0();
	    rc->ExecuteReactionsPHREEQC0();
	    REACT_vec.clear();
	    REACT_vec.push_back(rc);        
#else
	    
	    rc->CreateREACT();//SB
	    rc->InitREACT();
	    //SB4501        rc->ExecuteReactions();
	    
#ifdef LIBPHREEQC  // MDL: new functions with built-in phreeqc
	    rc->ExecuteReactionsPHREEQCNewLib();
#else
	    rc->ExecuteReactionsPHREEQCNew();
#endif //LIBPHREEQC
	    REACT_vec.clear();
	    REACT_vec.push_back(rc);
#endif // REACTION_ELEMENT
	  }
      }
      //  delete rc;
    }
#endif // GEM_REACT
  
#ifdef BRNS
  // Here to test BRNS; HB 02.11.2007
  // REACT_BRNS* pBRNS;
  // pBRNS = new REACT_BRNS();
  m_vec_BRNS = new REACT_BRNS();
  m_vec_BRNS->InitBRNS();
#endif
  
  
#ifdef CHEMAPP
  CEqlink *eq=NULL;
  eq = eq->GetREACTION();
  if(cp_vec.size()>0  && eq){ //MX
    eq->TestCHEMAPPParameterFile(pcs_vector[0]->file_name_base); 
    if (eq->flag_chemapp){
      eq->callCHEMAPP(pcs_vector[0]->file_name_base);
    }
  }
#endif
  //  delete rc;
  
  //----------------------------------------------------------------------
  // DDC
  int no_processes =(int)pcs_vector.size();
  CRFProcess* m_pcs = NULL;
  if(dom_vector.size()>0)
    {
      
      // -----------------------
      DOMCreate();
      //
      for(i=0;i<no_processes;i++)
      {
         m_pcs = pcs_vector[i];
         m_pcs->CheckMarkedElement();
         CountDoms2Nodes(m_pcs);
         // Config boundary conditions for domain decomposition 
         m_pcs->SetBoundaryConditionSubDomain(); //WW
      }
      //
      node_connected_doms.clear();
      // Release some memory. WW
#if defined(USE_MPI) //TEST_MPI WW
      // Release memory of other domains. WW
      for(i=0;i<(int)dom_vector.size();i++) 
	{
	  if(i!=myrank)
	    {
	      // If shared memory, skip the following line 
#if defined(NEW_BREDUCE2)
	      dom_vector[i]->ReleaseMemory();
#else
	      // If MPI__Allreduce is used for all data conlection, activate following
	      delete dom_vector[i];
	      dom_vector[i] = NULL;
#endif
	    }
	}
#endif
      
    }
  //----------------------------------------------------------------------
  PCSRestart(); //SB
  if(transport_processes.size()>0) //WW. 12.12.2008
    {
      //----------------------------------------------------------------------
      KRConfig();
      //----------------------------------------------------------------------
      // Configure Data for Blobs (=>NAPL dissolution) 
      KBlobConfig();
      KBlobCheck();
      //WW CreateClockTime();
    }
	OUTCheck(); // new SB
  //========================================================================
  // Controls for coupling. WW
  loop_index = 0;
  max_coupling_iterations = 1;
  coupling_tolerance = 1.e-4; 
  //========================================================================
  // WW
  char line[MAX_ZEILE];
  string line_string;
  ios::pos_type position;
  std::stringstream in_num;
  // File handling
  string num_file_name = FileName + NUM_FILE_EXTENSION;
  ifstream num_file (num_file_name.data(),ios::in);
  if (num_file.good()) 
    {
      num_file.seekg(0L,ios::beg);
      while (!num_file.eof())
	{
	  num_file.getline(line,MAX_ZEILE);
	  line_string = line;
	  if(line_string.find("#STOP")!=string::npos)
	    break;
	  if(line_string.find("$OVERALL_COUPLING")!=string::npos) 
	    {
	      in_num.str(GetLineFromFile1(&num_file));
	      in_num>>max_coupling_iterations >> coupling_tolerance;
	      break;
	    }  
	} 
      num_file.close();
    } 
  //========================================================================
  // For time stepping. WW
  CTimeDiscretization *m_tim = NULL;
  start_time = 1.e+8;
  end_time = 0.;
  max_time_steps = 0;
  bool time_ctr = false;
  // Determine the start and end times from all available process related data.
  for(i=0; i<(int)time_vector.size(); i++)
    {
      m_tim = time_vector[i];
      m_tim->FillCriticalTime();
      if(m_tim->time_start<start_time) 
	start_time = m_tim->time_start;
      if(m_tim->time_end>end_time)
	end_time = m_tim->time_end; 
      if(max_time_steps<(int)m_tim->time_step_vector.size())
	max_time_steps = (int)m_tim->time_step_vector.size(); 
      if(m_tim->GetTimeStepCrtlType()>0)
	time_ctr = true;  
    }
  if(max_time_steps==0) max_time_steps = 1000000;
  current_time =  start_time; 
  if(time_ctr)
    {
      int maxi_dof = 0;
      int maxi_nnodes = 0;
      for(i=0; i<no_processes; i++)
	{
	  m_pcs = pcs_vector[i];
	  if(m_pcs->GetPrimaryVNumber()>maxi_dof)
	    maxi_dof = m_pcs->GetPrimaryVNumber();
	  if(m_pcs->m_msh->GetNodesNumber(false)>maxi_nnodes)
	    maxi_nnodes = m_pcs->m_msh->GetNodesNumber(false);
	}
      buffer_array = new double[maxi_dof*maxi_nnodes]; 
    }
  else
    buffer_array = NULL;  
  //========================================================================  
  CRFProcessDeformation *dm_pcs = NULL;

  //  //WW
  for(i=0; i<no_processes; i++)
    {
      m_pcs = pcs_vector[i];
      m_pcs->CalcSecondaryVariables(true); //WW
      m_pcs->Extropolation_MatValue();  //WW
    }
  // Calculation of the initial stress and released load for excavation simulation
  // 07.09.2007  WW
  // Excavation for defromation
  dm_pcs = (CRFProcessDeformation *)total_processes[12];
  if(dm_pcs)
    dm_pcs->CreateInitialState4Excavation();      
  
}
/**************************************************************************
GeoSys - Function: Desstructor
Task: 
Programing:
 08/2008 WW Set it as an constructor of class problem based on the 
            PreTimeloop
 
Modification:
 12.2008  WW  
***************************************************************************/
Problem::~Problem()
{
  delete [] active_processes;
  delete [] exe_flag;
  if(buffer_array) delete [] buffer_array;
  buffer_array = NULL;
  active_processes = NULL; 
  exe_flag = NULL;
  //
  PCSDestroyAllProcesses();
  //
  if(GetRFProcessProcessingAndActivation("MT") && GetRFProcessNumComponents()>0) {
    DestroyREACT(); //SB
    cp_vec.clear(); // Destroy component properties vector
  }
  //
#ifdef CHEMAPP
  if (Eqlink_vec.size()>0){ 
    Eqlink_vec[0]->DestroyMemory();
    Eqlink_vec.clear();
  }
#endif
 //WW ClockTimeVec[0]->PrintTimes();
#ifdef GEM_REACT
  // HS:
  delete m_vec_GEM; 
#endif

    #ifdef BRNS
    // Here to delete BRNS instance; HB 12.11.2007
    // delete m_vec_BRNS.at(0);
    delete m_vec_BRNS;
    #endif
  cout<<"\n^O^: Your simulation is terminated normally ^O^ "<<endl;
}
/*-------------------------------------------------------------------------
GeoSys - Function: SetActiveProcesses
Task:   
   total_processes:
    0: LIQUID_FLOW     | 1: GROUNDWATER_FLOW  | 2: RICHARDS_FLOW
    3: PS_GLOBAL   | 4: MULTI_PHASE_FLOW  | 5: COMPONENTAL_FLOW
    6: OVERLAND_FLOW   | 7: AIR_FLOW          | 8: HEAT_TRANSPORT
    9: FLUID_MOMENTUM  |10: RANDOM_WALK       |11: MASS_TRANSPORT
   12: DEFORMATION     |
Return: 
Programming: 
07/2008 WW 
03/2009 PCH added PS_GLOBAL
Modification:
-------------------------------------------------------------------------*/
inline int Problem::AssignProcessIndex(CRFProcess *m_pcs,  bool activefunc)
{  
  if(m_pcs->pcs_type_name.compare("LIQUID_FLOW")==0) 
  {
    if(!activefunc) return 0;
    total_processes[0] = m_pcs;
    active_processes[0] =  &Problem::LiquidFlow;
    return 0;
  }         
  else if(m_pcs->pcs_type_name.compare("GROUNDWATER_FLOW")==0) 
  {
    if(!activefunc) return 1;
    total_processes[1] = m_pcs;
    active_processes[1] =  &Problem::GroundWaterFlow;
    return 1;
  }         
  else if(m_pcs->pcs_type_name.compare("RICHARDS_FLOW")==0) 
  {
    if(!activefunc) return 2;
    total_processes[2] = m_pcs;
    active_processes[2] =  &Problem::RichardsFlow;
    return 2;
  }         
  else if(m_pcs->pcs_type_name.compare("TWO_PHASE_FLOW")==0) 
  {
    if(!activefunc) return 3;
    total_processes[3] = m_pcs;
    active_processes[3] =  &Problem::TwoPhaseFlow;
    return 3;
  }         
  else if(m_pcs->pcs_type_name.compare("MULTI_PHASE_FLOW")==0) 
  {
    if(!activefunc) return 4;
    total_processes[4] = m_pcs;
    active_processes[4] =  &Problem::MultiPhaseFlow;
    return 4;
  }         
  else if(m_pcs->pcs_type_name.compare("COMPONENTAL_FLOW")==0) 
  {
    if(!activefunc) return 5;
    total_processes[5] = m_pcs;
    active_processes[5] =  &Problem::ComponentalFlow;
    return 5;
  }         
  else if(m_pcs->pcs_type_name.compare("OVERLAND_FLOW")==0) 
  {
    if(!activefunc) return 6;
    total_processes[6] = m_pcs;
    active_processes[6] = &Problem::OverlandFlow;
    return 6;
  }         
  else if(m_pcs->pcs_type_name.compare("AIR_FLOW")==0) 
  {
    if(!activefunc) return 7;
    total_processes[7] = m_pcs;
    active_processes[7] = &Problem::AirFlow;
    return 7;
  }         
  else if(m_pcs->pcs_type_name.compare("HEAT_TRANSPORT")==0) 
  {
    if(!activefunc) return 8;
    total_processes[8] = m_pcs;
    active_processes[8] = &Problem::HeatTransport;
    return 8;
  }         
  else if(m_pcs->pcs_type_name.compare("FLUID_MOMENTUM")==0) 
  {
    if(!activefunc) return 9;
    total_processes[9] = m_pcs;
    active_processes[9] = &Problem::FluidMomentum;
    return 9;
  }         
  else if(m_pcs->pcs_type_name.compare("RANDOM_WALK")==0) 
  {
    if(!activefunc) return 10;
    total_processes[10] = m_pcs;
    active_processes[10] = &Problem::RandomWalker;
    return 10;
  }         
  else if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0) 
  {
    if(!activefunc) return 11;
    total_processes[11] = m_pcs;
    active_processes[11] = &Problem::MassTrasport;
    return 11;
  }         
  else if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos) 
  {
    if(!activefunc) return 12;
    total_processes[12] = m_pcs;
    active_processes[12] = &Problem::Deformation;
    return 12;
  }  
	else if(m_pcs->pcs_type_name.find("PS_GLOBAL")!=string::npos)
  {
//    if(!activefunc) return 13;
    if(!activefunc) return 3;
    total_processes[3] = m_pcs;
    active_processes[3] = &Problem::PS_Global;
    return 3;
  }
  cout<<"Error: no process is specified. "<<endl; 
  return -1;         
}
/*-------------------------------------------------------------------------
GeoSys - Function: SetActiveProcesses
Task:   
   total_processes:
    0: LIQUID_FLOW     | 1: GROUNDWATER_FLOW  | 2: RICHARDS_FLOW
    3: TWO_PHASE_FLOW  | 4: MULTI_PHASE_FLOW  | 5: COMPONENTAL_FLOW
    6: OVERLAND_FLOW   | 7: AIR_FLOW          | 8: HEAT_TRANSPORT
    9: FLUID_MOMENTUM  |10: RANDOM_WALK       |11: MASS_TRANSPORT
   12: DEFORMATION     |13: PS_GLOBAL         |
Return: 
Programming: 
07/2008 WW 
03/2009 PCH add PS_GLOBAL
Modification:
--------------------------------------------------------------------*/
void Problem::SetActiveProcesses()
{
  int i;
  CRFProcess* m_pcs = NULL;
  const int max_processes = 14;	// PCH
  total_processes.resize(max_processes);
  active_processes = new ProblemMemFn[max_processes];
  coupled_process_index.resize(max_processes);
  exe_flag = new bool[max_processes];
  //
  for(i=0; i<max_processes; i++)
  {
    total_processes[i] = NULL;
    active_processes[i] = NULL;
    coupled_process_index[i] = -1;
  }
  //
  for(i=0; i<(int)pcs_vector.size(); i++)
  {
     m_pcs = pcs_vector[i];
     AssignProcessIndex(m_pcs);
  } 
  //
  for(i=0; i<max_processes; i++)
  {
    if(total_processes[i]) 
    {
      m_pcs = PCSGet(total_processes[i]->m_num->cpl_variable); 
      if(m_pcs)
        coupled_process_index[i] = AssignProcessIndex(m_pcs, false); 
      active_process_index.push_back(i);
    }
  }
  // Transport  porcesses
  for(i=0;i<(int)pcs_vector.size();i++)
  {
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
      transport_processes.push_back(m_pcs);
    if(m_pcs->pcs_type_name.compare("TWO_PHASE_FLOW")==0)  //09.01.2008. WW
      multiphase_processes.push_back(m_pcs);   
  }
 
}

/**************************************************************************     <
ROCKFLOW - Function: PCSCreate
Task: 
Programing:
 02/2003 OK Implementation
 03/2003 OK H processes
 04/2003 OK C processes
 05/2003 OK T processes
 05/2003 OK TH processes
 08/2004 OK PCS2
 08/2004 WW The new creation of the deformation process
 10/2004 OK H gas processes
 01/2005 WW New element calculation
 01/2005 OK H unsatutated process
 02/2005 MB switch case in config()
 06/2005 OK MMP2PCSRelation
 07/2008 WW Capsulated into class Problem
Modification:
***************************************************************************/
void Problem::PCSCreate()
{
  //----------------------------------------------------------------------
  cout << "---------------------------------------------" << endl;
  cout << "Create PCS processes" << endl;
  //----------------------------------------------------------------------
  int i;
  int no_processes =(int)pcs_vector.size();
  CRFProcess* m_pcs = NULL;
  //
  //----------------------------------------------------------------------
  //OK_MOD if(pcs_deformation>0) Init_Linear_Elements();
  for(i=0;i<no_processes;i++)
  {
    m_pcs = pcs_vector[i];
    m_pcs->pcs_type_number = i;
    m_pcs->Config(); //OK
  }
  //
#ifdef NEW_EQS
  CreateEQS_LinearSolver(); //WW 
#endif
  //
  for(i=0;i<no_processes;i++)
  {
    cout << "............................................." << endl;
    m_pcs = pcs_vector[i];
    cout << "Create: " << m_pcs->pcs_type_name << endl;
	if(!m_pcs->pcs_type_name.compare("MASS_TRANSPORT")){
		cout << " for " << m_pcs->pcs_primary_function_name[0] << " ";
	    cout << " pcs_component_number " << m_pcs->pcs_component_number;       
	}
    cout << endl;
    m_pcs->Create();
  }
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    MMP2PCSRelation(m_pcs);
  }
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++)
  { //WW
    m_pcs = pcs_vector[i];
    m_pcs->ConfigureCouplingForLocalAssemblier();
  }
}

/*-------------------------------------------------------------------------
ROCKFLOW - Function: PCSRestart
Task: Insert process to list
Programming: 
06/2003 OK Implementation
07/2008 WW Capsulated into class Problem
Modification:
-------------------------------------------------------------------------*/
void Problem::PCSRestart()
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
//OK411      CopyNodeVals(nidx1,nidx0);
    }
  }
}
/**************************************************************************
FEMLib-Method: 
07/2008 WW Implementation
01/2009 WW Update
**************************************************************************/
void Problem::Euler_TimeDiscretize()
{
  long accepted_times = 0; 
  long rejected_times = 0; 
  //
  CTimeDiscretization *m_tim = NULL;
  aktueller_zeitschritt = 0; 
#if defined(USE_MPI)  
  if(myrank==0)
  { 
#endif
  cout<<"\n\n***Start time steps\n";
  // Dump the initial conditions.
  OUTData(0.0,aktueller_zeitschritt);
#if defined(USE_MPI)  
  }
#endif
  // 
  while(end_time>current_time)
  {
    //
    // Minimum time step.
    dt = 1.0e20;
    for(int i=0; i<(int)active_process_index.size(); i++)   //09.01.2009. WW   
    {
       m_tim = total_processes[active_process_index[i]]->Tim;
       dt0 = m_tim->CalcTimeStep(current_time);
       if(dt0<dt)
          dt = dt0;  
    }   
    if(dt<DBL_EPSILON)
    {
       cout<<"!!! Too small time step size. Quit the simulation now."<<endl;
       exit(1);
    }  
    // 
    aktueller_zeitschritt++;  // Might be removed late on
    current_time += dt;
    aktuelle_zeit = current_time ;
    // Print messsage
#if defined(USE_MPI)  
  if(myrank==0)
  {
#endif 
    cout<<"\n\n#############################################################";
    cout<<"\nTime step: "<<aktueller_zeitschritt<<"|  Time: "<<current_time<<"|  Time step size: "<<dt<<endl;
#if defined(USE_MPI)  
  }
#endif 
    if(CouplingLoop())
    {
#if defined(USE_MPI)  
  if(myrank==0)
#endif 
      cout<<"This step is accepted." <<endl;
      PostCouplingLoop();
      if(print_result)			
#if defined(USE_MPI) 
      {
       if(myrank==0) 
#endif
        OUTData(current_time, aktueller_zeitschritt);
#if defined(USE_MPI) 
       // MPI_Barrier (MPI_COMM_WORLD); 
      }
#endif
      //
      accepted_times++; 
#ifdef MFC
 /*START: Update Visualization for OpenGL and other MFC view e.g. Diagram*/ 
 CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
 CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
 CGeoSysDoc* m_pDoc = (CGeoSysDoc *)pChild->GetActiveDocument();
 CGeoSysOUTProfileView *pView = (CGeoSysOUTProfileView *) pChild->GetActiveView();
 POSITION pos = m_pDoc->GetFirstViewPosition();
  while(pos!=NULL) {
    CView* pView = m_pDoc->GetNextView(pos);
    pView->UpdateWindow();
  }
 CMainFrame* mainframe = (CMainFrame*)AfxGetMainWnd();
 CRFProcess* m_pcs = NULL;
 if(pcs_vector.size()==0)
 return;
 m_pcs = PCSGet((string)mainframe->m_pcs_name);
 if(!m_pcs)
 {
   m_pcs = pcs_vector[0];
   //OK AfxMessageBox("Problem::Euler_TimeDiscretize() - no PCS data");
   //OK return;
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
 m_pDoc->SetModifiedFlag(1);
 m_pDoc->UpdateAllViews(NULL,0L,NULL);
 /*END: Update Visualization for OpenGL and other MFC view e.g. Diagram*/ 
#endif       
    }
    else
    { 
      current_time -= dt;
      // aktuelle_zeit might be removed late on
      aktuelle_zeit = current_time;
      aktueller_zeitschritt--;  // Might be removed late on
      //
      for(int i=0; i<(int)pcs_vector.size(); i++)      
      {
         if(pcs_vector[i]->pcs_type_name.find("DEFORMATION")!=string::npos)
           continue;
         pcs_vector[i]->CopyTimestepNODValues(false);
      }     
      rejected_times++; 
#if defined(USE_MPI)  
  if(myrank==0)
#endif 
      cout<<"This step is rejected." <<endl;
    } 
#if defined(USE_MPI)  
  if(myrank==0)
#endif 
    cout<<"\n#############################################################\n";
    if(aktueller_zeitschritt>=max_time_steps)
      break;
  } 

  cout<<"----------------------------------------------------\n";
  cout<<"|Acccept step times |"<<accepted_times;
  cout<<"  |Reject step times |"<<rejected_times<<endl;
  cout<<"----------------------------------------------------\n";
  //           
}

/*-----------------------------------------------------------------------
GeoSys - Function: Coupling loop
Task: 
Return: error
Programming: 
07/2008 WW 
Modification:
12.2008 WW Update 
-------------------------------------------------------------------------*/
bool Problem::CouplingLoop()
{
   int i, j, index, cpl_index;
   double error = 1.e8;
   double error_cpl = 1.e8;
   CRFProcess *m_pcs = NULL;
   CRFProcess *a_pcs = NULL;
   CTimeDiscretization *m_tim = NULL;
   //
   print_result = false;
   int acounter = 0;
   //  
   for(i=0; i<(int)pcs_vector.size(); i++) 
     pcs_vector[i]-> UpdateTransientBC(); 

   for(i=0; i<(int)total_processes.size(); i++)      	     
   {
      if(active_processes[i]&&total_processes[i]->selected) //CB 12/09 Timtypesteady
      {
        m_tim = total_processes[i]->Tim;
        if(m_tim->CheckTime(current_time, dt)>DBL_MIN)
        {
           exe_flag[i] = true;
           total_processes[i]->SetDefaultTimeStepAccepted();
           acounter++;
           m_tim->step_current++;
        } 
        else   
           exe_flag[i] = false;
      }
      else
        exe_flag[i] = false;
   }
   //
   int num_processes = (int)active_process_index.size();
// To do
//SB->WW I do not understand this condition, why switch off output?
//SB   if(acounter==num_processes)
      print_result = true;
   //
   bool accept = true;
   for(loop_index=0; loop_index<max_coupling_iterations; loop_index++)                    
   {
    for(i=0; i<num_processes; i++)
	  {
         index = active_process_index[i];
         //RealFunction aProcess = active_processes[index];
         cpl_index = coupled_process_index[index];
         if(exe_flag[index])
         {
           a_pcs = total_processes[index];
           error = Call_Member_FN(this, active_processes[index])(); //aProcess(); 
           if(!a_pcs->TimeStepAccept())
           {            
              accept = false;            
              break;
           }
           // If not accepted, m_tim->step_current++
           if(cpl_index>-1&&exe_flag[cpl_index])
           {
             m_pcs = total_processes[cpl_index];
             for(j=0; j<m_pcs->m_num->cpl_iterations; j++)
             {
               error_cpl =  Call_Member_FN(this, active_processes[cpl_index])(); //bProcess();  
               if(!m_pcs->TimeStepAccept())
               {            
                  accept = false;            
                  break;
               }               
               // If not accepted, m_tim->step_current++
               if(fabs(error_cpl-error)<m_pcs->m_num->cpl_tolerance)
                 break;
               error =  Call_Member_FN(this, active_processes[index])(); //aProcess(); 
               if(!a_pcs->TimeStepAccept())
               {            
                 accept = false;            
                 break;
               }
               // If not accepted, m_tim->step_current++
             }
             exe_flag[cpl_index] = false;    
           }  
           else
           {
			  if(a_pcs->type != 55){//SB Not for fluid momentum process
				if(fabs(error_cpl-error)<coupling_tolerance)
				  if(loop_index > 0) //SB4900 - never break in first coupling loop?
						break;
              error_cpl = error;
			   }
		   
		   } 
           // 
           if(!accept) break;         
         }        
      }
	  if(error_cpl<coupling_tolerance) break; // JOD/WW 4.10.01
      cout<<"Coupling loop: "<<loop_index+1<<" of "<<max_coupling_iterations<<endl;
      if(!accept) break;     
 /*index = active_process_index[1];
  cpl_index = coupled_process_index[index];
  if(exe_flag[index]) {
	  a_pcs = total_processes[index];
      error =  Call_Member_FN(this, active_processes[index])();
   } 
	  index = active_process_index[0];
	    cpl_index = coupled_process_index[index];
		if(exe_flag[index]){	
		a_pcs = total_processes[index];
      error =  Call_Member_FN(this, active_processes[index])();
}	 
	  index = active_process_index[2];
	    cpl_index = coupled_process_index[index];
		if(exe_flag[index]){
		a_pcs = total_processes[index];
      error =  Call_Member_FN(this, active_processes[index])();
}*/

   }
   // 
   return accept;
}

/*-----------------------------------------------------------------------
GeoSys - Function: post Coupling loop
Task: 
Return: error
Programming: 
08/2008 WW 
Modification:
-------------------------------------------------------------------------*/
void Problem::PostCouplingLoop()
{
  CRFProcess *m_pcs = NULL;
  if(total_processes[12])
  {  
    CRFProcessDeformation *dm_pcs = (CRFProcessDeformation *)(total_processes[12]); 
    if(H_Process&&dm_pcs->type!=41) // HM partitioned scheme
       dm_pcs->ResetTimeStep(); 
    dm_pcs->Extropolation_GaussValue();
  } 

  //CB new NAPL and Water Saturations after reactions for Two_Phase_Flow and NAPL-Dissolution
  //WW if(MASS_TRANSPORT_Process) // if(MASS_TRANSPORT_Process&&NAPL_Dissolution) //CB Todo
  if(transport_processes.size()>0&&total_processes[3]) // 12.2008. WW
    if (KNaplDissCheck())   // Check if NAPLdissolution is modeled
      CalcNewNAPLSat(total_processes[3]);
    
  /* CB 21/09 The next fct. was necessary in 4.08. Still needed here? I think so 
  // for TWO_PHASE_FLOW the new time step results for secondary variables 
  // PRESSURE2 and SATURATION1 are not copied below in the function 
  // CopyTimestepNODValues(); but I can do it here:
  if (m_pcs = PCSGet("TWO_PHASE_FLOW"))     
     CopyTimestepNODValuesSVTPhF();
  */      
      
  //  Update the results
  for(int i=0;i<(int)pcs_vector.size();i++)
  {
     m_pcs = pcs_vector[i];
     if (hasAnyProcessDeactivatedSubdomains) //NW
       m_pcs->CheckMarkedElement();
#if defined(USE_MPI) // 18.10.2007 WW
     if(myrank==0) {
#endif  
         m_pcs->WriteSolution(); //WW
#ifdef GEM_REACT
	if (i==0) { // for GEM_REACT we also need information on porosity (node porosity internally stored in Gems process)!....do it only once and it does not matter for which process ! ....we assume that the first pcs process is the flow process...if reload not defined for every process, restarting with gems will not work in any case 

	if (( m_pcs->reload==1 || m_pcs->reload==3 ) && !(( aktueller_zeitschritt % m_pcs->nwrite_restart  ) > 0) ) m_vec_GEM->WriteReloadGem();
	}
#endif
#if defined(USE_MPI) // 18.10.2007 WW
     }
#endif  

     m_pcs->Extropolation_MatValue();  //WW
     if(m_pcs->cal_integration_point_value) //WW
        m_pcs->Extropolation_GaussValue();
     m_pcs->CopyTimestepNODValues(); //MB
#define SWELLING
#ifdef SWELLING
     for(int j=0;j<m_pcs->pcs_number_of_evals;j++){  //MX ToDo//CMCD here is a bug in j=7
        int nidx0 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j]);
        int nidx1 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j])+1;
        for(long l=0;l<(long)m_pcs->m_msh->ele_vector.size();l++)
            m_pcs->SetElementValue(l,nidx0, m_pcs->GetElementValue(l,nidx1));
     }
#endif
  }
  // WW
#ifndef NEW_EQS //WW. 07.11.2008
  if(total_processes[1])  
    total_processes[1]->AssembleParabolicEquationRHSVector();
#endif
  LOPCalcELEResultants(); 
}
/*-------------------------------------------------------------------------
GeoSys - Function: LiquidFlow
Task: Similate liquid flow  
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::LiquidFlow()
{
  double error = 0.;
  CRFProcess *m_pcs = total_processes[0];
  if(!m_pcs->selected) return error; 
//  error = m_pcs->Execute();
  error = m_pcs->ExecuteNonLinear();
#ifdef RESET_4410
  PCSCalcSecondaryVariables(); // PCS member function
#endif
  m_pcs->CalIntegrationPointValue(); //WW
  if(m_pcs->tim_type_name.compare("STEADY")==0)
    m_pcs->selected = false;
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: RichardsFlow
Task: Similate Richards flow  
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::RichardsFlow()
{
   //-------  WW                          
   double error = 0.;
   CRFProcess *m_pcs = total_processes[2];
   if(!m_pcs->selected) return error; 
   bool twoflowcpl = false;
   //if(GROUNDWATER_FLOW|| OVERLAND_FLOW) WW
   if(total_processes[1]||total_processes[6])  
     twoflowcpl = true;
   if(twoflowcpl)
   {  //-------  WW            
      lop_coupling_iterations = m_pcs->m_num->cpl_iterations;  // JOD coupling      
      if(pcs_vector.size()>1 && lop_coupling_iterations > 1)
      {
         m_pcs->CopyCouplingNODValues();
         TolCoupledF = m_pcs->m_num->cpl_tolerance;
      }
      //WW if(m_pcs->adaption) PCSStorage();
      CFEMesh* m_msh = FEMGet("RICHARDS_FLOW");
      if(m_msh->geo_name.compare("REGIONAL")==0)
        LOPExecuteRegionalRichardsFlow(m_pcs);
      else
        pcs_flow_error = error = m_pcs->ExecuteNonLinear(); // JOD 4.10.01
      if(m_pcs->saturation_switch == true)
        m_pcs->CalcSaturationRichards(1, false); // JOD
      else
        m_pcs->CalcSecondaryVariablesUnsaturatedFlow();  //WW
#ifndef NEW_EQS //WW. 07.11.2008
     // if(lop_coupling_iterations > 1) // JOD  4.10.01 removed
     //    pcs_coupling_error = m_pcs->CalcCouplingNODError();
#endif
       conducted = true; //WW 
   }
   else  //WW
   {
      CFEMesh* m_msh = FEMGet("RICHARDS_FLOW"); //WW
      if(m_msh->geo_name.compare("REGIONAL")==0)
        LOPExecuteRegionalRichardsFlow(m_pcs);
      else
        error = m_pcs->ExecuteNonLinear();
      if(m_pcs->TimeStepAccept())
      {
        m_pcs->CalcSecondaryVariablesUnsaturatedFlow();  //WW
        CalcVelocities = true;
        conducted = true; //WW 
      }
   } 
   if(m_pcs->TimeStepAccept())
     m_pcs->CalIntegrationPointValue(); //WW		
   return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: TwoPhaseFlow
Task: Similate twp-phase flow  
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
12.2008 WW Update
01.2008 WW Add phases
-------------------------------------------------------------------------*/
inline double Problem::TwoPhaseFlow()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[3];
  if(!m_pcs->selected) return error; //12.12.2008 WW
  //
  for(int i=0;i<(int)multiphase_processes.size();i++) //08.01.2009. WW
  {
    m_pcs = multiphase_processes[i];
    error = m_pcs->ExecuteNonLinear();
    if(m_pcs->TimeStepAccept())
    {
      PCSCalcSecondaryVariables(); 
      m_pcs->CalIntegrationPointValue(); 
      //CB 12/09 (first time added on 010808) Velocity at CenterOfGravity, required for NAPL dissolution
      if (i==0) // is 0 in all cases the correct index?
        m_pcs->CalcELEVelocities(); 
    }
  }
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: MultiPhaseFlow()
Task: Similate multi-phase flow by p-p scheme  
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
12.2008 WW Update
-------------------------------------------------------------------------*/
inline double Problem::MultiPhaseFlow()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[4];
  if(!m_pcs->selected) return error; //12.12.2008 WW
  error = m_pcs->ExecuteNonLinear();
  if(m_pcs->TimeStepAccept())
    m_pcs->CalIntegrationPointValue(); //WW
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: PS_Global()
Task: Similate multi-phase flow by p-p scheme
Return: error
Programming:
03/2009 PCH Implementation
Modification:
-------------------------------------------------------------------------*/
inline double Problem::PS_Global()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[3];
  if(!m_pcs->selected) return error;
  error = m_pcs->ExecuteNonLinear();
  if(m_pcs->TimeStepAccept())
    m_pcs->CalIntegrationPointValue();
  return error;
}

/*-------------------------------------------------------------------------
GeoSys - Function: GroundWaterFlow()
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
12.2008 WW Update
-------------------------------------------------------------------------*/
inline double Problem::GroundWaterFlow()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[1];
  if(!m_pcs->selected) return error; //12.12.2008 WW
  error =  m_pcs->ExecuteNonLinear();
  //................................................................
  // Calculate secondary variables
  // NOD values
  conducted = true; //WW 
  cout << "      Calculation of secondary NOD values" << endl;
  if(m_pcs->TimeStepAccept())
  {
#ifdef RESET_4410
    PCSCalcSecondaryVariables(); // PCS member function
#endif
    cout << "      Calculation of secondary GP values" << endl;
    m_pcs->CalIntegrationPointValue(); //WW
    m_pcs->cal_integration_point_value = false; //WW Do not extropolate Gauss velocity
	if(m_pcs->tim_type_name.compare("STEADY")==0) //SB
		m_pcs->selected = false;
  }
  // ELE values
#ifndef NEW_EQS //WW. 07.11.2008
  if(m_pcs->tim_type_name.compare("STEADY")==0) //CMCD 05/2006
  {
     cout << "      Calculation of secondary ELE values" << endl;
     m_pcs->AssembleParabolicEquationRHSVector(); //WW LOPCalcNODResultants();
     m_pcs->CalcELEVelocities();
     m_pcs->selected = false;
  }
#endif
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: ComponentalFlow();
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::ComponentalFlow()
{
  double error = 1.e8;
  CRFProcess *m_pcs = total_processes[5];
  if(!m_pcs->selected) return error; //12.12.2008 WW
  //
  error = m_pcs->ExecuteNonLinear();
  m_pcs->CalIntegrationPointValue(); //WW
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: OverlandFlow()
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::OverlandFlow()
{
  double error = 1.e8;
  CRFProcess *m_pcs = total_processes[6];
  if(!m_pcs->selected) return error; //12.12.2008 WW

  error = m_pcs->ExecuteNonLinear();
  if(m_pcs->TimeStepAccept())
    PCSCalcSecondaryVariables(); 
  return error;
}

/*-------------------------------------------------------------------------
GeoSys - Function: AirFlow()
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::AirFlow()
{
  double error = 1.e8;
  CRFProcess *m_pcs = total_processes[7];
  if(!m_pcs->selected) return error; //12.12.2008 WW
  
  error = m_pcs->ExecuteNonLinear();
  m_pcs->CalIntegrationPointValue(); //WW
  m_pcs->CalcELEVelocities(); //OK
  //
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: HeatTransport
Task: Similate heat transport 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::HeatTransport()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[8];
  if(!m_pcs->selected) return error; //12.12.2008 WW

  error = m_pcs->ExecuteNonLinear();
  //if(m_pcs->non_linear)
  //  error = m_pcs->ExecuteNonLinear();
  //else
  //  error = m_pcs->Execute();
  return error;
}
/*-------------------------------------------------------------------------
GeoSys - Function: MassTrasport
Task: Similate heat transport 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
12.2008 WW Immigrtate the new functionalities  from loop_pcs.cpp
-------------------------------------------------------------------------*/
inline double Problem::MassTrasport()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[11];
  //
  if(!m_pcs->selected) return error; //12.12.2008 WW
  
  for(int i=0;i<(int)transport_processes.size();i++)
    {
      m_pcs = transport_processes[i];      //18.08.2008 WW
      if(CPGetMobil(m_pcs->GetProcessComponentNumber())> 0) //Component Mobile ? 
      error = m_pcs->ExecuteNonLinear(); //NW. ExecuteNonLinear() is called to use the adaptive time step scheme
    }
  // Calculate Chemical reactions, after convergence of flow and transport 
  // Move inside iteration loop if couplingwith transport is implemented SB:todo
  //SB:todo move into Execute Reactions	  if((aktueller_zeitschritt % 1) == 0)  
  //REACT *rc = NULL; //OK
  //rc = REACT_vec[0]; //OK
  //				if(rc->flag_pqc) rc->ExecuteReactions();
  //				delete rc;
  if(KinReactData_vector.size() > 0) 
    {  // WW moved the following lines into this curly braces. 12.12.2008
      //SB4900    ClockTimeVec[0]->StopTime("Transport");
      //SB4900    ClockTimeVec[0]->StartTime();
      // Calculate Chemical reactions, after convergence of flow and transport 
      // Move inside iteration loop if couplingwith transport is implemented SB:todo
      // First calculate kinetic reactions
      KinReactData_vector[0]->ExecuteKinReact();
      //SB4900 ClockTimeVec[0]->StopTime("KinReactions");
      //SB4900 ClockTimeVec[0]->StartTime();
    }
  if(REACT_vec.size()>0) //OK
    {
      if(REACT_vec[0]->flag_pqc){ 
#ifdef REACTION_ELEMENT
	REACT_vec[0]->ExecuteReactionsPHREEQC0();
#else
	// REACT_vec[0]->ExecuteReactions();

#ifdef LIBPHREEQC
	// MDL: built-in phreeqc
	REACT_vec[0]->ExecuteReactionsPHREEQCNewLib();
#else
	REACT_vec[0]->ExecuteReactionsPHREEQCNew();
#endif // LIBPHREEQC

#endif // REACTION_ELEMENT
      }
    }
#ifdef GEM_REACT
  else    // WW moved these pare of curly braces inside  ifdef GEM_REACT
    {
      if (m_vec_GEM->initialized_flag == 1)//when it was initialized. 
	{
	  int m_time = 1; // 0-previous time step results; 1-current time step results
          
	  // Check if the Sequential Iterative Scheme needs to be intergrated
	  if (m_pcs->m_num->cpl_iterations > 1)
	    m_vec_GEM->flag_iterative_scheme = 1; // set to standard iterative scheme;
	  // write time
	  cout << "CPU time elapsed before GEMIMP2K: " << TGetTimer(0) << " s" << endl;
	  // Move current xDC to previous xDC
	  m_vec_GEM->CopyCurXDCPre();
	  // Get info from MT
	  // m_vec_GEM->ConvPorosityNodeValue2Elem(); // 
	  m_vec_GEM->GetReactInfoFromMassTransport(m_time);	// second arguments should be one if we work with concentrations
	  // m_vec_GEM->ConcentrationToMass();	    
           m_vec_GEM->Run_MainLoop(FileName,aktueller_zeitschritt);     // Run GEM
	  
	  // m_vec_GEM->MassToConcentration();
	  // Calculate the different of xDC
	  m_vec_GEM->UpdateXDCChemDelta();						  
	  // Set info in MT
	  m_vec_GEM->SetReactInfoBackMassTransport(m_time);
	  //m_vec_GEM->ConvPorosityNodeValue2Elem(); // update element porosity and push back values
	  // write time
	  cout << "CPU time elapsed after GEMIMP2K: " << TGetTimer(0) << " s" << endl;
	}
    }
#endif // GEM_REACT
  
#ifdef CHEMAPP
  if(Eqlink_vec.size()>0) 
    Eqlink_vec[0]->ExecuteEQLINK();
#endif
#ifdef BRNS
  if(m_vec_BRNS->init_flag == true)
    {m_vec_BRNS->RUN(  dt  /*time value in seconds*/);} 
#endif

  // if(KinReactData_vector.size() > 0)  //12.12.2008 WW
  //SB4900    ClockTimeVec[0]->StopTime("EquiReact");

  return error; 
}
/*-------------------------------------------------------------------------
GeoSys - Function: FluidMomentum()
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::FluidMomentum()
{
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[9];
  //
  if(!m_pcs->selected) return error; //12.12.2008 WW

  CFluidMomentum *fm_pcs = NULL; // by PCH
  //
  CFEMesh* m_msh = fem_msh_vector[0];  // Something must be done later on here.
  if(m_pcs->tim_type_name.compare("STEADY")==0 && aktueller_zeitschritt>5) // JT 2010, allow a few equilibration steps before "STEADY"
    	m_pcs->selected = false;

  fm_pcs = m_msh->fm_pcs;
  fm_pcs->Execute();

  // Switch off rechard flow if
  if(m_pcs->num_type_name.compare("STEADY")==0 && aktueller_zeitschritt>5)
  {
	 // Turn off FLUID_MOMENTUM
     m_pcs->selected = false;
     // Turn off RICHARDS_FLOW
     m_pcs = PCSGet("RICHARDS_FLOW");
     if(m_pcs)
       m_pcs->selected = false;
     // Turn off LIQUID_FLOW
     m_pcs = PCSGet("LIQUID_FLOW");
     if(m_pcs)
       m_pcs->selected = false;
     // Turn off GROUNDWATER_FLOW
     m_pcs = PCSGet("GROUNDWATER_FLOW");
     if(m_pcs)
       m_pcs->selected = false;
  }
  //
  //error = 0.0 // JTARON... in unsteady flow, setting error=0.0 corresponds to error_cpl=0.0, and the coupling loop ceases before RWPT is performed
  //            // What is the correct way to handle this, rather than setting error=1.e8???
  return error;
}

/*-------------------------------------------------------------------------
GeoSys - Function: RandomWalker()
Task: 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
12.2008 WW
-------------------------------------------------------------------------*/
inline double Problem::RandomWalker()
{
	double error = 1.0e+8;
	//
	CRFProcess *m_pcs = total_processes[10];
	//
	if(!m_pcs->selected) return error; //12.12.2008 WW
	//
	CFEMesh* m_msh = NULL;

	if(m_pcs&&m_pcs->selected)
	{
		lop_coupling_iterations = 1;

		// Mount the proper mesh
		CFEMesh* m_msh = NULL;
		for(int i=0; i< (int)pcs_vector.size(); ++i)
		{
			m_pcs = pcs_vector[i];

			// Select the mesh whose process name has the mesh for Fluid_Momentum
			if( m_pcs->pcs_type_name.find("RICHARDS_FLOW")!=string::npos)
				m_msh = FEMGet("RICHARDS_FLOW");
			else if( m_pcs->pcs_type_name.find("LIQUID_FLOW")!=string::npos)
				m_msh = FEMGet("LIQUID_FLOW");
			else if( m_pcs->pcs_type_name.find("GROUNDWATER_FLOW")!=string::npos)
				m_msh = FEMGet("GROUNDWATER_FLOW");
			else;
		}

#ifdef RANDOM_WALK
		RandomWalk *rw_pcs = NULL; // By PCH
		rw_pcs = m_msh->PT;

		// Do I need velocity fileds solved by the FEM?
		if(m_pcs->tim_type_name.compare("PURERWPT")==0)
		{
			rw_pcs->PURERWPT = 1;
			char *dateiname = NULL;
			int sizeOfWord = 100;
			dateiname = (char *)malloc(sizeOfWord * sizeof(char ));

			string filename = FileName;
			for(int i=0; i<= (int)filename.size(); ++i)
				dateiname[i] = filename[i];

			rw_pcs->ReadInVelocityFieldOnNodes(dateiname);

			delete [] dateiname;
		}

		// Set the mode of the RWPT method
		if(m_pcs->num_type_name.compare("HETERO")==0)
		{
			rw_pcs->RWPTMode = 1;	// Set it for heterogeneous media
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HOMO_ADVECTION")==0)
		{
			rw_pcs->RWPTMode = 2;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HETERO_ADVECTION")==0)
		{
			rw_pcs->RWPTMode = 3;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HOMO_DISPERSION")==0)
		{
			rw_pcs->RWPTMode = 4;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HETERO_DISPERSION")==0)
		{
			rw_pcs->RWPTMode = 5;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HETERO_FDM")==0)
		{
			rw_pcs->RWPTMode = 1;	// Set it for heterogeneous media
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HOMO_ADVECTION_FDM")==0)
		{
			rw_pcs->RWPTMode = 2;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HETERO_ADVECTION_FDM")==0)
		{
			rw_pcs->RWPTMode = 3;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HOMO_DISPERSION_FDM")==0)
		{
			rw_pcs->RWPTMode = 4;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else if(m_pcs->num_type_name.compare("HETERO_DISPERSION_FDM")==0)
		{
			rw_pcs->RWPTMode = 5;
			cout << "RWPT is on " << m_pcs->num_type_name << " mode." << endl;
		}
		else	// HOMO Advection + Dispersion
		{
			rw_pcs->RWPTMode = 0;
			cout << "RWPT is on HOMO_ADVECTION_DISPERSION mode." << endl;
		}

		if(m_pcs->num_type_name.find("FDM")!=string::npos)
		{
			rw_pcs->PURERWPT = 2;
			if(rw_pcs->FDMIndexSwitch == 0)
			{
				rw_pcs->buildFDMIndex();
				// Switch off
				rw_pcs->FDMIndexSwitch = 1;
			}
		}

		if(rwpt_numsplits < 0)
			rwpt_numsplits=10;		// JTARON 2010 set default value, unless specified in .tim input file

		rw_pcs->AdvanceBySplitTime(dt,rwpt_numsplits);
	//	rw_pcs->TraceStreamline(); // JTARON, no longer needed
		rw_pcs->RandomWalkOutput(aktuelle_zeit,aktueller_zeitschritt);
#endif
	}

return 0.0;
}


/*-------------------------------------------------------------------------
GeoSys - Function: Deformation
Task: Similate deformation 
Return: error
Programming: 
07/2008 WW Extract from LOPTimeLoop_PCS();
Modification:
-------------------------------------------------------------------------*/
inline double Problem::Deformation()
{
  CRFProcessDeformation *dm_pcs = NULL;
  double error = 1.0e+8;
  CRFProcess *m_pcs = total_processes[12];
  //
  dm_pcs = (CRFProcessDeformation *)(m_pcs);
  error = dm_pcs->Execute(loop_index);
  //Error
  if (dm_pcs->type==41)
  {
    m_pcs->cal_integration_point_value = true;
    dm_pcs->CalIntegrationPointValue(); 
  }
  return error;
}
/**************************************************************************
FEMLib-Method: 
02/2005 OK Implementation
08/2005 WW Changes due to geometry objects applied
08/2005 MB Changes ... (OK to what ?)
04/2006 OK and once again ...
07/2008 WW Extract from LOPTimeLoop_PCS();
**************************************************************************/
inline void Problem::LOPExecuteRegionalRichardsFlow(CRFProcess*m_pcs_global)
{
  int j,k;
  long i;
  CElem* m_ele = NULL;
  CNode* m_nod = NULL;
  int no_local_elements = m_pcs_global->m_msh->no_msh_layer;
  int no_local_nodes = no_local_elements + 1;
  long g_element_number,g_node_number;
  CFEMesh* m_msh_local = NULL;
  CRFProcess* m_pcs_local = NULL;
  vec<CNode*>ele_nodes(20);
  double value;
#if defined(USE_MPI_REGSOIL)
  double *values;
  int rp;
  int num_parallel_blocks;
  int l;
#endif
  int timelevel = 1;
  int idxp  = m_pcs_global->GetNodeValueIndex("PRESSURE1") + timelevel;
  //WW int idxcp = m_pcs_global->GetNodeValueIndex("PRESSURE_CAP") + timelevel;
  int idxS  = m_pcs_global->GetNodeValueIndex("SATURATION1") + timelevel;
  CElem* m_ele_local = NULL;
  CNode* m_nod_local = NULL;
  
#if defined(USE_MPI_REGSOIL)
  values      = new double[no_local_nodes];   // Should be more sophisticated
#endif

  //======================================================================
  if(aktueller_zeitschritt==1)
  {
    //--------------------------------------------------------------------
    // Create local RICHARDS process
    cout << "    Create local RICHARDS process" << endl;
    m_pcs_local = new CRFProcess();
    m_pcs_local->pcs_type_name = m_pcs_global->pcs_type_name;
    m_pcs_local->num_type_name = m_pcs_global->num_type_name;
    m_pcs_local->cpl_type_name = m_pcs_global->cpl_type_name;
    m_pcs_local->Write_Matrix = m_pcs_global->Write_Matrix;
    m_pcs_local->pcs_type_number = (int)pcs_vector.size();
    m_pcs_local->Config();
    //--------------------------------------------------------------------
    // Create local MSH
    m_msh_local = new CFEMesh();
    m_msh_local->geo_name = "RICHARDS_FLOW_LOCAL";
    m_msh_local->ele_type = 1;
    m_msh_local->no_msh_layer = m_pcs_global->m_msh->no_msh_layer;
    //....................................................................
    m_msh_local->ele_vector.resize(no_local_elements);
    for(j=0;j<no_local_elements;j++)
    {
      m_ele = m_pcs_global->m_msh->ele_vector[j];
      m_ele_local = new CElem(j,m_ele);
      for(k=0;k<2;k++) // ele_type
        m_ele_local->nodes_index[k] = j+k;
      m_msh_local->ele_vector[j] = m_ele_local;
    }
    m_msh_local->nod_vector.resize(no_local_nodes);
    m_msh_local->Eqs2Global_NodeIndex.resize(no_local_nodes);
    for(j=0;j<no_local_nodes;j++)
    {
      m_nod = m_pcs_global->m_msh->nod_vector[j];
      m_nod_local = new CNode(j,m_nod->X(),m_nod->Y(),m_nod->Z());
      //m_nod_local = m_nod;
      m_msh_local->nod_vector[j] = m_nod_local;
   	  m_msh_local->nod_vector[j]->SetEquationIndex(j);
      m_msh_local->Eqs2Global_NodeIndex[j] = m_msh_local->nod_vector[j]->GetIndex();
    }
    m_msh_local->ConstructGrid();
    m_msh_local->FillTransformMatrix();
    m_msh_local->FaceNormal();
    //....................................................................
    fem_msh_vector.push_back(m_msh_local);
    //....................................................................
    m_pcs_local->m_msh = m_msh_local;
    m_pcs_local->Create();
    //....................................................................
    // BC
    //....................................................................
    // ST
/*
    for(s=0;s<(int)m_pcs_global->st_node_value.size();s++)
    {
      m_nod_value = new CNodeValue(); //OK
      //m_nod_value = m_pcs_global->st_node_value[s];
      m_nod_value->node_value = m_pcs_global->st_node_value[s]->node_value;
      m_nod_value->msh_node_number = m_pcs_global->st_node_value[s]->msh_node_number;
      m_nod_value->geo_node_number =  m_nod_value->msh_node_number; //WW
      m_pcs_local->st_node_value.push_back(m_nod_value);
    }
*/
    m_pcs_local->st_node_value.clear();
    m_pcs_local->st_node.clear();
    for(j=0;j<(int)m_pcs_global->st_node_value.size();j++)             
      m_pcs_local->st_node_value.push_back(m_pcs_global->st_node_value[j]); 
    for(j=0;j<(int)m_pcs_global->st_node.size();j++)             
      m_pcs_local->st_node.push_back(m_pcs_global->st_node[j]); 
    //....................................................................
    pcs_vector.push_back(m_pcs_local);

  }
  //======================================================================
  else
  {
    for(i=0;i<(long)m_pcs_global->m_msh->nod_vector.size();i++)
    {
      value = m_pcs_global->GetNodeValue(i,idxp);
      m_pcs_global->SetNodeValue(i,idxp-1,value);
     //WW value = m_pcs_global->GetNodeValue(i,idxcp);
     //WW m_pcs_global->SetNodeValue(i,idxcp-1,value);
      value = m_pcs_global->GetNodeValue(i,idxS);
      m_pcs_global->SetNodeValue(i,idxS-1,value);
    }
  }
  //======================================================================
  cout << "    ->Process " << m_pcs_global->pcs_number << ": " \
       << "REGIONAL_" << m_pcs_global->pcs_type_name << endl;
  int no_richards_problems = (int)(m_pcs_global->m_msh->ele_vector.size()/no_local_elements);
#ifndef USE_MPI_REGSOIL
  for(i=0;i<no_richards_problems;i++)
  //for(i=0;i<2;i++)
  {
    cout << "->Conlumn number " << i<<endl;
    m_pcs_local = pcs_vector[(int)pcs_vector.size()-1];
    m_pcs_local->pcs_number = i;
    m_msh_local = fem_msh_vector[(int)fem_msh_vector.size()-1];
    //....................................................................
    // Set local NODs
    for(j=0;j<no_local_nodes;j++)
    {
      g_node_number = j+(i*no_local_nodes);
      m_nod = m_pcs_global->m_msh->nod_vector[g_node_number];
      m_nod_local = m_msh_local->nod_vector[j];
      //m_nod_local = m_nod;
      m_nod_local->connected_elements.push_back(i);
//m_nod_local->ok_dummy = i;
    }
    //....................................................................
    // Set local ELEs
    for(j=0;j<no_local_elements;j++)
    {
      g_element_number = j+(i*no_local_elements);
      m_ele = m_pcs_global->m_msh->ele_vector[g_element_number];
      m_ele_local = m_msh_local->ele_vector[j];
      m_ele_local->SetPatchIndex(m_ele->GetPatchIndex());
    }
    //....................................................................
    // Set ICs
    if(aktueller_zeitschritt==1)   //YD
    {
     for(j=0;j<no_local_nodes;j++)
     {
        g_node_number = j+(i*no_local_nodes);
        value = m_pcs_global->GetNodeValue(g_node_number,idxp);
        m_pcs_local->SetNodeValue(j,idxp-1,value);
        m_pcs_local->SetNodeValue(j,idxp,value);       
       //WW value = m_pcs_global->GetNodeValue(g_node_number,idxcp);
       //WW m_pcs_local->SetNodeValue(j,idxcp-1,value);
       //WW  m_pcs_local->SetNodeValue(j,idxcp,value);     
        value = m_pcs_global->GetNodeValue(g_node_number,idxS);
        m_pcs_local->SetNodeValue(j,idxS-1,value);
        m_pcs_local->SetNodeValue(j,idxS,value);     
     }
    }
    else
    {
     for(j=0;j<no_local_nodes;j++)
     {
        g_node_number = j+(i*no_local_nodes);
        value = m_pcs_global->GetNodeValue(g_node_number,idxp);
        m_pcs_local->SetNodeValue(j,idxp-1,value);
        //value = m_pcs_global->GetNodeValue(g_node_number,idxcp);
        //m_pcs_local->SetNodeValue(j,idxcp-1,value);
        value = m_pcs_global->GetNodeValue(g_node_number,idxS);
        m_pcs_local->SetNodeValue(j,idxS-1,value);
     }
    }
    //....................................................................
    // Set local BCs
    m_pcs_local->CreateBCGroup();
    //....................................................................
    // Set local STs
    // m_pcs_local->CreateSTGroup();
    // look for corresponding OF-triangle
    // m_ele_of = m_msh_of->GetElement(m_pnt_sf);
    //....................................................................
    m_pcs_local->ExecuteNonLinear();
    //....................................................................
    // Store results in global PCS tables
    for(j=0;j<no_local_nodes;j++)
    {
      g_node_number = j+(i*no_local_nodes);
      value = m_pcs_local->GetNodeValue(j,idxp);
      m_pcs_global->SetNodeValue(g_node_number,idxp,value);
     // value = m_pcs_local->GetNodeValue(j,idxcp);
      //m_pcs_global->SetNodeValue(g_node_number,idxcp,value);
      value = m_pcs_local->GetNodeValue(j,idxS);
      m_pcs_global->SetNodeValue(g_node_number,idxS,value);
    }
    //m_pcs->m_msh = FEMGet("RICHARDS_FLOW");
    // pcs->m_msh->RenumberNodesForGlobalAssembly(); related to Comment 1  // MB/WW
  }
#else
  num_parallel_blocks = no_richards_problems / size;
  
#ifdef TRACE
  std::cout << "Num parallel blocks: " << num_parallel_blocks << std::endl;
#endif

  for(i=0; i<num_parallel_blocks+1; i++)
  //for(i=0;i<2;i++)
  {
    if(i*size + myrank < no_richards_problems) {   // Do a parallel block
      rp = i*size + myrank;
      m_pcs_local = pcs_vector[(int)pcs_vector.size()-1];
      m_pcs_local->pcs_number = rp;
      m_msh_local = fem_msh_vector[(int)fem_msh_vector.size()-1];
      //....................................................................
      // Set local NODs
      for(j=0;j<no_local_nodes;j++)
      {
        g_node_number = j+(rp*no_local_nodes);
        m_nod = m_pcs_global->m_msh->nod_vector[g_node_number];
        m_nod_local = m_msh_local->nod_vector[j];
        //m_nod_local = m_nod;
        m_nod_local->connected_elements.push_back(rp);  // ????
      }
      //....................................................................
      // Set local ELEs
      for(j=0;j<no_local_elements;j++)
      {
        g_element_number = j+(rp*no_local_elements);
        m_ele = m_pcs_global->m_msh->ele_vector[g_element_number];
        m_ele_local = m_msh_local->ele_vector[j];
        m_ele_local->SetPatchIndex(m_ele->GetPatchIndex());
      }
      //....................................................................
      // Set ICs
      if(aktueller_zeitschritt==1)   //YD
      {
        for(j=0;j<no_local_nodes;j++)
        {
          g_node_number = j+(rp*no_local_nodes);
          value = m_pcs_global->GetNodeValue(g_node_number,idxp);
          m_pcs_local->SetNodeValue(j,idxp-1,value);
          m_pcs_local->SetNodeValue(j,idxp,value);       
          value = m_pcs_global->GetNodeValue(g_node_number,idxcp);
          m_pcs_local->SetNodeValue(j,idxcp-1,value);
          m_pcs_local->SetNodeValue(j,idxcp,value);     
          value = m_pcs_global->GetNodeValue(g_node_number,idxS);
          m_pcs_local->SetNodeValue(j,idxS-1,value);
          m_pcs_local->SetNodeValue(j,idxS,value);     
        }
      }
      else
      {
        for(j=0;j<no_local_nodes;j++)
        {
          g_node_number = j+(rp*no_local_nodes);
          value = m_pcs_global->GetNodeValue(g_node_number,idxp);
          m_pcs_local->SetNodeValue(j,idxp-1,value);
          value = m_pcs_global->GetNodeValue(g_node_number,idxcp);
          m_pcs_local->SetNodeValue(j,idxcp-1,value);
          value = m_pcs_global->GetNodeValue(g_node_number,idxS);
          m_pcs_local->SetNodeValue(j,idxS-1,value);
        }
      }
      //....................................................................
      // Set local BCs
      m_pcs_local->CreateBCGroup();
      //....................................................................
      // Set local STs
      // m_pcs_local->CreateSTGroup();
      // look for corresponding OF-triangle
      // m_ele_of = m_msh_of->GetElement(m_pnt_sf);
      //....................................................................
#ifdef TRACE
      std::cout << "Executing local process." << std::endl;
#endif
      m_pcs_local->ExecuteNonLinear();
    }  // End of parallel block
    //....................................................................
    // Store results in global PCS tables
    for(int k=0; k<size; k++) {
      rp = i*size + k;
      if(rp < no_richards_problems) {
        if(myrank == k) {
          // idxp
          for(l=0; l<no_local_nodes; l++) 
            values[l] = m_pcs_local->GetNodeValue(l, idxp);
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxp, values[l]);
          // idxcp
          for(l=0; l<no_local_nodes; l++) 
            values[l] = m_pcs_local->GetNodeValue(l, idxcp);
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxcp, values[l]);
          // idxS
          for(l=0; l<no_local_nodes; l++) 
            values[l] = m_pcs_local->GetNodeValue(l, idxS);
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxS, values[l]);
        }
        else {
          // idxp
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxp, values[l]);
          // idxcp
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxcp, values[l]);
          // idxS
          MPI_Bcast((void *)values, no_local_nodes, MPI_DOUBLE, k, MPI_COMM_WORLD);
          for(l=0; l<no_local_nodes; l++)
            m_pcs_global->SetNodeValue(l+rp*no_local_nodes, idxS, values[l]);
        }
      }
    }
  }
#endif
  //----------------------------------------------------------------------

#if defined(USE_MPI_REGSOIL)
  delete[] values;
#endif

}



/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2003 SB Implementation
01/2004 MX k_eff 
11/2004 OK fluid mass fluxes
08/2008 WW Extract from LOPTimeLoop_PCS();
last modification:
**************************************************************************/
void Problem::LOPCalcELEResultants()
{
  int p;
  int no_processes = (int)pcs_vector.size();
  CRFProcess* m_pcs = NULL;

  for(p=0;p<no_processes;p++){
    m_pcs = pcs_vector[p];
    if(!m_pcs->selected) //OK4108
      continue;
    //cout << "LOPCalcELEResultants: " << m_pcs->pcs_type_name << endl;
    switch(m_pcs->pcs_type_name[0]){
      default:
        break;
      case 'L': // Liquid flow
        m_pcs->CalcELEVelocities();
        break;
      case 'G': // Groundwater flow
        m_pcs->CalcELEVelocities();
        break;
      case 'A': // Gas flow
        m_pcs->CalcELEVelocities();
        m_pcs->CalcELEMassFluxes();
        break;
      case 'T': // Two-phase flow
        break;
      case 'C': // Componental flow
        break;
      case 'H': // Heat transport
        break;
      case 'M': // Mass transport
        break;
      case 'D': // Deformation
        break;
      case 'O': // Overland flow
        m_pcs->CalcELEVelocities();
        break;
      case 'R': // Richards flow
        m_pcs->CalcELEVelocities();   
        break;
	  case 'F': // Fluid Momentum
        break;
    }
  }
}

/**************************************************************************
 ROCKFLOW - Funktion: ASMCalcNodeWDepth
                                                                          
 Task:
   Berechnung und Speichern der Knotenfl?se
 Parameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: long i: node index
 Result:
   - void -
                                                                          
 Programmaenderungen:
   11/2002   MB/OK  Implementation
   10/2004   MB     PCS   
   12/2008   WW     Encapsulate to this class                                            
**************************************************************************/
inline void Problem::ASMCalcNodeWDepth(CRFProcess *m_pcs)
{
  int nidx, nidy, nidz;
  //OK411 int timelevel = 1; 
  double WDepth;

  nidx = m_pcs->GetNodeValueIndex("HEAD")+1;
  nidy = m_pcs->GetNodeValueIndex("WDEPTH");
  nidz = m_pcs->GetNodeValueIndex("COUPLING");
  for(long nn=0;nn<(long)m_pcs->m_msh->nod_vector.size();nn++)
  {
    WDepth = m_pcs->GetNodeValue(nn, nidx) - m_pcs->m_msh->nod_vector[nn]->Z();
	m_pcs->SetNodeValue(nn,nidz, m_pcs->GetNodeValue(nn,nidz+1) ); // JOD only needed for GREEN_AMPT source term 
    if (WDepth < 0.0) {
      WDepth  = 0.0;
    }
    m_pcs->SetNodeValue(nn, nidy, WDepth);
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: PCSCalcSecondaryVariables
                                                                          */
/* Aufgabe:
   Berechung von secondary variables w?rend der Zeitschleife
   Abfrage je nach Prozess, der aktiv ist
                                                                          */
/* Programmaenderungen:
   08/2003   SB   Implementation
   01/2006   YD   add dual porosity                                       
   01/2007 OK Two-phase flow
                                                                          */
/**************************************************************************/
void Problem::PCSCalcSecondaryVariables()
{
//WW  long j;

  int i, ptype;
  CRFProcess *m_pcs=NULL;
  //OK411 CRFProcess* m_pcs_phase_1 = NULL;
  //OK411 CRFProcess* m_pcs_phase_2 = NULL;
 //WW int ndx_p_gas_old,ndx_p_gas_new,ndx_p_liquid_old,ndx_p_liquid_new,ndx_p_cap_old;
  //----------------------------------------------------------------------
  //OK411 bool pcs_cpl = true; 
  //----------------------------------------------------------------------
  // Check if NAPLdissolution is modeled, required by MMPCalcSecondaryVariablesNew
  bool NAPLdiss = false;
  NAPLdiss = KNaplDissCheck();
  /* go through all processes */
  int no_processes =(int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
	/* get process */
	//pcs = pcs->GetProcessByNumber(i+1);
    m_pcs = pcs_vector[i]; //JOD
	if(m_pcs != NULL){
	ptype = m_pcs->GetObjType();
	switch (ptype) {
    case 1: /* Flow process */
      // do nothing
      break;
    case 66:
      //Temp mit pcs, only for test MB
     ASMCalcNodeWDepth(m_pcs);
      break;
    case 11: /* Non-isothermal flow process */
      break;
    case 13: /* Non-isothermal flow process */
      break;
    case 2: /* Mass transport process */
      break;
    case 3: /* Heat transport */
      // do nothing 
      break;
    case 4: /* Deformation */
      // do nothing
      break;
    case 41: /* Deformation-flow coupled system in monolithic scheme */
      // do nothing
      break;
    case 12: /* Multi-phase flow process */
        MMPCalcSecondaryVariablesNew(m_pcs, NAPLdiss);
        //MMPCalcSecondaryVariablesNew(m_pcs);
        break;
	default:
		cout << "PCSCalcSecondaryVariables - nothing to do" << endl;
		break;
	}
	} //If
 } // while
}

/**************************************************************************
FEMLib-Method: 
05/2009 OK Implementation
**************************************************************************/
bool Problem::Check()
{
  CRFProcess* m_pcs = NULL;
  for(int i=0;i<(int)total_processes.size();i++)
  {
    m_pcs = total_processes[i];
    if(!m_pcs->Check())
      return false;
  }
  return true;
}

/**************************************************************************
FEMLib-Method: 
06/2009 OK Implementation
**************************************************************************/
bool MODCreate()
{
  PCSConfig(); //OK
  if(!PCSCheck()) //OK
  {
    cout << "Not enough data for MOD creation.\n";
    return false;
  }
  else
  {
    return true;
  }
}

#endif //PROBLEM_CLASS

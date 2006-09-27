/**************************************************************************
ROCKFLOW - Source: loop_pcs.c
Task: LOP Template of process-oriented version
Programing:
02/2003 OK Implementation
03/2003 OK SM is OK
04/2003 OK SM+RTM are OK
05/2003 OK 05.05.03 V31 MPC+HTM are OK (seufz...)
05/2003 OK 06.05.03 V32 SM+HTM are OK
05/2003 OK/WW DM start
06/2003 WW DM poro-elasticity
06/2003 WW DM poro-plasticity
07/2004 OK PCS2
***************************************************************************/
/* Includes */
/*========================================================================*/
#if defined(USE_MPI_REGSOIL)
#include <mpi.h>
#include "par_ddc.h"
#endif
#include "stdafx.h"
#include <iostream>
using namespace std;
/*------------------------------------------------------------------------*/
/* Pre-processor definitions */
#include "makros.h"
/*------------------------------------------------------------------------*/
#include "msh_node.h"

/* MshLib */
#include "msh_lib.h"
/*------------------------------------------------------------------------*/
/* PCS */
#include "pcs_dm.h"
//TODO by PCH #include "rf_fluid_momentum.h"	// By PCH
#include "loop_pcs.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_pcs.h"
#include "rf_apl.h"
#include "rf_react.h"
#include "par_ddc.h"
#include "rf_st_new.h"
#include "rf_tim_new.h"
#include "rfmat_cp.h"
#include "elements.h"
#include "cvel.h"
#include "rf_vel_new.h"
#include "cel_mmp.h"
#include "cgs_mpc.h"
#include "cgs_mmp.h"
#include "cgs_asm.h"
#include "rf_fluid_momentum.h"	// By PCH
#include "rf_random_walk.h"     // By PCH
void LOPCalcNodeFluxes(void);
void SetCriticalDepthSourceTerms(void);
// Finite element
#include "fem_ele_std.h"
extern void MTM2CalcSecondaryVariables (void);
#include "rf_bc_new.h"
#include "rf_out_new.h"
#include "tools.h"
#ifdef CHEMAPP
  #include "eqlink.h"
#endif

namespace process{class CRFProcessDeformation;}
using process::CRFProcessDeformation;

//=============================================
// For up coupling caculation in cel_*.cpp
// Will be removed when new FEM is ready
namespace FiniteElement {class element;}
using FiniteElement::CElement;
//=============================================
/*------------------------------------------------------------------------*/
/* LOP */
void LOPCalcELEResultants(void);
int LOPPreTimeLoop_PCS(void);
int LOPTimeLoop_PCS(double*);
int LOPPostTimeLoop_PCS(void);
VoidFuncVoid LOPCalcSecondaryVariables_USER;
void LOPExecuteRegionalRichardsFlow(CRFProcess*m_pcs);
void LOPCalcNODResultants(void);
/*------------------------------------------------------------------------*/
/* Tools */
#include "mathlib.h"
#include "math.h" /* pow() */
#include "matrix.h" /*MXDumpGLS*/

double dt_sum = 0.0;
/**************************************************************************
ROCKFLOW - Function: LOPConfig_PCS
Task: 
Programing:
 02/2003 OK Implementation
last modified:
***************************************************************************/
void LOPConfig_PCS(void)
{
  //OK TimeLoop = LOPTimeLoop_PCS;
  PreTimeLoop = LOPPreTimeLoop_PCS;
  PostTimeLoop = LOPPostTimeLoop_PCS;
}

/**************************************************************************
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
last modified:
***************************************************************************/
void PCSCreate(void)
{
  //----------------------------------------------------------------------
  cout << "---------------------------------------------" << endl;
  cout << "Create PCS processes" << endl;
  //----------------------------------------------------------------------
  int i;
  int no_processes =(int)pcs_vector.size();
  CRFProcess* m_pcs = NULL;
  //----------------------------------------------------------------------
  //OK_MOD if(pcs_deformation>0) Init_Linear_Elements();
  for(i=0;i<no_processes;i++){
    cout << "............................................." << endl;
    m_pcs = pcs_vector[i];
    cout << "Create: " << m_pcs->pcs_type_name << endl;
    m_pcs->pcs_type_number = i;
    m_pcs->Config(); //OK
    if(!m_pcs->pcs_type_name.compare("MASS_TRANSPORT")) cout << " for " << m_pcs->pcs_primary_function_name[0] << " ";
    cout << endl;
    m_pcs->Create();
  }
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    MMP2PCSRelation(m_pcs);
  }
  //----------------------------------------------------------------------
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    m_pcs->ConfigureCouplingForLocalAssemblier();
  }
}

void
PCSProcessDependencies()
{
// pcs_primary_function_name
	std::vector<CRFProcess*> :: iterator it;
	
	for(it=pcs_vector.begin(); it != pcs_vector.end(); it++) {
		std::cout << "Process " << (*it)->pcs_name << std::endl;
		for(int j=0; j<4; j++) {
			if((*it)->pcs_primary_function_name[j] != NULL) {
				 std::cout << "  Function " << j << ": " << (*it)->pcs_primary_function_name[j] << std::endl;					
			}
		}
	}
}


/**************************************************************************
ROCKFLOW - Function: LOPPreTimeLoop_PCS
Task: 
Programing:
 02/2003 OK Implementation
 03/2003 OK H processes
 04/2003 OK C processes
 05/2003 OK T processes
 05/2003 OK TH processes
 08/2004 OK PCS2
 08/2004 WW The new creation of the deformation process
 07/2006 WW Adjust things for DDC
last modified:
***************************************************************************/
int LOPPreTimeLoop_PCS(void)
{
  //----------------------------------------------------------------------
  // Create ST
  //OK STCreateFromPNT();
  //----------------------------------------------------------------------
  GetHeterogeneousFields(); //OK/MB
  //----------------------------------------------------------------------
  // Test MSH-MMP //OK
  int g_max_mmp_groups;
  g_max_mmp_groups = MSHSetMaxMMPGroups();
  if(g_max_mmp_groups>(int)mmp_vector.size()){
    cout << "Error: not enough MMP data";
#ifdef MFC
    AfxMessageBox( "Fatal error: not enough MMP data");
#endif
    abort();
  }
  //----------------------------------------------------------------------
  // Create PCS processes
  PCSCreate();
  //----------------------------------------------------------------------
  // Calculate secondary variables
  if(LOPCalcSecondaryVariables_USER)
    LOPCalcSecondaryVariables_USER();
  //----------------------------------------------------------------------
  // REACTIONS 
  // Initialization of REACT structure for rate exchange between MTM2 and Reactions
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
        rc->ExecuteReactions();
	    REACT_vec.clear();
	    REACT_vec.push_back(rc);
      #endif
      }
    }
//  delete rc;
  }
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
  if(dom_vector.size()>0)
  {
     //WW ----- Domain decomposition ------------------
     int i;
     int no_processes =(int)pcs_vector.size();
     CRFProcess* m_pcs = NULL;
     bool DOF_gt_one = false;
     //----------------------------------------------------------------------
     for(i=0;i<no_processes;i++){
       m_pcs = pcs_vector[i];
       if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
       {
           DOF_gt_one = true;
           break;
       } 
     }
     if(!DOF_gt_one)
        m_pcs = pcs_vector[0];
     // -----------------------
     DOMCreate(m_pcs);
     //
     for(i=0;i<no_processes;i++){
       m_pcs = pcs_vector[i];
       // Config boundary conditions for domain decomposition 
       m_pcs->SetBoundaryConditionSubDomain(); //WW
     }

	
// This will be removed after new sparse matrix is ready. WW
// for solver
#ifdef USE_MPI
     long max_edim; 
     max_edim = 0;
     int dof = 1;
     for(i=0;i<(int)dom_vector.size();i++)
     {
       if(dom_vector[i]->eqs->dim>max_edim)
         max_edim = dom_vector[i]->eqs->dim;
        dof  = GetUnknownVectorDimensionLinearSolver(dom_vector[i]->eqs);    
     }
     //
     p_array = new double[max_edim];
     v_array = new double[max_edim];
     s_array = new double[max_edim];
     t_array = new double[max_edim];
     r_zero = new double[max_edim]; 
     r_array = new double[max_edim]; 
     //
     dof *= overlapped_entry_sizeHQ;
     buff_bc    = new double[dof];
     p_array_bc = new double[dof];
     v_array_bc = new double[dof];
     s_array_bc = new double[dof];
     t_array_bc = new double[dof];
     r_zero_bc  = new double[dof]; 
     r_array_bc = new double[dof]; 
     x_array_bc = new double[dof]; 
     //
     max_edim = 0;
     for(i=0;i<(int)PCS_Solver.size();i++)
     {
       if(PCS_Solver[i]->dim>max_edim)
         max_edim = PCS_Solver[i]->dim;
     }
     buff_global =  new double[max_edim];    
#endif
     //
     node_connected_doms.clear();
   }
  	
  // PA PCSProcessDependencies();

  //----------------------------------------------------------------------
  PCSRestart(); //SB
  //----------------------------------------------------------------------
  // Characteristic numbers
// CalcNeumannNumber();
  //----------------------------------------------------------------------
  return 1;
}

/**************************************************************************
ROCKFLOW - Function: LOPTimeLoop_PCS
Task: 
Programing:
 02/2003 OK Implementation
 08/2004 OK PCS2
 08/2004 WW The deformation process changes due to the new PCS configuration 
 01/2005 OK H unsaturated process
 03/2005 WW Global process coupling indicators
 05/2005 OK MSH
last modified:
***************************************************************************/
int LOPTimeLoop_PCS(double*dt_sum)
{
  int i,j,k;
  //----------------------------------------------------------------------
  int nidx0,nidx1;
  int timelevel;
  int no_processes =(int)pcs_vector.size();
  CRFProcess *m_pcs = NULL;
  CRFProcessDeformation *dm_pcs = NULL;
  CFluidMomentum *fm_pcs = NULL; // by PCH
#ifdef RANDOM_WALK
  RandomWalk* rw_pcs = NULL; // By PCH
#endif
  //----------------------------------------------------------------------
  double TolCoupledF = 1.0e8;
  double pcs_flow_error = 1.0e8;
  double pcs_flow_error0 = 1.0e8;
  double pcs_dm_error = 1.0e8;
  double pcs_dm_error0 = 1.0e8;
  double pcs_dm_cp_error = 1.0e8;
  int lop_coupling_iterations = 10; 
//  int lop_nonlinear_iterations = 15; //OK_OUT 2;
  double pcs_coupling_error = 1000; //MB
  bool CalcVelocities = false;
  //----------------------------------------------------------------------
  //
  if(pcs_vector.size()==1) 
      lop_coupling_iterations = 1; 
  if(pcs_vector.size()>1) {
      lop_coupling_iterations = pcs_vector[0]->m_num->cpl_iterations;
      TolCoupledF = pcs_vector[0]->m_num->cpl_tolerance;
  }
  //----------------------------------------------------------------------
  // Problem type
  string pcs_problem_type = PCSProblemType();
  //----------------------------------------------------------------------
  // Need velocities ? //MB
  if(T_Process||M_Process||MASS_TRANSPORT_Process){
    cout << "  VELOCITIES " << endl;
    CalcVelocities = true;
  }
  //======================================================================
  // Coupling loop
  for(k=0;k<lop_coupling_iterations;k++)
  {
    if(lop_coupling_iterations > 1)
      cout << "  PCS coupling iteration: " << k << "/" << lop_coupling_iterations << endl;
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      // Flow processes
      //------------------------------------------------------------------
		m_pcs = PCSGet("LIQUID_FLOW");
		if(m_pcs&&m_pcs->selected){
			pcs_flow_error = m_pcs->Execute();
			PCSCalcSecondaryVariables(); // PCS member function
			if(!m_pcs->m_msh) //OK
				VELCalcAll(m_pcs);
			else
        m_pcs->CalIntegrationPointValue(); //WW
      if(m_pcs->tim_type_name.compare("STEADY")==0)
				m_pcs->selected = false;
    }
    //------------------------------------------------------------------
    m_pcs = PCSGet("GROUNDWATER_FLOW");
      if(m_pcs&&m_pcs->selected)
      {
        //................................................................
        // Solve PCS for primary variables
        lop_coupling_iterations = m_pcs->m_num->cpl_iterations;
        if(lop_coupling_iterations > 1){
          m_pcs->CopyCouplingNODValues();
          TolCoupledF = pcs_vector[1]->m_num->cpl_tolerance;
        }
        pcs_flow_error =  m_pcs->ExecuteNonLinear();
        if(lop_coupling_iterations > 1){ 
          pcs_coupling_error = m_pcs->CalcCouplingNODError();
        }
        //................................................................
        // Calculate secondary variables
        // NOD values
        cout << "      Calculation of secondary NOD values" << endl;
        PCSCalcSecondaryVariables(); // PCS member function
        // GP values
        if(CalcVelocities)
        { 
          cout << "      Calculation of secondary GP values" << endl;
          m_pcs->CalIntegrationPointValue(); //WW
        }
        // ELE values
		if(m_pcs->tim_type_name.compare("STEADY")==0) //CMCD 05/2006
        {
          cout << "      Calculation of secondary ELE values" << endl;
          LOPCalcNODResultants();
          m_pcs->CalcELEVelocities();
		  m_pcs->selected = false;
        }
      }
      //------------------------------------------------------------------
      m_pcs = PCSGet("RICHARDS_FLOW");
      if(m_pcs&&m_pcs->selected){
        if(m_pcs->adaption) PCSStorage();
        CFEMesh* m_msh = FEMGet("RICHARDS_FLOW");
        if(m_msh->geo_name.compare("REGIONAL")==0)
          LOPExecuteRegionalRichardsFlow(m_pcs);
        else
          pcs_flow_error = m_pcs->ExecuteNonLinear();
        PCSCalcSecondaryVariables(); // PCS member function
        if (CalcVelocities)
          m_pcs->CalIntegrationPointValue(); //WW
      }
#ifdef _FEMPCHDEBUG_
	// PCH Let's monitor what's going on in the FEM
	// This messagebox is for debugging the primary variables at every time step.
	// Should combine with the picking...
	CWnd * pWnd = NULL;
	pWnd->MessageBox("Richard's Flow is just solved!!!","Debug help", MB_ICONINFORMATION);
#endif
	  //--------------------------------------------------------------------
      m_pcs = PCSGet("TWO_PHASE_FLOW");
      if(m_pcs&&m_pcs->selected){
        for(i=0;i<no_processes;i++){
          m_pcs = pcs_vector[i];
          if(m_pcs->pcs_type_name.compare("TWO_PHASE_FLOW")==0)
          {
            pcs_flow_error = m_pcs->ExecuteNonLinear();
            PCSCalcSecondaryVariables(); // PCS member function
            if(!m_pcs->m_msh) //OK
              VELCalcAll(m_pcs);
		    else
              m_pcs->CalIntegrationPointValue(); //WW
          }
        }
      }
      //--------------------------------------------------------------------
      m_pcs = PCSGet("COMPONENTAL_FLOW");
      if(m_pcs&&m_pcs->selected){
        for(i=0;i<no_processes;i++){
          m_pcs = pcs_vector[i];
          if(m_pcs->pcs_type_name.compare("COMPONENTAL_FLOW")==0)
          {
            pcs_flow_error = m_pcs->ExecuteNonLinear();
            if(!m_pcs->m_msh) //OK
              VELCalcAll(m_pcs);
		    else
              m_pcs->CalIntegrationPointValue(); //WW
          }
        }
        PCSCalcSecondaryVariables(); // PCS member function
      }
      //--------------------------------------------------------------------
      m_pcs = PCSGet("OVERLAND_FLOW");
      if(m_pcs&&m_pcs->selected){
        lop_coupling_iterations = m_pcs->m_num->cpl_iterations;
        pcs_flow_error = m_pcs->ExecuteNonLinear();
        PCSCalcSecondaryVariables(); // PCS member function
      }
      //--------------------------------------------------------------------
      m_pcs = PCSGet("AIR_FLOW");
      if(m_pcs&&m_pcs->selected){
        m_pcs->ExecuteNonLinear();
        PCSCalcSecondaryVariables(); // PCS member function
        if(!m_pcs->m_msh) //OK
          VELCalcAll(m_pcs);
		else
          m_pcs->CalIntegrationPointValue(); //WW
        m_pcs->CalcELEVelocities(); //OK
      }
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  m_pcs = PCSGet("HEAT_TRANSPORT"); //WW
      if(m_pcs&&m_pcs->selected){
        if(m_pcs->non_linear){
          m_pcs->ExecuteNonLinear();
        }
        else{
          lop_coupling_iterations = 1;
          m_pcs->Execute();
        }
      }
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if(k==0) pcs_flow_error0 = pcs_flow_error;
	  m_pcs = PCSGet("FLUID_MOMENTUM");
      if(m_pcs&&m_pcs->selected)
      {
		CFEMesh* m_msh = fem_msh_vector[0];  // Something must be done later on here.
		if(m_pcs->tim_type_name.compare("STEADY")==0)
		  m_pcs->selected = false;
		fm_pcs = m_msh->fm_pcs;
		fm_pcs->Execute();
      }
      //--------------------------------------------------------------------
	  // PCH The velocity process ends here.
#ifdef _FEMPCHDEBUG_
	// PCH Let's monitor what's going on in the FEM
	// This messagebox is for debugging the primary variables at every time step.
	// Should combine with the picking...
//	CWnd * pWnd = NULL;
	pWnd->MessageBox("Velocity is just solved!!!","Debug help", MB_ICONINFORMATION);
#endif
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      // PCH Random Walk Particle Tracking starts here.
      m_pcs = PCSGet("RANDOM_WALK"); 
      if(m_pcs&&m_pcs->selected)
	  {
        lop_coupling_iterations = 1;

		CFEMesh* m_msh = fem_msh_vector[0];  // Something must be done later on here.
#ifdef RANDOM_WALK
		rw_pcs = m_msh->PT;		
		//	rw_pcs->AdvanceParticlesLaBolle(dt); 	
		//rw_pcs->AdvanceByAdvection(dt); 
//		rw_pcs->AdvanceByAdvectionNDispersion(dt);
		rw_pcs->AdvanceByAdvectionNDispersionSplitTime(dt, 20);
		rw_pcs->SetElementBasedConcentration();  
#endif
	  }
#ifdef OLD
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if(T_Process) //WW
	  {
        double dt_pcs;
        CTimeDiscretization *m_tim = NULL;
        m_pcs = PCSGet("HEAT_TRANSPORT");
        if(m_pcs)
        {
          m_tim = TIMGet("HEAT_TRANSPORT");
          if(m_tim)
          {
            dt_pcs = m_tim->time_step_vector[0];
            if(*dt_sum>=dt_pcs)
            {
              m_pcs->Execute();
              if(!MASS_TRANSPORT_Process) //MX
                *dt_sum = 0.0;
            }
          }
          else
          {
            cout << "Error in LOPTimeLoop_PCS: no TIM data" << endl;
          }
        }
	  }
#endif
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if(MASS_TRANSPORT_Process) //WW
	  {
        //----------------------------------------------------------------------
        //SB:GS4   Mass transport processes
        // Calculate conservative transport
        double dt_pcs;
        CTimeDiscretization *m_tim = NULL;
        m_pcs = PCSGet("MASS_TRANSPORT");
        if(m_pcs){
          m_tim = TIMGet("MASS_TRANSPORT");
          if(m_tim){
            //WW/OK  if(m_tim->time_control_name.compare("COURANT_MANIPULATE")==0) m_tim->CheckCourant();
            //WW/OK  if (aktueller_zeitschritt == 1) m_tim->CheckCourant();//CMCD 03/2006
            dt_pcs = m_tim->time_step_vector[0];
            if(*dt_sum>=dt_pcs){
              for(i=0;i<no_processes;i++){
                m_pcs = pcs_vector[i];
                if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0)
	              if(CPGetMobil(m_pcs->GetProcessComponentNumber())> 0) //Component Mobile ? 
		                	m_pcs->Execute();
              }
              // Calculate Chemical reactions, after convergence of flow and transport 
              // Move inside iteration loop if couplingwith transport is implemented SB:todo
              //SB:todo move into Execute Reactions	  if((aktueller_zeitschritt % 1) == 0)  
			  //REACT *rc = NULL; //OK
			  //rc = REACT_vec[0]; //OK
//				if(rc->flag_pqc) rc->ExecuteReactions();
//				delete rc;
              if(REACT_vec.size()>0) //OK
	   		    if(REACT_vec[0]->flag_pqc){ 
                    #ifdef REACTION_ELEMENT
                      REACT_vec[0]->ExecuteReactionsPHREEQC0();
                    #else
                      REACT_vec[0]->ExecuteReactions();
                    #endif
                }
#ifdef CHEMAPP
              if(Eqlink_vec.size()>0) 
                Eqlink_vec[0]->ExecuteEQLINK();
#endif
	         *dt_sum = 0.0;
            }
          }
          else
            cout << "Error in LOPTimeLoop_PCS: no time discretization" << endl;
       }
    }
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      //if(H_Process&&aktueller_zeitschritt==1) break;
      // Deformation process
      for(i=0;i<no_processes;i++)
      {
        m_pcs = pcs_vector[i];
        if(m_pcs->num_type_name.find("EXCAVATION")!=string::npos)
          continue;
 	    if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
        {
          dm_pcs = (CRFProcessDeformation *)(m_pcs);
          pcs_dm_error=dm_pcs->Execute(k);
          //Error
          if(k==0) pcs_dm_cp_error = 1.0;
          else
             pcs_dm_cp_error = fabs(pcs_dm_error-pcs_dm_error0)/pcs_dm_error0;
          pcs_dm_error0 = pcs_dm_error;
          pcs_flow_error = max(pcs_flow_error, pcs_dm_cp_error);
        }
      }
      //if(!H_Process) break;
      if(k>0)
      {
        if(pcs_flow_error<TolCoupledF)
        //    ||pcs_flow_error/pcs_flow_error0<TolCoupledF)
        break;
      }
      if(H_Process&&M_Process&&k>0) 
        cout << "\t    P-U coupling iteration: " << k 
             <<" Error: " <<pcs_flow_error<<endl;
  } // coupling iterations
  //======================================================================
  // Extropolate the Gauss values to element nodes for deformation process
  //----------------------------------------------------------------------
  if(dm_pcs)
  {
    if(H_Process&&dm_pcs->type!=41) // HM partitioned scheme
       dm_pcs->ResetTimeStep(); 
     dm_pcs->Extropolation_GaussValue();
  }
  //----------------------------------------------------------------------
  //  Update the results
  for(i=0;i<no_processes;i++)
  {
    m_pcs = pcs_vector[i];
    m_pcs->WriteSolution();
    if(m_pcs->m_msh) // MSH
    { 
      m_pcs->CopyTimestepNODValues(); //MB
#define SWELLING
#ifdef SWELLING
		for(j=0;j<m_pcs->pcs_number_of_evals;j++){  //MX ToDo//CMCD here is a bug in j=7
          nidx0 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j]);
          nidx1 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j])+1;
		  for(long l=0;l<(long)m_pcs->m_msh->ele_vector.size();l++)
             m_pcs->SetElementValue(l,nidx0, m_pcs->GetElementValue(l,nidx1));
		}
#endif
    }
    else
    {
      for(j=0;j<m_pcs->pcs_number_of_primary_nvals;j++)
      {
        timelevel=0;
        nidx0 = PCSGetNODValueIndex(m_pcs->pcs_primary_function_name[j],timelevel);
        timelevel=1;
        nidx1 = PCSGetNODValueIndex(m_pcs->pcs_primary_function_name[j],timelevel);
        CopyNodeVals(nidx1,nidx0);
      }
    }
  }
  //----------------------------------------------------------------------
  LOPCalcELEResultants();
  cout << "Calculation of NOD resultants" << endl;
  LOPCalcNODResultants(); //OK
  //----------------------------------------------------------------------
  return 1;
}
#ifdef LOOP_TO_DO
/*
  //WW  LOPCalcElementResultants1();
  if (GetSoilPorosityModel(0) == 3) { 
    for(phase=0;phase<GetRFProcessNumPhases();phase++) {
      if (phase==1){
        sprintf(pcs_name,"%s%d","COMP_FLOW",2);
        processes0 = processes0->Get(pcs_name);
        processes0->LOPCopySwellingStrain(processes0);}  //MX
      }
  }
*/
#endif

/**************************************************************************
ROCKFLOW - Function: LOPPostTimeLoop_PCS
Task: 
Programing:
 02/2003 OK Implementation
last modified:
***************************************************************************/
int LOPPostTimeLoop_PCS(void)
{
  PCSDestroyAllProcesses();
  return 1;
}

/**************************************************************************
ROCKFLOW - Function: SetCriticalDepthSourceTerms
Task: 
Programing:
 12/2004 MB Implementation
 03/2006 WW 
last modified:
***************************************************************************/
void SetCriticalDepthSourceTerms(void)
{
  double value = 0;
  //  int interp_method=0;
  //  int valid=0;

  long msh_node;
  long i; //, j;
  double valueAdd;
  double H=0.0;
  double Haverage = 0.0;
  double AnzNodes = 0.0;

//  int test;
  //-----------------------s------------------------------------------------
  CRFProcess *m_pcs = NULL;
  m_pcs = pcs_vector[0];
  CNodeValue *cnodev = NULL; //WW
  // 
  //-----------------------------------------------------------------------
  long group_vector_length = (int)m_pcs->st_node_value.size();
  for(i=0;i<group_vector_length;i++) {
    cnodev = m_pcs->st_node_value[i]; //WW
    if (cnodev->conditional == 1){
      msh_node = cnodev->msh_node_number;
	  Haverage += GetNodeVal(msh_node,1);
      AnzNodes += 1;
     }
  }   
  Haverage = Haverage / AnzNodes;

  for(i=0;i<group_vector_length;i++) {
    cnodev = m_pcs->st_node_value[i]; //WW
    //WW  msh_node = m_st_group->group_vector[i]->msh_node_number;
    msh_node = cnodev->msh_node_number;
    if(msh_node>=0) {
        value = cnodev->node_value;
        
        //double MobileDepth = 0.001;
        double MobileDepth = 0.0;
        double NodeWidth = 100.0;
        double factor = 2;
        
        if (cnodev->conditional == 1){
        //H = GetNodeVal(msh_node,1);
          H = Haverage;
          H = H - MobileDepth;
          if (H < 0.0)  {H = 0.0;}      
          double H3 = pow(H,3);
          valueAdd = - 1 * sqrt(9.81 * H3);
          if (msh_node == 1 || msh_node == 2) NodeWidth = NodeWidth / 2.0;
          valueAdd = valueAdd * NodeWidth * factor;
          //value += valueAdd;
          value = valueAdd;
          cnodev->node_value = value;
          printf("\n Node %ld: Depth Hmobile valueAdd %e %e %e ", msh_node, GetNodeVal(msh_node,1), H, valueAdd);

          // rausschreiben der Flux Werte, Achtung, 
          SetNodeVal(msh_node,PCSGetNODValueIndex("FLUX",1),value);

        } // end if conditional == 1
      } // end if msh_node>=0
    } // end for group_vector_length
}

/**************************************************************************
ROCKFLOW - Function: 
Task: Calculate node fluxes
Programming: 
11/2002 MB/OK Implementation
03/2004 OK/CMcD extension for PCS
last modified:
**************************************************************************/
/*
void LOPCalcNodeFluxes_PCS(void) //CMCD altered alot!!
{
 long index;
 int i;
 // Zeitdiskretisierung
 static double dt_inverse,theta;
// Element-Matrizen 
 static double *capacitance_matrix;
 static double *conductance_matrix;
// static double *coupling_matrix_x;
// static double *coupling_matrix_y;
 static double left_matrix[64];
 static double right_matrix[64];
 static double *gravity_vector;
   // Knoten-Daten
 static long nn,nn2;
 long *element_nodes;
   // Materialdaten
 static double k_rel_iteration;
 static double pressure[8];
 double flux[8],mflux[8];
 double fluxold[8],fluxnew[8];
 double pressurenew[8],pressureold[8];
 static double gp[3]; 

 int phase=0; int timelevel=1;
 double g, rho;
 double k_rel,temp;
 temp=0.;
 CRFProcess *m_pcs = NULL;

 // Time discretization
 dt_inverse = 1.0 / dt;
 theta = GetNumericalTimeCollocation("PRESSURE"); // ToDo auf NUMERICS umstellen 

 // Initialize NodeFlux from previous time steps
 // Important as contributions of elements are added with
 // flux[i] += MODGetNodeFlux_MB(phase,element_nodes[i],-1,timelevel
 // LOPInitFluxes_MB();
 for (index=0;index<NodeListLength;index++) {
   SetNodeVal(index,m_pcs->PCSGetNODValueIndexNew("VOLUME_FLUX1",timelevel),0.0);
   SetNodeVal(index,m_pcs->PCSGetNODValueIndexNew("MASS_FLUX1",timelevel),0.0);
 }
 //=============================================================================
 for (index=0;index<ElListSize();index++) {
     if (ElGetElement(index)!=NULL) { //Element existiert
     // Parameter fuer die Ortsdiskretisierung 
     nn = ElNumberOfNodes[ElGetElementType(index)-1];
     nn2 = nn*nn;
     element_nodes = ElGetElementNodes(index);
      //-------------------------------------------------------------------------
     // Elementmatrizen und -vektoren bereitstellen oder berechnen 
     if(memory_opt==0) { // gespeicherte Element-Matrizen holen 
       capacitance_matrix = ASMGetElementCapacitanceMatrix(index);
       conductance_matrix = ASMGetElementConductanceMatrix(index);
       gravity_vector = ASMGetElementGravityVector(index);
     }
     else if (memory_opt==1) { // jede Element-Matrix berechnen 
       DisplayMsgLn("LOPCalcNodeFluxes_MB: Speichermodell"); 
       abort();
     }
     else if (memory_opt==2) { // Element-Prototypen benutzen
       DisplayMsgLn("LOPCalcNodeFluxes_MB: Speichermodell"); 
       abort();
     }
     k_rel_iteration = 1.0; // CalcNonlinearFlowPermeability1(index,ASMGetNodePress1,NULL);
//k_0
     k_rel = MATCalcRelativePermeability(phase, index, 0.,0.,0.,theta); //CMCD 03 2004
     //-------------------------------------------------------------------------
     // LHS matrix: [C]/dt + theta [K] k_rel 
     for(i=0;i<nn2;i++) {
       left_matrix[i] = dt_inverse * capacitance_matrix[i] + \
                        theta * k_rel_iteration*k_rel * conductance_matrix[i]; //CMCD 03 2004
     }
     //-------------------------------------------------------------------------
     g = gravity_constant;
         gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
     rho = MATCalcFluidDensity(phase,index,gp,theta,0,NULL);

     timelevel = 1;
     for (i=0;i<nn;i++) {
       pressurenew[i] = GetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("PRESSURE1",timelevel));
     }
     MMultMatVec(left_matrix,nn,nn,pressurenew,nn,fluxnew,nn);
    // RHS matrix: CMCD 03 2004
    timelevel = 0;
    for(i=0;i<nn2;i++){
            right_matrix[i] = dt_inverse * capacitance_matrix[i] - \
                              (1.0 - theta) * k_rel_iteration*k_rel * conductance_matrix[i];
    }
    for (i=0;i<nn;i++) {
       pressureold[i] = GetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("PRESSURE1",timelevel));
    }
    MMultMatVec(right_matrix,nn,nn,pressureold,nn,fluxold,nn);
    for(i=0;i<nn;i++){
            flux[i]=fluxnew[i]-fluxold[i];
			mflux[i]=flux[i]*rho;
    }
    // Stop at a certain element
    if (index == 4516 ){
    temp = temp;
    }
     //-------------------------------------------------------------------------
     // Sum fluxes for nodes 
     // Contributions from different elements are added to calculate the node flux 
    timelevel = 1; 
    for (i=0;i<nn;i++) {
        flux[i] += GetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("VOLUME_FLUX1",timelevel));
		mflux[i] += GetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("MASS_FLUX1",timelevel));
       //flux[i] += MODGetNodeFlux_MB(phase,element_nodes[i],-1,timelevel);
       if(fabs(flux[i])<MKleinsteZahl) flux[i]=0.0;
       //MODSetNodeFlux_MB(phase,element_nodes[i],timelevel,flux[i]);
       SetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("VOLUME_FLUX1",timelevel),flux[i]);
	   if(fabs(mflux[i])<MKleinsteZahl) mflux[i]=0.0;
	   SetNodeVal(element_nodes[i],m_pcs->PCSGetNODValueIndexNew("MASS_FLUX1",timelevel),mflux[i]);
     }
   } // endif element exists 
 } // endfor element loop
}
*/

/**************************************************************************
ROCKFLOW - Function: Checkcourant
Task: Identify the critical courant numbers and plausible time steps
A bit quick and dirty richt now!! )
CMCD 7/2004 Implemented
**************************************************************************/
double Checkcourant_PCS() 
{
	//Pointer to the elements CMCD
	Element *elem = NULL; 
	int index;
	int elementnumber;
	double smallesttimestep;
	smallesttimestep = 1000000000.0;
	elementnumber = -1;
	for (index=0;index<ElListSize();index++){
		elem = ElGetElement(index);
		if (smallesttimestep > elem->TimestepCourant){
			smallesttimestep = elem->TimestepCourant;
			elementnumber = index;
		}
	}
	cout << "Critical Element: " << elementnumber<< endl;
	return smallesttimestep;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2003 SB Implementation
01/2004 MX k_eff 
11/2004 OK fluid mass fluxes
last modification:
**************************************************************************/
void LOPCalcELEResultants(void)
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
#ifdef MFC
        AfxMessageBox("Error in LOPCalcELEResultants: no valid process !");
#endif
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

/**************************************************************************/
/* ROCKFLOW - Funktion: PCSCalcSecondaryVariables
                                                                          */
/* Aufgabe:
   Berechung von secondary variables w?rend der Zeitschleife
   Abfrage je nach Prozess, der aktiv ist
                                                                          */
/* Programmaenderungen:
   08/2003   SB   Implementation
   01/2006   YD   add dual porosity                                                                          */
/**************************************************************************/
void PCSCalcSecondaryVariables(void){

  int i, ptype;
  CRFProcess *m_pcs=NULL;
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
      MPCCalcSecondaryVariables();
      break;
    case 13: /* Non-isothermal flow process */
      MPCCalcSecondaryVariablesRichards();
      break;
    case 2: /* Mass transport process */
      MTM2CalcSecondaryVariables();
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
      MMPCalcSecondaryVariables();
      break;
	default:
		//DisplayMsgLn(" Error: Unknown PCS type in PCSCalcSecondaryVariables");
		break;
	}
	} //If
 } // while
}



/**************************************************************************
FEMLib-Method: 
02/2005 OK Implementation
08/2005 WW Changes due to geometry objects applied
08/2005 MB Changes ... (OK to what ?)
04/2006 OK and once again ...
**************************************************************************/
void LOPExecuteRegionalRichardsFlow(CRFProcess*m_pcs_global)
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
  int idxcp = m_pcs_global->GetNodeValueIndex("PRESSURE_CAP") + timelevel;
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
    m_msh_local->ConstructGrid(false);
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
      value = m_pcs_global->GetNodeValue(i,idxcp);
      m_pcs_global->SetNodeValue(i,idxcp-1,value);
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
        g_node_number = j+(i*no_local_nodes);
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
    m_pcs_local->ExecuteNonLinear();
    //....................................................................
    // Store results in global PCS tables
    for(j=0;j<no_local_nodes;j++)
    {
      g_node_number = j+(i*no_local_nodes);
      value = m_pcs_local->GetNodeValue(j,idxp);
      m_pcs_global->SetNodeValue(g_node_number,idxp,value);
      value = m_pcs_local->GetNodeValue(j,idxcp);
      m_pcs_global->SetNodeValue(g_node_number,idxcp,value);
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
GeoSys-Method:
Task: 
Programing:
02/2005 OK Implementation
ToDo: > msh_nod
**************************************************************************/
/*
void CFEMesh::SetActiveNodes()
{
  int j;
  long i,k;
  vector<long>active_nodes_vector;
  //-----------------------------------------------------------------------
  // Fill active_nodes_vector
  for(i=0;i<(long)ele_vector.size();i++){
    m_ele = ele_vector[i];
    if(m_ele->selected){
      for(j=0;j<m_ele->nnodes;j++){
        active_nodes_vector.push_back(m_ele->nodes_index[j]);
      }
    }
  }
  long no_active_nodes = (long)active_nodes_vector.size();
  //-----------------------------------------------------------------------
  // Inactivate nodes
  //.......................................................................
  for(i=0;i<(long)nod_vector.size();i++){
    nod_vector[i]->selected = false;
  }
  //.......................................................................
  // Activate nodes and set node index
  k=0;
  CMSHNodes*m_nod = NULL;
  for(i=0;i<no_active_nodes;i++){
    m_nod = nod_vector[active_nodes_vector[i]];
    if(m_nod->selected){
      continue;
    }
    else{
      m_nod->selected = true;
      m_nod->eqs_index = k;
      k++;
    }
  }
  //-----------------------------------------------------------------------
  active_nodes_vector.clear();
}
*/

/**************************************************************************
FEMLib-Method: 
01/2006 OK Implementation
**************************************************************************/
void LOPCalcNODResultants(void)
{
  CRFProcess* m_pcs = NULL;
  for(int p=0;p<(int)pcs_vector.size();p++){
    m_pcs = pcs_vector[p];
    if(!m_pcs->selected) //OK4108
      continue;
    switch(m_pcs->pcs_type_name[0]){
      default:
#ifdef MFC
        AfxMessageBox("Error in LOPCalcNODResultants: no valid process !");
#endif
        break;
      case 'L': // Liquid flow
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'G': // Groundwater flow
        m_pcs->AssembleParabolicEquationRHSVector();
        //m_pcs->GlobalAssembly();
        m_pcs->SetNODFlux();
        break;
      case 'A': // Gas flow
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'T': // Two-phase flow
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'C': // Componental flow
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'H': // Heat transport
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'M': // Mass transport
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'D': // Deformation
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'O': // Overland flow
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
      case 'R': // Richards flow
        // m_pcs->AssembleParabolicEquationRHSVector();
        // m_pcs->SetNODFlux();
        break;
	  case 'F': // Fluid Momentum
        cout << "LOPCalcNODResultants: not implemented for this process" << endl;
        break;
    }
  }
}
/**************************************************************************
Task: 
Programing:PCSStorage
   06/2006   YD   Implementation                                                                         */
/**************************************************************************/
void PCSStorage(void){
  CRFProcess* m_pcs = NULL;
  for(int p=0;p<(int)pcs_vector.size();p++){
    m_pcs = pcs_vector[p];
    if(!m_pcs->selected) //OK4108
      continue;
    switch(m_pcs->pcs_type_name[0]){
      default:
        break;
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
        m_pcs->PrimaryVariableStorageRichards();
        break;
    }
  }
}

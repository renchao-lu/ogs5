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
last modified:
***************************************************************************/
/* Includes */
/*========================================================================*/
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

//#define EXCAVATION    

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
        rc->CreateREACT();//SB
        rc->InitREACT();
        rc->ExecuteReactions();
	    REACT_vec.clear();
	    REACT_vec.push_back(rc);
      }
    }
  }
//  delete rc;
  //----------------------------------------------------------------------
  // DDC
  DOMCreate();
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
  for(k=0;k<lop_coupling_iterations;k++){
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
      //-------------------------------------------------------------------
      m_pcs = PCSGet("GROUNDWATER_FLOW");
      if(m_pcs&&m_pcs->selected){
        lop_coupling_iterations = m_pcs->m_num->cpl_iterations;
        if(lop_coupling_iterations > 1){
          m_pcs->CopyCouplingNODValues();
          TolCoupledF = pcs_vector[1]->m_num->cpl_tolerance;
        }
        pcs_flow_error =  m_pcs->ExecuteNonLinear();
        if(lop_coupling_iterations > 1){ 
          pcs_coupling_error = m_pcs->CalcCouplingNODError();
        }
        PCSCalcSecondaryVariables(); // PCS member function

        if (CalcVelocities){ 
        m_pcs->CalIntegrationPointValue(); //WW
        }
			  if(m_pcs->tim_type_name.compare("STEADY")==0){//CMCD 05/2006
          LOPCalcNODResultants();
          LOPCalcELEResultants();
				  m_pcs->selected = false;
        }
      }
      //------------------------------------------------------------------
      m_pcs = PCSGet("RICHARDS_FLOW");
      if(m_pcs&&m_pcs->selected){
        if(m_pcs->m_msh->no_msh_layer==0){
          pcs_flow_error = m_pcs->ExecuteNonLinear();
          // ToDo: For Regional Richards Flow with more than one m_pcs and m_msh 
          // for(i=0;i<no_processes;i++){
          //   m_pcs = pcs_vector[i];
          //   pcs_flow_error = m_pcs->ExecuteNonLinear();
          // }
        }
        else{
          //For Regional Richards Flow with just one m_pcs and m_msh 
          //(as used in Benchmark COUPLED_FLOW) 
          //LOPExecuteRegionalRichardsFlow(m_pcs);
          m_pcs->CalcFluxesForCoupling();
        }
        if (CalcVelocities){ 
         m_pcs->CalIntegrationPointValue(); //WW
        }
      }
	  //--------------------------------------------------------------------
/*
#ifdef _FEMPCHDEBUG_
	// PCH Let's monitor what's going on in the FEM
	// This messagebox is for debugging the primary variables at every time step.
	// Should combine with the picking...
	CWnd * pWnd = NULL;
	pWnd->MessageBox("Richard's Flow is just solved!!!","Debug help", MB_ICONINFORMATION);
#endif
*/
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
      }
      //--------------------------------------------------------------------     
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
      //--------------------------------------------------------------------
      if(k==0) pcs_flow_error0 = pcs_flow_error;

	m_pcs = PCSGet("FLUID_MOMENTUM");
    if(m_pcs&&m_pcs->selected){
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
	CWnd * pWnd = NULL;
	pWnd->MessageBox("Velocity is just solved!!!","Debug help", MB_ICONINFORMATION);
#endif
    //----------------------------------------------------------------------
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
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(T_Process) //WW
	{
       // Heat transport process
       double dt_pcs;
       CTimeDiscretization *m_tim = NULL;
       m_pcs = PCSGet("HEAT_TRANSPORT");
       if(m_pcs){
         m_tim = TIMGet("HEAT_TRANSPORT");
         if(m_tim){
           dt_pcs = m_tim->time_step_vector[0];
           if(*dt_sum>=dt_pcs){
              m_pcs->Execute();
           if(!MASS_TRANSPORT_Process) //MX
             *dt_sum = 0.0;
           }
         }
         else{
           cout << "Error in LOPTimeLoop_PCS: no TIM data" << endl;
         }
       }
	}
#endif
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
          if (aktueller_zeitschritt == 1) m_tim->CheckCourant();//CMCD 03/2006
          if(m_tim){
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
	   		    if(REACT_vec[0]->flag_pqc) REACT_vec[0]->ExecuteReactions();
	         *dt_sum = 0.0;
            }
          }
          else
              cout << "Error in LOPTimeLoop_PCS: no time discretization" << endl;
       }
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //if(H_Process&&aktueller_zeitschritt==1) break;
    // Deformation process
    for(i=0;i<no_processes;i++){
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

  //
  // Extropolate the Gauss values to element nodes for deformation process
  if(dm_pcs)
  {
      if(H_Process&&dm_pcs->type!=41) // HM partitioned scheme
         dm_pcs->ResetTimeStep(); 
      dm_pcs->Extropolation_GaussValue();
  }
  // 
  //  
  //  Update the results
  for(i=0;i<no_processes;i++){
     m_pcs = pcs_vector[i];
     if(m_pcs->m_msh){ // MSH
        m_pcs->CopyTimestepNODValues(); //MB
#define SWELLING
#ifdef SWELLING
		for(j=7;j<m_pcs->pcs_number_of_evals;j++){  //MX ToDo//CMCD here is a bug in j=7
          nidx0 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j]);
          nidx1 =  m_pcs->GetElementValueIndex(m_pcs->pcs_eval_name[j])+1;
		  for(long l=0;l<(long)m_pcs->m_msh->ele_vector.size();l++)
             m_pcs->SetElementValue(l,nidx0, m_pcs->GetElementValue(l,nidx1));
		}
#endif
     }
     else{
        for(j=0;j<m_pcs->pcs_number_of_primary_nvals;j++){
          timelevel=0;
          nidx0 = PCSGetNODValueIndex(m_pcs->pcs_primary_function_name[j],timelevel);
          timelevel=1;
          nidx1 = PCSGetNODValueIndex(m_pcs->pcs_primary_function_name[j],timelevel);
          CopyNodeVals(nidx1,nidx0);
        }
     }
  }
  //

  //----------------------------------------------------------------------
  LOPCalcELEResultants();
  cout << "Calculation of NOD resultants" << endl;
  LOPCalcNODResultants(); //OK
  //----------------------------------------------------------------------

//DECOVALEX TEST
//#define  EXCAVATION  
#ifdef EXCAVATION    
   //TEST for DECOVALEX
   string StressFileName = FileName+".pat";
   fstream file_pat (StressFileName.data(),ios::trunc|ios::out);   
   string deli = " ";
  // 
  int idx0, idx1, idx00, idx11;
  double val0, val1;
  idx0 = GetNodeValueIndex("TEMPERATURE1");
  idx1 = idx0+1;
  idx00 = GetNodeValueIndex("PRESSURE1");
  idx11 = idx00+1;

  for(i=0; i<m_pcs->m_msh->NodesInUsage(); i++)
  {
	   val0 = GetNodeValue(i,idx0); 
	   val1 = GetNodeValue(i,idx1);
       if(fabs(val0)<1.0e-9) val0 =  25.0; //THM1 // 23.6; //THM2//
       if(fabs(val1)<1.0e-9) val1 =  25.0; //THM1 // 23.6; //THM2 //
       file_pat<<val0<<deli<<val1<<endl;
	   val0 = GetNodeValue(i,idx00); 
	   val1 = GetNodeValue(i,idx11);
       if(fabs(val0)<1.0e-9) val0 = 1.0e5; //THM1 //0.0; //THM2 //
       if(fabs(val1)<1.0e-9) val1 = 1.0e5;  //THM1 //0.0; //THM2 //
       file_pat<<val0<<deli<<val1<<endl;
   }    

   file_pat.close();
#endif
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
        break;
      case 'G': // Groundwater flow
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
        break;
      case 'R': // Richards flow
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
Task: 
Programing:
02/2005 OK Implementation
08/2005 WW Changes due to geometry objects applied
08/2005 MB Changes .....
last modification:
**************************************************************************/
void LOPExecuteRegionalRichardsFlow(CRFProcess*m_pcs)
{
  int j;
  long i;
  vector<long>sfc_lines_vector;
  int no_layer_lines = m_pcs->m_msh->no_msh_layer;
  long no_richards_problems = (long)(m_pcs->m_msh->ele_vector.size()/no_layer_lines);

  if(aktueller_zeitschritt==1){
    m_pcs->eqs = DestroyLinearSolver(m_pcs->eqs);
	//pcs->eqs = CreateLinearSolver(pcs->m_num->ls_storage_method, pcs->m_msh->NodesInUsage());
    m_pcs->eqs = CreateLinearSolver(m_pcs->m_num->ls_storage_method, no_layer_lines+1);
    InitializeLinearSolver(m_pcs->eqs,m_pcs->m_num);
  }
  
  for(i=0;i<no_richards_problems;i++){
    
    cout<< "" << endl;
    cout<< "Richard Problem Number: " << i << endl;
    
    // Select line elements related to surface element
    for(j=0;j<no_layer_lines;j++){
      sfc_lines_vector.push_back(j+(i*no_layer_lines));
    }
    /*for(j=0;j<(long) sfc_lines_vector.size();j++){  
      cout<< " " << j << "  " << sfc_lines_vector[j] <<  endl;
    }*/

    m_pcs->m_msh->SetMSHPart(sfc_lines_vector, i);

    //Assign small mesh
    m_pcs->m_msh = FEMGet("MSH_Strang");

    m_pcs->ExecuteNonLinear();

    //Assign big mesh again
    m_pcs->m_msh = FEMGet("RICHARDS_FLOW");


    // pcs->m_msh->RenumberNodesForGlobalAssembly(); related to Comment 1  // MB/WW
    sfc_lines_vector.clear();
  }
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


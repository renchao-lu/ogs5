/*
   The members of class Element definitions.
*/
// MFC
#include "stdafx.h"
// C++ STL
#include <iostream>
// Method
#include "fem_ele_std.h"
#include "mathlib.h"
// Steps
#include "rf_pcs.h"
#include "rf_tim_new.h"
// Sytem matrix
#include "matrix.h"
// Geometry
//#include "elements.h"
#include "nodes.h"
#include "adaptiv.h"
// Parallel computing
#include "par_ddc.h"
// MSHLib
#include "msh_elem.h"

#include "pcs_dm.h" // displacement coupled
extern double gravity_constant;// TEST, must be put in input file

#define GAS_CONSTANT_V  461.5  //WW

#include "rfmat_cp.h"
namespace FiniteElement{

//========================================================================
// Element calculation
//========================================================================
CFiniteElementStd:: CFiniteElementStd(CRFProcess *Pcs, const int C_Sys_Flad, const int order)
                   : CElement(C_Sys_Flad, order), phase(0), comp(0), SolidProp(NULL), 
                      FluidProp(NULL), MediaProp(NULL), 
                     pcs(Pcs), dm_pcs(NULL), HEAD_Flag(false)
{
    int i;
	string name2;
	char name1[MAX_ZEILE];
    char pcsT;
    cpl_pcs=NULL; 
    CRFProcess *m_pcs=NULL;  //MX

    GravityMatrix = NULL;
    dynamic = false;
    if(pcs->pcs_type_name.find("DYNAMIC")!=string::npos)
      dynamic = true;

	dm_pcs =NULL;
    heat_phase_change=false;

    idx_vel_disp[0] = idx_vel_disp[1] = idx_vel_disp[2] = -1;
    idx_pres = -1; 

    idxS=idx3=-1;
    if(pcs->primary_variable_name.compare("HEAD")==0)
        HEAD_Flag = true;
//SB4218 added  
	string pcs_primary = pcs->pcs_primary_function_name[0];
	if(pcs_primary.compare("HEAD")==0)
        HEAD_Flag = true;    
    for(i=0; i<9; i++) mat[i] = 0.0;

    idx0 = idx1 = 0; // column index in the node value data
	  LocalShift = 0; 
    pcsT = pcs->pcs_type_name[0];
    if(pcs->pcs_type_name.find("GAS")!=string::npos)
       pcsT = 'A';
    if(pcs->pcs_type_name.find("DUAL_RICHARDS")!=string::npos)
       pcsT = 'B';
    switch(pcsT){
      default:
        PcsType = L;
        GravityMatrix = new  SymMatrix(20);
        idx0 = pcs->GetNodeValueIndex("PRESSURE1");
        idx1 = idx0+1;
        break;
      case 'L': // Liquid flow
        PcsType = L;
        GravityMatrix = new  SymMatrix(20);
        if(dynamic)
        {
           idx0 = pcs->GetNodeValueIndex("PRESSURE_RATE1");
           idx1 = idx0+1;
           idx_pres = pcs->GetNodeValueIndex("PRESSURE1");
           idx_vel_disp[0] = pcs->GetNodeValueIndex("VELOCITY_X1");
           idx_vel_disp[1] = pcs->GetNodeValueIndex("VELOCITY_Y1");
           if(dim==3)
              idx_vel_disp[2] = pcs->GetNodeValueIndex("VELOCITY_Z1");
        }
        else
        {
           idx0 = pcs->GetNodeValueIndex("PRESSURE1");
           idx1 = idx0+1;
        } 
        break;
      case 'U': // Unconfined flow
        PcsType = U;
        break;
      case 'G': // Groundwater flow
        PcsType = G;
        idx0 = pcs->GetNodeValueIndex("HEAD");
        idx1 = idx0+1;
        break;
      case 'T': // Two-phase flow
        PcsType = T;
        break;
      case 'C': // Componental flow
        PcsType = C;
        break;
      case 'H': // heat transport
        PcsType = H;
        idx0 = pcs->GetNodeValueIndex("TEMPERATURE1");
        idx1 = idx0+1;
//SB4218        idxS = pcs->GetNodeValueIndex("SATURATION1")+1;
        for (i=0; i<(int) pcs_vector.size(); i++){  //MX
	      m_pcs = pcs_vector[i];
	      if(m_pcs->pcs_type_name.compare("RICHARDS_FLOW") == 0){
            idxS = m_pcs->GetNodeValueIndex("SATURATION1")+1; //MX
            break;
          }
        }
        break;
      case 'M': // Mass transport
        PcsType = M;
	   	sprintf(name1, "%s",pcs->pcs_primary_function_name[0]);
	    name2 = name1;
	    idx0 = pcs->GetNodeValueIndex(name2);
        idx1 = idx0+1;
        break;
      case 'O': // Liquid flow
        PcsType = O;
        break;
      case 'R': //OK4104 Richards flow
        GravityMatrix = new  SymMatrix(20);
        idx0 = pcs->GetNodeValueIndex("PRESSURE1");
        idx1 = idx0+1;
        idxS = pcs->GetNodeValueIndex("SATURATION1")+1;
        PcsType = R;
        break;
      case 'A': // Air (gas) flow
        PcsType = A;
        break;
      case 'B': //YD Dual Richards flow
        GravityMatrix = new  SymMatrix(20);
        idx0 = pcs->GetNodeValueIndex("PRESSURE_D");
        idx1 = idx0+1;
        idxS = pcs->GetNodeValueIndex("SATURATION_D")+1;
        PcsType = R;
	  case 'F':	// Fluid Momentum Process
		PcsType = R;	// R should include L if the eqn of R is written right.
        break;
    }

    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
      Mass = new SymMatrix(20);
      Laplace = new Matrix(20,20);
	  Advection = new Matrix(20,20);
	  Storage = new Matrix(20,20);
	  Content = new Matrix(20,20);
      if(D_Flag) 
          StrainCoupling = new Matrix(20,60);
      else StrainCoupling = NULL;
      RHS = new Vec(20);
    }
    else   // Local matrices stored and allocated the pcs.cpp;
    {
       Mass = NULL;
       Laplace = NULL;
	   Advection = NULL;
	   Storage = NULL;
	   Content = NULL;
       StrainCoupling = NULL;
       RHS = NULL;
    }

    StiffMatrix = new Matrix(20, 20);
    AuxMatrix = new Matrix(20, 20);
    AuxMatrix1 = new Matrix(20, 20);

   	time_unit_factor = pcs->time_unit_factor;
    for(i=0; i<4; i++) NodeShift[i] = 0;
    check_matrices = true;
}

// Destructor
CFiniteElementStd::~CFiniteElementStd()
{
    if(GravityMatrix) delete GravityMatrix;
    GravityMatrix = NULL;
 
    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
       if(Mass) delete Mass;
       if(Laplace) delete Laplace;
	   if(Advection) delete Advection;
	   if(Storage) delete Storage;
	   if(Content) delete Content;
       if(StrainCoupling) delete StrainCoupling;
       if(RHS) delete RHS;
       Mass = NULL;
       Laplace = NULL;
	   Advection = NULL;
	   Storage = NULL;
	   Content = NULL;
       StrainCoupling = NULL;
       RHS = NULL;
    }

    delete StiffMatrix;
    delete AuxMatrix;
    delete AuxMatrix1;

    StiffMatrix = NULL;
    AuxMatrix = NULL;
    AuxMatrix1 = NULL;
}



/**************************************************************************
   GeoSys - Function: SetMemory

   Aufgabe:
         Set memory for local matrices
   Programmaenderungen:
   01/2005   WW    Erste Version
   
**************************************************************************/
void CFiniteElementStd::SetMemory()
{
    int Size=0;
    ElementMatrix * EleMat = NULL;
    // Prepare local matrices
    // If local matrices are not stored, resize the matrix
    if(pcs->Memory_Type==0) 
    {
       Mass->LimitSize(nnodes);
       Laplace->LimitSize(nnodes, nnodes);
	   Advection->LimitSize(nnodes, nnodes); //SB4200
	   Storage->LimitSize(nnodes, nnodes); //SB4200
	   Content->LimitSize(nnodes, nnodes); //SB4209
       if(D_Flag>0) 
          StrainCoupling->LimitSize(nnodes, dim*nnodesHQ);
       Size = nnodes;
       RHS->LimitSize(nnodes);


    }
    else
    {
        EleMat = pcs->Ele_Matrices[Index];
        Mass = EleMat->GetMass();
        Laplace = EleMat->GetLaplace();
		// Advection, Storage, Content SB:Todo ?
        RHS = EleMat->GetRHS();
        if(D_Flag>0) 
           StrainCoupling = EleMat->GetCouplingMatrixB();
		if(D_Flag==41) LocalShift = dim*nnodesHQ; 
    }

    if(GravityMatrix) GravityMatrix->LimitSize(nnodes); 

    StiffMatrix->LimitSize(nnodes, nnodes);
    AuxMatrix->LimitSize(nnodes, nnodes);
    AuxMatrix1->LimitSize(nnodes, nnodes);
}


/**************************************************************************
   GeoSys - Function: ConfigureCoupling

   Aufgabe:
         Set coupling information for local fem calculation
   Programmaenderungen:
   01/2005   WW    Erste Version
   
**************************************************************************/
void  CFiniteElementStd::ConfigureCoupling(CRFProcess* pcs, const int *Shift, bool dyn)
{
  int i;  

  CRFProcess *pcs_D = PCSGet("DUAL_RICHARDS");
  char pcsT; 
  pcsT = pcs->pcs_type_name[0];
 if(pcs->pcs_type_name.find("GAS")!=string::npos)
    pcsT = 'A';



  if(D_Flag>0) 
  {      
     for(i=0;i<(int)pcs_vector.size();i++){
	   if(pcs_vector[i]->pcs_type_name.find("DEFORMATION")!=string::npos){
          dm_pcs = (CRFProcessDeformation*)pcs_vector[i];
          break;
       }
     }
     if(dyn)
     {        
        Idx_dm0[0] = dm_pcs->GetNodeValueIndex("ACCELERATION_X1");
        Idx_dm0[1] = dm_pcs->GetNodeValueIndex("ACCELERATION_Y1");
     }
     else
     {
       Idx_dm0[0] = dm_pcs->GetNodeValueIndex("DISPLACEMENT_X1");
       Idx_dm0[1] = dm_pcs->GetNodeValueIndex("DISPLACEMENT_Y1");
     }
     Idx_dm1[0] = Idx_dm0[0]+1;
     Idx_dm1[1] = Idx_dm0[1]+1; 
     //     if(problem_dimension_dm==3)
     if(dim==3)
     {
        if(dyn)       
          Idx_dm0[2] = dm_pcs->GetNodeValueIndex("ACCELERATION_Z1");
        else
          Idx_dm0[2] = dm_pcs->GetNodeValueIndex("DISPLACEMENT_Z1");
        Idx_dm1[2] = Idx_dm0[2]+1;
     }  

     for(i=0; i<4; i++)
         NodeShift[i] = Shift[i];
  }

  switch(pcsT){
    default:
      if(T_Flag)
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT");
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'L': // Liquid flow
      if(T_Flag)
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT");
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'U': // Unconfined flow
      break;
    case 'G': // Groundwater flow
      if(T_Flag)
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT");
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'T': // Two-phase flow
      break;
    case 'C': // Componental flow
      break;
    case 'H': // heat transport
      //SB CMCD this needs to be fixed
      cpl_pcs = PCSGet("GROUNDWATER_FLOW"); 
      if(cpl_pcs == NULL) 
        cpl_pcs = PCSGet("LIQUID_FLOW"); 
      if(cpl_pcs == NULL) 
        cpl_pcs = PCSGet("RICHARDS_FLOW"); //OK
      if (cpl_pcs){  //MX
      idx_c0 = cpl_pcs->GetNodeValueIndex("PRESSURE1");
      idx_c1 = idx_c0+1;
      }
      break;
    case 'M': // Mass transport
      if(T_Flag)
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT");
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'O': // Liquid flow
      break;
    case 'R': // Richards flow
      if(T_Flag) //if(PCSGet("HEAT_TRANSPORT"))
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT"); 
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      if(RD_Flag){
        idx_pd = pcs_D->GetNodeValueIndex("PRESSURE_D")+1;
      }
      break;
    case 'A': //Gas flow
      break;
    }
}


/*************************************************************************
FEMLib-Function: 
Task: Set material pointers to the current element
Programming: 
01/2005 WW Implementation
03/2005 OK MultiMSH
11/2005 YD Set cursor of gas
last modified:
**************************************************************************/
void CFiniteElementStd::SetMaterial(int phase)
{
  phase = 0;
  if((int)mmp_vector.size()<pcs->m_msh->max_mmp_groups + 1){//CC8888
#ifdef MFC
    AfxMessageBox("Not enough MMP groups");
#endif
    cout << "Not enough MMP groups" << endl;
    return;
  }
  //----------------------------------------------------------------------
  // MMP
  long group;
  group = MeshElement->GetPatchIndex();
  MediaProp = mmp_vector[group];
  MediaProp->m_pcs = pcs;
  MediaProp->Fem_Ele_Std = this;
  //----------------------------------------------------------------------
  // MSP
  if(msp_vector.size()>0) {
    SolidProp = msp_vector[group];
    SolidProp->Fem_Ele_Std = this;//CMCD for Decovalex
    }
  //----------------------------------------------------------------------
  // MFP
  if(PCSGet("LIQUID_FLOW")){
    FluidProp = MFPGet("LIQUID");
    if(!FluidProp)
      cout << "Warning ..." << endl;
  }
  if(mfp_vector.size()>0){
    FluidProp = mfp_vector[0];
    FluidProp->Fem_Ele_Std = this;
  }
  if(PCSGet("RICHARDS_FLOW")&&PCSGet("HEAT_TRANSPORT")){
    FluidProp = MFPGet("LIQUID");
    FluidProp->Fem_Ele_Std = this;
    //FluidProp = mfp_vector[0];
    GasProp = MFPGet("GAS");
    if (GasProp) GasProp->Fem_Ele_Std = this;
  }
  //----------------------------------------------------------------------
  // MCP
  //----------------------------------------------------------------------
}

////MB nur Zeitweise hier 
/*************************************************************************
FEMLib-Function: 
Task: 
Programming: 
**************************************************************************/
void CFiniteElementStd::Get_Matrix_Quad()
{
  if(MeshElement->geo_type==2){

    edlluse[0] = 0.5;
    edlluse[1] = -0.5;
    edlluse[2] = 0.0;
    edlluse[3] = 0.0;
    edlluse[4] = -0.5;
    edlluse[5] = 0.5;
    edlluse[6] = 0.;
    edlluse[7] = 0.;
    edlluse[8] = 0.;
    edlluse[9] = 0.;
    edlluse[10] = 0.5;
    edlluse[11] = -0.5;
    edlluse[12] = 0.;
    edlluse[13] = 0.;
    edlluse[14] = -0.5;
    edlluse[15] = 0.5;

    edttuse[0] = 0.5;
    edttuse[1] = 0.;
    edttuse[2] = 0.;
    edttuse[3] = -0.5;
    edttuse[4] = 0.;
    edttuse[5] = 0.5;
    edttuse[6] = -0.5;
    edttuse[7] = 0.;
    edttuse[8] = 0.;
    edttuse[9] = -0.5;
    edttuse[10] = 0.5;
    edttuse[11] = 0.;
    edttuse[12] = -0.5;
    edttuse[13] = 0.;
    edttuse[14] = 0.;
    edttuse[15] = 0.5;
  }

  if(MeshElement->geo_type==1){
   
    edlluse[0] = 1.0; 
    edlluse[1] = -1.0; 
    edlluse[2] = -1.0; 
    edlluse[3] = 1.0; 

    edttuse[0] = 0.0;
    edttuse[1] = 0.0;
    edttuse[2] = 0.0;
    edttuse[3] = 0.0;
  }
}

/**************************************************************************
FEMLib-Method: 
Task: Calculates consitutive relationships for Overland Flow -> swval, swold and krwval
Programing:
06/2005 MB Implementation
**************************************************************************/
void CFiniteElementStd::CalcNLTERMS(int nn, double* Haa, double* HaaOld, double power, double* swval, double* swold, double* krwval)
{
  int i; 
  double H;
  double zwu;
  double volume_depth;
  int channelswitch = 0;
  channelswitch = 0;

  if(MeshElement->geo_type==1){ // line
    channelswitch = 1;
    //channelswitch = 0;
  }
	
  //Compute_nlterms_olf
  for(i=0; i<nn; i++)  {
    H = Haa[i];
    
    //-----------------------------------------------
    //CHANNEL
    if (channelswitch == 1){
      // H subsituted with Rc
      // Rc = A / Pc
      double width;
      double wetted_area;
      double wetted_perimeter;

	  width = MediaProp->channel_width;
    
      wetted_area = width * Haa[i];
      wetted_perimeter = width + 2 * Haa[i];
      H = wetted_area / wetted_perimeter;
    }
    //-----------------------------------------------


    if(H<0.0) H = 0.0;
    
    zwu = pow(H,power);
    krwval[i] = H * zwu;
	
    //swval
    if(H > 0) {
	  volume_depth = (H + 1.0e-3)*(H + 1.0e-3) / (( H+ 1.0e-3)) -1.0e-6 / (1.0e-3);
    }  
    else  {
      volume_depth = 0.0;
    }
    swval[i] = volume_depth;
 
    //swold
    H = HaaOld[i]; 
    if(H > 0) {
	  volume_depth = (H + 1.0e-3)*(H + 1.0e-3) / (( H+ 1.0e-3)) -1.0e-6 / (1.0e-3);
    }  
    else  {
      volume_depth = 0.0;
    }
    swold[i] = volume_depth;

    //-----------------------------------------------
    //CHANNEL
    double ratio;
    double swell[2];
    double swo[2];
    double xxx;

    if (channelswitch == 1){
      ratio = Haa[i] / 1.0e-3;
      if (ratio > 1.0){
        swell[i] = 1.0;
      }
      else if (ratio > 0.0){
        xxx = 2.0 * (1.0 - ratio);
        swell[i] = pow(ratio,xxx);
      }
      else{
        swell[i] = 0.0;
      }
      swval[i] = swell[i];
   
      ratio = HaaOld[i] / 1.0e-3;
      if (ratio > 1.0){
        swo[i] = 1.0;
      }
      else if (ratio > 0.0){
        xxx = 2.0 * (1.0 - ratio);
        swo[i] = pow(ratio,xxx);
      }
      else{
        swo[i] = 0.0;
      }
      swold[i] = swo[i];
    }//end if channel
    //-----------------------------------------------


  } //end for

}

/**************************************************************************
FEMLib-Method: 
Task: Calculates upstream weighting for Overland Flow -> ckwr and iups 
Programing:
06/2005 MB Implementation
**************************************************************************/
void CFiniteElementStd::CalcCKWR(int nn, double* haa, double* z, double power, double* test, int* xxx)
{
double pandz[4];
int i, j;
int iups[4][4];
double ckwr[4][4];
double maxZ;
double H;
double width;

width = MediaProp->channel_width;
pandz[0] =  haa[0]; 
pandz[1] =  haa[1]; 
pandz[2] =  haa[2]; 
pandz[3] =  haa[3]; 

for (i = 0; i < nn; i++){
  for (j = 0; j < nn; j++){
    iups[i][j] = i;
    if(pandz[j]>pandz[i])
      iups[i][j]= j;
      
  //iups[j][i] = iups[i][j];// not necessary??
    maxZ = MMax(z[i],z[j]);
	if(i==iups[i][j]) 
	H = haa[i] - maxZ;
	else 
    H = haa[j] - maxZ;
	if(H<0.0) H = 0.0;
     
  if(MeshElement->geo_type==1) // line
    ckwr[i][j] = width * H * pow(H * width / (2 * H + width),power);
  else
    ckwr[i][j] = pow(H,power + 1);
  } //end for j
} //end for i
  
for (i = 0; i < nn; i++){
  for (j = 0; j < nn; j++){
    test[i*nn+j] = ckwr[i][j];
    xxx[i*nn+j] = iups[i][j];
  }
}

}

/**************************************************************************
FEMLib-Method: 
Task: Calculate nodal enthalpy
Programming: WW 09/2005
**************************************************************************/
inline void CFiniteElementStd::CalNodalEnthalpy()
{
    int i;
	double temp, dT;
    for(i=0;i<nnodes;i++)
    {
       heat_phase_change = 
         SolidProp->CheckTemperature_in_PhaseChange(NodalVal0[i], NodalVal1[i]);
       if(heat_phase_change) break;
    }
    if(!heat_phase_change) return;
    // Calculate node enthalpy
    for(i=0;i<nnodes;i++)
    {
       NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
       SetCenterGP();
       temp =  FluidProp->Density() 
              *MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta)
			  *NodalVal_Sat[i] ;
       // Enthalpy
       dT=0.0;
       NodalVal2[i] = SolidProp->Enthalpy(NodalVal0[i], temp);
       if(fabs(NodalVal1[i]-NodalVal0[i])<1.0e-8)
           dT=1.0e-8;
       NodalVal3[i] = SolidProp->Enthalpy(NodalVal1[i]+dT, temp);
    }
}

/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for mass matrix
Programing:
01/2005 WW/OK Implementation
03/2005 WW Heat transport
07/2005 WW Change for geometry element object
08/2005 OK Gas flow
10/2005 YD/OK: general concept for heat capacity
11/2005 CMCD Heat capacity function included in mmp
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoefMass() 
{
  int Index = MeshElement->GetIndex();
  double val = 0.0;
  double poro = 0.0;
  double Sw = 0.0;
  double humi = 0.0;
  double rhov = 0.0;
  double rhow = 0.0; 
  double S_P = 0.0;
  CompProperties *m_cp = NULL;

  if(pcs->m_num->ele_mass_lumping)
    ComputeShapefct(1); 
  switch(PcsType){
    default:
      cout << "Fatal rrror in CalCoefMass: No valid PCS type" << endl;
      break;
    case L: // Liquid flow
      //OK coefficient = m_mmp->StorageFunction(ele,gp,1.0);
      val = MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta);
      val /=time_unit_factor;
      break;
    case U: // Unconfined flow
      break;
    case G: // MB now Groundwater flow
      if(MediaProp->unconfined_flow_group>0) //OK
        val = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
      else
        val = MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta);
      break;
    case T: // Two-phase flow
/*OK
      phase = m_pcs->pcs_type_number; // ToDo ->fluid_phase
      m_mfp = mfp_vector[phase];
      sprintf(char_phase,"%i",phase+1);
      nod_val_name += char_phase;
      saturation = PCSGetELEValue(ele,gp,theta,nod_val_name);
      coefficient = m_mmp->Porosity(ele,gp,theta) \
                  * m_mfp->drho_dp \
                  / m_mfp->Density() \
                  * MMax(0.,saturation) \
                  + m_mmp->StorageFunction(ele,gp,theta) \
                  * MMax(0.,saturation);
*/
      break;
    case C: // Componental flow
      //OK comp = m_pcs->pcs_type_number;
      //OK coefficient = MPCCalcStorativityNumber(ele,phase,comp,gp);
      break;
    //....................................................................
    case H: // Heat transport
      val = MediaProp->HeatCapacity(Index,unit,pcs->m_num->ls_theta,this);  //MX *time_unit_factor;
     //YD val += FluidProp->SpecificHeatCapacity()*time_unit_factor;
      val /=time_unit_factor;
      break;
    //....................................................................
    case M: // Mass transport //SB4200
	  	val = MediaProp->Porosity(Index, unit,pcs->m_num->ls_theta); // Porosity
		val *= PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 1);
	  	m_cp = cp_vec[pcs->pcs_component_number]; 
	  	val *= m_cp->CalcElementRetardationFactorNew(Index, unit, pcs); //Retardation Factor
      break;
    case O: // Liquid flow
      val = 1.0; 
      break;
    case R: // Richards
      Sw = interpolate(NodalVal_Sat);
      rhow = FluidProp->Density(); 
      S_P = MediaProp->SaturationPressureDependency(Sw,pcs->m_num->ls_theta);
	    poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
      // Storativity
      val = MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) *Sw;

      // Fluid compressibility
      val += poro  *Sw* FluidProp->drho_dp / rhow;
      // Capillarity
      val += poro * S_P;
      if(MediaProp->heat_diffusion_model==273) //WW
      {
  	     PG = fabs(interpolate(NodalVal1));                      
         TG = interpolate(NodalValC)+273.15; 
         //Rv = GAS_CONSTANT;
         humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
         rhov = humi*FluidProp->vaporDensity(TG); 
         //
         val -= poro * rhov*S_P/rhow;                  
         val += (1.0-Sw)*poro*rhov/(rhow*rhow*GAS_CONSTANT_V*TG);
      }	  
     if(RD_Flag)
     {
      //val += MediaProp->transfer_coefficient*MediaProp->unsaturated_hydraulic_conductivity  \
      //      /(pcs->preferential_factor*FluidProp->Density()*gravity_constant);
      val += Sw*MediaProp->specific_storage;  
     }
      break;
    case F:	// Fluid Momentum
  		val = 1.0;
      break;
    case A: // Air (gas) flow
      val = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
      break;
  }
  return val;
}
/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for mass matrix
Programing:
01/2005 WW/OK Implementation
03/2005 WW Heat transport
07/2005 WW Change for geometry element object
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoefStorage() 
{
/*
  int Index = MeshElement->GetIndex();
  double poro = 0.0;
  double Sw = 0.0;
  double humi = 0.0;
  double rhov = 0.0;
  double rhow = 0.0; 
  double S_P = 0.0;
  double retard, lambda, dS;
*/
  double val = 0.0;
  CompProperties *m_cp =NULL; //CMCD
  //CompProperties *m_cp = cp_vec[pcs->pcs_component_number]; //SB4200
  switch(PcsType){
    default:
      cout << "Fatal error in CalCoefStorage: No valid PCS type" << endl;
      break;
    case L: // Liquid flow
      break;
    case U: // Unconfined flow
      break;
    case G: // MB now Groundwater flow
      break;
    case T: // Two-phase flow
      break;
    case C: // Componental flow
      break;
    case H: // heat transport
      val = 0.0;
      break;
    case M: // Mass transport //SB4200
    m_cp = cp_vec[pcs->pcs_component_number];//CMCD
		val = MediaProp->Porosity(Index, unit,pcs->m_num->ls_theta); //Porosity
        val *= PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 1);
//		val *= FluidProp->Density();
//		val *= m_cp->CalcElementDecayRate(Index); // Decay rate
		val *= m_cp->CalcElementDecayRateNew(Index, pcs); // Decay rate
      break;
    case O: // Liquid flow
      break;
    case R: // Richards
      break;
    case F:	// Fluid Momentum
      break;
    case A: // Air (gas) flow
      break;
  }
   return val;
}

/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for Content matrix
Programing:
01/2005 WW/OK Implementation
03/2005 WW Heat transport
07/2005 WW Change for geometry element object
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoefContent() 
{
  int Index = MeshElement->GetIndex();
  double val = 0.0;
  double dS = 0.0;
  double nodeval0, nodeval1;
  CompProperties *m_cp = NULL; //SB4200
  string name;

  switch(PcsType){
    default:
      cout << "Fatal error in CalCoefContent: No valid PCS type" << endl;
      break;
    case L: // Liquid flow
      break;
    case U: // Unconfined flow
      break;
    case G: // MB now Groundwater flow
      break;
    case T: // Two-phase flow
      break;
    case C: // Componental flow
      break;
    case H: // heat transport
      break;
	case M:{ // Mass transport //SB4200
		val = MediaProp->Porosity(Index, unit,pcs->m_num->ls_theta); // Porosity
//		val *= PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 1);
//		val *= FluidProp->Density(Index, unit,pcs->m_num->ls_theta); // fluid density
		m_cp = cp_vec[pcs->pcs_component_number]; 
		val *= m_cp->CalcElementRetardationFactorNew(Index, unit, pcs); // Retardation factor
		// Get saturation change:
		nodeval0 = PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 0);
		nodeval1 = PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 1);
		dS = nodeval1 - nodeval0; // 1/dt accounted for in assemble function
//		if(Index == 195) cout << val << "Sat_old = " << nodeval0 << ", Sa_new: "<< nodeval1<< ", dS: " << dS << endl;
		val*= dS;
		break;}
    case O: // Liquid flow
      break;
    case R: // Richards
      break;
    case F:	// Fluid Momentum
      break;
    case A: // Air (gas) flow
      break;
  }
   return val;
}
/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for Laplacian matrix
Programing:
01/2005 WW/OK Implementation
02/2005 OK Richards flow
03/2005 WW Heat transport
06/2005 OK Overland flow based on CalcEle2DQuad_OF by MB
07/2005 WW Change for geometry element object
08/2005 Air (gas) flow
last modification:
**************************************************************************/
inline void CFiniteElementStd::CalCoefLaplace(bool Gravity, int ip) 
{
  int i=0;
  double mat_fac = 1.0;
  double Dpv = 0.0;
  double poro = 0.0;
  double tort = 0.0;
  double humi = 0.0;
  double rhow = 0.0; 
  double *tensor = NULL;
  double Hav,manning,chezy,expp,chezy4,Ss,arg;
  static double Hn[9],z[9];
  double GradH[3],Gradz[3],w[3],v1[3],v2[3];
  int nidx1;
  int Index = MeshElement->GetIndex();
  CRFProcess* m_pcs = PCSGet("FLUID_MOMENTUM"); // PCH
  // For nodal value interpolation
  ComputeShapefct(1);
  //======================================================================
   switch(PcsType){
      default:
        break;
      case L: // Liquid flow
        tensor = MediaProp->PermeabilityTensor(Index);
        mat_fac = FluidProp->Viscosity();
        if(gravity_constant<MKleinsteZahl) // HEAD version
          mat_fac = 1.0;
        if(HEAD_Flag) mat_fac=1.0;
        for(i=0; i<dim*dim; i++)
            mat[i] = tensor[i]/mat_fac;
        break;
      case G: // Groundwater flow
/* SB4218 - moved to ->PermeabilityTensor(Index);
        if(MediaProp->permeability_model==2){ //?efficiency
          for(i=0;i<(int)pcs->m_msh->mat_names_vector.size();i++){
            if(pcs->m_msh->mat_names_vector[i].compare("PERMEABILITY")==0)
              break;
          }

          mat_fac = MeshElement->mat_vector(i);
          mat_fac /= FluidProp->Viscosity();
         for(i=0; i<dim; i++) //WW
            mat[i*dim+i] = mat_fac; 
        }
        else{
*/
        tensor = MediaProp->PermeabilityTensor(Index);
        for(i=0;i<dim*dim;i++)
          mat[i] = tensor[i];
        break;
      case T: // Two-phase flow
        break;
      case C: // Componental flow
        break;
      case H: // heat transport
        if(SolidProp->GetCapacityModel()==3) // DECOVALEX THM1  //MX
		{
            Sw = interpolate(NodalVal_Sat); 
            for(i=0; i<dim*dim; i++) mat[i] = 0.0; 
			for(i=0; i<dim; i++) 
	           mat[i*dim+i] = SolidProp->Heat_Conductivity(Sw);  
		}  
        else if(SolidProp->GetCapacityModel()==1 && MediaProp->heat_diffusion_model == 273){
          tensor = MediaProp->HeatConductivityTensor(Index);
          for(i=0; i<dim*dim; i++) 
            mat[i] = tensor[i]; //mat[i*dim+i] = tensor[i];
        }
        else
        {
          tensor = MediaProp->HeatDispersionTensorNew(ip);
          for(i=0;i<dim*dim;i++) 
            mat[i] = tensor[i];  //MX *time_unit_factor; //mat[i*dim+i] = tensor[i];
 
       }      
        break;
      case M: // Mass transport
		    tensor = MediaProp->MassDispersionTensorNew(ip);
		    mat_fac = 1.0; //MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta); // porosity now included in MassDispersionTensorNew()
            if(PCSGet("RICHARDS_FLOW"))
     		    mat_fac *= PCSGetEleMeanNodeSecondary(Index, "RICHARDS_FLOW", "SATURATION1", 1);
		for(i=0;i<dim*dim;i++) 
          mat[i] = tensor[i]*mat_fac*time_unit_factor; 
        break;
      //------------------------------------------------------------------
      case O: // Overland flow
	    //................................................................
        // H - water level
        nidx1 = pcs->GetNodeValueIndex("HEAD")+1;
        Hav = 0.0;
        for(i=0;i<nnodes;i++){
          z[i] = MeshElement->nodes[i]->Z();  
		    Hn[i] = pcs->GetNodeValue(MeshElement->nodes_index[i],nidx1) - z[i];
          if (Hn[i] < 0.0) {Hn[i] = 0.0;}
          Hav += Hn[i]/(double)nnodes;
        }
        //................................................................
        // Friction coefficient
        tensor = MediaProp->PermeabilityTensor(Index);
        manning = MediaProp->permeability_tensor[0]; // Manning-coefficient: n
        // ToDo MB MMP function: m_mmp->FrictionCoefficientChezy(gp)
        if (MediaProp->conductivity_model==3){ // Chezy-coefficient C
          expp = 1.0/6.0;
          chezy = pow(Hav,expp) / manning; // f? b >> h gilt: C = H**1/6 n**-1
          // Grad H: grad_N H J^-1
          MMultMatVec(dshapefct,dim,nnodes,Hn,nnodes,v1,dim);
          MMultVecMat(v1,dim,invJacobian,dim,dim,GradH,dim);
          // Grad z: ? s.Z.380ff
          MMultMatVec(dshapefct,dim,nnodes,z,nnodes,v2,dim);
          MMultVecMat(v2,dim,invJacobian,dim,dim,Gradz,dim);
          w[0] = GradH[0] + Gradz[0];
          w[1] = GradH[1] + Gradz[1];
          chezy4 = pow(chezy,4);
          Ss = ((w[0] * w[0]) / chezy4) +  ((w[1] * w[1]) / chezy4);
          Ss = pow(Ss,0.25);
          if (fabs(Ss) < 1.0e-7){
            Ss = 1.0e-7;
          }
          expp =  5.0 / 3.0;
          arg = (pow(Hav,expp))/(chezy*chezy);
          mat_fac = arg / Ss; 
        }
        //................................................................
        // Tensor
        for(i=0;i<dim*dim;i++)
          mat[i] = tensor[i]/manning * mat_fac; //ToDo
        break;
      //------------------------------------------------------------------
      case R: // Richards flow
		// The following line only applies when Fluid Momentum is on
		if(m_pcs)
			idxS = pcs->GetNodeValueIndex("SATURATION1")+1;	// PCH FM needs this.
		for(i=0;i<nnodes;i++) //SB 4209 - otherwise saturations are nonsense
              NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
        Sw = interpolate(NodalVal_Sat);
//		cout << " Index, Sw: " << Index << ", " << Sw << endl;
        tensor = MediaProp->PermeabilityTensor(Index);
        mat_fac = time_unit_factor* MediaProp->PermeabilitySaturationFunction(Sw,0) \
                / FluidProp->Viscosity();
        for(i=0; i<dim*dim; i++)
            mat[i] = tensor[i] * mat_fac;
        if(MediaProp->heat_diffusion_model==273&&!Gravity)
		{
            rhow = FluidProp->Density(); 
			PG = fabs(interpolate(NodalVal1));                      
			TG = interpolate(NodalValC)+273.15; 
			poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
			tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
            //Rv = GAS_CONSTANT;
            humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
            Dpv = 2.16e-5*tort*(1-Sw)*poro*pow(TG/273.15, 1.8);

			Dpv *= time_unit_factor*FluidProp->vaporDensity(TG)*humi/(GAS_CONSTANT_V*rhow*TG);
            for(i=0; i<dim; i++)
               mat[i*dim+i] += Dpv/rhow;         
		}
       break;
      //------------------------------------------------------------------
      case A: // Air flow
        mat_fac = FluidProp->Viscosity();
        tensor = MediaProp->PermeabilityTensor(Index);
        // Pressure
        nidx1 = pcs->GetNodeValueIndex("PRESSURE1");
        Hav = 0.0;
        for(i=0;i<nnodes;i++){
		    Hav += pcs->GetNodeValue(MeshElement->nodes_index[i],nidx1);
        }
        Hav /= (double)nnodes;
        // 
        for(i=0;i<dim*dim;i++)
          mat[i] = tensor[i]/mat_fac*Hav;
        break;
      //------------------------------------------------------------------
    }
}

//SB4200
/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for advection matrix
Programing:
01/2005 WW/OK Implementation
03/2005 WW Heat transport
07/2005 WW Change for geometry element object
09/2005 SB
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoefAdvection() 
{
  double val = 0.0;
  //OK long Index = MeshElement->GetIndex();
  //----------------------------------------------------------------------
  switch(PcsType){
    default:
      cout << "Fatal error in CalCoefAdvection: No valid PCS type" << endl;
      break;
    case L: // Liquid flow
      break;
    case U: // Unconfined flow
      break;
    case G: // MB now Groundwater flow
      break;
    case T: // Two-phase flow
      break;
    case C: // Componental flow
      break;
    case H: // heat transport
      val = FluidProp->SpecificHeatCapacity()*FluidProp->Density();  //MX *time_unit_factor;
      break;
    case M: // Mass transport //SB4200
		// Get velocity(Gausspoint)/porosity(element)
	  val = 1.0*time_unit_factor; //FluidProp->Density();
      break;
    case O: // Liquid flow
      val = 1.0; 
      break;
    case R: // Richards
      break;
    case F:	// Fluid Momentum
      break;
    case A: // Air (gas) flow
      break;
  }
  return val;
}
/**************************************************************************
   GeoSys - Function: CalCoefStrainCouping

   Aufgabe:
         Calculate coefficient for StrainCouping matrix
   Programmaenderungen:
   01/2005   WW/OK    Erste Version
   07/2005 WW Change for geometry element object   
**************************************************************************/
inline double CFiniteElementStd::CalCoefStrainCouping() 
{
   double val = 0.0;  
   /*
   double r = unit[0];
   double s = unit[1];
   double t = unit[2];
   */

   switch(PcsType){
      default:
        break;
      case L: // Liquid flow
        // 
        val = 1.0;
        break;
      case U: // Unconfined flow
        break;
      case G: // Groundwater
        break;
      case T: // Two-phase flow
        break;
      case C: // Componental flow
        break;
      case H: // heat transport
        break;
      case M: // Mass transport
        break;
      case O: // Overland flow
        break;
      case R: // Richard flow
        val = interpolate(NodalVal_Sat);  // Water saturation
      case A:
        break;
       break;
    }
    return val;
}



/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcMass
   Aufgabe:
           Compute mass matrix, i.e. int (N.mat.N). Linear interpolation
 
   Programming:
   01/2005   WW   
02/2005 OK GEO factor
**************************************************************************/
void CFiniteElementStd::CalcMass()
{
  int i, j;
  // ---- Gauss integral
  int gp;
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt,mat_fac;
  // Material
  mat_fac = 1.0;
  //----------------------------------------------------------------------
  //======================================================================
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
	  // Compute geometry
      ComputeShapefct(1); // Linear interpolation function
      // Material
      mat_fac = CalCoefMass();
	  // if(Index < 0) cout << "mat_fac in CalCoeffMass: " << mat_fac << endl;
      // GEO factor
      fkt *= mat_fac;  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
         for (j = 0; j < nnodes; j++)
         {
             if(j>i) continue;
             (*Mass)(i,j) += fkt *shapefct[i]*shapefct[j];
         }
  }
   //TEST OUTPUT
//  if(Index == 195){cout << "Mass Matrix: " << endl; Mass->Write(); }
}
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcStorage
   Aufgabe:
           Compute mass matrix, i.e. int (N.mat.N). Linear interpolation
 
   Programming:
   01/2005   WW   
02/2005 OK GEO factor
**************************************************************************/
void CFiniteElementStd::CalcStorage()
{
  int i, j;
  // ---- Gauss integral
  int gp;
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt,mat_fac;
  // Material
  mat_fac = 1.0;
  //----------------------------------------------------------------------
  //======================================================================
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
	  // Compute geometry
      ComputeShapefct(1); // Linear interpolation function
      // Material
      mat_fac = CalCoefStorage();
      // GEO factor
      fkt *= mat_fac;  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
         for (j = 0; j < nnodes; j++)
             (*Storage)(i,j) += fkt *shapefct[i]*shapefct[j];

  }
  //TEST OUTPUT
//  if(Index == 195){cout << "Storage Matrix: " << endl; Storage->Write(); }
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcContent
   Aufgabe:
           Compute Content matrix, i.e. int (N.mat.N). Linear interpolation
 
   Programming:
   01/2005   WW   
02/2005 OK GEO factor
**************************************************************************/
void CFiniteElementStd::CalcContent()
{
  int i, j;
  // ---- Gauss integral
  int gp;
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt,mat_fac;
  // Material
  mat_fac = 1.0;
  //----------------------------------------------------------------------
  //======================================================================
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
	  // Compute geometry
      ComputeShapefct(1); // Linear interpolation function
      // Material
      mat_fac = CalCoefContent();
      // GEO factor
      fkt *= mat_fac;  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
         for (j = 0; j < nnodes; j++)
             (*Content)(i,j) += fkt *shapefct[i]*shapefct[j];

  }
  //TEST OUTPUT
//  if(Index == 195){cout << "COntent Matrix: " << endl; Content->Write(); }
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcLumpedMass
   Aufgabe:
           Compute lumped mass matrix, i.e. int (N.mat.N). Linear interpolation
 
   Programming:
   01/2005   WW   
02/2005 OK GEO factor
**************************************************************************/
void CFiniteElementStd::CalcLumpedMass()
{
  int i;
  double factor;
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  // Initialize
  (*Mass) = 0.0; 

  // Center of the reference element
  SetCenterGP();
  // Factor
  factor = CalCoefMass(); 
  pcs->timebuffer = factor;  // Tim Control "Neumann"
  // Volume
  factor *= MeshElement->GetVolume()/(double)nnodes;
  for (i=0; i<nnodes; i++)
     (*Mass)(i,i) = factor;
    			
#ifdef otherLumpedMass
  int i, j;
  int gp_r=0, gp_s=0, gp_t=0;
  double fkt;
  //----------------------------------------------------------------------
  for (i=0; i<nnodes; i++)			
  {
    for(j=0; j<ele_dim; j++)
      x2buff[j] = nodes_xyz[j*nnodes+i];
    UnitCoordinates(x2buff);
    fkt = GetGaussData(i, gp_r, gp_s, gp_t)*CalCoefMass();     
    (*Mass)(i,i) += fkt ; 
  }
  //----------------------------------------------------------------------
#endif
  //TEST OUT
  //Mass->Write();
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcLaplace
   Aufgabe:
           Compute mass matrix, i.e. int (gradN.mat.gradN). Linear interpolation
 
   Programming:
   01/2005   WW  
02/2005 OK GEO factor
**************************************************************************/
void CFiniteElementStd::CalcLaplace()
{
  int i, j, k, l;
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt, water_depth; 
  //----------------------------------------------------------------------
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
      //---------------------------------------------------------
	  // Compute geometry
      ComputeGradShapefct(1); // Linear interpolation function
      //---------------------------------------------------------
      // Material
      CalCoefLaplace(false,gp); 

      
      //---------------------------------------------------------
      // Calculate mass matrix
      //this->pcs->m_msh
	  //if(PcsType == G && MediaProp->unconfined_flow_group == 1 && MeshElement->ele_dim == 2) {
	  water_depth = 1.0;
      // The following "if" is done by WW
      if(PcsType == G && MediaProp->unconfined_flow_group == 1 && MeshElement->ele_dim == 2 && !pcs->m_msh->cross_section) {
          water_depth = 0.0;
          for(i=0; i< nnodes; i++) 
             water_depth += (pcs->GetNodeValue(nodes[i],idx1) -Z[i])*shapefct[i];          
	  }
      fkt *= water_depth;
      for (i = 0; i < nnodes; i++) {
		  for (j = 0; j < nnodes; j++) {
   			      //  if(j>i) continue;  //MB temporary as Laplace now defined unsymmetric
             for (k = 0; k < dim; k++) {
				 for(l=0; l< dim; l++) {
			         (*Laplace)(i,j) += fkt * dshapefct[k*nnodes+i] \
                         * mat[dim*k+l] * dshapefct[l*nnodes+j];
				    
   		          } 
			 }
          }
	    }
      ///
      /*
      //WW
      // The following "if" is done by MB (??)
	  if(PcsType == G && MediaProp->unconfined_flow_group == 1 && MeshElement->ele_dim == 2 && !pcs->m_msh->cross_section) {          		  
		  for (i = 0; i < nnodes; i++) {
		  for (j = 0; j < nnodes; j++) {
           //  if(j>i) continue;  //MB temporary as Laplace now defined unsymmetric
             for (k = 0; k < dim; k++) {
				 for(l=0; l< dim; l++) {
			       for(m=0; m< nnodes; m++) {
                       z = MeshElement->nodes[m]->Z(); 
                       water_depth = (pcs->GetNodeValue(MeshElement->nodes_index[m],idx1) -z)*shapefct[m];
                    (*Laplace)(i,j) += fkt * dshapefct[k*nnodes+i] \
                                    * mat[dim*k+l] * water_depth \
                                    * dshapefct[l*nnodes+j];
             }
         }
	  }
  }
	    }
	  }
	  else {
		for (i = 0; i < nnodes; i++) {
		  for (j = 0; j < nnodes; j++) {
   			      //  if(j>i) continue;  //MB temporary as Laplace now defined unsymmetric
             for (k = 0; k < dim; k++) {
				 for(l=0; l< dim; l++) {
			         (*Laplace)(i,j) += fkt * dshapefct[k*nnodes+i] \
                         * mat[dim*k+l] * dshapefct[l*nnodes+j];
				    
   		          } 
			 }
          }
	    }
	  }
	  ///////
     */
  }
  //TEST OUTPUT
  // Laplace->Write();
}

//SB4200
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcAdvection
   Aufgabe:  Calculate the advection matrix
 
   Programming:
   01/2005   WW  
   02/2005   OK GEO factor
   09/2005   SB - adapted to advection
   11/2005   CMCD debugged, altered alot.
**************************************************************************/
void CFiniteElementStd::CalcAdvection()
{
  int i, j, k;
  int gp, gp_r=0, gp_s=0, gp_t;
  double fkt,mat_factor = 0.0;
  double vel[3];

  ElementValue* gp_ele = ele_gp_value[Index];

  //Initial values
  gp_t = 0;
 (*Advection)=0.0;

  //----------------------------------------------------------------------
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
    //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
    //---------------------------------------------------------
    fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
    mat_factor = CalCoefAdvection();//T
    //---------------------------------------------------------
	  // Compute geometry
    ComputeGradShapefct(1); // Linear interpolation function....dNJ-1....var dshapefct
	ComputeShapefct(1);// Linear interpolation N....var shapefct
    //---------------------------------------------------------
    //Velocity
    vel[0] = gp_ele->Velocity(0, gp);   
    vel[1] = gp_ele->Velocity(1, gp);
    vel[2] = gp_ele->Velocity(2, gp);
     
    for (i = 0; i< nnodes; i++){
      for (j = 0; j < nnodes; j++)
         for (k = 0; k < dim; k++)
            (*Advection)(i,j) += mat_factor*fkt*shapefct[i]*vel[k]
                                 *dshapefct[k*nnodes+j];             
    }
  }
  //TEST OUTPUT
  //  cout << "Advection Matrix: " << endl; Advection->Write(); 
}

void CFiniteElementStd::CalcRHS_by_ThermalDiffusion()
{
  int i, j, k;
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt;
  double Dv = 0.0;
  double Dtv = 0.0;
  double poro = 0.0;
  double tort = 0.0;
  double humi = 0.0;
  double rhov = 0.0;
  double drdT = 0.0;
  double beta = 0.0;
 
  (*Laplace) = 0.0;
  (*Mass) = 0.0;
  //----------------------------------------------------------------------
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

      //---------------------------------------------------------
	  // Compute geometry
      ComputeGradShapefct(1); // Linear interpolation function
      //---------------------------------------------------------
      // Material
//	  if(FluidProp->diffusion_model=273)
//    {
         ComputeShapefct(1); 
         double rhow = FluidProp->Density(); 
  	     PG = fabs(interpolate(NodalVal1));                      
         TG = interpolate(NodalValC)+273.15; 
         Sw = interpolate(NodalVal_Sat);
         poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
         tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
         beta = poro*MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) *Sw;
         //Rv = GAS_CONSTANT;
         humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
         Dv = tort*(1.0-Sw)*poro*2.16e-5*pow(TG/273.15, 1.8);
         rhov = humi*FluidProp->vaporDensity(TG); 
         drdT= (FluidProp->vaporDensity_derivative(TG)*humi-rhov*PG/(GAS_CONSTANT_V*rhow*pow(TG, 2.0)))/rhow;
         Dtv = time_unit_factor*Dv*drdT;
              
//    }        
      //---------------------------------------------------------
      // Calculate a Laplace
      for (i = 0; i < nnodes; i++)
	  {
         for (j = 0; j < nnodes; j++)
         {
             if(j>i) continue;
             for (k = 0; k < dim; k++)
                (*Laplace)(i,j) +=   fkt*Dtv*dshapefct[k*nnodes+i] 
                                   * dshapefct[k*nnodes+j];
             (*Mass)(i,j) += fkt*poro*(beta+(1.0-Sw)*drdT)*shapefct[i]*shapefct[j];
         }
	  }
  }
  // Symmetry  
  for (i = 0; i < nnodes; i++)
  {
     for (j = 0; j < nnodes; j++)
     {
        if(j<=i) continue;
        (*Laplace)(i,j) = (*Laplace)(j,i);
     }
  }
  for (i = 0; i < nnodes; i++)
  {
      for (j = 0; j < nnodes; j++)
      { 
         (*RHS)(i) -= (*Laplace)(i,j)*NodalValC[j];
		 (*RHS)(i) += (*Mass)(i,j)*(NodalValC1[j]-NodalValC[j])/dt;
      }
      pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]]
           += (*RHS)(i);
  }

  //TEST OUTPUT
  // Laplace->Write();
  // Mass->Write();
  // RHS->Write();
}




/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcStrainCoupling
   Aufgabe:
           Compute the strain couping matrix
 
   Programming:
   01/2005   WW   
**************************************************************************/
void CFiniteElementStd::CalcStrainCoupling()
{


   int i,k,l,kl, gp, gp_r, gp_s, gp_t;
   double fkt;
   setOrder(2);
   // Swap cordinates in case of (x, 0.0, z) only for 2D problem 
   if(coordinate_system%10==2) // Z has number
   {
        switch(dim)
		{
            case 1:
              for(i=0; i<nNodes; i++)
              {
                 X[i] = MeshElement->nodes[i]->Z();  
		         Y[i] = MeshElement->nodes[i]->Y();       
		         Z[i] = MeshElement->nodes[i]->X();        
	          }
              break;
			case 2:
              for(i=0; i<nNodes; i++)
              {
                 X[i] = MeshElement->nodes[i]->X();  
		         Y[i] = MeshElement->nodes[i]->Z();       
		         Z[i] = MeshElement->nodes[i]->Y();        
	          }

		}
   }
   // Loop over Gauss points
   for(i=0; i<dim; i++ )
   {
      for (gp = 0; gp < nGaussPoints; gp++)
      {
         fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

         ComputeGradShapefct(2);
         ComputeShapefct(1);

         fkt *= CalCoefStrainCouping();

         for (k=0;k<nnodes;k++)
		 {
            for (l=0;l<nnodesHQ;l++) {    
                kl = nnodesHQ*i+l;       
               (*StrainCoupling)(k, kl) += shapefct[k] * dshapefctHQ[kl] * fkt;  
            }
        }
     }
   }
   setOrder(1);
  // StrainCoupling->Write();
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: Assemby_Gravity
   Aufgabe:
           Assemble the contribution of gravity to RHS in Darcy flow
           to the global system
 
   Programming:
   01/2005   WW/OK   
**************************************************************************/
// Local assembly
void  CFiniteElementStd::Assemble_Gravity()
{
  if((coordinate_system)%10!=2&&(!axisymmetry)){
   (*GravityMatrix) = 0.0;
    return;
  }
  int i, k;
  
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt, rho; //, rich_f;
  double k_rel_iteration;
  // GEO
  double geo_fac = MediaProp->geo_area;
  if(!FluidProp->CheckGravityCalculation()) return; 
  //rich_f = 1.0; 
  //if(PcsType==R) rich_f = -1.0; //WW
   
  k_rel_iteration = 1.0;

  for (i = 0; i < nnodes; i++)
     NodalVal[i] = 0.0;

  (*GravityMatrix) = 0.0;
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determination
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

      //---------------------------------------------------------
	  // Compute geometry
      //---------------------------------------------------------
      ComputeGradShapefct(1); // Linear interpolation function

      // Material
      CalCoefLaplace(true);
      rho = FluidProp->Density();            //Index,unit,pcs->m_num->ls_theta
      if(gravity_constant<MKleinsteZahl) // HEAD version
        rho = 1.0;
      else if(HEAD_Flag) rho = 1.0;
      else
        rho *= gravity_constant; 
      fkt *= rho; //*rich_f;		  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
	  {
         for (k = 0; k < dim; k++)
         {
            NodalVal[i] -= fkt*dshapefct[k*nnodes+i]
                           *mat[dim*k+dim-1];
         }
      }
  }

  for (i=0;i<nnodes;i++)
  {
      pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]]
               += k_rel_iteration* geo_fac*NodalVal[i];
     (*RHS)(i+LocalShift) += NodalVal[i];
  }
  //TEST OUTPUT
  //RHS->Write();
}
////////////////////////////////////////////////////////////////
/*
void  CFiniteElementStd::Assemble_Gravity()
{
  if((coordinate_system)%10!=2)
     return;
  int i, j, k, l;
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt, rho;
  double k_rel_iteration;
  // GEO
  double geo_fac = MediaProp->geo_area;

  k_rel_iteration = 1.0;

  (*GravityMatrix) = 0.0;
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determination
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

      //---------------------------------------------------------
	  // Compute geometry
      //---------------------------------------------------------
      ComputeGradShapefct(1); // Linear interpolation function

      // Material
      CalCoefLaplace(true);
      rho = FluidProp->Density(Index,unit,pcs->m_num->ls_theta);
      if(gravity_constant<MKleinsteZahl) // HEAD version
        rho = 1.0;
      else if(HEAD_Flag) rho = 1.0;
      else
        rho *= gravity_constant; 

      fkt *= rho;		  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
         for (j = 0; j < nnodes; j++)
         {
             if(j>i) continue;
             for (k = 0; k < dim; k++)
             {
                 for(l=0; l<dim; l++)
                    (*GravityMatrix)(i,j) += fkt*dshapefct[k*nnodes+i]
                                 *mat[dim*k+l]* dshapefct[l*nnodes+j];
             }
         }
  }

  //TEST OUTPUT
  //GravityMatrix->Write();
 
  double* G_coord = NULL;
  if((coordinate_system)/10==1)
     G_coord = X;
  else if((coordinate_system)/10==2)
     G_coord = Y;
  else if((coordinate_system)/10==3)
     G_coord = Z;
 
  for (i = 0; i < nnodes; i++)
  {
     NodalVal[i] = 0.0;
     for (j = 0; j < nnodes; j++)
         NodalVal[i] -= (*GravityMatrix)(i,j)* G_coord[j];
  }

  for (i=0;i<nnodes;i++)
  {
      pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]]
               += k_rel_iteration* geo_fac*NodalVal[i];
     (*RHS)(i+LocalShift) += NodalVal[i];
  }
  //TEST OUTPUT
  //RHS->Write();
}
*/

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: Velocity calulation
 
   Programming:  WW
   08/2005      
**************************************************************************/
// Local assembly
void  CFiniteElementStd::Cal_Velocity()
{
  int i, j, k;
  static double vel[3];  
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  double coef = 0.0;
  gp_t = 0;

  // Get room in the memory for local matrices
  SetMemory();
  // Set material
  SetMaterial();

  ElementValue* gp_ele = ele_gp_value[Index];

  gp_ele->Velocity = 0.0;
  // Loop over Gauss points
  k = (coordinate_system)%10;
  for(i=0; i<nnodes; i++)
	 NodalVal[i] = pcs->GetNodeValue(nodes[i], idx1); 
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determination
      //---------------------------------------------------------
      GetGaussData(gp, gp_r, gp_s, gp_t);

      //---------------------------------------------------------
  	  // Compute geometry
      //---------------------------------------------------------
      ComputeGradShapefct(1); // Linear interpolation function

      // Material
      CalCoefLaplace(true);

      // Velocity
      for (i = 0; i < dim; i++)
      {
         vel[i] = 0.0; 
         for(j=0; j<nnodes; j++)
			 vel[i] += NodalVal[j]*dshapefct[i*nnodes+j];
//			 vel[i] += fabs(NodalVal[j])*dshapefct[i*nnodes+j];
	  }     
      // Gravity term
      if(k==2&&(!HEAD_Flag))
	  {
         coef  =  gravity_constant*FluidProp->Density();
         if(dim==3&&ele_dim==2)
		 {
            for(i=0; i<dim; i++)
		    { 
               for(j=0; j<ele_dim; j++)          
			      vel[i] += coef*(*MeshElement->tranform_tensor)(i, k)*(*MeshElement->tranform_tensor)(2, k);   
		    }
		 } // To be correctted   
		 else
           vel[dim-1] += coef;
	  }
	  for (i = 0; i < dim; i++)
      {
         for(j=0; j<dim; j++)
//            gp_ele->Velocity(i, gp) -= mat[dim*i+j]*vel[j];  // unit as that given in input file
            gp_ele->Velocity(i, gp) -= mat[dim*i+j]*vel[j]/time_unit_factor;
      }
  }
  //
// gp_ele->Velocity.Write();
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: Assemby_Gravity
   Aufgabe:
           Assemble the contribution of known gradient of hydraulic head or
		   pressure and gravity to RHS in Darcy flow
           to the global system
 
   Programming:
   05/2005   PCH   
   09/2005   PCH 
**************************************************************************/
// Local assembly
void  CFiniteElementStd::AssembleRHS(int dimension)
{ 
	 // ---- Gauss integral
	int gp, gp_r=0, gp_s=0, gp_t;
	gp_t = 0;
	double fkt, fktG, rho;
 
	// Declare two known properties on node
	// Since I declare these variables locally, the object of Vec should handle destruction nicely  
	// when this local function is done so that I don't bother with memory leak.
	
	// Initialize Pressure from the value already computed previously.
    CRFProcess* m_pcs = NULL;
	for(int i=0; i< (int)pcs_vector.size(); ++i)
    {
        m_pcs = pcs_vector[i];
        if(m_pcs->pcs_type_name.find("FLOW")!=string::npos)
            break;
    }
	// Update the process for proper coefficient calculation.
	pcs = m_pcs;

	int nidx1 = m_pcs->GetNodeValueIndex("PRESSURE1")+1;
	for (int i = 0; i < nnodes; ++i)
	{
		NodalVal[i] = 0.0;
		NodalVal1[i] = m_pcs->GetNodeValue(nodes[i], nidx1);   
        NodalVal2[i] = 0.0;
	}

	// Loop over Gauss points
	for (gp = 0; gp < nGaussPoints; gp++)
	{
		//---------------------------------------------------------
		//  Get local coordinates and weights 
		//  Compute Jacobian matrix and its determination
		//---------------------------------------------------------
		fktG = fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

		//---------------------------------------------------------
		// Compute geometry
		//---------------------------------------------------------
		ComputeGradShapefct(1); // Linear interpolation function
		ComputeShapefct(1); // Linear interpolation function

		// Material
		CalCoefLaplace(true);

		// Calculate vector that computes dNj/dx*Ni*Pressure(j)
		// These index are very important. 
        rho = FluidProp->Density();
        if(gravity_constant<MKleinsteZahl) // HEAD version
            rho = 1.0;
        else if(HEAD_Flag) rho = 1.0;
        else
            rho *= gravity_constant; 

        fktG *= rho;		  
		for (int i = 0; i < nnodes; i++)
			for (int j = 0; j < nnodes; j++)
				for (int k = 0; k < ele_dim; k++)
                {
					NodalVal[i]  -= fkt*dshapefct[dimension*nnodes+j]
							    *mat[ele_dim*dimension+k]* shapefct[i] * NodalVal1[j];
                    NodalVal2[i] += fktG*dshapefct[dimension*nnodes+j]
                                 *mat[ele_dim*dimension+k]* shapefct[i] * MeshElement->nodes[j]->Z();
                }
    }

    // Just influence when it's the gravitational direction in the case of Liquid_Flow
    // Thus, it needs one more switch to tell Liquid_Flow and Groundwater_Flow.
    int IsGroundwaterIntheProcesses = 0;
    for(int i=0; i< (int)pcs_vector.size(); ++i)
    {
        m_pcs = pcs_vector[i];
        if(m_pcs->pcs_type_name.find("GROUNDWATER_FLOW")!=string::npos)
            IsGroundwaterIntheProcesses = 1;
    }

	// Checking the coordinateflag for proper solution.
	int checkZaxis = 0;
	int coordinateflag = pcs->m_msh->GetCoordinateFlag(); 
	if( (coordinateflag == 12) || (coordinateflag == 22 && dimension == 1) ||
		(coordinateflag == 32 && dimension == 2) )
		checkZaxis = 1;	// Then, this gotta be z axis.

	// Compansate the gravity term along Z direction
    if(checkZaxis && IsGroundwaterIntheProcesses == 0 )
        for (int i = 0; i < nnodes; i++)
            NodalVal[i] -= NodalVal2[i];

	// Store the influence into the global vectors.
    m_pcs = PCSGet("FLUID_MOMENTUM");
	for (int i=0;i<nnodes;i++)
        m_pcs->eqs->b[eqs_number[i]] += NodalVal[i];    
  	
	// OK. Let's add gravity term that incorporates the density coupling term.
	// This is convenient. The function is already written in RF.	
	//Assemble_Gravity();
}

/**************************************************************************
FEMLib-Method: 
Task: Assemble local matrices of parabolic equation to the global system
Programing:
01/2005 WW/OK Implementation
05/2005 WW Dynamics and others -> new equation type
**************************************************************************/
void CFiniteElementStd::AssembleParabolicEquation()
{
  int i,j;
  // NUM
  double theta = pcs->m_num->ls_theta; //OK
  //----------------------------------------------------------------------
  // Non-linearities
  double non_linear_function_iter = 1.0; //OK MediaProp->NonlinearFlowFunction(Index,unit,theta);
  double non_linear_function_t0   = 1.0; //OK MediaProp->NonlinearFlowFunction(Index,unit,0.0);
  //----------------------------------------------------------------------
  // Dynamic
// WW: is dynamic really parabolic equation type -> AssembleDynamicEquation
  dynamic = false;
  double *p_n = NULL; 
  double fac1, fac2;
  double beta1 = 0.0;
  if(pcs->pcs_type_name.find("DYNAMIC")==0){ //OK
    dynamic = true;
    if(pcs->m_num->CheckDynamic()) // why NUM, it is PCS
    beta1  = pcs->m_num->GetDynamicDamping_beta1();
  }
  //----------------------------------------------------------------------
  // Initialize.
  // if (pcs->Memory_Type==2) skip the these initialization
  (*Mass) = 0.0;
  (*Laplace) = 0.0;
  //----------------------------------------------------------------------
  // GEO
  // double geo_fac = MediaProp->geo_area;
  //----------------------------------------------------------------------
  // Calculate matrices
  // Mass matrix..........................................................
  if(pcs->m_num->ele_mass_lumping)
    CalcLumpedMass();
  else
    CalcMass();
  // Laplace matrix.......................................................
  CalcLaplace();

  pcs->timebuffer /= mat[0]; //YD
  //======================================================================
  // Assemble global matrix
  //----------------------------------------------------------------------
  // Time discretization 
  // ToDo PCS time step
  double dt_inverse = 0.0;
  if(dt<MKleinsteZahl){
    cout<<"\n Zeitschritt ist Null ! Abbruch !"<<endl;
    return;
  }
  else 
    dt_inverse = 1.0 / dt; 
  //----------------------------------------------------------------------
  // Assemble local left matrix: 
  // [C]/dt + theta [K] non_linear_function for static problems
  // [C] + beta1*dt [K] for dynamic problems: ? different equation type
  if(dynamic)
  {
    fac1 = 1.0;
    fac2 = beta1*dt;
  }
  else 
  {
    fac1 = dt_inverse; //*geo_fac;
    fac2 = theta * non_linear_function_iter; //*geo_fac;
  }

  //Mass matrix
  *StiffMatrix    = *Mass;
  (*StiffMatrix) *= fac1;
  // Laplace matrix
  *AuxMatrix      = *Laplace;
  (*AuxMatrix)   *= fac2;
  *StiffMatrix   += *AuxMatrix;
  //----------------------------------------------------------------------
  // Add local matrix to global matrix
  for(i=0;i<nnodes;i++){
    for(j=0;j<nnodes;j++){
      MXInc(NodeShift[problem_dimension_dm]+eqs_number[i],\
            NodeShift[problem_dimension_dm]+eqs_number[j],\
            (*StiffMatrix)(i,j));
    }
  }
  //======================================================================
  // Assemble local RHS vector: 
  // ( [C]/dt - (1.0-theta) [K] non_linear_function ) u0  for static problems
  // ( [C] + beta1*dt [K] ) dp  for dynamic problems
  if(dynamic)
  {
    fac1 = -1.0;
    fac2 = -beta1*dt;
  }
  else 
  {
    fac1 = dt_inverse; //*geo_fac;
    fac2 = (1.0-theta) * non_linear_function_t0; //*geo_fac;
  }

  // Mass - Storage
  *AuxMatrix1    = *Mass;
  (*AuxMatrix1) *= fac1;
  //Laplace - Diffusion
  *AuxMatrix     = *Laplace;
  (*AuxMatrix)  *= fac2;
  *AuxMatrix1   -= *AuxMatrix;

  for (i=0;i<nnodes; i++)
  {
    NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx0);
    NodalVal[i] = 0.0;
  }
  AuxMatrix1->multi(NodalVal1, NodalVal);
  if(dynamic)
  {
    // Velocity of pressure of the previous step 
    p_n = dm_pcs->GetAuxArray();     
    for (i=0;i<nnodes; i++)
      NodalVal1[i] = p_n[nodes[i]+NodeShift[problem_dimension_dm]];
    Mass->multi(NodalVal1, NodalVal, -1.0);
    //p_n+vp*dt 
    for (i=0;i<nnodes; i++)
    {
      NodalVal1[i] *= dt;
      NodalVal1[i] += pcs->GetNodeValue(nodes[i],idx_pres); 
    }
    Laplace->multi(NodalVal1, NodalVal, -1.0);       
  }
  for (i=0;i<nnodes;i++)
  {
    pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]] += NodalVal[i];
    (*RHS)(i+LocalShift) +=  NodalVal[i];
  }
	//Debug output
	if(Index < 0){
	cout << " Element Number " << Index << endl;
	cout << " Mass matrix" << endl;
	Mass->Write();
	cout << " Laplace matrix" << endl;
	Laplace->Write();
	cout << " Left matrix" << endl;
	StiffMatrix->Write();
	cout << " Right matrix" << endl;
	AuxMatrix1->Write();
	cout << " RHS Vector "<< endl << "| " << NodalVal[0] << " |" << endl;
	cout << "| " << NodalVal[1] << " |" << endl;
	cout << " initial values" << endl;
	cout << "| " << NodalVal1[0] << " |" << endl;
	cout << "| " << NodalVal1[1] << " |" << endl;
	}
}

//SB4200
/**************************************************************************
FEMLib-Method: 
Task: Assemble local matrices of parabolic equation to the global system
Programing:
01/2005 WW/OK Implementation
05/2005 WW Dynamics and others
09/2005 SB Adapted from AssembleParabolicEquation to assemble transport equation
**************************************************************************/
void CFiniteElementStd::AssembleMixedHyperbolicParabolicEquation()
{
  int i,j;
  // NUM
  double theta = pcs->m_num->ls_theta; //OK
  //----------------------------------------------------------------------
  unit[0] = unit[1] = unit[2] = 0.0; 
  // Non-linearities
//  double non_linear_function_iter = 1.0; //OK MediaProp->NonlinearFlowFunction(Index,unit,theta);
//  double non_linear_function_t0   = 1.0; //OK MediaProp->NonlinearFlowFunction(Index,unit,0.0);
  double fac_mass, fac_laplace, fac_advection, fac_storage, fac_content;
  // Initialize.
  (*Mass) = 0.0;
  (*Laplace) = 0.0;
  (*Advection) = 0.0;
  (*Storage) = 0.0;
  (*Content) = 0.0;
  //----------------------------------------------------------------------
  // GEO
  // double geo_fac = MediaProp->geo_area;
  //----------------------------------------------------------------------
  // Calculate matrices
  // Mass matrix..........................................................
  if(pcs->m_num->ele_mass_lumping)
    CalcLumpedMass();
  else
    CalcMass();
  // Laplace matrix.......................................................
  CalcLaplace();
  // Advection matrix.....................................................
  CalcAdvection();	
  // Calc Storage Matrix for decay 
  CalcStorage();
   // Calc Content Matrix for  saturation changes
  CalcContent();
  //======================================================================
  // Assemble global matrix
  //----------------------------------------------------------------------
  // Time discretization 
  // ToDo PCS time step
  double dt_inverse = 0.0;
  if(dt<MKleinsteZahl){
    cout<<"\n Zeitschritt ist Null ! Abbruch !"<<endl;
    return;
  }
  else 
    dt_inverse = 1.0 / dt; 
  //----------------------------------------------------------------------
  // Assemble local left matrix: 
  // [C]/dt + theta [K] non_linear_function for static problems

   fac_mass = dt_inverse; //*geo_fac;
   fac_laplace = theta ; //* non_linear_function_iter; //*geo_fac; 
   fac_advection = theta;
   fac_storage = theta;
   fac_content = theta*dt_inverse;

  //Mass matrix
  *StiffMatrix    = *Mass;
  (*StiffMatrix) *= fac_mass;
  // Laplace matrix
  *AuxMatrix      = *Laplace;
  (*AuxMatrix)   *= fac_laplace;
  *StiffMatrix   += *AuxMatrix;
  // Advection matrix
  *AuxMatrix      = *Advection;
  (*AuxMatrix)   *= fac_advection;
  *StiffMatrix   += *AuxMatrix;
  // Storage matrix
  *AuxMatrix      = *Storage;
  (*AuxMatrix)   *= fac_storage;
  *StiffMatrix   += *AuxMatrix;
  // Content matrix
  *AuxMatrix      = *Content;
  (*AuxMatrix)   *= fac_content;
  *StiffMatrix   += *AuxMatrix;

  //----------------------------------------------------------------------
  // Add local matrix to global matrix
  for(i=0;i<nnodes;i++){
    for(j=0;j<nnodes;j++){
       MXInc(NodeShift[problem_dimension_dm]+eqs_number[i],\
            NodeShift[problem_dimension_dm]+eqs_number[j],\
               (*StiffMatrix)(i,j));
    }
  }
  //======================================================================
  // Assemble local RHS vector: 
  // ( [C]/dt - (1.0-theta) [K] non_linear_function ) u0  for static problems
  // ( [C] + beta1*dt [K] ) dp  for dynamic problems

  fac_mass = dt_inverse; //*geo_fac;
  fac_laplace = -(1.0-theta); // * non_linear_function_t0; //*geo_fac;
  fac_advection = -(1.0-theta);
  fac_storage = -(1.0-theta); //*lambda
  fac_content = -(1.0-theta)*dt_inverse;

  // Mass - Storage
  *AuxMatrix1    = *Mass;
  (*AuxMatrix1) *= fac_mass;
  //Laplace - Diffusion
  *AuxMatrix     = *Laplace;
  (*AuxMatrix)  *= fac_laplace;
  *AuxMatrix1   += *AuxMatrix;
  // Advection
  *AuxMatrix     = *Advection;
  (*AuxMatrix)  *= fac_advection;
  *AuxMatrix1   += *AuxMatrix;
  // Storage
  *AuxMatrix     = *Storage;
  (*AuxMatrix)  *= fac_storage;
  *AuxMatrix1   += *AuxMatrix;
  // Content
  *AuxMatrix     = *Content;
  (*AuxMatrix)  *= fac_content;
  *AuxMatrix1   += *AuxMatrix;

  
   for (i=0;i<nnodes; i++)
   {
       NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx0);
       NodalVal[i] = 0.0;
   }
   AuxMatrix1->multi(NodalVal1, NodalVal);  //AuxMatrix1 times vector NodalVal1 = NodalVal
  //----------------------------------------------------------------------
  if(dom_vector.size()>0)
  {
    for (i=0;i<nnodes;i++)
    {
      m_dom->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]] += NodalVal[i];
      (*RHS)(i+LocalShift) +=  NodalVal[i];
    }
  }
  else
  {
   for (i=0;i<nnodes;i++)
   {
       pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]] += NodalVal[i];
       (*RHS)(i+LocalShift) +=  NodalVal[i];
   }
  }
  //----------------------------------------------------------------------
	//Debug output
	if(Index < 0){
	cout << " Element Number " << Index << endl;
	cout << " Mass matrix" << endl;
	Mass->Write();
	cout << " Advection matrix" << endl;
	Advection->Write();
	cout << " Dispersion matrix" << endl;
	Laplace->Write();
	cout << " Storage matrix" << endl;
	Storage->Write();
	cout << " Content matrix" << endl;
	Content->Write();
	cout << " Left matrix" << endl;
	StiffMatrix->Write();
	cout << " Right matrix" << endl;
	AuxMatrix1->Write();
	cout << endl ;
	for (i=0;i<nnodes; i++) cout << "| " << NodalVal[i] << " |" << endl;
	cout << " initial concentrations" << endl;
	for (i=0;i<nnodes; i++) cout << "| " << NodalVal1[i] << " |" << endl;
	
  }
}
/**************************************************************************
FEMLib-Method: 
Task: Assemble local matrices of parabolic equation to the global system
Comment: Based on hydrosphere, CVFE Method, noch lange nicht allgemein,
Programing:
06/2005 MB Implementation
**************************************************************************/
void CFiniteElementStd::AssembleParabolicEquationNewton()
{
  //---------------------------------------------------------
  long i,j;
  double Haa[4];
  double HaaOld[4];
  double haa[4];
  double z[4];
  int nidx1, nidx0;
  double fric;
  double eslope;
  double dx = 0.; //OKWW
  double dy = 0.; //OKWW
  double dzx;
  double dzy;
  double axx = 0; //OKWW
  double ayy = 0; //OKWW
  double delt = 0.;
  double power;
  double ast=0.0;
  double dummy[4];
  double amat[4][4];
  double gammaij;
  double maxZ;
  double swval[4];
  double swold[4];
  double rhs[4];
  double ckwr[16];
  int iups[16];
  double GradH[2];
  double dhds;
  double x2, x3, y2, y3;
  double delt2inv, delt2;
  double b[3], g[3];
  double el = 1.;
  //double akwr;
  double astor[4];
  double width;
  
  GradH[0] = 0.0;
  GradH[1] = 0.0;

  MNulleVec(astor,4);

  /* Initialisieren */
  MNulleMat(ckwr, nnodes, nnodes);
  MNulleMat(edlluse, nnodes, nnodes);
  MNulleMat(edttuse, nnodes, nnodes);
  MNulleVec(rhs,nnodes);
  for (i = 0; i < nnodes; i++){
    for (j = 0; j < nnodes; j++){
      amat[i][j]= 0.0;
    }
  }

  /* Numerische Parameter */
  //------------------------------------------------------------------------
  // Element tolpology

  //------------------------------------------------------------------------
  // Element MMP group
  fric =  MediaProp->friction_coefficient;
  width = MediaProp->channel_width;

  nidx1 = pcs->GetNodeValueIndex("HEAD")+1;
  nidx0 = pcs->GetNodeValueIndex("HEAD")+0;

  for(i=0;i<nnodes;i++){
    z[i] = Z[i];
    haa[i] = pcs->GetNodeValue(nodes[i],nidx1);
    Haa[i] = haa[i] - Z[i]; 
    if (Haa[i] < 0.0) {Haa[i] = 0.0;}
    HaaOld[i] = pcs->GetNodeValue(nodes[i],nidx0) - Z[i];
  }

  if(MeshElement->geo_type==1){ // line
    
    dx = X[1] - X[0];
    dy = Y[1] - Y[0];
    dzx = Z[1] - Z[0];
  
    el = sqrt(dx*dx + dy*dy);
    delt = el * width; 
  
    // 1/sqrt(dhds)
    //eslope = CalcEslope(index, m_pcs);
    GradH[0] = (haa[0] - haa[1]) / el;
    GradH[1] = 0.0;
  
    //Get Influence Coefficient Matrices for analytical integration 
    Get_Matrix_Quad();
  }


  if(MeshElement->geo_type==2){ // quad
    dx = X[1] - X[0]; //ell
    dy = Y[3] - Y[0]; //ett
    dzx = Z[1] - Z[0];
    dzy = Z[3] - Z[0]; 

    dx = sqrt(dx*dx + dzx*dzx); 
    dy = sqrt(dy*dy + dzy*dzy); 
    //in call freldim
    delt = dx * dy;
  
    // 1/sqrt(dhds)
    //eslope = CalcEslope(index, m_pcs);
    GradH[0] = (haa[0] - haa[1] - haa[2] + haa[3]) / (2.0*dx);
    GradH[1] = (haa[0] + haa[1] - haa[2] - haa[3]) / (2.0*dy) ;
  
    //Get Influence Coefficient Matrices for analytical integration 
    Get_Matrix_Quad();
  } // endif quad

  if(MeshElement->geo_type==4){ // tri

    x2 = X[1] - X[0];
    x3 = X[2] - X[0];
    y2 = Y[1] - Y[0]; 
    y3 = Y[2] - Y[0]; 
    //in call freldim
    delt = (x2*y3 - x3*y2) * 0.5; 
    delt2 = 2.0 * delt; 
    delt2inv = 1.0 / delt2; 
    b[0] = (y2-y3) * delt2inv; 
    b[1] = y3 * delt2inv; 
    b[2] = -y2 * delt2inv; 
    g[0] = (x3-x2) * delt2inv; 
    g[1] = -x3 * delt2inv; 
    g[2] = x2 * delt2inv; 

    for(i=0; i<nnodes; i++)  {
      for(j=0; j<nnodes; j++)  {
	     //edlluse[i][j] = b[i] * b[j];
	     //edttuse[i][j] = g[i] * g[j];
        edlluse[i*nnodes + j] = b[i] * b[j];
	    edttuse[i*nnodes + j] = g[i] * g[j];
      }
    }

    // 1/sqrt(dhds)
    GradH[0] = ( b[0]*haa[0] + b[1]*haa[1] +  b[2]*haa[2] );
    GradH[1] =( g[0]*haa[0] + g[1]*haa[1] +  g[2]*haa[2] );

  } //endif tri

  // dh/ds (dh in the direction of maximum slope)
  dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1]));
  dhds = MMax(1.0e-4,dhds);
  eslope = 1.0 / sqrt(dhds);

  /* Chezy-coefficient: C */
  /* fÃ¼r b >> h gilt: C = H**1/6 n**-1 */
  if (MediaProp->friction_model==2){ // Chezy-coefficient C
    if(MeshElement->geo_type==1){ 
      axx = (eslope * fric) / el;
    }
    if(MeshElement->geo_type==2){ 
      axx = eslope * fric * dy/dx; //ett/ell
      ayy = eslope * fric * dx/dy;
    }
    if(MeshElement->geo_type==4){
      axx = eslope * fric * delt;
      ayy = eslope * fric * delt;
    }
    power = 1.0 / 2.0;

  }
  else{
    if(MeshElement->geo_type==1){ 
      axx = ((eslope / fric) / el);
    }
    if(MeshElement->geo_type==2){ 
      axx = (eslope / fric) * dy/dx; //ett/ell
      ayy = (eslope / fric) * dx/dy;
    }
    if(MeshElement->geo_type==4){ 
      axx = eslope / fric * delt;
      ayy = eslope / fric * delt;
    }
    power = 2.0 / 3.0;
  }

  ast = delt /(double) (dt * nnodes); 

  //Compute constitutive relationships: swval, swold, krwval
  CalcNLTERMS(nnodes, Haa, HaaOld, power, swval, swold, dummy);

  //Compute upstream weighting: ckwr, iups
  CalcCKWR(nnodes, haa, z, power, ckwr, iups);

  //Form elemental matrix
  for (i = 0; i < nnodes; i++){
    for (j = (i+1); j < nnodes; j++){
	  //gammaij = ckwr[i][j] * ((edlluse[i][j] * axx) + (edttuse[i][j]* ayy));
      gammaij = ckwr[i*nnodes+j] * ((edlluse[i*nnodes+j] * axx) + (edttuse[i*nnodes+j]* ayy));
	  amat[i][j]= gammaij;
      amat[j][i]= gammaij;
      amat[i][i]= amat[i][i] - gammaij;
      amat[j][j]= amat[j][j] - gammaij;
    }
  }

  for (i = 0; i < nnodes; i++){
    rhs[i] = -ast * (swval[i] - swold[i]);
  }

  if(MeshElement->geo_type==1){ // line
    astor[0] = swval[0] * ast; 
    astor[1] = swval[1] * ast; 
	rhs[0] = swold[0] * ast * HaaOld[0]; // swval ?????
    rhs[1] = swold[1] * ast * HaaOld[1]; // swval ?????
  }

double test1[16];
for (i = 0; i < nnodes; i++){
  for (j = 0; j < nnodes; j++){
    test1[i*nnodes+j] = amat[i][j];
  }
}

//MZeigMat(test1,nnodes,nnodes,"amat");

  /////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////
  //Jacobian for Newton Raphson
  ////////////////////////////////////////////////
  double epsilon = 1.e-5;
  double head_eps[4];
  double head_keep[4];
  //double oldkrw;
  double oldhead;
  double head[4];
  double sumjac=0.0;
  double flow_depth;
  double akrw;
  double amateps = 0.0;
  double amatold;
  double jacobian[4][4];
  double epsinv;
  double stor_eps;
  double sum;
  double storinit[4];

  double residual[4];
  double swval_eps[4];
  double test[16];


  epsinv = 1 / epsilon;

  //Form the residual excluding the right hand side vector 
  
    for(i=0; i<nnodes; i++)  {
      sum=0.0;
      for(j=0; j<nnodes; j++)  {
	    sum=sum + (amat[i][j]*( haa[j])); 
      }
      storinit[i]= -rhs[i]+astor[i]*Haa[i];
      residual[i]=sum+storinit[i];
    }
  //Compute non-linear terms: krw and Sw using current pressures
  //     plus epsilon (to compute the Jacobian numerically)
  for(i=0; i<nnodes; i++)  {
    head_eps[i] = Haa[i] + epsilon;
    head_keep[i] = Haa[i];
    head[i] = Haa[i];
  }

  CalcNLTERMS(nnodes, head_eps, HaaOld, power, swval_eps, swold, dummy);

  // Form jacobian !
    for(i=0; i<nnodes; i++)  {
    oldhead=Haa[i];
    head[i]=head_eps[i];
    sumjac=0.0;

    for(j=0; j<nnodes; j++)  {
	  if(i!=j) {
          maxZ = MMax(z[i],z[j]);
        if(i==iups[i*nnodes+j])
		  flow_depth = head[i] + z[i] - maxZ;
	    else
		   flow_depth = head[j] + z[j] - maxZ;
     
		   if(flow_depth<0.0) {flow_depth = 0.0;}
		if(MeshElement->geo_type==1) 
	      akrw = width * flow_depth * pow(flow_depth * width / ( 2 * flow_depth + width), power);		
		else 
	      akrw = pow(flow_depth, power + 1);
		
         gammaij= akrw*( axx*edlluse[i*nnodes+j] + ayy*edttuse[i*nnodes+j] );
         
        
	     amateps = gammaij*(head[j]+z[j]-head[i]-z[i]);
	     amatold = amat[i][j] * (head_keep[j] + z[j] - head_keep[i] -z[i]);
	     jacobian[j][i] = -(amateps-amatold) * epsinv;
         
         if(MeshElement->geo_type==1){ // line
           sumjac = sumjac + amateps + swval_eps[i] * ast * (head[i] - HaaOld[i]);
         }
         else{
           sumjac = sumjac + amateps;
         }
         
	   } //end if (i!=j)
    } //end j

    //Compute diagonal for row i 
    //Lump the storage term
    stor_eps=ast*(swval_eps[i]-swold[i]);
    sumjac=sumjac+stor_eps;
    jacobian[i][i]=(sumjac-residual[i])*epsinv ; 
    head[i]=oldhead;
  } // end i

////////ENDHIER???
for (i = 0; i < nnodes; i++){
  for (j = 0; j < nnodes; j++){
    test[i*nnodes+j] = jacobian[i][j];
  }
}
//MZeigMat(test,nnodes,nnodes,"test: jacobian-MATRIX");

// Speichern 
for (i = 0; i < nnodes; i++){
  for (j = 0; j < nnodes; j++){
     (*Laplace)(i,j) = jacobian[i][j];
  }
}


for (i=0;i<nnodes;i++) {
  residual[i]= - residual[i];
  pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]] += residual[i];
  (*RHS)(i+LocalShift) +=  residual[i];
}

for(i=0;i<nnodes;i++){
  for(j=0;j<nnodes;j++){
    MXInc(NodeShift[problem_dimension_dm]+eqs_number[i],\
    NodeShift[problem_dimension_dm]+eqs_number[j],\
    (*Laplace)(i,j));
  }
}

//MZeigMat(test,4,4,"test: jacobian-MATRIX");
//Laplace->Write();
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: Assemby_strainCPL
   Aufgabe:
           Assemble local metrices of strain coupling
           to the global system
 
   Programming:
   01/2005   WW/OK   
   05/2005   WW dyn   
   07/2005   WW Change due to geometry element object
**************************************************************************/
void CFiniteElementStd::Assemble_strainCPL()
{
    int i, j;
    double *u_n = NULL; // Dynamic
    double fac; 
    int Residual = -1;
    fac = 1.0 / dt;
    if(D_Flag != 41)
       Residual = 0;
    else // Mono
    {
       if(pcs_deformation>100) // Pls
         Residual = 1;       
    } 
    if(dynamic)
    { 
       Residual = 2;
       fac = pcs->m_num->GetDynamicDamping_beta1()*dt;  
       u_n = dm_pcs->GetAuxArray();     
    }  

    (*StrainCoupling) = 0.0;
    CalcStrainCoupling(); 
    //	if(D_Flag != 41&&aktueller_zeitschritt>1)
    if(Residual>=0)
    { // Incorparate this after the first time step
       if(Residual==0) // Partitioned
       {
          for (i=0;i<nnodesHQ;i++)
	      {    
             NodalVal2[i] = -fac*(dm_pcs->GetNodeValue(nodes[i],Idx_dm1[0])-dm_pcs->GetNodeValue(nodes[i],Idx_dm0[0]));  
             NodalVal3[i] = -fac*(dm_pcs->GetNodeValue(nodes[i],Idx_dm1[1])-dm_pcs->GetNodeValue(nodes[i],Idx_dm0[1]));
             if(dim==3) // 3D. 
                 NodalVal4[i] = -fac*(dm_pcs->GetNodeValue(nodes[i],Idx_dm1[2])
                                     -dm_pcs->GetNodeValue(nodes[i],Idx_dm0[2]));
	      }
       }
       else if(Residual==1) //Mono
       {
          // du is stored in u_0
          for (i=0;i<nnodesHQ;i++)
	      {   
             NodalVal2[i] = -fac*pcs->GetNodeValue(nodes[i],Idx_dm0[0]);  
             NodalVal3[i] = -fac*pcs->GetNodeValue(nodes[i],Idx_dm0[1]);
             if(dim==3) // 3D. 
                 NodalVal4[i] = -fac*pcs->GetNodeValue(nodes[i],Idx_dm0[2]);
	      }
       }
       else if(Residual==2) //Mono dynamic
       {
          // da is stored in a_0
          // v_{n+1} = v_{n}+a_n*dt+beta1*dt*da 
          // a_n is in dm_pcs->ARRAY
          for (i=0;i<nnodesHQ;i++)
	      {   
             NodalVal2[i] = -(pcs->GetNodeValue(nodes[i],idx_vel_disp[0])
                              +fac*pcs->GetNodeValue(nodes[i],Idx_dm0[0])
                              +u_n[nodes[i]]*dt);  
             NodalVal3[i] = -(pcs->GetNodeValue(nodes[i],idx_vel_disp[1])
                              +fac*pcs->GetNodeValue(nodes[i],Idx_dm0[1])
                              +u_n[nodes[i]+NodeShift[1]]*dt); 
             if(dim==3) // 3D. 
                 NodalVal4[i] = -(pcs->GetNodeValue(nodes[i],idx_vel_disp[2])
                              +fac*pcs->GetNodeValue(nodes[i],Idx_dm0[2])
                              +u_n[nodes[i]+NodeShift[2]]*dt); 

	      }
       }

       for (i=0;i<nnodes; i++)
       {
           NodalVal[i] = 0.0; 
           for (j=0;j<nnodesHQ; j++)
           {
               NodalVal[i] += (*StrainCoupling)(i,j)*NodalVal2[j];
               NodalVal[i] += (*StrainCoupling)(i,j+nnodesHQ)*NodalVal3[j];
               if(dim==3) // 3D. 
                  NodalVal[i] += (*StrainCoupling)(i,j+2*nnodesHQ)*NodalVal4[j];
            }
        }
        // Add RHS
        for (i=0;i<nnodes;i++)
        {
           pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]]
		            += NodalVal[i];
           (*RHS)(i+LocalShift) +=  NodalVal[i];
        }
	 }
     // Monolithic scheme. 
     if(D_Flag == 41)
     { 
        // if Richard, StrainCoupling should be multiplied with -1.
        for(i=0;i<nnodes;i++){
           for(j=0;j<nnodesHQ;j++) {  
             MXInc(NodeShift[problem_dimension_dm] + eqs_number[i],
                eqs_number[j]+NodeShift[0],(*StrainCoupling)(i,j)*fac);
             MXInc(NodeShift[problem_dimension_dm] + eqs_number[i],
                eqs_number[j]+NodeShift[1], (*StrainCoupling)(i,j+nnodesHQ)*fac);
             if(problem_dimension_dm==3)
                 MXInc(NodeShift[problem_dimension_dm] + eqs_number[i],
                    eqs_number[j]+NodeShift[2], (*StrainCoupling)(i,j+2*nnodesHQ)*fac);
           } 
        }
    }
}

/**************************************************************************
FEMLib-Method: 
Task: Assemble local mass matrices to the global system
Programing:
05/2005 PCH Implementation
**************************************************************************/
void CFiniteElementStd::AssembleMassMatrix()
{
	// Calculate matrices
	// Mass matrix..........................................................
	// ---- Gauss integral
	int gp;
	int gp_r=0,gp_s=0,gp_t=0;
	double fkt,mat_fac;
	// Material
	mat_fac = 1.0;

	//----------------------------------------------------------------------
	//======================================================================
	// Loop over Gauss points
	for (gp = 0; gp < nGaussPoints; gp++)
	{	  
		//---------------------------------------------------------
		//  Get local coordinates and weights 
 		//  Compute Jacobian matrix and its determinate
		//---------------------------------------------------------
		fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
		
		// Compute geometry
		ComputeShapefct(1); // Linear interpolation function
	
		// Calculate mass matrix
		for (int i = 0; i < nnodes; i++)
			for (int j = 0; j < nnodes; j++)
			{
				if(j>i) continue;
				(*Mass)(i,j) += fkt *shapefct[i]*shapefct[j];
			}
	}

	// Add local matrix to global matrix
	for(int i=0;i<nnodes;i++)
		for(int j=0;j<nnodes;j++)
			MXInc(eqs_number[i],eqs_number[j],(*Mass)(i,j));
}

/**************************************************************************
FEMLib-Method:
Task: Assemble local matrices to the global system
Programing:
01/2005 WW Implementation
02/2005 OK Richards flow
02/2005 WW Matrix output
03/2005 WW Heat transport
04/2005 OK MSH
05/2005 OK regional PCS
08/2005 OK Air (gas) flow
10/2005 OK DDC
**************************************************************************/
void CFiniteElementStd::Assembly()
{
  int i,j, nn;
  int idx_tr;
  CRFProcess *m_pcs=NULL;  //MX
  CRFProcess *pcs_D = PCSGet("DUAL_RICHARDS");
  //----------------------------------------------------------------------
  //OK index = m_dom->elements[e]->global_number;
  index = Index;
  //----------------------------------------------------------------------
  nn = nnodes;
  if(pcs->type==41||pcs->type==4) nn = nnodesHQ; // ?2WW
  //----------------------------------------------------------------------
  // EQS indices
  for(i=0;i<nn;i++){
    if(dom_vector.size()>0)
    eqs_number[i] = MeshElement->domain_nodes[i];
    else    
      if(pcs->m_msh) {
        eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();
      }
      else 
        eqs_number[i] = GetNodeIndex(nodes[i]);
    }
  //----------------------------------------------------------------------
  // Get room in the memory for local matrices
  SetMemory();
  //----------------------------------------------------------------------
  // Set material
  SetMaterial();
  //----------------------------------------------------------------------
  if((D_Flag==41&&pcs_deformation>100)||dynamic) // ?2WW
    dm_pcs = (process::CRFProcessDeformation*)pcs;
  //----------------------------------------------------------------------
  // Initialize RHS
  if(pcs->Memory_Type>0)
  {
    for(i=LocalShift;i<RHS->Size();i++)
      (*RHS)(i) = 0.0;
  } 
  else
    (*RHS) = 0.0;
  //----------------------------------------------------------------------
  // Node value of the previous time step
  for(i=0;i<nnodes;i++)
  {
    NodalVal0[i] = pcs->GetNodeValue(nodes[i],idx0); 
    NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx1); 
  }
  if(cpl_pcs) // ?2WW: flags are necessary
  {
    for(i=0;i<nnodes;i++)
	{
      NodalValC[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c0); 
      NodalValC1[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c1); 
	}
  }
  //======================================================================
  switch(PcsType){
    //....................................................................
    case L: // Liquid flow
      AssembleParabolicEquation(); 
      Assemble_Gravity();
      if(D_Flag)
         Assemble_strainCPL();
      break;
    //....................................................................
    //case U: // Unconfined flow  //  part of Groundwater flow mmp keyword ($UNCONFINED)
    //....................................................................
    case G: // Groundwater flow
      AssembleParabolicEquation(); 
      //RHS->Write();
      if(D_Flag)
         Assemble_strainCPL();
      break;
    //....................................................................
    case T: // Two-phase flow
      for(i=0;i<nnodes;i++)
        NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
        //AssembleParabolicEquation();
        // Saturation
      break;
    //....................................................................
    case C: // Componental flow
      for(i=0;i<nnodes;i++)
        NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
      break;
    //....................................................................
    case H: // Heat transport
      heat_phase_change = false; // ?2WW 
      if(SolidProp->GetCapacityModel()==2) // Boiling model
        CalNodalEnthalpy();
      if(SolidProp->GetCapacityModel()==3) // D_THM1, bentonite
	  {
        for (i=0; i<(int) pcs_vector.size(); i++){  //MX
	      m_pcs = pcs_vector[i];
	      if(m_pcs->pcs_type_name.compare("RICHARDS_FLOW") == 0){
 //           idxS = m_pcs->GetNodeValueIndex("SATURATION1")+1; //MX
            break;
          }
        }
        for(i=0;i<nnodes;i++)
          NodalVal_Sat[i] = m_pcs->GetNodeValue(nodes[i], idxS);
	  }
        AssembleMixedHyperbolicParabolicEquation(); //CMCD4213 
	  break;
    //....................................................................
      case M: // Mass transport
        AssembleMixedHyperbolicParabolicEquation(); //SB4200
        break;
    //....................................................................
    case O: // Overland flow
      if(pcs->m_num->nls_method == 0) //PICARD
        AssembleParabolicEquation(); //OK
      else
        AssembleParabolicEquationNewton(); //NEWTON
      break;
    //....................................................................
    case R: // Richards flow
      for(i=0;i<nnodes;i++)
        NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
      if(MediaProp->heat_diffusion_model==273)
        CalcRHS_by_ThermalDiffusion(); 
      AssembleParabolicEquation(); //OK
      Assemble_Gravity();
      if(RD_Flag)
      {
        idx_tr = pcs_D->GetNodeValueIndex("TRANSFER")+1;  
        idx_pd = pcs_D->GetNodeValueIndex("PRESSURE_D")+1;
      if(pcs->pcs_type_name.find("RICHARDS_FLOW")!=string::npos){
      for(i=0;i<nnodes;i++)
        NodalVal_P[i] = pcs_D->GetNodeValue(nodes[i], idx_pd)- pcs_D->GetNodeValue(nodes[i], idx_tr);   
        //NodalVal_P[i] = pcs_D->GetNodeValue(nodes[i], idx_pd);   
       }
      if(pcs->pcs_type_name.find("DUAL_RICHARDS")!=string::npos){
      for(i=0;i<nnodes;i++)  
        NodalVal_P[i] = pcs_D->GetNodeValue(nodes[i], idx_tr)-pcs_D->GetNodeValue(nodes[i], idx_pd);                      
        //NodalVal_P[i] = pcs_D->GetNodeValue(nodes[i], idx_tr);    
       }
         Assemble_Transfer();
      }
      if(D_Flag)
        Assemble_strainCPL();
      break;
    //....................................................................
	case F: // Fluid Momentum - Assembly handled in Assembly in Fluid_Momentum file
	  break;
    //....................................................................
    case A: // Air (gas) flow
      AssembleParabolicEquation();
      break;
    //....................................................................
    default:
      cout << "Fatal error: No valid PCS type" << endl;
      break;
  }
  //----------------------------------------------------------------------
  // Irregulaere Knoten eliminieren 
  if(GetRFControlGridAdapt()){
    if(AdaptGetMethodIrrNodes() == 1)
    {
      for(i=0;i<nnodes;i++)
      {
        NodalVal[i] = (*RHS)(i+LocalShift);
        for (j = 0; j < nnodes; j++)
          OldMatrix[i*nnodes+j] = (*StiffMatrix)(i,j);
      }
      switch (ElGetElementType(index)) {
        case 2: DelIrrNodes2D(index,nodes,OldMatrix,NodalVal);
          break;
        case 3: DelIrrNodes3D(index,nodes,OldMatrix,NodalVal);
          break;
	  }
    }
  }
  //----------------------------------------------------------------------
  // Output matrices
  if(pcs->Write_Matrix)
  {
    (*pcs->matrix_file) << "### Element: " << Index << endl;
    (*pcs->matrix_file) << "---Mass matrix: " << endl;
    Mass->Write(*pcs->matrix_file);
    (*pcs->matrix_file) << "---Laplacian matrix: " << endl;
    Laplace->Write(*pcs->matrix_file);
    (*pcs->matrix_file) << "---Advective matrix: " << endl;//CMCD
    Advection->Write(*pcs->matrix_file);
    (*pcs->matrix_file) << "---RHS: " <<endl;
    RHS->Write(*pcs->matrix_file);
    (*pcs->matrix_file) <<endl;
    (*pcs->matrix_file) << "Stiffness: " <<endl;
    StiffMatrix->Write(*pcs->matrix_file);
    (*pcs->matrix_file) <<endl;
  }
}

/**************************************************************************
FEMLib-Method:
Task: Assemble local matrices to the global system
Programing:
01/2005 WW Implementation
02/2005 OK Richards flow
02/2005 WW Matrix output
03/2005 WW Heat transport
08/2005 PCH for Fluid_Momentum
last modification:
**************************************************************************/
void  CFiniteElementStd::Assembly(int dimension)
{
    int i,j, nn;
  //----------------------------------------------------------------------
#ifdef PARALLEL
    index = m_dom->elements[e]->global_number;
#else
    index = Index;
#endif
  //----------------------------------------------------------------------

   nn = nnodes;
// PCH should check the following line carefully.
   if(pcs->type==41||pcs->type==4) nn = nnodesHQ;

   for(i=0;i<nn;i++){
#ifdef PARALLEL
    eqs_number[i] = MeshElement->domain_nodes[i];
#else
    eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();
#endif
   }

    // Get room in the memory for local matrices
    SetMemory();

    // Set material
    SetMaterial();

    // Initialize.
    // if (pcs->Memory_Type==2) skip the these initialization
    (*Mass) = 0.0;
    (*Laplace) = 0.0;
	if(pcs->Memory_Type>0)
	{
        for(i=LocalShift;i<RHS->Size();i++)
           (*RHS)(i) = 0.0;
	} 
	else
    (*RHS) = 0.0;
   
	// Fluid Momentum
	AssembleMassMatrix();		// This is exactly same with CalcMass().
	AssembleRHS(dimension); 

   // Irregulaere Knoten eliminieren 
   if (GetRFControlGridAdapt())
     if (AdaptGetMethodIrrNodes() == 1)
     {
        for (i = 0; i < nnodes; i++)
        {
           NodalVal[i] = (*RHS)(i+LocalShift);
           for (j = 0; j < nnodes; j++)
               OldMatrix[i*nnodes+j] = (*StiffMatrix)(i,j);
        }

         switch (ElGetElementType(index)) {
           case 2: DelIrrNodes2D(index,nodes,OldMatrix,NodalVal);
              break;
           case 3: DelIrrNodes3D(index,nodes,OldMatrix,NodalVal);
              break;
	    }
     }
      
     //Output matrices
     if(pcs->Write_Matrix)
     {
        for (i = 0; i < nnodes; i++)
           (*RHS)(i) = NodalVal[i];

        (*pcs->matrix_file) << "### Element: " << Index << endl;
        (*pcs->matrix_file) << "---Mass matrix: " << endl;
         Mass->Write(*pcs->matrix_file);
        (*pcs->matrix_file) << "---Laplacian matrix: " << endl;
         Laplace->Write(*pcs->matrix_file);
        (*pcs->matrix_file) << "---RHS: " <<endl;
         RHS->Write(*pcs->matrix_file);
        (*pcs->matrix_file) <<endl;
        (*pcs->matrix_file) << "Stiffness: " <<endl;
         StiffMatrix->Write(*pcs->matrix_file);
        (*pcs->matrix_file) <<endl;
     }

}

//WW 08/2007
ElementValue::ElementValue(CRFProcess* m_pcs, CElem* ele):pcs(m_pcs)
{
   int NGPoints=0, NGP = 0;
   int ele_dim, ele_type;

   ele_type = ele->GetElementType();
   ele_dim = ele->GetDimension();
        
   NGP = GetNumericsGaussPoints(ele_type);
   if(ele_type==1)
      NGPoints = m_pcs->m_num->ele_gauss_points; //OKWW
   else if(ele_type==4)
      NGPoints=3;
   else if(ele_type==5)
      NGPoints=15;
   else NGPoints = (int)pow((double)NGP, (double)ele_dim);

   Velocity.resize(m_pcs->m_msh->GetCoordinateFlag()/10, NGPoints);
   Velocity = 0.0;
}
//WW 08/2007
void ElementValue::getIPvalue_vec(const int IP, double * vec)
{
	for(int i=0; i<Velocity.Rows(); i++) vec[i] = Velocity(i, IP);
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
01/2006 YD Implementation
last modification:
**************************************************************************/
void ElementValue::GetEleVelocity(double * vec)
{
	for(int i=0; i<Velocity.Rows(); i++)
    { 
      vec[i] = 0.0;
      for(int j=0; j<Velocity.Cols(); j++) 
        vec[i] += Velocity(i, j);
      vec[i] /= Velocity.Cols();
    }
 }

ElementValue::~ElementValue()
{
   Velocity.resize(0,0);
}

/**************************************************************************
FEMLib-Method: 
01/2006 OK Implementation
**************************************************************************/
//void CFiniteElementStd::AssembleLHSMatrix()
void CFiniteElementStd::AssembleParabolicEquationRHSVector()
{
  int i;
  //----------------------------------------------------------------------
  // TIM
  double dt_inverse = 0.0;
  dt_inverse = 1.0 / dt; 
  //----------------------------------------------------------------------
  // Initialize
  // if (pcs->Memory_Type==2) skip the these initialization
  (*Mass) = 0.0;
  (*Laplace) = 0.0;
  //----------------------------------------------------------------------
  // Calculate matrices
  // Mass matrix..........................................................
  if(pcs->m_num->ele_mass_lumping)
    CalcLumpedMass();
  else
    CalcMass();
  // Laplace matrix.......................................................
  CalcLaplace();
  //----------------------------------------------------------------------
  // Assemble local LHS matrix: 
  // [C]/dt + theta [K]
  //Mass matrix
  *StiffMatrix    = *Mass;
  (*StiffMatrix) *= dt_inverse;
  // Laplace matrix
  *AuxMatrix      = *Laplace;
  *StiffMatrix   += *AuxMatrix;
  //----------------------------------------------------------------------
  for (i=0;i<nnodes; i++)
  {
    NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx1);
    NodalVal[i] = 0.0;
  }
  //----------------------------------------------------------------------
  StiffMatrix->multi(NodalVal1, NodalVal);
  //----------------------------------------------------------------------
  for (i=0;i<nnodes;i++)
  {
    eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();
    pcs->eqs->b[eqs_number[i]] +=  NodalVal[i];
  }
  //----------------------------------------------------------------------
}
/**************************************************************************
FEMLib-Method: 
03/2006 YD Implementation
**************************************************************************/
void  CFiniteElementStd::Assemble_Transfer()
{

  int i;
  double fkt;
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determination
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
      // Material
    fkt *= MediaProp->transfer_coefficient*MediaProp->unsaturated_hydraulic_conductivity  \
          /(pcs->preferential_factor*FluidProp->Density()*gravity_constant) ;
    		  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
	  {
            NodalVal[i] = fkt*NodalVal_P[i];
      }
  }
      for (i=0;i<nnodes;i++)
      {
         pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]]
	   	            += NodalVal[i];
         (*RHS)(i+LocalShift) +=  NodalVal[i];
       } 

  //RHS->Write();
}
}// end namespace
//////////////////////////////////////////////////////////////////////////

using FiniteElement::ElementValue;
vector<ElementValue*> ele_gp_value;

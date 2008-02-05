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
#include "nodes.h"
// Parallel computing
#include "par_ddc.h"
// MSHLib
#include "msh_elem.h"

#include "pcs_dm.h" // displacement coupled
extern double gravity_constant;// TEST, must be put in input file
#define COMP_MOL_MASS_AIR   28.96 // kg/kmol WW  28.96
#define COMP_MOL_MASS_WATER 18.016  //WW 18.016
#define GAS_CONSTANT    8314.41 // J/(kmol*K) WW 
#define GAS_CONSTANT_V  461.5  //WW
#define T_KILVIN_ZERO  273.15  //WW

#include "rfmat_cp.h"
namespace FiniteElement{

//========================================================================
// Element calculation
//========================================================================
/**************************************************************************
   GeoSys - Function: Constructor
   Programmaenderungen:
   01/2005   WW    Erste Version   
**************************************************************************/
CFiniteElementStd:: CFiniteElementStd(CRFProcess *Pcs, const int C_Sys_Flad, const int order)
                   : CElement(C_Sys_Flad, order), phase(0), comp(0), SolidProp(NULL), 
                      FluidProp(NULL), MediaProp(NULL), 
                     pcs(Pcs), dm_pcs(NULL), HEAD_Flag(false)
{
    int i;
    int size_m = 20; //25.2.2007
	string name2;
	char name1[MAX_ZEILE];
    cpl_pcs=NULL; 
    CRFProcess *m_pcs=NULL;  //MX
    //27.2.2007 WW
    newton_raphson = false;
    if(pcs->m_num->nls_method_name.compare("NEWTON_RAPHSON")==0) //WW
      newton_raphson = true;
    Mass = NULL;
    Mass2 = NULL;
    Laplace = NULL;
    Advection = NULL;
    Storage = NULL;
    Content = NULL;
    StrainCoupling = NULL;
    RHS = NULL;
    //
    NodalVal1 = new double [size_m];   
    NodalVal2 = new double [size_m]; 
    NodalVal3 = new double [size_m]; 
    NodalVal4 = new double [size_m]; 
    NodalValC = new double [size_m]; 
    NodalValC1 = new double [size_m]; 
    NodalVal_Sat = new double [size_m]; 
    NodalVal_p2 = new double [size_m]; 
    //
    // 27.2.2007. GravityMatrix = NULL;
    m_dom = NULL;
    eqs_rhs = NULL; //08.2006 WW
    //
    // 12.12.2007 WW
    for(i=0; i<4; i++) NodeShift[i] = 0;
    //
    dynamic = false;
    if(pcs->pcs_type_name_vector.size()&&pcs->pcs_type_name_vector[0].find("DYNAMIC")!=string::npos)
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
    if(pcs->pcs_type_name.find("AIR")!=string::npos) //OK
       pcsT = 'A';
    if(pcs->pcs_type_name.find("MULTI")!=string::npos) // 24.02.2007 WW
       pcsT = 'V';    // Non-isothermal multiphase flow  
    switch(pcsT){
      default:
        PcsType = L;
        //WW GravityMatrix = new  SymMatrix(size_m);
        if(dynamic)
        {
           idx0 = pcs->GetNodeValueIndex("PRESSURE_RATE1");
           idx1 = idx0+1;
           idx_pres = pcs->GetNodeValueIndex("PRESSURE1");
           idx_vel_disp[0] = pcs->GetNodeValueIndex("VELOCITY_DM_X");
           idx_vel_disp[1] = pcs->GetNodeValueIndex("VELOCITY_DM_Y");
           if(dim==3)
              idx_vel_disp[2] = pcs->GetNodeValueIndex("VELOCITY_DM_Z");
        }
		else
		{
          idx0 = pcs->GetNodeValueIndex("PRESSURE1");
          idx1 = idx0+1;
		}
        break;
      case 'L': // Liquid flow
        PcsType = L;
        // 02.2.2007 GravityMatrix = new  SymMatrix(size_m);
        if(dynamic)
        {
           idx0 = pcs->GetNodeValueIndex("PRESSURE_RATE1");
           idx1 = idx0+1;
           idx_pres = pcs->GetNodeValueIndex("PRESSURE1");
           idx_vel_disp[0] = pcs->GetNodeValueIndex("VELOCITY_DM_X");
           idx_vel_disp[1] = pcs->GetNodeValueIndex("VELOCITY_DM_Y");
           if(dim==3)
              idx_vel_disp[2] = pcs->GetNodeValueIndex("VELOCITY_DM_Z");
        }
        else
        {
           idx0 = pcs->GetNodeValueIndex("PRESSURE1");
           idx1 = idx0+1;
        } 
        idx_vel[0] =  pcs->GetNodeValueIndex("VELOCITY_X1");
        idx_vel[1] =  pcs->GetNodeValueIndex("VELOCITY_Y1");
        idx_vel[2] =  pcs->GetNodeValueIndex("VELOCITY_Z1");
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
        // 02.2.2007 GravityMatrix = new  SymMatrix(size_m);
        idx0 = pcs->GetNodeValueIndex("PRESSURE1");
        idx1 = idx0+1;
        idxS = pcs->GetNodeValueIndex("SATURATION1")+1;
        idx_vel[0] =  pcs->GetNodeValueIndex("VELOCITY_X1");
        idx_vel[1] =  pcs->GetNodeValueIndex("VELOCITY_Y1");
        idx_vel[2] =  pcs->GetNodeValueIndex("VELOCITY_Z1");
        if((int)pcs->dof>1) //Dual porosity model. WW
        {
           idxp20 = pcs->GetNodeValueIndex("PRESSURE2");
           idxp21 = idxp20+1;  
           Advection = new Matrix(size_m, size_m);    //WW        
        }
        PcsType = R;
        break;
      case 'A': // Air (gas) flow
        PcsType = A;
        idx0 = pcs->GetNodeValueIndex("PRESSURE1"); //OK
        idx1 = idx0+1; //OK
        break;
	  case 'F':	// Fluid Momentum Process
		PcsType = R;	// R should include L if the eqn of R is written right.
        break;
      case 'V': // 24.02.2007 WW
        // // 02.2.2007 GravityMatrix = new  SymMatrix(size_m);
        idx0 = pcs->GetNodeValueIndex("PRESSURE1");
        idx1 = idx0+1;
        idxp20 = pcs->GetNodeValueIndex("PRESSURE2");
        idxp21 = idxp20+1;             
        idxS = pcs->GetNodeValueIndex("SATURATION1")+1;
        idx_vel[0] =  pcs->GetNodeValueIndex("VELOCITY_X1");
        idx_vel[1] =  pcs->GetNodeValueIndex("VELOCITY_Y1");
        idx_vel[2] =  pcs->GetNodeValueIndex("VELOCITY_Z1");
        PcsType = V;
        size_m = 40;       
        break;
    }
    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
      if(PcsType == V)  
        Mass2 = new Matrix(size_m, size_m);
      else
        Mass = new SymMatrix(size_m);
      Laplace = new Matrix(size_m,size_m);
      if(pcsT=='H'||pcsT=='M')
	  {
         Advection = new Matrix(size_m,size_m);
         Storage = new Matrix(size_m,size_m);
         Content = new Matrix(size_m,size_m);
	  }
      if(D_Flag) 
         StrainCoupling = new Matrix(size_m,60);
      RHS = new Vec(size_m);
    }
    //
    StiffMatrix = new Matrix(size_m, size_m);
    AuxMatrix = new Matrix(size_m, size_m);
    AuxMatrix1 = new Matrix(size_m, size_m);

   	time_unit_factor = pcs->time_unit_factor;
    for(i=0; i<4; i++) NodeShift[i] = 0;
    check_matrices = true;
    //
    SolidProp1 = NULL;
    MediaProp1 = NULL;
    flag_cpl_pcs = false; //OK
    // size_m changed
    NodalVal = new double [size_m]; 
    NodalVal0 = new double [size_m];
}
/**************************************************************************
   GeoSys - Function: Destructor
   Programmaenderungen:
   01/2005   WW    Erste Version
**************************************************************************/
// Destructor
CFiniteElementStd::~CFiniteElementStd()
{
    //  02.2.2007 if(GravityMatrix) delete GravityMatrix;
    // 02.2.2007  GravityMatrix = NULL;
 
    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
       if(Mass) delete Mass;
       if(Mass2) delete Mass2;
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
    // 27.2.2007 WW
    delete [] NodalVal;
    delete [] NodalVal0;   
    delete [] NodalVal1;
    delete [] NodalVal2; 
    delete [] NodalVal3; 
    delete [] NodalVal4; 
    delete [] NodalValC; 
    delete [] NodalValC1; 
    delete [] NodalVal_Sat; 
    delete [] NodalVal_p2; 
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
    int Size=nnodes;
    if(PcsType==V) //24.2.2007 WW
       Size *= 2;
    ElementMatrix * EleMat = NULL;
    // Prepare local matrices
    // If local matrices are not stored, resize the matrix
    if(pcs->Memory_Type==0) 
    {
       if(PcsType==V) //24.2.2007 WW
         Mass2->LimitSize(Size, Size);
       else  
         Mass->LimitSize(Size);
       Laplace->LimitSize(Size, Size);
       if(PcsType==H||PcsType==M)
	   {
	      Advection->LimitSize(Size, Size); //SB4200
	      Storage->LimitSize(Size, Size); //SB4200
	      Content->LimitSize(Size, Size); //SB4209
	   }
       if(PcsType==R&&pcs->type==22) //dual-porosity. WW
          Advection->LimitSize(Size, Size);  
       if(D_Flag>0) 
          StrainCoupling->LimitSize(Size, dim*nnodesHQ);
       RHS->LimitSize(Size);
    }
    else
    {
        EleMat = pcs->Ele_Matrices[Index];
        // if(PcsType==V) //24.2.2007 WW
        // Mass2 = EleMat->GetMass2();
        Mass = EleMat->GetMass();
        Laplace = EleMat->GetLaplace();
		// Advection, Storage, Content SB:Todo ?
        RHS = EleMat->GetRHS();
        if(D_Flag>0) 
           StrainCoupling = EleMat->GetCouplingMatrixB();
		if(D_Flag==41) LocalShift = dim*nnodesHQ; 
    }

    //25.2.2007.WW if(GravityMatrix) GravityMatrix->LimitSize(nnodes); 

    StiffMatrix->LimitSize(Size, Size);
    AuxMatrix->LimitSize(Size, Size);
    AuxMatrix1->LimitSize(Size, Size);
}


/**************************************************************************
   GeoSys - Function: ConfigureCoupling

   Aufgabe:
         Set coupling information for local fem calculation
   Programmaenderungen:
   01/2005   WW    Erste Version
   02/2007   WW    Multi phase flow
   
**************************************************************************/
void  CFiniteElementStd::ConfigureCoupling(CRFProcess* pcs, const int *Shift, bool dyn)
{
  int i;  

  char pcsT; 
  pcsT = pcs->pcs_type_name[0];
  if(pcs->pcs_type_name.find("AIR")!=string::npos) //OK
    pcsT = 'A';
  if(pcs->pcs_type_name.find("MULTI")!=string::npos) //24.2.2007 WW
    pcsT = 'V';



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
      if(cpl_pcs) //WW
	  {
        idx_c0 = cpl_pcs->GetNodeValueIndex("HEAD");
        idx_c1 = idx_c0+1;
	  }
	  else
	  {
         cpl_pcs = PCSGet("LIQUID_FLOW"); 
         if(cpl_pcs == NULL) 
         {
           cpl_pcs = PCSGet("RICHARDS_FLOW"); //OK
           if(cpl_pcs)
             idxS = cpl_pcs->GetNodeValueIndex("SATURATION1")+1; //WW
         } 
         if(cpl_pcs == NULL) 
         {
           cpl_pcs = PCSGet("MULTI_PHASE_FLOW"); //24.042.2004 WW
           if(cpl_pcs)
             idxS = cpl_pcs->GetNodeValueIndex("SATURATION1")+1; //WW
         }
         if (cpl_pcs){  //MX
           idx_c0 = cpl_pcs->GetNodeValueIndex("PRESSURE1");
           idx_c1 = idx_c0+1;
         }
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
      break;
    case 'V': // Multi-phase flow. 24.2.2007 WW
      if(T_Flag) //if(PCSGet("HEAT_TRANSPORT"))
      {
         cpl_pcs = PCSGet("HEAT_TRANSPORT"); 
         idx_c0 = cpl_pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
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
  int mmp_index=0;
  long group = MeshElement->GetPatchIndex();
  mmp_index = group;
  // Single continua thermal:
  if(msp_vector.size()>0)
  {
    SolidProp = msp_vector[mmp_index];
    SolidProp->Fem_Ele_Std = this;//CMCD for Decovalex
  }

  if(pcs->type==22) //WW/YD
  {
     if(pcs->GetContinnumType()== 0) // Matrix //WW
       mmp_index = 2*group;
     else                            // fracture //WW
       mmp_index = 2*group+1;
  }  
  MediaProp = mmp_vector[mmp_index];
  MediaProp->m_pcs = pcs;
  MediaProp->Fem_Ele_Std = this;
  //----------------------------------------------------------------------
  // MSP
  // If dual thermal:
  /*
  if(msp_vector.size()>0)
  {
    SolidProp = msp_vector[mmp_index];
    SolidProp->Fem_Ele_Std = this;//CMCD for Decovalex
  }
  */
  if(pcs->type==22) //WW
  {
     if(pcs->GetContinnumType()== 0) // Matrix //WW
       mmp_index = 2*group+1;
     else                            // fracture //WW
       mmp_index = 2*group;
     MediaProp1 = mmp_vector[mmp_index];
     MediaProp1->m_pcs = pcs;
     MediaProp1->Fem_Ele_Std = this;
     //----------------------------------------------------------------------
     // MSP
     // If dual thermal:
     /*
     if(msp_vector.size()>0)
     {
       SolidProp1 = msp_vector[mmp_index];
       SolidProp1->Fem_Ele_Std = this;//CMCD for Decovalex
     }
     */
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
  if(PCSGet("RICHARDS_FLOW")&&PCSGet("HEAT_TRANSPORT")||pcs->type==1212){
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

/*************************************************************************
FEMLib-Function: 
Task: Line element integration data for CVFEM overland flow  
      to move
Programming: 
     6/2007 : JOD 
**************************************************************************/
void CFiniteElementStd::GetOverlandBasisFunctionMatrix_Line()
{
 
	edlluse[0] = 1.0; 
    edlluse[1] = -1.0; 
    edlluse[2] = -1.0; 
    edlluse[3] = 1.0; 

    edttuse[0] = 0.0;
    edttuse[1] = 0.0;
    edttuse[2] = 0.0;
    edttuse[3] = 0.0;
////MB nur Zeitweise hier 
}
/*************************************************************************
FEMLib-Function: 
Task: Quad element integration data for CVFEM overland flow  
      to move
Programming: 
         ?    MB
**************************************************************************/
void CFiniteElementStd::GetOverlandBasisFunctionMatrix_Quad()
{

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

/**************************************************************************
FEMLib-Method: 
Task: Calculates consitutive relationships for CVFEM Overland Flow -> swval, swold 
      for surface structure
Programing:
06/2005 MB Implementation
04/2007 JOD modifications
**************************************************************************/
void CFiniteElementStd::CalcOverlandNLTERMS(double* haa, double* haaOld, double* swval, double* swold)
{	
	
   if(MediaProp->channel == 1)
     CalcOverlandNLTERMSChannel(haa, haaOld, swval, swold);
   else
     CalcOverlandNLTERMSRills(haa, haaOld, swval, swold);
 

}
/**************************************************************************
FEMLib-Method: 
Task: Calculates consitutive relationships for CVFEM Overland Flow -> swval, swold 
      for surface structure
Programing:
06/2007 JOD implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandNLTERMSRills(double* haa, double* haaOld, double* swval, double* swold)
{
  double WDepth[4], WDepthOld[4];
  double rill_height = MediaProp->rill_height;
  double eps = MediaProp->rill_epsilon;
  
  for(int i=0; i<nnodes; i++)  {
	WDepth[i] = haa[i] - Z[i];
	WDepthOld[i] = haaOld[i] - Z[i]; 
	if (MediaProp->rill_epsilon > 0) {
      if(WDepth[i] > 0)
	    swval[i] = (WDepth[i] + eps)*(WDepth[i] + eps) / ( WDepth[i] + rill_height + eps) - pow(eps, 2.) / (rill_height + eps); 
      else  
        swval[i] = 0;
  
	  if(WDepthOld[i] > 0)
	    swold[i] = (WDepthOld[i] + eps)*(WDepthOld[i] + eps) / ( WDepthOld[i] + rill_height + eps) - pow(eps, 2.) / (rill_height + eps);  // JOD	
	  else  
        swold[i] = 0;
	} // end epsilon > 0 
	else {
      swval[i] = WDepth[i];
      swold[i] = WDepthOld[i];
	}
  }

}

/**************************************************************************
FEMLib-Method: 
Task: Calculates consitutive relationships for CVFEM Overland Flow -> swval, swold 
      for channel
Programing:
06/2007 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandNLTERMSChannel(double* haa, double* haaOld, double* swval, double* swold)
{
    double WDepth[4], WDepthOld[4];
    double eps = MediaProp->rill_epsilon;
    double ratio;
    double xxx;
	
	for(int i=0; i<2; i++)  {
	  WDepth[i] = haa[i] - Z[i];
	  WDepthOld[i] = haaOld[i] - Z[i];
	  if (eps > 0) {
        ratio = WDepth[i] / eps;
        if (ratio > 1.0)
          swval[i] = WDepth[i];
        else if (ratio > 0.0){
          xxx = 2.0 * (1.0 - ratio);
          swval[i] = WDepth[i] * pow(ratio,xxx);
        }
        else
          swval[i] = 0.0;
        ////////////////////////
       
        ratio = WDepthOld[i] / eps;
        if (ratio > 1.0)
          swold[i] = WDepthOld[i];
        else if (ratio > 0.0){
          xxx = 2.0 * (1.0 - ratio);
          swold[i] =  WDepthOld[i] * pow(ratio,xxx);
        }
        else
          swold[i] = 0.0;
	  } // end epsilon > 0
	  else {
      swval[i] = WDepth[i];
      swold[i] = WDepthOld[i];
	  }
    } //end for

}

/**************************************************************************
FEMLib-Method: 
Task: Calculates upstream weighting for CVFEM Overland Flow -> ckwr and iups 
Programing:
06/2005 MB Implementation
04/2007 JOD modifications
**************************************************************************/
void CFiniteElementStd::CalcOverlandCKWR(double* head, double* ckwr, int* iups)
{

  double width = MediaProp->overland_width;
  double depth_exp = MediaProp->friction_exp_depth;
  double rill_depth = MediaProp->rill_height;
  int i, j;
  double maxZ;
  double flow_depth;

  for (i = 0; i < nnodes; i++){
    for (j = 0; j < nnodes; j++){
      maxZ = MMax(Z[i],Z[j]);
	  if( head[i] > head[j] ) {
	    iups[i*nnodes + j] = i;
   	    flow_depth = head[i] - maxZ - rill_depth;
      }
	  else  {
	    iups[i*nnodes + j]= j;
        flow_depth = head[j] - maxZ - rill_depth;
	  }
	  ////////////////////////////////////////
	  if(flow_depth<0.0)  
		ckwr[i*nnodes + j] = 0.0;
	  else { 
        if(MediaProp->channel == 1)
          ckwr[i*nnodes + j] = flow_depth * pow(flow_depth * width / (2 * flow_depth + width),depth_exp);
        else
          ckwr[i*nnodes + j] = pow(flow_depth,depth_exp + 1);
	  }
    } //end for j
  } //end for i
  
}

/**************************************************************************
FEMLib-Method: 
Task: Calculates upstream weighting for CVFEM Overland Flow -> ckwr and iups 
      at node (i,j)
	  used in AssemleParabolicEquationNewtonJacobi()
Programing:
06/2005 MB Implementation
04/2007 JOD modifications
**************************************************************************/
void CFiniteElementStd::CalcOverlandCKWRatNodes(int i, int j, double* head, double* ckwr, int* iups) 
 {
  double width = MediaProp->overland_width;
  double depth_exp = MediaProp->friction_exp_depth;
  double rill_depth = MediaProp->rill_height;
  double flow_depth;
  double maxZ;

  maxZ = MMax(Z[i],Z[j]);
  if(iups[i*nnodes+j] == i)
	  flow_depth = head[i] - maxZ - rill_depth; 
  else
    flow_depth = head[j] - maxZ - rill_depth;
  ///////////////////////////////////////
  if(flow_depth < 0.0) 
	*ckwr = 0;
  else {
   if(MediaProp->channel == 1)
	 *ckwr = flow_depth * pow(flow_depth * width / ( 2 * flow_depth + width), depth_exp);		
   else 
	 *ckwr = pow(flow_depth, depth_exp + 1);
  }

}
/**************************************************************************
FEMLib-Method: 
Task: calculate upwinded diffusion matric coefficient for CVFEM
      used in AssemleParabolicEquationNewton()
	          AssemleParabolicEquationNewtonJacobi()
Programing:
06/2007 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandUpwindedCoefficients(double** amat, double* ckwr, double axx, double ayy) 
{
    //double** amat;
	double gammaij;

    //amat = (double**) Malloc(nnodes * sizeof(double));
    //for (int i = 0; i < nnodes; i++) 
    //  amat[i] = (double*) Malloc(nnodes*sizeof(double));
  
    //for (int i = 0; i < nnodes; i++)
    //  for (int j = 0; j < nnodes; j++)
    //    amat[i][j]= 0.0;


	for (int i = 0; i < nnodes; i++){  
      for (int j = (i+1); j < nnodes; j++){
        gammaij = ckwr[i*nnodes+j] * ((edlluse[i*nnodes+j] * axx) + (edttuse[i*nnodes+j]* ayy));
	    amat[i][j]= gammaij;
        amat[j][i]= gammaij;
        amat[i][i]= amat[i][i] - gammaij;
        amat[j][j]= amat[j][j] - gammaij;
      }
    }

    //return amat;
 
}
/**************************************************************************
FEMLib-Method: 
Task: residual vector for overland CVFEM
      used in AssemleParabolicEquationNewton()
Programing:
06/2007 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandResidual(double* head, double* swval, double* swold, double ast, double* residual, double** amat)
{
  double sum;
  double storinit[4], astor[4], rhs[4];

  MNulleVec(astor,4); 
  MNulleVec(rhs,nnodes);
  
  for (int i = 0; i < nnodes; i++) // storage term
     rhs[i] = -ast * (swval[i] - swold[i]);
	
  
/* if(MediaProp->channel ==1){ // channel, JOD removed, don't know what it was for
    astor[0] = swval[0] * ast; 
    astor[1] = swval[1] * ast; 
	rhs[0] = swold[0] * ast * HaaOld[0]; // swval ?????
    rhs[1] = swold[1] * ast * HaaOld[1]; // swval ?????
  }
*/
   //Form the residual excluding the right hand side vector 
  
    for(int i = 0; i < nnodes; i++)  {
      sum = 0.0;
      for(int j = 0; j < nnodes; j++)  {
	    sum = sum + ( amat[i][j]* head[j] ); 
      }
      storinit[i] = -rhs[i] + astor[i]* (head[i] - Z[i]); // astor = 0, rillDepth??
      residual[i] = sum + storinit[i];
    }

}
/**************************************************************************
FEMLib-Method: 
Task: calcukate jacobi overland CVFEM
      used in  AssemleParabolicEquationNewtonJacobi()
Programing:
06/2007 JOD Implementation
**************************************************************************/
double CFiniteElementStd::CalcOverlandJacobiNodes(int i, int j, double *head, double *headKeep, double akrw, double axx, double ayy, double** amat, double* sumjac )
{
    double jacobi, gammaij, amatEps, amatKeep;

	gammaij= akrw *( axx*edlluse[i*nnodes+j] + ayy*edttuse[i*nnodes+j] );
   	amatEps = gammaij * (head[j] - head[i]);
	amatKeep = amat[i][j] * (headKeep[j] - headKeep[i]);
	jacobi = -(amatEps-amatKeep);
 
	*sumjac = *sumjac + amatEps;
      
    return jacobi;

}

/**************************************************************************
FEMLib-Method: 
Task: calculate topology coefficients for overland CVFEM
Programing:
08/2006 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandCoefficients(double* head, double* axx, double* ayy, double* ast )
{   
   if(MeshElement->geo_type==1){ 
      CalcOverlandCoefficientsLine(head, axx, ast );
      ayy = 0;
	}
    else if(MeshElement->geo_type==2)
      CalcOverlandCoefficientsQuad(head, axx, ayy, ast );
    else if(MeshElement->geo_type==4)
      CalcOverlandCoefficientsTri(head, axx, ayy, ast );
	else
      cout << "Error in CFiniteElementStd::CalcOverlandCoefficients !!!";

}
/**************************************************************************
FEMLib-Method: 
Task:  calculate topology coefficientsfor overland CVFEM, line elements
Programing:
08/2006 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandCoefficientsLine(double* head, double* axx, double* ast )
{

  double dx, dy, dzx;
  double delt, dhds;
  double fric, width, eslope, slope_exp;

  fric =  MediaProp->friction_coefficient;
  slope_exp = MediaProp->friction_exp_slope;
  width = MediaProp->overland_width;
 
  dx = X[1] - X[0];
  dy = Y[1] - Y[0];
  dzx = Z[1] - Z[0];
  delt = sqrt(dx*dx + dy*dy);
  dhds = fabs( (head[0] - head[1]) / delt );
   
  GetOverlandBasisFunctionMatrix_Line();   
 
  dhds = MMax(1.0e-10,dhds); 
  eslope = 1.0 / dhds;
  eslope = pow(eslope, 1 - slope_exp);

  *axx = eslope * fric * width / delt;
  *ast = delt * width /(double) (nnodes * dt); 

}
/**************************************************************************
FEMLib-Method: 
Task:  calculate topology coefficientsfor overland CVFEM, rectangles
Programing:
08/2006 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandCoefficientsQuad(double* head, double* axx, double* ayy, double* ast )
{

  double dx, dy, dzx, dzy;
  double delt;
  double dhds, GradH[2];
  double fric, eslope, slope_exp;

  fric =  MediaProp->friction_coefficient;
  slope_exp = MediaProp->friction_exp_slope;
 
/////////////////////////////
  dx = X[1] - X[0]; //ell
  dy = Y[3] - Y[0]; //ett
  dzx = Z[1] - Z[0];
  dzy = Z[3] - Z[0]; 
  dx = sqrt(dx*dx + dzx*dzx); 
  dy = sqrt(dy*dy + dzy*dzy); 
  delt = dx * dy;
  
  GetOverlandBasisFunctionMatrix_Quad();  
 
  GradH[0] = (head[0] - head[1] - head[2] + head[3]) / (2.0*dx);
  GradH[1] = (head[0] + head[1] - head[2] - head[3]) / (2.0*dy) ;
  dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1])); // dh/ds (dh in the direction of maximum slope)
  dhds = MMax(1.0e-10,dhds); 
  eslope = 1.0 / dhds;
  eslope = pow(eslope, 1 - slope_exp);
  
  *axx = eslope * fric * dy/dx; //ett/ell
  *ayy = eslope * fric * dx/dy;
  *ast = delt /(double) (nnodes * dt ); 

}
/**************************************************************************
FEMLib-Method: 
Task:  calculate topology coefficientsfor overland CVFEM, triangles
Programing:
08/2006 JOD Implementation
**************************************************************************/
void CFiniteElementStd::CalcOverlandCoefficientsTri(double* head, double* axx, double* ayy, double* ast )
{

  double x2, x3, y2, y3;
  double delt, delt2, delt2inv, b[3], g[3];
  double dhds, GradH[2];
  double fric, eslope, slope_exp;

  fric =  MediaProp->friction_coefficient;
  slope_exp = MediaProp->friction_exp_slope;
  
  x2 = X[1] - X[0];
  x3 = X[2] - X[0];
  y2 = Y[1] - Y[0]; 
  y3 = Y[2] - Y[0]; 
  delt = (x2*y3 - x3*y2) * 0.5; 
  delt2 = 2.0 * delt; 
  delt2inv = 1.0 / delt2; 

  /////////////////////  GetOverlandBasisFunctionMatrix_Tri()
  b[0] = (y2-y3) * delt2inv; 
  b[1] = y3 * delt2inv; 
  b[2] = -y2 * delt2inv; 
  g[0] = (x3-x2) * delt2inv; 
  g[1] = -x3 * delt2inv; 
  g[2] = x2 * delt2inv; 

  for(int i=0; i<nnodes; i++)  
    for(int j=0; j<nnodes; j++)  {
      edlluse[i*nnodes + j] = b[i] * b[j];
	  edttuse[i*nnodes + j] = g[i] * g[j];
    }
  //////////////////////////

  GradH[0] = ( b[0]*head[0] + b[1]*head[1] +  b[2]*head[2] );
  GradH[1] =( g[0]*head[0] + g[1]*head[1] +  g[2]*head[2] );
  dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1]));// dh/ds (dh in the direction of maximum slope)
  dhds = MMax(1.0e-10,dhds); 
  eslope = 1.0 / dhds;

  eslope = pow(eslope, 1 - slope_exp);
  *axx = eslope * fric * delt;
  *ayy = eslope * fric * delt;
  *ast = delt /(double) (nnodes * dt ); 
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
01/2007 OK Two-phase flow
**************************************************************************/
inline double CFiniteElementStd::CalCoefMass() 
{
  int Index = MeshElement->GetIndex();
  double val = 0.0;
  double humi = 1.0;
  double rhov = 0.0;
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
#ifdef RESET_4410
      // val = (1/rho*n*d_rho/d_p*S + Se*S )
      if(pcs->pcs_type_number==0)
      {
        //saturation = PCSGetELEValue(ele,gp,theta,nod_val_name);
        Sw = interpolate(NodalVal_Sat);
        val = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta) \
            * FluidProp->drho_dp \
            / FluidProp->Density() \
            * MMax(0.,Sw) \
            + MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) \
            * MMax(0.,Sw);
      }
      if(pcs->pcs_type_number==1)
      {
        val = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta) \
            * MediaProp->geo_area;
      }
#endif
      break;
    case C: // Componental flow
      //OK comp = m_pcs->pcs_type_number;
      //OK coefficient = MPCCalcStorativityNumber(ele,phase,comp,gp);
      break;
    //....................................................................
    case H: // Heat transport
      TG = interpolate(NodalVal1);   
      val = MediaProp->HeatCapacity(Index,unit,pcs->m_num->ls_theta,this); 
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
      PG = interpolate(NodalVal1); //12.02.2007.  Important! WW
      Sw = MediaProp->SaturationCapillaryPressureFunction(-PG,0); //WW
 //     Sw = interpolate(NodalVal_Sat);
      rhow = FluidProp->Density(); 
      dSdp = MediaProp->SaturationPressureDependency(Sw, rhow, pcs->m_num->ls_theta);
      poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
      // Storativity
      val = MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) *Sw;

      // Fluid compressibility
      if(rhow>0.0)
        val += poro  *Sw* FluidProp->drho_dp / rhow;
      // Capillarity
      val += poro * dSdp;
      if(MediaProp->heat_diffusion_model==273) //WW
      {
//  	     PG = fabs(interpolate(NodalVal1));                      
         TG = interpolate(NodalValC)+T_KILVIN_ZERO; 
         //Rv = GAS_CONSTANT;
         humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
         rhov = humi*FluidProp->vaporDensity(TG); 
         //
         val -= poro * rhov*dSdp/rhow;                  
         val += (1.0-Sw)*poro*rhov/(rhow*rhow*GAS_CONSTANT_V*TG);
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
02/2007 WW Multi-phase flow
**************************************************************************/
inline double CFiniteElementStd::CalCoefMass2(int dof_index) 
{
  int Index = MeshElement->GetIndex();
  double val = 0.0;
  double expfactor = 0.0;
  //
  if(pcs->m_num->ele_mass_lumping)
    ComputeShapefct(1); 
  switch(dof_index)
  {
     case 0:  
       PG = interpolate(NodalVal1); // Capillary pressure
       Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
       rhow = FluidProp->Density(); 
       dSdp = -MediaProp->SaturationPressureDependency(Sw, rhow, pcs->m_num->ls_theta);
       poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
       // Storativity
       val = MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) *Sw;
       // Fluid compressibility
       val += poro  *Sw* FluidProp->drho_dp / rhow;
       // Coupled (T)
       if(cpl_pcs)
          TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
       else
          TG = 296.0;
       // Water vapour pressure
       expfactor = COMP_MOL_MASS_WATER/(rhow*GAS_CONSTANT*TG);
       rho_gw = FluidProp->vaporDensity(TG)*exp(-PG*expfactor);
       // 
       val += poro * dSdp*(1.0-rho_gw/rhow);
       //
       val -= (1.0-Sw)*poro*COMP_MOL_MASS_WATER*rho_gw
              /(rhow*GAS_CONSTANT*TG*rhow);                     
       break;
     case 1: //01
       val = 0.0;
       break;
     case 2: //
       // (1-S)n(d rhop_c/d p_c)
       val = (1.0-Sw)*COMP_MOL_MASS_WATER*rho_gw
              /(rhow*GAS_CONSTANT*TG*rhow);        
       p_gw = rho_gw*GAS_CONSTANT*TG/COMP_MOL_MASS_WATER;
       PG2 = interpolate(NodalVal_p2);   
       rho_ga = (PG2-p_gw)*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG);
       val -= rho_ga*dSdp/rhow;
       val *= poro;             
       break;
     case 3: //
       // Water vapour pressure
       val = (1.0-Sw)*poro*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG*rhow);
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
  int Index = MeshElement->GetIndex();
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
		val *= m_cp->CalcElementDecayRateNew(Index, pcs); // Decay rate
		val *= m_cp->CalcElementRetardationFactorNew(Index, unit, pcs); //Retardation Factor
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
		/*
		m_cp = cp_vec[pcs->pcs_component_number]; 
		val *= m_cp->CalcElementRetardationFactorNew(Index, unit, pcs); // Retardation factor
		*/
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
08/2005 OK Air (gas) flow
01/2007 OK Two-phase flow
**************************************************************************/
inline void CFiniteElementStd::CalCoefLaplace(bool Gravity, int ip) 
{
  int i=0;
  double mat_fac = 1.0;
  double Dpv = 0.0;
  double poro = 0.0;
  double tort = 0.0;
  double humi = 1.0;
  double rhow = 0.0; 
  double *tensor = NULL;
  double Hav,manning,chezy,expp,chezy4,Ss,arg;
  static double Hn[9],z[9];
  double GradH[3],Gradz[3],w[3],v1[3],v2[3];
  int nidx1;
  int Index = MeshElement->GetIndex();
//WW  CRFProcess* m_pcs = PCSGet("FLUID_MOMENTUM"); // PCH
  // For nodal value interpolation
  //======================================================================
  switch(PcsType)
  {
      default:
        break;
      case L: // Liquid flow
        tensor = MediaProp->PermeabilityTensor(Index);
        mat_fac = FluidProp->Viscosity();
        if(gravity_constant<MKleinsteZahl) // HEAD version
          mat_fac = 1.0;
        if(HEAD_Flag) mat_fac=1.0;
        if(MediaProp->permeability_stress_mode>1) // Modified LBNL model WW
        {
           if(cpl_pcs)
             TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
           else
             TG = 296.0;
           MediaProp->CalStressPermeabilityFactor(w, TG);
           for(i=0; i<dim; i++)
              tensor[i*dim+i] *= w[i];
        }
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
      //..................................................................
      case T: // Two-phase flow
#ifdef RESET_4410
        //fkt = A / L * permeability_rel * permeability[0] / viscosity_gp;
        if(pcs->pcs_type_number==1&&(!flag_cpl_pcs))
        {
          for(i=0;i<dim*dim;i++)
            mat[i] = 0.0;
          return;
        }
        if(pcs->pcs_type_number==0)
        {
// upwind
if(aktueller_zeitschritt==1)
{
shapefct[0] = 0.; //OK
shapefct[1] = 1.; //OK
}
else
{
shapefct[0] = 1.; //OK
shapefct[1] = 0.; //OK
}
         tensor = MediaProp->PermeabilityTensor(Index);
double k_rel;
if(aktueller_zeitschritt==2)
{
          // phase 1
         idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION1"); //1
         for(i=0;i<nnodes;i++)
           NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
if(aktueller_zeitschritt>=2&&pcs->pcs_type_number==0)
NodalVal_Sat[0]=0.8;
          Sw = interpolate(NodalVal_Sat);
double k_rel = MediaProp->PermeabilitySaturationFunction(Sw,0);
          mat_fac = time_unit_factor * k_rel \
                  / mfp_vector[0]->Viscosity();
          // phase 2
if(aktueller_zeitschritt>1&&pcs->pcs_type_number==0)
{
         idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION2");
         for(i=0;i<nnodes;i++)
           NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
          Sw = interpolate(NodalVal_Sat);
          k_rel = MediaProp->PermeabilitySaturationFunction((Sw),0); //4
          mat_fac += time_unit_factor * k_rel \
                  / mfp_vector[1]->Viscosity();
}
}
else
{
          // phase 1
         idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION1");
         for(i=0;i<nnodes;i++)
           NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
         Sw = interpolate(NodalVal_Sat);
         k_rel = MediaProp->PermeabilitySaturationFunction(Sw,0);
         mat_fac = time_unit_factor * k_rel \
                 / mfp_vector[0]->Viscosity();
          // phase 2
         idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION2");
         for(i=0;i<nnodes;i++)
           NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
         Sw = interpolate(NodalVal_Sat);
         k_rel = MediaProp->PermeabilitySaturationFunction(Sw,1);
         mat_fac += time_unit_factor * k_rel \
                 / mfp_vector[1]->Viscosity();
}
//-------------------------------------------------------------------
          for(i=0;i<dim*dim;i++)
            mat[i] = tensor[i] * mat_fac;
        }
//-------------------------------------------------------------------
//-------------------------------------------------------------------
        else if(pcs->pcs_type_number==1) // for coupling purposes
        {
          if(flag_cpl_pcs)
          {
shapefct[0] = 1.; //OK
shapefct[1] = 0.; //OK
            idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION2");
            for(i=0;i<nnodes;i++)
              NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
            Sw = interpolate(NodalVal_Sat);
            tensor = MediaProp->PermeabilityTensor(Index);
            mat_fac = time_unit_factor* MediaProp->PermeabilitySaturationFunction(Sw,0) \
                    / FluidProp->Viscosity();
            for(i=0;i<dim*dim;i++)
              mat[i] = tensor[i] * mat_fac;
          }
        }
#endif
        break;
      //..................................................................
      case C: // Componental flow
        break;
      case H: // heat transport
        if(SolidProp->GetCapacityModel()==2) // Boiling model. DECOVALEX THM2
        {
           TG = interpolate(NodalVal1); 
           for(i=0; i<dim*dim; i++) mat[i] = 0.0; 
           for(i=0; i<dim; i++) 
             mat[i*dim+i] = SolidProp->Heat_Conductivity(TG);
        }
        else if(SolidProp->GetCapacityModel()==3) // DECOVALEX THM1 
        {
            // WW
            PG = interpolate(NodalValC1); 
            if(cpl_pcs->type!=1212)
              PG *= -1.0;
            Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
            for(i=0; i<dim*dim; i++) mat[i] = 0.0; 
            mat_fac = SolidProp->Heat_Conductivity(Sw); 
            for(i=0; i<dim; i++) 
              mat[i*dim+i] = mat_fac; 
        }  
//WW        else if(SolidProp->GetCapacityModel()==1 && MediaProp->heat_diffusion_model == 273){
        else if(SolidProp->GetCapacityModel()==1){
          tensor = MediaProp->HeatConductivityTensor(Index);
          for(i=0; i<dim*dim; i++) 
            mat[i] = tensor[i]; //mat[i*dim+i] = tensor[i];
        }
        else
        {
          TG = interpolate(NodalVal1); 
          tensor = MediaProp->HeatDispersionTensorNew(ip);
          for(i=0;i<dim*dim;i++) 
            mat[i] = tensor[i];  
 
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
        PG = interpolate(NodalVal1); //05.01.07 WW
        Sw = MediaProp->SaturationCapillaryPressureFunction(-PG,0); //05.01.07 WW

        tensor = MediaProp->PermeabilityTensor(Index);
        mat_fac = time_unit_factor* MediaProp->PermeabilitySaturationFunction(Sw,0) \
                / FluidProp->Viscosity();
        if(MediaProp->permeability_stress_mode>1) // Modified LBNL model WW
        {
           if(cpl_pcs)
             TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
           else
             TG = 296.0;
           MediaProp->CalStressPermeabilityFactor(w, TG);
           for(i=0; i<dim; i++)
              tensor[i*dim+i] *= w[i];
        }
        //
        for(i=0; i<dim*dim; i++)
            mat[i] = tensor[i] * mat_fac;
        if(MediaProp->heat_diffusion_model==273&&!Gravity)
		{
            rhow = FluidProp->Density(); 
			//PG = fabs(interpolate(NodalVal1));                      
			TG = interpolate(NodalValC)+T_KILVIN_ZERO; 
			poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
			tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
            //Rv = GAS_CONSTANT;
            humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
            //
            Dpv = 2.16e-5*tort*(1-Sw)*poro*pow(TG/T_KILVIN_ZERO, 1.8);
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
        nidx1 = pcs->GetNodeValueIndex("PRESSURE1")+1; //OK
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

///////
/**************************************************************************
FEMLib-Method: 
Task: Calculate material coefficient for Laplacian matrix of multi-phase
      flow
Programing:
02/2007 WW Implementation
last modification:
**************************************************************************/
inline void CFiniteElementStd::CalCoefLaplace2(bool Gravity,  int dof_index) 
{
  int i=0;
  double *tensor = NULL;
  double mat_fac = 1.0, m_fac=0.;
  double expfactor, D_gw, D_ga;
  expfactor = D_gw = D_ga =0.0;
  int Index = MeshElement->GetIndex();
  //======================================================================
  for(i=0; i<dim*dim; i++)
     mat[i] = 0.0;
  switch(dof_index)
  {
    case 0:
      PG = interpolate(NodalVal1); 
      Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
      //
      tensor = MediaProp->PermeabilityTensor(Index);
      mat_fac = MediaProp->PermeabilitySaturationFunction(Sw,0) \
                / FluidProp->Viscosity();
      for(i=0; i<dim*dim; i++)
        mat[i] = -tensor[i] * mat_fac*time_unit_factor;
      // For velocity caculation
      if(!Gravity)
      {
        if(cpl_pcs)
          TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
        else
          TG = 296.0;
        //
        PG2 = interpolate(NodalVal_p2);
        rhow = FluidProp->Density(); 
        poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
        tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
        tort *=(1.0-Sw)*poro*2.16e-5*pow(TG/T_KILVIN_ZERO, 1.8);
        expfactor = COMP_MOL_MASS_WATER/(rhow*GAS_CONSTANT*TG);
        rho_gw = FluidProp->vaporDensity(TG)*exp(-PG*expfactor);
        p_gw = rho_gw*GAS_CONSTANT*TG/COMP_MOL_MASS_WATER;
        rho_ga = (PG2-p_gw)*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG);
        rho_g = rho_ga+rho_gw;
        // 1/Mg
        M_g = (rho_gw/COMP_MOL_MASS_WATER+rho_ga/COMP_MOL_MASS_AIR)/rho_g;
        D_gw = tort*rho_g*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g/rhow;
        D_gw *= rho_gw/(rhow*PG2);
        for(i=0; i<dim; i++)
          mat[i*dim+i] -= D_gw*time_unit_factor;
      }
      break;
    case 1:
      if(Gravity)
      {
        if(cpl_pcs)
          TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
        else
          TG = 296.0;
        PG = interpolate(NodalVal1); 
        PG2 = interpolate(NodalVal_p2);
        Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
        rhow = FluidProp->Density(); 
        expfactor = COMP_MOL_MASS_WATER/(rhow*GAS_CONSTANT*TG);
        rho_gw = FluidProp->vaporDensity(TG)*exp(-PG*expfactor);     
        p_gw = rho_gw*GAS_CONSTANT*TG/COMP_MOL_MASS_WATER;
        rho_ga = (PG2-p_gw)*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG);
        rho_g = rho_ga+rho_gw;      
      }
      tensor = MediaProp->PermeabilityTensor(Index);
      mat_fac = MediaProp->PermeabilitySaturationFunction(Sw,0) \
                / FluidProp->Viscosity();
      m_fac = rho_gw*MediaProp->PermeabilitySaturationFunction(Sw,1) \
                / (GasProp->Viscosity()*rhow);   
      if(Gravity)
         mat_fac = mat_fac+m_fac*rho_g/rhow;
      else
        mat_fac += m_fac;
      // 
      for(i=0; i<dim*dim; i++)
        mat[i] = tensor[i] * mat_fac*time_unit_factor;
      //
      if(!Gravity)
      {
        D_gw = tort*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g*rho_g/rhow;
        D_gw *= p_gw/(PG2*PG2);
        for(i=0; i<dim; i++)
          mat[i*dim+i] -= D_gw*time_unit_factor;
      }
      break;
    case 2:
      D_ga = tort*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g*rho_g/rhow;
      D_ga *= rho_gw/(PG2*rhow);      
      for(i=0; i<dim; i++)
        mat[i*dim+i] = D_ga*time_unit_factor;
      break;
    case 3:
      //
      tensor = MediaProp->PermeabilityTensor(Index);
      mat_fac = rho_ga*MediaProp->PermeabilitySaturationFunction(Sw,1) \
                / (GasProp->Viscosity()*rhow);
      //
      if(Gravity)
        mat_fac *= rho_g/rhow;
      //
      for(i=0; i<dim*dim; i++)
        mat[i] = tensor[i] * mat_fac*time_unit_factor;
      if(!Gravity)
      {
         D_ga = tort*rho_g*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g/rhow;
         D_ga *= p_gw/(PG2*PG2);
         for(i=0; i<dim; i++)
           mat[i*dim+i] += D_ga*time_unit_factor;
      }
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
      val = FluidProp->SpecificHeatCapacity()*FluidProp->Density();  
      break;
    case M: // Mass transport //SB4200
		// Get velocity(Gausspoint)/porosity(element)
	  val = 1.0*time_unit_factor; //*MediaProp->Porosity(Index, unit,pcs->m_num->ls_theta); // Porosity; 
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
      mat_fac *= fkt;  
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
      {
         for (j = 0; j < nnodes; j++)
         {
            if(j>i) continue;
            (*Mass)(i,j) += mat_fac *shapefct[i]*shapefct[j];
         }
      }
  }
}
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcMass2
   Programming:
   02/2007   WW   
**************************************************************************/
void CFiniteElementStd::CalcMass2()
{
  int i, j,in,jn;
  // ---- Gauss integral
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt,mat_fac;
  // Material
  int dof_n = 2;
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
      for(in=0; in<dof_n; in++)
      {
         for(jn=0; jn<dof_n; jn++)
         {
           // Material
           mat_fac = CalCoefMass2(in*dof_n+jn);
           mat_fac *= fkt;  
           // Calculate mass matrix
           for (i = 0; i < nnodes; i++)
           {
             for (j = 0; j < nnodes; j++)
                (*Mass2)(i+in*nnodes,j+jn*nnodes) += mat_fac *shapefct[i]*shapefct[j];
           }
         }
      }
  }
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcLumpedMass
   Aufgabe:
           Compute lumped mass matrix, i.e. int (N.mat.N). Linear interpolation
 
   Programming:
   01/2005   WW    
   02/2005 OK GEO factor
   02/2007   WW Multi-phase flow   
   05/2007   WW Axismmetry volume
**************************************************************************/
void CFiniteElementStd::CalcLumpedMass()
{
  int i, gp_r, gp_s, gp_t;
  double factor, vol=0.0;
  gp=0;
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  // Initialize
  (*Mass) = 0.0; 
  // Volume
  if(axisymmetry)
  {  // This calculation should be done in CompleteMesh. 
     // However, in order not to destroy the concise of the code,
     // it is put here. Anyway it is computational cheap. WW
     vol = 0.0;
     for (gp = 0; gp < nGaussPoints; gp++)
     {
        //---------------------------------------------------------
        //  Get local coordinates and weights 
 	    //  Compute Jacobian matrix and its determinate
        //---------------------------------------------------------
        vol += GetGaussData(gp, gp_r, gp_s, gp_t);
     }
  }
  else
    vol = MeshElement->GetVolume();
  // Center of the reference element
  SetCenterGP();
  factor = CalCoefMass();
  pcs->timebuffer = factor;  // Tim Control "Neumann"
  factor *= vol/(double)nnodes;
  for (i=0; i<nnodes; i++)
    (*Mass)(i,i) =  factor;
  //    			
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
     (*Mass)(i+in*nnodes,i+jn*nnodes) += fkt ; 
   }
   //----------------------------------------------------------------------
#endif
  //TEST OUT
 // Mass->Write();
}

///
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcLumpedMass2
   Programming:
   02/2007   WW Multi-phase flow    
**************************************************************************/
void CFiniteElementStd::CalcLumpedMass2()
{
  int i, in, jn, gp_r, gp_s, gp_t;
  double factor, vol=0.0;
  int dof_n = 2;
  //----------------------------------------------------------------------
  // Volume
  if(axisymmetry)
  {  // This calculation should be done in CompleteMesh. 
     // However, in order not to destroy the concise of the code,
     // it is put here. Anyway it is computational cheap. WW
     vol = 0.0;
     for (gp = 0; gp < nGaussPoints; gp++)
     {
        //---------------------------------------------------------
        //  Get local coordinates and weights 
 	    //  Compute Jacobian matrix and its determinate
        //---------------------------------------------------------
        vol += GetGaussData(gp, gp_r, gp_s, gp_t);
     }
  }
  else
    vol = MeshElement->GetVolume();
  //----------------------------------------------------------------------
  // Initialize
  (*Mass2) = 0.0; 
  // Center of the reference element
  SetCenterGP();
  for(in=0; in<dof_n; in++)
  {
    for(jn=0; jn<dof_n; jn++)
    {
      // Factor
      factor = CalCoefMass2(in*dof_n+jn);
      pcs->timebuffer = factor;  // Tim Control "Neumann"
      // Volume
      factor *= vol/(double)nnodes;
      for (i=0; i<nnodes; i++)
        (*Mass2)(i+in*nnodes,i+jn*nnodes) = factor;
    }
  }
  //TEST OUT
 // Mass2->Write();
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
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcLaplace
   Aufgabe:
           Compute mass matrix, i.e. int (gradN.mat.gradN). Linear interpolation
 
   Programming:
   01/2005   WW  
   02/2005 OK GEO factor
   02/2007 WW Multi-phase
**************************************************************************/
void CFiniteElementStd::CalcLaplace()
{
  int i, j, k, l, in, jn;
  // ---- Gauss integral
  int gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt, water_depth; 
  int dof_n = 1;
  if(PcsType==V) dof_n = 2;

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
      ComputeShapefct(1);   // Moved from CalCoefLaplace(). 12.3.2007 WW
      // Calculate mass matrix
      water_depth = 1.0;
      // The following "if" is done by WW
      if(PcsType == G && MediaProp->unconfined_flow_group == 1 && MeshElement->ele_dim == 2 && !pcs->m_msh->cross_section)
      {
         water_depth = 0.0;
         for(i=0; i< nnodes; i++) 
           water_depth += (pcs->GetNodeValue(nodes[i],idx1) -Z[i])*shapefct[i];          
      }
      fkt *= water_depth;
      //---------------------------------------------------------
      for (in = 0; in < dof_n; in++)
      {       
        for (jn = 0; jn < dof_n; jn++)
        {
           // Material
           if(dof_n==1) 
             CalCoefLaplace(false,gp);
           else if (dof_n==2)
             CalCoefLaplace2(false,in*dof_n+jn);       
           //---------------------------------------------------------
           for (i = 0; i < nnodes; i++)
           {
              for (j = 0; j < nnodes; j++)
              {
                 //  if(j>i) continue;  //MB temporary as Laplace now defined unsymmetric
                 for (k = 0; k < dim; k++)
                 {
	                for(l=0; l< dim; l++)
                    {
			           (*Laplace)(i+in*nnodes,j+jn*nnodes) += fkt * dshapefct[k*nnodes+i] \
                           * mat[dim*k+l] * dshapefct[l*nnodes+j];
				    
   		            } 
                 }
              } // j: nodes
           } // i: nodes
        } // dof j
     } // dof i
  }
  //TEST OUTPUT
  // Laplace->Write();
}
/**************************************************************************
FEMLib-Method: 
10/2006 YD Implementation
01/2007 WW Fundamental changes
**************************************************************************/
void CFiniteElementStd:: Assemble_DualTransfer()
{
  int i,j;
  int gp_r=0, gp_s=0, gp_t=0;
  double W, fkt,mat_fac = 0.;
  //Inintialize
  //-------------------------- WW
  W = pcs->continuum_vector[pcs->GetContinnumType()];
  //
  for(i=0;i<nnodes;i++) 
  {
     NodalVal3[i] = pcs->GetNodeValue(nodes[i], idx1);  // Pressure 1 
     NodalVal4[i] = pcs->GetNodeValue(nodes[i], idxp21);  // Pressure 2 
  }
  (*Advection) = 0.0;  
  //---------------------------------------------------------
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determination
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
      mat_fac = CalcCoefDualTransfer();
      mat_fac *= fkt;      
      // Material
      ComputeShapefct(1); // Linear interpolation function
      // Calculate mass matrix
      for (i = 0; i < nnodes; i++)
      {
	     for (j = 0; j < nnodes; j++)
	         (*Advection)(i,j) += mat_fac*shapefct[i]*shapefct[j];
	  }
  }
  // Add local matrix to global matrix
  // 15.02.2007 WW
  long cshift = pcs->eqs->dim/pcs->dof;
  double fm = 1.0/W;
  //
  if(pcs->continuum == 0)
  {
      double ff = 1.0/(1.0-W);
      if(MediaProp->transfer_coefficient<0.0) // for LBNL
        ff = 1.0;      
      for(int i=0;i<nnodes;i++)
      { 
	     for(int j=0;j<nnodes;j++)
         {
             MXInc(eqs_number[i], eqs_number[j]+cshift, -fm*(*Advection)(i,j));
             MXInc(eqs_number[i]+cshift, eqs_number[j], -ff*(*Advection)(i,j));
         }
      }
   }
   else
   {
      if(MediaProp->transfer_coefficient<0.0) // for LBNL
        fm = 1.0;
   }   
   //
   (*Advection) *= fm;
   (*Laplace) += (*Advection); 
   //
   //-------------------------- WW
}
/**************************************************************************
FEMLib-Method: 
10/2006 YD Implementation
01/2007 WW Fundamental changes
**************************************************************************/
inline double  CFiniteElementStd::CalcCoefDualTransfer()
{
  double Sm=0.0, Sf=0.0, ExFac=0.0;
  double pm=0.0, pf=0.0, matrix_conductivity, val=0;
  //double* permeability;
  double *permeability = NULL;
  //-------------------------------------------WW
  CMediumProperties *m_matrix = NULL;
  CMediumProperties *f_matrix = NULL;
  if(pcs->GetContinnumType()== 0) 
  {
     m_matrix = MediaProp;
     f_matrix = MediaProp1;
  }
  else                            // fracture //WW
  {
     m_matrix = MediaProp1;
     f_matrix = MediaProp;
  }
  //-------------------------------------------WW
  switch(PcsType)
  {
    default:
      break;
    case R: 
       pm = interpolate(NodalVal3); 
       pf = interpolate(NodalVal4);
       Sm = m_matrix->SaturationCapillaryPressureFunction(-pm,0);    // Matrix
       Sf = f_matrix->SaturationCapillaryPressureFunction(-pf,0);    // Fracture
       permeability = m_matrix->PermeabilityTensor(Index);
       ExFac = m_matrix->transfer_coefficient;
       // Dual by van Genuchten
       if(ExFac>0.0)
          matrix_conductivity = 0.5*(m_matrix->PermeabilitySaturationFunction(Sm,0)
                                    +m_matrix->PermeabilitySaturationFunction(Sf,0))
                                    /FluidProp->Viscosity();     
       
       else // by LBNL. WW
       {
          double Sf_e = (Sf - f_matrix->saturation_res[phase])/(f_matrix->saturation_max[phase]
                         -f_matrix->saturation_res[phase]);
          matrix_conductivity = Sf_e*m_matrix->PermeabilitySaturationFunction(Sm,0)\
                /FluidProp->Viscosity();  
          ExFac *= -1.0;
       } 
       //   
       val = time_unit_factor*permeability[0]*matrix_conductivity*ExFac;
      break;
    //---------------------------------------------------------
    case H:

      break;
  }
  return val;    
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
   03/2007   WW - Fluid advetion with multiphase flow
**************************************************************************/
void CFiniteElementStd::CalcAdvection()
{
  int i, j, k;
  int gp_r=0, gp_s=0, gp_t;
  double fkt,mat_factor = 0.0;
  double vel[3];
  CFluidProperties *m_mfp_g = NULL;
  bool multiphase = false;
  if(cpl_pcs&&cpl_pcs->type==1212)
  {
     multiphase = true; 
     m_mfp_g =  mfp_vector[1];
  }
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
    vel[0] = mat_factor*gp_ele->Velocity(0, gp);   
    vel[1] = mat_factor*gp_ele->Velocity(1, gp);
    vel[2] = mat_factor*gp_ele->Velocity(2, gp);
    if(multiphase)  //02/2007 WW
    {
       PG2 = interpolate(NodalVal_p2);   
       rho_g = PG2*COMP_MOL_MASS_AIR/(GAS_CONSTANT*(TG+273.15));
       mat_factor = rho_g*m_mfp_g->SpecificHeatCapacity();
       vel[0] += mat_factor*gp_ele->Velocity_g(0, gp);   
       vel[1] += mat_factor*gp_ele->Velocity_g(1, gp);
       vel[2] += mat_factor*gp_ele->Velocity_g(2, gp);
    }
    for (i = 0; i< nnodes; i++){
      for (j = 0; j < nnodes; j++)
         for (k = 0; k < dim; k++)
            (*Advection)(i,j) += fkt*shapefct[i]*vel[k]
                                 *dshapefct[k*nnodes+j];             
    }
  }
  //TEST OUTPUT
  //  cout << "Advection Matrix: " << endl; Advection->Write(); 
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: CalcAdvection
   Aufgabe:  Calculate the advection matrix
 
   Programming:
   12/2005   WW  
***************************************************************************/
void CFiniteElementStd::CalcRHS_by_ThermalDiffusion()
{
  int i, j, k;
  // ---- Gauss integral
  int gp_r=0, gp_s=0, gp_t;
  gp = 0;
  double fkt;
  double Dv = 0.0;
  double Dtv = 0.0;
  double poro = 0.0;
  double tort = 0.0;
  double humi = 1.0;
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
  	     PG = interpolate(NodalVal1);                      
         TG = interpolate(NodalValC)+T_KILVIN_ZERO; 
         Sw = MediaProp->SaturationCapillaryPressureFunction(-PG,0); //WW
         poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
         tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
         beta = poro*MediaProp->StorageFunction(Index,unit,pcs->m_num->ls_theta) *Sw;
         //Rv = GAS_CONSTANT;
         humi = exp(PG/(GAS_CONSTANT_V*TG*rhow));
         Dv = tort*(1.0-Sw)*poro*2.16e-5*pow(TG/T_KILVIN_ZERO, 1.8);
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
         (*RHS)(i) -= (*Laplace)(i,j)*(NodalValC[j]+T_KILVIN_ZERO);
		 (*RHS)(i) += (*Mass)(i,j)*(NodalValC1[j]-NodalValC[j])/dt;
      }
      eqs_rhs[NodeShift[problem_dimension_dm] + eqs_number[i]]
           += (*RHS)(i);
  }

  //TEST OUTPUT
  // Laplace->Write();
  // Mass->Write();
  // RHS->Write();
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementStd:: Coordinates for high order nodes
   Aufgabe:
           Compute the strain couping matrix
 
   Programming:
   02/2007   WW   
**************************************************************************/
void CFiniteElementStd::SetHighOrderNodes()
{
   int i=0;
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
              break;
			case 3:
              for(i=nnodes; i<nnodesHQ; i++)
              {
                 X[i] = MeshElement->nodes[i]->X();  
		         Y[i] = MeshElement->nodes[i]->Y();       
		         Z[i] = MeshElement->nodes[i]->Z();        
	          }
        
		}
   }
   else
   {
      if(dim==1||dim==2)
	  {
          for(i=nnodes; i<nnodesHQ; i++)
          {
             X[i] = MeshElement->nodes[i]->X();  
             Y[i] = MeshElement->nodes[i]->Y();       
             Z[i] = MeshElement->nodes[i]->Z();        
          }
	  }
   } 
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
   double fkt, du=0.0;
   SetHighOrderNodes();
   // Loop over Gauss points
   for (gp = 0; gp < nGaussPoints; gp++)
   {
      for (gp = 0; gp < nGaussPoints; gp++)
      {
         fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

         ComputeGradShapefct(2);
         ComputeShapefct(1);
         ComputeShapefct(2);
         //
         fkt *= CalCoefStrainCouping();
         for(i=0; i<dim; i++ )
		 {
            for (k=0;k<nnodes;k++)
	        {
               for (l=0;l<nnodesHQ;l++) 
               {    
                  kl = nnodesHQ*i+l;                   
                  du  = dshapefctHQ[kl];
                  if(i==0&&axisymmetry) du += shapefctHQ[l]/Radius;
                  (*StrainCoupling)(k, kl) += shapefct[k] * du * fkt;  
               }
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
   08/2006   WW Re-implement   
   02/2007   WW Multi-phase flow   
**************************************************************************/
// Local assembly
void  CFiniteElementStd::Assemble_Gravity()
{
  if((coordinate_system)%10!=2&&(!axisymmetry)){
   // 27.2.2007 WW (*GravityMatrix) = 0.0;
    return;
  }
  int i, ii, k;
  // ---- Gauss integral
  int gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt, rho; //, rich_f;
  double k_rel_iteration;
  // GEO
  double geo_fac = MediaProp->geo_area;
  if(!FluidProp->CheckGravityCalculation()) return; 
  long cshift = 0; //WW 
  int dof_n = 1;  // 27.2.2007 WW 
  if(PcsType==V) dof_n = 2;

  //WW 05.01.07
  cshift = 0;
  if(pcs->dof>1)
    cshift = pcs->continuum*pcs->eqs->dim/pcs->dof;

  //rich_f = 1.0; 
  //if(PcsType==R) rich_f = -1.0; //WW
   
  k_rel_iteration = 1.0;

  for (i = 0; i < dof_n*nnodes; i++)
     NodalVal[i] = 0.0;

  // (*GravityMatrix) = 0.0;
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
      ComputeShapefct(1);   // Moved from CalCoefLaplace(). 12.3.2007 WW
      // Material
      rho = FluidProp->Density();  //Index,unit,pcs->m_num->ls_theta
      if(gravity_constant<MKleinsteZahl) // HEAD version
        rho = 1.0;
      else if(HEAD_Flag) rho = 1.0;
      else
        rho *= gravity_constant; 
      fkt *= rho; //*rich_f;		  
      //
      for(ii=0; ii<dof_n; ii++)
      {
         if(dof_n==1) 
            CalCoefLaplace(true);
         if(dof_n==2) 
            CalCoefLaplace2(true, ii*dof_n+1);
         // Calculate mass matrix
         for (i = 0; i < nnodes; i++)
         {
            for (k = 0; k < dim; k++)
               NodalVal[i+ii*nnodes] -= fkt*dshapefct[k*nnodes+i]
                                        *mat[dim*k+dim-1];
         }
      }
  }
  //
  cshift += NodeShift[problem_dimension_dm]; // 05.01.07 WW
  int ii_sh = 0;
  for(ii=0; ii<dof_n; ii++) // 07.02.07 WW
  {
    cshift += ii*pcs->eqs->dim/pcs->dof; 
    ii_sh = ii*nnodes;
    for (i=0;i<nnodes;i++)
    {
        eqs_rhs[cshift + eqs_number[i]]
                 += k_rel_iteration* geo_fac*NodalVal[i+ii_sh];
       (*RHS)(i+LocalShift+ii_sh) += NodalVal[i+ii_sh];
    }
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
   03/2007   WW  Multi-phase flow     
**************************************************************************/
// Local assembly
void  CFiniteElementStd::Cal_Velocity()
{
  int i, j, k;
  static double vel[3], vel_g[3];  
  // ---- Gauss integral
  int gp_r=0, gp_s=0, gp_t;
  double coef = 0.0;
  int dof_n = 1;
  if(PcsType==V) dof_n = 2;
  //
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
  if(PcsType==V)
  {
    for(i=0; i<nnodes; i++)
    {
       NodalVal[i] -= pcs->GetNodeValue(nodes[i], idxp21);
       NodalVal1[i] = pcs->GetNodeValue(nodes[i], idxp21);
    }
  }
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
      ComputeShapefct(1);   // Moved from CalCoefLaplace(). 12.3.2007 WW
      // Material
      if(dof_n==1) 
        CalCoefLaplace(true);
      else if (dof_n==2)
        CalCoefLaplace2(true,0);       
      // Velocity
      for (i = 0; i < dim; i++)
      {
         vel[i] = 0.0; 
         for(j=0; j<nnodes; j++)         
            vel[i] += NodalVal[j]*dshapefct[i*nnodes+j];
//			 vel[i] += fabs(NodalVal[j])*dshapefct[i*nnodes+j];
      }     
      if(PcsType==V)
      {
         for (i = 0; i < dim; i++)
         {
           vel_g[i] = 0.0; 
           for(j=0; j<nnodes; j++)         
             vel_g[i] += NodalVal1[j]*dshapefct[i*nnodes+j];
         }  
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
               {     
                  vel[i] += coef*(*MeshElement->tranform_tensor)(i, k)
                            *(*MeshElement->tranform_tensor)(2, k);
                  if(PcsType==V)
                     vel_g[i] += rho_g*gravity_constant*(*MeshElement->tranform_tensor)(i, k)
                              *(*MeshElement->tranform_tensor)(2, k);
                  
               }   
            }
         } // To be correctted   
         else
         {
            if(PcsType==V)
            {
               vel[dim-1] -= coef;
               vel_g[dim-1] += gravity_constant*rho_g;
            }
            else
               vel[dim-1] += coef;
         }
      }
      for (i = 0; i < dim; i++)
      {
         for(j=0; j<dim; j++)
//            gp_ele->Velocity(i, gp) -= mat[dim*i+j]*vel[j];  // unit as that given in input file
            gp_ele->Velocity(i, gp) -= mat[dim*i+j]*vel[j]/time_unit_factor;
      }
      //
      if(PcsType==V)
      {
         
         CalCoefLaplace2(true,3); 
         coef = rhow/rho_ga;      
         for (i = 0; i < dim; i++)
         {
           for(j=0; j<dim; j++)
              gp_ele->Velocity_g(i, gp) -= coef*mat[dim*i+j]*vel_g[j]/time_unit_factor;
         }
      }
      //      
  }
  //
  if(pcs->Write_Matrix)
  {
    (*pcs->matrix_file) << "### Element: " << Index << endl;
    (*pcs->matrix_file) << "---Velocity of water " << endl;
    gp_ele->Velocity.Write(*pcs->matrix_file);
    if(gp_ele->Velocity_g.Size()>0)
    {
      (*pcs->matrix_file) << "---Velocity of gas " << endl;
      gp_ele->Velocity_g.Write(*pcs->matrix_file);
    }
  }
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
	int gp_r=0, gp_s=0, gp_t;
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
	int nidx1;
	if(!(m_pcs->pcs_type_name.find("GROUNDWATER_FLOW")!=string::npos))
		nidx1 = m_pcs->GetNodeValueIndex("PRESSURE1")+1;
	else	// then, this is GROUNDWATER_FLOW
	{
		nidx1 = m_pcs->GetNodeValueIndex("HEAD")+1;
		HEAD_Flag = 1;
		PcsType = G;
	}
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
		// Let's get the viscosity too.
		CFluidProperties *FluidProp = mfp_vector[0];
        if(gravity_constant<MKleinsteZahl) // HEAD version
            rho = 1.0;
        else if(HEAD_Flag)
		{
			fkt = fkt*rho * gravity_constant/FluidProp->Viscosity();
			rho = 1.0;
		}
        else
            rho *= gravity_constant; 
//			rho *= gravity_constant/FluidProp->Viscosity();		// This seems to divide viscosity two times. Thus, wrong.

        fktG *= rho;		  
		for (int i = 0; i < nnodes; i++)
			for (int j = 0; j < nnodes; j++)
				for (int k = 0; k < dim; k++)
                {
					NodalVal[i]  -= fkt*dshapefct[dimension*nnodes+j]
							    *mat[dim*dimension+k]* shapefct[i] * NodalVal1[j];
                    NodalVal2[i] += fktG*dshapefct[dimension*nnodes+j]
                                 *mat[dim*dimension+k]* shapefct[i] * MeshElement->nodes[j]->Z();
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
02/2007 WW Mono-scheme for dual porosity flow
02/2007 WW Mult-phase flow
**************************************************************************/
void CFiniteElementStd::AssembleParabolicEquation()
{
  int i,j, ii, jj;
  // NUM
  double relax0, relax1;
  //----------------------------------------------------------------------
  long cshift = 0; //WW 05.01.07
  //WW 05.01.07
  relax0 = pcs->m_num->nls_relaxation; //WW
  relax1 = 1.0;
  if(relax0<DBL_MIN)
    relax0 = 1.0;
  relax1 = 1.0-relax0;
  //
  cshift = 0;
  if(pcs->dof>1)
    cshift = pcs->continuum*pcs->eqs->dim/pcs->dof;
  //----------------------------------------------------------------------
  // Dynamic
// WW: is dynamic really parabolic equation type -> AssembleDynamicEquation
  dynamic = false;
  double *p_n = NULL; 
  double fac1, fac2;
  double beta1 = 0.0;
  if(pcs->pcs_type_name_vector.size()&&pcs->pcs_type_name_vector[0].find("DYNAMIC")==0){ 
    dynamic = true;
    if(pcs->m_num->CheckDynamic()) // why NUM, it is PCS
    beta1  = pcs->m_num->GetDynamicDamping_beta1();
  }
  //----------------------------------------------------------------------
  // Initialize.
  // if (pcs->Memory_Type==2) skip the these initialization
  if(PcsType==V) //WW
    (*Mass2) = 0.0;
  else 
  (*Mass) = 0.0;
  (*Laplace) = 0.0;
  //----------------------------------------------------------------------
  // GEO
  // double geo_fac = MediaProp->geo_area;
  //----------------------------------------------------------------------
  // Calculate matrices
  // Mass matrix..........................................................
  if(PcsType==V) //WW
  {
    if(pcs->m_num->ele_mass_lumping)
      CalcLumpedMass2();
    else
      CalcMass2();
  }
  else
  {
    if(pcs->m_num->ele_mass_lumping)
      CalcLumpedMass();
    else
      CalcMass();
  }
  // Laplace matrix.......................................................
  CalcLaplace();
  if(RD_Flag)      //YD /WW
     Assemble_DualTransfer();
  if(pcs->Tim->time_control_name.find("NEUMANN")!=string::npos)
    pcs->timebuffer /= mat[0]; //YD
  //======================================================================
  // Assemble global matrix
  //----------------------------------------------------------------------
  // Time discretization 
  // ToDo PCS time step
  double dt_inverse = 0.0;
  if(dt<MKleinsteZahl){
    cout<<"\n Zeitschritt ist Null ! Abbruch !"<<endl;
    abort(); //WW. return;
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
    fac1 = dt_inverse; 
    fac2 = relax0; //unterrelaxation WW theta* non_linear_function_iter; //*geo_fac;
  }

  //Mass matrix
  if(PcsType==V) //WW
    *StiffMatrix    = *Mass2;
  else
    *StiffMatrix    = *Mass;
  (*StiffMatrix) *= fac1;
  // Laplace matrix
  *AuxMatrix      = *Laplace;
  (*AuxMatrix)   *= fac2;
  *StiffMatrix   += *AuxMatrix;
  //----------------------------------------------------------------------
  // Add local matrix to global matrix
  if(PcsType==V) // For DOF>1: 27.2.2007 WW
  {
     int ii_sh, jj_sh;
     long i_sh, j_sh=0;
     long g_nodes = pcs->eqs->dim/pcs->dof; 
     for(ii=0;ii<pcs->dof;ii++)
     {
       i_sh = ii*g_nodes+NodeShift[problem_dimension_dm];
       ii_sh = ii*nnodes;
       for(jj=0;jj<pcs->dof;jj++)
       {
         j_sh = jj*g_nodes+NodeShift[problem_dimension_dm];
         jj_sh = jj*nnodes;
         for(i=0;i<nnodes;i++)
         {
           for(j=0;j<nnodes;j++)
           {
              MXInc(i_sh+eqs_number[i], j_sh+eqs_number[j],\
              (*StiffMatrix)(i+ii_sh,j+jj_sh));
           }
         }           
       }
     }   
  }
  else
  {
    cshift += NodeShift[problem_dimension_dm]; //WW 05.01.07
    for(i=0;i<nnodes;i++){
      for(j=0;j<nnodes;j++){
        MXInc(cshift+eqs_number[i], cshift+eqs_number[j],\
           (*StiffMatrix)(i,j));
      }
    }
  }
  //======================================================================
  // Assemble local RHS vector: 
  // ( [C]/dt - (1.0-theta) [K] non_linear_function ) u0  for static problems
  // ( [C] + beta1*dt [K] ) dp  for dynamic problems
  if(dynamic)
  {
    fac1 = -1.0;
    fac2 = beta1*dt;
  }
  else 
  {
    fac1 = dt_inverse; 
    fac2 = relax1; // Unerrelaxation. WW  (1.0-theta) * non_linear_function_t0; //*geo_fac;
  }

  // Mass - Storage
  if(PcsType==V) //WW
    *AuxMatrix1 = *Mass2;
  else
    *AuxMatrix1 = *Mass;
  (*AuxMatrix1) *= fac1;
  //Laplace - Diffusion
  *AuxMatrix     = *Laplace;
  (*AuxMatrix)  *= fac2;
  *AuxMatrix1   -= *AuxMatrix;
  // 07.01.07 WW
  int idx = idx0;
  if(pcs->continuum==1)
    idx = idxp20;
  for (i=0;i<nnodes; i++)
  {
    NodalVal0[i] = pcs->GetNodeValue(nodes[i],idx);
    NodalVal[i] = 0.0;
  }
  if(PcsType==V) // For DOF>1: 27.2.2007 WW
  {
    for (i=0;i<nnodes; i++)
    {
      NodalVal0[i+nnodes] = pcs->GetNodeValue(nodes[i],idxp20);
      NodalVal[i+nnodes] = 0.0;
    }      
  }
  AuxMatrix1->multi(NodalVal0, NodalVal);
  //
  if(dynamic)
  {
    // Velocity of pressure of the previous step 
    p_n = dm_pcs->GetAuxArray();     
    for (i=0;i<nnodes; i++)
      NodalVal0[i] = p_n[nodes[i]+NodeShift[problem_dimension_dm]];
    Mass->multi(NodalVal0, NodalVal, -1.0);
    //p_n+vp*dt 
    for (i=0;i<nnodes; i++)
    {
      NodalVal0[i] *= dt;
      NodalVal0[i] += pcs->GetNodeValue(nodes[i],idx_pres); 
    }
    Laplace->multi(NodalVal0, NodalVal, -1.0);       
  }
  // 
  if(PcsType==V) // For DOF>1: 27.2.2007 WW
  {
    int ii_sh;
    long i_sh;
    long g_nodes = pcs->eqs->dim/pcs->dof; 
    for(ii=0;ii<pcs->dof;ii++)
    {
      i_sh = ii*g_nodes+NodeShift[problem_dimension_dm];
      ii_sh = ii*nnodes;
      for (i=0;i<nnodes;i++)
      {
         eqs_rhs[i_sh + eqs_number[i]] += NodalVal[i+ii_sh];
         (*RHS)(i+LocalShift+ii_sh) +=  NodalVal[i+ii_sh];
      }
    }
  }
  else
  {
    for (i=0;i<nnodes;i++)
    {
      eqs_rhs[cshift + eqs_number[i]] += NodalVal[i];
      (*RHS)(i+LocalShift) +=  NodalVal[i];
    }
  } 
  // 
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
  for (i=0;i<nnodes;i++)
  {
      eqs_rhs[NodeShift[problem_dimension_dm] + eqs_number[i]] += NodalVal[i];
      (*RHS)(i+LocalShift) +=  NodalVal[i];
  }
  //----------------------------------------------------------------------

}
/**************************************************************************
FEMLib-Method: 
Task: Assemble local matrices of parabolic equation to the global system
Comment: Based on hydrosphere, CVFE Method, noch lange nicht allgemein,
Programing:
06/2005 MB Implementation
06/2007 JOD Separation of 1D channel and overland flow
            Introduction of rill depth
			Surface structure with parameter rill_epsilon in st-file 
**************************************************************************/
void CFiniteElementStd::AssembleParabolicEquationNewton()
{
  double haaOld[4], haa[4];
  int nidx;
  double axx = 0, ayy = 0, ast=0.0, ckwr[16];
  double swval[4], swold[4];
  double residual[4];
  double **jacobian ;
  double **amat;
  int iups[16];
  
  jacobian = (double**) Malloc(nnodes * sizeof(double));
  amat = (double**) Malloc(nnodes * sizeof(double));
  for (int i = 0; i < nnodes; i++) 
  {
    jacobian[i] = (double*) Malloc(nnodes*sizeof(double));
    amat[i] = (double*) Malloc(nnodes*sizeof(double));
  }

//////////////////////////// initialize with 0
  MNulleMat(ckwr, nnodes, nnodes);
  MNulleMat(edlluse, nnodes, nnodes);
  MNulleMat(edttuse, nnodes, nnodes);
  for (int i = 0; i < nnodes; i++)
	for (int j = 0; j < nnodes; j++)
	{
      jacobian[i][j] = 0;
      amat[i][j] = 0;
	}
  
 /////////////////////////// fetch head (depth)
  nidx = pcs->GetNodeValueIndex("HEAD");  
 
  for(int i=0;i<nnodes;i++){
    haa[i] = pcs->GetNodeValue(nodes[i],nidx + 1);
    haaOld[i] = pcs->GetNodeValue(nodes[i],nidx);
  }
  ///////////////////////////// assemble upwinded coefficients
  CalcOverlandCoefficients(haa, &axx, &ayy, &ast); 
      // compute axx, ayy, ast  basis functions edlluse, edttuse (element topology (with friction coef and inv. headdiff))
  CalcOverlandNLTERMS(haa, haaOld, swval, swold); 
      // compute swval, swold, introduces surface structure in storage term
  CalcOverlandCKWR(haa, ckwr, iups); 
      //compute ckwr, iups,  upstream weighting, hydraulic radius for channel
  CalcOverlandUpwindedCoefficients(amat, ckwr, axx, ayy); 
      //Form elemental matrix 
  /////////////////////////// form residual vector and jacobi matrix
  CalcOverlandResidual(haa, swval, swold, ast, residual, amat);
  AssembleParabolicEquationNewtonJacobian(jacobian, haa, haaOld, axx, ayy, amat, ast, swold, residual, iups);
  /////////////////////////// store
   for(int i = 0; i < nnodes; i++) {
     pcs->eqs->b[NodeShift[problem_dimension_dm] + eqs_number[i]] -= residual[i];
     for(int j=0;j<nnodes;j++) 
	   MXInc( NodeShift[problem_dimension_dm]+eqs_number[i], NodeShift[problem_dimension_dm]+eqs_number[j], jacobian[i][j] );
   }

   for(int i = 0; i < nnodes; i++) {
    free(jacobian[i]);
    free(amat[i]);
   }
  free(jacobian);
  free(amat);

}


/**************************************************************************
FEMLib-Method: 
Task: Calculates jacobi matrix for AssembleParabolicEquationNewton()
      be carefull with epsilon
Programing:
06/2007 JOD Implementation
**************************************************************************/
void CFiniteElementStd::AssembleParabolicEquationNewtonJacobian(double** jacob, double* haa, double* hOld, double axx, double ayy, double** amat, double ast, double* swold, double* residual, int* iups)
{

 // double** jacob;
  double hEps[4], hKeep[4], swval_eps[4];
  double sumjac, stor_eps, akrw, remember;
  double epsilon = 1.e-7;// be carefull, like in primary variable dependent source terms (critical depth, normal depth)

 /* jacob = (double**) Malloc(nnodes * sizeof(double));
  for (int i = 0; i < nnodes; i++) 
   jacob[i] = (double*) Malloc(nnodes*sizeof(double));
  for (int i = 0; i < nnodes; i++)
    for (int j = 0; j < nnodes; j++)
      jacob[i][j]= 0.0;
*/	
  for(int i = 0; i < nnodes; i++)  {
    hEps[i] = haa[i] + epsilon;
    hKeep[i] = haa[i]; 
  }

  CalcOverlandNLTERMS(hEps, hOld, swval_eps, swold);  
    // compute swval_eps, swold, introduces surface structure in storage term

 
  for(int i = 0; i < nnodes; i++)  { // Form jacobian !
    remember = haa[i];
    haa[i] = hEps[i];
    sumjac = 0.0;

    for(int j = 0; j < nnodes; j++)  {
	  if(i != j) {  // nondiagonal 
         CalcOverlandCKWRatNodes(i, j, haa, &akrw, iups); 
             //compute ckwr, iups,  upstream weighting, hydraulic radius for channel
         jacob[j][i] = CalcOverlandJacobiNodes(i, j, haa, hKeep, akrw, axx, ayy, amat, &sumjac) / epsilon;
		// if(MediaProp->channel ==1)
         // sumjac +=  swval_eps[i] * ast * (Haa[i] - Hold[i]);
      } //end if (i!=j)
    } //end j

    //Compute diagonal for row i, Lump the storage term
    stor_eps = ast * (swval_eps[i] - swold[i]);
    sumjac = sumjac + stor_eps;
    jacob[i][i] = (sumjac - residual[i]) / epsilon ; 
    haa[i] = remember;
  } // end i

 // return jacob;

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
    if(MediaProp->storage_model==7) //RW/WW
      fac *= MediaProp->storage_model_values[0];
    //
    for (i=nnodes;i<nnodesHQ;i++)
       nodes[i] = MeshElement->nodes_index[i];
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
           eqs_rhs[NodeShift[problem_dimension_dm] + eqs_number[i]]
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
06/2005 WW Adjustment in DDC
07/2007 WW Nonisothermal multi-phase flow 
10/2007 OK Two-phase flow
**************************************************************************/
void CFiniteElementStd::Assembly()
{
  int i, nn;
  //----------------------------------------------------------------------
  //OK index = m_dom->elements[e]->global_number;
  index = Index;
  //----------------------------------------------------------------------
  nn = nnodes;
  if(pcs->type==41||pcs->type==4) nn = nnodesHQ; // ?2WW
  //----------------------------------------------------------------------
  // For DDC WW
  eqs_rhs = pcs->eqs->b;
 
  // EQS indices
  if(m_dom) //WW
  {
     eqs_rhs = m_dom->eqs->b;
     for(i=0;i<nn;i++)
		eqs_number[i] = element_nodes_dom[i]; //WW   
  }
  else
  {
     if(pcs->m_msh)
	 {
       for(i=0;i<nn;i++)
          eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();         
	 }
	 else
	 {
       for(i=0;i<nn;i++)
          eqs_number[i] = GetNodeIndex(nodes[i]);        
	 }
  }  
  //----------------------------------------------------------------------
  // Get room in the memory for local matrices
  SetMemory();
  //----------------------------------------------------------------------
  // Set material
  SetMaterial();
  //----------------------------------------------------------------------
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
  int idx00 = idx0;  //----------WW 05.01.07
  int idx11 = idx1; 
  if(pcs->GetContinnumType()==1)
  {
    idx00 = idxp20;
    idx11 = idxp21;
  }
  for(i=0;i<nnodes;i++)
  {
    NodalVal0[i] = pcs->GetNodeValue(nodes[i],idx00); 
    NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx11); 
  }                  //----------WW 05.01.07
  if(PcsType==V) // 25.2.2007
  { 
    for(i=0;i<nnodes;i++)
       NodalVal_p2[i] = pcs->GetNodeValue(nodes[i],idxp21); 
  }                  //----------WW 05.01.07
  if(cpl_pcs) // ?2WW: flags are necessary
  {
    for(i=0;i<nnodes;i++)
    {
      NodalValC[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c0); 
      NodalValC1[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c1); 
      if(cpl_pcs->type==1212)
        NodalVal_p2[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c1+2);        
    }
  }
  //======================================================================
  switch(PcsType){
    //....................................................................
    case L: // Liquid flow
      AssembleParabolicEquation(); 
      Assemble_Gravity();
      if(dm_pcs)
         Assemble_strainCPL();
      break;
    //....................................................................
    //case U: // Unconfined flow  //  part of Groundwater flow mmp keyword ($UNCONFINED)
    //....................................................................
    case G: // Groundwater flow
      AssembleParabolicEquation(); 
      //RHS->Write();
      if(dm_pcs)
         Assemble_strainCPL();
      break;
    //....................................................................
    case T: // Two-phase flow
#ifdef RESET_4410
      idxS = pcs_vector[1]->GetNodeValueIndex("SATURATION1");
      for(i=0;i<nnodes;i++)
        NodalVal_Sat[i] = pcs_vector[1]->GetNodeValue(nodes[i],idxS+1);
      if(pcs->pcs_type_number==0)
      { 
        AssembleParabolicEquation();
      }
      else if(pcs->pcs_type_number==1)
      {
        //MMSCalcElementMatrices(this);
        AssembleParabolicEquation();
        //MMSMakeFluxMatrixEntry()
        AssembleRHSVector();
      }
#endif
      break;
    //....................................................................
    case C: // Componental flow
      for(i=0;i<nnodes;i++)
        NodalVal_Sat[i] = pcs->GetNodeValue(nodes[i], idxS);
      break;
    //....................................................................
    case H: // Heat transport
      heat_phase_change = false; // ?2WW 
      //  if(SolidProp->GetCapacityModel()==2) // Boiling model
      //    CalNodalEnthalpy();
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
      if(MediaProp->heat_diffusion_model==273)
        CalcRHS_by_ThermalDiffusion(); 
      AssembleParabolicEquation(); //OK
      Assemble_Gravity();
      if(dm_pcs)
        Assemble_strainCPL();
      break;
    //....................................................................
	case F: // Fluid Momentum - Assembly handled in Assembly in Fluid_Momentum file
	  break;
    //....................................................................
    case A: // Air (gas) flow
      AssembleParabolicEquation();
      break;
    case V: // Multi-phase flow 24.02.2007 WW
      AssembleParabolicEquation();       
      Assemble_Gravity();
      if(cpl_pcs)
        Assemble_RHS_T_MPhaseFlow(); 
      if(dm_pcs)
        Assemble_RHS_M(); 
      break;

    //....................................................................
    default:
      cout << "Fatal error: No valid PCS type" << endl;
      break;
  }

  //----------------------------------------------------------------------
  // Irregulaere Knoten eliminieren 
  //----------------------------------------------------------------------
  // Output matrices
  if(pcs->Write_Matrix)
  {
    (*pcs->matrix_file) << "### Element: " << Index << endl;
    (*pcs->matrix_file) << "---Mass matrix: " << endl;
    if(Mass)
      Mass->Write(*pcs->matrix_file);
    else if(Mass2)
      Mass2->Write(*pcs->matrix_file);
    (*pcs->matrix_file) << "---Laplacian matrix: " << endl;
    Laplace->Write(*pcs->matrix_file);
    if(Advection)
    {
      (*pcs->matrix_file) << "---Advective matrix: " << endl;//CMCD
      Advection->Write(*pcs->matrix_file);
    }
    if(StrainCoupling)
    {
      (*pcs->matrix_file) << "---Strain couping matrix: " << endl;
      StrainCoupling->Write(*pcs->matrix_file);
    }    
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
    int i,nn;
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
/**************************************************************************
FEMLib-Method:
Task: 
Programing:
18/02/2006 WW Implementation
**************************************************************************/
void CFiniteElementStd::ExtropolateGauss(CRFProcess *m_pcs, const int idof)
{
  int i, j, gp, gp_r, gp_s, gp_t, idx_v2=0;
  int i_s, i_e, ish;
  double EV, EV1=0.0, varx=0.0;
  double r=0.0;
  //
  int ElementType = MeshElement->GetElementType();

  if(m_pcs->type==1212)  // Multi-phase flow
  {
     switch(idof)
     {
       case 0:
         idx_v2 = m_pcs->GetNodeValueIndex("VELOCITY_X2");
         break;
       case 1:
         idx_v2 = m_pcs->GetNodeValueIndex("VELOCITY_Y2");
         break;
       case 2:
         idx_v2 = m_pcs->GetNodeValueIndex("VELOCITY_Z2");
         break;
     }
     
  }
  // For strain and stress extropolation all element types
  // Number of elements associated to nodes
  for(i=0; i<nnodes; i++)
	 dbuff[i] = (double)MeshElement->nodes[i]->connected_elements.size();
  //
  gp_r=gp_s=gp_t=gp=0;
  ElementValue* gp_ele = ele_gp_value[Index];
  // 
  for(gp=0; gp<nGaussPoints; gp++)
  {
      if(ElementType==2||ElementType==3)
      {  
         if(ElementType==2)
         {
            gp_r = (int)(gp/nGauss);
            gp_s = gp%nGauss;
            gp_t = 0;
         } 
         else if(ElementType==3)
         {
            gp_r = (int)(gp/(nGauss*nGauss));
            gp_s = (gp%(nGauss*nGauss));
            gp_t = gp_s%nGauss;
            gp_s /= nGauss;
         }
         i = GetLocalIndex(gp_r, gp_s, gp_t);
         if(i==-1) continue;
      }
      else
        i = gp;
      NodalVal1[i] = gp_ele->Velocity(idof,gp)*time_unit_factor;
      //
      //
      if(m_pcs->type==1212)  // Multi-phase flow
         NodalVal2[i] =gp_ele->Velocity_g(idof,gp)*time_unit_factor;
  }

  if(ElementType==2||ElementType==3)
  {
     Xi_p = 0.0; 
     for (gp = 0; gp < nGauss; gp++)
     {
        r = MXPGaussPkt(nGauss, gp);
        if(fabs(r)>Xi_p) Xi_p = fabs(r);
     }
     r = 1.0/Xi_p;
     Xi_p = r;
   }
   //
   i_s=0;
   i_e=nnodes;
   ish=0;
   if(ElementType==5) // tet
   {
     i_s=1;
     i_e=nnodes+1;
     ish=1;
   } 
   //---------------------------------------------------------
   // Mapping Gauss point strains to nodes and update nodes 
   // strains:
   //---------------------------------------------------------
   for(i=0; i<nnodes; i++)
   { 
      EV = EV1 = varx = 0.0; 
      SetExtropoGaussPoints(i);
      //
      ComputeShapefct(1); // Linear interpolation function
      for(j=i_s; j<i_e; j++)
        EV += NodalVal1[j]*shapefct[j-ish]; 
     // Average value of the contribution of ell neighbor elements 
      EV /= dbuff[i]; 
      EV += m_pcs->GetNodeValue(nodes[i],idx_vel[idof]); 
      m_pcs->SetNodeValue (nodes[i], idx_vel[idof], EV);
      //
      if(m_pcs->type==1212)  // Multi-phase flow
      {
         for(j=i_s; j<i_e; j++)
            EV1 += NodalVal2[j]*shapefct[j-ish];  
         //
         EV1 /= dbuff[i]; 
         EV1 += m_pcs->GetNodeValue(nodes[i],idx_v2); 
         m_pcs->SetNodeValue (nodes[i], idx_v2, EV1);       
      }
      //
   }
}

/***********************************************************************
 27.03.2007 WW
***********************************************************************/
void CFiniteElementStd::CalcSatution()
{
  int i, j, gp, gp_r, gp_s, gp_t, idx_cp, idx_S;
  int i_s, i_e, ish;
  //  int l1,l2,l3,l4; //, counter;
  double sign, eS=0.0;
  double r=0.0;
  // 
  int ElementType = MeshElement->GetElementType();
  //----------------------------------------------------------------------
  // Media
  int mmp_index=0;
  long group = MeshElement->GetPatchIndex();
  mmp_index = group;
  //
  if(pcs->type==22) 
  {
     if(pcs->GetContinnumType()== 0) // Matrix //WW
       mmp_index = 2*group;
     else                            // fracture //WW
       mmp_index = 2*group+1;
  }  
  MediaProp = mmp_vector[mmp_index];
  MediaProp->m_pcs = pcs;
  MediaProp->Fem_Ele_Std = this;
  //
  sign = -1.0;
  idx_cp = pcs->GetNodeValueIndex("PRESSURE1")+1;
  idx_S =  pcs->GetNodeValueIndex("SATURATION1")+1;
  if(pcs->type==22&&pcs->GetContinnumType()==1)  // Dual Richards
  {
     idx_cp = pcs->GetNodeValueIndex("PRESSURE2")+1;
     idx_S =  pcs->GetNodeValueIndex("SATURATION2")+1;
  } 
  if(pcs->type==1212)
    sign = 1.0;
  //
  for(i=0; i<nnodes; i++)
  {
     // Number of elements associated to nodes
     dbuff[i] = (double)MeshElement->nodes[i]->connected_elements.size();
     NodalVal0[i] = sign*pcs->GetNodeValue(nodes[i],idx_cp); // pressure
  } 
  // 
  gp_r=gp_s=gp_t=gp=0;
  // 
  for(gp=0; gp<nGaussPoints; gp++)
  {
     SetGaussPoint(gp, gp_r, gp_s, gp_t);
     if(ElementType==2||ElementType==3)
     {  
       i = GetLocalIndex(gp_r, gp_s, gp_t);
       if(i==-1) continue;
     }
     else
       i = gp;
     // 
     if(i>nnodes) continue;
     ComputeShapefct(1);
     //
     PG = interpolate(NodalVal0); 
     NodalVal_Sat[i] = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
  }

  if(ElementType==2||ElementType==3)
  {
     Xi_p = 0.0; 
     for (gp = 0; gp < nGauss; gp++)
     {
        r = MXPGaussPkt(nGauss, gp);
        if(fabs(r)>Xi_p) Xi_p = fabs(r);
     }
     r = 1.0/Xi_p;
     Xi_p = r;
   }
   //
   i_s=0;
   i_e=nnodes;
   ish=0;
   if(ElementType==5) // tet
   {
     i_s=1;
     i_e=nnodes+1;
     ish=1;
   } 
   //---------------------------------------------------------
   // Mapping Gauss point strains to nodes and update nodes 
   // strains:
   //---------------------------------------------------------
   for(i=0; i<nnodes; i++)
   { 
      eS = 0.0; 
      SetExtropoGaussPoints(i);
      //
      ComputeShapefct(1); // Linear interpolation function
      for(j=i_s; j<i_e; j++)
        eS += NodalVal_Sat[j]*shapefct[j-ish]; 
     // Average value of the contribution of ell neighbor elements 
      eS /= dbuff[i]; 
      eS += pcs->GetNodeValue(nodes[i],idx_S); 
      // In case the node is on the material interface
      if(eS>1.0)
         eS = 1.0;
      //
      pcs->SetNodeValue (nodes[i], idx_S, eS);
   }
}
/**************************************************************************
FEMLib-Method:
Task: Caculate material parameter at element nodes for output
Programing:
04/2007 WW Implementation
**************************************************************************/
void CFiniteElementStd::CalcNodeMatParatemer()
{
  int i, j, k, gp_r, gp_s, gp_t, idx_perm[3], idxp=0;
  int i_s, i_e, ish;
  double w[3], r=0.0, nval=0.0;
  // 
  int ElementType = MeshElement->GetElementType();
  //----------------------------------------------------------------------
  gp = 0;
  index = Index;
  w[0] = w[1] = w[2] = 1.0;
  //----------------------------------------------------------------------
  setOrder(1);
  // Set material
  SetMaterial();
  //----------------------------------------------------------------------
  // Node value of the previous time step
  int idx11 = idx1; 
  if(pcs->GetContinnumType()==1)
    idx11 = idxp21;
  for(i=0;i<nnodes;i++)
     NodalVal1[i] = pcs->GetNodeValue(nodes[i],idx11); 
  if(PcsType==V) 
  { 
    for(i=0;i<nnodes;i++)
       NodalVal_p2[i] = pcs->GetNodeValue(nodes[i],idxp21); 
  }                 
  if(cpl_pcs)
  {
    for(i=0;i<nnodes;i++)
	{
      NodalValC[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c0); 
      NodalValC1[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c1); 
      if(cpl_pcs->type==1212)
        NodalVal_p2[i] = cpl_pcs->GetNodeValue(nodes[i],idx_c1+2);        
	}
  }
  //
  if((pcs->additioanl2ndvar_print>0)&&(pcs->additioanl2ndvar_print<3))
  {
    idx_perm[0] = pcs->GetNodeValueIndex("PERMEABILITY_X1");
    idx_perm[1] = pcs->GetNodeValueIndex("PERMEABILITY_Y1");
    if(dim==3) // 3D   
       idx_perm[2] = pcs->GetNodeValueIndex("PERMEABILITY_Z1");
  }
  if(pcs->additioanl2ndvar_print>1)
     idxp = pcs->GetNodeValueIndex("POROSITY");  
  // Number of elements associated to nodes
  for(i=0; i<nnodes; i++)
     dbuff[i] = (double)MeshElement->nodes[i]->connected_elements.size();
  // 
  gp_r=gp_s=gp_t=gp=0;
  // 
  for(gp=0; gp<nGaussPoints; gp++)
  {
     SetGaussPoint(gp, gp_r, gp_s, gp_t);
     if(ElementType==2||ElementType==3)
     {  
       i = GetLocalIndex(gp_r, gp_s, gp_t);
       if(i==-1) continue;
     }
     else
       i = gp;
     // 
     if(i>nnodes) continue;
     ComputeShapefct(1);
     PG = interpolate(NodalVal1);     
     //
     if((pcs->additioanl2ndvar_print>0)&&(pcs->additioanl2ndvar_print<3))
     {
       double* tensor = MediaProp->PermeabilityTensor(Index);
       if( MediaProp->permeability_stress_mode==2||MediaProp->permeability_stress_mode==3) // Modified LBNL model
       {
           if(cpl_pcs)
             TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
           else
             TG = 296.0;
           MediaProp->CalStressPermeabilityFactor(w, TG);
           for(j=0; j<dim; j++)
              tensor[j*dim+j] *= w[j];
       }
       NodalVal2[i] = tensor[0]; // w[0];
       NodalVal3[i] = tensor[dim+1];// w[1]; //
       if(dim==3) 
         NodalVal4[i] = tensor[2*dim+2];// w[2]; //
     }
     // Porosity
     if(pcs->additioanl2ndvar_print>1)
        NodalVal0[i] = MediaProp->Porosity(MeshElement->index, NULL, 1.0);  //MediaProp->Porosity(this); 
  }
  //
  if(ElementType==2||ElementType==3)
  {
     Xi_p = 0.0; 
     for (gp = 0; gp < nGauss; gp++)
     {
        r = MXPGaussPkt(nGauss, gp);
        if(fabs(r)>Xi_p) Xi_p = fabs(r);
     }
     r = 1.0/Xi_p;
     Xi_p = r;
   }
   //
   i_s=0;
   i_e=nnodes;
   ish=0;
   if(ElementType==5) // tet
   {
     i_s=1;
     i_e=nnodes+1;
     ish=1;
   } 
   //---------------------------------------------------------
   // Mapping Gauss point strains to nodes and update nodes 
   // strains:
   //---------------------------------------------------------
   for(i=0; i<nnodes; i++)
   { 
      SetExtropoGaussPoints(i);
      //
      ComputeShapefct(1); // Linear interpolation function
      if((pcs->additioanl2ndvar_print>0)&&(pcs->additioanl2ndvar_print<3))
      {
        w[0] = w[1] = w[2] = 0.0; 
        for(j=i_s; j<i_e; j++)
        {
           w[0] += NodalVal2[j]*shapefct[j-ish]; 
           w[1] += NodalVal3[j]*shapefct[j-ish]; 
           if(dim==3)
             w[2] += NodalVal4[j]*shapefct[j-ish]; 
        }
        // Average value of the contribution of ell neighbor elements 
        for(k=0; k<dim; k++)
        {
           w[k] /= dbuff[i]; 
           w[k] += pcs->GetNodeValue(nodes[i],idx_perm[k]); 
           //
           pcs->SetNodeValue (nodes[i], idx_perm[k], w[k]);
        }
      }
      if(pcs->additioanl2ndvar_print>1)
      {
         nval = 0.0;
         for(j=i_s; j<i_e; j++)
             nval += NodalVal0[j]*shapefct[j-ish];            
         nval /=  dbuff[i]; 
         nval += pcs->GetNodeValue(nodes[i],idxp); 
         //
         pcs->SetNodeValue (nodes[i], idxp, nval);
      }
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

   //WW Velocity.resize(m_pcs->m_msh->GetCoordinateFlag()/10, NGPoints);
   Velocity.resize(3, NGPoints);
   Velocity = 0.0;
   if(pcs->type ==1212) // 15.3.2007 Multi-phase flow WW
   {
      Velocity_g.resize(3, NGPoints);
      Velocity_g = 0.0;     
   }
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
//WW
ElementValue::~ElementValue()
{
   Velocity.resize(0,0);
   Velocity_g.resize(0,0);
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
  eqs_rhs = pcs->eqs->b; //WW
  for (i=0;i<nnodes;i++)
  {
    eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();
    eqs_rhs[eqs_number[i]] +=  NodalVal[i];
  }
  //----------------------------------------------------------------------
}

///////
/**************************************************************************
FEMLib-Method: 
Task: Calculate  coefficient of temperature induced RHS of multi-phase
      flow
Programing:
02/2007 WW Implementation
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoef_RHS_T_MPhase(int dof_index) 
{
  double val = 0.0, D_gw=0.0, D_ga=0.0;
  double expfactor=0.0;
  int Index = MeshElement->GetIndex();
  ComputeShapefct(1);
  //======================================================================
  switch(dof_index)
  {
    case 0:
      PG = interpolate(NodalVal1); 
      Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
      TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
      TG0 = interpolate(NodalValC)+T_KILVIN_ZERO; 
      PG2 = interpolate(NodalVal_p2);
      rhow = FluidProp->Density(); 
      poro = MediaProp->Porosity(Index,unit,pcs->m_num->ls_theta);
      expfactor = COMP_MOL_MASS_WATER/(rhow*GAS_CONSTANT*TG);
      rho_gw = FluidProp->vaporDensity(TG)*exp(-PG*expfactor);
      // 
      drho_gw_dT = (FluidProp->vaporDensity_derivative(TG)
                +PG*expfactor*FluidProp->vaporDensity(TG)/TG)*exp(-PG*expfactor);
      val = (1.-Sw)*poro*drho_gw_dT/rhow;
      //
      if(SolidProp)  
         val -= (1.0-poro)*((1-Sw)*rho_gw/rhow+Sw)*SolidProp->Thermal_Expansion();
      //
      // val += n*(1.0-rho_gw/rhow)*(dSw/dT)
      val *= (TG-TG0);
      break;
    case 1:
      //
      val = -(1.-Sw)*poro*drho_gw_dT/rhow;
      //
      if(SolidProp)  
         val -= (1.0-poro)*(1-Sw)*rho_ga*SolidProp->Thermal_Expansion()/rhow;
      // 
      // val -= n*rho_ga/rhow)*(dSw/dT)
      //---------------------------------------------------------------
      val *= (TG-TG0);
      break;
    case 2: 
      //------------------------------------------------------------------------
      // From grad (p_gw/p_g)
      tort = MediaProp->TortuosityFunction(Index,unit,pcs->m_num->ls_theta);
      tort *=(1.0-Sw)*poro*2.16e-5*pow(TG/T_KILVIN_ZERO, 1.8);
      //
      p_gw = rho_gw*GAS_CONSTANT*TG/COMP_MOL_MASS_WATER;
      rho_ga = (PG2-p_gw)*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG);
      rho_g = rho_ga+rho_gw;
      // 1/Mg
      M_g = (rho_gw/COMP_MOL_MASS_WATER+rho_ga/COMP_MOL_MASS_AIR)/rho_g;
      D_gw = tort*rho_g*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g/rhow;
      val = D_gw*drho_gw_dT*GAS_CONSTANT*TG/(COMP_MOL_MASS_WATER*PG2)*time_unit_factor;
      break;
    case 3:
      //---------------------------------------------------------------
      // 
      D_ga = tort*rho_g*COMP_MOL_MASS_WATER*COMP_MOL_MASS_AIR*M_g*M_g/rhow;
      // From grad (p_gw/p_g)
      val = -D_ga*drho_gw_dT*GAS_CONSTANT*TG/(COMP_MOL_MASS_WATER*PG2)*time_unit_factor;
      
      break;
      //------------------------------------------------------------------
    }
    return val;
}
/***************************************************************************
   GeoSys - Funktion: 
          Assemble_RHS_T_MPhaseFlow 
   Programming:
   02/2007   WW   
**************************************************************************/
void CFiniteElementStd::Assemble_RHS_T_MPhaseFlow()
{
  int i, j, k, ii;
  // ---- Gauss integral
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt, fac;
  // Material
  int dof_n = 2;
  //----------------------------------------------------------------------
  for (i = 0; i < dof_n*nnodes; i++) NodalVal[i] = 0.0;
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
     ComputeGradShapefct(1); // Linear interpolation function
     ComputeShapefct(1); // Linear interpolation function
     for(ii=0; ii<dof_n; ii++)
     {
        // Material
        fac = fkt*CalCoef_RHS_T_MPhase(ii)/dt;
        // Calculate THS
        for (i = 0; i < nnodes; i++)
           NodalVal[i+ii*nnodes] += fac *shapefct[i];
     }
     // grad T
     for(ii=0; ii<dof_n; ii++)
     {
        // Material
        fac = fkt*CalCoef_RHS_T_MPhase(ii+dof_n);
        // Calculate THS
        for (i = 0; i < nnodes; i++)
        {
           for (j = 0; j < nnodes; j++)
           {
               for (k = 0; k < dim; k++)
                 NodalVal[i+ii*nnodes] += 
                   fac*dshapefct[k*nnodes+i]*dshapefct[k*nnodes+j]
                      *(NodalValC1[j]+T_KILVIN_ZERO);
           }
        }
     }
  }
  int ii_sh;
  long i_sh;
  long g_nodes = pcs->eqs->dim/pcs->dof; 
  for(ii=0;ii<pcs->dof;ii++)
  {
    i_sh = ii*g_nodes+NodeShift[problem_dimension_dm];
    ii_sh = ii*nnodes;
    for (i=0;i<nnodes;i++)
    {
       eqs_rhs[i_sh + eqs_number[i]] -= NodalVal[i+ii_sh];
       (*RHS)(i+LocalShift+ii_sh) -=  NodalVal[i+ii_sh];
    }
  }
//  
}

/***************************************************************************
   GeoSys - Funktion: 
          Assemble_RHS_M
   Programming:
   02/2007   WW   
**************************************************************************/
void CFiniteElementStd::Assemble_RHS_M()
{
  int i, ii;
  // ---- Gauss integral
  int gp_r=0,gp_s=0,gp_t=0;
  double fkt, fac, grad_du=0.0;
  // Material
  int dof_n = 2;
  //----------------------------------------------------------------------
  for (i = 0; i < dof_n*nnodes; i++) NodalVal[i] = 0.0;
  for (i=nnodes;i<nnodesHQ;i++)
    nodes[i] = MeshElement->nodes_index[i];
  for (i=0;i<nnodesHQ;i++)
  {
    NodalVal2[i] = ( dm_pcs->GetNodeValue(nodes[i],Idx_dm1[0])
                    -dm_pcs->GetNodeValue(nodes[i],Idx_dm0[0]));  
    NodalVal3[i] = ( dm_pcs->GetNodeValue(nodes[i],Idx_dm1[1])
                    -dm_pcs->GetNodeValue(nodes[i],Idx_dm0[1]));
    if(dim==3) // 3D. 
        NodalVal4[i] = ( dm_pcs->GetNodeValue(nodes[i],Idx_dm1[2])
                        -dm_pcs->GetNodeValue(nodes[i],Idx_dm0[2]));
  }     
  //======================================================================
  SetHighOrderNodes();
  //
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
     //ComputeShapefct(2); 
     ComputeGradShapefct(2); 
     grad_du = 0.0;
     for (i=0;i<nnodesHQ;i++)
     {
        grad_du += dshapefctHQ[i]*NodalVal2[i]+dshapefctHQ[i+nnodesHQ]*NodalVal3[i];
        if(dim==3) // 3D. 
           grad_du += dshapefctHQ[i+nnodesHQ*2]*NodalVal4[i];
     }
     grad_du /= dt;
     for(ii=0; ii<dof_n; ii++)
     {
        // Material
        fac = fkt*grad_du*CalCoef_RHS_M_MPhase(ii);
        // Calculate MHS
        for (i = 0; i < nnodes; i++)
           NodalVal[i+ii*nnodes] += fac *shapefct[i];
     }
  }
  //
  int ii_sh;
  long i_sh;
  long g_nodes = pcs->eqs->dim/pcs->dof; 
  for(ii=0;ii<pcs->dof;ii++)
  {
    i_sh = ii*g_nodes+NodeShift[problem_dimension_dm];
    ii_sh = ii*nnodes;
    for (i=0;i<nnodes;i++)
    {
       eqs_rhs[i_sh + eqs_number[i]] -= NodalVal[i+ii_sh];
       (*RHS)(i+LocalShift+ii_sh) -=  NodalVal[i+ii_sh];
    }
  }
  setOrder(1);
//  
}
///////
/**************************************************************************
FEMLib-Method: 
Task: Calculate  coefficient of displacement induced RHS of multi-phase
      flow
Programing:
02/2007 WW Implementation
last modification:
**************************************************************************/
inline double CFiniteElementStd::CalCoef_RHS_M_MPhase(int dof_index) 
{
  double val = 0.0;
  double expfactor=0.0;
  //======================================================================
  switch(dof_index)
  {
    case 0:
      PG = interpolate(NodalVal1); 
      Sw = MediaProp->SaturationCapillaryPressureFunction(PG,0); 
      if(cpl_pcs)
        TG = interpolate(NodalValC1)+T_KILVIN_ZERO; 
      else
        TG = 296.0;
      //
      PG2 = interpolate(NodalVal_p2);
      rhow = FluidProp->Density(); 
      expfactor = COMP_MOL_MASS_WATER/(rhow*GAS_CONSTANT*TG);
      rho_gw = FluidProp->vaporDensity(TG)*exp(-PG*expfactor);
      p_gw = rho_gw*GAS_CONSTANT*TG/COMP_MOL_MASS_WATER;
      rho_ga = (PG2-p_gw)*COMP_MOL_MASS_AIR/(GAS_CONSTANT*TG);
      val = Sw+(1.0-Sw)*rho_gw/rhow;
      break;
    case 1:
      val =  (1.0-Sw)*rho_ga/rhow;
      break;
      //------------------------------------------------------------------
    }
    return val;
}

//

/**************************************************************************
PCSLib-Method: 
01/2007 OK Implementation
**************************************************************************/

/**************************************************************************
PCSLib-Method: 
01/2007 OK Implementation
**************************************************************************/
void CFiniteElementStd::AssembleRHSVector()
{
  int i;
  int idx_fv=0;
  double NodalVal_FV[20]; 
  double FV;
  CRFProcess* m_pcs_cpl = NULL;
  //----------------------------------------------------------------------
  // Initializations
  for(i=0;i<nnodes;i++)
  {
    NodalVal[i] = 0.0;
  }
  switch(PcsType)
  {
    //....................................................................
    case T: // Two-phase flow
      (*Laplace) = 0.0;
      break;
    //....................................................................
  }
  //----------------------------------------------------------------------
  // Field variables
  switch(PcsType)
  {
    //....................................................................
    case T: // Two-phase flow
      m_pcs_cpl = pcs_vector[0];
      idx_fv = m_pcs_cpl->GetNodeValueIndex("PRESSURE1");
      break;
    //....................................................................
  }
  for(i=0;i<nnodes;i++)
  {
    NodalVal_FV[i] = m_pcs_cpl->GetNodeValue(nodes[i],idx_fv+1); 
  } 
  FV = interpolate(NodalVal_FV);
  //----------------------------------------------------------------------
  // Element matrices
  switch(PcsType)
  {
    //....................................................................
    case T: // Two-phase flow
      flag_cpl_pcs = true;
      CalcLaplace(); // PCS[0]
      flag_cpl_pcs = false;
      break;
    //....................................................................
  }
  //----------------------------------------------------------------------
  // Calc RHS contribution
  switch(PcsType)
  {
    //....................................................................
    case T: // Two-phase flow
      Laplace->multi(NodalVal_FV,NodalVal);
      break;
    //....................................................................
  }
  //----------------------------------------------------------------------
  // Store RHS contribution
  for(i=0;i<nnodes;i++)
  {
    pcs->eqs->b[NodeShift[problem_dimension_dm]+eqs_number[i]] -= NodalVal[i];
    (*RHS)(i+LocalShift) -= NodalVal[i];
  } 
  //----------------------------------------------------------------------
  //RHS->Write();
}

}// end namespace
//////////////////////////////////////////////////////////////////////////

using FiniteElement::ElementValue;
vector<ElementValue*> ele_gp_value;

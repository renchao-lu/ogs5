/*
   The members of class Element definitions.
   Designed and programmed by WW, 06/2004
*/

#include "stdafx.h"                    /* MFC */
#include "makros.h"
#include <iostream>
// Sytem matrix
#include "matrix.h"
//#include "nodes.h"
#include "mathlib.h"
#include "pcs_dm.h"
#include "rf_msp_new.h"
//Time step
#include "rf_tim_new.h"
extern double gravity_constant;
// MSHLib
#include "msh_elem.h"
// FEMLib
#include "fem_ele.h"
#include "fem_ele_vec.h"

#include "rf_pcs.h"
std::vector<FiniteElement::ElementValue_DM*> ele_value_dm;

namespace FiniteElement{

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// class Element_DM

//  Constructor of class Element_DM 
CFiniteElementVec::CFiniteElementVec(process::CRFProcessDeformation *dm_pcs, const int C_Sys_Flad, const int order)
 :CElement(C_Sys_Flad, order), pcs(dm_pcs)
{
    int i;
    h_pcs = NULL;
    t_pcs = NULL;

    for(i=0; i<4; i++)
       NodeShift[i] = pcs->Shift[i];
    if(dm_pcs->pcs_type_name.find("DYNAMIC")!=string::npos)
    {
       // Indecex in nodal value table
       Idx_dm0[0] = pcs->GetNodeValueIndex("ACCELERATION_X1");
       Idx_dm0[1] = pcs->GetNodeValueIndex("ACCELERATION_Y1");
       Idx_dm1[0] = Idx_dm0[0]+1;
       Idx_dm1[1] = Idx_dm0[1]+1; 
       Idx_Vel[0] = pcs->GetNodeValueIndex("VELOCITY_X1"); 
       Idx_Vel[1] = pcs->GetNodeValueIndex("VELOCITY_Y1"); 
       //     if(problem_dimension_dm==3)
       if(dim==3)
       {
          Idx_dm0[2] = pcs->GetNodeValueIndex("ACCELERATION_Z1");
          Idx_dm1[2] = Idx_dm1[2]+1;
          Idx_Vel[2] = pcs->GetNodeValueIndex("VELOCITY_Z1"); 
       }   
       Mass = new SymMatrix(20);
       dAcceleration = new Vec(60); 
        
       beta2 = dm_pcs->m_num->GetDynamicDamping_beta2();
       bbeta1 = dm_pcs->m_num->GetDynamicDamping_bbeta();
       dynamic = true;
    }
    else
    {
       dynamic = false;
       dAcceleration = NULL; 
       Idx_Vel[0] = Idx_Vel[1] = Idx_Vel[2] = -1;
       beta2 = bbeta1 = 1.0;
       // Indecex in nodal value table
       Idx_dm0[0] = pcs->GetNodeValueIndex("DISPLACEMENT_X1");
       Idx_dm0[1] = pcs->GetNodeValueIndex("DISPLACEMENT_Y1");
       Idx_dm1[0] = Idx_dm0[0]+1;
       Idx_dm1[1] = Idx_dm0[1]+1; 

       //     if(problem_dimension_dm==3)
       if(dim==3)
       {
          Idx_dm0[2] = pcs->GetNodeValueIndex("DISPLACEMENT_Z1");
          Idx_dm1[2] = Idx_dm0[2]+1;
       }  
    }

    idx_pls =  pcs->GetNodeValueIndex("STRAIN_PLS");
    // Strain
    Idx_Strain[0] = pcs->GetNodeValueIndex("STRAIN_XX");
    Idx_Strain[1] = pcs->GetNodeValueIndex("STRAIN_YY");
    Idx_Strain[2] = pcs->GetNodeValueIndex("STRAIN_ZZ");
    Idx_Strain[3] = pcs->GetNodeValueIndex("STRAIN_XY");
    // Stress
    Idx_Stress[0] = pcs->GetNodeValueIndex("STRESS_XX");
    Idx_Stress[1] = pcs->GetNodeValueIndex("STRESS_YY");
    Idx_Stress[2] = pcs->GetNodeValueIndex("STRESS_ZZ");
    Idx_Stress[3] = pcs->GetNodeValueIndex("STRESS_XY");

    idx_S = -1;
	// Saturation;
	S_Water = 1.0;
    //
	switch(dim)
	{
	   case 2:
         ns =4;
         B_matrix = new Matrix(4,2);
         B_matrix_T = new Matrix(2,4);
		 dstress = new double[4];
		 dstrain = new double[4];
         De = new Matrix(4,4);
         ConsistDep = new Matrix(4,4);
         AuxMatrix = new Matrix(2,2);
		 Disp = new double[18];
		 Temp = new double[9];

		 Sxx = new double[9];
		 Syy = new double[9];
		 Szz = new double[9];
		 Sxy = new double[9];
		 pstr = new double[9];

         Sxz = NULL;
         Syz = NULL;

         if(enhanced_strain_dm)
		 {
            NodesInJumpedA = new bool[9];
			Ge = new Matrix(4,2);
			Pe = new Matrix(2,4);
			PeDe = new Matrix(2,4);   
            
            BDG = new Matrix(2,18);
            PDB = new Matrix(18,2);
            DtD = new Matrix(2,2);
		 }
         break;
	   case 3:
         ns = 6;
         B_matrix = new Matrix(6,3);
         B_matrix_T = new Matrix(3,6);
		 dstress = new double[6];
		 dstrain = new double[6];
         De = new Matrix(6,6);
         ConsistDep = new Matrix(6,6);
         AuxMatrix = new Matrix(3,3);
		 Disp = new double[60];
		 Temp = new double[20];

		 Sxx = new double[20];
		 Syy = new double[20];
		 Szz = new double[20];
		 Sxy = new double[20];
		 Sxz = new double[20];
		 Syz = new double[20];
		 pstr = new double[20];
   
         // Indecex in nodal value table
         Idx_Strain[4] = pcs->GetNodeValueIndex("STRAIN_XZ");
         Idx_Strain[5] = pcs->GetNodeValueIndex("STRAIN_YZ");
         //
         Idx_Stress[4] = pcs->GetNodeValueIndex("STRESS_XZ");
         Idx_Stress[5] = pcs->GetNodeValueIndex("STRESS_YZ");
         break;
	}
    *B_matrix = 0.0;
    *B_matrix_T = 0.0;

    PressureC = NULL;
    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
        Stiffness = new Matrix(60,60);
        RHS = new Vec(60);
        if(H_Process)
           PressureC = new Matrix(60,20);
    }
    else   // Local matrices stored and allocated the pcs.cpp;
    {
        Stiffness = NULL;
        RHS = NULL;
    }
 
    // Material properties
    smat = NULL;
    // Fluid coupling
    m_mfp = NULL;
    // Medium property
    m_mmp = NULL; // 

    // Coupling
    Flow_Type = -1;
    idx_P = -1;
    for(int i=0;i<(int)pcs_vector.size();i++){
      if(pcs_vector[i]->pcs_type_name.find("FLOW")!=string::npos)
      {
         h_pcs = pcs_vector[i];
         if(GetRFProcessNumPhases()==1) Flow_Type = 0; 
         if(h_pcs->pcs_type_name.find("RICHARDS")!=string::npos)
            Flow_Type = 1;
         else if  (GetRFProcessNumPhases()==2) Flow_Type = 2;
         idx_P0 = pcs->GetNodeValueIndex("POROPRESSURE0");
         break;
      }
    }
    if(Flow_Type==0)
    {
       idx_P1 = h_pcs->GetNodeValueIndex("PRESSURE1")+1;
       if(dynamic)
       {
          idx_P = h_pcs->GetNodeValueIndex("PRESSURE1")+1;
          idx_P1 = h_pcs->GetNodeValueIndex("PRESSURE_RATE1");
       }
    }
    else if(Flow_Type==1)
    {
       idx_P1 = h_pcs->GetNodeValueIndex("PRESSURE1")+1;
       idx_P1_0 = h_pcs->GetNodeValueIndex("PRESSURE1");
       idx_S0 = h_pcs->GetNodeValueIndex("SATURATION1");
       idx_S = h_pcs->GetNodeValueIndex("SATURATION1")+1;
    }
    else if(Flow_Type==2)
    {
       idx_P1 = h_pcs->GetNodeValueIndex("PRESSURE1")+1;
       idx_P2 = h_pcs->GetNodeValueIndex("PRESSURE2")+1;
       idx_S0 = h_pcs->GetNodeValueIndex("SATURATION2");
       idx_S = h_pcs->GetNodeValueIndex("SATURATION2")+1;
    }
 
    for(int i=0;i<(int)pcs_vector.size();i++){
      if(pcs_vector[i]->pcs_type_name.find("HEAT")!=string::npos)
      {
         t_pcs = pcs_vector[i];
         break;
	  }
	}
    if(T_Flag)
    {
       idx_T0 =  t_pcs->GetNodeValueIndex("TEMPERATURE1"); 
       idx_T1 =  idx_T0 +1;
    }
	//
    // Time unit factor
  	time_unit_factor = pcs->time_unit_factor;
}

//  Constructor of class Element_DM 
CFiniteElementVec::~CFiniteElementVec()
{
	delete B_matrix;
	delete B_matrix_T;
    delete dstress;
    delete dstrain;
	delete De;
    delete ConsistDep;
	delete AuxMatrix;
    delete Disp;
    delete Temp;
    delete Sxx;
    delete Syy;
    delete Szz;
    delete Sxy;
    delete pstr;
    if(Sxz) delete Sxz;
	if(Syz) delete Syz;
	
    if(dynamic) 
    {
       delete Mass;
       delete dAcceleration; 
       Mass = NULL;
       dAcceleration = NULL;
    }

    if(pcs->Memory_Type==0)  // Do not store local matrices
    {
        delete Stiffness;
        delete RHS;
        if(F_Flag) delete PressureC;
        Stiffness = NULL;
        RHS = NULL;
        PressureC = NULL;
    }

    if(enhanced_strain_dm)
    {
        delete NodesInJumpedA;
        delete Ge;
        delete Pe;
        delete PeDe;
        delete BDG;
        delete PDB;
        delete DtD;

        NodesInJumpedA = NULL;
		Ge = NULL;
		Pe = NULL;
		PeDe = NULL;
		BDG = NULL;
        PDB = NULL;
        DtD = NULL;
    }

    B_matrix = NULL;
    B_matrix_T = NULL;
    dstress = NULL;
    dstrain = NULL;
	De = NULL;
    ConsistDep = NULL;
	AuxMatrix = NULL;
    Disp = NULL;
    Temp = NULL;
    Sxx = NULL;
    Syy = NULL;
    Szz = NULL;
    Sxy = NULL;
    Sxz = NULL;
    Syz = NULL;
    pstr = NULL;
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::SetMaterial(const int EleIndex)

   Aufgabe:
         Set material data for local assembly 
   Formalparameter:
           E: 
 
   Programming:
   11/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::SetMaterial()
{
   // Get material properties
   //
   int MatGroup = MeshElement->GetPatchIndex();
   smat = msp_vector[MatGroup];
   smat->axisymmetry = pcs->m_msh->isAxisymmetry();
   // Single yield surface model   
   if(smat->Plasticity_type==2)  smat->ResizeMatricesSYS(ele_dim); 

   if(F_Flag)
      m_mfp = MFPGet("LIQUID"); // YD

   m_mmp = mmp_vector[MatGroup];
}



/**************************************************************************
   GeoSys - Function: SetMemory

   Aufgabe:
         Set memory for local matrices
   Programmaenderungen:
   01/2005   WW    Erste Version
   05/2005   WW    Dynamic analysis
   
**************************************************************************/
void CFiniteElementVec::SetMemory()
{
    int size =0;
    ElementMatrix * EleMat = NULL;
   
    // Prepare local matrices
    if(pcs->Memory_Type==0) 
    {
       // If local matrices are not stored, resize the matrix
       size = dim*nnodesHQ;
       Stiffness->LimitSize(size, size);
       if(F_Flag) 
          PressureC->LimitSize(size, nnodes);
       RHS->LimitSize(size);

    }
    else
    {
        EleMat = pcs->Ele_Matrices[Index];
        Stiffness = EleMat->GetStiffness();
        RHS = EleMat->GetRHS();
        if(F_Flag) 
           PressureC = EleMat->GetCouplingMatrixA();
    }

    if(dynamic)
    {
       Mass->LimitSize(nnodesHQ);
       dAcceleration->LimitSize(nnodesHQ*dim);
    } 

}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: setB_Matrix(const int LocalIndex)

   Aufgabe:
          Form B matric     
   Formalparameter:
           E: 
             const int LocalIndex   : Local node index
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::setB_Matrix(const int LocalIndex)
{
     switch(dim)
	 {
	     case 2:
            // B_11, dN/dx
			(*B_matrix)(0,0) = dshapefctHQ[LocalIndex]; 
            // B_12, 0.0
			(*B_matrix)(0,1) = 0.0;

			if(axisymmetry) // Axisymmtry
			{
               // B_21, N/r
               (*B_matrix)(1,0) = shapefctHQ[LocalIndex]/Radius;      
               // B_22, 0.0
		       (*B_matrix)(1,1) = 0.0; 
               // B_31, 0.0
			   (*B_matrix)(2,0) = 0.0; 
               // B_32, dN/dz
			   (*B_matrix)(2,1) = dshapefctHQ[nnodesHQ+LocalIndex];

			}
			else
			{
               // B_21, 0.0
		       (*B_matrix)(1,0) = 0.0;      
               // B_22, dN/dy
		       (*B_matrix)(1,1) = dshapefctHQ[nnodesHQ+LocalIndex]; 
               // B_31, 0.0
			   (*B_matrix)(2,0) = 0.0; 
               // B_32, 0.0
			   (*B_matrix)(2,1) = 0.0;
			}
            // B_41, dN/dy
			(*B_matrix)(3,0) = dshapefctHQ[nnodesHQ+LocalIndex];
            // B_42, dN/dx
			(*B_matrix)(3,1) = dshapefctHQ[LocalIndex]; 

			break;
	     case 3:
            // B_11, dN/dx
			(*B_matrix)(0,0) = dshapefctHQ[LocalIndex]; 
            // B_22, dN/dy
			(*B_matrix)(1,1) = dshapefctHQ[nnodesHQ+LocalIndex];
            // B_33, dN/dz
			(*B_matrix)(2,2) = dshapefctHQ[2*nnodesHQ+LocalIndex];
            //
            // B_41, dN/dy
			(*B_matrix)(3,0) = dshapefctHQ[nnodesHQ+LocalIndex];
            // B_42, dN/dx
			(*B_matrix)(3,1) = dshapefctHQ[LocalIndex]; 
            //
            // B_51, dN/dz
			(*B_matrix)(4,0) = dshapefctHQ[2*nnodesHQ+LocalIndex];
            // B_53, dN/dx
			(*B_matrix)(4,2) = dshapefctHQ[LocalIndex]; 
            //
            // B_62, dN/dz
			(*B_matrix)(5,1) = dshapefctHQ[2*nnodesHQ+LocalIndex];
            // B_63, dN/dy
			(*B_matrix)(5,2) = dshapefctHQ[nnodesHQ+LocalIndex]; 

			break;

	 }

}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: setTransB_Matrix(const int LocalIndex)

   Aufgabe:
          Form the tanspose of B matric
   Formalparameter:
           E: 
             const int LocalIndex   : Local node index
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::setTransB_Matrix(const int LocalIndex)
{
 
    setB_Matrix(LocalIndex);
    B_matrix->GetTranspose(*B_matrix_T);
}



/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ComputeStrain()

   Aufgabe:
          Compute strains
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::ComputeStrain()
{
  int i, j=0, k=0;
  switch(dim)
  {
    case 2:
       for(i=0; i<ns; i++)
          dstrain[i] = 0.0;
       if(axisymmetry)
       {
          for(i=0; i<nnodesHQ; i++)
	      {
             j = i+nnodesHQ;
             dstrain[0] += Disp[i]*dshapefctHQ[i];
             dstrain[1] += Disp[i]*shapefctHQ[i];
             dstrain[2] += Disp[j]*dshapefctHQ[j];
             dstrain[3] += Disp[i]*dshapefctHQ[j]
                          +Disp[j]*dshapefctHQ[i];
		  }
          dstrain[1] /= Radius;         
       }
       else
       {
          for(i=0; i<nnodesHQ; i++)
          {
             j = i+nnodesHQ;
             dstrain[0] += Disp[i]*dshapefctHQ[i];
             dstrain[1] += Disp[j]*dshapefctHQ[j];
             dstrain[3] += Disp[i]*dshapefctHQ[j]
                           +Disp[j]*dshapefctHQ[i];
          }
       }
       break;
     case 3:
       for(i=0; i<ns; i++)
         dstrain[i] = 0.0;
       for(i=0; i<nnodesHQ; i++)
       {
          j = i+nnodesHQ;
          k = i+2*nnodesHQ;
          dstrain[0] += Disp[i]*dshapefctHQ[i];
          dstrain[1] += Disp[j]*dshapefctHQ[j];
          dstrain[2] += Disp[k]*dshapefctHQ[k];
          dstrain[3] += Disp[i]*dshapefctHQ[j]
                       +Disp[j]*dshapefctHQ[i];
          dstrain[4] += Disp[i]*dshapefctHQ[k]
                       +Disp[k]*dshapefctHQ[i];
          dstrain[5] += Disp[j]*dshapefctHQ[k]
                       +Disp[k]*dshapefctHQ[j];
        }
        break;
    }
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: CalDensity()

   Aufgabe:
          Caculate density of porous medium
   Programming:
   05/2005     WW        Erste Version
**************************************************************************/
double CFiniteElementVec::CalDensity()
{
   double rho;
//OK_MFP
  //--------------------------------------------------------------------
  // MFP fluid properties
   double density_fluid = 0.0;
   double porosity = 0.0;  
   int no_phases = (int)mfp_vector.size();
   int phase = 0;
   
   rho = 0.0;
   if(F_Flag)
   {
      if((no_phases>0)&&(no_phases>phase))
         density_fluid = m_mfp->Density();
 
//OK_MMP
     //--------------------------------------------------------------------
     // MMP medium properties
     porosity = m_mmp->Porosity(Index,unit,1.0) ;
     // Assume solid density is constant. (*smat->data_Density)(0)
     if(smat->Density()>0.0)
           rho = (1. - porosity) * fabs(smat->Density())+porosity * density_fluid;
	 else rho = 0.0; 	  
     
   }
   else
   {  // If negative value is given in the .msp file, gravity by solid is skipped
      if(smat->Density()>0.0)
        rho = smat->Density();
   }      
   return rho;
} 
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: ComputeMatrix_RHS(const double fkt)
   Aufgabe:
           as the name
 
   Programming:
   07/2004   WW   
   08/2004   OK   MFP implementation                                                                      
**************************************************************************/
void CFiniteElementVec::ComputeMatrix_RHS(const double fkt, 
								   const Matrix *p_D)
{
   int i, j, k, l;
   double rho;
   rho = CalDensity();
   for (i = 0; i < nnodesHQ; i++)
   {
      setTransB_Matrix(i);   
      // Local assembly of A*u=int(B^t*sigma) for Newton-Raphson method  			 
      for (j = 0; j<ele_dim; j++)
      {
          for (k = 0; k<ns; k++) 
             (*RHS)(j*nnodesHQ+i) += 
//TEST             (*B_matrix_T)(j,k)*dstress[k]*fkt;
                 (*B_matrix_T)(j,k)*(dstress[k]-stress0[k])*fkt;
      }         
      if(PreLoad==11) continue;       
      // Local assembly of stiffness matrix, B^T C B       
      for (j = 0; j < nnodesHQ; j++)
      {
          setB_Matrix(j);
          // Compute stiffness matrix
          (*AuxMatrix) = 0.0;
          B_matrix_T->multi(*p_D, *B_matrix, *AuxMatrix);      

          // Local assembly of stiffness matrix
          for (k = 0; k < ele_dim ; k++)
          {
              for (l = 0; l < ele_dim; l++)
                  (*Stiffness)(i+k*nnodesHQ, j+l*nnodesHQ) += (*AuxMatrix)(k,l) * fkt; 
	      }
      }  // loop j 	    
  } // loop i 
  //---------------------------------------------------------
  // Assemble coupling matrix
  //---------------------------------------------------------
  // SM -> DM - Kopplungsgroeßen 
  // Gauss-Punkte, Quadratische Ansätze fuer Verschiebungen 
  // Lineare fuer den Druck  
  // LoadFactor: factor of incremental loading, prescibed in rf_pcs.cpp	

  if(F_Flag&&!PreLoad)
  {
    for (k=0;k<nnodesHQ;k++) {   
       for (l=0;l<nnodes;l++) {   
         for(j=0; j<ele_dim; j++)
            (*PressureC)(nnodesHQ*j+k,l) += S_Water* LoadFactor* dshapefctHQ[nnodesHQ*j+k] * shapefct[l] * fkt;
       }
    }
  }	  
  //---------------------------------------------------------
  // Assemble gravity force vector
  //---------------------------------------------------------
  if(rho>0.0&&GravityForce)
  {  
      // 2D, in y-direction
      // 3D, in z-direction
      i = (ele_dim-1)*nnodesHQ;
      for (k = 0; k < nnodesHQ; k++) 
          (*RHS)(i+k) += LoadFactor * rho * gravity_constant * shapefctHQ[k] * fkt;
  }
}
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: LocalAssembly
   Aufgabe:
           Compute the local finite element matrices     
   Formalparameter:
           E: 
			const int update  : indicator to update stress and strain only
 
   Programming:
   06/2004   WW   Generalize for different element types as a member of class
   05/2005   WW   ...
**************************************************************************/
void CFiniteElementVec::LocalAssembly(const int update)
{
    int i, j;
    double *a_n = NULL; 

    SetMemory();
    SetMaterial();

    (*RHS) = 0.0;
    (*Stiffness) = 0.0;
   	if(F_Flag)
    (*PressureC) = 0.0;

    for(i=0;i<nnodesHQ;i++){
#ifdef PARALLEL
    eqs_number[i] = nodes[i];
#else
    eqs_number[i] = MeshElement->nodes[i]->GetEquationIndex();
#endif
  }


    // For strain and stress extropolation all element types
    // Number of elements associated to nodes
    for(i=0; i<nnodes; i++)
      dbuff[i] = (double)MeshElement->nodes[i]->connected_elements.size();

    // Get displacement_n
    if(dynamic)
    {
       a_n = pcs->GetAuxArray();     
       for(i=0; i<dim; i++)
       {
          for(j=0; j<nnodesHQ; j++)
          {
             // Increment of acceleration, da
             (*dAcceleration)(i*nnodesHQ+j) = pcs->GetNodeValue(nodes[j],Idx_dm0[i]);
             // Increment of displacement
             // du = v_n*dt+0.5*a_n*dt*dt+0.5*beta2*da*dt*dt
             // a_n = a_{n+1}-da
             Disp[j+i*nnodesHQ] =  pcs->GetNodeValue(nodes[j],Idx_Vel[i])*dt\
                                  +0.5*dt*dt*(a_n[nodes[j]+NodeShift[i]]
                                  +beta2*(*dAcceleration)(i*nnodesHQ+j));
             
          }
       }      
    }
    else
    {
       for(i=0; i<dim; i++)
       {
          for(j=0; j<nnodesHQ; j++)
             Disp[j+i*nnodesHQ] = pcs->GetNodeValue(nodes[j],Idx_dm0[i]);
       }
    }

    // Get saturation of element nodes
   	if(Flow_Type>0)
   	{
       for(i=0; i<nnodes; i++)
	   {
           AuxNodal_S[i] = h_pcs->GetNodeValue(nodes[i], idx_S);
           AuxNodal_S0[i] = h_pcs->GetNodeValue(nodes[i], idx_S0);
	   }
    }
    // 

    if(enhanced_strain_dm&&ele_value_dm[MeshElement->GetIndex()]->Localized)
        LocalAssembly_EnhancedStrain(update);
    else 
        LocalAssembly_continuum(update);

    if(update==0)
    {
        if(dynamic)
            ComputeMass();
        GlobalAssembly();
        //Output matrices
        if(pcs->Write_Matrix)
        {
           (*pcs->matrix_file) << "### Element: " << Index << endl;
           (*pcs->matrix_file) << "---Stiffness matrix: " << endl;
           Stiffness->Write(*pcs->matrix_file);
           (*pcs->matrix_file) << "---RHS: " <<endl;
           RHS->Write(*pcs->matrix_file);
        }
    }

}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::  GlobalAssembly()
   Aufgabe:
           Assemble local matrics and vectors to the global system        
 
   Programming:
   02/2005   WW   
**************************************************************************/
bool CFiniteElementVec::GlobalAssembly()
{
    GlobalAssembly_RHS();
    if(PreLoad==11) return true;
    GlobalAssembly_Stiffness();
    return true;
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::  GlobalAssembly_Stiffness()
   Aufgabe:
           Assemble local matrics and vectors to the global system        
 
   Programming:
   02/2005   WW   
**************************************************************************/
void CFiniteElementVec::GlobalAssembly_Stiffness()
{
   int i,j, k, l;
   double f1,f2;
   f1=1.0;
   f2=-1.0;
   
   if(dynamic)
   {
      f1 = 0.5*beta2*dt*dt;
      f2 = -0.5*bbeta1*dt; 
      // Assemble stiffness matrix
      for (i = 0; i < nnodesHQ; i++)
      {
         for (j = 0; j < nnodesHQ; j++)
         {
             // Local assembly of stiffness matrix
             for (k = 0; k < ele_dim ; k++)
             {
                 MXInc(eqs_number[i]+NodeShift[k],eqs_number[j]+NodeShift[k], 
                      (*Mass)(i, j));
	         }
         }  // loop j 	    
      } // loop i    
   } 
   // Assemble stiffness matrix
   for (i = 0; i < nnodesHQ; i++)
   {
      for (j = 0; j < nnodesHQ; j++)
      {
          // Local assembly of stiffness matrix
          for (k = 0; k < ele_dim ; k++)
          {
              for (l = 0; l < ele_dim; l++)
                 MXInc(eqs_number[i]+NodeShift[k],eqs_number[j]+NodeShift[l], 
                      f1*(*Stiffness)(i+k*nnodesHQ, j+l*nnodesHQ));
	      }
      }  // loop j 	    
   } // loop i    
 
//TEST OUT
//Stiffness->Write();

   // Assemble coupling matrix
   if(Flow_Type>=0&&D_Flag == 41) // Monolithic scheme
   {   // Add pressure coupling matrix to the stifness matrix
       for (i=0;i<nnodesHQ;i++) {   
          for (j=0;j<nnodes;j++) {   
            for(k=0; k<ele_dim; k++)
                 MXInc(NodeShift[k]+eqs_number[i], NodeShift[dim]+eqs_number[j],\
                       f2*(*PressureC)(nnodesHQ*k+i,j));
          }
       }
   }

//TEST OUT
//PressureC->Write();
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ComputeMass()
   Aufgabe:
           Compute the mass matrix for dynamic analyses
   Programming:
   05/2005   WW   Elastische Elemente
**************************************************************************/
void CFiniteElementVec::ComputeMass()
{
  int i,j;  
  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt=0.0;

  (*Mass) = 0.0;
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
      fkt *= CalDensity();

      ComputeShapefct(2); // Quadratic interpolation function
      for(i=0; i<nnodesHQ; i++)
      {
         for(j=0; j<nnodesHQ; j++)
         {
            if(i>j) continue;
            (*Mass)(i,j) += fkt*shapefctHQ[i]*shapefctHQ[j];
         }
      }

  }
  
}



/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::  GlobalAssembly_RHS()
   Aufgabe:
           Assemble local matrics and vectors to the global system        
 
   Programming:
   02/2005   WW   
   05/2005   WW dyn  
**************************************************************************/
void CFiniteElementVec::GlobalAssembly_RHS()
{
   int i, j, k;
   double fact, val_n=0.0;
   double *a_n = NULL; 

   bool Residual;  
   Residual = false;
   fact = 1.0;  
   k=0;

   if(Flow_Type>=0)
   {
      if(D_Flag==41) // Monolithic scheme
      { 
         // If nonlinear deformation
         if(pcs_deformation>100)
           Residual = true;
      }    
      else // Partitioned scheme
        Residual = true;
   }
   if(dynamic)
   {
      fact = bbeta1*dt;  
      Residual = true;
      // Solution of the previous step 
      a_n = pcs->GetAuxArray();     

   } 

   // Assemble coupling matrix
   // If dynamic GetNodeValue(nodes[i],idx_P0) = 0;
   if(Residual) 
   {  
      switch(Flow_Type)
      {
          case 0:  // Liquid flow
             for (i=0;i<nnodes;i++)
			 {
                val_n = h_pcs->GetNodeValue(nodes[i],idx_P1); 
                if(val_n>0.0)
                   AuxNodal[i] = LoadFactor*( val_n -Max(pcs->GetNodeValue(nodes[i],idx_P0),0.0));   
				else
                   AuxNodal[i] = 0.0;
			 }
             break;
          case 1:  // Richards flow
             for (i=0;i<nnodes;i++)
			 {
                val_n = h_pcs->GetNodeValue(nodes[i],idx_P1);
                if(val_n>0.0)                  
                  AuxNodal[i] = LoadFactor*(val_n-Max(pcs->GetNodeValue(nodes[i],idx_P0),0.0));
				else
                   AuxNodal[i] = 0.0;
			 }
             break;
          case 2:  // Multi-phase-flow
             for (i=0;i<nnodes;i++)
               AuxNodal[i] = LoadFactor*((1.- h_pcs->GetNodeValue(nodes[i],idx_S))
                                 *h_pcs->GetNodeValue(nodes[i],idx_P1)
                                + h_pcs->GetNodeValue(nodes[i],idx_S)
                                *(h_pcs->GetNodeValue(nodes[i],idx_P2)-h_pcs->GetNodeValue(nodes[i],idx_P0)));
             break;
      }
 
      // If dymanic
      if(dynamic)
      {
         for (i=0;i<nnodes;i++)
         {
             AuxNodal[i] *= fact; 
             AuxNodal[i] += dt*a_n[nodes[i]+NodeShift[problem_dimension_dm]]
                           +pcs->GetNodeValue(nodes[i],idx_P);  
         }
      }
      
       // Coupling effect to RHS
       for (i=0;i<dim*nnodesHQ;i++)
           AuxNodal1[i] = 0.0;
       PressureC->multi(AuxNodal, AuxNodal1);
       for (i=0;i<dim*nnodesHQ;i++)
           (*RHS)(i) -= AuxNodal1[i];
   } // End if partioned

   // If dymanic
   if(dynamic)
   {
      for (i=0;i<dim;i++)
      {
          for (j=0;j<nnodesHQ;j++)
          {
              for (k=0;k<nnodesHQ;k++)
                (*RHS)(i*nnodesHQ+j) += (*Mass)(j,k)*(
                                    (*dAcceleration)(i*nnodesHQ+k)
                                    +a_n[nodes[k]+NodeShift[i]]); 
          
          }
      } 
   } 

//RHS->Write();

   for (i=0;i<dim;i++)
   {
       for (j=0;j<nnodesHQ;j++)
           pcs->eqs->b[eqs_number[j]+NodeShift[i]] -= (*RHS)(i*nnodesHQ+j); 
   }

}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec:: LocalAssembly_continuum()
   Aufgabe:
           Compute the local finite element matrices with the framework
        of continuum assumption
   Formalparameter:
           E: 
			const int update  : indicator to update stress and strain only
 
   Programming:
   02/2000   OK   Elastische Elemente
   09/2002   WW   Local assemby of stiffness matrix of elasto-plastic 
                  tangential model
                  Local assemby of residual 
   07/2003   WW   Quadratic triangle element
   06/2004   WW   Generalize for different element types as a member of class
   12/2005   WW   Creep
**************************************************************************/
void CFiniteElementVec::LocalAssembly_continuum(const int update)
{
  long i;
  
  Matrix *p_D = NULL; 
  eleV_DM = ele_value_dm[MeshElement->GetIndex()];

  // ---- Gauss integral
  int gp, gp_r=0, gp_s=0, gp_t;
  gp_t = 0;
  double fkt=0.0;

  double *DevStress ;
  const int PModel = smat->Plasticity_type;
  double dPhi = 0.0; // Sclar factor for the plastic strain  
  double J2=0.0;
  double dS = 0.0;

 
  double ThermalExpansion=0.0;
  double Tem=0.0;
  bool Strain_TCS = false;
  //
  ThermalExpansion=0.0;
  // Thermal effect 
  if(smat->Thermal_Expansion()>0.0) 
      ThermalExpansion =smat->Thermal_Expansion();
  
  //Get porosity model
  // ---- Material properties
  //For swelling pressure;
  double deporo = 0.0;
//OK_MMP
  //--------------------------------------------------------------------
  // MMP medium properties
  int PoroModel = m_mmp->porosity_model;
  if(PoroModel==4)
     //OK deporo =  PCSGetElementPorosityChangeRate(index)/(double)ele_dim;
    //MX  deporo = PCSGetELEValue(index,NULL,1.0,"n_sw_Rate")/(double)ele_dim;
      deporo = h_pcs->GetElementValue(Index,pcs->GetElementValueIndex("n_sw_rate"))/(double)ele_dim;
  if(T_Flag)
  { 
    for (i = 0; i < nnodes; i++) 
      Temp[i] =  t_pcs->GetNodeValue(nodes[i],idx_T1)-t_pcs->GetNodeValue(nodes[i],idx_T0);
  }


  if(PModel==1) smat->CalulateCoefficent_DP();
  if(PModel!=3)
  {
      smat->Calculate_Lame_Constant();
      smat->ElasticConsitutive(ele_dim, De);  
  }
  //
  if(PoroModel==4||T_Flag||smat->Creep_mode>0)
    Strain_TCS =true;
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
      //---------------------------------------------------------
      ComputeGradShapefct(2);
      ComputeShapefct(2); 
      ComputeStrain();
 	   
      if( F_Flag||T_Flag) 
          ComputeShapefct(1); // Linear order interpolation function
      //---------------------------------------------------------
      // Material properties (Integration of the stress)
      //---------------------------------------------------------
      // Initial the stress vector  
      if(PModel!=3)
	  {
         for (i = 0; i < ns; i++)
            dstress[i] = 0.0;
          De->multi(dstrain, dstress);
	  }
      //---------------------------------------------------------
      // Integrate the stress by return mapping:
      //---------------------------------------------------------
      switch(PModel)
      { 
	     case -1:   // Pure elasticity
           for (i = 0; i < ns; i++)
             dstress[i] += (*eleV_DM->Stress)(i, gp);
           break;
         case 1:  // Drucker-Prager model               
            DevStress
                =smat->StressIntegrationDP(gp, eleV_DM, dstress, &dPhi, update);	  		
            J2 = sqrt(TensorMutiplication2(DevStress, DevStress, ele_dim));
 	       
            // If yield, compute consistent tangent operator 
            if(dPhi*J2>0.0)
                smat->ConsistentTangentialDP(ConsistDep, DevStress, dPhi, ele_dim);                      
            break;
         case 2:  // Rotational hardening model                
            // Compute stesses and plastic multi-plier 
            dPhi = 0.0;
            if(smat->CalStress_and_TangentialMatrix_SYS(gp, eleV_DM, 
                 De, ConsistDep, dstress,  update)>0)
            dPhi = 1.0;
            break;
         case 3:  // Generalized Cam-Clay model 
            for (i = 0; i < ns; i++)
               dstress[i] = dstrain[i];
            smat->CalStress_and_TangentialMatrix_CC(gp, eleV_DM,
                           dstress,  ConsistDep, update);
            dPhi = 1.0;
            break;    
      }
      // --------------------------------------------------------------------
      // Stress increment by heat, swelling, or heat
      //
      if(Strain_TCS)
	  {
        if(PModel==3)
           smat->ElasticConsitutive(ele_dim, De);
        for (i = 0; i < ns; i++)
          strain_ne[i] = 0.0;
        if(PoroModel==4)// For swelling pressure
        {
            for (i = 0; i < 3; i++)
              strain_ne[i] -= deporo;
        }
        //
        if(T_Flag) // Contribution by thermal expansion 
        {
            Tem=0.0; 
            for(i = 0; i< nnodes; i++)
               Tem += shapefct[i]* Temp[i];
            for (i = 0; i < 3; i++)
                strain_ne[i] -= ThermalExpansion*Tem;
        } 
        if(smat->Creep_mode==1) // Strain increment by creep
	    {
           for (i = 0; i < ns; i++)
             stress_ne[i] = (*eleV_DM->Stress)(i, gp);
		   smat->AddStain_by_Creep(ns,stress_ne, strain_ne);           
	    }
        // Compute try stress, stress incremental: 
        De->multi(strain_ne, dstress);
        for (i = 0; i < ns; i++)
          dstrain[i] += strain_ne[i];
	  } 

      // Fluid coupling;
      S_Water=1.0;
      if(Flow_Type>0)
         S_Water=interpolate(AuxNodal_S,1);
      // Decovalex. Swelling pressure
      if(smat->SwellingPressureType==1)
      {
        dS = -interpolate(AuxNodal_S0,1);
        dS += S_Water;
        for (i = 0; i < 3; i++)
           dstress[i] -= 2.0*S_Water*dS*smat->Max_SwellingPressure;     
      }
      else if(smat->SwellingPressureType==2) // LBNL's model
      {
        dS = -interpolate(AuxNodal_S0,1);
        dS += S_Water;
        for (i = 0; i < 3; i++)
           dstress[i] -= dS*smat->Max_SwellingPressure;     
      }
      // Assemble matrices and RHS
      if(update<1)
  	  {
         //---------------------------------------------------------
         // Assemble matrices and RHS
         //---------------------------------------------------------
         if(dPhi<=0.0) p_D = De;
         else p_D = ConsistDep;
         for (i = 0; i<ns; i++)
            stress0[i] = (*eleV_DM->Stress0)(i,gp);
         ComputeMatrix_RHS(fkt, p_D);
      }  
	  else  // Update stress
	  {
        RecordGuassStrain(gp, gp_r, gp_s, gp_t);
        for(i=0; i<ns; i++)
           (*eleV_DM->Stress)(i, gp) = dstress[i];
	  }
  }
  // The mapping of Gauss point strain to element nodes
  if(update)
      ExtropolateGuassStrain();	

}
/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::GetLocalIndex()
           For quadralateral and hexahedra element on the assumption that
           selected Gauss points form a quadralateral or hexahedra
   Aufgabe:
           Accumulate stress at each nodes      
   Formalparameter:
 
   Programming:
   06/2004   WW  
**************************************************************************/
int CFiniteElementVec::GetLocalIndex(const int gp_r, const int gp_s, int gp_t)
{
   int LoIndex = -1;
   double r,s,t;

   //---------------------------------------------------------
   // Accumulate strains
   //---------------------------------------------------------
   switch(MeshElement->GetElementType())
   { 
     case 2:  // Quadralateral
         r = MXPGaussPkt(nGauss, gp_r);
         s = MXPGaussPkt(nGauss, gp_s);
         if(r>0.0&&s>0.0)  LoIndex = 0;
         else if(r<0.0&&s>0.0) LoIndex = 1;
         else if(r<0.0&&s<0.0) LoIndex = 2;
         else if(r>0.0&&s<0.0) LoIndex = 3;
         else if(fabs(r)<MKleinsteZahl&&s>0.0) LoIndex = 4;
         else if(r<0.0&&fabs(s)<MKleinsteZahl) LoIndex = 5;
         else if(fabs(r)<MKleinsteZahl&&s<0.0) LoIndex = 6;
         else if(r>0.0&&fabs(s)<MKleinsteZahl) LoIndex = 7;
         else if(fabs(r)<MKleinsteZahl&&fabs(s)<MKleinsteZahl) LoIndex = 8;
         break;
     case 3:  // Hexahedra
         r = MXPGaussPkt(nGauss, gp_r);
         s = MXPGaussPkt(nGauss, gp_s);
         t = MXPGaussPkt(nGauss, gp_t);
 
         if(t>0.0)
         {
            if(r>0.0&&s>0.0)  LoIndex = 0;
            else if(r<0.0&&s>0.0) LoIndex = 1;
            else if(r<0.0&&s<0.0) LoIndex = 2;
            else if(r>0.0&&s<0.0) LoIndex = 3;
            else if(fabs(r)<MKleinsteZahl&&s>0.0) LoIndex = 8;       
            else if(r<0.0&&fabs(s)<MKleinsteZahl) LoIndex = 9;
            else if(fabs(r)<MKleinsteZahl&&s<0.0) LoIndex = 10;
            else if(r>0.0&&fabs(s)<MKleinsteZahl) LoIndex = 11;
            else if(fabs(r)<MKleinsteZahl&&fabs(s)<MKleinsteZahl) return -1;
         }
         else if(fabs(t)<MKleinsteZahl)
         {
            if(fabs(r)<MKleinsteZahl||fabs(s)<MKleinsteZahl) return -1;
            if(r>0.0&&s>0.0)  LoIndex = 16;
            else if(r<0.0&&s>0.0) LoIndex = 17;
            else if(r<0.0&&s<0.0) LoIndex = 18;
            else if(r>0.0&&s<0.0) LoIndex = 19;
         }
         if(t<0.0)
         {
            if(r>0.0&&s>0.0)  LoIndex = 4;
            else if(r<0.0&&s>0.0) LoIndex = 5;
            else if(r<0.0&&s<0.0) LoIndex = 6;
            else if(r>0.0&&s<0.0) LoIndex = 7;
            else if(fabs(r)<MKleinsteZahl&&s>0.0) LoIndex = 12;       
            else if(r<0.0&&fabs(s)<MKleinsteZahl) LoIndex = 13;
            else if(fabs(r)<MKleinsteZahl&&s<0.0) LoIndex = 14;
            else if(r>0.0&&fabs(s)<MKleinsteZahl) LoIndex = 15;
            else if(fabs(r)<MKleinsteZahl&&fabs(s)<MKleinsteZahl) return -1;
         }
      break;
   }
   return LoIndex;
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::RecordGuassValues()
   Aufgabe:
           Accumulate stress at each nodes      
   Formalparameter:
           E: 
 
   Programming:
   06/2004   WW  
**************************************************************************/
bool CFiniteElementVec::RecordGuassStrain(const int gp, 
              const int gp_r, const int gp_s, int gp_t)
{
   int LoIndex = 0;

   //---------------------------------------------------------
   // Accumulate strains
   //---------------------------------------------------------
   switch(MeshElement->GetElementType())
   { 
     case 2:  // Quadralateral
         LoIndex = GetLocalIndex(gp_r, gp_s, gp_t);
         Sxx[LoIndex] = dstrain[0];
	     Syy[LoIndex] = dstrain[1];
         Sxy[LoIndex] = dstrain[3];
         Szz[LoIndex] = dstrain[2];
         break;
     case 4: // Triangle     
         Sxx[gp] = dstrain[0];
         Syy[gp] = dstrain[1];
         Szz[gp] = dstrain[2];
         Sxy[gp] = dstrain[3];
	     break;
     case 3:  // Hexahedra
         LoIndex = GetLocalIndex(gp_r, gp_s, gp_t);
         if(LoIndex<0) return false;
         Sxx[LoIndex] = dstrain[0];
	     Syy[LoIndex] = dstrain[1];
         Szz[LoIndex] = dstrain[2];
         Sxy[LoIndex] = dstrain[3];
         Sxz[LoIndex] = dstrain[4];
         Syz[LoIndex] = dstrain[5];
         break;
     case 5: // Tedrahedra     
         Sxx[gp] = dstrain[0];
         Syy[gp] = dstrain[1];
         Szz[gp] = dstrain[2];
         Sxy[gp] = dstrain[3];
         Sxz[gp] = dstrain[4];
         Syz[gp] = dstrain[5];
	     break;
     default:  break;
		 // 3D
   }
   return false;
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ExtropolateGuassStrain()
   Aufgabe:
           Extropolate the Gauss point strains to nodes    
   Formalparameter:
           E: 
 
   Programming:
   06/2004   WW  
**************************************************************************/
void CFiniteElementVec::ExtropolateGuassStrain()
{
  int i, j, gp=0;
  int l1,l2,l3,l4, counter;
  double ESxx, ESyy, ESzz, ESxy, ESxz, ESyz;
  double r=0.0, Xi_p = 0.0;
  double Area1, Area2, Tol=10e-9;

  l1=l2=l3=l4=0;

  if(MeshElement->GetElementType()==2||MeshElement->GetElementType()==3)
  {
     for (gp = 0; gp < nGauss; gp++)
     {
        r = MXPGaussPkt(nGauss, gp);
        if(fabs(r)>Xi_p) Xi_p = fabs(r);
     }
     r = 1.0/Xi_p;
     Xi_p = r;
  }

   //---------------------------------------------------------
   // Mapping Gauss point strains to nodes and update nodes 
   // strains:
   //---------------------------------------------------------
   for(i=0; i<nnodes; i++)
   { 
      ESxx = ESyy = ESzz = ESxy = ESxz = ESyz = 0.0;

      switch(MeshElement->GetElementType()) 
      {
         case 4: // Traingle
           // Compute values at verteces  
           switch(i)
	       {
		      case 0:
	             unit[0] = -0.1666666666667;
	             unit[1] = -0.1666666666667;
                 break;
		      case 1:
	             unit[0] = 1.6666666666667;
	             unit[1] = -0.1666666666667;
                 break;
		      case 2:
	             unit[0] = -0.1666666666667;
	             unit[1] = 1.6666666666667;
                 break;
		   }
           ComputeShapefct(1); // Linear interpolation function
           for(j=0; j<nnodes; j++)
           {
              ESxx += Sxx[j]*shapefct[j]; 
              ESyy += Syy[j]*shapefct[j]; 
              ESxy += Sxy[j]*shapefct[j]; 
              ESzz += Szz[j]*shapefct[j]; 
           }
	       break;
	     case 2: // Quadralateral element 
 
	       // Extropolation over nodes
           switch(i)
	       {
	          case 0:
                 unit[0] = Xi_p;
                 unit[1] = Xi_p;
		         break;
	          case 1:
                 unit[0] = -Xi_p;
                 unit[1] = Xi_p;
	             break;
	          case 2:
                 unit[0] = -Xi_p;
                 unit[1] = -Xi_p;
		         break;
	          case 3:
                 unit[0] = Xi_p;
                 unit[1] = -Xi_p;
                 break;
           }

           ComputeShapefct(2); // High order interpolation function

           // Strain
           for(j=0; j<nnodesHQ; j++)
           {
              ESxx += Sxx[j]*shapefctHQ[j]; 
              ESyy += Syy[j]*shapefctHQ[j]; 
              ESxy += Sxy[j]*shapefctHQ[j]; 
              ESzz += Szz[j]*shapefctHQ[j]; 
           }
           break;                              
        case 3: // Hexahedra 
           if(i<4)
           {  
              j = i;
              unit[2] = Xi_p;
           }
           else
           {
              j = i-4; 
              unit[2] = -Xi_p;
           }

           switch(j)
	       {
	          case 0:
                 unit[0] = Xi_p;
                 unit[1] = Xi_p;
		         break;
	          case 1:
                 unit[0] = -Xi_p;
                 unit[1] = Xi_p;
	             break;
	          case 2:
                 unit[0] = -Xi_p;
                 unit[1] = -Xi_p;
		         break;
	          case 3:
                 unit[0] = Xi_p;
                 unit[1] = -Xi_p;
                 break;
           }

           ComputeShapefct(2); // High order interpolation function
           // Strain
           for(j=0; j<nnodesHQ; j++)
           {
              ESxx += Sxx[j]*shapefctHQ[j]; 
              ESyy += Syy[j]*shapefctHQ[j]; 
              ESxy += Sxy[j]*shapefctHQ[j]; 
              ESzz += Szz[j]*shapefctHQ[j]; 
              ESxz += Sxz[j]*shapefctHQ[j]; 
              ESyz += Syz[j]*shapefctHQ[j]; 
           }
           break;                               
         case 5: // Tedrahedra 
           switch(i)
           {
              case 0:
                  l1 = 4;
                  l2 = 9;
                  l3 = 6;
                  l4 = 0;
                  break;
               case 1:
                  l1 = 4;
                  l2 = 5;
                  l3 = 7;
                  l4 = 1;
                  break;
               case 2:
                  l1 = 6;
                  l2 = 8;
                  l3 = 5;
                  l4 = 2;
                  break;
               case 3:
                  l1 = 7;
                  l2 = 8;
                  l3 = 9;
                  l4 = 3;
                  break;
            }

            x1buff[0] = X[l1];
            x2buff[0] = X[l2];
            x3buff[0] = X[l3];
            x4buff[0] = X[l4];
            //
			x1buff[1] = Y[l1];
            x2buff[1] = Y[l2];
            x3buff[1] = Y[l3];
            x4buff[1] = Y[l4];
			//
            x1buff[2] = Z[l1];
            x2buff[2] = Z[l2];
            x3buff[2] = Z[l3];
            x4buff[2] = Z[l4];

			// Volume of the corner tet to node i 
            Area1 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
            counter = 0;
            for (gp = 0; gp < nGauss; gp++)
            {
               SamplePointTet15(gp, unit);
               ComputeShapefct(2);
               // Real coorinates of this Guass point
               RealCoordinates(X0);
               Area2 = ComputeDetTex(x1buff, x2buff, x3buff, X0)
                      +ComputeDetTex(x1buff, x4buff, x2buff, X0)
                      +ComputeDetTex(x3buff, x4buff, x1buff, X0)
                      +ComputeDetTex(x2buff, x4buff, x3buff, X0);
               
               if(fabs(Area1-Area2)<Tol)
               {
                   // This point is within the corner
                   counter++;
                   ESxx += Sxx[gp]; 
                   ESyy += Syy[gp]; 
                   ESxy += Sxy[gp]; 
                   ESzz += Szz[gp]; 
                   ESxz += Sxz[gp]; 
                   ESyz += Syz[gp]; 
               }
            }
#ifdef gDEBUG
            if(counter==0)
            {
                cout<<" No gauss point close to vertex "<<i<<endl;
               abort();
            }
#endif
            ESxx /= (double)counter; 
            ESyy /= (double)counter; 
            ESxy /= (double)counter; 
            ESzz /= (double)counter; 
            ESxz /= (double)counter; 
            ESyz /= (double)counter; 
            
        break;    
      }

      // Average value of the contribution of ell neighbor elements 
      ESxx /= dbuff[i]; 
      ESyy /= dbuff[i]; 
      ESxy /= dbuff[i]; 
      ESzz /= dbuff[i];

      ESxx += pcs->GetNodeValue(nodes[i],Idx_Strain[0]); 
      ESyy += pcs->GetNodeValue(nodes[i],Idx_Strain[1]);  
      ESzz += pcs->GetNodeValue(nodes[i],Idx_Strain[2]);  
      ESxy += pcs->GetNodeValue(nodes[i],Idx_Strain[3]);  
 
      pcs->SetNodeValue (nodes[i], Idx_Strain[0], ESxx);
      pcs->SetNodeValue (nodes[i], Idx_Strain[1], ESyy);
      pcs->SetNodeValue (nodes[i], Idx_Strain[2], ESzz);
      pcs->SetNodeValue (nodes[i], Idx_Strain[3], ESxy);
    
      if(ele_dim==3)
      {
         ESxz /= dbuff[i]; 
         ESyz /= dbuff[i]; 
         ESxz += pcs->GetNodeValue(nodes[i],Idx_Strain[4]);
         ESyz += pcs->GetNodeValue(nodes[i],Idx_Strain[5]);

         pcs->SetNodeValue (nodes[i], Idx_Strain[4], ESxz);
         pcs->SetNodeValue (nodes[i], Idx_Strain[5], ESyz);
      }
   }
}



/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ExtropolateGuassStress()
   Aufgabe:
           Extropolate the Gauss point strains to nodes    
   Formalparameter:
           E: 
 
   Programming:
   06/2004   WW  
**************************************************************************/
void CFiniteElementVec::ExtropolateGuassStress()
{
  int i, j, gp, gp_r, gp_s, gp_t;
  int l1,l2,l3,l4, counter;
  double ESxx, ESyy, ESzz, ESxy, ESxz, ESyz, Pls;
  double r=0.0, Xi_p = 0.0;
  double Area1, Area2, Tol=10e-9;

  int ElementType = MeshElement->GetElementType();

  // For strain and stress extropolation all element types
  // Number of elements associated to nodes
  for(i=0; i<nnodes; i++)
	 dbuff[i] = (double)MeshElement->nodes[i]->connected_elements.size();
/*
// Decovalex
  if(smat&&smat->SwellingPressureType==1)
  {
     S = 0.0;
     SetMaterial();
     for(i=0; i<nnodes; i++)
        AuxNodal_S[i] = GetNodeValue(nodes[i], idx_S);
  }
*/

  l1=l2=l3=l4=0;
  gp_r=gp_s=gp_t=gp=0;
  eleV_DM = ele_value_dm[MeshElement->GetIndex()];
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
      Sxx[i] = (*eleV_DM->Stress)(0,gp);
      Syy[i] = (*eleV_DM->Stress)(1,gp);
      Szz[i] = (*eleV_DM->Stress)(2,gp);
      Sxy[i] = (*eleV_DM->Stress)(3,gp);
      pstr[i] = (*eleV_DM->pStrain)(gp);
      if(ele_dim==3)
      {
        Sxz[i] = (*eleV_DM->Stress)(4,gp);
        Syz[i] = (*eleV_DM->Stress)(5,gp);
      }
/*
// Test for decovalex
   if(smat&&smat->SwellingPressureType==1)
   {      
	  GetGaussData(gp, gp_r, gp_t, gp_s);
	  ComputeShapefct(1);
	  S = interpolate(AuxNodal_S);
      
	  Sxx[i] -= S*S*smat->Max_SwellingPressure;
      Syy[i] -= S*S*smat->Max_SwellingPressure;
      Szz[i] -= S*S*smat->Max_SwellingPressure;   
   }

/////////////////////////////////////
*/
  }

  if(ElementType==2||ElementType==3)
  {
     for (gp = 0; gp < nGauss; gp++)
     {
        r = MXPGaussPkt(nGauss, gp);
        if(fabs(r)>Xi_p) Xi_p = fabs(r);
     }
     r = 1.0/Xi_p;
     Xi_p = r;
   }

   //---------------------------------------------------------
   // Mapping Gauss point strains to nodes and update nodes 
   // strains:
   //---------------------------------------------------------
   for(i=0; i<nnodes; i++)
   { 
      ESxx = ESyy = ESzz = ESxy = ESxz = ESyz = Pls = 0.0;

      switch(ElementType) 
      {
         case 4: // Traingle
           // Compute values at verteces  
           // Compute values at verteces  
           switch(i)
	       {
		      case 0:
	             unit[0] = -0.1666666666667;
	             unit[1] = -0.1666666666667;
                 break;
		      case 1:
	             unit[0] = 1.6666666666667;
	             unit[1] = -0.1666666666667;
                 break;
		      case 2:
	             unit[0] = -0.1666666666667;
	             unit[1] = 1.6666666666667;
                 break;
		   }
           ComputeShapefct(1); // Linear interpolation function
           for(j=0; j<nnodes; j++)
           {
              ESxx += Sxx[j]*shapefct[j]; 
              ESyy += Syy[j]*shapefct[j]; 
              ESxy += Sxy[j]*shapefct[j]; 
              ESzz += Szz[j]*shapefct[j]; 
              Pls += shapefct[j]*pstr[j];
           }
	       break;
	     case 2: // Quadralateral element  
	       // Extropolation over nodes
           switch(i)
	       {
	          case 0:
                 unit[0] = Xi_p;
                 unit[1] = Xi_p;
		         break;
	          case 1:
                 unit[0] = -Xi_p;
                 unit[1] = Xi_p;
	             break;
	          case 2:
                 unit[0] = -Xi_p;
                 unit[1] = -Xi_p;
		         break;
	          case 3:
                 unit[0] = Xi_p;
                 unit[1] = -Xi_p;
                 break;
           }

           ComputeShapefct(2); // High order interpolation function

           // Strain
           for(j=0; j<nnodesHQ; j++)
           {
              ESxx += Sxx[j]*shapefctHQ[j]; 
              ESyy += Syy[j]*shapefctHQ[j]; 
              ESxy += Sxy[j]*shapefctHQ[j]; 
              ESzz += Szz[j]*shapefctHQ[j]; 
              Pls += pstr[j]*shapefctHQ[j]; 
           }
           break;                              
        case 3: // Hexahedra 
           if(i<4)
           {  
              j = i;
              unit[2] = Xi_p;
           }
           else
           {
              j = i-4; 
              unit[2] = -Xi_p;
           }

           switch(j)
	       {
	          case 0:
                 unit[0] = Xi_p;
                 unit[1] = Xi_p;
		         break;
	          case 1:
                 unit[0] = -Xi_p;
                 unit[1] = Xi_p;
	             break;
	          case 2:
                 unit[0] = -Xi_p;
                 unit[1] = -Xi_p;
		         break;
	          case 3:
                 unit[0] = Xi_p;
                 unit[1] = -Xi_p;
                 break;
           }

           ComputeShapefct(2); // High order interpolation function
           // Strain
           for(j=0; j<nnodesHQ; j++)
           {
              ESxx += Sxx[j]*shapefctHQ[j]; 
              ESyy += Syy[j]*shapefctHQ[j]; 
              ESxy += Sxy[j]*shapefctHQ[j]; 
              ESzz += Szz[j]*shapefctHQ[j]; 
              ESxz += Sxz[j]*shapefctHQ[j]; 
              ESyz += Syz[j]*shapefctHQ[j]; 
              Pls += pstr[j]*shapefctHQ[j]; 
           }
           break;                               
         case 5: // Tedrahedra 
           switch(i)
           {
              case 0:
                  l1 = 4;
                  l2 = 9;
                  l3 = 6;
                  l4 = 0;
                  break;
               case 1:
                  l1 = 4;
                  l2 = 5;
                  l3 = 7;
                  l4 = 1;
                  break;
               case 2:
                  l1 = 6;
                  l2 = 8;
                  l3 = 5;
                  l4 = 2;
                  break;
               case 3:
                  l1 = 7;
                  l2 = 8;
                  l3 = 9;
                  l4 = 3;
                  break;
            }
            x1buff[0] = X[l1];
            x2buff[0] = X[l2];
            x3buff[0] = X[l3];
            x4buff[0] = X[l4];
            //
			x1buff[1] = Y[l1];
            x2buff[1] = Y[l2];
            x3buff[1] = Y[l3];
            x4buff[1] = Y[l4];
			//
            x1buff[2] = Z[l1];
            x2buff[2] = Z[l2];
            x3buff[2] = Z[l3];
            x4buff[2] = Z[l4];

			// Volume of the corner tet to node i 
            Area1 = ComputeDetTex(x1buff, x2buff, x3buff, x4buff);
            counter = 0;
            for (gp = 0; gp < nGauss; gp++)
            {
               SamplePointTet15(gp, unit);
               ComputeShapefct(2);
               // Real coorinates of this Guass point
               RealCoordinates(X0);
               Area2 = ComputeDetTex(x1buff, x2buff, x3buff, X0)
                      +ComputeDetTex(x1buff, x4buff, x2buff, X0)
                      +ComputeDetTex(x3buff, x4buff, x1buff, X0)
                      +ComputeDetTex(x2buff, x4buff, x3buff, X0);
               
               if(fabs(Area1-Area2)<Tol)
               {
                   // This point is within the corner
                   counter++;
                   ESxx += Sxx[gp]; 
                   ESyy += Syy[gp]; 
                   ESxy += Sxy[gp]; 
                   ESzz += Szz[gp]; 
                   ESxz += Sxz[gp]; 
                   ESyz += Syz[gp]; 
                   Pls += pstr[gp]; 
               }
            }
#ifdef gDEBUG
            if(counter==0)
            {
                cout<<" No gauss point close to vertex "<<i<<endl;
                abort();
            }
#endif
            ESxx /= (double)counter; 
            ESyy /= (double)counter; 
            ESxy /= (double)counter; 
            ESzz /= (double)counter; 
            ESxz /= (double)counter; 
            ESyz /= (double)counter; 
            Pls /= (double)counter; 
            
        break;    
      }

      // Average value of the contribution of ell neighbor elements 
      ESxx /= dbuff[i]; 
      ESyy /= dbuff[i]; 
      ESxy /= dbuff[i]; 
      ESzz /= dbuff[i];
      Pls /= dbuff[i];

      ESxx += pcs->GetNodeValue(nodes[i],Idx_Stress[0]); 
      ESyy += pcs->GetNodeValue(nodes[i],Idx_Stress[1]);  
      ESzz += pcs->GetNodeValue(nodes[i],Idx_Stress[2]);  
      ESxy += pcs->GetNodeValue(nodes[i],Idx_Stress[3]);  
      Pls  += pcs->GetNodeValue(nodes[i],idx_pls);  
 
      pcs->SetNodeValue (nodes[i], Idx_Stress[0], ESxx);
      pcs->SetNodeValue (nodes[i], Idx_Stress[1], ESyy);
      pcs->SetNodeValue (nodes[i], Idx_Stress[2], ESzz);
      pcs->SetNodeValue (nodes[i], Idx_Stress[3], ESxy);
      pcs->SetNodeValue (nodes[i], idx_pls, fabs(Pls));
   
      if(ele_dim==3)
      {
         ESxz /= dbuff[i]; 
         ESyz /= dbuff[i]; 

         ESxz += pcs->GetNodeValue(nodes[i],Idx_Stress[4]);
         ESyz += pcs->GetNodeValue(nodes[i],Idx_Stress[5]);

         pcs->SetNodeValue (nodes[i], Idx_Stress[4], ESxz);
         pcs->SetNodeValue (nodes[i], Idx_Stress[5], ESyz);
      }
   }
}

//==========================================================================
// Enhanced strain element
/***************************************************************************
   GeoSys - Funktion: 
        CFiniteElementVec::CheckNodesInJumpedDomain(const double *tangJump)

   Aufgabe:
         Compute the regular enhanced strain matrix (Only 2D)
		    Ge: (See the related references)
 		(Prorior: element nodes is fetched )
	    
   Formalparameter:
         E: 
         const double *tangJump   : Tangential to the jump surface
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::CheckNodesInJumpedDomain()
{
    int i;
    double cdotpdt;
    //	static double dphi_e[3];

    // 2D
    // Get the center of the discontinuity

    X0[0] = 0.5*((*eleV_DM->NodesOnPath)(0,0)+(*eleV_DM->NodesOnPath)(0,1));
    X0[1] = 0.5*((*eleV_DM->NodesOnPath)(1,0)+(*eleV_DM->NodesOnPath)(1,1));
    X0[2] = 0.5*((*eleV_DM->NodesOnPath)(2,0)+(*eleV_DM->NodesOnPath)(2,1));

    // Determine nodes in the jumping part
    for(i=0; i<nnodesHQ; i++)
    {
        NodesInJumpedA[i] = false;
        cdotpdt = 0.0;
        cdotpdt += n_jump[0]*( X[i]-X0[0]);
        cdotpdt += n_jump[1]*( Y[i]-X0[1]);
        if(ele_dim==3) 
           cdotpdt += n_jump[2]*( Z[i]-X0[2]);
        if(cdotpdt>0.0) NodesInJumpedA[i] = true; // Nodes in $\Omega_+$
    }
}

/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ComputeRESM(double * normJump)

   Aufgabe:
         Compute the regular enhanced strain matrix (Only 2D)
		    Ge: (See the related references)
 		(Prorior: element nodes is fetched )
	    
   Formalparameter:
         E: 
         const double *tangJump   : Tangential to the jump surface
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::ComputeRESM(const double *tangJump)
{
	int i;
	static double dphi_e[3];

//TEST
//    computeJacobian(1);
//    ComputeGradShapefct(1);
//
	for(i=0; i<ele_dim; i++)
	{
		dphi_e[i] = 0.0;
        for(int j=0; j<nnodesHQ; j++)
//        for(int j=0; j<nnodes; j++)
		{
            if(NodesInJumpedA[j])
                dphi_e[i] += dshapefctHQ[i*nnodesHQ+j];
//               dphi_e[i] += dshapefct[i*nnodes+j];
		}
	}
    // !!! Only for 2D up to now
	tangJump = tangJump;
	// Column 1
	(*Ge)(0,0) = n_jump[0]*dphi_e[0];
	(*Ge)(1,0) = n_jump[1]*dphi_e[1];
	(*Ge)(2,0) = 0.0;
	(*Ge)(3,0) = n_jump[0]*dphi_e[1]+n_jump[1]*dphi_e[0];

	// Column 2
	(*Ge)(0,1) = -n_jump[1]*dphi_e[0];
	(*Ge)(1,1) =  n_jump[0]*dphi_e[1];
	(*Ge)(2,1) =  0.0;
	(*Ge)(3,1) = -n_jump[1]*dphi_e[1]+n_jump[0]*dphi_e[0];
}


/***************************************************************************
   GeoSys - Funktion: 
           CFiniteElementVec::ComputeSESM(double * normJump)

   Aufgabe:
         Compute the singular enhanced strain matrix (Only 2D)
		    Ge: (See the related references)
 		(Prorior: element nodes is fetched )
	    
   Formalparameter:
         E: 
         const double *tangJump   : Tangential to the jump surface
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CFiniteElementVec::ComputeSESM(const double *tangJump)
{
    // !!! Only for 2D up to now
	tangJump = tangJump;
	// Column 1
	(*Pe)(0,0) = n_jump[0]*n_jump[0];
	(*Pe)(0,1) = n_jump[1]*n_jump[1];
    (*Pe)(0,2) = 0.0;
	(*Pe)(0,3) = 2.0*n_jump[0]*n_jump[1];

	// Column 2
	(*Pe)(1,0) = -n_jump[0]*n_jump[1];
	(*Pe)(1,1) = n_jump[1]*n_jump[0];
    (*Pe)(1,2) = 0.0;
	(*Pe)(1,3) = n_jump[0]*n_jump[0]- n_jump[1]*n_jump[1];
}


/***************************************************************************
   GeoSys - Funktion: 
      CFiniteElementVec::ComputePrincipleStresses(double *dEStress)
      (2D only)
   Aufgabe:
	    
   Formalparameter:
         E: 
   const double *Stresses: Stresses 
   
   Return: Angle of maxium principle stress component to x direction
           sig2<sig1

   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
double CFiniteElementVec::ComputePrincipleStresses(const double *Stresses)
{
   double prin_ang, var;
   // Angle of the principle plane
   if(fabs(Stresses[0]-Stresses[1])< MKleinsteZahl) return 0.0;
   prin_ang = atan(2.0*Stresses[3]/(Stresses[0]-Stresses[1]));
   // Principle stress 1
   pr_stress[0] = 0.5*(Stresses[0]+Stresses[1])
         +0.5*(Stresses[0]-Stresses[1])*cos(prin_ang)
              +Stresses[3]*sin(prin_ang);
   // Principle stress 2
   pr_stress[1] = 0.5*(Stresses[0]+Stresses[1])
         -0.5*(Stresses[0]-Stresses[1])*cos(prin_ang)
              -Stresses[3]*sin(prin_ang);
   pr_stress[2] = Stresses[2];

   prin_ang *= 0.5;

   // Jump direction
   if(pr_stress[1]>=pr_stress[0]) 
   {
      prin_ang += 0.5*pai;
      var = pr_stress[0];
      pr_stress[0] = pr_stress[1];
      pr_stress[1] = var;
   }          
   //if(pr_stress[0]<0.0) prin_ang += pai;
   return  prin_ang;      
}
/***************************************************************************
   GeoSys - Funktion: 
      CFiniteElementVec::ComputeJumpDirectionAngle(const double *Stresses, double *Mat)
      (2D Drucker-Prager plasticity only. 
	    cf. K. Runesson, D. Peric and S. Sture, 
		Discontinuous bifucations of elasto-plastic solutions at plane stress 
		and plane strain, Int. J. Plasticity 7(1991) 2087-2105)
   Aufgabe:
	    
   Formalparameter:
         E: 
   const double *Stresses: Stresses 
   const double *Mat     : Material parameters
                           0, dilatancy
                           1, frictional
                           2, Poission ratio
   Return: Angle of maxium principle stress component to x direction
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
double CFiniteElementVec::ComputeJumpDirectionAngle(const double *Mat)
{
   double NormS, c1, c2;
   DeviatoricStress(pr_stress);
   NormS = sqrt(TensorMutiplication2(pr_stress, pr_stress,ele_dim));
   c1 = pr_stress[1]+Mat[2]*pr_stress[2]+0.5*(1.0+Mat[2])*
                                    (Mat[0]+Mat[1])*NormS;        
   c2 = pr_stress[0]+Mat[2]*pr_stress[2]+0.5*(1.0+Mat[2])*
                                    (Mat[0]+Mat[1])*NormS;        
    
   if(c1>=0.0||c2<=0.0) NormS = -1.0;
   else
      NormS = atan(sqrt(-c2/c1));
  
   return NormS; // The angle return through NormS
}



/**************************************************************************
  GeoSys - Funktion: 
    void CFiniteElementVec::LocalAssembly_CheckLocalization
 
   Aufgabe:
     Trace discontinuity surface and determine the normal direction to it 
	  element-wisely. (Material related)
    (Drucker-Prager model and 2D only)	

  
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E :
                                                                         
  Programmaenderungen:
     06/2004   WW  Erste Version
**************************************************************************/
bool CFiniteElementVec::LocalAssembly_CheckLocalization(CElem* MElement)
{
  int i,j,k; 
  int MatGroup; 

  double ep, p, normXi, n1, n2;

  // For enhanced strain element
  double h_loc, detA, h_tol=1.0e-5;
  double pr_stress_ang, loc_ang;
  static double OriJ[2], Nj[2], Aac[4], Mat[3];
  bool LOCed = false;
  //
  MeshElement = MElement;
  eleV_DM = ele_value_dm[MeshElement->GetIndex()];

  p = 0.0;
  // Get the total effective plastic strain 
  ep = 0.0;
  for(i=0; i<nGaussPoints; i++)
     ep += (*eleV_DM->pStrain)(i);
  ep /= (double)nGaussPoints;

  if(ep>0.0) // in yield status 
  {

     if(!eleV_DM->Localized)
     {
        smat->Calculate_Lame_Constant();
        smat->CalulateCoefficent_DP();
        
		MatGroup = MeshElement->GetPatchIndex();
        smat = msp_vector[MatGroup];

        Mat[0] = smat->Al;
        Mat[1] = smat->Xi;
        Mat[2] = smat->Poisson_Ratio();

        // Compute the average stress of this element
        for(i=0; i<ns; i++) dstress[i] = 0.0;
        for(i=0; i<nGaussPoints; i++)
        {
            for(j=0; j<ns; j++)
                dstress[j] += (*eleV_DM->Stress)(j,i); 
        }
        for(i=0; i<ns; i++) dstress[i] /= (double)nGaussPoints;

        // Get the converged stresses of the previous step 
		//--- Compute the determinate of the acoustic tensor
        pr_stress_ang = ComputePrincipleStresses(dstress); 
        loc_ang = ComputeJumpDirectionAngle(Mat);
        
        normXi = sqrt(TensorMutiplication2(pr_stress, pr_stress, ele_dim));
        		 
        // Compute the localization condition
        h_loc = pr_stress[2]/normXi+0.5*(smat->Al+smat->Xi)
            -0.5*sqrt(2.0/(1.0-smat->Poisson_Ratio()))*(smat->Al-smat->Xi);
        detA = 1.0e8;
        
        // Two possible jump orientation, i.e. pr_stress_ang+/-loc_ang
        OriJ[0] = pr_stress_ang -loc_ang+0.5*pai;
        OriJ[1] = pr_stress_ang +loc_ang+0.5*pai;

		// Compute the acoustic matrix
        DeviatoricStress(dstress);
        normXi = sqrt(TensorMutiplication2(dstress, dstress, ele_dim));
        if(loc_ang>0.0)
        {
            for(i=0; i<ns; i++)
                dstress[i] /= normXi;             
            for(k=0; k<2; k++)
            {
               n_jump[0] = cos(OriJ[k]);
               n_jump[1] = sin(OriJ[k]); 
               // 
               Nj[0] = dstress[0]*n_jump[0]+dstress[3]*n_jump[1];
               Nj[1] = dstress[3]*n_jump[0]+dstress[1]*n_jump[1];

               for(i=0; i<4; i++)
                  Aac[i] = 0.0;
               for(i=0; i<2; i++)
               {
                  Aac[i*2+i] = smat->G;         
                  for(j=0; j<2; j++)
                     Aac[i*2+j] += (smat->K+smat->G/3.0)*n_jump[i]*n_jump[j]
                                  -(4.5*smat->Al*smat->Xi*smat->K*smat->K*
                                       n_jump[i]*n_jump[j]+3.0*smat->G*smat->K
		                       *(smat->Al*Nj[i]*n_jump[j]+smat->Xi*Nj[j]*n_jump[i])
                                    +2.0*smat->G*smat->G*Nj[i]*Nj[j])
                                   /(4.5*smat->Al*smat->Xi*smat->K+smat->G);
               }
               detA = Aac[0]*Aac[3]-Aac[1]*Aac[2];
               if(detA<=0.0)
               {
                  LOCed = true;    
                  break;
               }
            }
            for(i=0; i<ns; i++)
                dstress[i] *= normXi;
        }
        //
        if(fabs(h_loc)<h_tol)
           LOCed = true;
 
        if(LOCed)
        {
            eleV_DM->orientation = new double[ele_dim];
            for(i=0; i<2; i++)
               eleV_DM->orientation[i] = OriJ[i]; 
            eleV_DM->Localized = true;
 
            // Choose one orientation. Empirical formular. 2D
            if(!Localizing) 
            {
               for(i=0; i<3; i++) dstress[i]+= p/3.0;
               n1 = (dstress[0]*cos(0.5*pai+OriJ[0])+dstress[1]*sin(0.5*pai+OriJ[0]))/normXi;
               n2 = (dstress[0]*cos(0.5*pai+OriJ[1])+dstress[1]*sin(0.5*pai+OriJ[1]))/normXi;
               if(n2>0.0)
               {   
                   // Always use orientation[0]
                   eleV_DM->orientation[0] = OriJ[1];
                   eleV_DM->orientation[1] = OriJ[0];
               }
            }
        }
      }
   }
   return LOCed;
}



/**************************************************************************
  GeoSys - Funktion: 
    void CFiniteElementVec::IntersectionPoint(const int O_edge, const double k,
                                       const double *NodeA, double nodeB  )
    2D only   
   Aufgabe:
        Determine the second intersection point of a line and the element.
        2D only (Geometry)
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E :
          const int O_edge       :   Edge will the departure point on   
          const double *NodeA    :   original node
          double nodeB           :   Intersection point 
                                                                         
  Programmaenderungen:
     06/2004   WW  Erste Version
**************************************************************************/
int CFiniteElementVec::IntersectionPoint(const int O_edge, const double *NodeA, double *NodeB  )
{
  int i,j, k, numf, nfnode; 

  static double k1, k2, n1, n2, xA[3],xB[3];
  static int Face_node[8]; // Only 2D
 
  double Tol=1.0e-12;
  double area0, area1;
  area0 = MeshElement->GetVolume();

  k=-1;

  eleV_DM = ele_value_dm[Index];


  numf = MeshElement->GetFacesNumber();
  for(i=0; i<numf; i++)
  {
     k=-1;
     if(i!=O_edge)
     {
         nfnode = MeshElement->GetElementFaceNodes(i, Face_node); 
 
         xA[0] = X[Face_node[0]];
         xA[1] = Y[Face_node[0]];
         xA[2] = Z[Face_node[0]];
         xB[0] = X[Face_node[1]];
         xB[1] = Y[Face_node[1]];
         xB[2] = Z[Face_node[1]];
  
         n1 = cos(eleV_DM->orientation[0]);
         n2 = sin(eleV_DM->orientation[0]);

         if(fabs((xB[0]-xA[0])*n1+(xB[1]-xA[1])*n2)<Tol) // parallel
            continue;

         if(fabs(n2)<Tol)
         { 
           NodeB[0] = NodeA[0];
           NodeB[1] = (NodeA[0]-xA[0])*(xB[1]-xA[1])/(xB[0]-xA[0])
                    +xA[1];  
         }
         else if(fabs(xB[0]-xA[0])<Tol)
         {
            NodeB[0] = xA[0];
            NodeB[1] = -n1*(xA[0]-NodeA[0])/n2+NodeA[1];
         }
         else
         {
            k1 = (xB[1]-xA[1])/(xB[0]-xA[0]);
            k2 = -n1/n2;
            NodeB[0] = (NodeA[1]-xA[1]+k1*xA[0]-k2*NodeA[0])/(k1-k2);
            NodeB[1] = k1*(NodeB[0]-xA[0])+xA[1];
         }
         k = i;
     }

     // Check if this point is on an edge of this element.
     if(k>=0) // Has intersection
     {
        area1 = 0.0;
        for(j=0; j<numf; j++)
        {
            if(j==k) continue;
            nfnode = MeshElement->GetElementFaceNodes(j, Face_node); 
            xA[0] = X[Face_node[0]];
            xA[1] = Y[Face_node[0]];
            xA[2] = Z[Face_node[0]];
            xB[0] = X[Face_node[1]];
            xB[1] = Y[Face_node[1]];
            xB[2] = Z[Face_node[1]];
            area1 += ComputeDetTri(xA, xB, NodeB);
        }
        if(fabs(area0-area1)<Tol) 
           break;
     }
  }
  return k; // Local index of other intersection point
}


/**************************************************************************
  GeoSys - Funktion: void CFiniteElementVec::LocalAssembly_EnhancedStrain
 
   Aufgabe:
     Local assembly within the strong discontinuity assumption
    (Drucker-Prager model and 2D only)	

  
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E :
	 const int update: 1 update Gauss values
	                   0 do not update 

  Ergebnis:
   - double* - Deviatoric effetive stresses, s11,s22,s12,s33
                                                                         
  Programmaenderungen:
     06/2004   WW  Erste Version
**************************************************************************/
void CFiniteElementVec::LocalAssembly_EnhancedStrain(const int update)
{
  int i,j, k,l, ii, jj, gp, gp_r, gp_s, gp_t; 
  double fkt=0.0 , area, Jac_e = 0.0, f_j;
 
  // For enhanced strain element
  double f_tol=1.0e-4; //Triangle 1.0e-8;
  double loc_dilatancy = 0.0, zeta_t0, zeta_t1;
  double sign, sj0,sj=0.0;
  static double tt0[2], tt[2], zeta[2], D_prj[2];

  bool isLoop = true; // Used only to avoid warnings with .net
  
  double ThermalExpansion = 0.0, Tem = 0.0;

  BDG->LimitSize(2, 2*nnodesHQ);
  PDB->LimitSize(2*nnodesHQ,2);

  gp_r = gp_s = gp_t = 0;

  eleV_DM = ele_value_dm[MeshElement->GetIndex()];

  ThermalExpansion=0.0;
  if(T_Flag)
  {
    // Thermal effect 
    if(smat->Thermal_Expansion()>0.0) 
        ThermalExpansion =smat->Thermal_Expansion();
    for(i=0; i<nnodes; i++)
       Temp[i] =  t_pcs->GetNodeValue(nodes[i],idx_T1)-t_pcs->GetNodeValue(nodes[i],idx_T0);
  }

  // Elastic modulus
  smat->Calculate_Lame_Constant(); 
  smat->ElasticConsitutive(ele_dim, De);  
     
  // Plasticity
  smat->CalulateCoefficent_DP();

  
  loc_dilatancy = smat->Al*sqrt(2.0/(1.0-3.0*smat->Al*smat->Al));
  double Hd = 0.5*smat->Hard_Loc/(1.0-3.0*smat->Al*smat->Al);

  
/*
  // Reference
  loc_dilatancy = 1.5*sqrt(2.0)*smat->Al*sqrt(1.0-6.0*smat->Al*smat->Al);
  double Hd = smat->Hard_Loc/(3.0-18.0*smat->Al*smat->Al); 
*/

  zeta_t0 = eleV_DM->disp_j;
  zeta_t1 = zeta_t0;
     
  area = MeshElement->volume;


  n_jump[0] = cos(eleV_DM->orientation[0]);
  n_jump[1] = sin(eleV_DM->orientation[0]); 

  // Compute traction on the jump plane
  // Compute Pe, singular part of enhanced strain-jump matrix
  ComputeSESM();
  // PeDe=P^t *D_e
  (*PeDe) = 0.0; 
  Pe->multi(*De, *PeDe);
  //
 
  // If this is the beginning of localization
  if(fabs(eleV_DM->tract_j)<MKleinsteZahl)
  {
     
     // average of stresses within an element
     for(j=0; j<ns; j++)
     {
        dstress[j] = 0.0;
        for(i=0; i<nGaussPoints; i++)
           dstress[j] += (*eleV_DM->Stress)(j,i); 
     }
     for(i=0; i<ns; i++) dstress[i] /= (double)nGaussPoints;

     for(i=0; i<ele_dim; i++)
     {
        tt0[i] = 0.0;
        for(j=0; j<ns; j++)
           tt0[i] += (*Pe)(i,j)*dstress[j];     
     }
     eleV_DM->tract_j = loc_dilatancy*tt0[0]+fabs(tt0[1]);
	 /*
     //
     for(gp=0; gp<nGaussPoints; gp++)
     {
        //--------------------------------------------------------------
        //-----------  Integrate of traction on the jump plane ---------
        //--------------------------------------------------------------
        for(i=0; i<ns; i++) dstress[i] = (*eleV_DM->Stress)(i,gp);
        fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
        for(i=0; i<ele_dim; i++)
        {
           tt0[i] = 0.0;
           for(j=0; j<ns; j++)
              tt0[i] += (*Pe)(i,j)*dstress[j];     
        }
        eleV_DM->tract_j = fkt*(loc_dilatancy*tt0[0]+fabs(tt0[1]));         
     }
     eleV_DM->tract_j /= area;
	 */
  }
  //
  sj0 = eleV_DM->tract_j;
  //
  CheckNodesInJumpedDomain();
  // On discontinuity by enhanced strain 
  // AuxMatrix temporarily used to store PDG
  (*AuxMatrix) = 0.0;
  // Integration of P^t*Stress^{elastic try}
  for(i=0; i<ele_dim; i++) tt0[i] = 0.0;
  //TEST
  for(i=0; i<ns; i++) dstress[i] = 0.0; //Test average appoach
  for(gp=0; gp<nGaussPoints; gp++)
  {
     //--------------------------------------------------------------
     //-----------  Integrate of traction on the jump plane ---------
     //--------------------------------------------------------------
     fkt = GetGaussData(gp, gp_r, gp_s, gp_t);
     //---------------------------------------------------------
     // Compute geometry
     //---------------------------------------------------------
     ComputeGradShapefct(2);
     ComputeStrain();
     // Compute Ge, regular part of enhanced strain-jump matrix
     ComputeRESM();     
     if(T_Flag) // Contribution by thermal expansion 
     {
        ComputeShapefct(1); // Linear interpolation function
        Tem=0.0; 
        for(i = 0; i< nnodes; i++)
           Tem += shapefct[i]* Temp[i];
        for (i = 0; i < 3; i++)
           dstrain[i] -= ThermalExpansion*Tem;
     } 
     /*
     for(i=0; i<ns; i++) dstress[i] = (*eleV_DM->Stress)(i,gp);
     De->multi(dstrain, dstress);
     // Try stress
     Pe->multi(dstress, tt0, fkt);
	 */
     // Pe*De*Ge
     PeDe->multi(*Ge, *AuxMatrix, fkt);

     //TEST -----------Average approach -------------------------
     for(i=0; i<ns; i++) dstress[i] += (*eleV_DM->Stress)(i,gp);
     De->multi(dstrain, dstress);
     //----------------------------------------------------------- 
  }
  
  //TEST average approach
  for(i=0; i<ns; i++) dstress[i] /= (double)nGaussPoints;
  Pe->multi(dstress, tt0, 1.0);
  for(i=0; i<ele_dim; i++) tt0[i] *= area;    
  //-------------------------------------------------------------

  //  Local Newton iteration for discontinuity number 
  while(isLoop)
  {

     zeta[1] = zeta_t1-zeta_t0; 
     zeta[0] = loc_dilatancy*fabs(zeta[1]);
		   
     // Sign(zeta_t)
     if(fabs(zeta[1])< MKleinsteZahl) sign = 1.0;
     else  sign =  zeta[1]/fabs( zeta[1]); 
          
     // 
     D_prj[0] = sign*loc_dilatancy;
     D_prj[1] = 1.0;

     sj = sj0 + Hd*zeta[1];
 
     //--------------------------------------------------------------
     //----------------   Local Jacobian   --------------------------
     //--------------------------------------------------------------
     for(i=0; i<ele_dim; i++) tt[i] = tt0[i];    
     AuxMatrix->multi(zeta, tt, -1.0);

     Jac_e =  0.0;
     for (i = 0; i < ele_dim; i++)
     { 
        for(j = 0; j< ele_dim; j++)
            Jac_e += D_prj[i]*(*AuxMatrix)(i,j)*D_prj[j];
     } 
     
     Jac_e /= area;
     for(i=0; i<ele_dim; i++) tt[i] /= area;
             
     f_j = D_prj[0]*tt[0]+tt[1]-sj;

     Jac_e += Hd;
     if(fabs(f_j)<f_tol) break;

     zeta_t1 +=  f_j/Jac_e;            
  }//Loop of the local Newton for enhanced parameter


  // Compute local RHS
  (*BDG) = 0.0;
  (*PDB) = 0.0;
  for(gp=0; gp<nGaussPoints; gp++)
  {
     //--------------------------------------------------------------
     //-----------  Integrate of traction on the jump plane ---------
     //--------------------------------------------------------------
     for(i=0; i<ns; i++) dstress[i] = (*eleV_DM->Stress)(i,gp);
     fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

     //---------------------------------------------------------
     // Compute geometry
     //---------------------------------------------------------
     ComputeGradShapefct(2);
     ComputeStrain();
     // Compute Ge
     ComputeRESM();

     if(T_Flag) // Contribution by thermal expansion 
     {
        ComputeShapefct(1); // Linear interpolation function
        Tem=0.0; 
        for(i = 0; i< nnodes; i++)
           Tem += shapefct[i]* Temp[i];
        for (i = 0; i < 3; i++)
           dstrain[i] -= ThermalExpansion*Tem;
     } 
  
     // Ehhanced strain:
     Ge->multi(zeta, dstrain, -1.0);

     // Compute try stress. 1. stress incremental:
     De->multi(dstrain, dstress);
   
     // Update stresses if convergence is reached
     if(update)
     {
        RecordGuassStrain(gp, gp_r, gp_s, gp_t); // Two Dimensional
        for(i=0; i<ns; i++) 
        {
            (*eleV_DM->Stress)(i,gp) =dstress[i];
             dstrain[i] = 0.0;
        }
        Ge->multi(zeta, dstrain, 1.0);
        DeviatoricStress(dstrain);
        (*eleV_DM->pStrain)(gp) += sqrt(2.0*TensorMutiplication2(dstrain, dstrain, 2)/3.0);
     }
     else
     {
        // Compute stiffness matrix
        ComputeMatrix_RHS(fkt, De);
        // Stiffness contributed by enhanced strain to the stiffness matrix
        for (i = 0; i < nnodesHQ; i++)
        {
           setTransB_Matrix(i);
           // B^T*D*G
           (*AuxMatrix) = 0.0;
           B_matrix_T->multi(*De, *Ge, *AuxMatrix);

           for (k = 0; k < ele_dim ; k++)
           {
              for (l = 0; l < ele_dim; l++)
                (*BDG)(k, ele_dim*i+l) += fkt*(*AuxMatrix)(k,l); 
           }           
           //
           // P*D*B          
           setB_Matrix(i);
           (*AuxMatrix) = 0.0;
           PeDe->multi(*B_matrix, *AuxMatrix);
           for (k = 0; k < ele_dim ; k++)
           {
              for (l = 0; l < ele_dim; l++)
                  (*PDB)(ele_dim*i+k, l) += fkt*(*AuxMatrix)(k,l)/area; 
           }           
        } 
     }
  } // End of RHS assembly                   
  
  // Those contributed by enhanced strain to the stiffness matrix
  // D*D^T
  for(i=0; i<ele_dim; i++) 
     for(j=0; j<ele_dim; j++) 
	    (*DtD)(i,j) = D_prj[i]*D_prj[j]; 

  // 
  for (i = 0; i < nnodesHQ; i++)
  {		   
     for (j = 0; j < nnodesHQ; j++)
     {
        // Local assembly of stiffness matrix
        for (k = 0; k < ele_dim ; k++)
        {
            for (l = 0; l < ele_dim; l++)
            {                   
                f_j = 0.0;
                for (ii = 0; ii < ele_dim ; ii++)
                {
                   for (jj = 0; jj < ele_dim; jj++)
                      f_j += 
                           (*BDG)(k, ele_dim*i+ii)*(*DtD)(ii,jj)*(*PDB)(ele_dim*j+jj, l); 
                }   
                (*Stiffness)(i+k*nnodesHQ, l*nnodesHQ+j) -= f_j/Jac_e; 
           }
        }
     }
  }   

  if(update)
  {
     // Update strains.
     // The mapping of Gauss point strain to element nodes
     ExtropolateGuassStrain();		 
     // Update enhanced parameter
     eleV_DM->disp_j = zeta_t1;
     eleV_DM->tract_j = sj;
            
  }
  
}

/*----------------------------------------------------------------
Class ElementValue_DM

Allocate memory for element value
     Matrix *Mat: 
                          | Index  |  Paramete |
                          ---------------------- 
						  |    0   |  alpha    |
                          |    1   |  beta     |
						  |    2   |  delta    |
                          |    3   |  epsilon  |
						  |    4   |  kappa    |
                          |    5   |  gamma    |
						  |    6   |  m        |
                          ----------------------
-----------------------------------------------------------------*/
ElementValue_DM::ElementValue_DM(CElem* ele, bool HM_Staggered):NodesOnPath(NULL), 
                                 orientation(NULL)
{
   int Plastic = 1;
   const int LengthMat=7; // Number of material parameter of SYS model.
   int LengthBS=4;  // Number of stress/strain components
   int NGPoints=0, NGP = 0;
   CSolidProperties *sdp = NULL;
   int ele_dim, ele_type;
   //
   Stress = NULL;
   pStrain = NULL;
   prep0 = NULL;
   e_i = NULL;
   xi = NULL;
   MatP = NULL;
   NodesOnPath = NULL;
   orientation = NULL;
   ele_type = ele->GetElementType();
   ele_dim = ele->GetDimension();
   sdp = msp_vector[ele->GetPatchIndex()];
   Plastic = sdp->Plastictity();

   if(ele_dim == 2) LengthBS = 4; 
   else if(ele_dim == 3) LengthBS = 6;
        
   NGP = GetNumericsGaussPoints(ele_type);
   if(ele_type==4)
      NGPoints=3;
   else if(ele_type==5)
      NGPoints=15;
   else NGPoints = (int)pow((double)NGP, (double)ele_dim);

   Stress0 = new Matrix(LengthBS, NGPoints);
   Stress_i = new Matrix(LengthBS, NGPoints);
   Stress = Stress_i;
   if(HM_Staggered)
      Stress_j = new Matrix(LengthBS, NGPoints);
   else
      Stress_j = NULL; // for HM coupling iteration   
   pStrain = new Matrix(NGPoints);
             
   *Stress = 0.0;
   *pStrain = 0.0;

   if(Plastic==2) // Rotational hardening model  
   {
      xi = new Matrix(LengthBS-1, NGPoints);
      MatP = new Matrix(LengthMat, NGPoints);
      *xi = 0.0;
      *MatP = 0.0;
   }
   if(Plastic==3)  // Cam-Clay
   {
       prep0 = new Matrix(NGPoints);
       e_i = new Matrix(NGPoints);
       *prep0 = 0.0;
       *e_i = 0.0;
   }
   disp_j=0.0;
   tract_j=0.0;
   Localized = false;
}
// 01/2006 WW
void ElementValue_DM::Write_BIN(fstream& os)
{
   Stress0->Write_BIN(os); 
   Stress_i->Write_BIN(os); 
   pStrain->Write_BIN(os);    
   if(xi) xi->Write_BIN(os);    		
   if(MatP) MatP->Write_BIN(os);    
   if(prep0) prep0->Write_BIN(os);    
   if(e_i) e_i->Write_BIN(os);    
   if(NodesOnPath) NodesOnPath->Write_BIN(os);    
   if(orientation) os.write((char*)(orientation), sizeof(*orientation));    
   os.write((char*)(&disp_j), sizeof(disp_j));    
   os.write((char*)(&tract_j), sizeof(tract_j));    
   os.write((char*)(&Localized), sizeof(Localized));    
}
// 01/2006 WW
void ElementValue_DM::Read_BIN(fstream& is)
{
   Stress0->Read_BIN(is); 
   Stress_i->Read_BIN(is); 
   pStrain->Read_BIN(is);    
   if(xi) xi->Read_BIN(is);    		
   if(MatP) MatP->Read_BIN(is);    
   if(prep0) prep0->Read_BIN(is);    
   if(e_i) e_i->Read_BIN(is);    
   if(NodesOnPath) NodesOnPath->Read_BIN(is);    
   if(orientation) 
     is.read((char*)(orientation), sizeof(*orientation));    
   is.read((char*)(&disp_j), sizeof(disp_j));    
   is.read((char*)(&tract_j), sizeof(tract_j));    
   is.read((char*)(&Localized), sizeof(Localized));    
}

void ElementValue_DM::ResetStress(bool cpl_loop)
{
   if(cpl_loop) // For coupling loop
   {
	   (*Stress_j) = (*Stress_i);
       Stress = Stress_j;
   }
   else // Time loop
   {
	   (*Stress_i) = (*Stress_j);
       Stress = Stress_i;
   }
}

ElementValue_DM::~ElementValue_DM()
{
    delete Stress0;
    if(Stress_i) delete Stress_i;
    if(Stress_j) delete Stress_j;
    delete pStrain;
    // Preconsolidation pressure
    if(prep0) delete prep0;               
    if(e_i) delete e_i;       // Void ratio        
    // Variables of single yield surface model   
    if(xi) delete xi; // Rotational hardening variables     
    if(MatP) delete MatP;    // Material parameters

    if(NodesOnPath) delete NodesOnPath;
    if(orientation) delete orientation;

    NodesOnPath = NULL;
    orientation = NULL;
    Stress0 = NULL;
    Stress = NULL;
    Stress_i = NULL; // for HM coupling iteration   
    Stress_j = NULL; // for HM coupling iteration   
    pStrain = NULL;
    prep0 = NULL;
    e_i = NULL;
    xi = NULL;
    MatP = NULL;
}

} // end namespace FiniteElement

/**************************************************************************/
/* ROCKFLOW - Modul: cel_asm.c
                                                                          */
/* Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen fuer das Modul aSM.
                                                                          */
/* Programmaenderungen:
   02/1997   RK   Uebertragen aus dem aTM
   09/1999   AH   GetNumericalTimeCollocation fuer stat. Fall
   05/2000   RK   Anpassungen an Permeabilitaetsmodell
   07/2000   OK   Dreieck-Elemente
   11/2000   OK   Optimierungen (Speicher/Rechenzeit)
   07/2000   OK   Tetraeder-Elemente
   11/2001   AH   Warnung entfernt
   03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt
   05/2003   OK   SMCreateELEMatricesPointer, SMDestroyELEMatricesPointer
   05/2003   OK   data access functions for PCS element matrices
   10/2003   MB   Numerische und analytische Lösung für Dreiecksprismen  
08/2004 OK MFP implementation                                                                      
08/2004 OK MMP implementation                      
11/2004 OK NUM                                                
                                                                          */
/**************************************************************************/
#include "stdafx.h"
#include "makros.h"
#include "rf_pcs.h" //OK_MOD"
#include "pcs_dm.h" 
#include "int_asm.h"
#include "nodes.h"
#include "elements.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
#include "rf_mfp_new.h"
#include "rf_tim_new.h"
#include "femlib.h"
#include "mathlib.h"
extern double gravity_constant;

#include "tools.h"
#define noTESTCEL_ASM
#define NUMERIC_ASM
double CalcEslope(long, CRFProcess*);
void Get_Matrix_Quad(double* edlluse, double* edttuse);
void CalcCKWR(int nn, double* haa, double* z, double power, double* test, int* iups);
void CalcNLTERMS(int nn, double* H, double* HaaOld, double power, double* swval, double* swold, double* krwval);
// MSHLib
#include "msh_elem.h"
using Mesh_Group::CFEMesh;
using Mesh_Group::CElem;
/* noNUMERIC_ASM --> analytische Lösung für EMatrizen (für Dreiecksprismen) */

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK PCS2 Implementation
**************************************************************************/
void SMCalcElementMatrices(CRFProcess *m_pcs)
{
  long i;
  Element* element = NULL;
  for (i=0;i<ElListSize();i++){
    element = ElGetElement(i);
    if(element&&element->aktiv)
      SMCalcElementMatrix(i,m_pcs);
  }
}
/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
11/2004 OK Implementation
**************************************************************************/
void SMCalcElementMatrix(long index,CRFProcess *m_pcs)
{
    switch (ElGetElementType(index)) {
        case 1:
        CalcEle1D_ASM(index,NULL,NULL,NULL,m_pcs);
        break;
    case 2:
        CalcEle2D_ASM(index,NULL,NULL,NULL,m_pcs);
        break;
    case 3:
        CalcEle3D_ASM(index,NULL,NULL,NULL,m_pcs);
        break;
    case 4:
        CalcEle2DTriangle_ASM(index,NULL,NULL,NULL,m_pcs);
        break;
    case 5:
        CalcEle3DTetraeder_ASM(index,m_pcs);
        break;
    case 6:
        #ifdef NUMERIC_ASM /* Numerische Lösung */
        CalcEle3DPrism_ASM_num(index,NULL,NULL,NULL,m_pcs);
        #else              /* Analytische Lösung */
        CalcEle3DPrism_ASM_ana(index,NULL,NULL,NULL,m_pcs);
        #endif
        break;
    }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
02/2005 MB
**************************************************************************/
void OFCalcElementMatrices(CRFProcess *m_pcs)
{
  long i;
  Element* element = NULL;
  
  if(m_pcs->m_msh){
    for(i=0;i<(long)m_pcs->m_msh->ele_vector.size();i++){
        OFCalcElementMatrix(i,m_pcs);
    }
  }
  else{
    for (i=0;i<ElListSize();i++){
      element = ElGetElement(i);
      if(element&&element->aktiv)
        OFCalcElementMatrix(i,m_pcs);
    }
  }
}


/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
02/2005 MB Implementation
**************************************************************************/
void OFCalcElementMatrix(long index,CRFProcess *m_pcs)
{

if(m_pcs->m_msh){
  CElem* m_ele = NULL;
  m_ele = m_pcs->m_msh->ele_vector[index];
  switch(m_ele->GetElementType()){
    case 2:
      CalcEle2DQuad_OF(index,NULL,NULL,NULL,m_pcs);
    break;
    case 4:
      CalcEle2DTriangle_OF(index,NULL,NULL,NULL,m_pcs);
    break;
  }
}

else{
  switch (ElGetElementType(index)) {
    case 2:
      CalcEle2DQuad_OF(index,NULL,NULL,NULL,m_pcs);
      break;
    case 4:
      CalcEle2DTriangle_OF(index,NULL,NULL,NULL,m_pcs);
      break;
  }
}
}

/**************************************************************************
FEMLib-Method: 
Task: 
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 1D - Elements ohne 1/dt.
Programing:
02/1997 RK Uebertragen aus dem aTM und angepasst an das aSM
03/1997 RK Druckformulierung
03/1998 CT Dichtestroemungen
11/2000 OK Direkte Rckgabe der Matrizen fuer Speicheroptimierung
11/2002 MB MATCalcFluidDensity
06/2003 OK Bugfix: memory allocation for element matrices
06/2004 WW Rearrange memory allocation for element matrices
08/2004 OK MFP Implementation
08/2004 OK MMP Implementation
**************************************************************************/
void CalcEle1D_ASM(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
  static double *invjac, detjac;
  static double *k;
  static double kf, kmy, tkft, tkmyt, vorfk,storativity;
  static double g, rho, my, theta; /* , rho_add */ /* UJ rf3217*/
  static double elevation[2];
  static long *element_nodes;
  int phase = 0;
  double gp[3]; 

  //======================================================================
  // Memory WW
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    if(!capacitance_matrix)
      capacitance_matrix = (double *) Malloc(4 * sizeof(double));
    if(!conductance_matrix)
      conductance_matrix = (double *) Malloc(4 * sizeof(double));
    if(!gravity_vector)
      gravity_vector = (double *) Malloc(2 * sizeof(double));
  }
  //--------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  invjac = GetElementJacobiMatrix(index,&detjac); // detjac = AL/2, invjac = 2/(AL)
  gp[0]=0.0;gp[1]=0.0;gp[2]=0.0;
  //----------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();
if(gravity_constant<MKleinsteZahl){ // HEAD version
  my = 1.0;
  rho = 1.0;
}
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  k = m_mat_mp->PermeabilityTensor(index);
  storativity = m_mat_mp->StorageFunction(index,gp,theta);
  kf = k[0] * g * rho / my;
  kmy = k[0] / my;
  // t ist das Produkt aus der inversen Jacobi-und der Koordinaten-Transformations-Matrix
  tkft = kf * MSkalarprodukt(invjac, invjac, 3);
  tkmyt = kmy * MSkalarprodukt(invjac, invjac, 3);
  //======================================================================
  // 2. Elementmatrizen berechnen
  //----------------------------------------------------------------------
  // Capacitance matrix
/* WW
    if(memory_opt==0) { // schnelle, speicherintensive Variante 
      capacitance_matrix = ASMGetElementCapacitanceMatrix(index); //SB:todo Speicher
      if(!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(4 * sizeof(double));
    }
*/
  /* vorfk = S * A * L / 6 */
  vorfk = detjac * Mdrittel * storativity;
  capacitance_matrix[0] = 2.0 * vorfk;
  capacitance_matrix[1] = 1.0 * vorfk;
  capacitance_matrix[2] = 1.0 * vorfk;
  capacitance_matrix[3] = 2.0 * vorfk;
  if(memory_opt==0) // schnelle, speicherintensive Variante
    SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
  //----------------------------------------------------------------------
  // Conductivity matrix
/*WW
    if(memory_opt==0) { // schnelle, speicherintensive Variante 
      conductance_matrix = ASMGetElementConductanceMatrix(index);
      if(!conductance_matrix)
        conductance_matrix = (double *) Malloc(4 * sizeof(double));
    }
*/
  /* vorfk = k / my  / L  */
  vorfk = (detjac * 0.5) * tkmyt;
  conductance_matrix[0] = +1.0 * vorfk;
  conductance_matrix[1] = -1.0 * vorfk;
  conductance_matrix[2] = -1.0 * vorfk;
  conductance_matrix[3] = +1.0 * vorfk;
  if(memory_opt==0) // schnelle, speicherintensive Variante
    SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
  //----------------------------------------------------------------------
  // gravity vector matrix
/* Comment by WW
    if(memory_opt==0) { // schnelle, speicherintensive Variante 
      gravity_vector = ASMGetElementGravityVector(index);
      if(!gravity_vector)
        gravity_vector = (double *) Malloc(2 * sizeof(double));
    }
*/
  element_nodes = ElGetElementNodes(index);
  elevation[0] = ((GetNode(element_nodes[0]))->z);
  elevation[1] = ((GetNode(element_nodes[1]))->z);
  element_nodes = NULL;
  /* vorfk = (k * rho * g) / (L * my)  */
  vorfk = (detjac * 0.5) * tkft;
  gravity_vector[0] = vorfk * (elevation[0] - elevation[1]);
  gravity_vector[1] = vorfk * (elevation[1] - elevation[0]);
  if(memory_opt==0) // schnelle, speicherintensive Variante
    SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
  //----------------------------------------------------------------------
#ifdef TESTCEL_ASM
  MZeigMat(capacitance_matrix, 2, 2, "1D CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix, 2, 2, "1D CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector, 2, " GRAVITY-VECTOR");
#endif
}

/**************************************************************************
FEMLib-Method: 
Task: 
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 2D - Elementes ohne 1/dt.
Programing:
02/1997 RK Uebertragen aus dem aTM und angepasst an das aSM
04/1997 RK Druckformulierung
03/1998 CT Dichtestroemungen
04/2000 OK Erweiterung fr Konsolidierungsmodell
11/2000 OK Direkte Rckgabe der Matrizen fuer Speicheroptimierung
05/2001 MK DM/ASM-Kopplungsmatrizen korrigiert, 4x9-Matrix
01/2002 MK Achsensymmetrie im 2D
11/2002 MB MATCalcFluidDensity
06/2003 OK Bugfix: memory allocation for element matrices
04/2004 WW Improve the efficiency
08/2004 OK MFP Implementation
08/2004 OK MMP Implementation
10/2004 WW New coupling matrix computation in the context of OOP
02/2005 MB head version
**************************************************************************/
void CalcEle2D_ASM(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
    static double invjac[4],invjac_t[4],detjac;
    static double phi[4],g_phi_t[8];
    //static double g_ome_t[8];

    static double *coupling_matrix_u_x, *coupling_matrix_u_y;
    static double gravity_matrix[16];
    static long *element_nodes;
    static double elevation[4];
    static int anzgp;
    static long i,j; //,k,l,kl;
    static double r, s, fkt;
    static double g, my, rho; /* , rho_add */ /* UJ rf3217*/
    static double tkmyt[4], kmy[4];
    static double mat2x2[4]; //WW,mat4x4[16],mat4x2[8];
    static double theta;
    static double area,storativity,k_xx,k_yy;
    //    static double *perm;
    int phase = 0;
	int dof = 4;
	int ii,jj,kk,ll;
    double gp[3]; 
    double var; 
  //--------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  anzgp = m_pcs->m_num->ele_gauss_points;
  //--------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  
  element_nodes = ElGetElementNodes(index);

    /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
    if(memory_opt==0) { /* schnelle, speicherintensive Variante */
//*OKToDo      capacitance_matrix = ASMGetElementCapacitanceMatrix(index);
//*OKToDo      if(!capacitance_matrix)
        conductance_matrix = (double *) Malloc(16 * sizeof(double));
        capacitance_matrix = (double *) Malloc(16 * sizeof(double));
//*OKToDo      conductance_matrix = ASMGetElementConductanceMatrix(index);
//*OKToDo      if(!conductance_matrix)
//*OKToDo      gravity_vector = ASMGetElementGravityVector(index);
//*OKToDo      if(!gravity_vector)
        gravity_vector = (double *) Malloc(4 * sizeof(double));
        if(GetRFProcessProcessing("SD")) {
/* DM -> SM - Kopplungsgroeï¿½n */
//*OKToDo          coupling_matrix_u_x = ASMGetElementStrainCouplingMatrixX(index);
//*OKToDo          if(!coupling_matrix_u_x)
            coupling_matrix_u_x = (double *) Malloc(4*9*sizeof(double)); 
//*OKToDo          coupling_matrix_u_y = ASMGetElementStrainCouplingMatrixY(index);
//*OKToDo          if(!coupling_matrix_u_y)
            coupling_matrix_u_y = (double *) Malloc(4*9*sizeof(double)); 
       }
    }

    /* Initialisieren */
    MNulleMat(conductance_matrix, 4, 4);
    MNulleMat(capacitance_matrix, 4, 4);
    MNulleMat(gravity_matrix, 4, 4);
    MNulleVec(gravity_vector, 4);
    if(GetRFProcessProcessing("SD")) {
/* DM -> SM - Kopplungsgroeï¿½n */
        MNulleMat(coupling_matrix_u_x, 4, 9);
        MNulleMat(coupling_matrix_u_y, 4, 9);

    }

    /* Materialparameter */
    area = m_mmp->geo_area;
    g = gravity_constant;

  // Consolidation
  bool sm_cpl_deformation = false;
  string pcs_problem_type = PCSProblemType();
  if(pcs_problem_type.find("DEFORMATION")!=string::npos)
    sm_cpl_deformation = true;

  //--------------------------------------------------------------------
  // MMP medium properties
  area = m_mmp->geo_area;
  double* permeability;
  permeability = m_mmp->PermeabilityTensor(index);
  k_xx = k_yy = permeability[0];
  storativity = m_mmp->StorageFunction(index,gp,theta);
  //----------------------------------------------------------------------
  // Fluid properties
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    my = 1.0;
    rho = 1.0;
  }
    /*------------------------------------------------------------------------*/
    /* Schleife ueber GaussPunkte */
    for (i = 0; i < anzgp; i++) {
        for (j = 0; j < anzgp; j++) {
            r = MXPGaussPkt(anzgp, i);
            s = MXPGaussPkt(anzgp, j);

            /* J^-1 */
            Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
            /* Faktor */
            fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;

            /* conductance tensor */
            MNulleMat(kmy, 2, 2);
            kmy[0] = (k_xx) / my;
            kmy[3] = (k_yy) / my;


            /*----------------------------------------------------------------*/
            /* capacitance matrix */
            /* phi */
            MPhi2D(phi,r,s);
			// WW
            for(ii=0; ii<dof; ii++)
            {
                for(jj=0; jj<dof; jj++)
                    capacitance_matrix[ii*dof+jj] 
					     += phi[ii]*phi[jj]*fkt;
            }


            /*----------------------------------------------------------------*/
            /* conductance matrix und gravity matrix*/
            /* tkmyt = J^-1 * (K/my) * (J^-1)T */
            MMultMatMat(invjac,2,2,kmy,2,2,mat2x2,2,2);
            MTranspoMat(invjac,2,2,invjac_t);
            MMultMatMat(mat2x2,2,2,invjac_t,2,2,tkmyt,2,2);
            /* grad(phi)^T */
            MGradPhi2D(g_phi_t,r,s);         /* 2 Zeilen 4 Spalten */
	        // WW
            for(ii=0; ii<dof; ii++)
		    {
               for(jj=0; jj<dof; jj++)
		       {
                  var = 0.0;
			      for(kk=0; kk<2; kk++)
			      {
     			      for(ll=0; ll<2; ll++)
                         var += g_phi_t[kk*dof+ii]*tkmyt[2*kk+ll]*g_phi_t[ll*dof+jj];
			      }
                  gravity_matrix[ii*dof+jj] += var * fkt * rho * g;
                  conductance_matrix[ii*dof+jj] += var * fkt;
			    }
			 }

        }
    } /* Ende der Schleife ueber GaussPunkte */
    /*------------------------------------------------------------------------*/

    /* Volumenelemente */
	/*
    if (problem_2d_type_dm==2) {   // axially symmetric 
      area = 0;  
      for (i = 0; i < 4; i++) {
        area += 0.25*((GetNode(element_nodes[i]))->x) * 2.0 * PI;
      }   // axially symmetric 
    }
	*/
    for (i = 0; i < 16; i++) {
        capacitance_matrix[i] *= (area * storativity);
        conductance_matrix[i] *= (area);
        gravity_matrix[i] *= (area);
    }
    if(GetRFProcessProcessing("SD")) {
        for (i = 0; i < (4*9); i++) {
            coupling_matrix_u_x[i] *= (area);    
            coupling_matrix_u_y[i] *= (area);               
        }
    }

    /* Schwerkraftterm berechnen (gravity vector) */
    for (i = 0; i < 4; i++)
        elevation[i] = ((GetNode(element_nodes[i]))->z);
      /* gravity matrix * elevation */
    MMultMatVec(gravity_matrix,4,4,elevation,4,gravity_vector,4);


    /* Elementmatrizen speichern */
    if(memory_opt==0) { /* schnelle, speicherintensive Variante */
       SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
       SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
       SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);

    }
#ifdef TESTCEL_ASM
    MZeigMat(capacitance_matrix,4,4,"CalcEle2D_ASM: CAPACITANCE-MATRIX");
    MZeigMat(conductance_matrix,4,4,"CalcEle2D_ASM: CONDUCTANCE-MATRIX");
    MZeigVec(gravity_vector,4,"CalcEle2D_ASM: GRAVITY-VECTOR");
    if(GetRFProcessProcessing("SD")) {
/* DM -> SM - Kopplungsgroeï¿½n */
        MZeigMat(coupling_matrix_u_x,4,9,"CalcEle2D_ASM: STRAIN COUPLING MATRIX X");
        MZeigMat(coupling_matrix_u_y,4,9,"CalcEle2D_ASM: STRAIN COUPLING MATRIX Y");

    }
#endif

    element_nodes = NULL;
}


/**************************************************************************
FEMLib-Method: 
Task:    Influence coefficient matrices
Programing:
06/2005 MB Implementation
**************************************************************************/
void Get_Matrix_Quad(double* edlluse, double* edttuse)
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
Task:    Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) 
   für Overland Flow
Programing:
02/2005 MB Implementation
**************************************************************************/
void CalcEle2DQuad_OF(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
index = index;
capacitance_matrix = NULL;
conductance_matrix = NULL;
gravity_vector = NULL;
m_pcs = NULL;
//long *element_nodes;
//long i,j;
//double Haa[4];
//double HaaOld[4];
//double haa[4];
//double z[4];
//int nidx1, nidx0;
//double fric;
//double eslope;
//double dx;
//double dy;
//double dzx;
//double dzy;
//double axx;
//double ayy;
//double delt;
//double power;
//double ast=0.0;
//double krwval[4];
//double amat[4][4];
//double gammaij;
//double maxZ;
//double swval[4];
//double swold[4];
//double rhs[4];
//double edlluse[16];
//double edttuse[16];
//double ckwr[16];
//int iups[16];
//int nn;
//double GradH[2];
//double dhds;
//CMediumProperties *m_mmp = NULL;
//FiniteElement::CElement* m_ele = NULL;
//
//   /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
//if(memory_opt==0) { /* schnelle, speicherintensive Variante */
//  conductance_matrix = (double *) Malloc(16 * sizeof(double));
//  capacitance_matrix = (double *) Malloc(16 * sizeof(double));
//  gravity_vector = (double *) Malloc(4 * sizeof(double));
//}
//
//nn = 4;
//
///* Initialisieren */
//MNulleMat(conductance_matrix, 4, 4);
//MNulleMat(capacitance_matrix, 4, 4);
//MNulleMat(ckwr, 4, 4);
//MNulleMat(edlluse, 4, 4);
//MNulleMat(edttuse, 4, 4);
//MNulleVec(rhs,4);
//for (i = 0; i < 4; i++){
//  for (j = 0; j < 4; j++){
//    amat[i][j]= 0.0;
//  }
//}
//
////Get Influence Coefficient Matrices for analytical integration
//Get_Matrix_Quad(edlluse, edttuse);
//  
///* Numerische Parameter */
////------------------------------------------------------------------------
//// Element tolpology
//
//if(m_pcs->m_msh){
//  m_ele = m_pcs->m_msh->ele_vector[index];
//  element_nodes = m_ele->nodes;
//  nn = m_ele->nnodes;
//}
//else {
//  element_nodes = ElGetElementNodes(index);
//}
////------------------------------------------------------------------------
//// Element MMP group
////if(m_pcs->m_msh){ // use later in rf_ele_fem
////    fric = m_pcs->fem->MediaProp->permeability_tensor[0]; // Manning-coefficient: n
////  fric = 0.001;
////}
////else{
//if(m_pcs->m_msh){
//  m_mmp = mmp_vector[m_ele->GetPatchIndex()];
//  //fric = m_mmp->permeability_tensor[0];
//  fric = m_mmp->friction_coefficient;
//}
//else {
//  /* MMP medium properties */
//  long group = ElGetElementGroupNumber(index);
//  m_mmp = mmp_vector[group];
//  double* permeability;
//  //permeability = m_mmp->PermeabilityTensor(index);
//  //fric = permeability[0]; /* friction-coefficient: Manning or Chezy */ 
//  fric = m_mmp->friction_coefficient;
//}
//
//if(m_pcs->m_msh){
//  nidx1 = GetNodeValueIndex("HEAD")+1;
//  nidx0 = GetNodeValueIndex("HEAD")+0;
//  for(i=0;i<nn;i++){
//    haa[i] = GetNodeValue(element_nodes[i],nidx1);
//    z[i] = m_pcs->m_msh->nod_vector[element_nodes[i]]->z;
//    Haa[i] = haa[i] - z[i]; 
//    if (Haa[i] < 0.0) {Haa[i] = 0.0;}
//    HaaOld[i] = GetNodeValue(element_nodes[i],nidx0) - z[i];
//  }
//  dx = m_pcs->m_msh->nod_vector[element_nodes[1]]->x - m_pcs->m_msh->nod_vector[element_nodes[0]]->x; //ell
//  dy = m_pcs->m_msh->nod_vector[element_nodes[3]]->y - m_pcs->m_msh->nod_vector[element_nodes[0]]->y; //ett
//  dzx = z[1] - z[0];
//  dzy = z[3] - z[0]; 
//}
//else{
//  nidx1 = PCSGetNODValueIndex("HEAD",1);
//  nidx0 = PCSGetNODValueIndex("HEAD",0);
//    
//  /* Heads */
//  haa[0] = GetNodeVal(element_nodes[0],nidx1);
//  haa[1] = GetNodeVal(element_nodes[1],nidx1);
//  haa[2] = GetNodeVal(element_nodes[2],nidx1);
//  haa[3] = GetNodeVal(element_nodes[3],nidx1);
//
//  z[0] = GetNodeZ(element_nodes[0]);
//  z[1] = GetNodeZ(element_nodes[1]);
//  z[2] = GetNodeZ(element_nodes[2]);
//  z[3] = GetNodeZ(element_nodes[3]);
//
//  /* Wassertiefen */
//  Haa[0] = haa[0] - z[0];  if (Haa[0] < 0.0) {Haa[0] = 0.0;}
//  Haa[1] = haa[1] - z[1];  if (Haa[1] < 0.0) {Haa[1] = 0.0;}
//  Haa[2] = haa[2] - z[2];  if (Haa[2] < 0.0) {Haa[2] = 0.0;}
//  Haa[3] = haa[3] - z[3];  if (Haa[3] < 0.0) {Haa[3] = 0.0;}
//  HaaOld[0] = GetNodeVal(element_nodes[0],nidx0) - z[0];  if (HaaOld[0] < 0.0) {HaaOld[0] = 0.0;}
//  HaaOld[1] = GetNodeVal(element_nodes[1],nidx0) - z[1];  if (HaaOld[1] < 0.0) {HaaOld[1] = 0.0;}
//  HaaOld[2] = GetNodeVal(element_nodes[2],nidx0) - z[2];  if (HaaOld[2] < 0.0) {HaaOld[2] = 0.0;}
//  HaaOld[3] = GetNodeVal(element_nodes[3],nidx0) - z[3];  if (HaaOld[3] < 0.0) {HaaOld[3] = 0.0;}
//
//  //Element Dimensions
//  //Achtung nur für Rechtecke !
//  //später Kapseln
//  dx = GetNodeX(element_nodes[1])-GetNodeX(element_nodes[0]); //ell
//  dy = GetNodeY(element_nodes[3])-GetNodeY(element_nodes[0]); //ett
//  dzx = GetNodeZ(element_nodes[1])-GetNodeZ(element_nodes[0]);
//  dzy = GetNodeZ(element_nodes[3])-GetNodeZ(element_nodes[0]);
//}
//
//dx = sqrt(dx*dx + dzx*dzx); 
//dy = sqrt(dy*dy + dzy*dzy); 
////in call freldim
//delt = dx * dy;
//  
//// 1/sqrt(dhds)
////eslope = CalcEslope(index, m_pcs);
//GradH[0] = (haa[0] - haa[1] - haa[2] + haa[3]) / 2.0*dx;
//GradH[1] = (haa[0] + haa[1] - haa[2] - haa[3]) / 2.0*dy ;
//// dh/ds (dh in the direction of maximum slope)
//dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1]));
//dhds = MMax(1.0e-4,dhds);
//eslope = 1.0 / sqrt(dhds);
//
///* Chezy-coefficient: C */
///* für b >> h gilt: C = H**1/6 n**-1 */
//if(m_pcs->m_msh){
//  if (m_mmp->friction_model==2){ // Chezy-coefficient C
//    axx = eslope * fric * dy/dx; //ett/ell
//    ayy = eslope * fric * dx/dy;
//    power = 1.0 / 2.0;
//  }
//  else{
//    axx = eslope / fric * dy/dx; //ett/ell
//    ayy = eslope / fric * dx/dy;
//    power = 2.0 / 3.0;
//  }
//}
//else{
//  if (m_mmp->friction_model == 2)  {
//    axx = eslope * fric * dy/dx; //ett/ell
//    ayy = eslope * fric * dx/dy;
//    power = 1.0 / 2.0;
//  }
//  else{
//    axx = eslope / fric * dy/dx; //ett/ell
//    ayy = eslope / fric * dx/dy;
//    power = 2.0 / 3.0;
//  }
//}
//ast = delt /(double) (dt * 4); 
//
////Compute constitutive relationships: swval, swold, krwval
//CalcNLTERMS(nn, Haa, HaaOld, power, swval, swold, krwval);
//
////Compute upstream weighting: ckwr, iups
//CalcCKWR(nn, haa, z, power, ckwr, iups);
//
////Form elemental matrix
// for (i = 0; i < nn; i++){
//   for (j = (i+1); j < nn; j++){
//	 //gammaij = ckwr[i][j] * ((edlluse[i][j] * axx) + (edttuse[i][j]* ayy));
//     gammaij = ckwr[i*nn+j] * ((edlluse[i*nn+j] * axx) + (edttuse[i*nn+j]* ayy));
//	    amat[i][j]= gammaij;
//     amat[j][i]= gammaij;
//     amat[i][i]= amat[i][i] - gammaij;
//     amat[j][j]= amat[j][j] - gammaij;
//   }
// }
////double test1[16];
////for (i = 0; i < nn; i++){
////  for (j = 0; j < nn; j++){
////    test1[i*4+j] = amat[i][j];
////  }
////}
////MZeigMat(test1,4,4,"NewtonJacobian_msh");
//
//for (i = 0; i < nn; i++){
//  rhs[i] = -ast * (swval[i] - swold[i]);
//}
//
////////////////////////////////////////////////
////Jacobian for Newton Raphson
//////////////////////////////////////////////////
//double epsilon = 1.e-5;
//double head_eps[4];
//double head_keep[4];
//double krwval_eps[4];
//double oldkrw;
//double oldhead;
//double head[4];
//double sumjac=0.0;
//double flow_depth;
//double akrw;
//double amateps = 0.0;
//double amatold;
//double jacobian[4][4];
//double epsinv;
//double stor_eps;
//double sum;
//double storinit[4];
//double astor[4];
//double residual[4];
//double swval_eps[4];
//double test[16];
//
//MNulleVec(astor,4);
//epsinv = 1 / epsilon;
//
////Form the residual excluding the right hand side vector
//for(i=0; i<nn; i++)  {
//  sum=0.0;
//  for(j=0; j<nn; j++)  {
//	sum=sum + (amat[i][j]*( haa[j])); 
//  }
//  storinit[i]= -rhs[i]+astor[i]*Haa[i];
//  residual[i]=sum+storinit[i];
//}
//
////Compute non-linear terms: krw and Sw using current pressures
////     plus epsilon (to compute the Jacobian numerically)
//for(i=0; i<nn; i++)  {
//  head_eps[i] = Haa[i] + epsilon;
//  head_keep[i] = Haa[i];
//  head[i] = Haa[i];
//}
//
//CalcNLTERMS(nn, head_eps, HaaOld, power, swval_eps, swold, krwval_eps);
//
//// Form jacobian !
//for(i=0; i<nn; i++)  {
//
//  oldkrw=krwval[i];
//  krwval[i]=krwval_eps[i];
//  oldhead=Haa[i];
//  head[i]=head_eps[i];
//  sumjac=0.0;
//
//  for(j=0; j<nn; j++)  {
//	if(i!=j) {
//      //if(i==iups[i][j]){
//      if(i==iups[i*nn+j]){
//        maxZ = MMax(z[i],z[j]);
//		flow_depth = head[i] + z[i] - maxZ;
//		if(flow_depth<0.0) {flow_depth = 0.0;}
//		akrw = flow_depth*(pow(flow_depth, power));
//	  }
//	  else{
//        maxZ = MMax(z[i],z[j]);
//		flow_depth = head[j] + z[j] - maxZ;
//		if(flow_depth<0.0) {flow_depth = 0.0;}
//		akrw = flow_depth*(pow(flow_depth, power));
//	  }
//		
//	  //gammaij= akrw*( axx*edlluse[i][j] + ayy*edttuse[i][j] );
//   gammaij= akrw*( axx*edlluse[i*nn+j] + ayy*edttuse[i*nn+j] );
//
//	  amateps = gammaij*(head[j]+z[j]-head[i]-z[i]);
//	  amatold = amat[i][j] * (head_keep[j] + z[j] - head_keep[i] -z[i]);
//	  jacobian[j][i] = -(amateps-amatold) * epsinv;
//      sumjac = sumjac + amateps;
//
//	} //end if (i!=j)
//  } //end j
//
//  //Compute diagonal for row i 
//  //Lump the storage term
//  stor_eps=ast*(swval_eps[i]-swold[i]);
//  sumjac=sumjac+stor_eps;
//  jacobian[i][i]=(sumjac-residual[i])*epsinv ; 
//  head[i]=oldhead;
//  krwval[i]=oldkrw;
//} // end i
//
//for (i = 0; i < nn; i++){
//  for (j = 0; j < nn; j++){
//    test[i*4+j] = jacobian[i][j];
//  }
//}
////MZeigMat(test,4,4,"test: jacobian-MATRIX");
////Speichern von Jacobian in conductance_matrix
//for (i = 0; i < 16; i++) {
//  conductance_matrix[i] = test[i];
//}
//
////Speichern von residual in gravity_vector 
//gravity_vector[0]= - residual[0];
//gravity_vector[1]= - residual[1];
//gravity_vector[2]= - residual[2];
//gravity_vector[3]= - residual[3];
//
//MZeigMat(conductance_matrix,4,4,"a2Ele1.txt");
//MZeigVec(gravity_vector,4,"a2Ele1Graf.txt");
//
//
///* Elementmatrizen speichern */
//if(memory_opt==0) { /* schnelle, speicherintensive Variante */
//if(m_pcs){
//  SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
//  SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
//  SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
//}
//else{
//  ASMSetElementCapacitanceMatrix(index, capacitance_matrix);
//  ASMSetElementConductanceMatrix(index, conductance_matrix);
//  ASMSetElementGravityVector(index, gravity_vector);
//}
//}
//
//#ifdef TESTCEL_ASM
//  MZeigMat(conductance_matrix,4,4,"CalcEle2D_ASM: CONDUCTANCE-MATRIX");
//  MZeigVec(gravity_vector,4,"CalcEle2D_ASM: GRAVITY-VECTOR");
//#endif
//
//  element_nodes = NULL;
}

/**************************************************************************
FEMLib-Method: 
Task:    Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) 
   für Overland Flow
Programing:
02/2005 MB Implementation
**************************************************************************/
void CalcEle2DTriangle_OF(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
long *element_nodes;
long i,j;
double Haa[3];
double HaaOld[3];
double haa[3];
double z[3];
int nidx1, nidx0;
double fric;
double eslope;
double axx;
double ayy;
double delt;
double power;
double ast=0.0;
double krwval[3];
double amat[3][3];
double gammaij;
double maxZ;
double swval[3];
double swold[3];
double rhs[3];
double edlluse[9];
double edttuse[9];
double ckwr[9];
int iups[9];
double x2, x3, y2, y3;
double delt2inv, delt2;
double b[3], g[3];
int nn;
int nn2;
double GradH[2];
double dhds;

/* Numerische Parameter */
element_nodes = ElGetElementNodes(index);
nn = 3;
nn2 = nn*nn;

/* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
if(memory_opt==0) { /* schnelle, speicherintensive Variante */
  conductance_matrix = (double *) Malloc(9 * sizeof(double));
  capacitance_matrix = (double *) Malloc(9 * sizeof(double));
  gravity_vector = (double *) Malloc(3 * sizeof(double));
}

/* Initialisieren */
MNulleMat(conductance_matrix, nn, nn);
MNulleMat(capacitance_matrix, nn, nn);
MNulleMat(ckwr, nn, nn);
MNulleMat(edlluse, nn, nn);
MNulleMat(edttuse, nn, nn);
MNulleVec(rhs,nn);
for (i = 0; i < nn; i++){
  for (j = 0; j < nn; j++){
    amat[i][j]= 0.0;
  }
}
  
/* MMP medium properties */
CMediumProperties *m_mmp = NULL;
long group = ElGetElementGroupNumber(index);
m_mmp = mmp_vector[group];
double* permeability;
permeability = m_mmp->PermeabilityTensor(index);
fric = permeability[0]; /* friction-coefficient: Manning or Chezy */ 

nidx1 = PCSGetNODValueIndex("HEAD",1);
nidx0 = PCSGetNODValueIndex("HEAD",0);
  
/* Heads */
haa[0] = GetNodeVal(element_nodes[0],nidx1);
haa[1] = GetNodeVal(element_nodes[1],nidx1);
haa[2] = GetNodeVal(element_nodes[2],nidx1);

z[0] = GetNodeZ(element_nodes[0]);
z[1] = GetNodeZ(element_nodes[1]);
z[2] = GetNodeZ(element_nodes[2]);

/* Wassertiefen */
Haa[0] = haa[0] - z[0];  if (Haa[0] < 0.0) {Haa[0] = 0.0;}
Haa[1] = haa[1] - z[1];  if (Haa[1] < 0.0) {Haa[1] = 0.0;}
Haa[2] = haa[2] - z[2];  if (Haa[2] < 0.0) {Haa[2] = 0.0;}
HaaOld[0] = GetNodeVal(element_nodes[0],nidx0) - z[0];  if (HaaOld[0] < 0.0) {HaaOld[0] = 0.0;}
HaaOld[1] = GetNodeVal(element_nodes[1],nidx0) - z[1];  if (HaaOld[1] < 0.0) {HaaOld[1] = 0.0;}
HaaOld[2] = GetNodeVal(element_nodes[2],nidx0) - z[2];  if (HaaOld[2] < 0.0) {HaaOld[2] = 0.0;}

//Element Dimensions
//Achtung nur für Dreiecke !
x2 = GetNodeX(element_nodes[1])-GetNodeX(element_nodes[0]);
x3 = GetNodeX(element_nodes[2])-GetNodeX(element_nodes[0]);
y2 = GetNodeY(element_nodes[1])-GetNodeY(element_nodes[0]);
y3 = GetNodeY(element_nodes[2])-GetNodeY(element_nodes[0]);

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

//Get Influence Coefficient Matrices for analytical integration
//Get_Matrix_Quad(edlluse, edttuse);
for(i=0; i<nn; i++)  {
  for(j=0; j<nn; j++)  {
	//edlluse[i][j] = b[i] * b[j];
	//edttuse[i][j] = g[i] * g[j];
    edlluse[i*nn + j] = b[i] * b[j];
	edttuse[i*nn + j] = g[i] * g[j];
  }
}

// 1/sqrt(dhds)
//for Triangles neu!!!!
GradH[0] = ( b[0]*haa[0] + b[1]*haa[1] +  b[2]*haa[2] );
GradH[1] =( g[0]*haa[0] + g[1]*haa[1] +  g[2]*haa[2] );
// dh/ds (dh in the direction of maximum slope)
dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1]));
dhds = MMax(1.0e-4,dhds);
eslope = 1.0 / sqrt(dhds);

/* Chezy-coefficient: C */
/* für b >> h gilt: C = H**1/6 n**-1 */
if (m_mmp->conductivity_model == 3)  {
  /////////////for Triangles neu
  axx = eslope * fric * delt;
  ayy = eslope * fric * delt;
  power = 1.0 / 2.0;
}
else{
  axx = eslope / fric * delt;
  ayy = eslope / fric * delt;
  power = 2.0 / 3.0;
}

ast = delt /(double) (dt * nn); 

//Compute constitutive relationships: swval, swold, krwval
CalcNLTERMS(nn, Haa, HaaOld, power, swval, swold, krwval);

//Compute upstream weighting: ckwr, iups
CalcCKWR(nn, haa, z, power, ckwr, iups);

//Form elemental matrix
 for (i = 0; i < nn; i++){
   for (j = (i+1); j < nn; j++){
	 //gammaij = ckwr[i][j] * ((edlluse[i][j] * axx) + (edttuse[i][j]* ayy));
     gammaij = ckwr[i*nn+j] * ((edlluse[i*nn+j] * axx) + (edttuse[i*nn+j]* ayy));
	 amat[i][j]= gammaij;
     amat[j][i]= gammaij;
     amat[i][i]= amat[i][i] - gammaij;
     amat[j][j]= amat[j][j] - gammaij;
   }
 }

for (i = 0; i < nn; i++){
  rhs[i] = -ast * (swval[i] - swold[i]);
}

//////////////////////////////////////////////
//Jacobian for Newton Raphson
////////////////////////////////////////////////
double epsilon = 1.e-5;
double head_eps[3];
double head_keep[3];
double krwval_eps[3];
double oldkrw;
double oldhead;
double head[3];
double sumjac=0.0;
double flow_depth;
double akrw;
double amateps = 0.0;
double amatold;
double jacobian[3][3];
double epsinv;
double stor_eps;
double sum;
double storinit[3];
double astor[3];
double residual[3];
double swval_eps[3];
double test[9];

MNulleVec(astor,nn);
epsinv = 1 / epsilon;

//Form the residual excluding the right hand side vector
for(i=0; i<nn; i++)  {
  sum=0.0;
  for(j=0; j<nn; j++)  {
	sum=sum + (amat[i][j]*( haa[j])); 
  }
  storinit[i]= -rhs[i]+astor[i]*Haa[i];
  residual[i]=sum+storinit[i];
}

//Compute non-linear terms: krw and Sw using current pressures
//     plus epsilon (to compute the Jacobian numerically)
for(i=0; i<nn; i++)  {
  head_eps[i] = Haa[i] + epsilon;
  head_keep[i] = Haa[i];
  head[i] = Haa[i];
}

CalcNLTERMS(nn, head_eps, HaaOld, power, swval_eps, swold, krwval_eps);

// Form jacobian !
for(i=0; i<nn; i++)  {

  oldkrw=krwval[i];
  krwval[i]=krwval_eps[i];
  oldhead=Haa[i];
  head[i]=head_eps[i];
  sumjac=0.0;

  for(j=0; j<nn; j++)  {
	if(i!=j) {
      //if(i==iups[i][j]){
      if(i==iups[i*nn+j]){
        maxZ = MMax(z[i],z[j]);
		flow_depth = head[i] + z[i] - maxZ;
		if(flow_depth<0.0) {flow_depth = 0.0;}
		akrw = flow_depth*(pow(flow_depth, power));
	  }
	  else{
        maxZ = MMax(z[i],z[j]);
		flow_depth = head[j] + z[j] - maxZ;
		if(flow_depth<0.0) {flow_depth = 0.0;}
		akrw = flow_depth*(pow(flow_depth, power));
	  }
		
	  //gammaij= akrw*( axx*edlluse[i][j] + ayy*edttuse[i][j] );
      gammaij= akrw*( axx*edlluse[i*nn+j] + ayy*edttuse[i*nn+j] );

	  amateps = gammaij*(head[j]+z[j]-head[i]-z[i]);
	  amatold = amat[i][j] * (head_keep[j] + z[j] - head_keep[i] -z[i]);
	  jacobian[j][i] = -(amateps-amatold) * epsinv;
      sumjac = sumjac + amateps;

	} //end if (i!=j)
  } //end j

  //Compute diagonal for row i 
  //Lump the storage term
  stor_eps=ast*(swval_eps[i]-swold[i]);
  sumjac=sumjac+stor_eps;
  jacobian[i][i]=(sumjac-residual[i])*epsinv ; 
  head[i]=oldhead;
  krwval[i]=oldkrw;
} // end i

for (i = 0; i < nn; i++){
  for (j = 0; j < nn; j++){
    test[i*nn+j] = jacobian[i][j];
  }
}
//MZeigMat(test,4,4,"test: jacobian-MATRIX");
//Speichern von Jacobian in conductance_matrix
for (i = 0; i < nn2; i++) {
  conductance_matrix[i] = test[i];
}

//Speichern von residual in gravity_vector 
gravity_vector[0]= - residual[0];
gravity_vector[1]= - residual[1];
gravity_vector[2]= - residual[2];

 /*MZeigMat(conductance_matrix,4,4,"CalcEle2D_ASM: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,4,"CalcEle2D_ASM: GRAVITY-VECTOR");*/

/* Elementmatrizen speichern */
if(memory_opt==0) { /* schnelle, speicherintensive Variante */
if(m_pcs){
  SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
  SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
  SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
}
else{
  ASMSetElementCapacitanceMatrix(index, capacitance_matrix);
  ASMSetElementConductanceMatrix(index, conductance_matrix);
  ASMSetElementGravityVector(index, gravity_vector);
}
}

#ifdef TESTCEL_ASM
  MZeigMat(capacitance_matrix,nn,nn,"CalcEle2D_ASM: CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix,nn,nn,"CalcEle2D_ASM: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,nn,"CalcEle2D_ASM: GRAVITY-VECTOR");
#endif

  element_nodes = NULL;
}


/**************************************************************************
FEMLib-Method: 
Task: Calculates 1 / squrt(dh/ds) for Overland Flow
Programing:
06/2005 MB Implementation
**************************************************************************/
double CalcEslope(long index,  CRFProcess *m_pcs)
{
index = index;
m_pcs = NULL;
//double grad_omega[8];
//double haa[4];
//int nidx1;
//long *element_nodes;
//double v1[2];
//double invjac[4],detjac;
//double GradH[2];
//double dhds;
double eslope = 0.;
//int r,s,t;
//int nn;
//int i,j;
//FiniteElement::CElement* m_ele = NULL;
//  
//if(m_pcs->m_msh){
//  m_ele = m_pcs->m_msh->ele_vector[index];
//  m_ele->ConfigElement(index, 1, m_ele);
//  
//  m_ele->unit[0] = m_ele->unit[1] = m_ele->unit[2] = 0.0 ;
//  if(m_ele->ElementType==4)
//     m_ele->unit[0] = m_ele->unit[1] = .5 ;
//  else if(m_ele->ElementType==5)
//     m_ele->unit[0] = m_ele->unit[1] = m_ele->unit[2] = 0.5; 
//    
//  element_nodes = m_ele->nodes;
//  nn = m_ele->nnodes;
//
//  nidx1 = GetNodeValueIndex("HEAD")+1;
//  for(i=0;i<nn;i++){
//    haa[i] = GetNodeValue(element_nodes[i],nidx1);
//  }
//
//  m_ele->GetGaussData(0, r,s,t);
//  m_ele->ComputeGradShapefct(1);
//  //dh
//  for(i=0;i<m_ele->Dim();i++)
//  {
//     GradH[i]= 0.0;
//     for(j=0;j<nn;j++){
//       GradH[i] = m_ele->dshapefct[i*nn+j]*haa[j];
//     }
//  }
//   
//
//
//}
//
//else {
//element_nodes = ElGetElementNodes(index);
//
//nidx1 = PCSGetNODValueIndex("HEAD",1);
//haa[0] = GetNodeVal(element_nodes[0],nidx1);
//haa[1] = GetNodeVal(element_nodes[1],nidx1);
//haa[2] = GetNodeVal(element_nodes[2],nidx1);
//haa[3] = GetNodeVal(element_nodes[3],nidx1);
//
//}
//
//// dh/dx and dh/dy
//MGradOmega2D(grad_omega, 0., 0.);
///* Grad h */
//MMultMatVec(grad_omega, 2, 4, haa, 4, v1, 2);
////m_ele->computeJacobian;
//Calc2DElementJacobiMatrix(index, 0.0, 0.0, invjac, &detjac);
//MMultVecMat(v1, 2, invjac, 2, 2, GradH, 2);
//// dh/ds (dh in the direction of maximum slope)
//dhds = sqrt((GradH[0] * GradH[0]) + (GradH[1] * GradH[1]));
//dhds = MMax(1.0e-4,dhds);
////dhds = MMax(1.0e-10,dhds);
//eslope = 1.0 / sqrt(dhds);
//
return eslope;
}



/**************************************************************************
FEMLib-Method: 
Task: Calculates upstream weighting for Overland Flow -> ckwr and iups 
Programing:
06/2005 MB Implementation
**************************************************************************/
void CalcCKWR(int nn, double* haa, double* z, double power, double* test, int* xxx)
{
double pandz[4];
int i, j;
int iups[4][4];
double ckwr[4][4];
double maxZ;
double H;
double zwu;

pandz[0] =  haa[0]; 
pandz[1] =  haa[1]; 
pandz[2] =  haa[2]; 
pandz[3] =  haa[3]; 

for (i = 0; i < nn; i++){
  for (j = 0; j < nn; j++){
    iups[i][j] = i;
    if(pandz[j]>pandz[i]){
      iups[i][j]= j;
    }
  iups[j][i] = iups[i][j];
      
  if(i==iups[i][j]){
    maxZ = MMax(z[i],z[j]);
	H = haa[i] - maxZ;
    if(H<0.0) H = 0.0;
    zwu = pow(H,power);
    ckwr[i][j] = H * zwu;        
  }
  else {
    maxZ = MMax(z[i],z[j]);
    H = haa[j] - maxZ;
	if(H<0.0) H = 0.0;
      zwu = pow(H,power);
      ckwr[i][j] = H * zwu;  
    }
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
Task: Calculates consitutive relationships for Overland Flow -> swval, swold and krwval
Programing:
06/2005 MB Implementation
**************************************************************************/
void CalcNLTERMS(int nn, double* Haa, double* HaaOld, double power, double* swval, double* swold, double* krwval)
{
int i;
double H;
double zwu;
double volume_depth;
	
//Compute_nlterms_olf
for(i=0; i<nn; i++)  {
  H = Haa[i]; 
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
} //end for

}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle3D_ASM
                                                                          */
/* Aufgabe:
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 3D - Elements ohne 1/dt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index                        Element-Index
   R double *capacitance_matrix        Element-Matrix
   R double double *conductance_matrix Element-Matrix
   R double double *gravity_vector     Element-Vector
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1997   RK   Uebertragen aus dem aTM
   07/1997   RK   Angepasst an das aSM
   01/1998   CT   Dichtestroemungen
   11/2000   OK   Direkte Rckgabe der Matrizen fuer Speicheroptimierung
   08/2002   MK   Kopplungsgroeï¿½n DM -> SM (coupling_matrix_pu)
   11/2002   MB   MATCalcFluidDensity
08/2004 OK MFP Implementation
08/2004 OK MMP Implementation
10/2004 WW New coupling matrix computation in the context of OOP 
02/2005 MB head version                                                   */
/**************************************************************************/
void CalcEle3D_ASM(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
    static double invjac[9], detjac;
    static double gravity_matrix[64];
    static long *element_nodes;
    static double elevation[8];
    static double g, my, theta, rho;
    static double tkmyt[9], kmy[9];
    static int anzgp, nn, nd;
    static long i, j, k, l; //, m;
    static double r, s, t, fkt;
    static double zwi[64];
    static double zwa[24], zwo[9];
    static double phi[8], ome[24];
/* DM -> MMP - Kopplungsgroeï¿½n */
//    static double invjac_t_20N[9],detjac_20N;
//    static double g_ome_t_20N[60];  
//   static double g_ome_xyz_t_20N[60]; 
    static double *coupling_matrix_u_x, *coupling_matrix_u_y, *coupling_matrix_u_z;

    static double storativity,k_xx,k_yy,k_zz;
    static double *perm;
    static int phase;
    static double gp[3]; 
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  anzgp = m_pcs->m_num->ele_gauss_points;
  //----------------------------------------------------------------------
    g = gravity_constant;
    nn = NumbersOfElementNode(index);
    if(memory_opt==0) { /* schnelle, speicherintensive Variante */
        capacitance_matrix = (double *) Malloc(64 * sizeof(double));
        conductance_matrix = (double *) Malloc(64 * sizeof(double));
        gravity_vector = (double *) Malloc(8 * sizeof(double));
        if(GetRFProcessProcessing("SD")) { /* DM -> SM - Kopplungsgroeï¿½n */
          nd = NumbersOfElementNodeHQ(index);
          coupling_matrix_u_x = (double *) Malloc(nn*nd*sizeof(double));
          coupling_matrix_u_y = (double *) Malloc(nn*nd*sizeof(double));
          coupling_matrix_u_z = (double *) Malloc(nn*nd*sizeof(double));
        }
    }

    /* Initialisieren */
    MNulleMat(capacitance_matrix, 8, 8);
    MNulleMat(conductance_matrix, 8, 8);
    MNulleMat(gravity_matrix, 8, 8);
    MNulleVec(gravity_vector, 8);
    if(GetRFProcessProcessing("SD")) { /* DM -> SM - Kopplungsgroeï¿½n */
        MNulleMat(coupling_matrix_u_x,nn,nd);
        MNulleMat(coupling_matrix_u_y,nn,nd);
        MNulleMat(coupling_matrix_u_z,nn,nd);
    }

  // Consolidation
  bool sm_cpl_deformation = false;
  string pcs_problem_type = PCSProblemType();
  if(pcs_problem_type.find("DEFORMATION")!=string::npos)
    sm_cpl_deformation = true;

    /* Material */
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  perm = m_mat_mp->PermeabilityTensor(index);
  k_xx = perm[0];
  k_yy = perm[4];
  k_zz = perm[8];
  storativity = m_mat_mp->StorageFunction(index,gp,theta);
  //----------------------------------------------------------------------
  // Fluid properties
  phase = 0;
  
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();
  
  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    my = 1.0;
    rho = 1.0;
  }

    /* Schleife ueber GaussPunkte */
    for (i = 0; i < anzgp; i++)
        for (j = 0; j < anzgp; j++)
            for (k = 0; k < anzgp; k++) {
                r = MXPGaussPkt(anzgp, i);
                s = MXPGaussPkt(anzgp, j);
                t = MXPGaussPkt(anzgp, k);

                /* conductance tensor */
                MNulleMat(kmy, 3, 3);
                kmy[0] = (k_xx) / my;
                kmy[4] = (k_yy) / my;
                kmy[8] = (k_zz) / my;

                Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
                /* Faktor */
                fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * \
                    MXPGaussFkt(anzgp, k) * detjac;

                /* tkmyt = (J^-1)T * (K/my) * J^-1 */
                MMultMatMat(kmy, 3, 3, invjac, 3, 3, zwa, 3, 3);
                MTranspoMat(invjac, 3, 3, zwi);
                MMultMatMat(zwi, 3, 3, zwa, 3, 3, tkmyt, 3, 3);

                /* capacitance matrix */
                /* phi * omega */
                MPhi3D(phi, r, s, t);
                MOmega3D(zwo, r, s, t);
                MMultVecVec(phi, 8, zwo, 8, zwi, 8, 8);
                for (l = 0; l < 64; l++)
                    capacitance_matrix[l] += (zwi[l] * fkt);

                /* conductance matrix und gravity matrix */
                /* grad(phi)T */
                MGradPhi3D(zwa, r, s, t);       /* 3 Zeilen 8 Spalten */
                MTranspoMat(zwa, 3, 8, zwi);    /* jetzt: 8 Zeilen 3 Spalten */
                /* grad(phi)T * tkmyt */
                MMultMatMat(zwi, 8, 3, tkmyt, 3, 3, zwa, 8, 3);
                /* grad(omega) */
                MGradOmega3D(ome, r, s, t);     /* 3 Zeilen 8 Spalten */
                /* grad(phi)T * tkmyt * grad(omega) */
                MMultMatMat(zwa, 8, 3, ome, 3, 8, zwi, 8, 8);

                for (l = 0; l < 64; l++) {
                    conductance_matrix[l] += (zwi[l] * fkt);
                    gravity_matrix[l] += (zwi[l] * fkt) * rho * g;
                }


           }                   /* Ende der Schleife ueber GaussPunkte */

    for (l = 0; l < 64; l++) {
        capacitance_matrix[l] *= (storativity);
    }

    /* Schwerkraftterm berechnen (gravity vector) */
    element_nodes = ElGetElementNodes(index);
    for (l = 0; l < 8; l++)
        elevation[l] = ((GetNode(element_nodes[l]))->z);

    element_nodes = NULL;

    /* gravity matrix * elevation */
    MMultMatVec(gravity_matrix, 8, 8, elevation, 8, gravity_vector, 8);

               /*- coupling matrices --------------------------------------------*/
    if(memory_opt==0) { /* schnelle, speicherintensive Variante */
       SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
       SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
       SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
   }


#ifdef TESTCEL_ASM
    MZeigMat(capacitance_matrix, 8, 8, "3d CAPACITANCE-MATRIX");
    MZeigMat(conductance_matrix, 8, 8, "   CONDUCTANCE-MATRIX");
    MZeigVec(gravity_vector, 8, "GRAVITY-VECTOR");
#endif
}


/**************************************************************************
FEMLib-Method: 
Task:
Programing:
05/2000 OK Erste Version
08/2000 OK Beliebige Raumlage der Dreieckselemente
11/2000 OK Direkte Rckgabe der Matrizen fuer Speicheroptimierung
07/2003 WW Coupling matrix
08/2004 OK MFP Implementation
08/2004 OK MMP Implementation
11/2004 OK NUM 
**************************************************************************/
void CalcEle2DTriangle_ASM(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
  static double k_xx,k_yy,storativity;
  static double g, my, rho; /* , rho_add */ /* UJ rf3217*/
  static double *coupling_matrix_u_x, *coupling_matrix_u_y;
  //  static double invjac[4];
  //  static double g_ome_xy_t[12]; 
  static long *element_nodes;
  static double x[3],y[3],elevation[3];
  static long i;
  static double theta;
  static int nn,nn2, nd;
  static double fac_C,fac_K;
  static double volume;
  //  static double ome[3];
  //  static double unit[3]; /* Unit coordinates of an element*/
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // Geometry
  element_nodes = ElGetElementNodes(index);
  nn = 3;
  nd = 6;
  nn2 = 9;
  volume = ElGetElementVolume(index); // Element-Volumen: Flaeche*Dicke, CMCD thickness
  Calc2DElementCoordinatesTriangle(index,x,y,NULL); // Element-Koordinaten im lokalen System (x',y')==(a,b)
  //----------------------------------------------------------------------
  // Consolidation
  bool sm_cpl_deformation = false;
  string pcs_problem_type = PCSProblemType();
  if(pcs_problem_type.find("DEFORMATION")!=string::npos)
    sm_cpl_deformation = true;
  //----------------------------------------------------------------------
  // Memory allocation
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    capacitance_matrix    = (double*)Malloc(nn2*sizeof(double));
    conductance_matrix    = (double*)Malloc(nn2*sizeof(double));
    gravity_vector        = (double*)Malloc(nn *sizeof(double));
    if(sm_cpl_deformation){
      coupling_matrix_u_x = (double*)Malloc(nn*nd*sizeof(double));
      coupling_matrix_u_y = (double*)Malloc(nn*nd*sizeof(double));
    }
  }
  //----------------------------------------------------------------------
  // Initializations
  MNulleMat(capacitance_matrix,nn,nn);
  MNulleMat(conductance_matrix,nn,nn);
  MNulleVec(gravity_vector,nn);
  if(sm_cpl_deformation){
    MNulleMat(coupling_matrix_u_x,nn,nd);
    MNulleMat(coupling_matrix_u_y,nn,nd);
  }
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  double* permeability;
  permeability = m_mat_mp->PermeabilityTensor(index);
  k_xx = permeability[0];
  k_yy = permeability[3];
  storativity = m_mat_mp->StorageFunction(index,NULL,theta);
  //----------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  int phase = 0;
  double gp[3];
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();
if(gravity_constant<MKleinsteZahl){ // HEAD version
  my = 1.0;
  rho = 1.0;
}
  //======================================================================
  // Element matrices
  //----------------------------------------------------------------------
  // Capacitance matrix
  capacitance_matrix[0] = 2.0;
  capacitance_matrix[1] = 1.0;
  capacitance_matrix[2] = 1.0;
  capacitance_matrix[3] = 1.0;
  capacitance_matrix[4] = 2.0;
  capacitance_matrix[5] = 1.0;
  capacitance_matrix[6] = 1.0;
  capacitance_matrix[7] = 1.0;
  capacitance_matrix[8] = 2.0;
  fac_C = (storativity*volume)/(12.);
  //----------------------------------------------------------------------
  // Conductance matrix
  conductance_matrix[0] = k_xx*(y[1]-y[2])*(y[1]-y[2]) + k_yy*(x[2]-x[1])*(x[2]-x[1]);
  conductance_matrix[1] = k_xx*(y[1]-y[2])*(y[2]-y[0]) + k_yy*(x[2]-x[1])*(x[0]-x[2]);
  conductance_matrix[2] = k_xx*(y[1]-y[2])*(y[0]-y[1]) + k_yy*(x[2]-x[1])*(x[1]-x[0]);
  conductance_matrix[3] = k_xx*(y[2]-y[0])*(y[1]-y[2]) + k_yy*(x[0]-x[2])*(x[2]-x[1]);
  conductance_matrix[4] = k_xx*(y[2]-y[0])*(y[2]-y[0]) + k_yy*(x[0]-x[2])*(x[0]-x[2]);
  conductance_matrix[5] = k_xx*(y[2]-y[0])*(y[0]-y[1]) + k_yy*(x[0]-x[2])*(x[1]-x[0]);
  conductance_matrix[6] = k_xx*(y[0]-y[1])*(y[1]-y[2]) + k_yy*(x[1]-x[0])*(x[2]-x[1]);
  conductance_matrix[7] = k_xx*(y[0]-y[1])*(y[2]-y[0]) + k_yy*(x[1]-x[0])*(x[0]-x[2]);
  conductance_matrix[8] = k_xx*(y[0]-y[1])*(y[0]-y[1]) + k_yy*(x[1]-x[0])*(x[1]-x[0]);
  fac_K = 1./(4.*volume*my);
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  // Volumenelemente 
  for(i=0;i<nn2;i++){
    capacitance_matrix[i] *= fac_C;
    conductance_matrix[i] *= fac_K;
  }
  //----------------------------------------------------------------------
  // Gravity term
  for(i=0;i<nn;i++){
    elevation[i] = (GetNode(element_nodes[i])->z)*rho*g;
  }
  MMultMatVec(conductance_matrix,nn,nn,elevation,nn,gravity_vector,nn);
  //----------------------------------------------------------------------
  // Elementmatrizen speichern 
    SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
    SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
    SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
  //----------------------------------------------------------------------
#ifdef TESTCEL_ASM
  MZeigMat(capacitance_matrix,nn,nn,"CalcEle2DTriangle_ASM: CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix,nn,nn,"CalcEle2DTriangle_ASM: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,nn,"CalcEle2DTriangle_ASM: GRAVITY-VECTOR");
  if(sm_cpl_deformation) {
    MZeigMat(coupling_matrix_u_x,nn,nn,"CalcEle2DTriangle_ASM: STRAIN COUPLING MATRIX X");
    MZeigMat(coupling_matrix_u_y,nn,nn,"CalcEle2DTriangle_ASM: STRAIN COUPLING MATRIX Y");
  }
#endif
  element_nodes = NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle2DTetraeder_ASM
                                                                          */
/* Aufgabe:
   Elementmatrizen fuer lineares Tetraederelement
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/2000   OK   Erste Version
                                                                          */
/**************************************************************************/
void CalcEle3DTetraeder_ASM(long index,CRFProcess *m_pcs)
{
  /* Materials */
  static double k_xx,k_yy,k_zz,storativity;
  static double g, my, rho; /* , rho_add */ /* UJ rf3217*/
  /* Matrices */
  static double *capacitance_matrix;
  static double *conductance_matrix;
  static double *gravity_vector;
  static double *coupling_matrix_u_x, *coupling_matrix_u_y, *coupling_matrix_u_z;
  /* */
  static long *element_nodes;
  static double x[4],y[4],z[4],elevation[4];
  static long i;
  static double theta;
  static int nn,nd, nn2;
  static double fac_C,fac_K;
  static double volume;
  static double b1,b2,b3,b4;
  static double c1,c2,c3,c4;
  static double d1,d2,d3,d4;
  static double mat3x3[9];
  static double GrdShpFct[12],GrdShpFctT[12];
  static double GrdShpFctLoc[12];
  static double Jac[9],DetJac;
  static double mat4x3[12];
  static double perm[9];
  double gp[3];
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  element_nodes = ElGetElementNodes(index);
  nn = 4;
  nd = 10;
  nn2 = nn*nn;

  // Consolidation
  bool sm_cpl_deformation = false;
  string pcs_problem_type = PCSProblemType();
  if(pcs_problem_type.find("DEFORMATION")!=string::npos)
    sm_cpl_deformation = true;

  //WW
  // Memory allocation
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
  capacitance_matrix  = (double *) Malloc(nn2 * sizeof(double));
  conductance_matrix  = (double *) Malloc(nn2 * sizeof(double));
  gravity_vector = (double *) Malloc(nn * sizeof(double));
    if(sm_cpl_deformation){
      coupling_matrix_u_x = (double*)Malloc(nn*nd*sizeof(double));
      coupling_matrix_u_y = (double*)Malloc(nn*nd*sizeof(double));
      coupling_matrix_u_z = (double*)Malloc(nn*nd*sizeof(double));
    }
  }
  // Initializations
  MNulleMat(capacitance_matrix,nn,nn);
  MNulleMat(conductance_matrix,nn,nn);
  MNulleVec(gravity_vector,nn);
  if(sm_cpl_deformation){
    MNulleMat(coupling_matrix_u_x,nn,nd);
    MNulleMat(coupling_matrix_u_y,nn,nd);
    MNulleMat(coupling_matrix_u_z,nn,nd);
  }
  //----------------------------------------------------------------------
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0; //CMCD move this function 
  CalculateSimpleMiddelPointElement (index,gp);
  //----------------------------------------------------------------------
  // MMP Medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  //k_xx = k_yy = k_zz = m_mat_mp->permeability;
  double* permeability;
  permeability = m_mat_mp->PermeabilityTensor(index);
  storativity = m_mat_mp->StorageFunction(index,NULL,theta);
  MNulleVec(perm,9);
  perm[0] = permeability[0];
  perm[4] = permeability[4];
  perm[8] = permeability[8];
  //----------------------------------------------------------------------
  // Fluid properties
  int phase = 0;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();
if(m_pcs->primary_variable_name.compare("HEAD")==0){
  my = 1.0;
  rho = 1.0;
}
  //----------------------------------------------------------------------
  // Element-Geometrie
  volume = ElGetElementVolume(index);
  for(i=0;i<nn;i++) {
    x[i] = GetNodeX(element_nodes[i]);
    y[i] = GetNodeY(element_nodes[i]);
    z[i] = GetNodeZ(element_nodes[i]);
  }
  //----------------------------------------------------------------------
  // Form-Funktionen -> mathlib 
  mat3x3[0]=1.0; mat3x3[1]=y[1]; mat3x3[2]=z[1];
  mat3x3[3]=1.0; mat3x3[4]=y[2]; mat3x3[5]=z[2];
  mat3x3[6]=1.0; mat3x3[7]=y[3]; mat3x3[8]=z[3];
  b1 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=1.0; mat3x3[1]=y[2]; mat3x3[2]=z[2];
  mat3x3[3]=1.0; mat3x3[4]=y[3]; mat3x3[5]=z[3];
  mat3x3[6]=1.0; mat3x3[7]=y[0]; mat3x3[8]=z[0];
  b2 = 1.0 * M3Determinante(mat3x3);
  mat3x3[0]=1.0; mat3x3[1]=y[3]; mat3x3[2]=z[3];
  mat3x3[3]=1.0; mat3x3[4]=y[0]; mat3x3[5]=z[0];
  mat3x3[6]=1.0; mat3x3[7]=y[1]; mat3x3[8]=z[1];
  b3 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=1.0; mat3x3[1]=y[0]; mat3x3[2]=z[0];
  mat3x3[3]=1.0; mat3x3[4]=y[1]; mat3x3[5]=z[1];
  mat3x3[6]=1.0; mat3x3[7]=y[2]; mat3x3[8]=z[2];
  b4 = 1.0 * M3Determinante(mat3x3);
  GrdShpFct[0]=b1/6./volume;
  GrdShpFct[1]=b2/6./volume;
  GrdShpFct[2]=b3/6./volume;
  GrdShpFct[3]=b4/6./volume;

  mat3x3[0]=x[1]; mat3x3[1]=1.0; mat3x3[2]=z[1];
  mat3x3[3]=x[2]; mat3x3[4]=1.0; mat3x3[5]=z[2];
  mat3x3[6]=x[3]; mat3x3[7]=1.0; mat3x3[8]=z[3];
  c1 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[2]; mat3x3[1]=1.0; mat3x3[2]=z[2];
  mat3x3[3]=x[3]; mat3x3[4]=1.0; mat3x3[5]=z[3];
  mat3x3[6]=x[0]; mat3x3[7]=1.0; mat3x3[8]=z[0];
  c2 = 1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[3]; mat3x3[1]=1.0; mat3x3[2]=z[3];
  mat3x3[3]=x[0]; mat3x3[4]=1.0; mat3x3[5]=z[0];
  mat3x3[6]=x[1]; mat3x3[7]=1.0; mat3x3[8]=z[1];
  c3 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[0]; mat3x3[1]=1.0; mat3x3[2]=z[0];
  mat3x3[3]=x[1]; mat3x3[4]=1.0; mat3x3[5]=z[1];
  mat3x3[6]=x[2]; mat3x3[7]=1.0; mat3x3[8]=z[2];
  c4 = 1.0 * M3Determinante(mat3x3);
  GrdShpFct[4]=c1/6./volume;
  GrdShpFct[5]=c2/6./volume;
  GrdShpFct[6]=c3/6./volume;
  GrdShpFct[7]=c4/6./volume;

  mat3x3[0]=x[1]; mat3x3[1]=y[1]; mat3x3[2]=1.0;
  mat3x3[3]=x[2]; mat3x3[4]=y[2]; mat3x3[5]=1.0;
  mat3x3[6]=x[3]; mat3x3[7]=y[3]; mat3x3[8]=1.0;
  d1 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[2]; mat3x3[1]=y[2]; mat3x3[2]=1.0;
  mat3x3[3]=x[3]; mat3x3[4]=y[3]; mat3x3[5]=1.0;
  mat3x3[6]=x[0]; mat3x3[7]=y[0]; mat3x3[8]=1.0;
  d2 = 1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[3]; mat3x3[1]=y[3]; mat3x3[2]=1.0;
  mat3x3[3]=x[0]; mat3x3[4]=y[0]; mat3x3[5]=1.0;
  mat3x3[6]=x[1]; mat3x3[7]=y[1]; mat3x3[8]=1.0;
  d3 = -1.0 * M3Determinante(mat3x3);
  mat3x3[0]=x[0]; mat3x3[1]=y[0]; mat3x3[2]=1.0;
  mat3x3[3]=x[1]; mat3x3[4]=y[1]; mat3x3[5]=1.0;
  mat3x3[6]=x[2]; mat3x3[7]=y[2]; mat3x3[8]=1.0;
  d4 = 1.0 * M3Determinante(mat3x3);
  GrdShpFct[8]=d1/6./volume;
  GrdShpFct[9]=d2/6./volume;
  GrdShpFct[10]=d3/6./volume;
  GrdShpFct[11]=d4/6./volume;

  GrdShpFctLoc[0] = -1.0;
  GrdShpFctLoc[1] =  1.0;
  GrdShpFctLoc[2] =  0.0;
  GrdShpFctLoc[3] =  0.0;

  GrdShpFctLoc[4] = -1.0;
  GrdShpFctLoc[5] =  0.0;
  GrdShpFctLoc[6] =  1.0;
  GrdShpFctLoc[7] =  0.0;

  GrdShpFctLoc[8] = -1.0;
  GrdShpFctLoc[9] =  0.0;
  GrdShpFctLoc[10] =  0.0;
  GrdShpFctLoc[11] =  1.0;
    
  Jac[0] = -1.0*x[0] + x[1];
  Jac[1] = -1.0*y[0] + y[1];
  Jac[2] = -1.0*z[0] + z[1];
  Jac[3] = -1.0*x[0] + x[2];
  Jac[4] = -1.0*y[0] + y[2];
  Jac[5] = -1.0*z[0] + z[2];
  Jac[6] = -1.0*x[0] + x[3];
  Jac[7] = -1.0*y[0] + y[3];
  Jac[8] = -1.0*z[0] + z[3];

  DetJac = M3Determinante(Jac);
  M3Invertiere(Jac);

  MMultMatMat(Jac,3,3,GrdShpFctLoc,3,4,GrdShpFct,3,4);
  MTranspoMat(GrdShpFct,3,4,GrdShpFctT);
  //----------------------------------------------------------------------
  // Capacitance matrix
  capacitance_matrix[0] = 2.0;
  capacitance_matrix[1] = 1.0;
  capacitance_matrix[2] = 1.0;
  capacitance_matrix[3] = 1.0;
  capacitance_matrix[4] = 1.0;
  capacitance_matrix[5] = 2.0;
  capacitance_matrix[6] = 1.0;
  capacitance_matrix[7] = 1.0;
  capacitance_matrix[8] = 1.0;
  capacitance_matrix[9] = 1.0;
  capacitance_matrix[10] = 2.0;
  capacitance_matrix[11] = 1.0;
  capacitance_matrix[12] = 1.0;
  capacitance_matrix[13] = 1.0;
  capacitance_matrix[14] = 1.0;
  capacitance_matrix[15] = 2.0;
  fac_C = (storativity*volume)/(20.);
  //----------------------------------------------------------------------
  // conductance matrix
  conductance_matrix[0]  = k_xx*b1*b1 + k_yy*c1*c1 + k_zz*d1*d1;
  conductance_matrix[1]  = k_xx*b1*b2 + k_yy*c1*c2 + k_zz*d1*d2;
  conductance_matrix[2]  = k_xx*b1*b3 + k_yy*c1*c3 + k_zz*d1*d3;
  conductance_matrix[3]  = k_xx*b1*b4 + k_yy*c1*c4 + k_zz*d1*d4;
  conductance_matrix[4]  = k_xx*b2*b1 + k_yy*c2*c1 + k_zz*d2*d1;
  conductance_matrix[5]  = k_xx*b2*b2 + k_yy*c2*c2 + k_zz*d2*d2;
  conductance_matrix[6]  = k_xx*b2*b3 + k_yy*c2*c3 + k_zz*d2*d3;
  conductance_matrix[7]  = k_xx*b2*b4 + k_yy*c2*c4 + k_zz*d2*d4;
  conductance_matrix[8]  = k_xx*b3*b1 + k_yy*c3*c1 + k_zz*d3*d1;
  conductance_matrix[9]  = k_xx*b3*b2 + k_yy*c3*c2 + k_zz*d3*d2;
  conductance_matrix[10] = k_xx*b3*b3 + k_yy*c3*c3 + k_zz*d3*d3;
  conductance_matrix[11] = k_xx*b3*b4 + k_yy*c3*c4 + k_zz*d3*d4;
  conductance_matrix[12] = k_xx*b4*b1 + k_yy*c4*c1 + k_zz*d4*d1;
  conductance_matrix[13] = k_xx*b4*b2 + k_yy*c4*c2 + k_zz*d4*d2;
  conductance_matrix[14] = k_xx*b4*b3 + k_yy*c4*c3 + k_zz*d4*d3;
  conductance_matrix[15] = k_xx*b4*b4 + k_yy*c4*c4 + k_zz*d4*d4;
  fac_K = 1./(36.*volume*my);

  MMultMatMat(GrdShpFctT,4,3,perm,3,3,mat4x3,4,3);
  MMultMatMat(mat4x3,4,3,GrdShpFct,3,4,conductance_matrix,4,4);
  fac_K = 1.0;
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  // Volumenelemente 
  for (i=0;i<nn2;i++) {
    capacitance_matrix[i] *= fac_C;
    conductance_matrix[i] *= fac_K;
  }
  //----------------------------------------------------------------------
  // Schwerkraftterm berechnen (gravity vector)
if(!(m_pcs->primary_variable_name.compare("HEAD")==0)){
  for(i=0;i<nn;i++)
    elevation[i] = (GetNode(element_nodes[i])->z)*rho*g;
    /* gravity matrix * elevation */
  MMultMatVec(conductance_matrix,nn,nn,elevation,nn,gravity_vector,nn);
}
  //----------------------------------------------------------------------
 // Elementmatrizen speichern 
    SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
    SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
    SMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
#ifdef TESTCEL_ASM
  MZeigMat(capacitance_matrix,nn,nn,"CalcEle3DTetraeder_ASM: CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix,nn,nn,"CalcEle3DTetraeder_ASM: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,nn,"CalcEle3DTetraeder_ASM: GRAVITY-VECTOR");
  if(GetRFProcessProcessing("SD")) {
    MZeigMat(coupling_matrix_u_x,nn,nn,"CalcEle3DTetraeder_ASM: STRAIN COUPLING MATRIX X");
    MZeigMat(coupling_matrix_u_y,nn,nn,"CalcEle3DTetraeder_ASM: STRAIN COUPLING MATRIX Y");
  }
#endif
  element_nodes = NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle3DPrism_ASM_num
                                                                          */
/* Aufgabe:
   Numerische Berechnung Elementmatrizen fuer lineares Dreiecksprisma
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2003   MB   Erste Version aufgebaut auf CalcEle2DTriangle_ASM
11/2004 OK NUM 
02/2005  MB  head version                                                       */
/**************************************************************************/
void CalcEle3DPrism_ASM_num(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
  /* Numerik */
  static double theta;
  static long *element_nodes=NULL;
  static int anzgptri, anzgplin;
  static double r, s, t, fkt;
  static double gp[3]; 
  static int nn=6;
  static int nn2=36;
  /* Material */
  //static double storativity,k_xx,k_yy,k_zz;
  static double storativity;
  static int phase;
  static double g, my, rho;
  static double elevation[6];
  /* Matrizen */
  static double invjac[9], detjac;
  static double OmPrism[8];
  static double GradOmPrism[18];
  static double TransGradOmPrism[18];
  static double TransInvjac[18];
  static double tkmyt[9], kmmy[9];
  static long i, j, l;
  static double zwa[18];
  static double zwi[36]; // 18->36 MB please check
  static double zwu[36];


  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    capacitance_matrix  = (double *) Malloc(nn2 * sizeof(double));
    conductance_matrix  = (double *) Malloc(nn2 * sizeof(double));
    gravity_vector = (double *) Malloc(nn * sizeof(double));
  }

  /* Initialisieren */
  MNulleMat(capacitance_matrix,nn,nn);
  MNulleMat(conductance_matrix,nn,nn);
  MNulleVec(gravity_vector,nn);
  MNulleMat(kmmy, 3, 3); 
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  anzgptri = 3;  /* Anzahl Gausspunkte; lineares Dreieck */
  anzgplin = 2;  /* Anzahl Gausspunkte; Linearkomponente */
  /* Materialparameter */
  g = gravity_constant;
  //----------------------------------------------------------------------
  // Fluid properties
  phase = 0;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();
  
  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    my = 1.0;
    rho = 1.0;
  }
  //----------------------------------------------------------------------
  // Medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  double* permeability;
  permeability = m_mat_mp->PermeabilityTensor(index);
  kmmy[0] = permeability[0] / my;
  kmmy[4] = permeability[4] / my;
  kmmy[8] = permeability[8] / my;
  storativity = m_mat_mp->StorageFunction(index,gp,theta);
  /*------------------------------------------------------------------------*/
  /* Schleife ueber GaussPunkte */
  for (i = 0; i < anzgplin; i++)  {
    for (j = 0; j < anzgptri; j++)  {
        
      r = MXPGaussPktTri(anzgptri,j,0);
      s = MXPGaussPktTri(anzgptri,j,1);
      t = MXPGaussPkt(anzgplin,i);

      CalcPrismElementJacobiMatrix(index, r, s, t, invjac, &detjac);
      MTranspoMat(invjac, 3, 3, TransInvjac);

      /* Wichtung der Gausspunkte */
      fkt = MXPGaussFktTri(anzgptri, j) * MXPGaussFkt(anzgplin,i) * detjac;

      /*------------------------------------------------------------------------*/
      /*---- Capacitance matrix ------------------------------------------------*/
      /*------------------------------------------------------------------------*/
      
      /* Omega T * Omega * fkt */
      MOmegaPrism(OmPrism, r, s, t);
      MMultVecVec(OmPrism, nn, OmPrism, nn, zwi, nn, nn);
      for (l = 0; l < nn2; l++)  {
        capacitance_matrix[l] += (zwi[l] * fkt);
      }

      /*------------------------------------------------------------------------*/
      /*---- Conductance matrix ------------------------------------------------*/
      /*------------------------------------------------------------------------*/

      /* tkmyt = (J^-1)T * (K/my) * J^-1 */
      MMultMatMat(kmmy, 3, 3, invjac, 3, 3, zwa, 3, 3);
      MMultMatMat(TransInvjac, 3, 3, zwa, 3, 3, tkmyt, 3, 3);
 
      /* GradOmega T */
      MGradOmegaPrism( r, s, t, GradOmPrism);               /* 3 Zeilen 6 Spalten */
      MTranspoMat(GradOmPrism, 3, nn, TransGradOmPrism);    /* 6 Zeilen 3 Spalten */
      /* GradOmega T * tkmyt */
      MMultMatMat(TransGradOmPrism,nn,3,tkmyt,3,3, zwi, nn, 3);
      /* GradOmega T * tkmyt * GradOmega */
      MMultMatMat(zwi,nn,3,GradOmPrism,3,nn,zwu,6,6);
      /* GradOmega T * tkmyt * GradOmega * fkt */
      for (l = 0; l < nn2; l++) {
        conductance_matrix[l] += (zwu[l] * fkt); 
      }

    }  /* Ende Schleife über Anzahl der GaussPunkte im Dreieck (xy-Richtung) */
  }    /* Ende Schleife über Anzahl der GaussPunkte in z Richtung */


  for (l = 0; l < nn2 ; l++) {
    capacitance_matrix[l] *= (storativity);
  }

  /*------------------------------------------------------------------------*/
  /*---- gravity vector ----------------------------------------------------*/
  /*------------------------------------------------------------------------*/
  element_nodes = ElGetElementNodes(index);
  for (i=0;i<nn;i++)  {
    elevation[i] = (GetNode(element_nodes[i])->z)*rho*g;  
  }
  MMultMatVec(conductance_matrix,nn,nn,elevation,nn,gravity_vector,nn);

  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    if(m_pcs){
      SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
      SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
      SMSetElementGravityVector(index, gravity_vector,m_pcs->pcs_number);
    }
    else{
      ASMSetElementCapacitanceMatrix(index, capacitance_matrix);
      ASMSetElementConductanceMatrix(index, conductance_matrix);
      ASMSetElementGravityVector(index, gravity_vector);
    }
  }

#ifdef TESTCEL_ASM
  MZeigMat(capacitance_matrix,nn,nn,"CalcEle3DPrism_ASM_num: CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix,nn,nn,"CalcEle3DPrism_ASM_num: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,nn,"CalcEle3DPrism_ASM_num: GRAVITY-VECTOR");
#endif
}
    

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle3DPrism_ASM_ana
                                                                          */
/* Aufgabe:
   Analytische Berechnung Elementmatrizen fuer lineares Dreiecksprisma
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2003   MB     Erste Version aufgebaut auf CalcEle2DTriangle_ASM
11/2004 OK NUM 
02/2005  MB  head version                                                 */
/**************************************************************************/
void CalcEle3DPrism_ASM_ana(long index,\
                   double *capacitance_matrix,\
                   double *conductance_matrix,\
                   double *gravity_vector,\
                   CRFProcess *m_pcs)
{
  /* Numerik */
  static double theta;
  static double gp[3];
  static int nn=6;
  static int nn2=36;
  /* Material */
  static double storativity,k_xx,k_yy,k_zz;
  static int phase;
  static double g, my, rho; 
  /* Element */
  static double area;
  static double elevation[6];
  static long *element_nodes=NULL;
  /* Matrizen */
  static double InvJac3Dz;
  static double DetJac3Dz;
  static double GradXTri[3];
  static double GradYTri[3];
  static double Tri_x_Tri[9];
  static double GradXTri_x_GradXTri[9];
  static double GradYTri_x_GradYTri[9];
  static double Kijxx[36];
  static double Kijyy[36];
  static double Kijzz[36];
  static double Ct[36];
  static double GradNGradN[36];
  /* Laufvariablen */
  static long i;
  static double fac1;
  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    capacitance_matrix  = (double *) Malloc(nn2 * sizeof(double));
    conductance_matrix  = (double *) Malloc(nn2 * sizeof(double));
    gravity_vector = (double *) Malloc(nn * sizeof(double));
  }
  /* Initialisieren */
  MNulleMat(capacitance_matrix,nn,nn);
  MNulleMat(conductance_matrix,nn,nn);
  MNulleVec(gravity_vector,nn);
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  /* Elementdaten */
  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 
  /* Get linear component matrices */
  Get_Nt_x_Nt(Ct);
  Get_gradNt_x_gradNt(GradNGradN);
  InvJac3Dz = GetInvJac3Dz(index);
  DetJac3Dz = 1.0 / InvJac3Dz;

  /* Materialparameter */
  g = gravity_constant;
  //----------------------------------------------------------------------
  // Fluid properties
  phase = 0;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  my  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    my = 1.0;
    rho = 1.0;
  }

  //----------------------------------------------------------------------
  // Medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double* permeability;
  permeability = m_mmp->PermeabilityTensor(index);
  k_xx = permeability[0] / my;
  k_yy = permeability[4] / my;
  k_zz = permeability[8] / my;
  storativity = m_mmp->StorageFunction(index,gp,theta);
 
  Get_NTrinangle_x_NTrinangle(index, Tri_x_Tri);  
  MMultMatSkalar(Tri_x_Tri,DetJac3Dz,3,3);
  GetPriMatFromTriMat(Tri_x_Tri, capacitance_matrix);
  MMultMatMat2(capacitance_matrix, 6, 6, Ct, capacitance_matrix);
  MMultMatSkalar(capacitance_matrix,storativity,6,6);    


  /*------------------------------------------------------------------------*/
  /*---- Conductance matrix ------------------------------------------------*/
  /*------------------------------------------------------------------------*/

  /*----------------------------------------------------------------*/
  /* Calc Kijxx */
  fac1 = DetJac3Dz * area;  
  CalcGradXTri(index, GradXTri);
  MMultVecVec(GradXTri,3,GradXTri,3,GradXTri_x_GradXTri,3,3);
  MMultMatSkalar(GradXTri_x_GradXTri,fac1,3,3);
  GetPriMatFromTriMat(GradXTri_x_GradXTri, Kijxx);
  MMultMatMat2(Kijxx, 6, 6, Ct, Kijxx);
  MMultMatSkalar(Kijxx,k_xx,6,6);

  /* Calc Kijyy */
  CalcGradYTri(index, GradYTri);
  MMultVecVec(GradYTri,3,GradYTri,3,GradYTri_x_GradYTri,3,3);
  MMultMatSkalar(GradYTri_x_GradYTri,fac1,3,3);
  GetPriMatFromTriMat(GradYTri_x_GradYTri, Kijyy);
  MMultMatMat2(Kijyy, 6, 6, Ct, Kijyy);  
  MMultMatSkalar(Kijyy,k_yy,6,6);
    
  /* Calc Kijzz */
  Get_NTrinangle_x_NTrinangle(index, Tri_x_Tri);
  MMultMatSkalar(Tri_x_Tri,InvJac3Dz,3,3);
  GetPriMatFromTriMat(Tri_x_Tri, Kijzz);
  MMultMatMat2(Kijzz, 6, 6, GradNGradN, Kijzz);
  MMultMatSkalar(Kijzz,k_zz,6,6);

  for (i=0;i<nn2;i++) {
    conductance_matrix[i] = Kijxx[i] + Kijyy[i] + Kijzz[i];
  }


  /*------------------------------------------------------------------------*/
  /*---- Gravity Vector ----------------------------------------------------*/
  /*------------------------------------------------------------------------*/
  for (i=0;i<nn;i++)
    elevation[i] = (GetNode(element_nodes[i])->z)*rho*g;
  /* gravity matrix * elevation */
  MMultMatVec(conductance_matrix,nn,nn,elevation,nn,gravity_vector,nn);


  /*------------------------------------------------------------------------*/
  /* Elementmatrizen speichern */
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    if(m_pcs){
      SMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
      SMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
      SMSetElementGravityVector(index, gravity_vector,m_pcs->pcs_number);
    }
    else{
      ASMSetElementCapacitanceMatrix(index, capacitance_matrix);
      ASMSetElementConductanceMatrix(index, conductance_matrix);
      ASMSetElementGravityVector(index, gravity_vector);
    }
  }


#ifdef TESTCEL_ASMp
  MZeigMat(capacitance_matrix,nn,nn,"CalcEle3DPrism_ASM_ana: CAPACITANCE-MATRIX");
  MZeigMat(conductance_matrix,nn,nn,"CalcEle3DPrism_ASM_ana: CONDUCTANCE-MATRIX");
  MZeigVec(gravity_vector,nn,"CalcEle3DPrism_ASM_ana: GRAVITY-VECTOR");
#endif

}

      

/*************************************************************************
ROCKFLOW - Function: SMCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 05/2003 OK Implementation
last modified:
**************************************************************************/
void *SMCreateELEMatricesPointer(void)
{ 
  SMElementMatrices *data = NULL;
  data = (SMElementMatrices *) Malloc(sizeof(SMElementMatrices));
  data->capacitancematrix = NULL;
  data->conductancematrix = NULL;
  data->gravityvector = NULL;
  data->strain_coupling_matrix_x = NULL;
  data->strain_coupling_matrix_y = NULL;
  data->strain_coupling_matrix_z = NULL;

  return (void *) data;
}

/*************************************************************************
ROCKFLOW - Function: SMDestroyELEMatricesPointer
Task: Destruct element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *SMDestroyELEMatricesPointer(void *data)
{
//OKnew
 if(data) {
  if(((SMElementMatrices *) data)->capacitancematrix)
     ((SMElementMatrices *) data)->capacitancematrix = \
       (double *) Free(((SMElementMatrices *) data)->capacitancematrix);
  if(((SMElementMatrices *) data)->conductancematrix)
     ((SMElementMatrices *) data)->conductancematrix = \
       (double *) Free(((SMElementMatrices *) data)->conductancematrix);
  if(((SMElementMatrices *) data)->gravityvector)
     ((SMElementMatrices *) data)->gravityvector = \
       (double *) Free(((SMElementMatrices *) data)->gravityvector);
//OKnew
 if(GetRFProcessProcessing("SD")) {
  if(((SMElementMatrices *) data)->strain_coupling_matrix_x)
     ((SMElementMatrices *) data)->strain_coupling_matrix_x = \
       (double *) Free(((SMElementMatrices *) data)->strain_coupling_matrix_x);
  if(((SMElementMatrices *) data)->strain_coupling_matrix_y)
     ((SMElementMatrices *) data)->strain_coupling_matrix_y = \
       (double *) Free(((SMElementMatrices *) data)->strain_coupling_matrix_y);
  if(((SMElementMatrices *) data)->strain_coupling_matrix_z)
     ((SMElementMatrices *) data)->strain_coupling_matrix_z = \
       (double *) Free(((SMElementMatrices *) data)->strain_coupling_matrix_z);
 }
  data = (void*) Free(data);
 }
  return data;
}

/*------------------------------------------------------------------------*/
/* ElementCapacitanceMatrix */
void SMSetElementCapacitanceMatrix(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->capacitancematrix =  (double *)Free(data->capacitancematrix);
  data->capacitancematrix = matrix;
}

double *SMGetElementCapacitanceMatrix(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->capacitancematrix;
}

/*------------------------------------------------------------------------*/
/* ElementConductanceMatrix */
void SMSetElementConductanceMatrix(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->conductancematrix =  (double *)Free(data->conductancematrix);
  data->conductancematrix = matrix;
}

double *SMGetElementConductanceMatrix(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->conductancematrix;
}

/*------------------------------------------------------------------------*/
/* ElementGravityVector */
void SMSetElementGravityVector(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->gravityvector =  (double *)Free(data->gravityvector);
  data->gravityvector = matrix;
}

double *SMGetElementGravityVector(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->gravityvector;
}

/*------------------------------------------------------------------------*/
/* ElementStrainCouplingXMatrix */
void SMSetElementStrainCouplingMatrixX(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
    //WW, will be removed if class is involved
  data->strain_coupling_matrix_x =  (double *)Free(data->strain_coupling_matrix_x);

  data->strain_coupling_matrix_x = matrix;
}

double *SMGetElementStrainCouplingMatrixX(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->strain_coupling_matrix_x;
}

/*------------------------------------------------------------------------*/
/* ElementStrainCouplingYMatrix */
void SMSetElementStrainCouplingMatrixY(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
 
  //WW, will be removed if class is involved
  data->strain_coupling_matrix_y =  (double *)Free(data->strain_coupling_matrix_y);
  
  data->strain_coupling_matrix_y = matrix;
}

double *SMGetElementStrainCouplingMatrixY(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->strain_coupling_matrix_y;
}


//WW ElementStrainCouplingYMatrix 
void SMSetElementStrainCouplingMatrixZ(long number,double *matrix,int pcs_number)
{
  static SMElementMatrices *data;
  data = (SMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
 
  //WW, will be removed if class is involved
  data->strain_coupling_matrix_z =  (double *)Free(data->strain_coupling_matrix_z);
  
  data->strain_coupling_matrix_z = matrix;
}

double *SMGetElementStrainCouplingMatrixZ(long number,int pcs_number)
{
  return ((SMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->strain_coupling_matrix_z;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcElementMatrix_ASM
                                                                          */
/* Aufgabe:
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix)
   und den Schwerkraftvektor (gravity vector) des angegebenen Elements
   ohne 1/dt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1997   RK   Uebertragen aus dem aTM und angepasst an das aSM
                  (-> MakeMat_ASM)
   04/1997   RK   Umbenannt zu CalcElementMatrix_ASM
   07/2000   OK   Erweiterung fuer Dreieck-Elemente
                                                                          */
/**************************************************************************/
void CalcElementMatrix_ASM(long index)
{
    switch (ElGetElementType(index)) {
        case 1:
        CalcEle1D_ASM(index,NULL,NULL,NULL,NULL);
        break;
    case 2:
        CalcEle2D_ASM(index,NULL,NULL,NULL,NULL);
        break;
    case 3:
        CalcEle3D_ASM(index,NULL,NULL,NULL,NULL);
        break;
    case 4:
        CalcEle2DTriangle_ASM(index,NULL,NULL,NULL,NULL);
        break;
    case 5:
        CalcEle3DTetraeder_ASM(index,NULL);
        break;
    case 6:
        #ifdef NUMERIC_ASM /* Numerische Lösung */
        CalcEle3DPrism_ASM_num(index,NULL,NULL,NULL,NULL);
        #else              /* Analytische Lösung */
        CalcEle3DPrism_ASM_ana(index,NULL,NULL,NULL,NULL);
        #endif
        break;
    }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CalcElementMatrix_ASM_New
                                                                          */
/* Aufgabe:
   Berechnung der Element-Matrizen fuer Kernel ASM
   verschiedene Speichermodelle
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: long index                     - Elementnummer
   R: double *capacitance_matrix_cgs - Element-Matrix
   R: double *conductance_matrix_cgs - Element-Matrix
   R: double *gravity_vector_cgs     - Element-Vektor
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/2000   OK   aus CalcElementMatrix_ASM (RK) abgeleitet
                                                                          */
/**************************************************************************/
void CalcElementMatrix_ASM_New(long index,\
                               double *capacitance_matrix_cgs,\
                               double *conductance_matrix_cgs,\
                               double *gravity_vector_cgs)
{
  switch (ElGetElementType(index)) {
      case 1:
               CalcEle1D_ASM(index,capacitance_matrix_cgs,\
                                   conductance_matrix_cgs,\
                                   gravity_vector_cgs,NULL);
               break;
      case 2:
               CalcEle2D_ASM(index,capacitance_matrix_cgs,\
                                   conductance_matrix_cgs,\
                                   gravity_vector_cgs,NULL);
               break;
      case 3:
               CalcEle3D_ASM(index,capacitance_matrix_cgs,\
                                   conductance_matrix_cgs,\
                                   gravity_vector_cgs,NULL);
               break;
      case 4:
               CalcEle2DTriangle_ASM(index,\
                                     capacitance_matrix_cgs,\
                                     conductance_matrix_cgs,\
                                     gravity_vector_cgs,NULL);
               break;
  }
}

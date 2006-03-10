/**************************************************************************
 ROCKFLOW - Modul: cel_agm.c

 Aufgabe:
 Funktionen zur Berechnung der Elementmatrizen fuer Modul aGM

 Programmaenderungen:
 04/1997 OK Implementation
 05/2000 RK Anpassungen an Permeabilitaetsmodell
 10/2004 OK PCS2
 **************************************************************************/
#include "stdafx.h" 
#include "makros.h"
#include "nodes.h"
#include "elements.h"
#include "rf_num_new.h"
#include "tools.h"
#include "femlib.h"
#include "mathlib.h"
#include "int_agm.h"
#include "cel_agm.h"
#include "rf_pcs.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "par_ddc.h"
extern double gravity_constant;

#define noTESTCEL_AGM

/**************************************************************************
 ROCKFLOW - Funktion: GMCalcElementMatrix1D

 Aufgabe:
  Berechnet die Speichermatrix (capacitance matrix),
  die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
  den Schwerkraftvektor (gravity vector)
  des angegebenen 1D - Elements ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E long index: Elementnummer

  Ergebnis:
  - void -

 Programmaenderungen:
   04/97       OK          Anpassung fuer Gasmodell
   letzte Aenderung: OK 26.05.1997

 **************************************************************************/
static void GMCalcElementMatrix1D(long index,CRFProcess* m_pcs)
{
/* Element matrices */
  static long *element_nodes;
  static double *invjac, detjac;
  static double *capacitance_matrix;
  static double *conductance_matrix;
  static double *gravity_vector;
  static double elevation[2];
  static double vorfk;
/* Soil properties */
  static double porosity;
/* Fluid properties */
  static double tkmut,g,rho,mu;
  static double *k_r;
  int phase = 0;
  double gp[3]; 
/* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  capacitance_matrix = (double *) Malloc(4*sizeof(double)); /* Speicherfreigabe??? */
  conductance_matrix = (double *) Malloc(4*sizeof(double));
  gravity_vector = (double *) Malloc(2*sizeof(double));

/**************************************************************************/
/* 1 Provide material properties */
  //--------------------------------------------------------------------
  // NUM
  double theta = m_pcs->m_num->ls_theta;
  /* Determinante und Inverse der Jacobi-Matrix bereitstellen */
    /* detjac = AL/2, invjac = [J_1D^-1][1D_T_3D] = 2/L !!! */
  invjac = GetElementJacobiMatrix(index,&detjac);
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  //----------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  porosity = m_mat_mp->Porosity(index,gp,theta);
  k_r = m_mat_mp->PermeabilityTensor(index);
  k_r[0] *= MSkalarprodukt(invjac,invjac,3);
  tkmut = k_r[0] / mu;
/**************************************************************************/
/* 2 Calculate element matrices  */
  /* Speichermatrix (capacitance matrix) s. Abschn. A4.2 */
    /* vorfk = S_0 * A * L / 6, S_0=phi */
  vorfk = detjac * Mdrittel * porosity;
  capacitance_matrix[0] = 2.0 * vorfk;
  capacitance_matrix[1] = 1.0 * vorfk;
  capacitance_matrix[2] = 1.0 * vorfk;
  capacitance_matrix[3] = 2.0 * vorfk;
  GMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
  /* Hydr. Leitfaehigkeitsmatrix (conductance matrix) s. Abschn. A4.2 */
    /* vorfk = k / mu * A / L  */
  vorfk = (detjac * 0.5) * tkmut;
  conductance_matrix[0] = +1.0 * vorfk;
  conductance_matrix[1] = -1.0 * vorfk;
  conductance_matrix[2] = -1.0 * vorfk;
  conductance_matrix[3] = +1.0 * vorfk;
  GMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
  /* Schwerkraftterm berechnen (gravity vector) */
  element_nodes = ElGetElementNodes(index);
  elevation[0] = (GetNode(element_nodes[0]))->z;
  elevation[1] = (GetNode(element_nodes[1]))->z;
  element_nodes = NULL;
    /* vorfk = (k * rho * g) / (L * mu)  */
  vorfk = (detjac * 0.5) * tkmut * rho * g;
  gravity_vector[0] = vorfk * (elevation[0]-elevation[1]);
  gravity_vector[1] = vorfk * (elevation[1]-elevation[0]);
  GMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
#ifdef TESTCEL_AGM
  MZeigMat(capacitance_matrix,2,2,"CalcEle1D_AGM: Capacitance matrix");
  MZeigMat(conductance_matrix,2,2,"CalcEle1D_AGM: Conductance matrix");
  MZeigVec(gravity_vector,2,"CalcEle1D_AGM: Gravity vector");
#endif
}

/**************************************************************************
        ROCKFLOW - Funktion: CalcEle2D_AGM

        Aufgabe:
        Berechnet die Speichermatrix (capacitance matrix),
        die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
        den Schwerkraftvektor (gravity vector)
        des angegebenen 2D - Elementes ohne 1/dt.

        Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
        E long index: Elementnummer

        Ergebnis:
   - void -

   Programmaenderungen:

   04/97       OK          Anpassung fuer Gasmodell
   10.03.1999  C.Thorenz    anz_matxx, start_mat entfernt

 **************************************************************************/
static void GMCalcElementMatrix2D(long index,CRFProcess* m_pcs)
{
  static double *capacitance_matrix;
  static double *conductance_matrix;
  static double *gravity_vector;
  static double gravity_matrix[16];
  static long *element_nodes;
  static double elevation[4];
  static int anzgp;
  static long i,j,k;
  static double r,s;
  static double rho, g, mu,fkt;
  static double zwi[16];
  static double zwa[8],zwo[4];
  static double phi[4],ome[8];
  static double detjac,invjac[4];
  static double kmu[4],tkmut[4];
  static double element_thickness, porosity;
  static double *k_ij; //,*k_rs; /* Speicher */
  double gp[3]; 
  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  capacitance_matrix = (double *) Malloc(16*sizeof(double));
  conductance_matrix = (double *) Malloc(16*sizeof(double));
  gravity_vector = (double *) Malloc(4*sizeof(double));
  /*k_ij = (double *) Malloc(4*sizeof(double));*/
  /*k_rs = (double *) Malloc(4*sizeof(double));
  Groesse in aufrufender Funktion definiert */
  /* Initialisieren der zu berechnenden Elementmatrizen und -vektoren */
  MNulleMat(capacitance_matrix,4,4);
  MNulleMat(conductance_matrix,4,4);
  MNulleVec(gravity_vector,4);
  MNulleMat(kmu,2,2);
  //--------------------------------------------------------------------
  // NUM
  double theta = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  int phase = 0;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  porosity = m_mmp->Porosity(index,gp,theta);
  element_thickness = m_mmp->geo_area;
  k_ij = m_mmp->PermeabilityTensor(index);
  /* hydr. Widerstand - kmu = k / mu */
  for(i=0;i<4;i++) {
      kmu[i] = k_ij[i] / mu;
      k_ij[i] = k_ij[i] / mu;
  }
  /* Gausspunkte zur Integration */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  /* 2. Berechnen der Element-Integrale mittels Gauss-Quadratur,
        Schleife ueber die Gauss-Punkte */
  for (i=0;i<anzgp;i++)
      for (j=0;j<anzgp;j++) {
          r = MXPGaussPkt(anzgp,i);
          s = MXPGaussPkt(anzgp,j);
          /* Berechnen der Element-Jacobi-Matrix [J_2D],
             ihrer Determinante det[J_2D] sowie Inversen [J_2D^-1]
             in den Gauss-Punkten */
          Calc2DElementJacobiMatrix(index,r,s,invjac,&detjac);
          /* Faktor g_i * g_j * det[J_2D] */
          fkt = MXPGaussFkt(anzgp,i) * MXPGaussFkt(anzgp,j) * detjac;
          /* tkmut = [J_2D^-1] * (k/mu) * [J_2D^-1]^T */
          MMultMatMat(invjac,2,2,kmu,2,2,zwa,2,2);
          MTranspoMat(invjac,2,2,zwi);
          MMultMatMat(zwa,2,2,zwi,2,2,tkmut,2,2);
          /*k_rs = CalculateLocalPermeabilityTensorRST(index,k_ij,r,s,0);*/
          //k_rs = Calculate2DLocalPermeabilityTensorRS(invjac,k_ij); //SB:m
          /* Aufstellen der hydr. Speichermatrix [C] */
             /* Ansatzfunktion im Gauss-Punkt phi==N */
          MPhi2D(phi,r,s);
            /* Wichtungsfunktionen im Gauss-Punkt omega==N */
          MOmega2D(zwo,r,s);
            /* Berechnen phi * omega */
          MMultVecVec(phi,4,zwo,4,zwi,4,4);
          for (k=0;k<16;k++)
              capacitance_matrix[k] += (zwi[k] * fkt);
          /* Aufstellen der hydr. Leitfaehigkeits-Elementmatrix [K]*/
          /* conductance matrix und gravity matrix*/
            /* grad(phi)T */
          MGradPhi2D(zwa,r,s); /* 2 Zeilen 4 Spalten */
          MTranspoMat(zwa,2,4,zwi); /* jetzt: 4 Zeilen 2 Spalten */
            /* grad(phi)^T * tkmut */
          MMultMatMat(zwi,4,2,tkmut,2,2,zwa,4,2);
          //OK MMultMatMat(zwi,4,2,k_rs,2,2,zwa,4,2);
            /* grad(omega) */
          MGradOmega2D(ome,r,s); /* 2 Zeilen 4 Spalten */
            /* grad(phi)^T * tkmut * grad(omega) */
          MMultMatMat(zwa,4,2,ome,2,4,zwi,4,4);
          for (k=0;k<16;k++) {
              conductance_matrix[k] += (zwi[k] * fkt);
              gravity_matrix[k] = conductance_matrix[k] * rho *g;
          }
      } /* Ende der Schleife ueber die Gauss-Punkte */
  for (i=0;i<16;i++) {
     capacitance_matrix[i] *= (element_thickness * porosity);
     conductance_matrix[i] *= (element_thickness);
     gravity_matrix[i] *= (element_thickness);
  }
  GMSetElementCapacitanceMatrix(index,capacitance_matrix,m_pcs->pcs_number);
  GMSetElementConductanceMatrix(index,conductance_matrix,m_pcs->pcs_number);
  /* Schwerkraftterm berechnen (gravity vector) */
  element_nodes = ElGetElementNodes(index);
  elevation[0] = ((GetNode(element_nodes[0]))->z);
  elevation[1] = ((GetNode(element_nodes[1]))->z);
  elevation[2] = ((GetNode(element_nodes[2]))->z);
  elevation[3] = ((GetNode(element_nodes[3]))->z);
  element_nodes = NULL;
    /* gravity matrix * elevation */
  MMultMatVec(gravity_matrix,4,4,elevation,4,gravity_vector,4);
  GMSetElementGravityVector(index,gravity_vector,m_pcs->pcs_number);
#ifdef TESTCEL_AGM
  MZeigVec(gravity_vector,4,"CalcEle2D_AGM: Gravity vector");
  MZeigMat(capacitance_matrix,4,4,"CalcEle2D_AGM: Capacitance matrix ");
  MZeigMat(conductance_matrix,4,4,"CalcEle2D_AGM: Conductance matrix");
#endif

  /*k_ij = Free (k_ij);*/
  /*k_rs = Free (k_rs);*/
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
10/2004 OK Implementation
last modified:
**************************************************************************/
void *GMCreateELEMatricesPointer(void)
{ 
  GMElementMatrices *data = NULL;
  data = (GMElementMatrices *) Malloc(sizeof(GMElementMatrices));
  data->capacitancematrix = NULL;
  data->conductancematrix = NULL;
  data->gravityvector = NULL;
  return (void *) data;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
10/2004 OK Implementation
last modified:
**************************************************************************/
void *GMDestroyELEMatricesPointer(void *data)
{
//OKnew
 if(data) {
  if(((GMElementMatrices *) data)->capacitancematrix)
     ((GMElementMatrices *) data)->capacitancematrix = \
       (double *) Free(((GMElementMatrices *) data)->capacitancematrix);
  if(((GMElementMatrices *) data)->conductancematrix)
     ((GMElementMatrices *) data)->conductancematrix = \
       (double *) Free(((GMElementMatrices *) data)->conductancematrix);
  if(((GMElementMatrices *) data)->gravityvector)
     ((GMElementMatrices *) data)->gravityvector = \
       (double *) Free(((GMElementMatrices *) data)->gravityvector);
//OKnew
  data = (void*) Free(data);
 }
  return data;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
10/2004 OK Implementation
last modified:
**************************************************************************/
void GMCalcElementMatrix(long index,CRFProcess *m_pcs)
{
  switch (ElGetElementType(index)) {
    case 1:
      GMCalcElementMatrix1D(index,m_pcs);
      break;
    case 2:
      GMCalcElementMatrix2D(index,m_pcs);
      break;
  }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
10/2004 OK Implementation
last modified:
**************************************************************************/
void GMCalcElementMatrices(CRFProcess *m_pcs)
{
#ifdef PARALLEL
  int j;
  CPARDomain *m_dom = NULL;
  int no_domains =(int)dom_vector.size();
  for(j=0;j<no_domains;j++){
    m_dom = dom_vector[j];
    m_dom->CalcElementMatrices(m_pcs);
  }
#else
  long i;
  Element* element = NULL;
  for (i=0;i<ElListSize();i++){
    element = ElGetElement(i);
    if(element&&element->aktiv)
      GMCalcElementMatrix(i,m_pcs);
  }
#endif
}

/*------------------------------------------------------------------------*/
/* ElementCapacitanceMatrix */
void GMSetElementCapacitanceMatrix(long number,double *matrix,int pcs_number)
{
  static GMElementMatrices *data;
  data = (GMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->capacitancematrix =  (double *)Free(data->capacitancematrix);
  data->capacitancematrix = matrix;
}

double *GMGetElementCapacitanceMatrix(long number,int pcs_number)
{
  return ((GMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->capacitancematrix;
}

/*------------------------------------------------------------------------*/
/* ElementConductanceMatrix */
void GMSetElementConductanceMatrix(long number,double *matrix,int pcs_number)
{
  static GMElementMatrices *data;
  data = (GMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->conductancematrix =  (double *)Free(data->conductancematrix);
  data->conductancematrix = matrix;
}

double *GMGetElementConductanceMatrix(long number,int pcs_number)
{
  return ((GMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->conductancematrix;
}

/*------------------------------------------------------------------------*/
/* ElementGravityVector */
void GMSetElementGravityVector(long number,double *matrix,int pcs_number)
{
  static GMElementMatrices *data;
  data = (GMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  //WW, will be removed if class is involved
  data->gravityvector =  (double *)Free(data->gravityvector);
  data->gravityvector = matrix;
}

double *GMGetElementGravityVector(long number,int pcs_number)
{
  return ((GMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->gravityvector;
}


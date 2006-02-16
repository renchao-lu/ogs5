#include "stdafx.h" // MFC

#include <iostream>

#include "rf_vel_new.h"
#include "elements.h"
#include "nodes.h"
#include "rf_mmp_new.h"
#include "rf_mfp_new.h"
#include "femlib.h"
#include "mathlib.h"
//#include "msh_nodes_rfi.h"

extern double gravity_constant;

/**************************************************************************
ROCKFLOW - Funktion: VELCalcAll
Aufgabe:
Programmaenderungen:
3/2005   based on VELCalculateAllGaussVelocities /GeoSys Version 4105
         Contributions from RK, OK, CT 
         MB
**************************************************************************/

void VELCalcAll(CRFProcess*m_pcs)
{
long i;
int phase;

for (i = 0; i < ElListSize(); i++)  {
  if (ElGetElement(i) != NULL)  {       /* Element existiert */
    if (ElGetElementActiveState(i)) {
      for (phase=0;phase<GetRFProcessNumPhases();phase++)  {
		//CECalcGaussVelo_NEW 
        VELCalcGauss(phase,i,m_pcs);
		//VELCalcElementVelocity(i,theta,phase);
        VELCalcEle(phase, i, m_pcs);
      }
    }
  }
} /* end for */
}  


/**************************************************************************
ROCKFLOW - Funktion: VELCalcGauss
Aufgabe:
Programmaenderungen:
3/2005   based on CECalcGaussVelo_NEW /GeoSys Version 4105
         Contributions from RK, OK, CT 
         MB
**************************************************************************/
void VELCalcGauss(int phase, long index, CRFProcess*m_pcs)
{
int eletype;
eletype = ElGetElementType(index) - 1;

switch(eletype){
  case 0:
    //CalcVelo1D(phase, index, theta, m_pcs);
    VELCalcGaussLine(phase, index, m_pcs);
  break;
  case 1:
    //CalcVelo2D(phase, index, theta, m_pcs);
    VELCalcGaussQuad(phase, index, m_pcs);
  break;
  case 2:			
    //CalcVelo3D(phase, index, theta, m_pcs);
    VELCalcGaussHex(phase, index, m_pcs);
  break;
  case 3:		
    //CalcVelo2DTriangle(phase, index, theta, m_pcs);
    VELCalcGaussTri(phase, index, m_pcs);  // dummy
  break;
  case 4: 
  //CalcVelo3DTetrahedra(phase, index, theta, m_pcs);
    VELCalcGaussTet(phase, index, m_pcs);  // dummy
  break;
  case 5:   
    //CalcVelo3DPrismRST_NEW(phase, index, theta, 0.0, 0.0, 0.0, m_pcs);
    VELCalcGaussPris(phase, index, m_pcs);  // dummy
  break;
  default:
  DisplayMsgLn(" Error - unknown element type in CECalcGaussVelo_NEW");
};
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussLine
Aufgabe:
Programmaenderungen:
3/2005   based on CalcVelo1D /GeoSys Version 4105
         Contributions from RK, OK, CT 
         MB
*************************************************************************/
void VELCalcGaussLine(int phase, long index, CRFProcess*m_pcs)
{
  double *velovec;
  double gp[3];
  double velo[1];

  /* Speicher holen */
  velovec = ElGetVelocityNew(index, phase);
  // nur ein Gausspunkt ?? MB
  gp[0]= 0.0;
  gp[1]= 0.0;
  gp[2]= 0.0;
  VELCalcGaussLineLocal(phase, index, gp, m_pcs, velo);
  
  velovec[0] = velo[0];

  return;
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussQuad
Aufgabe: 
Programmaenderungen:
03/2005  based on CalcVelo2D /GeoSys Version 4105
         Contributions from RK, OK, CT 
         MB
*************************************************************************/
void VELCalcGaussQuad(int phase, long index, CRFProcess*m_pcs)
{
  int i, j;
  long ind;
  double r, s;
  long *nodes;
  int anzgp;
  double *velovec;
  double gp[3]; 
  double velo[2];

  //----------------------------------------------------------------------
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  velovec = ElGetVelocityNew(index, phase);
  nodes = ElGetElementNodes(index);

  //----------------------------------------------------------------------
  /* An irregulaeren Knoten wird die Geschw. auf Basis des Vorgaengers
     in Elementmitte eingetragen  -> Dichtestroemungen!!! */
  //if (VELCorrectVelocityOnIrrNodesElementsNew == 1)  {
  //  status = CriticalElements(index, &vorgaenger);
  //  if (status) {
  //    //CalcVelo2Drs(phase, vorgaenger, theta, 0., 0., 0., v);
  //    VELCalcGaussQuadLocal(phase, vorgaenger, gp, m_pcs, v);
  //    /* Es muss der Uebergang auf globalen Koordinaten ermittelt werden ! */  
  //    Calc2DElement_ab2xyz_Vector(vorgaenger, v, v_xyz);
  //    Calc2DElement_xyz2ab_Vector(index, v_xyz, v);

  //    ind = -1;
  //    for (i = 0; i < anzgp; i++) {
  //      for (j = 0; j < anzgp; j++)  {
  //        velovec[++ind] = v[0];
  //        velovec[++ind] = v[1];
  //      }
  //    }
  //    return;
  //  }
  //}

  //----------------------------------------------------------------------
  ind = -1;
  for (i = 0; i < anzgp; i++)  {
    for (j = 0; j < anzgp; j++)  {
      
      r = MXPGaussPkt(anzgp, i);
      s = MXPGaussPkt(anzgp, j);
 
      gp[0]=r; gp[1]=s; gp[2]=0.0;

      VELCalcGaussQuadLocal(phase, index, gp, m_pcs, velo);

      velovec[++ind] = velo[0];
      velovec[++ind] = velo[1];
    }
  }
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussHex
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo3D /GeoSys Version 4105
         Contributions from RK, OK, CT, CMCD
         MB
*************************************************************************/
void VELCalcGaussHex(int phase, long index,CRFProcess*m_pcs)
{
  int i, j, k, anzgp;
  int inde;
  double r, s, t;
  double *velovec;
  double gp[3]; 
  double velo[3];

  /* Numerics */ 
  anzgp = m_pcs->m_num->ele_gauss_points;
 
  /* Speicher holen */
  velovec = ElGetVelocityNew(index, phase);

  /* An irregulaeren Knoten wird die Geschw. auf Basis des Vorgaengers
     in Elementmitte eingetragen  -> Dichtestroemungen!!! */

  //if (VELCorrectVelocityOnIrrNodesElements == 1)
  //  {
  //    status = CriticalElements(index, &vorgaenger);
  //    if (status)
  //      {
  //        index = vorgaenger;
  //        CalcVelo3Drst(phase, index, theta, 0., 0., 0., v);
  //        ind = -1;
  //        for (i = 0; i < anzgp; i++)
  //          {
  //            for (j = 0; j < anzgp; j++)
  //              {
  //                for (k = 0; k < anzgp; k++)
  //                  {
  //                    velovec[++ind] = v[0];
  //                    velovec[++ind] = v[1];
  //                    velovec[++ind] = v[2];
  //                  }
  //              }
  //          }
  //        return;
  //      }
  //  }
  
  /* r,s,t-Koordinatensystem */
  inde = -1;

  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)  {

        r = MXPGaussPkt(anzgp, i);
        s = MXPGaussPkt(anzgp, j);
        t = MXPGaussPkt(anzgp, k);

        gp[0]=r; gp[1]=s; gp[2]=t;

        VELCalcGaussHexLocal(phase, index, gp, m_pcs, velo);

        velovec[++inde] = velo[0];
        velovec[++inde] = velo[1];
        velovec[++inde] = velo[2];
      }
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussTri
Aufgabe: 
Programmaenderungen:
03/2005  based on VELCalcGaussQuad
         MB
*************************************************************************/
void VELCalcGaussTri(int phase, long index, CRFProcess*m_pcs)
{
  double* velo=NULL;

  // Speicher holen
  velo = ElGetVelocityNew(index, phase);
  
  //später Schleife über Gausspunkte und 
  // VELCalcGaussTriLocal(.............)
  VELCalcTriGlobal(phase, index, m_pcs, velo);
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussTet
Aufgabe: 
Programmaenderungen:
03/2005  based on VELCalcGaussQuad
         MB
*************************************************************************/
void VELCalcGaussTet(int phase, long index, CRFProcess*m_pcs)
{
  double* velo=NULL;

  // Speicher holen
  velo = ElGetVelocityNew(index, phase);
  
  //später Schleife über Gausspunkte und 
  // VELCalcGaussTetLocal(.............)
  VELCalcTetGlobal(phase, index, m_pcs, velo);
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussPris
Aufgabe: 
Programmaenderungen:
03/2005  MB based on VELCalcGaussHex 
*************************************************************************/
void VELCalcGaussPris(int phase, long index, CRFProcess*m_pcs)
{
  int i, j;
  long ind;
  double r, s, t;
  long *nodes;
  double *velovec;
  double gp[3]; 
  double velo[3];
 
  int anzgptri, anzgplin;

  //----------------------------------------------------------------------
  velovec = ElGetVelocityNew(index, phase);
  nodes = ElGetElementNodes(index);

  anzgptri = 3;  /* Anzahl Gausspunkte; lineares Dreieck */
  anzgplin = 2;  /* Anzahl Gausspunkte; Linearkomponente */

  ind = -1;
  for (i = 0; i < anzgplin; i++)  {
    for (j = 0; j < anzgptri; j++)  {
        
      r = MXPGaussPktTri(anzgptri,j,0);
      s = MXPGaussPktTri(anzgptri,j,1);
      t = MXPGaussPkt(anzgplin,i);

      gp[0]=r; gp[1]=s; gp[2]= t;

      VELCalcGaussPrisLocal(phase, index, gp, m_pcs, velo);

      velovec[++ind] = velo[0];
      velovec[++ind] = velo[1];
      velovec[++ind] = velo[2];   
    }
  }
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussLineLocal
Aufgabe:
Programmaenderungen:
03/2005   based on CalcVelo1D /GeoSys Version 4105
          Contributions from RK, OK, CT 
          MB
*************************************************************************/
void VELCalcGaussLineLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *v)
{
  /* Elementgeometriedaten */
  static double invjac[3], detjac;
  /* Knotendaten */
  static int nn = 2, i;
  static long *nodes;           /* Knoten des Elements */
  static double p[2], delta_p;  /* Knotendruecke */
  static double delta_x, delta_y, delta_z, L2;
  static Knoten *k0;
  static Knoten *k1;
  static double vorfk;
  /* Materialdaten */
  static double g, rho;
  static double mu;
  static double k_rel;
  
  static double theta;
  int nidx0, nidx1;

  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;

  /* Medium properties */ 
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double* permeability;
  permeability = m_mmp->PermeabilityTensor(index);
  double permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
  k_rel = 1.;

  /* Fluid materials */
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    nidx1 = PCSGetNODValueIndex(pcs_primary,1);
  }  
  else{
    nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
    nidx1 = PCSGetNODValueIndex("PRESSURE1",1);
  }

  /* Elementdaten bereitstellen */
  nodes = ElGetElementNodes(index);
  nn = ElNumberOfNodes[0];             /* Knotenanzahl nn muss 2 sein ! */

  /* Inverse Jakobi-Matrix berechnen - kann entfallen */
  Calc1DElementJacobiMatrix(index, invjac, &detjac);
  /* invjac = GetElementJacobiMatrix(index,&detjac); */

  /* Elementlaenge L berechnen */
  k0 = GetNode(nodes[0]);
  k1 = GetNode(nodes[1]);
  delta_x = k1 -> x - k0 -> x;
  delta_y = k1 -> y - k0 -> y;
  delta_z = k1 -> z - k0 -> z;
  L2 = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;

  /* Druckdifferenz delta_p berechnen */
  for (i = 0; i < nn; i++)  {
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
  }

  delta_p = p[1] - p[0];

  /* Permeabilitaet fuer nichtlineares Fliessgesetz */
/*OK_MMP
  if (GetNonlinearFlowElement(index))
    {
   //grad_h[0] = 1. / sqrt(L2) * (delta_p / (rho * g) + delta_z);
   //k_rel = CalculateMeanNonlinearFlowPermeability(index, grad_h);
      k_rel = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, GetElementJacobiMatrix);
    }
*/
  /* Modellgestuetzte relative Permeabilitaet */
//OK_MMP  if (VelocityGetRelativePermeability)
//OK_MMP    k_rel *= VelocityGetRelativePermeability(phase, index, 0., 0., 0., theta);
    k_rel *= permeability_rel;
/*OK ???
  if (((GetSoilRelPermPressModel(index) !=10) && (k_rel > 0.0)) || GetSoilPorosityModel(index) == 0)  // MX 022004
      k_rel = MMax(MKleinsteZahl, k_rel);
*/
//OK_MMP  vorfk = -k_ij[0] * k_rel / mu;
  vorfk = -permeability[0] * k_rel / mu;

  v[0] = vorfk * (0.5 * delta_p * invjac[0] + rho * g * delta_z * delta_x / L2);
  v[1] = vorfk * (0.5 * delta_p * invjac[1] + rho * g * delta_z * delta_y / L2);
  v[2] = vorfk * (0.5 * delta_p * invjac[2] + rho * g * delta_z * delta_z / L2);

}



/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussQuadLocal
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo2Drs /GeoSys Version 4105
         Contributions from RK, OK, CT 
         MB
*************************************************************************/
void VELCalcGaussQuadLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velo)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static int i, nn = 4;
  /* Elementgeometriedaten */
  static double detjac, invjac[4];
  static double grad_omega[8];
  /* Knotendaten */
  static long *nodes;
  static double p[4], z[4];
  /* Elementdaten */
  static double v[2], w[2];
  static double v1[2], v2[2];
  /* Materialdaten */
  static int nonlinear_flow_element;
  static double k_rel_grad_p = 1.;
  static double k_rel_S = 1.;
  static double mu, rho, g;
  static double kf[4];
  static double *k_ij;
  static int nidx0, nidx1;
  static double theta;

  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;

  /* Knotendaten */
  nodes = ElGetElementNodes(index);

  /* Materialdaten */
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  m_mmp->m_pcs = m_pcs; //OK
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  nonlinear_flow_element = 0;
  k_rel_S = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);

  // MFP fluid properties
  g = gravity_constant;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    nidx1 = PCSGetNODValueIndex(pcs_primary,1);  
  }  
  else {
    //--------------------------------------------------------------------
    // Phase pressure
    string pressure_phase = "PRESSURE";
    char phase_char[1];
    sprintf(phase_char,"%i",phase+1);
    pressure_phase.append(phase_char);
    nidx0 = PCSGetNODValueIndex(pressure_phase,0);
    nidx1 = PCSGetNODValueIndex(pressure_phase,1);  
  }
  for(i=0;i<nn;i++){
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
    z[i] = GetNode(nodes[i]) -> z;
  }
/****************************************************************************/
/* Permeabilitaetstensor berechnen
 ************************************************************************** */
  /* Nichtlineares Fliessgesetz */
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = m_mmp->NonlinearFlowFunction(index,gp,theta);
/*
   if (nonlinear_flow_element == 1) {
   for (i = 0; i < 4; i++)
   h[i] = p[i] / (g * rho) + z[i];
   k_rel = CalculateMeanNonlinearFlowPermeability(index, grad_h);
   }
*/
  /* Spezifischer Permeabilitaetstensor */
  for (i = 0; i < 4; i++)
    kf[i] = -k_ij[i] * k_rel_grad_p / mu;
/****************************************************************************/
/* Filtergeschwindigkeiten im lokalen Element-Koordinatensystem berechnen
 ************************************************************************** */
  /* Jakobi-Matrix und ihre Inverse berechnen */
  Calc2DElementJacobiMatrix(index, gp[0], gp[1], invjac, &detjac);
  /* Gradientenmatrix */
  MGradOmega2D(grad_omega, gp[0], gp[1]);

  /* Druckformulierung */
  /* k/my * J^-1 * grad(omega) * p - Druckkraft */
  MMultMatVec(grad_omega, 2, 4, p, 4, v, 2);
  MMultVecMat(v, 2, invjac, 2, 2, w, 2);
  MMultMatVec(kf, 2, 2, w, 2, v1, 2);

  /* k/my * J^-1 * grad(omega) * z - Schwerkraft */
  MMultMatVec(grad_omega, 2, 4, z, 4, v, 2);
  MMultVecMat(v, 2, invjac, 2, 2, w, 2);
  MMultMatVec(kf, 2, 2, w, 2, v2, 2);

  velo[0] = (v1[0] + rho * g * v2[0]) * k_rel_S;
  velo[1] = (v1[1] + rho * g * v2[1]) * k_rel_S;
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussHexLocal
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo3Drst /GeoSys Version 4105
         Contributions from RK, OK, CT, CMCD
         MB
*************************************************************************/
void VELCalcGaussHexLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *v)
{
  static int i, anzgp, nn = 8;
  static double detjac;
  static double invjac[9];
  static double p[8];
  static double mu, rho, g, k_rel;
  static double grad[24];
  static double w[3];
  static long *nodes;
  static double *k_ij;
  static int nidx0, nidx1; 
  static double theta;
  
  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;
  anzgp = m_pcs->m_num->ele_gauss_points;

  nodes = ElGetElementNodes(index);
 
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_rel = 1.;
  double permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
  
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    nidx1 = PCSGetNODValueIndex(pcs_primary,1);  
  }  
 
  //--------------------------------------------------------------------
  // Phase pressure
  else  {
  string pressure_phase = "PRESSURE";
  char phase_char[1];
  sprintf(phase_char,"%i",phase+1);
  pressure_phase.append(phase_char);

  nidx0 = PCSGetNODValueIndex(pressure_phase,0);
  nidx1 = PCSGetNODValueIndex(pressure_phase,1);  
  }

  for(i=0;i<nn;i++){
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
  }

  /* r,s,t-Koordinatensystem */
  /* inverse Jakobi-Matrix */
  Calc3DElementJacobiMatrix(index, gp[0], gp[1], gp[2], invjac, &detjac);
  /* kf * J^-1 * grad(omega) * p */
  MGradOmega3D(grad, gp[0], gp[1], gp[2]);         /* Gradientenmatrix */
  MMultMatVec(grad, 3, 8, p, 8, v, 3);
  MMultMatVec(invjac, 3, 3, v, 3, w, 3);
  w[2] += rho * g;

  k_rel = permeability_rel;  /*MX*/

/*OK ???
  if (((GetSoilRelPermPressModel(index) !=10) && (k_rel > 0.0)) || GetSoilPorosityModel(index) == 0)  // MX 022004
     k_rel = MMax(MKleinsteZahl, k_rel);
*/

  MMultMatVec(k_ij, 3, 3, w, 3, v, 3);

  v[0] *= -k_rel / mu;
  v[1] *= -k_rel / mu;
  v[2] *= -k_rel / mu;

  return;
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcTriGlobal
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo2DTriangle /GeoSys Version 4105
         Contributions from OK, CMCD
         MB
*************************************************************************/
void VELCalcTriGlobal(int phase, long index, CRFProcess*m_pcs, double *velovec)
{
  int i;
  int p_idx1;
  double x[3], y[3]; //CMCD local coordinates
  double xg[3], yg[3], zg[3]; //CMCD global coordinates
  double dN_dx[3],dN_dy[3]; //CMCD shape functions local coordinates
  double area; //CMCD
  long *nodes;
  double nodepressure[3]; //p[3];
  int nn = 3;
  double p1[2], p2[2];
  double dircos[6];
  int no_phases;
  int nonlinear_flow_element;
  double k_rel_grad_p = 1.;
  double k_rel_S = 1.;
  double mu, rho, g, k_rel; //k_rel added CMCD
  double k_x,k_y,k_z;//CMCD
  double px,py,pz;//CMCD
  double gp[3]; //CMCD
  long group;
  double theta;

  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;
  
  //--------------------------------------------------------------------
  velovec = ElGetVelocityNew(index, phase);
  //The triangles do not need to be transformed into local co-ordintes for the calculation of the fluid parameters.
  //The value of the gp for the density funtion and viscosity function is given as the center point of the triangle.
  //--------------------------------------------------------------------
  CalculateSimpleMiddelPointElement(index,gp); 
  nodes = ElGetElementNodes(index);
  for(i=0;i<nn;i++){
    xg[i] = GetNodeX(nodes[i]);
    yg[i] = GetNodeY(nodes[i]);
    zg[i] = GetNodeZ(nodes[i]);
  }
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_x = m_mmp->permeability_tensor[0];  
  k_y = m_mmp->permeability_tensor[1];
  k_z = m_mmp->permeability_tensor[1];//CMCD To Do, allow real 3D traingular permeability read function.
  k_rel = m_mmp->PermeabilityPressureFunction(index, gp,theta); 
  nonlinear_flow_element = 0;
  //--------------------------------------------------------------------
  // MFP fluid properties
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  g = gravity_constant;
  no_phases =(int)mfp_vector.size();
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    p_idx1 = PCSGetNODValueIndex("pcs_primary",1);
  }  
  else {
    p_idx1 = PCSGetNODValueIndex("PRESSURE1",1); 
  }
  
  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    nodepressure[i] = GetNodeVal(nodes[i],p_idx1);
  }	

  //--------------------------------------------------------------------
/*OK ReDo
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
  if (VelocityGetRelativePermeability)
    k_rel_S = VelocityGetRelativePermeability(phase, index, gp[0], gp[1], gp[2], theta);
*/
  //--------------------------------------------------------------------
  //Now we need to be careful about local and global co-ordinates.
  //Strictly a triangle is only 2D, but here we have x,y,z co-ordinates.
  //We need to convert the coordinates into the RS systems of the triangle to
  //define the shape functions in this plane.
  Calc2DElementCoordinatesTriangle(index,x,y,dircos); /*CMCD included 03/2004*/
  area = ElGetElementVolume(index)/m_mmp->geo_area;
  //Shape function derivatives for local co-ordinates
  dN_dx[0] = (y[1] - y[2]) / (2. * area);
  dN_dx[1] = (y[2] - y[0]) / (2. * area);
  dN_dx[2] = (y[0] - y[1]) / (2. * area);
  dN_dy[0] = (x[2] - x[1]) / (2. * area);
  dN_dy[1] = (x[0] - x[2]) / (2. * area);
  dN_dy[2] = (x[1] - x[0]) / (2. * area);
  //Pressure gradients in local coordintes
  p1[0] = MSkalarprodukt(dN_dx, nodepressure, nn);
  p1[1] = MSkalarprodukt(dN_dy, nodepressure, nn);
  p2[0] = MSkalarprodukt(dN_dx, zg, nn);
  p2[1] = MSkalarprodukt(dN_dy, zg, nn);
  //Transform the pressure gradients back into global co-ordinates
  //Sum the axis contributions from p1 and p2 to px, py, pz
  px=p1[0]*dircos[0]+(p2[0]*dircos[0]*rho*g)+p1[1]*dircos[1]+(p2[1]*dircos[1]*rho*g);
  py=p1[0]*dircos[2]+(p2[0]*dircos[2]*rho*g)+p1[1]*dircos[3]+(p2[1]*dircos[3]*rho*g);
  pz=p1[0]*dircos[4]+(p2[0]*dircos[4]*rho*g)+p1[1]*dircos[5]+(p2[1]*dircos[5]*rho*g);
  //Convert the pressure differences into velocities in the x,y,z directions.
  //Take into account relative permeabilities.
//OK ??? -/+
   velovec[0] = (-k_x * k_rel_grad_p * k_rel_S*k_rel / mu) * px;
   velovec[1] = (-k_y * k_rel_grad_p * k_rel_S*k_rel / mu) * py;
   velovec[2] = (-k_z * k_rel_grad_p * k_rel_S*k_rel / mu) * pz;
/*  Free(velovec);*/
}


/*************************************************************************
ROCKFLOW - Funktion: VELCalcGaussPrisLocal
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo3DPrismRST /GeoSys Version 4105
         Contributions from OK, CMCD
         MB
*************************************************************************/
void VELCalcGaussPrisLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velo)
{
  static int i;
  static long *nodes=NULL;
  static double p[6], z[6];
  static int nn = 6;
  static int nonlinear_flow_element;
  static double k_rel_grad_p = 1.;
  static double k_rel_S = 1.;
  static double mu, rho, g;
  static double k_x, k_y, k_z;
  double grad_p[3];
  double zwi[3];
  static double GradOmPrism[18];
  static double invjac[9], detjac;
  static double theta;
  static int nidx0, nidx1;

  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;
  nodes = ElGetElementNodes(index);

  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double* k_ij;
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_x = k_ij[0];
  k_y = k_ij[4];
  k_z = k_ij[9];
  nonlinear_flow_element = 0;
  //--------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    nidx1 = PCSGetNODValueIndex(pcs_primary,1);  
  } 
  else{
    nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
    nidx1 = PCSGetNODValueIndex("PRESSURE1",1);  
  }
  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
    z[i] = GetNode(nodes[i]) -> z;
  }
  //--------------------------------------------------------------------
/*ReDo
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
  if (VelocityGetRelativePermeability)
    k_rel_S = VelocityGetRelativePermeability(phase, index, 0, 0, 0., theta);
*/
  /* MGradOmegaPrism */
  MGradOmegaPrism( gp[0], gp[1], gp[2], GradOmPrism);               /* 3 Zeilen 6 Spalten */
  /* Inverse Jakobian Matrix */
  CalcPrismElementJacobiMatrix(index, gp[0], gp[1], gp[2], invjac, &detjac);
  /*  kf * InvJac * GradOmPrism * P  */
  MMultMatVec(GradOmPrism, 3, 6, p, 6, zwi, 3);
  MMultMatVec(invjac, 3, 3, zwi, 3, grad_p, 3);
  grad_p[2] = grad_p[2] + (rho * g);
  velo[0] = -k_x * k_rel_grad_p * k_rel_S / mu * grad_p[0];
  velo[1] = -k_y * k_rel_grad_p * k_rel_S / mu * grad_p[1];
  velo[2] = -k_z * k_rel_grad_p * k_rel_S / mu * grad_p[2];

}

/*************************************************************************
ROCKFLOW - Funktion: CalcVelo3DPrismXYZ
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo3DPrismXYZ /GeoSys Version 4105
         Contributions from OK, CMCD
         MB
*************************************************************************/
void VELCalcPrisGlobal(int phase, long index, CRFProcess*m_pcs, double *velo)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  int i;
  /* Elementdaten */
  int nn = 6;
  double area;
  double t;
  /* Knotendaten */
  long *nodes;
  double p[6], z[6];
  /* Materialdaten */
  int nonlinear_flow_element;
  double k_rel_grad_p = 1.;
  double k_rel_S = 1.;
  double mu, rho, g;
  double k_x, k_y, k_z;
  double* OmegaTri=NULL;
  double xx, yy;
  double grad_x_Nt[2];
  double Nt[2];
  double InvJac3Dz;
  double gradNpris[18];
  double grad_p[3];
  double* coord = NULL;
  double gp[3];
  double GradXTri[3];
  double GradYTri[3];
  double theta;
  int nidx0, nidx1;
  
  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;
 
  /* Knotendaten */
  nodes = ElGetElementNodes(index);

  /* Soil properties */
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double* k_ij;
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_x = k_ij[0];
  k_y = k_ij[4];
  k_z = k_ij[8];
  //OK_MMP nonlinear_flow_element = GetNonlinearFlowElement(index);
  nonlinear_flow_element = 0;
  //--------------------------------------------------------------------
  // MFP fluid properties
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    nidx1 = PCSGetNODValueIndex(pcs_primary,1);  
  }  
  else{
    nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
    nidx1 = PCSGetNODValueIndex("PRESSURE1",1);  
  }

  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
    z[i] = GetNode(nodes[i]) -> z;
  }
  //--------------------------------------------------------------------
/*OK ReDo
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
  if (VelocityGetRelativePermeability)
    k_rel_S = VelocityGetRelativePermeability(phase, index, 0, 0, 0., theta);
*/
  /****************************************************************************/
  /* Filtergeschwindigkeiten im Elementmittelpunkt berechnen
  ************************************************************************** */
  /*------------------------------------------------------------------------*/
  /*---- Omega3DPris -------------------------------------------------------*/
  /*------------------------------------------------------------------------*/
   /* Coordinates */
//TODO MARTIN    coord = MSHGetGravityCenter(index);
//  xx = coord[0];
//  yy = coord[1];

//SB
  xx = (GetNodeX(nodes[0]) + GetNodeX(nodes[1]) + GetNodeX(nodes[2]))/3 ;
  yy = (GetNodeY(nodes[0]) + GetNodeY(nodes[1]) + GetNodeY(nodes[2]))/3 ;
//SB

  t = 0;

  InvJac3Dz   = GetInvJac3Dz(index);
  area = CalcBaseAreaPrism(index); 

  /* Triangle Component */
  //GradXTri = CalcGradXTri(index);
  CalcGradXTri(index, GradXTri);
  //GradYTri = CalcGradYTri(index);
  CalcGradYTri(index, GradYTri);
  OmegaTri = GetOmegaTri(index, xx, yy);

  /* Linear Component */
  Nt[0] = (1 + t) / 2.0;
  Nt[1] = (1 - t) / 2.0;
  grad_x_Nt[0] = + 1 / 2.0;
  grad_x_Nt[1] = - 1 / 2.0;

  gradNpris[0] = GradXTri[0] * Nt[0];
  gradNpris[1] = GradXTri[1] * Nt[0];
  gradNpris[2] = GradXTri[2] * Nt[0];
  gradNpris[3] = GradXTri[0] * Nt[1];
  gradNpris[4] = GradXTri[1] * Nt[1];
  gradNpris[5] = GradXTri[2] * Nt[1];
 
  gradNpris[6] = GradYTri[0] * Nt[0];
  gradNpris[7] = GradYTri[1] * Nt[0];
  gradNpris[8] = GradYTri[2] * Nt[0];
  gradNpris[9] = GradYTri[0] * Nt[1];
  gradNpris[10] = GradYTri[1] * Nt[1];
  gradNpris[11] = GradYTri[2] * Nt[1];

  gradNpris[12] = OmegaTri[0] * grad_x_Nt[0] * InvJac3Dz;
  gradNpris[13] = OmegaTri[1] * grad_x_Nt[0] * InvJac3Dz;
  gradNpris[14] = OmegaTri[2] * grad_x_Nt[0] * InvJac3Dz;
  gradNpris[15] = OmegaTri[0] * grad_x_Nt[1] * InvJac3Dz;
  gradNpris[16] = OmegaTri[1] * grad_x_Nt[1] * InvJac3Dz;
  gradNpris[17] = OmegaTri[2] * grad_x_Nt[1] * InvJac3Dz;

  MMultMatMat(gradNpris, 3, 6, p, 6, 1, grad_p, 3, 1);

  grad_p[2] = grad_p[2] + (rho * g);
  velo[0] = k_x * k_rel_grad_p * k_rel_S / mu * grad_p[0];
  velo[1] = k_y * k_rel_grad_p * k_rel_S / mu * grad_p[1];
  velo[2] = k_z * k_rel_grad_p * k_rel_S / mu * grad_p[2];
  /* Speicherfreigabe */
  OmegaTri = (double*) Free(OmegaTri);
  coord = (double*) Free(coord);
}

/*************************************************************************
ROCKFLOW - Funktion: VELCalcTetGlobal
Aufgabe:
Programmaenderungen:
03/2005  based on CalcVelo3DTetrahedra /GeoSys Version 4105
         Contributions from OK, CMCD
         MB
*************************************************************************/
void VELCalcTetGlobal(int phase, long index, CRFProcess*m_pcs, double *velovec)
{
  int i;
  int p_idx0, p_idx1;
  double gp[3];
  double b1,b2,b3,b4;
  double c1,c2,c3,c4;
  double d1,d2,d3,d4;
  double dNdx[4],dNdy[4],dNdz[4];
  double mat3x3[16];
  double x[4], y[4], z[4]; 
  double volume; 
  long *nodes;
  double nodepressure[4];
  int nn = 4;
  double v1[3], v2[3];
  int nonlinear_flow_element=0;
  double k_rel_grad_p = 1.;
  double k_rel_S = 1.;
  double mu, rho, g, k_rel; 
  double k_x, k_y, k_z;
  double theta;

  /* Numerics */ 
  theta = m_pcs->m_num->ls_theta;
   
  //----------------------------------------------------------------------
  velovec = ElGetVelocityNew(index,phase);
  //----------------------------------------------------------------------
  nodes = ElGetElementNodes(index);
  
  for(i=0;i<nn;i++){
    x[i] = GetNodeX(nodes[i]);
    y[i] = GetNodeY(nodes[i]);
    z[i] = GetNodeZ(nodes[i]);
  }
  //The tetrahedra do not need to be transformed into local co-ordintes. Therefore
  //the value of the gp for the density funtion is given as the center point of the tetrahedra
  CalculateSimpleMiddelPointElement(index,gp);
  //----------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  if (m_mmp->flowlinearity_model != 0 )nonlinear_flow_element = 1;
  double* permeability;
  permeability = m_mmp->PermeabilityTensor(index);
  k_x = permeability[0];
  k_y = permeability[4];
  k_z = permeability[8];
  k_rel = m_mmp->PermeabilityPressureFunction(index, gp,theta); 
  //----------------------------------------------------------------------
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();
    //Element Data
	CalcElementVolume(index, &volume);
    /* Knotendruecke und geod. Hoehen bereitstellen */
  //----------------------------------------------------------------------
  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
    p_idx0 = PCSGetNODValueIndex(pcs_primary,0);
    p_idx1 = PCSGetNODValueIndex(pcs_primary,1);
  }  
  else{
    p_idx0 = PCSGetNODValueIndex("PRESSURE1",0);
    p_idx1 = PCSGetNODValueIndex("PRESSURE1",1);
  }


  for(i=0;i<nn;i++){
    nodepressure[i] = GetNodeVal(nodes[i],p_idx1);
  }
  //----------------------------------------------------------------------
/*OK ReDo
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
  if (VelocityGetRelativePermeability)
    k_rel_S = VelocityGetRelativePermeability(phase, index, 0, 0, 0., theta);
*/
  //----------------------------------------------------------------------
    /* Shape Functions */
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

    //Actual element shape function derivates
    dNdx[0] = b1/(6*volume);
    dNdx[1] = b2/(6*volume);
    dNdx[2] = b3/(6*volume);
    dNdx[3] = b4/(6*volume);

    dNdy[0] = c1/(6*volume);
    dNdy[1] = c2/(6*volume);
    dNdy[2] = c3/(6*volume);
    dNdy[3] = c4/(6*volume);

    dNdz[0] = d1/(6*volume);
    dNdz[1] = d2/(6*volume);
    dNdz[2] = d3/(6*volume);
    dNdz[3] = d4/(6*volume);


    v1[0] = MSkalarprodukt(dNdx, nodepressure, nn);
    v1[1] = MSkalarprodukt(dNdy, nodepressure, nn);
    v1[2] = MSkalarprodukt(dNdz, nodepressure, nn);
    v2[0] = MSkalarprodukt(dNdx, z, nn);
    v2[1] = MSkalarprodukt(dNdy, z, nn);
    v2[2] = MSkalarprodukt(dNdz, z, nn);

    velovec[0] = (-k_x * k_rel_grad_p * k_rel_S*k_rel / mu)*  (v1[0] + (rho * g * v2[0]));
    velovec[1] = (-k_y * k_rel_grad_p * k_rel_S*k_rel / mu) * (v1[1] + (rho * g * v2[1]));
    velovec[2] = (-k_z * k_rel_grad_p * k_rel_S*k_rel / mu) * (v1[2] + (rho * g * v2[2]));

    //Free(velovec); OK

}


/**************************************************************************
ROCKFLOW - Funktion: VELCalcEle
Aufgabe:
Programmaenderungen:
03/2005  based on VELCalcElementVelocity /GeoSys Version 4105
         Contributions from OK, CMCD
         MB
**************************************************************************/
void VELCalcEle(int phase, long index, CRFProcess*m_pcs)
{
  static double v[3],v_ab[3];
  static double vf[12];       /* Vektorfeld */
  static double trans[6];     /* Drehmatrix */
  static double gp[3];
  double* v_gp = NULL;

  gp[0] = 0.0;
  gp[1] = 0.0;
  gp[2] = 0.0;
	
  switch(ElGetElementType(index)) {
    case 1:
    //CalcVelo1Dr(phase, index, theta, 0., 0., 0., v);
      VELCalcGaussLineLocal(phase, index, gp, m_pcs, v);
    break;
    case 2:
      //not needed MB ?
	  //CalcVeloXDrstxyz(phase,index, theta, 0.0, 0.0, 0.0, v_ab);

      //für adaptive Gitter wichtig ?
      //CalcVeloXDrst(phase,index,theta,0.,0.,0.,v_ab);
      VELCalcGaussQuadLocal(phase, index, gp, m_pcs, v_ab);
      CalcTransformationsMatrix(index, vf, trans);
      MMultMatVec(trans, 3, 2, v_ab, 2, v, 3);
    break;
    case 3:
	  //CalcVelo3Drst(0, index, theta, 0., 0., 0., v);
      VELCalcGaussHexLocal(phase, index, gp, m_pcs, v);
    break;
    case 4:
      v_gp = ElGetVelocityNew(index,phase);
      v[0]=v_gp[0];v[1]=v_gp[1];v[2]=v_gp[2];
    break;
    case 6:
   	  // CalcVelo3DPrismXYZ(0, index, theta, v);
      VELCalcPrisGlobal(phase, index, m_pcs, v);
    break;
  } /* switch */

  switch(phase){
    case 0:
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY1_X"),v[0]);
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY1_Y"),v[1]);
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY1_Z"),v[2]);
      break;
    case 1:
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY2_X"),v[0]);
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY2_Y"),v[1]);
      ElSetElementVal(index,PCSGetELEValueIndex("VELOCITY2_Z"),v[2]);
      break;
    default:
      cout << "Error in VELCalcElementVelocity: invalid phase number" << endl;
  }
/*OK
	if(VELSetElementVelocityX){  //SB:todo
    VELSetElementVelocityX(i,phase,v[0]);
    VELSetElementVelocityY(i,phase,v[1]);
    VELSetElementVelocityZ(i,phase,v[2]);
	}
*/
}


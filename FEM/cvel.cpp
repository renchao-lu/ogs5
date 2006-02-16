/**************************************************************************
   ROCKFLOW - Modul: cvel.c

   Aufgabe:
   Funktionen zur Berechnung der Geschwindigkeiten in den Gauss-Punkten

   Programmaenderungen:
   02/1997     R.Kaiser    Funktionen uebertragen aus dem aTM
   08.06.1997  C.Thorenz   Verallgemeinert (cvel_asm.c -> cvel.c)
   09.06.1997  R.Kaiser    Ueberarbeitet
   11.06.1997  C.Thorenz   Anpassung an RF-Konzept
   26.11.1997  O.Kolditz   nichtlineare Fliessgesetze
   22.03.1998  C.Thorenz   Einbauen des Dichteeinflusses
   05.08.1998  OK          Umstellung auf globale Daten (optional)
   20.02.1999  C.Thorenz   Verallgemeinerung
   12/1999     C.Thorenz   Dichteeinfluss in Elementmitte
   05/2000     OK          Kapselung nichtlineare Fliessgesetze
    9/2000     C.Thorenz   Konfigurierung von Integrationspunkten etc.
    9/2000     C.Thorenz   Speicher nur einmal holen -> schneller
   04/2002     OK          VELGetNodeTemperature
                           CalcVelo2DTriangle
   03/2003     RK          Quellcode bereinigt, Globalvariablen entfernt   
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
 **************************************************************************/
#include "stdafx.h"
// C++ STL
#include <iostream>
// FEMLib
#include "makros.h"
#include "cvel.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
#include "femlib.h"
#include "intrface.h"
#include "rf_mfp_new.h"
#include "gridadap.h"
#include "rf_pcs.h" //OK_MOD"
#include "cel_mtm2.h" // f�r MTM2TimeCollocationGlobal()
#include "rf_pcs.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
extern double gravity_constant;
// MSHLib
//WW #include "msh_nodes_rfi.h"

DoubleFuncLong VelocityGetNodePress0 = NULL;
DoubleFuncLong VelocityGetNodePress1 = NULL;
DoubleFuncILLD VelocityGetNodePress = NULL;
DoubleFuncILDDDD VelocityGetRelativePermeability = NULL;
DoubleFuncLong VELGetNodeTemperature = NULL;
VoidFuncLID VELSetElementVelocityX = NULL;
VoidFuncLID VELSetElementVelocityY = NULL;
VoidFuncLID VELSetElementVelocityZ = NULL;

DoubleFuncLI VELGetElementVelocityX = NULL;
DoubleFuncLI VELGetElementVelocityY = NULL;
DoubleFuncLI VELGetElementVelocityZ = NULL;
/* Interne (statische) Deklarationen */
void CalcVelo1D(int phase, long index, double theta, CRFProcess*m_pcs);
void CalcVelo2D(int phase, long index, double theta, CRFProcess*m_pcs);
void CalcVelo3D(int phase, long index, double theta, CRFProcess*m_pcs);
void CalcVelo2DTriangle(int phase, long index, double theta, CRFProcess*m_pcs);


void CalcVelo1Dr(int, long, double, double, double, double, double *);
void CalcVelo2Drs(int, long, double, double, double, double, double *);
void CalcVelo3Drst(int, long, double, double, double, double, double *);

/* Funktionszeiger auf die 1D-, 2D-, 3D- Berechnungsfunktionen fuer
   lokale Koordinaten */
static VoidFuncILDDDDDX CalcVeloRST[3] =
{CalcVelo1Dr, CalcVelo2Drs, CalcVelo3Drst};

/* Interne Steuerung der Geschw.berechnung */
//static int integration_array_size = 0;
//OK static INTEGRATIONPOINTS *integration_array = NULL;

int VELElementIntegrationMethodDensity = 0;
int VELElementIntegrationMethodViscosity = 0;
int VELElementIntegrationMethodRelPerm = 1;
int VELElementIntegrationMethodMaximum = 1;
int VELCorrectVelocityOnIrrNodesElements = 0;

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_VEL

 Aufgabe:
   Initialisiert die Geschwindigkeitsberechnung gemaess der im
   Schluesselwort "#NUMERICS" gegebenen Parameter

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   9/2000   C.Thorenz    Erste Version

**************************************************************************/
void CECalcGaussVelo_NEW(int phase, long index, double theta,CRFProcess*m_pcs)
{
int eletype;
eletype = ElGetElementType(index) - 1;
// CalcVeloGauss[ElGetElementType(index) - 1] (phase, index, theta);  //ersetzt durch switchcase
switch(eletype){
	case 0:
		CalcVelo1D(phase, index, theta, m_pcs);
		break;
	case 1:
		CalcVelo2D(phase, index, theta, m_pcs);
		break;
	case 2:			
		CalcVelo3D(phase, index, theta, m_pcs);
		break;
	case 3:		
		CalcVelo2DTriangle(phase, index, theta, m_pcs);
		break;
	case 4: 
		CalcVelo3DTetrahedra(phase, index, theta, m_pcs); //CMCD 09/2004 Geosys 4
		break;
	case 5:   
		CalcVelo3DPrismRST_NEW(phase, index, theta, 0.0, 0.0, 0.0, m_pcs);
		break;
	default:
		DisplayMsgLn(" Error - unknown element type in CECalcGaussVelo_NEW");
};

}

/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo1D

 Aufgabe:
   Berechnung der Geschwindigkeiten in den Gauss-Punkten fuer 1D-Elemente

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements, dessen Gauss-Geschwindigkeiten
   berechnet werden sollen

 Ergebnis:
   - void -

 Programmaenderungen:
   02/1997     R.Kaiser       Uebertragen aus dem aTM
   04/1997     R.Kaiser       Druckformulierung des Darcy-Gesetzes
   11/1997     O.Kolditz      nichtlineare Fliessgesetze
   22.03.1998  C.Thorenz       Einbauen des Dichteeinflusses
   20.02.1999  C.Thorenz       Verallgemeinerung
    9/2000     C.Thorenz  Speicher nur einmal holen -> schneller
   03/2005     MB head version

*************************************************************************/
void CalcVelo1D(int phase, long index, double theta, CRFProcess*m_pcs)
{
  m_pcs = m_pcs;
  double *velovec;
  /* Speicher holen */
  velovec = ElGetVelocityNew(index, phase);
  CalcVelo1Dr(phase, index, theta, 0., 0., 0., velovec);
  velovec = velovec;
  return;
}


/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo2D

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten in den Gauss-Punkten fuer 2D-Elemente

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements, dessen Gauss-Geschwindigkeiten
   berechnet werden sollen

 Ergebnis:
   - void -

 Programmaenderungen:
   02/1997     RK   Uebertragen aus dem aTM
   10/1997     OK   Druckformulierung,
   Materialmodell (3) fuer Kluftpermeabilitaetsverteilung
   22.03.1998  CT   Einbauen des Dichteeinflusses
   20.02.1999  CT   Verallgemeinerung
   03/1999     OK   Kapselung - CalculateNonlinearFlowPermeability (rf3220)
   03/1999     OK   Kapselung - GetPermeabilityTensor (rf3220)
    9/2000     C.Thorenz  Speicher nur einmal holen -> schneller
   11/2002     MB   MATCalcFluidDensity
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version
*************************************************************************/
void CalcVelo2D(int phase, long index, double theta, CRFProcess*m_pcs)
{
  static int i, j, nn = 4;
  static long ind;
  static double r, s;
  static double detjac;
  static double grad_omega[8], invjac[4];
  static long *nodes;
  static double p[4], z[4];
  static int anzgp;
  static double *velovec;
  static double v[3], v_xyz[3], w[2], v1[2], v2[2];
  static int nonlinear_flow_element;
  static double k_rel = 1., k_r = 1., mu, rho, g, fakt;
  static double kf[4];
  static double *k_ij;
  static int status;
  static long vorgaenger;
  static double gp[3]; 
  static int nidx0, nidx1;
  //----------------------------------------------------------------------
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  velovec = ElGetVelocityNew(index, phase);
  nodes = ElGetElementNodes(index);
  //----------------------------------------------------------------------
  /* An irregulaeren Knoten wird die Geschw. auf Basis des Vorgaengers
     in Elementmitte eingetragen  -> Dichtestroemungen!!! */
  if (VELCorrectVelocityOnIrrNodesElements == 1)
    {
      status = CriticalElements(index, &vorgaenger);
      if (status)
        {
          CalcVelo2Drs(phase, vorgaenger, theta, 0., 0., 0., v);
          /* Es muss der Uebergang auf globalen Koordinaten ermittelt werden ! */  
          Calc2DElement_ab2xyz_Vector(vorgaenger, v, v_xyz);
          Calc2DElement_xyz2ab_Vector(index, v_xyz, v);

          ind = -1;
          for (i = 0; i < anzgp; i++)
            {
              for (j = 0; j < anzgp; j++)
                {
                  velovec[++ind] = v[0];
                  velovec[++ind] = v[1];
                }
            }
          return;
        }
    }
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  //OK_MMP nonlinear_flow_element = GetNonlinearFlowElement(index);
  nonlinear_flow_element = 0;
  k_r = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
  //--------------------------------------------------------------------
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
  if(nonlinear_flow_element==1) {
    for (i = 0; i < 4; i++)
      h[i] = p[i] / (g * rho) + z[i];
    k_rel *= CalculateMeanNonlinearFlowPermeability(index, grad_h);
  }
*/
  /* Permeabilitaetstensor */
  for (i = 0; i < 4; i++)
    kf[i] = -k_ij[i];
  fakt = k_rel * k_r / mu;
  //--------------------------------------------------------------------
  ind = -1;
  for (i = 0; i < anzgp; i++)
    {
      for (j = 0; j < anzgp; j++)
        {
          /* Gauss-Punkt */
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          gp[0]=r; gp[1]=s; gp[2]=0.0;
          /* Jakobi-Matrix und ihre Inverse berechnen */
          Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
          /* Gradientenmatrix */
          MGradOmega2D(grad_omega, r, s);
          /* kf * J^-1 * grad(omega) * h */
          /* Druckformulierung */
          /* k/my * J^-1 * grad(omega) * p - Druckkraft */
          MMultMatVec(grad_omega, 2, 4, p, 4, v, 2);
          MMultVecMat(v, 2, invjac, 2, 2, w, 2);
          MMultMatVec(kf, 2, 2, w, 2, v1, 2);
          /* k/my * J^-1 * grad(omega) * z - Schwerkraft */
          MMultMatVec(grad_omega, 2, 4, z, 4, v, 2);
          MMultVecMat(v, 2, invjac, 2, 2, w, 2);
          MMultMatVec(kf, 2, 2, w, 2, v2, 2);
          if (VELElementIntegrationMethodMaximum)
            {
              /* Modellgestuetzte relative Permeabilitaet */
              if (VELElementIntegrationMethodRelPerm)
                if (VelocityGetRelativePermeability)
                  k_r = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
              /* Dichte */
              if (VELElementIntegrationMethodDensity)
                rho = m_mfp->Density();
              /* Viscositaet */
              if (VELElementIntegrationMethodViscosity)
                mu  = m_mfp->Viscosity();

              fakt = k_rel * k_r / mu;
            }
          /* Vektor der Filtergeschwindigkeit im Gauss-Punkt */
          velovec[++ind] = (v1[0] + rho * g * v2[0]) * fakt;
          velovec[++ind] = (v1[1] + rho * g * v2[1]) * fakt;
        }
    }
}


/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo3D

 Aufgabe:
   Berechnung der Geschwindigkeiten in den Gauss-Punkten fuer 3D-Elemente

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements, dessen Gauss-Geschwindigkeiten
   berechnet werden sollen

 Ergebnis:
   - void -

 Programmaenderungen:
   02/1997      R.Kaiser         Uebertragen aus dem aTM
   27.07.1997   R.Kaiser         Verallgemeinert
   22.03.1998  C.Thorenz       Einbauen des Dichteeinflusses
   20.02.1999  C.Thorenz       Verallgemeinerung
    9/2000     C.Thorenz  Speicher nur einmal holen -> schneller
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version
*************************************************************************/
void CalcVelo3D(int phase, long index, double theta,CRFProcess*m_pcs)
{
  static int i, j, k, anzgp, nn = 8, status;
  static long ind, vorgaenger;
  static double detjac, r, s, t;
  static double *velovec;
  static double invjac[9], p[8];
  static double mu, rho, g, k_rel = 1., k_r = 1., fakt;
  static double grad[24], v[3], w[3];
  static long *nodes;
  static double *k_ij;
  static double gp[3]; 
  double gp_temp[3];
  int nidx0, nidx1;
  k_rel = 1., k_r = 1. ; /*CMCD 11.09.2003*/
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  /* Speicher holen */
  velovec = ElGetVelocityNew(index, phase);
  /* An irregulaeren Knoten wird die Geschw. auf Basis des Vorgaengers
     in Elementmitte eingetragen  -> Dichtestroemungen!!! */

  if (VELCorrectVelocityOnIrrNodesElements == 1)
    {
      status = CriticalElements(index, &vorgaenger);
      if (status)
        {
          index = vorgaenger;
          CalcVelo3Drst(phase, index, theta, 0., 0., 0., v);
          ind = -1;
          for (i = 0; i < anzgp; i++)
            {
              for (j = 0; j < anzgp; j++)
                {
                  for (k = 0; k < anzgp; k++)
                    {
                      velovec[++ind] = v[0];
                      velovec[++ind] = v[1];
                      velovec[++ind] = v[2];
                    }
                }
            }
          return;
        }
    }
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_r = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
  k_rel = m_mmp->PermeabilityPressureFunction(index,gp,theta);		// CMCD Einbau
  //--------------------------------------------------------------------
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
  else{
    nidx0 = PCSGetNODValueIndex("PRESSURE1",0);
    nidx1 = PCSGetNODValueIndex("PRESSURE1",1);
  }

  /* Modellgestuetzte relative Permeabilitaet */
  // if (VelocityGetRelativePermeability)  {  //MB
  //	k_r = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);  //MX 
  //  k_rel = m_mmp->PermeabilityPressureFunction(index,gp,theta);		// CMCD Einbau
  // }

  nodes = ElGetElementNodes(index);

  /* Vektor der Druecke */
  if (VelocityGetNodePress)
    {
      for (i = 0; i < nn; i++)
        {
          p[i] = VelocityGetNodePress(phase, nodes[i], index, theta);
        }
    }
  else
    {
      for (i = 0; i < nn; i++)
        {
          p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0)\
               + theta * GetNodeVal(nodes[i],nidx1);
        }
    }

  fakt = k_rel * k_r / mu;

  /* r,s,t-Koordinatensystem */
  ind = -1;

  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)  {

          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          t = MXPGaussPkt(anzgp, k);

          /* inverse Jakobi-Matrix */
          Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
          /* kf * J^-1 * grad(omega) * p */
          MGradOmega3D(grad, r, s, t); /* Gradientenmatrix */
          MMultMatVec(grad, 3, 8, p, 8, v, 3);
          MMultMatVec(invjac, 3, 3, v, 3, w, 3);

          if (VELElementIntegrationMethodMaximum)  {
            /* Modellgestuetzte relative Permeabilitaet */
            if (VELElementIntegrationMethodRelPerm)  {
// MB               if (VelocityGetRelativePermeability){
// MB                 k_r = VelocityGetRelativePermeability(phase, index, r, s, t, theta);
// MB               }
			  gp_temp[0] = r;
			  gp_temp[1] = s;
			  gp_temp[2] = t;
			  k_r = m_mmp->PermeabilitySaturationFunction(index,gp_temp,theta,phase);    //MX 
              k_rel = m_mmp->PermeabilityPressureFunction(index,gp_temp,theta);
            }		// CMCD Einbau
            /* Dichte */
            if (VELElementIntegrationMethodDensity)
              rho = m_mfp->Density();    // CMCD Einbau
            /* Viscositaet */
            if (VELElementIntegrationMethodViscosity)
              mu  = m_mfp->Viscosity();  // CMCD Einbau

              fakt = k_rel * k_r / mu;
            }

          w[2] += rho * g;

          MMultMatVec(k_ij, 3, 3, w, 3, v, 3);  /* OK rf3220 */

          velovec[++ind] = -v[0] * fakt;
          velovec[++ind] = -v[1] * fakt;
          velovec[++ind] = -v[2] * fakt;
        }

}


/**************************************************************************
 ROCKFLOW - Funktion: CalcVeloXDrst

 Aufgabe:
   Berechnung der Geschwindigkeit in einem Punkt

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
                   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-t-System
   double s    : Koordinate im r-s-t-System
   double t    : Koordinate im r-s-t-System

   R double *v[3]  : Geschw. im x-y-z-System (1D, 3D) oder a-b-System (2D)

 Ergebnis:
   - void -

 Programmaenderungen:
   03.03.1999  C.Thorenz   Erste Version
   9/2000  C.Thorenz       Kritische Element an irr. Knoten behandeln

**************************************************************************/
void CalcVeloXDrst(int phase, long index, double theta, double r, double s, double t, double *v)
{
  static int status;
  static long vorgaenger;

  if (VELCorrectVelocityOnIrrNodesElements == 1)
    {
      status = CriticalElements(index, &vorgaenger);
      if (status)
        index = vorgaenger;
    }

  CalcVeloRST[ElGetElementType(index) - 1] (phase, index, theta, r, s, t, v);
}
/**************************************************************************
 ROCKFLOW - Funktion: CalcVeloXDrstxyz

 Aufgabe:
   Berechnung der Geschwindigkeit in einem Punkt, gibt Geschwindigkeit
   fuer alle Dimensionen im xyz-System zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
                   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-t-System
   double s    : Koordinate im r-s-t-System
   double t    : Koordinate im r-s-t-System

   R double *v[3]  : Geschw. im x-y-z-System

 Ergebnis:
   - void -

 Programmaenderungen:
   1/2001  C.Thorenz       Erste Version

**************************************************************************/
void CalcVeloXDrstxyz(int phase, long index, double theta, double r, double s, double t, double *v)
{
  static int status;
  static long vorgaenger;
  static double v_ab[3];
  static double vf[12];         /* Vektorfeld */
  static double trans[6];       /* Drehmatrix */

  if (VELCorrectVelocityOnIrrNodesElements == 1)
    {
      status = CriticalElements(index, &vorgaenger);
      if (status)
        index = vorgaenger;
    }

  if (ElGetElementType(index) == 2)
    {
      CalcVeloRST[ElGetElementType(index) - 1] (phase, index, theta, r, s, t, v_ab);
      CalcTransformationsMatrix(index, vf, trans);
      MMultMatVec(trans, 3, 2, v_ab, 2, v, 3);
    }
  else
    {
      CalcVeloRST[ElGetElementType(index) - 1] (phase, index, theta, r, s, t, v);
    }

}



/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo1Dr

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer 1D-Elemente

   Die aufrufende Funktion muss Speicher fuer den Ergebnisvektor
   bereit stellen.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Nicht benutzt
   double s    : Nicht benutzt
   double t    : Nicht benutzt

   R double *v   : Geschw. im x-y-z-System


 Ergebnis:
   - void -

 Programmaenderungen:
   17.02.1998  C.Thorenz     Abgeleitet aus CalcVelo1D
   22.03.1998  C.Thorenz     Einbauen des Dichteeinflusses
   22.03.1998  C.Thorenz     Einbauen der Zeitwichtung fuer Druck
   20.02.1999  C.Thorenz     Verallgemeinerung
08/2004 OK MFP implementation

*************************************************************************/
void CalcVelo1Dr(int phase, long index, double theta, double r, double s, double t, double *v)
{
  static double r2;             /* Dummy */
  /* Datentypen der verwendeten Variablen */
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
  //  static double *k_ij;

  double gp[]={r,s,t};

  r2 = r;
  r2 = s;                              /* Dummy, keine Funktion !!! */
  r2 = t;                              /* Dummy, keine Funktion !!! */

  /* Materials */
  //----------------------------------------------------------------------
  // Medium properties 
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

if(gravity_constant<MKleinsteZahl){ // HEAD version
  mu = 1.0;
  rho = 1.0;
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
  if (VelocityGetNodePress)
    {
      for (i = 0; i < nn; i++)
        {
          p[i] = VelocityGetNodePress(phase, nodes[i], index, theta);
        }
    }
  else
    {
      for (i = 0; i < nn; i++)
        {
//OK_MOD          p[i] = (1.-theta) * VelocityGetNodePress0(nodes[i]) + theta * VelocityGetNodePress1(nodes[i]);
           p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",0))\
                + theta * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
        }
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
 ROCKFLOW - Funktion: CalcVelo2Drs

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer 2D-Elemente an
   beliebigen Punkten im rs-System

   Die aufrufende Funktion muss Speicher fuer den Ergebnisvektor
   bereit stellen. Rueckgabe im a-b-System

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-System
   double s    : Koordinate im r-s-System
   double t    : Nicht benutzt

   R double *velo[2]  : Geschw. im a-b-System


 Ergebnis:
   - void -

 Programmaenderungen:
   12.01.1998  C.Thorenz     Abgeleitet aus CalcVelo2d
   22.03.1998  C.Thorenz     Einbauen des Dichteeinflusses
   22.03.1998  C.Thorenz     Einbauen der Zeitwichtung fuer Druck
   6.05.1998  C.Thorenz     Bugfix Zeitwichtung
   20.02.1999  C.Thorenz     Verallgemeinerung
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version

*************************************************************************/
void CalcVelo2Drs(int phase, long index, double theta, double r, double s, double t, double *velo)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static int i, nn = 4;
  /* Elementgeometriedaten */
  static double detjac, invjac[4], r2;
  static double grad_omega[8];
  /* Knotendaten */
  static long *nodes;
  static double p[4], z[4];
  /* Elementdaten */
  static double *velovec;
  static double v[2], w[2];
  static double v1[2], v2[2];
  /* Materialdaten */
  static int nonlinear_flow_element;
  static double k_rel_grad_p = 1.;
  static double k_rel_S = 1.;
  static double mu, rho, g;
  static double kf[4];
  static double *k_ij;
  static double gp[3];
  static int nidx0, nidx1;
  r2 = t;                              /* Dummy, keine Funktion !!! */
/****************************************************************************/
/* Daten bereitstellen
 ************************************************************************** */
  /* Knotendaten */
  nodes = ElGetElementNodes(index);
  /* Materialdaten */
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  //OK_MMP nonlinear_flow_element = GetNonlinearFlowElement(index);
  nonlinear_flow_element = 0;
  k_rel_S = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
/*OK ???
  if (((GetSoilRelPermPressModel(index) !=10) && (k_rel_S > 0.0)) || GetSoilPorosityModel(index) == 0)  // MX 022004
      k_rel_S = MMax(MKleinsteZahl, k_rel_S);
*/
  //--------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  CRFProcess* m_pcs = NULL;
  m_pcs = pcs_vector[0];

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
    // MB head version p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex(pressure_phase,0))
    // MB head version      + theta * GetNodeVal(nodes[i],PCSGetNODValueIndex(pressure_phase,1));
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
    z[i] = GetNode(nodes[i]) -> z;
  }
/****************************************************************************/
/* Permeabilitaetstensor berechnen
 ************************************************************************** */
  /* Nichtlineares Fliessgesetz */
  if (nonlinear_flow_element == 1)
    //k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
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
  Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
  /* Gradientenmatrix */
  MGradOmega2D(grad_omega, r, s);

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

  Free(velovec);

}


/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo3Drst

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer 3D-Elemente an
   beliebigen Punkten im rst-System

   Die aufrufende Funktion muss Speicher fuer den Ergebnisvektor
   bereit stellen.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-t-System
   double s    : Koordinate im r-s-t-System
   double t    : Koordinate im r-s-t-System

   R double *v[3]  : Geschw. im x-y-z-System

 Ergebnis:
   - void -

 Programmaenderungen:
   17.02.1998  C.Thorenz     Abgeleitet aus CalcVelo3D
   22.03.1998  C.Thorenz     Einbauen des Dichteeinflusses
   22.03.1998  C.Thorenz     Einbauen der Zeitwichtung fuer Druck
   20.02.1999  C.Thorenz     Verallgemeinerung
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
10/2004 OK phase pressures
02/2005 MB head version
*************************************************************************/
void CalcVelo3Drst(int phase, long index, double theta, double r, double s, double t, double *v)
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
  static double gp[3]; 
  static int nidx0, nidx1; 
  //--------------------------------------------------------------------
  // 
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  nodes = ElGetElementNodes(index);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_rel = 1.;
  double permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
  //--------------------------------------------------------------------
  // MFP fluid properties
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
  mu  = m_mfp->Viscosity();

  CRFProcess* m_pcs = NULL;
  m_pcs = pcs_vector[0];

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
    //MB head version p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex(pressure_phase,0)
    //MB head version     + theta * GetNodeVal(nodes[i],PCSGetNODValueIndex(pressure_phase,1));
    p[i] = (1.-theta) * GetNodeVal(nodes[i],nidx0) + theta * GetNodeVal(nodes[i],nidx1);
  }

  /* r,s,t-Koordinatensystem */
  /* inverse Jakobi-Matrix */
  Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
  /* kf * J^-1 * grad(omega) * p */
  MGradOmega3D(grad, r, s, t);         /* Gradientenmatrix */
  MMultMatVec(grad, 3, 8, p, 8, v, 3);
  MMultMatVec(invjac, 3, 3, v, 3, w, 3);
  w[2] += rho * g;
  /* Modellgestuetzte relative Permeabilitaet */
//  if (VelocityGetRelativePermeability)
//    k_rel = VelocityGetRelativePermeability(phase, index, r, s, t, theta);
  permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,theta,phase);
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
ROCKFLOW - Funktion: CalcVelo2DTriangleRS
Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer 2D-Dreiecks-Elemente an
   beliebigen Punkten im rs-System
   Die aufrufende Funktion muss Speicher fuer den Ergebnisvektor
   bereit stellen. Rueckgabe im a-b-System
Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-System
   double s    : Koordinate im r-s-System
   double t    : Nicht benutzt
   R double *velo[2]  : Geschw. im a-b-System
Programmaenderungen:
07/2000 OK Implementierung
08/2003 OK MATCalcRelativePermeability
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
10/2004 CMCD
check vector direction
03/2005  MB head
*************************************************************************/
void CalcVelo2DTriangle(int phase, long index, double theta, CRFProcess*m_pcs)
{
  int i;
  int p_idx1;
  //int timelevel;
  double x[3], y[3]; //CMCD local coordinates
  double xg[3], yg[3], zg[3]; //CMCD global coordinates
  double dN_dx[3],dN_dy[3]; //CMCD shape functions local coordinates
  double area; //CMCD
  long *nodes;
  double nodepressure[3]; //p[3];
  int nn = 3;
  double *velovec;
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
  //OK_MMP nonlinear_flow_element = GetNonlinearFlowElement(index);
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
    //nidx0 = PCSGetNODValueIndex(pcs_primary,0);
    //nidx1 = PCSGetNODValueIndex(pcs_primary,1);  
  }  
  else {
    p_idx1 = PCSGetNODValueIndex("PRESSURE1",1); 
  }
  
  //--------------------------------------------------------------------
  // phase = 0; MB
  // timelevel = 1; MB
  // p_idx1 = PCSGetNODValueIndex("PRESSURE1",timelevel);  MB
  for(i=0;i<nn;i++){
    nodepressure[i] = GetNodeVal(nodes[i],p_idx1);
  }	
  for(i=0;i<nn;i++){
    //OK_MOD p[i] = (1. - theta) * VelocityGetNodePress0(nodes[i]) + theta * VelocityGetNodePress1(nodes[i]);
    //p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",0))
         //+ theta * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
    //z[i] = GetNode(nodes[i])->z;
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
ROCKFLOW - Funktion: CalcVelo2DTriangleRS
Aufgabe:
  Berechnung der Darcy-Geschwindigkeiten fuer 2D-Dreiecks-Elemente an
  beliebigen Punkten im rs-System
  Die aufrufende Funktion muss Speicher fuer den Ergebnisvektor
  bereit stellen. Rueckgabe im a-b-System
Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
  werden soll. Nummerierung beginnt mit 0.
  long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
  double theta: Wichtung zwischen alter und neuer Zeitebene
  double r    : Koordinate im r-s-System
  double s    : Koordinate im r-s-System
  double t    : Nicht benutzt
  R double *velo[2]  : Geschw. im a-b-System
Programmaenderungen:
07/2000   OK   Implementierung
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
*************************************************************************/
void CalcVelo2DTriangleRS(int phase, long index, double theta, double r, double s, double t, double *velo)
{
  static int i;
  static double x[3], y[3];
  static double grad_x_phi[3], grad_y_phi[3];
  static double area;
  static long *nodes;
  static double p[3], z[3];
  static int nn = 3;
  static double *velovec;
  static double v1[2], v2[2];
  static int nonlinear_flow_element;
  static double k_rel_grad_p = 1.;
  static double k_rel_S = 1.;
  static double mu, rho, g;
  static double k_x, k_y;
  static double gp[3]={0.0,0.0,0.0};
  r=r;s=s;t=t;
  //--------------------------------------------------------------------
  nodes = ElGetElementNodes(index);
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double* k_ij;
  k_ij = m_mmp->PermeabilityTensor(index); //permeability;
  k_x = k_ij[0];
  k_y = k_ij[3];
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
if(gravity_constant<MKleinsteZahl){ // HEAD version
  mu = 1.0;
  rho = 1.0;
}
  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    //OK_MOD p[i] = (1. - theta) * VelocityGetNodePress0(nodes[i]) + theta * VelocityGetNodePress1(nodes[i]);
    p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",0))\
         + theta * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
    z[i] = GetNode(nodes[i]) -> z;
  }
  //--------------------------------------------------------------------
/*OK ReDo
  if (nonlinear_flow_element == 1)
    k_rel_grad_p = CalcNonlinearFlowPermeability1(index, VelocityGetNodePress1, NULL);
  if (VelocityGetRelativePermeability)
    k_rel_S = VelocityGetRelativePermeability(phase, index, r, s, 0., theta);
*/
  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    x[i] = GetNodeX(nodes[i]);
    y[i] = GetNodeY(nodes[i]);
  }
  area = ElGetElementVolume(index);
  grad_x_phi[0] = (y[1] - y[2]) / (2. * area);
  grad_x_phi[1] = (y[2] - y[0]) / (2. * area);
  grad_x_phi[2] = (y[0] - y[1]) / (2. * area);
  grad_y_phi[0] = (x[2] - x[1]) / (2. * area);
  grad_y_phi[1] = (x[0] - x[2]) / (2. * area);
  grad_y_phi[2] = (x[1] - x[0]) / (2. * area);

  v1[0] = MSkalarprodukt(grad_x_phi, p, nn);
  v1[1] = MSkalarprodukt(grad_y_phi, p, nn);
  v2[0] = MSkalarprodukt(grad_x_phi, z, nn);
  v2[1] = MSkalarprodukt(grad_y_phi, z, nn);

  velo[0] = -k_x * k_rel_grad_p * k_rel_S / mu * (v1[0] + rho * g * v2[0]);
  velo[1] = -k_y * k_rel_grad_p * k_rel_S / mu * (v1[1] + rho * g * v2[1]);

  Free(velovec);

}

/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo3DTetrahedra
 Aufgabe:
   Calculation of velocity in tetrahedra
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   int phase   : Number of the phase for which the velocity should be calculated
   long number : Index of the element
   double theta: Time collation
 Programmaenderungen:
   05/2004   CMCD Implementation
   09/2004   CMCD GeoSys 4
   03/2005   MB head version
*************************************************************************/
void CalcVelo3DTetrahedra(int phase, long index, double theta, CRFProcess*m_pcs)
{
  int i;
  int p_idx0, p_idx1;
  //int timelevel;
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
  double *velovec;
  double v1[3], v2[3];
  int nonlinear_flow_element=0;
  double k_rel_grad_p = 1.;
  double k_rel_S = 1.;
  double mu, rho, g, k_rel; 
  double k_x, k_y, k_z;
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
  //phase = 0;
  //timelevel = 0;
  //p_idx0 = PCSGetNODValueIndex("PRESSURE1",timelevel);
  //timelevel = 1;
  //p_idx1 = PCSGetNODValueIndex("PRESSURE1",timelevel);
  
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


/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo3DPrismXYZ

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer Prismenelemente im Mittelpunkt
   des Elementes

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-System
   double s    : Koordinate im r-s-System
   double t    : Nicht benutzt

   R double *velo[2]  : Geschw. im a-b-System


 Ergebnis:
   - void -

 Programmaenderungen:
   02/2003   MB   Implementierung
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version

*************************************************************************/
void CalcVelo3DPrismXYZ(int phase, long index, double theta, double *velo)
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

  CRFProcess* m_pcs = NULL;
  m_pcs = pcs_vector[0];

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
  }  
  
  int nidx0 = PCSGetNODValueIndex(pcs_primary,0);
  int nidx1 = PCSGetNODValueIndex(pcs_primary,1);  

  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    //p[i] = (1.-theta) * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",0))
    //     + theta * GetNodeVal(nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
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
//TODO Martin  coord = MSHGetGravityCenter(index);
  xx = coord[0];
  yy = coord[1];

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
 ROCKFLOW - Funktion: CalcVelo3DPrismRST

 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer Prismenelemente im Mittelpunkt
   des Elementes

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-System
   double s    : Koordinate im r-s-System
   double t    : Nicht benutzt

   R double *velo[2]  : Geschw. im a-b-System

Programmaenderungen:
02/2003   MB   Implementierung
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version
*************************************************************************/
double* CalcVelo3DPrismRST(int phase, long index, double theta, double r, double s, double t)
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
  static double gp[3];
  static double GradOmPrism[18];
  static double velo[3];
  static double invjac[9], detjac;
  //--------------------------------------------------------------------
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

  CRFProcess* m_pcs = NULL;
  m_pcs = pcs_vector[0];

  string pcs_primary = m_pcs->pcs_primary_function_name[0];
  if(pcs_primary.find("HEAD")!=string::npos) {
    g = 0.0;
    mu = 1.0;
    rho = 1.0;
  }  
  
  int nidx0 = PCSGetNODValueIndex(pcs_primary,0);
  int nidx1 = PCSGetNODValueIndex(pcs_primary,1);  

  //--------------------------------------------------------------------
  for(i=0;i<nn;i++){
    //OK_MOD p[i] = (1. - theta) * VelocityGetNodePress0(nodes[i]) + theta * VelocityGetNodePress1(nodes[i]);
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
  MGradOmegaPrism( r, s, t, GradOmPrism);               /* 3 Zeilen 6 Spalten */
  /* Inverse Jakobian Matrix */
  CalcPrismElementJacobiMatrix(index, r, s, t, invjac, &detjac);
  /*  kf * InvJac * GradOmPrism * P  */
  MMultMatVec(GradOmPrism, 3, 6, p, 6, zwi, 3);
  MMultMatVec(invjac, 3, 3, zwi, 3, grad_p, 3);
  grad_p[2] = grad_p[2] + (rho * g);
  velo[0] = -k_x * k_rel_grad_p * k_rel_S / mu * grad_p[0];
  velo[1] = -k_y * k_rel_grad_p * k_rel_S / mu * grad_p[1];
  velo[2] = -k_z * k_rel_grad_p * k_rel_S / mu * grad_p[2];
  return velo;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: VELCalculateAllGaussVelocities
 */
/* Aufgabe:
   Berechnung der Geschwindigkeiten in den Gauss-Punkten der Elemente
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   0 bei Fehler, sonst 1 (dient nur zum Abbrechen der Funktion)
 */
/* Programmaenderungen:
   02/2004   SB/OK    Implementierung  //SB:2p
 */
/**************************************************************************/
void VELCalculateAllGaussVelocities(CRFProcess*m_pcs)
{
  long i;
  int phase;
  double theta = 1.0; //VELGetTimeCollocation();

  for (i = 0; i < ElListSize(); i++)
    if (ElGetElement(i) != NULL)       /* Element existiert */
      if (ElGetElementActiveState(i)) {
		for (phase=0;phase<GetRFProcessNumPhases();phase++)  {
		  CECalcGaussVelo_NEW(phase,i,theta,m_pcs);
		  VELCalcElementVelocity(i,theta,phase);
		}
      }
  }



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyAllElementGaussVelocities
 */
/* Aufgabe:
   Loeschen der Element-Gauss-Geschwindigkeiten
   aller aktiver Elemente
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   10/2000   OK   Implementierung
 */
/**************************************************************************/
void DestroyAllElementGaussVelocitiesPhase(int phase)
{
  long i;

  for (i = 0; i < ElListSize(); i++)
    {
      if (ElGetElement(i) != NULL)
        {                              /* Element existiert */
          if (ElGetElementActiveState(i))
            {                          /* aktives Element */
              ElDeleteElementGaussVeloPhase(i, phase);
            }
        }
    }

}


/**************************************************************************/
/* ROCKFLOW - Funktion: VELCalcElementVelocity              aemb
                                                                          */
/* Task:
   Aufruf der Funktionen zur Berechnung von Darcy Geschwindigkeiten
                                                                          */
/* Parameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: long i: element index
   E: double theta: timestep weighting
                                                                          */
/* Result:
   - void -
                                                                          */
/* Programmaenderungen:
   06/2002   MB   Implementation
   11/2002   JDJ  �bergabe der Phase, Verweis auf eigenes Modell
   5/2003    OK/MB/JDJ  Von MOD nach VEL
11/2004 OK Bugfix for triangles
                                                                          */
/**************************************************************************/

void VELCalcElementVelocity(long i,double theta,int phase)
{
	static double v[3],v_ab[3];
	static double vf[12];       /* Vektorfeld */
	static double trans[6];     /* Drehmatrix */
  double* v_gp = NULL;
	switch(ElGetElementType(i)) {
		case 1:
		  CalcVelo1Dr(phase, i, theta, 0., 0., 0., v);
		break;
		case 2:
		  CalcVeloXDrstxyz(phase, i, theta, 0.0, 0.0, 0.0, v_ab);
          CalcVeloXDrst(phase,i,theta,0.,0.,0.,v_ab);
          CalcTransformationsMatrix(i, vf, trans);
          MMultMatVec(trans, 3, 2, v_ab, 2, v, 3);
        break;
		case 3:
		  CalcVelo3Drst(0, i, theta, 0., 0., 0., v);
		break;
		case 4:
          v_gp = ElGetVelocityNew(i,phase);
          v[0]=v_gp[0];v[1]=v_gp[1];v[2]=v_gp[2];
 		break;
        case 6:
   		  CalcVelo3DPrismXYZ(0, i, theta, v);
		break;
        } /* switch */
  switch(phase){
    case 0:
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY1_X"),v[0]);
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY1_Y"),v[1]);
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY1_Z"),v[2]);
      break;
    case 1:
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY2_X"),v[0]);
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY2_Y"),v[1]);
      ElSetElementVal(i,PCSGetELEValueIndex("VELOCITY2_Z"),v[2]);
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
 
/*************************************************************************
 ROCKFLOW - Funktion: CalcVelo3DPrismRST_NEW
 Aufgabe:
   Berechnung der Darcy-Geschwindigkeiten fuer Prismenelemente im Mittelpunkt
   des Elementes
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase   : Nummer der Phase fuer die die Filtergeschwindigkeit berechnet
   werden soll. Nummerierung beginnt mit 0.
   long number : Index des Elements, dessen Geschwindigkeiten berechnet werden
   double theta: Wichtung zwischen alter und neuer Zeitebene
   double r    : Koordinate im r-s-System
   double s    : Koordinate im r-s-System
   double t    : Nicht benutzt
   R double *velo[2]  : Geschw. im a-b-System
Programmaenderungen:
02/2003   MB   Implementierung
02/2004   SB   Anpassen an PCS+MTM2 Concept
08/2004 OK MFP implementation start
08/2004 OK MMP implementation start
02/2005 MB head version
*************************************************************************/
void CalcVelo3DPrismRST_NEW(int phase, long index, double theta, double r, double s, double t, CRFProcess*m_pcs)
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
  static double gp[3];
  static double GradOmPrism[18];
  static double *velo;
  static double invjac[9], detjac;
  static int nidx0, nidx1;
  //----------------------------------------------------------------------
  nodes = ElGetElementNodes(index);
  //----------------------------------------------------------------------
  velo = ElGetVelocityNew(index, phase);
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
  //--------------------------------------------------------------------
  /* MGradOmegaPrism */
  MGradOmegaPrism( r, s, t, GradOmPrism);               /* 3 Zeilen 6 Spalten */
  /* Inverse Jakobian Matrix */
  CalcPrismElementJacobiMatrix(index, r, s, t, invjac, &detjac);
  /*  kf * InvJac * GradOmPrism * P  */
  MMultMatVec(GradOmPrism, 3, 6, p, 6, zwi, 3);
  MMultMatVec(invjac, 3, 3, zwi, 3, grad_p, 3);
  grad_p[2] = grad_p[2] + (rho * g);
/* OK
  velo[0] = -k_x * k_rel_grad_p * k_rel_S / mu * grad_p[0];
  velo[1] = -k_y * k_rel_grad_p * k_rel_S / mu * grad_p[1];
  velo[2] = -k_z * k_rel_grad_p * k_rel_S / mu * grad_p[2];
*/
  velo[0] = k_x * k_rel_grad_p * k_rel_S / mu * grad_p[0];
  velo[1] = k_y * k_rel_grad_p * k_rel_S / mu * grad_p[1];
  velo[2] = k_z * k_rel_grad_p * k_rel_S / mu * grad_p[2];
}

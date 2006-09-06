/**************************************************************************
FEMLib - Kernel MTM
Task: 
Programing:
  based on Kernel ATM by 
  MSR,HH,CB (09/1994-08/1996)
  OK,CT,RK  (10/1997-07/2002)
??/2003 SB
08/2004 SB GS/RF-4 Implementation
last modified:
**************************************************************************/
#include "stdafx.h"
#include "makros.h"

#define noTESTSM
#define noTESTSUPG
#define noTESTCEL_MTM2
/* noNUMERIC_MTM2 --> analytische Lösung für EMatrizen (für Dreiecksprismen) */
#define NUMERIC_MTM2_PRI

#include "cvel.h"
#include "rf_vel_new.h"
#include "cel_mtm2.h"
#include "int_mtm2.h"
#include "rf_pcs.h"
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "femlib.h"
#include "rfmat_cp.h"
#include "rf_num_new.h"
#include "tools.h"
#include "rf_tim_new.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
#include "rf_mfp_new.h"
#include "rf_pcs.h"
#include "rf_pcs.h" //OK_MOD"

/* Interne (statische) Deklarationen */
void CalcEle1D_MTM2(long index, long component, CRFProcess *m_pcs);
void CalcEle2D_MTM2(long index, long component, CRFProcess *m_pcs);
void CalcEle3D_MTM2(long index, long component, CRFProcess *m_pcs);
void MTM2CalcElementMatrices2DTriangle(long index, long component, CRFProcess *m_pcs); //SB:triangle
void MTM2CalcElementMatricesTetrahedra(long index, long component, CRFProcess *m_pcs); //SB: tetrahedra transport 
void CalcEle3DPrism_MTM2_ana(long index, long component, CRFProcess *m_pcs);
void CalcEle3DPrism_MTM2_num(long index, long component, CRFProcess *m_pcs);

/* SB für im */

/* Merker fuer die aktive Phase */
static int activ_phase;

/* Daten fuer den Matrixwiederaufbau */
typedef struct
  {
    char *name;
    int method;
    int mode;
    int phase;
    int comp;
    int curve;
    int group;
    double param[16];
    int number_reference_values;
    int *index_for_reference_values;
    int *index_for_current_values;
  }
Rebuild_info;

//static int MTM2_matrixrebuild_read = 0;
//static int MTM2_number_rebuild_info;
//static Rebuild_info *MTM2_rebuild_info;
//static int MTM2_rebuild_array_size = 0;
//static MATRIXREBUILD *MTM2_rebuild_array = NULL;


/* Vorkonfigurieren des Kernels */
//static int MTM2_new_kernel = 0;
//static int MTM2_method = 1;
static int MTM2_use_lagrange_for_2D = 0;
//static int MTM2_transport_in_phase = -1;
//static int MTM2_nonlinear_coupling = 1;

//static int MTM2_timecollocation_array_size = 0;
//static TIMECOLLOCATION *MTM2_timecollocation_array = NULL;
//static double MTM2_time_collocation_global = 1.; 
//WW: warning: `MTM2_time_collocation_global' was declared `extern'  and later `static
double MTM2_time_collocation_global;

//static double MTM2_time_collocation_cond_bc = 1.;
//static double MTM2_time_collocation_source = 1.;
static double MTM2_time_collocation_upwinding = 0.;
//static double MTM2_time_collocation_rebuild = 1.;

static int MTM2_upwind_method = 2;
static double MTM2_upwind_parameter = 1.;

static int MTM2_mass_lumping_method = 0;
//static double MTM2_mass_lumping_parameter = 0.;

//static int MTM2_predictor_method = 0;
//static double MTM2_predictor_parameter = 0.;

//static int MTM2_relaxation_method = 0;
//static double MTM2_relaxation_parameter[16];

static int MTM2_artificial_diffusion = 0;
static double MTM2_artificial_diffusion_param[16];

static int MTM2_oscil_damp_method = 0;
static double MTM2_oscil_damp_parameter[16];

#ifdef EFFORT
int MTM2_effort_index = -1;
int MTM2_effort_total_index = -1;
#endif

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_MTM2

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/


/**************************************************************************
 ROCKFLOW - Funktion: GetXXX_MTM2

 Aufgabe:
   Gibt div. Kernelparameter zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   22.04.99   C.Thorenz    Erste Version

**************************************************************************/
double GetTimeCollocationGlobal_MTM2(void)
{
  return MTM2_time_collocation_global;
}
/*
double GetTimeCollocationCondBC_MTM2(void)
{
  return MTM2_time_collocation_cond_bc;
}
double GetTimeCollocationSource_MTM2(void)
{
  return MTM2_time_collocation_source;
}
double GetTimeCollocationUpwinding_MTM2(void)
{
  return MTM2_time_collocation_upwinding;
}
*/
int GetArtificialDiffusion_MTM2(void)
{
  return MTM2_artificial_diffusion;
}
double *GetArtificialDiffusionParam_MTM2(void)
{
  return MTM2_artificial_diffusion_param;
}
/*
int GetOscillationDampingMethod_MTM2(void)
{
  return MTM2_oscil_damp_method;
}
double *GetOscillationDampingParam_MTM2(void)
{
  return MTM2_oscil_damp_parameter;
}
int GetMethod_MTM2(void)
{
  return MTM2_method;
}
int GetUseLagrange_MTM2(void)
{
  return MTM2_use_lagrange_for_2D;
}
int GetTransportInPhase_MTM2(void)
{
  return MTM2_transport_in_phase;
}
int GetNonLinearCoupling_MTM2(void)
{
  return MTM2_nonlinear_coupling;
}
int GetPredictorMethod_MTM2(void)
{
  return MTM2_predictor_method;
}
double GetPredictorParam_MTM2(void)
{
  return MTM2_predictor_parameter;
}
int GetRelaxationMethod_MTM2(void)
{
  return MTM2_relaxation_method;
}
double *GetRelaxationParam_MTM2(void)
{
  return MTM2_relaxation_parameter;
}

*/







/**************************************************************************
   ROCKFLOW - Funktion: CECalcPeclet_MTM2_NEW

   Aufgabe:
   Berechnet die Element-Peclet-Zahl und den Courant- (Cr=1) bzw.
   Neumann-Zeitschritt (Ne=0.5) des angegebenen 1D-, 2D- oder 3D-Elements
   Pec = v * ds / D
   dt_Courant = ds / v
   dt_Neumann = (ds * ds) / (2 * D)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long index: Index des Elements

   Ergebnis:
   - void -

   Programmaenderungen:
   01/1996     cb        Erste Version
   09/1996     cb        genauer - stroemungsgerichtet
   09/1996     cb        nur Courant-Kriterim einhalten
    9/2000     CT        Umbau fuer Mehrphasenmodell

 **************************************************************************/

/**************************************************************************
 ROCKFLOW - Funktion: CECalcEleMatrix_MTM2_NEW

 Aufgabe:
   Berechnet die Massenmatrix ohne 1/dt (nur zeitunabhaengigen Anteil),
   die dispersive und die advektive Matrix des angegebenen Elements
   (nur zeit- und geschwindigkeitsunabhaengigen Anteil, ohne Theta)

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long number: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   09/1994     MSR        Erste Version
   07/1995     hh         zweite Version
   05/1996     cb         dritte Version
    9/2000     CT        Umbau fuer Mehrphasenmodell

**************************************************************************/
void CECalcEleMatrix_MTM2_NEW( long index, long component, CRFProcess *m_pcs)
{
	  /* moved here from CECalcEleMatrix_MTM2 */
  MTM2_upwind_parameter = m_pcs->m_num->ele_upwinding; //OK
  MTM2_time_collocation_global = m_pcs->m_num->ls_theta; //OK

  switch (ElGetElementType(index))
    {
    case 1:
      CalcEle1D_MTM2(index, component, m_pcs);
      break;
    case 2:
      CalcEle2D_MTM2(index, component, m_pcs);
      break;
    case 3:
      CalcEle3D_MTM2(index, component, m_pcs);
      break;
	case 4:
	  MTM2CalcElementMatrices2DTriangle( index, component, m_pcs); //SB:triangle
	  break;
	case 5:
      MTM2CalcElementMatricesTetrahedra(index, component, m_pcs);
      break;
	case 6:
      #ifdef NUMERIC_MTM2_PRI /* Numerische Lösung */
      CalcEle3DPrism_MTM2_num(index, component, m_pcs);
      #else              /* Analytische Lösung */
      CalcEle3DPrism_MTM2_ana(index, component, m_pcs);
      #endif
      break;
    default:
      DisplayMsg("Kernel MTM2: Unknown element type!");
      abort();
    }
}



/**************************************************************************
 ROCKFLOW - Funktion: CalcEle1D_MTM2

 Aufgabe:
   Berechnet die Massenmatrix des angegebenen 1D - Elements
   ohne 1/dt fuer v=const (SUPG)

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   09/1994     Hans Herrmann    Erste Version
   03/1995     cb               SUPG in MakeGS
   07/1995     hh               zweite Version
   11/1995     msr              ueberarbeitet
   11/1995     cb               FLG = +-1
   05/1996     cb               dritte Version
   09/1996     cb               zu Testzwecken: Taylor-Galerkin (Kroehn)
   27.06.1997  R.Kaiser         Anpassung an die Funktion
                                CalcElementJacobiMatrix (femlib.c)
   10/1997  O.Kolditz        Implementierung des Waermetransportmodells
   12/1997  O.Kolditz        Filtergeschwindigkeiten q,qt
                                Abstandsgeschwindigkeiten v,vt
   10/1998  C.Thorenz       Zeitliche Aenderung der Porositaet
   06/1999  CT  Globalvariable halbwertszeit_1 entfernt
   10/1999  CT  Zugriff auf Saettigung wiederhergestellt (!)
    2/2000  CT  Phasennummer eingefuehrt
    7/2000  CT  Aenderung 10/1998 wieder zurueckgenommen.
    9/2000  CT  Umbau fuer Mehrphasenmodell, ML-FEM

  05/2003   SB  Umbau MTM2: Abbau, Sorption, Porositätsänderung
  10/2004   SB  Umbau, anpassen an GS4

**************************************************************************/
void CalcEle1D_MTM2(long index, long component, CRFProcess *m_pcs)
{

  static double porosity; 
//  static double molecular_diffusion, mass_dispersion_longitudinal;
  static double *dispersion_tensor;
  /* Elementgeometriedaten */
  static double *invjac, detjac;
  /* Elementdaten */
//  static double   d, tdt;
  static double v, vt, q, qt;
  static double *velovec=NULL;
  static double vorfk;
  static double zeta, pec, alpha[3], alpha_adv[3];      /* Upwind-Parameter */
  /* Elementmatrizen */
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;
  static double *decay;
  double gp[3], theta;

  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;

  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;

  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;

  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];

  theta = m_pcs->m_num->ls_theta; //OK

#ifdef TESTSUPG
  DisplayMsg("1D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
      /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);
      if (!mass)
        mass = (double *) Malloc(4 * sizeof(double));

      disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);

      if (!disp)
        disp = (double *) Malloc(4 * sizeof(double));

      adv = MTM2GetElementAdvMatrixNew(index, mtm_pcs_number);
      if (!adv)
        adv = (double *) Malloc(4 * sizeof(double));
	
	  if(GetRFProcessNumPhases()>1){
      content = MTM2GetElementFluidContentMatrixNew(index,  mtm_pcs_number);
      if (!content)
        content = (double *) Malloc(4 * sizeof(double));
	  }
      decay = MTM2GetElementDecayMatrixNew(index,  mtm_pcs_number);
      if (!decay)
        decay = (double *) Malloc(4 * sizeof(double));


  /* Initialisieren */
  MNulleMat(mass, 2, 2);
  MNulleMat(disp, 2, 2);
  MNulleMat(adv, 2, 2);
  MNulleMat(decay, 2, 2);
  if(GetRFProcessNumPhases()>1)
    MNulleMat(content, 2, 2);

  /* Elementdaten bereitstellen */
  invjac = GetElementJacobiMatrix(index, &detjac);      /* detjac= AL/2 */

  porosity = m_mat_mp->Porosity(index,gp,theta);
  if (m_mat_mp->porosity_model == 10)
	  porosity = m_mat_mp->PorosityVolumetricChemicalReaction(index); //MX
  if(GetRFProcessNumPhases()>1){
	//MX  porosity *= MTM2GetSaturation(iphase, index, 0., 0., 0., MTM2_time_collocation_global);
	  porosity *=PCSGetELEValue(index,gp,theta,"SATURATION2");
  }
 
  if(porosity < MKleinsteZahl) {
	  DisplayMsgLn("ERROR! Porosity is zero ");		
	  exit(0);
  }
  
  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);


  q = MBtrgVec(velovec, 3);
  v = q / porosity;
  qt = MSkalarprodukt(velovec, invjac, 3);

  vt = qt / porosity;

  dispersion_tensor = m_mat_mp->MassDispersionTensor(index,gp,theta,component);

#ifdef TESTTAYLOR
  /* Taylor-Galerkin nach Donea */
  printf("TESTTAYLOR, dt=%e\n", dt);
  MTM2_upwind_parameter = 0.0;          /* kein SU/PG */
  ElSetArtDiff(index, (-dt / 6.0 * vt * vt * porosity * (detjac * 0.5)));
#endif


/************************************************************************/
  /* SUPG                                                                 */
/************************************************************************/

  /* alpha initialisieren */
  alpha[0] = 0.0;
  alpha_adv[0] = 0.0;
  if (MTM2_upwind_method && (v > MKleinsteZahl) && (MTM2_upwind_parameter > 0.0))
    {
      /* Berechnung von zeta */
      zeta = vt / fabs(vt);            /* reine Advektion */
//      if (d > MKleinsteZahl)
	    if (dispersion_tensor[0] > MKleinsteZahl)
        {
          /* halbe Peclet-Zahl mit ds=2.0 */
//          pec = vt / tdt;
          pec = vt / dispersion_tensor[0];
          zeta = 0.0;                  /* reine Diffusion */
          if (fabs(pec) > 1.0)
            zeta = vt / fabs(vt) * (fabs(pec) - 1.0) / fabs(pec);
        }
      /* Upwind-Faktor */
      if (MTM2_upwind_method == 1)
        {
          /* Fully upwinding */
          alpha[0] = MTM2_upwind_parameter * zeta;
          alpha_adv[0] = MTM2_upwind_parameter * zeta;
        }
      if (MTM2_upwind_method == 2)
        {
          /* Advection upwinding */
          alpha_adv[0] = MTM2_upwind_parameter * zeta;
        }

#ifdef TESTSUPG
      DisplayDoubleVector(alpha, 1, "alpha=");
#endif
    }
/************************************************************************/
  /* Elementmatrizen berechnen                                            */
/************************************************************************/


  if (MTM2_mass_lumping_method == 0)
    {
      /* Massenmatrix mit SUPG ohne Zeitanteile */
      vorfk = detjac * Mdrittel * porosity;
      mass[0] = (2.0 - 1.5 * alpha[0]) * vorfk;
      mass[1] = (1.0 - 1.5 * alpha[0]) * vorfk;
      mass[2] = (1.0 + 1.5 * alpha[0]) * vorfk;
      mass[3] = (2.0 + 1.5 * alpha[0]) * vorfk;
    }
  else if (MTM2_mass_lumping_method == 1)
    {
		//SB:tod mass lumping aus htm übernehmen ?
      mass[0] = mass[3] = 0.5 * ElGetElementVolume(index) * porosity;
    }

#ifdef TESTCEL_MTM2
  MZeigMat(mass, 2, 2, "CalcEle1D_MTM2: mass capacitance matrix");
#endif
  
/* Dispersionsmatrix ohne Zeitanteile */
//  vorfk =  (detjac * 0.5) * tdt * porosity;
  vorfk =  (detjac * 0.5) * dispersion_tensor[0] * porosity;
  disp[0] = disp[3] = vorfk;
  disp[1] = disp[2] = -vorfk;

#ifdef TESTCEL_MTM2
  MZeigMat(disp, 2, 2, "CalcEle1D_MTM2: mass diffusion-dispersion matrix");
  DisplayMsg("d = "); DisplayDouble(dispersion_tensor[0],0,0); DisplayMsg(", v = "); DisplayDouble(v,0,0);
  DisplayMsg(", art_diff = "); DisplayDouble(ElGetArtDiff(index),0,0);
  DisplayMsg(" tdt = "); DisplayDouble(dispersion_tensor[0],0,0); DisplayMsgLn("");
#endif

  /* Advektive Matrix mit SUPG ohne Zeitanteile */
  vorfk = detjac * 0.5 * vt * porosity;
  adv[0] = vorfk * (-1.0 + alpha_adv[0]);
  adv[1] = vorfk * (1.0 - alpha_adv[0]);
  adv[2] = vorfk * (-1.0 - alpha_adv[0]);
  adv[3] = vorfk * (1.0 + alpha_adv[0]);

#ifdef TESTCEL_MTM2
  MZeigMat(adv, 2, 2, "CalcEle1D_MTM2: advection matrix");
#endif


  /* Zerfallsmatrix ohne Zeitanteile */ //SB: decay -Matrix kann auch für andere Speicherterme verwendet werden */
    vorfk = detjac * Mdrittel * porosity;
    decay[0] =  2. * vorfk;
    decay[1] =  1. * vorfk;
    decay[3] =  2. * vorfk;
    decay[2] =  1. * vorfk;

#ifdef TESTCEL_MTM2
    MZeigMat(decay, 2, 2, "CalcEle1D_MTM2: decay matrix");
#endif

/* Int(phi*(dn_dt*c))dV */
/* vorfk = 1/6 * A * L * dn_dt;  das 1/dt wird in der cgs_MTM2.c beruecksichtigt */
/* detjac= AL/2 */
/* dn_dt = (MTM2GetFluidContent(phase,index,0.,0.,0.,1.) - MTM2GetFluidContent(phase,index,0.,0.,0.,0.)) / dt */


	if(GetRFProcessNumPhases()>1)
    {
//      vorfk =  detjac * Mdrittel * (MTM2GetFluidContent(phase, index, 0., 0., 0., 1.) - MTM2GetFluidContent(phase, index, 0., 0., 0., 0.));
 //MX     vorfk =  detjac * Mdrittel *  m_mat_mp->Porosity(index,gp,theta) *(MTM2GetSaturation(phase, index, 0., 0., 0., 1.) - MTM2GetSaturation(phase, index, 0., 0., 0., 0.));
	  vorfk =  detjac * Mdrittel *  m_mat_mp->Porosity(index,gp,theta) *(PCSGetELEValue(index,gp,1.,"SATURATION2") - PCSGetELEValue(index,gp,0.,"SATURATION2"));
      content[0] = (2.0 - 1.5 * alpha[0]) * vorfk;
      content[1] = (1.0 - 1.5 * alpha[0]) * vorfk;
      content[2] = (1.0 + 1.5 * alpha[0]) * vorfk;
      content[3] = (2.0 + 1.5 * alpha[0]) * vorfk;
    }
  /* Einspeichern der ermittelten Matrizen */
      MTM2SetElementMassMatrixNew(index,  mtm_pcs_number, mass);
      MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
      MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	  MTM2SetElementDecayMatrixNew(index, mtm_pcs_number, decay);
	if(GetRFProcessNumPhases()>1)
        MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);

}

/**************************************************************************
 ROCKFLOW - Funktion: CalcEle2D_MTM2

 Aufgabe:
   Berechnet die 2-D Elementmatrizen fuer Stoff- und Waermetransport
   (siehe Manual Chapter A2.5 und A2.8)

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   07/1995     Hans Herrmann      Erste Version
   11/1995     msr                ueberarbeitet
   03/1996     cb                 Porositaet
   05/1996     cb                 zweite Version
   27.06.1997  R.Kaiser           Anpassung an die Funktion
                                  CalcElementJacobiMatrix (femlib.c)
   10/1997  O.Kolditz          Implementierung des Waermetransportmodells
   12/1997  O.Kolditz          Filtergeschwindigkeiten velovec
                                  Abstandsgeschwindigkeiten v,vt,vg
   06/1998  C.Thorenz          Lagrangeverfahren fuer 2D
   06/1999  CT  Globalvariable halbwertszeit_1 entfernt
   10/1999  CT  Zugriff auf Saettigung wiederhergestellt (!)
    2/2000  CT  Phasennummer eingefuehrt
    7/2000  CT  Upwinding 2d/3d auf r/s/t umgestellt. Aus Gaussschleife entfernt
    8/2000  CT  Zugriff auf Elementpecletzahl
    9/2000  CT  Umbau fuer Mehrphasenmodell, ML-FEM

    05/2003   SB  Umbau MTM2: Abbau, Sorption, Porositätsänderung
	10/2004   SB  Anpassen an GS4

**************************************************************************/
void CalcEle2D_MTM2(long index, long component, CRFProcess *m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i, j, k;
  static long ind;
  static double r, s;           /* Gausspunkte */
  /* Elementgeometriedaten */
  static double detjac, invjac[4], jacobi[4];
  /* Knotendaten */
  static double zwa[8], zwo[4];
  static double phi[4], ome[8];
  static double tdt[4]; // d[4]
  static double *mass_dispersion_tensor;
  //  static double dreh[4];
  /* Elementmatrizen */
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;
  static double *decay;
  static double zwi[16];
  /* Elementdaten */
  static int anzgp;
  //  static double art_diff;       /* ETG */
  static double alpha[3], alpha_adv[3];         /* SUPG */
  static double pec;
  static double *velovec, vg, v[2], vt[2], v_rs[2];
  static double fkt, sum;
  /* Materialdaten */
  static double area, porosity;
  static double dC;
  double gp[3], gp1[3], theta;
  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;

  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;

  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;

  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];

  theta = m_pcs->m_num->ls_theta;

#ifdef TESTSUPG
  DisplayMsg("2D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
      /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = MTM2GetElementMassMatrixNew(index,  mtm_pcs_number);
      if (!mass)
        mass = (double *) Malloc(16 * sizeof(double));

      disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);
      if (!disp)
        disp = (double *) Malloc(16 * sizeof(double));

      adv = MTM2GetElementAdvMatrixNew(index,  mtm_pcs_number);
      if (!adv)
        adv = (double *) Malloc(16 * sizeof(double));

	  if(GetRFProcessNumPhases()>1){
      content = MTM2GetElementFluidContentMatrixNew(index,  mtm_pcs_number);
      if (!content)
        content = (double *) Malloc(16 * sizeof(double));
	  }

      decay = MTM2GetElementDecayMatrixNew(index, mtm_pcs_number);
      if (!decay)
        decay = (double *) Malloc(16 * sizeof(double));


  /* Initialisieren */
  MNulleMat(mass, 4, 4);
  MNulleMat(disp, 4, 4);
  MNulleMat(adv, 4, 4);
  if(GetRFProcessNumPhases()>1)
	  MNulleMat(content, 4, 4);
  MNulleMat(decay, 4, 4);

  area = m_mat_mp->geo_area;

  porosity = m_mat_mp->Porosity(index,gp,theta);
  if (m_mat_mp->porosity_model == 10)
	  porosity = m_mat_mp->PorosityVolumetricChemicalReaction(index); //MX

  if(GetRFProcessNumPhases()>1){
//MX      int  iphase = 2;
//MX	  porosity *= MTM2GetSaturation(iphase, index, 0., 0., 0., MTM2_time_collocation_global);
	  porosity *= PCSGetELEValue(index,gp,theta,"SATURATION2");
  }

// if (MTM2_new_kernel)
//SB:3912    pec = MTM2GetElementPecletNumNew(index, phase, component);
// else
//  pec = MTM2GetElementPecletNum(index);

  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);

  /* SUPG */
  MNulleVec(alpha, 2);
  MNulleVec(alpha_adv, 2);

  if (MTM2_upwind_method && (MTM2_upwind_parameter > 0.0))
    {
      CalcVeloXDrst(phase, index, MTM2_time_collocation_upwinding, 0., 0., 0., v);

      /* Geschwindigkeitstransformation: a,b -> r,s */
      Calc2DElementJacobiMatrix(index, 0., 0., invjac, &detjac);
      MKopierVec(invjac, jacobi, 4);
      M2InvertiereUndTransponiere(jacobi); /* MK_bitte_pruefen!!!: Transponierte der */ /* Jacobi-Matrix */
      MMultMatVec(jacobi, 2, 2, v, 2, v_rs, 2);



#ifdef TESTCEL_MTM2
  DisplayMsg(" Element No: "); DisplayLong((long) index); DisplayMsgLn(": ");
//  DisplayMsg(" mass_dispersion_longitudinal: "); DisplayDouble(mass_dispersion_longitudinal,0,0);  DisplayMsgLn(": ");
  DisplayMsg(" velovec: "); DisplayDouble(velovec[0],0,0);  DisplayMsgLn(": ");
#endif


      /* Abstandsgeschwindigkeit im Gauss-Punkt */
      for (k = 0; k < 2; k++)
        v[k] = v_rs[k] / porosity;

      vg = MBtrgVec(v, 2);

      if (vg > MKleinsteZahl)
        {
          if (fabs(pec) > 2.0)
            {
              /* Upwind-Vektor */
              for (k = 0; k < 2; k++)
                alpha[k] = alpha_adv[k] = v[k] / vg * MTM2_upwind_parameter * (fabs(pec) - 1.0) / fabs(pec);

#ifdef TESTSUPG
              DisplayDoubleVector(alpha, 2, "alpha");
#endif

              if (MTM2_upwind_method == 2)
                {
                  /* Advection upwinding */
                  MNulleVec(alpha, 2);
                }
            }
        }
    }
  ind = -1;
  /* Schleife ueber GaussPunkte */
  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      {
        r = MXPGaussPkt(anzgp, i);
        s = MXPGaussPkt(anzgp, j);
        gp1[0] = r; gp1[1] = s; gp1[2] = 0.0; 
        /* Allgemeines */
        /* J^-1 */
        Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
        /* Faktor */
        fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;

        /* Veraenderliche Porositaet? */
		if(GetRFProcessNumPhases()>1)
//MX		    porosity = m_mat_mp->Porosity(index,gp,theta) * MTM2GetSaturation(phase+1, index, r, s, 0., MTM2_time_collocation_global);
         porosity = m_mat_mp->Porosity(index,gp,theta) * PCSGetELEValue(index,gp1,theta,"SATURATION2");
         sum = fkt * porosity;

        /* Abstandsgeschwindigkeit im Gauss-Punkt */
        for (k = 0; k < 2; k++)
          v[k] = velovec[++ind] / porosity;
        vg = MBtrgVec(v, 2);

        /* hydrodynamischen Dispersionstensor berechnen */
		mass_dispersion_tensor = m_mat_mp->MassDispersionTensor(index,gp,theta,component);
        /* tdt = J^-1 * D * (J^-1)T */
        MMultMatMat(invjac, 2, 2, mass_dispersion_tensor, 2, 2, zwa, 2, 2);
        MTranspoMat(invjac, 2, 2, zwi);
        MMultMatMat(zwa, 2, 2, zwi, 2, 2, tdt, 2, 2);
        /* vt = v * (J^-1)T */
        MMultVecMat(v, 2, zwi, 2, 2, vt, 2);

/********************************************************************/
/* Massenmatrix                                                     */
/********************************************************************/
        /* Wichtungsfunktion: phi */
        /* Fallunterscheidung fuer Standard-Galerkin oder SUPG-Verfahren */
        if (MTM2_upwind_method == 1)
          {
            /* Fully-Upwinding: Wichtungsfunktion: phi - SUPG */
            MPhi2D_SUPG(phi, r, s, alpha);
          }
        /* Wichtungsfunktion: phi */
        else
          {
            MPhi2D(phi, r, s);
          }

        /* Ansatzfunktion: omega */
        MOmega2D(zwo, r, s);

        if (MTM2_mass_lumping_method == 0)
          {
            /* Normale FEM */
            /* phi * omega */
            MMultVecVec(phi, 4, zwo, 4, zwi, 4, 4);

            /* Stofftransport - sorption matrix */
            for (k = 0; k < 16; k++)
              mass[k] += (zwi[k] * sum);
          }
        else if (MTM2_mass_lumping_method == 1)
          {
            /* Mass-Lumping */
            for (k = 0; k < 4; k++)
              mass[k * 5] += zwo[k] * sum;
          }
/********************************************************************/
/*  Fluidgehaltsmatrix                                              */
/********************************************************************/
	    if(GetRFProcessNumPhases()>1)
          {
     //       dC =  m_mat_mp->Porosity(index,gp,theta) *(MTM2GetSaturation(phase, index+1, 0., 0., 0., 1.) - MTM2GetSaturation(phase+1, index, 0., 0., 0., 0.));
            dC =  m_mat_mp->Porosity(index,gp,theta) *(PCSGetELEValue(index,gp1,1.,"SATURATION2") - PCSGetELEValue(index,gp1,0.,"SATURATION2"));
            /* Stofftransport - sorption matrix */
            for (k = 0; k < 16; k++)
              content[k] += zwi[k] * fkt * dC;
          }
/********************************************************************/
/* Dispersionsmatrix                                                */
/********************************************************************/
        /* grad(phi)T */
        MGradPhi2D(zwa, r, s);         /* 2 Zeilen 4 Spalten */
        MTranspoMat(zwa, 2, 4, zwi);   /* jetzt: 4 Zeilen 2 Spalten */
        /* grad(phi)T * tdt */
        MMultMatMat(zwi, 4, 2, tdt, 2, 2, zwa, 4, 2);
        /* grad(omega) */
        MGradOmega2D(ome, r, s);       /* 2 Zeilen 4 Spalten */
        /* grad(phi)T * tdt * grad(omega) */
        MMultMatMat(zwa, 4, 2, ome, 2, 4, zwi, 4, 4);

        /* Stofftransport - mass diffusion-dispersion matrix */
        for (k = 0; k < 16; k++)
          disp[k] += (zwi[k] * sum);

/********************************************************************/
/* Advektionsmatrix                                                 */
/********************************************************************/
        if (MTM2_upwind_method == 2)
          {
            /* Advection-Upwinding: Wichtungsfunktion: phi - SUPG */
            MPhi2D_SUPG(phi, r, s, alpha_adv);
          }
        /* phi * vt */
        MMultVecVec(phi, 4, vt, 2, zwa, 4, 2);
        /* phi * vt * grad(omega) */
        MMultMatMat(zwa, 4, 2, ome, 2, 4, zwi, 4, 4);

        /* Stofftransport - mass diffusion-dispersion matrix */
        for (k = 0; k < 16; k++)
          adv[k] += (zwi[k] * sum);

/********************************************************************/
/* SB neu */
/* Decaymatrix                                                 */
/********************************************************************/
       if (MTM2_mass_lumping_method == 0)
          {
            /* Normale FEM */
            /* phi * omega */
            MMultVecVec(phi, 4, zwo, 4, zwi, 4, 4);

            /* Stofftransport - sorption matrix */
            for (k = 0; k < 16; k++)
              decay[k] += (zwi[k] * sum);
          }
        else if (MTM2_mass_lumping_method == 1)
          {
            /* Mass-Lumping */
            for (k = 0; k < 4; k++)
              decay[k * 5] += zwo[k] * sum;
          }
/*  ende decay matrix */
      }                                /* Ende der Schleife ueber GaussPunkte */

  /* Stofftransport */
  for (i = 0; i < 16; i++)
    {
      mass[i] *= area /* * retard */;
	  if(GetRFProcessNumPhases()>1)
        content[i] *= area /* * retard */;
      disp[i] *= area;
      adv[i] *= area;
	  decay[i] *=area;
    }

  /* Wenn der advektive Anteil per Lagrangeverfahren
     behandelt wird, muss er hier zu Null gesetzt werden */
  if (MTM2_use_lagrange_for_2D)
    for (i = 0; i < 16; i++)
      adv[i] = 0.;

  /* Einspeichern der ermittelten Matrizen */
      MTM2SetElementMassMatrixNew(index, mtm_pcs_number, mass);
      MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
      MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	  if(GetRFProcessNumPhases()>1)
        MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);
	  MTM2SetElementDecayMatrixNew(index, mtm_pcs_number, decay);


#ifdef TESTCEL_MTM2
  MZeigMat(mass, 4, 4, "CalcEle2D_MTM2: mass capacitance matrix");
  MZeigMat(disp, 4, 4, "CalcEle2D_MTM2: mass diffusion-dispersion matrix");
  MZeigMat(adv, 4, 4, "CalcEle2D_MTM2: mass advection matrix");
  MZeigMat(decay, 4, 4, "CalcEle2D_MTM2: mass decay matrix");
  if(GetRFProcessNumPhases()>1)
	MZeigMat(content, 4, 4, "CalcEle2D_MTM2: fluid content matrix");
#endif
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle3D_MTM2

 Aufgabe:
   Berechnet die 3-D Elementmatrizen fuer Stoff- und Waermetransport
   (siehe Manual Chapter A2.5 und A2.8)

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   04/1996     cb         Erste Version
   05/1996     cb         zweite Version
   27.06.1997  R.Kaiser   Anpassung an die Funktion
                          CalcElementJacobiMatrix (femlib.c)
   10/1997  O.Kolditz  Implementierung des Waermetransportmodells
   12/1997  O.Kolditz  Filtergeschwindigkeiten velovec
                       Abstandsgeschwindigkeiten v,vt,vg
   10/1998  C.Thorenz/O.Kolditz Korrektur 3D-Elementmatrizen
   06/1999  CT  Globalvariable halbwertszeit_1 entfernt
   10/1999  CT  Zugriff auf Saettigung wiederhergestellt (!)
    2/2000  CT  Phasennummer eingefuehrt
    7/2000  CT  Upwinding 2d/3d auf r/s/t umgestellt. Aus Gaussschleife entfernt
    8/2000  CT  Zugriff auf Elementpecletzahl
    9/2000  CT  Umbau fuer Mehrphasenmodell, ML-FEM

	5/2003  SB  Umbau MTM2

**************************************************************************/
void CalcEle3D_MTM2(long index, long component, CRFProcess *m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */

  static long i, j, k, l;
  long inde;
  static double r, s, t;
  /* Elementgeometriedaten */
  static double invjac[9], jacobi[9], detjac;
  static double phi[8], ome[24];
  /* Elementmatrizen */
  static double *mass;          /* Massenmatrix */
  static double *content;       /* Aenderung des Feuchtegehalts */
  static double *disp;          /* Dispersionsmatrix */
  static double *adv;           /* Advektionsmatrix */
  static double *decay;           /* Decaymatrix */
  /* Elementdaten */
  static int anzgp;
  static double alpha[3], alpha_adv[3], pec;    /* SUPG */
//  static double art_diff;       /* ETG */
  static double *velovec, v[3], vt[3], v_rst[3], vg;
  static double  tdt[9]; // d[9]
//  static double retard;
  static double fkt, sum;
  /* Materialdaten */
  static double porosity, dC;
//  static double mass_dispersion_longitudinal=1., mass_dispersion_transverse=1., molecular_diffusion;
  /* Hilfsfelder */
  static double zwi[64];
  static double zwa[24], zwo[9];
  double * mass_dispersion_tensor;
  double gp[3], theta;
  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;

  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;

  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;

  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];

  theta = m_pcs->m_num->ls_theta;

#ifdef TESTSUPG
  DisplayMsg("3D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
      mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);
      if (!mass)
        mass = (double *) Malloc(64 * sizeof(double));

      disp = MTM2GetElementDispMatrixNew(index,  mtm_pcs_number);
      if (!disp)
        disp = (double *) Malloc(64 * sizeof(double));

      adv = MTM2GetElementAdvMatrixNew(index, mtm_pcs_number);
      if (!adv)
        adv = (double *) Malloc(64 * sizeof(double));

	  if(GetRFProcessNumPhases()>1){
      content = MTM2GetElementFluidContentMatrixNew(index, mtm_pcs_number);
      if (!content)
        content = (double *) Malloc(64 * sizeof(double));
	  }

	  decay = MTM2GetElementDecayMatrixNew(index,  mtm_pcs_number);
      if (!decay)
        decay = (double *) Malloc(64 * sizeof(double));


  /* Initialisieren */
  MNulleMat(mass, 8, 8);
  MNulleMat(disp, 8, 8);
  MNulleMat(adv, 8, 8);
  MNulleMat(decay, 8, 8);
    if(GetRFProcessNumPhases()>1)
    MNulleMat(content, 8, 8);

  /* Fuer teilgesaettigte Verhaeltnisse: */
  porosity = m_mat_mp->Porosity(index,gp,theta);
  if(GetRFProcessNumPhases()>1)
    porosity *= MTM2GetSaturation(phase, index, 0., 0., 0., MTM2_time_collocation_global);

  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);

  /* SUPG */
  MNulleVec(alpha, 3);
  MNulleVec(alpha_adv, 3);

  if (MTM2_upwind_method && (MTM2_upwind_parameter > 0.0))
    {
      CalcVeloXDrst(phase, index, MTM2_time_collocation_upwinding, 0., 0., 0., v);

      /* Geschwindigkeitstransformation: x,y,z -> r,s,t */
      Calc3DElementJacobiMatrix(index, 0., 0., 0., invjac, &detjac);
      MKopierVec(invjac, jacobi, 9);
      M3Invertiere(jacobi);            /* zurueck zur Jacobi-Matrix */
      MMultMatVec(jacobi, 3, 3, v, 3, v_rst, 3);

      /* Abstandsgeschwindigkeit  */
      for (l = 0; l < 3; l++)
        v[l] = v_rst[l] / porosity;

      vg = MBtrgVec(v, 3);

      if (vg > MKleinsteZahl)
        {
          if (fabs(pec) > 1.0)
            {
              /* Upwind-Vektor */
              for (k = 0; k < 3; k++)
                alpha[k] = alpha_adv[k] = v[k] / vg * MTM2_upwind_parameter * (fabs(pec) - 1.0) / pec;
              if (MTM2_upwind_method == 2)
                /* Advection upwinding */
                MNulleVec(alpha, 3);

#ifdef TESTSUPG
              DisplayDoubleVector(alpha, 3, "alpha");
#endif
            }
        }
    }
/***************************************************************************/
  /* Schleife ueber Gauss-Punkte                                             */
/***************************************************************************/
  inde = -1;
  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)
        {
//		 DisplayMsg(" i, j, k: "); DisplayLong(i); DisplayMsg(", ");DisplayLong(j); DisplayMsg(", ");DisplayLong(k); DisplayMsg(", "); DisplayMsgLn(" ");
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          t = MXPGaussPkt(anzgp, k);

          /* Faktoren */
          Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
          fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * detjac;

          /* Veraenderliche Porositaet? im Gauss Punkt */
          if(GetRFProcessNumPhases()>1)
		    porosity = m_mat_mp->Porosity(index,gp,theta) * MTM2GetSaturation(phase, index, r, s, 0., MTM2_time_collocation_global);

          sum = fkt * porosity;

          /* Abstandsgeschwindigkeit im Gauss-Punkt */
          for (l = 0; l < 3; l++)
            v[l] = velovec[++inde] / porosity;
          vg = MBtrgVec(v, 3);
//		  DisplayMsg(" v: "); for(l=0;l<3;l++){DisplayDouble(v[l],0,0); DisplayMsg(" ");} DisplayMsgLn(" ");

		  /* hydrodynamischen Dispersionstensor berechnen */
	      mass_dispersion_tensor = m_mat_mp->MassDispersionTensor(index,gp,theta,component);

          /* vt = v * J^-1 */
          MMultVecMat(v, 3, invjac, 3, 3, vt, 3);
          /* tdt = (J^-1)T * D * J^-1 */
		  MMultMatMat(mass_dispersion_tensor, 3, 3, invjac, 3, 3, zwa, 3, 3);
          MTranspoMat(invjac, 3, 3, zwi);
          MMultMatMat(zwi, 3, 3, zwa, 3, 3, tdt, 3, 3);



/************************************************************************/
/* Massenmatrix                                                         */
/************************************************************************/
          /* phi * omega */
          MPhi3D_SUPG(phi, r, s, t, alpha);
          MOmega3D(zwo, r, s, t);

          if (MTM2_mass_lumping_method == 0)
            {
              /* Normale FEM */
              /* phi * omega */
              MMultVecVec(phi, 8, zwo, 8, zwi, 8, 8);
              /* Stofftransport - sorption matrix */
              for (l = 0; l < 64; l++)
                mass[l] += (zwi[l] * sum);
            }
          else if (MTM2_mass_lumping_method == 1)
            {
              /* Wenn die Speicherung per ML-FEM
                 behandelt wird, wird nur die Diagonale gesetzt */
              for (l = 0; l < 8; l++)
                mass[l * 9] += zwo[l] * sum;
            }

/********************************************************************/
/*  Fluidgehaltsmatrix                                              */
/********************************************************************/
		  if(GetRFProcessNumPhases()>1)
            {
//              dC = (MTM2GetFluidContent(phase, index, r, s, t, 1.) - MTM2GetFluidContent(phase, index, r, s, t, 0.));
			  dC =  m_mat_mp->Porosity(index,gp,theta) *(MTM2GetSaturation(phase, index, 0., 0., 0., 1.) - MTM2GetSaturation(phase, index, 0., 0., 0., 0.));
              for (l = 0; l < 64; l++)
                content[l] += zwi[l] * fkt * dC;
            }
/************************************************************************/
          /* Dispersionsmatrix                                                    */
/************************************************************************/
          /* grad(phi)T */
          MGradPhi3D(zwa, r, s, t);    /* 3 Zeilen 8 Spalten */
          MTranspoMat(zwa, 3, 8, zwi); /* jetzt: 8 Zeilen 3 Spalten */
          /* grad(phi)T * tDt */
          MMultMatMat(zwi, 8, 3, tdt, 3, 3, zwa, 8, 3);
          /* grad(omega) */
          MGradOmega3D(ome, r, s, t);  /* 3 Zeilen 8 Spalten */
          /* grad(phi)T * tDt * grad(omega) */
          MMultMatMat(zwa, 8, 3, ome, 3, 8, zwi, 8, 8);

          /* Stofftransport - sorption matrix */
          for (l = 0; l < 64; l++)
            disp[l] += (zwi[l] * sum);

/************************************************************************/
          /* Advektionsmatrix                                                     */
/************************************************************************/
          if (MTM2_upwind_method == 2)
            {
              /* Advection-Upwinding: Wichtungsfunktion: phi - SUPG */
              MPhi3D_SUPG(phi, r, s, t, alpha_adv);
            }
          /* phi * vt */
          MMultVecVec(phi, 8, vt, 3, zwa, 8, 3);

          /* phi * vt * grad(omega) */
          MMultMatMat(zwa, 8, 3, ome, 3, 8, zwi, 8, 8);

          /* Stofftransport - mass advection matrix */
          for (l = 0; l < 64; l++)
            adv[l] += (zwi[l] * sum);


/************************************************************************/
/* Decaymatrix                                                         */
/************************************************************************/
        if (MTM2_mass_lumping_method == 0)
            {
              /* Normale FEM */
              /* phi * omega */
              MMultVecVec(phi, 8, zwo, 8, zwi, 8, 8);
              /* Stofftransport - sorption matrix */
              for (l = 0; l < 64; l++)
                decay[l] += (zwi[l] * sum);
            }
          else if (MTM2_mass_lumping_method == 1)
            {
              /* Wenn die Speicherung per ML-FEM
                 behandelt wird, wird nur die Diagonale gesetzt */
              for (l = 0; l < 8; l++)
                decay[l * 9] += zwo[l] * sum;
            }

/* end of decay-matrix */

        }                              /* Ende der Schleife ueber GaussPunkte */


  /* Stofftransport */
  for (l = 0; l < 64; l++)
    {
      mass[l] *= 1.0 ; /* retard */
      if(GetRFProcessNumPhases()>1)
        content[i] *= 1.0;  /* retard */
    }
   
  /* Einspeichern der ermittelten Matrizen */
      MTM2SetElementMassMatrixNew(index, mtm_pcs_number, mass);
      MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
      MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	  if(GetRFProcessNumPhases()>1)
        MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);
	  MTM2SetElementDecayMatrixNew(index, mtm_pcs_number, decay);


#ifdef TESTCEL_MTM2
if(index<5){
  DisplayMsgLn(" "); DisplayMsg(" CalcEle3d_MTM2 for Element Number: "); DisplayLong(index); DisplayMsgLn(" ");
  MZeigMat(mass, 8, 8, "CalcEle3D_MTM2: Mass matrix");
  MZeigMat(disp, 8, 8, "CalcEle3D_MTM2: Dispersion Matrix");
  MZeigMat(adv, 8, 8, "CalcEle3D_MTM2: Advection matrix");
  MZeigMat(decay, 8, 8, "CalcEle3D_MTM2: Decay matrix");
  if(content)
	MZeigMat(content, 8, 8, "CalcEle3D_MTM2: FluidContent matrix");
  DisplayMsg(" velovec: "); DisplayDouble(velovec[0],0,0); DisplayMsg(" ,");DisplayDouble(velovec[1],0,0); DisplayMsg(" ,");DisplayDouble(velovec[2],0,0); DisplayMsgLn(" .");
}
#endif
}



 /**************************************************************************
ROCKFLOW - Function: MTM2CalcElementMatrices2DTriangle
Task:
  Linear triangular element for mass transport
Programming:
  01/2003 OK Implementation
**************************************************************************/
void MTM2CalcElementMatrices2DTriangle(long index, long component, CRFProcess *m_pcs)
{
  /* Laufvariablen */
  static long i;
  int k;
  /* Geometry */
  static double x[3],y[3],z[3];
  //static double volume;
  static int nn=3;
  static int nn2=9;
  static double fac_geo;
  static long *element_nodes;
  /* NUM - Numerics */
  static double alpha0;
  static double art_diff;
  static double zwa[4];
  //static double dreh[4],zwi[4],zwa[4];
  static double alpha[3],zeta[2],pec[2];
  /* MAT - Materials */
   /* MAT-FP */
//  int phase=0;
   /* MAT-CP */
//  int component=0;
  static double molecular_diffusion;
  static double mass_dispersion_longitudinal,mass_dispersion_transverse;
  //  static double lambda_c=0.0;
   /* MAT-MP */
  static double d[4];
  static double *velovec, vg, v[3];
  static double area, porosity,exchange_area;
  static double fac_mat;
  static double k_xx,k_yy,k_zz,q_x,q_y;
//  static double dissolution_rate,solubility_coefficient;
   /* MAT-SP */
  //  static double density_rock;
  /* Element matrices */
  static double *mass;
  static double *disp;
  static double *adv;
  static double *content;
  static double *decay;  //SB:todo
  double gp[3], theta;
  double *mass_dispersion_tensor;
  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;
  //----------------------------------------------------------------------
  // NUM properties
  theta = m_pcs->m_num->ls_theta;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  //----------------------------------------------------------------------
  // MFP fluid properties
  int mtm_no_phases = (int)mfp_vector.size(); //OK
  //----------------------------------------------------------------------
  // MCP component properties
  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  /* ------------------------------------------------------------------ */
  /* Speicher-Modelle fuer Element-Matrizen */
  if (memory_opt == 0) { /* keine Speicheroptimierung */
    mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);
    if (!mass)
      mass = (double *) Malloc(nn2 * sizeof(double));
    disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);
    if (!disp)
      disp = (double *) Malloc(nn2 * sizeof(double));
    adv = MTM2GetElementAdvMatrixNew(index,mtm_pcs_number);
    if (!adv)
      adv = (double *) Malloc(nn2 * sizeof(double));
    decay = MTM2GetElementDecayMatrixNew(index,  mtm_pcs_number);
    if (!decay)
      decay = (double *) Malloc(nn2 * sizeof(double));
    if(mtm_no_phases>1){ //OK
  	  content = MTM2GetElementFluidContentMatrixNew(index,mtm_pcs_number);
      if (!content)
         content = (double *) Malloc(nn2 * sizeof(double));
    }
  }
  MNulleMat(mass,nn,nn);
  MNulleMat(disp,nn,nn);
  MNulleMat(adv,nn,nn);
  MNulleMat(decay,nn, nn);
  if(mtm_no_phases>1)
    MNulleMat(content, nn, nn);
  /* ------------------------------------------------------------------ */
  /* NUM - numerical properties*/
  alpha0 = m_pcs->m_num->ele_upwinding; //OK
  /* ------------------------------------------------------------------ */
  /* GEO - geometry */
   /* Element-Koordinaten im lokalen System (x',y')==(a,b) */
  //Calc2DElementCoordinatesTriangle(index,x,y);
  element_nodes = ElGetElementNodes(index);
  for (i=0;i<nn;i++) {
    x[i]=GetNodeX(element_nodes[i]);
    y[i]=GetNodeY(element_nodes[i]);
    z[i]=GetNodeZ(element_nodes[i]); //3D triangle
  }
  /* Element-Volumen: Flaeche * Dicke */    
  exchange_area = m_mat_mp->geo_area;
  area = fabs(ElGetElementVolume(index));
  porosity = m_mat_mp->Porosity(index,gp,theta);
  if(GetRFProcessNumPhases()>1)
    porosity *= MTM2GetSaturation(phase, index, 0., 0., 0., MTM2_time_collocation_global);
  //------------------------------------------------------------------
  // 3-D triangle
   // ADV = B * v_3D * grad_3D N * int N' dA
  double v_x,v_y,v_z;
  v_x = v[0]; v_y = v[1]; v_z = v[2];
   // grad_3D N = 3D_J_2D * grad_2D N'
  double dircos[6];
  double r[3],s[3];
  Calc2DElementCoordinatesTriangle(index,r,s,dircos);
  double cosxr = dircos[0];
  double cosxs = dircos[1];
  double cosyr = dircos[2];
  double cosys = dircos[3];
  double coszr = dircos[4];
  double coszs = dircos[5];
  double r1,r2,r3,s1,s2,s3;
  r[0] = r1 = x[0]*cosxr + y[0]*cosyr + z[0]*coszr;
  r[1] = r2 = x[1]*cosxr + y[1]*cosyr + z[1]*coszr;
  r[2] = r3 = x[2]*cosxr + y[2]*cosyr + z[2]*coszr;
  s[0] = s1 = x[0]*cosxs + y[0]*cosys + z[0]*coszs;
  s[1] = s2 = x[1]*cosxs + y[1]*cosys + z[1]*coszs;
  s[2] = s3 = x[2]*cosxs + y[2]*cosys + z[2]*coszs;
   //
  double dN1_dr,dN2_dr,dN3_dr,dN1_ds,dN2_ds,dN3_ds;
  dN1_dr = (s2-s3); // /(2.*area);
  dN2_dr = (s3-s1); // /(2.*area);
  dN3_dr = (s1-s2); // /(2.*area);
  dN1_ds = (r3-r2); // /(2.*area);
  dN2_ds = (r1-r3); // /(2.*area);
  dN3_ds = (r2-r1); // /(2.*area);
   //
  double dN1_dx,dN1_dy,dN1_dz,dN2_dx,dN2_dy,dN2_dz,dN3_dx,dN3_dy,dN3_dz;
  dN1_dx = dN1_dr*cosxr + dN1_ds*cosxs;
  dN1_dy = dN1_dr*cosyr + dN1_ds*cosys;
  dN1_dz = dN1_dr*coszr + dN1_ds*coszs;
  dN2_dx = dN2_dr*cosxr + dN2_ds*cosxs;
  dN2_dy = dN2_dr*cosyr + dN2_ds*cosys;
  dN2_dz = dN2_dr*coszr + dN2_ds*coszs;
  dN3_dx = dN3_dr*cosxr + dN3_ds*cosxs;
  dN3_dy = dN3_dr*cosyr + dN3_ds*cosys;
  dN3_dz = dN3_dr*coszr + dN3_ds*coszs;
  /* ------------------------------------------------------------------ */
  /* MAT - materials */
   /* MAT-CP */
  mass_dispersion_longitudinal = m_mat_mp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mat_mp->mass_dispersion_transverse;
  /* Diffusion coefficient */
  molecular_diffusion = m_cp->CalcDiffusionCoefficientCP(index);   
  molecular_diffusion *= m_mat_mp->TortuosityFunction(index, gp, theta);
  /*--------------------------------------------------------------------------*/
  /* pore velocity in Gauss-point */
  velovec = ElGetVelocity(index);
  v[0] = velovec[0]/porosity;
  v[1] = velovec[1]/porosity;
  v[2] = velovec[2]/porosity; //OK
  vg = MBtrgVec(v,3);
  /*--------------------------------------------------------------------------*/
  /* Dispersionstensor in stromlinienorientierten Koordinaten */
  art_diff = ElGetArtDiff(index);
/* now calculated in MassDispersionTensor();
  d[0] = molecular_diffusion \
       + ( mass_dispersion_transverse * v[1]*v[1]) / vg \
       + ( mass_dispersion_longitudinal * v[0]*v[0]) / vg;
  d[1] = (mass_dispersion_longitudinal-mass_dispersion_transverse)*v[0]*v[1] / vg;
  d[2] = (mass_dispersion_longitudinal-mass_dispersion_transverse)*v[0]*v[1] / vg;
  d[3] = molecular_diffusion \
       + ( mass_dispersion_transverse * v[0]*v[0]) / vg \
       + ( mass_dispersion_longitudinal * v[1]*v[1]) / vg;
*/
 mass_dispersion_tensor = m_mat_mp->MassDispersionTensor(index,gp,theta,component);
 #ifdef TESTCEL_MTM2
 if(index < 10){
  DisplayMsgLn("");DisplayLong(index);DisplayMsgLn(":");
  DisplayMsg(" dispersion_longitudinal:   "); DisplayDouble(mass_dispersion_longitudinal,0,0); DisplayMsgLn(" ");
  DisplayMsg(" dispersion_transverse:   "); DisplayDouble(mass_dispersion_transverse,0,0); DisplayMsgLn(" ");
  DisplayMsg(" molecular_diffusion:   "); DisplayDouble(molecular_diffusion,0,0); DisplayMsgLn(" ");
  DisplayMsg(" v[0]:   "); DisplayDouble(v[0],0,0); DisplayMsgLn(" ");
  DisplayMsg(" v[1]:   "); DisplayDouble(v[1],0,0); DisplayMsgLn(" ");
  DisplayMsg(" vg:   "); DisplayDouble(vg,0,0); DisplayMsgLn(" ");
  DisplayMsg(" area:   "); DisplayDouble(area,0,0); DisplayMsgLn(" ");
  DisplayMsg(" exchange_area:   "); DisplayDouble(exchange_area,0,0); DisplayMsgLn(" ");
  //DisplayMsg(" d[0]:   "); DisplayDouble(d[0],0,0); DisplayMsgLn(" ");
  //DisplayMsg(" d[1]:   "); DisplayDouble(d[1],0,0); DisplayMsgLn(" ");
 }
#endif
  /*--------------------------------------------------------------------------*/
  /* NUM - SUPG */
  MNulleVec(alpha,2);
  if (vg>MKleinsteZahl && alpha0>0.0) {
    /* Berechnung von zeta */
    for (k=0;k<2;k++)
      zeta[k] = -v[k] / vg;  /* reine Advektion */
    if (d[0]>MKleinsteZahl && d[3]>MKleinsteZahl) {
      /* halbe Peclet-Zahl mit ds=2.0 */
      /* pec = vt * tdt^-1 */
      for (k=0;k<4;k++)
        zwa[k] = d[k];
      M2Invertiere(zwa);
      MMultVecMat(v,2,zwa,2,2,pec,2);
      MNulleVec(zeta,2); /* fast reine Diffusion */
      for (k=0;k<2;k++)
        if (fabs(pec[k]) > 1.0)
          zeta[k] = (fabs(pec[k]) - 1.0) / pec[k];
    }
    /* Upwind-Faktoren */
    for (k=0;k<2;k++)
      alpha[k] = alpha0 * zeta[k];
#ifdef TESTSUPG
                 DisplayDoubleVector(alpha,2,"alpha");
#endif
  }
  /*==========================================================================*/
  /* --- Massenmatrix - Heat capacitance matrix ----------------------------- */
   /* Materials */
  fac_mat = porosity;
   /* 3x3 matrix */
      mass[0] = 2.0;
      mass[1] = 1.0;
      mass[2] = 1.0;
      mass[3] = 1.0;
      mass[4] = 2.0;
      mass[5] = 1.0;
      mass[6] = 1.0;
      mass[7] = 1.0;
      mass[8] = 2.0;
       /* Volumetric element */
      fac_geo = (area)/(12.);

      for (i=0;i<nn2;i++) {
        mass[i] *= fac_mat * fac_geo;
      }
  /* ToDo - SUPG ohne Zeitanteile */
  /* --- Decay matrix ------------------------------------------------------- */
//SB:todo
  fac_mat = porosity;
   /* 3x3 matrix */
      decay[0] = 2.0;
      decay[1] = 1.0;
      decay[2] = 1.0;
      decay[3] = 1.0;
      decay[4] = 2.0;
      decay[5] = 1.0;
      decay[6] = 1.0;
      decay[7] = 1.0;
      decay[8] = 2.0;
       /* Volumetric element */
      fac_geo = (area)/(12.); //SB
      for (i=0;i<nn2;i++) {
        decay[i] *= fac_mat * fac_geo;
      }
  /* --- Dispersionsmatrix - Heat diffusion-dispersion matrix --------------- */
      /* Materials */
      fac_mat = porosity;
//SB      k_xx = d[0]; // tdt
//SB      k_yy = d[3];
	  k_xx = mass_dispersion_tensor[0];
	  k_yy = mass_dispersion_tensor[1];
      k_zz = mass_dispersion_tensor[1];  //MX, for Decovalex
#ifdef TESTCEL_MTM2
 if(index < 10){
  DisplayMsg(" k_xx:   "); DisplayDouble(mass_dispersion_tensor[0],0,0); DisplayMsgLn(" ");
  DisplayMsg(" k_yy:   "); DisplayDouble(mass_dispersion_tensor[1],0,0); DisplayMsgLn(" ");
  DisplayMsg(" k_zz:   "); DisplayDouble(mass_dispersion_tensor[1],0,0); DisplayMsgLn(" ");
 }
#endif
//k_xx=k_yy=k_zz=1e-5;
       /* 3x3 matrix */
Calc2DElementCoordinatesTriangle(index,x,y,NULL);
/*
      disp[0] = k_xx*(y[1]-y[2])*(y[1]-y[2]) + k_yy*(x[2]-x[1])*(x[2]-x[1]);
      disp[1] = k_xx*(y[1]-y[2])*(y[2]-y[0]) + k_yy*(x[2]-x[1])*(x[0]-x[2]);
      disp[2] = k_xx*(y[1]-y[2])*(y[0]-y[1]) + k_yy*(x[2]-x[1])*(x[1]-x[0]);
      disp[3] = k_xx*(y[2]-y[0])*(y[1]-y[2]) + k_yy*(x[0]-x[2])*(x[2]-x[1]);
      disp[4] = k_xx*(y[2]-y[0])*(y[2]-y[0]) + k_yy*(x[0]-x[2])*(x[0]-x[2]);
      disp[5] = k_xx*(y[2]-y[0])*(y[0]-y[1]) + k_yy*(x[0]-x[2])*(x[1]-x[0]);
      disp[6] = k_xx*(y[0]-y[1])*(y[1]-y[2]) + k_yy*(x[1]-x[0])*(x[2]-x[1]);
      disp[7] = k_xx*(y[0]-y[1])*(y[2]-y[0]) + k_yy*(x[1]-x[0])*(x[0]-x[2]);
      disp[8] = k_xx*(y[0]-y[1])*(y[0]-y[1]) + k_yy*(x[1]-x[0])*(x[1]-x[0]);
*/
      disp[0] = k_xx* dN1_dx*dN1_dx + k_yy* dN1_dy*dN1_dy + k_zz* dN1_dz*dN1_dz;
      disp[1] = k_xx* dN1_dx*dN2_dx + k_yy* dN1_dy*dN2_dy + k_zz* dN1_dz*dN2_dz;
      disp[2] = k_xx* dN1_dx*dN3_dx + k_yy* dN1_dy*dN3_dy + k_zz* dN1_dz*dN3_dz;
      disp[3] = k_xx* dN2_dx*dN1_dx + k_yy* dN2_dy*dN1_dy + k_zz* dN2_dz*dN1_dz;
      disp[4] = k_xx* dN2_dx*dN2_dx + k_yy* dN2_dy*dN2_dy + k_zz* dN2_dz*dN2_dz;
      disp[5] = k_xx* dN2_dx*dN3_dx + k_yy* dN2_dy*dN3_dy + k_zz* dN2_dz*dN3_dz;
      disp[6] = k_xx* dN3_dx*dN1_dx + k_yy* dN3_dy*dN1_dy + k_zz* dN3_dz*dN1_dz;
      disp[7] = k_xx* dN3_dx*dN2_dx + k_yy* dN3_dy*dN2_dy + k_zz* dN3_dz*dN2_dz;
      disp[8] = k_xx* dN3_dx*dN3_dx + k_yy* dN3_dy*dN3_dy + k_zz* dN3_dz*dN3_dz;

       /* Volumetric element */
      fac_geo = 1./(4.*area);
      for (i=0;i<nn2;i++) {
        disp[i] *= fac_mat * fac_geo;
      }
  /* --- Advektionsmatrix ---------------------------------------------------- */
      /* Materials */
      fac_mat = porosity;
      q_x = v[0];
      q_y = v[1];
       /* 3x3 matrix */
/*
	  adv[0] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[1] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[2] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);
      adv[3] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[4] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[5] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);
      adv[6] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[7] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[8] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);
*/
/*OK 3D triangle */

      adv[0] = (v_x*dN1_dx + v_y*dN1_dy + v_z*dN1_dz);  
      adv[1] = (v_x*dN2_dx + v_y*dN2_dy + v_z*dN2_dz);  
      adv[2] = (v_x*dN3_dx + v_y*dN3_dy + v_z*dN3_dz);  
      adv[3] = (v_x*dN1_dx + v_y*dN1_dy + v_z*dN1_dz);  
      adv[4] = (v_x*dN2_dx + v_y*dN2_dy + v_z*dN2_dz);  
      adv[5] = (v_x*dN3_dx + v_y*dN3_dy + v_z*dN3_dz);  
      adv[6] = (v_x*dN1_dx + v_y*dN1_dy + v_z*dN1_dz);  
      adv[7] = (v_x*dN2_dx + v_y*dN2_dy + v_z*dN2_dz);  
      adv[8] = (v_x*dN3_dx + v_y*dN3_dy + v_z*dN3_dz);  
/**/
       /* Volumetric element */
      fac_geo = 1./6.; //OK ? 1/3
      for (i=0;i<nn2;i++) {
        adv[i] *= fac_mat * fac_geo;
      }
  /* --- Content matrix ------------------------------------------------------- */
if(mtm_no_phases>1){ //OK
//SB:todo
  fac_mat = porosity;
   /* 3x3 matrix */
      content[0] = 2.0;
      content[1] = 1.0;
      content[2] = 1.0;
      content[3] = 1.0;
      content[4] = 2.0;
      content[5] = 1.0;
      content[6] = 1.0;
      content[7] = 1.0;
      content[8] = 2.0;
       /* Volumetric element */
      fac_geo = (area)/(12.)*1.0; //SB ??

      for (i=0;i<nn2;i++) {
        content[i] *= fac_mat * fac_geo;
      }
}
  /* ------------------------------------------------------------------------ */
  for (i=0;i<nn2;i++)
    {                                  /* Elementdicke */
      if(fabs(mass[i])>MKleinsteZahl)
        mass[i] *= (exchange_area);
      else
        mass[i] = 0.0;
      if(fabs(mass[i])>MKleinsteZahl)
        disp[i] *= (exchange_area);
      else
        disp[i] = 0.0;
      if(fabs(mass[i])>MKleinsteZahl)
        adv[i] *= (exchange_area);
      else
        adv[i] = 0.0;
    }
  /* --------------------------------------------------- */
  /* Element-Matrizen speichern oder uebergeben */
  if (memory_opt == 0) {
    MTM2SetElementMassMatrixNew(index, mtm_pcs_number, mass);
    MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
    MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	MTM2SetElementDecayMatrixNew(index,mtm_pcs_number, decay);
    if(mtm_no_phases>1){ //OK
      MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);
    }
  }

#ifdef TESTCEL_MTM2
if(index < 10){
  DisplayMsgLn("");DisplayLong(index);DisplayMsgLn(":");
  MZeigMat(mass,nn,nn,"MTM2CalcElementMatrices2DTriangle: Mass capacitance matrix");
  MZeigMat(disp,nn,nn,"MTM2CalcElementMatrices2DTriangle: Mass diffusion-dispersion matrix");
  MZeigMat(adv,nn,nn, "MTM2CalcElementMatrices2DTriangle: Mass advection matrix");
  DisplayMsg(" velovec: "); DisplayDouble(velovec[0],0,0); DisplayMsg(" ,");DisplayDouble(velovec[1],0,0); DisplayMsg(" ,");DisplayDouble(velovec[2],0,0); DisplayMsgLn(" .");
}
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle3DPrism_MTM2_num
                                                                          */
/* Aufgabe:
   Elementmatrizen fuer lineares Dreiecksprismen-Element
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2003   MB     Erste Version aufgebaut auf CalcEle2DTriangle_ASM
                                                                          */
/**************************************************************************/
void CalcEle3DPrism_MTM2_num(long index, long component, CRFProcess *m_pcs){
  /* Numerik */
  static double theta;
  //  static long *element_nodes=NULL;
  static int anzgptri, anzgplin;
  static double r, s, t, fkt;
  static int nn=6;
  static int nn2=36;
  /* Material */
  static double porosity;
  static double mass_dispersion_longitudinal, mass_dispersion_transverse, molecular_diffusion;
  /* Matrizen */
  static double invjac[9], detjac;
  static double OmPrism[6];
  static double GradOmPrism[18];
  static double TransGradOmPrism[18];
  static double TransInvjac[18];
  static long i, j, l;
  static double zwa[18];
  static double zwi[36];
  static double zwu[36];
  double vg;
  static double* d;
  static double tDt[9];
  //static double* velo=NULL;
  double velo[3];
  static double velot[3];
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;
  static double *decay;
//  double *mass_dispersion_tensor;
  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;
  double gp[3];
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;

  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;

  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];

  theta = m_pcs->m_num->ls_theta; //OK

  /* Speicherplatzreservierung  */
  /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);
      if (!mass)
        mass = (double *) Malloc(nn2 * sizeof(double));

      disp = MTM2GetElementDispMatrixNew(index,  mtm_pcs_number);
      if (!disp)
        disp = (double *) Malloc(nn2 * sizeof(double));

      adv = MTM2GetElementAdvMatrixNew(index,  mtm_pcs_number);
      if (!adv)
        adv = (double *) Malloc(nn2 * sizeof(double));

	  if(GetRFProcessNumPhases()>1){
      content = MTM2GetElementFluidContentMatrixNew(index,  mtm_pcs_number);
      if (!content)
        content = (double *) Malloc(nn2 * sizeof(double));
	  }

      decay = MTM2GetElementDecayMatrixNew(index, mtm_pcs_number);
      if (!decay)
        decay = (double *) Malloc(nn2 * sizeof(double));


  d = (double *) Malloc(9 * sizeof(double));

  /* Initialisieren */
  MNulleMat(mass, nn, nn);
  MNulleMat(disp, nn, nn);
  MNulleMat(adv, nn, nn);
  MNulleMat(decay, nn, nn);
  MNulleMat(d,3,3);
  if(GetRFProcessNumPhases()>1)
    MNulleMat(content, 2, 2);

  /* Datenbereitstellung */
  anzgptri = 3;
  anzgplin = 2;
  
  /* Materialparameter */
  porosity = m_mat_mp->Porosity(index,gp,theta);
  if(GetRFProcessNumPhases()>1)
    porosity *= MTM2GetSaturation(phase, index, 0., 0., 0., MTM2_time_collocation_global);
  if(porosity < MKleinsteZahl) DisplayMsgLn("ERROR! Porosity is zero ");		

  /* Dispersion coefficient */
  mass_dispersion_longitudinal = m_mat_mp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mat_mp->mass_dispersion_transverse;
  /* Diffusion coefficient */
  molecular_diffusion = m_cp->CalcDiffusionCoefficientCP(index);   
  molecular_diffusion *= m_mat_mp->TortuosityFunction(index, gp, theta);


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

      /* Abstandsgeschwindigkeit im Gauss Punkt */
      //velo = CalcVelo3DPrismRST(phase, index, theta, r, s, t);
      gp[0] = r;
      gp[1] = s;
      gp[2] = t;
      VELCalcGaussPrisLocal(phase, index, gp, m_pcs, velo);

      velo[0] = velo[0] / porosity;
      velo[1] = velo[1] / porosity;
      velo[2] = velo[2] / porosity;
          
      /* Betrag des Vektors */
      vg = MBtrgVec(velo, 3);

      /* Dispersion im Gauss Punkt */
      /* Dispersionstensor */
      d[0] = molecular_diffusion + (mass_dispersion_longitudinal * vg);
      d[1] = 0.0;
      d[2] = 0.0;
      d[3] = 0.0;
      d[4] = molecular_diffusion + (mass_dispersion_transverse * vg);
      d[5] = 0.0;
      d[6] = 0.0;
      d[7] = 0.0;
      d[8] = molecular_diffusion + (mass_dispersion_transverse * vg);

      /* Drehen des Tensors von stromlinienorietierten Koordinaten in lokale Element Koordinaten */
      d = TensorDrehDich(d, velo);

      /* vt = v * J^-1 */
      MMultVecMat(velo, 3, invjac, 3, 3, velot, 3);

      /* tdt = (J^-1)T * D * J^-1 */
      MMultMatMat(d, 3, 3, invjac, 3, 3, zwa, 3, 3);
      MTranspoMat(invjac, 3, 3, zwi);
      MMultMatMat(zwi, 3, 3, zwa, 3, 3, tDt, 3, 3);


      /*------------------------------------------------------------------------*/
      /*---- Tracer Capacitance matrix -----------------------------------------*/
      /*------------------------------------------------------------------------*/
      
      /* Omega T * Omega * fkt */
      MOmegaPrism(OmPrism, r, s, t);
      MMultVecVec(OmPrism, nn, OmPrism, nn, zwi, nn, nn);

      for (l = 0; l < nn2; l++)  {
        mass[l] += (zwi[l] * fkt);
      }

	  /*------------------------------------------------------------------------*/
      /*---- Tracer decay Matrix -------------------------------------------*/
      /*------------------------------------------------------------------------*/
      for (l = 0; l < nn2; l++)  {
        decay[l] += (zwi[l] * fkt);
      }

      /*------------------------------------------------------------------------*/
      /*---- Tracer Fluid Content Matrix -------------------------------------------*/
      /*------------------------------------------------------------------------*/
	  if(GetRFProcessNumPhases()>1)
      for (l = 0; l < nn2; l++)  {
        content[l] += (zwi[l] * fkt);
      }

      
      /*------------------------------------------------------------------------*/
      /*---- Tracer Diffusion Matrix -------------------------------------------*/
      /*------------------------------------------------------------------------*/

      /* GradOmega T */
      MGradOmegaPrism( r, s, t, GradOmPrism);               /* 3 Zeilen 6 Spalten */
      MTranspoMat(GradOmPrism, 3, nn, TransGradOmPrism);    /* 6 Zeilen 3 Spalten */
    
      /* GradOmega T * tDt */
      MMultMatMat(TransGradOmPrism,nn,3,tDt,3,3, zwi, nn, 3);
    
      /* GradOmega T * tDt * GradOmega */
      MMultMatMat(zwi,nn,3,GradOmPrism,3,nn,zwu,6,6);
  
      /* GradOmega T * tDt * GradOmega * fkt */
      for (l = 0; l < nn2; l++) {
        disp[l] += (zwu[l] * fkt);        
      }


      /*------------------------------------------------------------------------*/
      /*---- Tracer Advection Matrix -------------------------------------------*/
      /*------------------------------------------------------------------------*/

      /* OmPrism * velot */
      MMultVecVec(OmPrism, nn, velot, 3, zwa, nn, 3); 
      /* OmPrism * velot * GradOmPrism */
      MMultMatMat(zwa, nn, 3, GradOmPrism, 3, nn, zwu, nn, nn);
      /* OmPrism * velot * GradOmPrism * fkt */
      for (l = 0; l < nn2; l++) {
        adv[l] += (zwu[l] * fkt);        
      }



    }  /* Ende Schleife über Anzahl der GaussPunkte im Dreieck (xy-Richtung) */
  }    /* Ende Schleife über Anzahl der GaussPunkte in z Richtung */


  /*------------------------------------------------------------------------*/
  /* --- Einspeichern der ermittelten Matrizen ---------------------------- */
  /*------------------------------------------------------------------------*/
 /* Einspeichern der ermittelten Matrizen */
      MTM2SetElementMassMatrixNew(index, mtm_pcs_number, mass);
      MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
      MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	  MTM2SetElementDecayMatrixNew(index, mtm_pcs_number, decay);
	  if(GetRFProcessNumPhases()>1)
		MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);
      

#ifdef TESTCEL_MTM2
  if(index < 10){
  DisplayMsgLn(" "); DisplayLong(index); DisplayMsg(":  "); DisplayDouble(velo[0],0,0); DisplayMsg(", "); DisplayDouble(velo[1],0,0); DisplayMsg(", "); DisplayDouble(velo[2],0,0); DisplayMsg(",   ");
  DisplayLong((long)mtm_pcs_number); DisplayMsgLn(", ");
  MZeigMat(mass, nn, nn, "CalcEle3DPrism_MTM2_num: MASS MATRIX");
  MZeigMat(disp, nn, nn, "CalcEle3DPrism_MTM2_num: DISPERSION MATRIX");
  MZeigMat(adv, nn, nn, "CalcEle3DPrism_MTM2_num: ADVECTION MATRIX");
  }
#endif

  /* Speicherfreigabe */
  d = (double *) Free(d);
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle3DPrism_MTM2_ana

 Aufgabe:
   Berechnet analytisch die 3-D Elementmatrizen fuer Stofftransport

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Fluidphase
   E int component: Komponente
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
 5/2003  MB  Implementierung
    
**************************************************************************/
void CalcEle3DPrism_MTM2_ana(long index, long component, CRFProcess *m_pcs){
  /* Numerik */
  static int nn=6;
  static int nn2=36;
  /* Materials */
  static double porosity;
  static double k_xx, k_xy, k_xz, k_yx, k_yy, k_yz, k_zx, k_zy, k_zz;
  static double mass_dispersion_longitudinal, mass_dispersion_transverse, molecular_diffusion;
  /* Element */
  static double area;
  static long *element_nodes=NULL;
  /* Matrizen */
  static double InvJac3Dz;
  static double DetJac3Dz;
  static double GradXTri[3];
  static double GradYTri[3];
  static double Tri_x_Tri[9];
  static double IntNTri[3];
  static double GradXTri_x_GradXTri[9];
  static double GradYTri_x_GradYTri[9];
  static double GradXTri_x_GradYTri[9];
  static double GradYTri_x_GradXTri[9];
  static double GradXTri_x_IntNTri[9];
  static double GradYTri_x_IntNTri[9];
  static double IntNTri_x_GradXTri[9];
  static double IntNTri_x_GradYTri[9];
  static double Bdeltax[9];
  static double Bdeltay[9];
  static double Dijxx[36];
  static double Dijyy[36];
  static double Dijxy[36];
  static double Dijyx[36];
  static double Dijxz[36];
  static double Dijzx[36];
  static double Dijyz[36];
  static double Dijzy[36];
  static double Dijzz[36];
  static double Bijxx[36];
  static double Bijyy[36];
  static double Bijzz[36];
  static double Ct[36];
  static double Nt_x_gradNt[36];
  static double gradNt_x_Nt[36];
  static double GradNGradN[36];
  double vg;
  double *v = NULL; //v[3];
  static double fac1;
  static double* d=NULL;
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;
  static double *decay;
  /* Laufvariablen */
  long i;

  double gp[3], theta;
//  double *mass_dispersion_tensor;

  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;

  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;

  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;

  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];

  theta = m_pcs->m_num->ls_theta;

  d = (double *) Malloc(9 * sizeof(double));

        /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = MTM2GetElementMassMatrixNew(index,  mtm_pcs_number);
      if (!mass)
        mass = (double *) Malloc(nn2 * sizeof(double));

      disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);
      if (!disp)
        disp = (double *) Malloc(nn2 * sizeof(double));

      adv = MTM2GetElementAdvMatrixNew(index,  mtm_pcs_number);
      if (!adv)
        adv = (double *) Malloc(nn2 * sizeof(double));
		
	  if(GetRFProcessNumPhases()>1){
      content = MTM2GetElementFluidContentMatrixNew(index, mtm_pcs_number);
      if (!content)
        content = (double *) Malloc(nn2 * sizeof(double));
	  }
      decay = MTM2GetElementDecayMatrixNew(index,  mtm_pcs_number);
      if (!decay)
        decay = (double *) Malloc(nn2 * sizeof(double));


//  element_nodes = (long *) Malloc(6 * sizeof(long));

  /* Initialisieren */
  MNulleMat(mass, nn, nn);
  MNulleMat(disp, nn, nn);
  MNulleMat(adv, nn, nn);
  MNulleMat(decay, nn, nn);
  if(GetRFProcessNumPhases()>1)
    MNulleMat(content, nn, nn);
  MNulleMat(d,3,3);


  /* Datenbereitstellung */
  /* Elementdaten */
  element_nodes = ElGetElementNodes(index);
  area = CalcBaseAreaPrism(index); 
  /* Get linear component matrices */
  Get_Nt_x_Nt(Ct);
  Get_Nt_x_gradNt(Nt_x_gradNt);
  Get_gradNt_x_Nt(gradNt_x_Nt);
  Get_gradNt_x_gradNt(GradNGradN);
  InvJac3Dz = GetInvJac3Dz(index);
  DetJac3Dz = 1.0 / InvJac3Dz;


  /*------------------------------------------------------------------------*/
  /*---- Tracer capacitance matrix -----------------------------------------*/
  /*------------------------------------------------------------------------*/

  Get_NTrinangle_x_NTrinangle(index, Tri_x_Tri);
  MMultMatSkalar(Tri_x_Tri,DetJac3Dz,3,3);
  GetPriMatFromTriMat(Tri_x_Tri, mass);
  MMultMatMat2(mass, 6, 6, Ct, mass);


  /*------------------------------------------------------------------------*/ 
  /* --- Tracer diffusion/dispersion matrix ------------------------------- */
  /*------------------------------------------------------------------------*/

  /* Materialparameter */
  porosity = m_mat_mp->Porosity(index,gp,theta);
  if(GetRFProcessNumPhases()>1)
    porosity *= MTM2GetSaturation(phase, index, 0., 0., 0., MTM2_time_collocation_global);
  if(porosity < MKleinsteZahl) DisplayMsgLn("ERROR! Porosity is zero ");		

  /* Dispersion coefficient */
  mass_dispersion_longitudinal = m_mat_mp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mat_mp->mass_dispersion_transverse;
  /* Diffusion coefficient */
  molecular_diffusion = m_cp->CalcDiffusionCoefficientCP(index);   
  molecular_diffusion *= m_mat_mp->TortuosityFunction(index, gp, theta);

  //SB: Abfrage der Geschwindigkeit ersetzt
  //  v = ElGetVelocityNew(index, phase);
 
  // v = CalcVelo3DPrismRST(phase, index, theta, 0., 0., 0.); //MB
  gp[0] = 0.0;
  gp[1] = 0.0;
  gp[2] = 0.0;
  VELCalcGaussPrisLocal(phase, index, gp, m_pcs, v);
  //oder besser:

  //VELCalcGaussPrisGlobal(phase, index, gp, m_pcs, v);

  v[0] = v[0] / porosity;
  v[1] = v[1] / porosity;
  v[2] = v[2] / porosity;
  /* Betrag des Vektors */
  vg = MBtrgVec(v, 3);

  /* --- Dispersionstensor */
  d[0] = molecular_diffusion + (mass_dispersion_longitudinal * vg);
  d[1] = 0.0;
  d[2] = 0.0;
  d[3] = 0.0;
  d[4] = molecular_diffusion + (mass_dispersion_transverse * vg);
  d[5] = 0.0;
  d[6] = 0.0;
  d[7] = 0.0;
  d[8] = molecular_diffusion + (mass_dispersion_transverse * vg);

  /* Drehen des Tensors von stromlinienorietierten Koordinaten in lokale Element Koordinaten */
  d = TensorDrehDich(d, v);

  k_xx =  d[0];
  k_xy =  d[1];
  k_xz =  d[2];
  k_yx =  d[3];
  k_yy =  d[4];
  k_yz =  d[5];
  k_zx =  d[6];
  k_zy =  d[7];
  k_zz =  d[8];

  /*----------------------------------------------------------------*/
  /* Calc Dijxx */
  fac1 = DetJac3Dz * area;  
  CalcGradXTri(index, GradXTri);
  MMultVecVec(GradXTri,3,GradXTri,3,GradXTri_x_GradXTri,3,3);
  MMultMatSkalar(GradXTri_x_GradXTri,fac1,3,3);
  GetPriMatFromTriMat(GradXTri_x_GradXTri, Dijxx);
  MMultMatMat2(Dijxx, 6, 6, Ct, Dijxx);
  MMultMatSkalar(Dijxx,k_xx,6,6);

  /* Calc Dijyy */
  CalcGradYTri(index, GradYTri);
  MMultVecVec(GradYTri,3,GradYTri,3,GradYTri_x_GradYTri,3,3);
  MMultMatSkalar(GradYTri_x_GradYTri,fac1,3,3);
  GetPriMatFromTriMat(GradYTri_x_GradYTri, Dijyy);
  MMultMatMat2(Dijyy, 6, 6, Ct, Dijyy);
  MMultMatSkalar(Dijyy,k_yy,6,6);
    
  /* Calc Dijzz */
  Get_NTrinangle_x_NTrinangle(index, Tri_x_Tri);
  MMultMatSkalar(Tri_x_Tri,InvJac3Dz,3,3);
  GetPriMatFromTriMat(Tri_x_Tri, Dijzz);
  MMultMatMat2(Dijzz, 6, 6, GradNGradN, Dijzz);
  MMultMatSkalar(Dijzz,k_zz,6,6);

  /* Calc Dijxy */
  MMultVecVec(GradXTri,3,GradYTri,3,GradXTri_x_GradYTri,3,3);
  MMultMatSkalar(GradXTri_x_GradYTri,fac1,3,3);
  GetPriMatFromTriMat(GradXTri_x_GradYTri, Dijxy);
  MMultMatMat2(Dijxy, 6, 6, Ct,   Dijxy);
  MMultMatSkalar(Dijxy,k_xy,6,6);
  
  /* Calc Dijyx */
  MMultVecVec(GradYTri,3,GradXTri,3,GradYTri_x_GradXTri,3,3);
  MMultMatSkalar(GradYTri_x_GradXTri,fac1,3,3);
  GetPriMatFromTriMat(GradYTri_x_GradXTri, Dijyx);
  MMultMatMat2(Dijyx, 6, 6, Ct,   Dijyx);
  MMultMatSkalar(Dijyx,k_yx,6,6);

  /* Calc Dijxz */
  Get_IntNTri(index, IntNTri);
  MMultVecVec(GradXTri,3,IntNTri,3,GradXTri_x_IntNTri,3,3);
  GetPriMatFromTriMat(GradXTri_x_IntNTri, Dijxz);
  MMultMatMat2(Dijxz, 6, 6, Nt_x_gradNt, Dijxz);
  MMultMatSkalar(Dijxz,k_xz,6,6);

  /* Calc Dijzx */
  Get_IntNTri(index, IntNTri);
  MMultVecVec(IntNTri,3,GradXTri,3,IntNTri_x_GradXTri,3,3);
  GetPriMatFromTriMat(IntNTri_x_GradXTri, Dijzx);
  MMultMatMat2(Dijzx, 6, 6, gradNt_x_Nt, Dijzx);
  MMultMatSkalar(Dijzx,k_zx,6,6);

  /* Calc Dijyz */
  MMultVecVec(GradYTri,3,IntNTri,3,GradYTri_x_IntNTri,3,3);
  GetPriMatFromTriMat(GradYTri_x_IntNTri, Dijyz);
  MMultMatMat2(Dijyz, 6, 6, Nt_x_gradNt, Dijyz);
  MMultMatSkalar(Dijyz,k_yz,6,6);
 
  /* Calc Dijzy */
  MMultVecVec(IntNTri,3,GradYTri,3,IntNTri_x_GradYTri,3,3);
  GetPriMatFromTriMat(IntNTri_x_GradYTri, Dijzy);
  MMultMatMat2(Dijzy, 6, 6, gradNt_x_Nt, Dijzy);
  MMultMatSkalar(Dijzy,k_zy,6,6);



  /*------------------------------------------------------------------------*/
  /* Volumenelemente */
  for (i=0;i<nn2;i++) {
    disp[i] =  Dijxx[i] + Dijzz[i] + Dijyy[i] \
                                + Dijxy[i] + Dijyx[i] \
                                + Dijxz[i] + Dijzx[i] \
                                + Dijyz[i] + Dijzy[i]   ;
  }
  

  /*------------------------------------------------------------------------*/
  /* --- Tracer advection matrix ------------------------------------------ */
  /*------------------------------------------------------------------------*/
  Get_IntNTri(index, IntNTri);

  /* Calc Bijxx */
  MMultVecVec(IntNTri,3,GradXTri,3,Bdeltax,3,3);
  MMultMatSkalar(Bdeltax,DetJac3Dz,3,3);
  GetPriMatFromTriMat(Bdeltax, Bijxx);
  MMultMatMat2(Bijxx, 6, 6, Ct, Bijxx);
  MMultMatSkalar(Bijxx,v[0],6,6);

  /* Calc Bijyy */
  MMultVecVec(IntNTri,3,GradYTri,3,Bdeltay,3,3);
  MMultMatSkalar(Bdeltay,DetJac3Dz,3,3);
  GetPriMatFromTriMat(Bdeltay, Bijyy);
  MMultMatMat2(Bijyy, 6, 6, Ct, Bijyy);
  MMultMatSkalar(Bijyy,v[1],6,6);

  /* Calc Bijzz */
  Get_NTrinangle_x_NTrinangle(index, Tri_x_Tri);
  GetPriMatFromTriMat(Tri_x_Tri, Bijzz);
  MMultMatMat2(Bijzz, 6, 6, Nt_x_gradNt, Bijzz);
  MMultMatSkalar(Bijzz,v[2],6,6);


  /*------------------------------------------------------------------------*/
  for (i=0;i<nn2;i++) {
    adv[i] = Bijxx[i] + Bijyy[i] + Bijzz[i];
  }

  /*------------------------------------------------------------------------*/
  /* --- Einspeichern der ermittelten Matrizen ---------------------------- */
  /*------------------------------------------------------------------------*/
      MTM2SetElementMassMatrixNew(index,  mtm_pcs_number, mass);
      MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
      MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	  MTM2SetElementDecayMatrixNew(index, mtm_pcs_number, decay);
      if(GetRFProcessNumPhases()>1)
        MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);


  /*------------------------------------------------------------------------*/
  /* Speicherfreigabe */
  d = (double *) Free(d);


#ifdef TESTCEL_MTM2
  if(index < 10){
  DisplayMsgLn(" "); DisplayLong(index); DisplayMsg(":  "); DisplayDouble(v[0],0,0); DisplayMsg(", "); DisplayDouble(v[1],0,0); DisplayMsg(", "); DisplayDouble(v[2],0,0); DisplayMsg(",   ");
  DisplayLong((long)mtm_pcs_number); DisplayMsgLn(", ");
  MZeigMat(mass, nn, nn, "CalcEle3DPrism_MTM2_ana: MASS MATRIX");
  MZeigMat(disp, nn, nn, "CalcEle3DPrism_MTM2_ana: DISPERSION MATRIX");
  MZeigMat(adv, nn, nn, "CalcEle3DPrism_MTM2_ana: ADVECTION MATRIX");
  }
#endif

}

 /**************************************************************************
ROCKFLOW - Function: MTM2CalcElementMatricesTetrahedra
Task:
  Linear tetrahedral element for mass transport
  11/2004 CMCD Implementation, based on Traingular solution & heat transport Tet.
**************************************************************************/
void MTM2CalcElementMatricesTetrahedra(long index, long component, CRFProcess *m_pcs)
{
  /* Laufvariablen */
  static long i;
  int k;
  /* Geometry */
  static double x[4],y[4],z[4];
  //static double volume;
  static int nn=4;
  static int nn2=16;
  static double fac_geo;
  static long *element_nodes;
  /* NUM - Numerics */
  static double alpha0;
  //  static double art_diff;
  static double zwa[4];
  double a1,a2,a3,a4;
  double b1,b2,b3,b4;
  double c1,c2,c3,c4;
  double d1,d2,d3,d4;
  double dN1dx,dN2dx,dN3dx,dN4dx;
  double dN1dy,dN2dy,dN3dy,dN4dy;
  double dN1dz,dN2dz,dN3dz,dN4dz;
  	double N1_1,N1_2,N1_3,N1_4;
	double N2_1,N2_2,N2_3,N2_4;
	double N3_1,N3_2,N3_3,N3_4;
	double N4_1,N4_2,N4_3,N4_4;
 
  double mat3x3[9];
  double volume;
  //static double dreh[4],zwi[4],zwa[4];
  static double alpha[3],zeta[2],pec[2];
  /* MAT - Materials */
  static double molecular_diffusion;
  static double mass_dispersion_longitudinal,mass_dispersion_transverse;
    double * mass_dispersion_tensor, d[3], d_1[9];
   /* MAT-MP */
//  static double *d;
  static double *velovec, vg, v[3];
  static double porosity;
  static double fac_mat;
  static double q_x,q_y,q_z;
  /* Element matrices */
  static double *mass;
  static double *disp;
  static double *adv;
  static double *content;
  static double *decay;  //SB:todo
  double gp[3], theta;

  static double GrdShpFct[12],GrdShpFctT[12];
  static double GrdShpFctLoc[12];
  static double Jac[9],DetJac;
  static double mat4x3[12];

//  double *mass_dispersion_tensor;
  int phase;
  int mtm_pcs_number = m_pcs->pcs_number;
  //----------------------------------------------------------------------
  // NUM properties
  
  theta = m_pcs->m_num->ls_theta;
  CalculateSimpleMiddelPointElement(index,gp);
  //----------------------------------------------------------------------
  // MFP fluid properties
  int mtm_no_phases = (int)mfp_vector.size(); //OK
  //----------------------------------------------------------------------
  // MCP component properties
  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mat_mp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mat_mp = mmp_vector[group];
  /* ------------------------------------------------------------------ */
  /* Speicher-Modelle fuer Element-Matrizen */
  if (memory_opt == 0) { /* keine Speicheroptimierung */
    mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);
    if (!mass)
      mass = (double *) Malloc(nn2 * sizeof(double));
    disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);
    if (!disp)
      disp = (double *) Malloc(nn2 * sizeof(double));
    adv = MTM2GetElementAdvMatrixNew(index,mtm_pcs_number);
    if (!adv)
      adv = (double *) Malloc(nn2 * sizeof(double));
    decay = MTM2GetElementDecayMatrixNew(index,  mtm_pcs_number);
    if (!decay)
      decay = (double *) Malloc(nn2 * sizeof(double));
    if(mtm_no_phases>1){ //OK
  	  content = MTM2GetElementFluidContentMatrixNew(index,mtm_pcs_number);
      if (!content)
         content = (double *) Malloc(nn2 * sizeof(double));
    }
  }
  MNulleMat(mass,nn,nn);
  MNulleMat(disp,nn,nn);
  MNulleMat(adv,nn,nn);
  MNulleMat(decay,nn, nn);
  if(mtm_no_phases>1)
    MNulleMat(content, nn, nn);

  /* ------------------------------------------------------------------ */
  /* GEO - geometry */
  CalculateSimpleMiddelPointElement(index,gp);
  element_nodes = ElGetElementNodes(index);
  for (i=0;i<nn;i++) {
    x[i]=GetNodeX(element_nodes[i]);
    y[i]=GetNodeY(element_nodes[i]);
    z[i]=GetNodeZ(element_nodes[i]);
  }
  
  //Element Data
  volume = (ElGetElementVolume(index));
  porosity = m_mat_mp->Porosity(index,gp,theta);
  if(GetRFProcessNumPhases()>1)
    porosity *= MTM2GetSaturation(phase, index, 0., 0., 0., MTM2_time_collocation_global);

  // MMP medium properties
//  CMediumProperties *m_mmp = NULL;
//  m_mmp = mmp_vector[group];
//  porosity = m_mmp->Porosity(index,gp,theta);
  
  /* ------------------------------------------------------------------ */
  /* MAT - materials */
   /* MAT-CP */
  mass_dispersion_longitudinal = m_mat_mp->mass_dispersion_longitudinal;
  mass_dispersion_transverse = m_mat_mp->mass_dispersion_transverse;
  /* Diffusion coefficient */
  molecular_diffusion = m_cp->CalcDiffusionCoefficientCP(index);   
  molecular_diffusion *= m_mat_mp->TortuosityFunction(index, gp, theta);

  mass_dispersion_tensor = m_mat_mp->MassDispersionTensor(index,gp,theta,component);
  d[0] = mass_dispersion_tensor[0];
  d[1] = mass_dispersion_tensor[4];
  d[2] = mass_dispersion_tensor[8];
  /*--------------------------------------------------------------------------*/
  /* pore velocity in Gauss-point */
  velovec = ElGetVelocity(index);
  v[0] = velovec[0]/porosity/100.;
  v[1] = velovec[1]/porosity/100.;
  v[2] = velovec[2]/porosity/100.; //OK
  vg = MBtrgVec(v,3);
  /*--------------------------------------------------------------------------*/
  /* Shape Functions */
	mat3x3[0]=x[1]; mat3x3[1]=y[1]; mat3x3[2]=z[1];
    mat3x3[3]=x[2]; mat3x3[4]=y[2]; mat3x3[5]=z[2];
    mat3x3[6]=x[3]; mat3x3[7]=y[3]; mat3x3[8]=z[3];
    a1 = 1.0 * M3Determinante(mat3x3);    
	mat3x3[0]=x[2]; mat3x3[1]=y[2]; mat3x3[2]=z[2];
    mat3x3[3]=x[3]; mat3x3[4]=y[3]; mat3x3[5]=z[3];
    mat3x3[6]=x[0]; mat3x3[7]=y[0]; mat3x3[8]=z[0];
    a2 = -1.0 * M3Determinante(mat3x3); 
	mat3x3[0]=x[3]; mat3x3[1]=y[3]; mat3x3[2]=z[3];
    mat3x3[3]=x[0]; mat3x3[4]=y[0]; mat3x3[5]=z[0];
    mat3x3[6]=x[1]; mat3x3[7]=y[1]; mat3x3[8]=z[1];
    a3 = 1.0 * M3Determinante(mat3x3); 
	mat3x3[0]=x[0]; mat3x3[1]=y[0]; mat3x3[2]=z[0];
    mat3x3[3]=x[1]; mat3x3[4]=y[1]; mat3x3[5]=z[1];
    mat3x3[6]=x[2]; mat3x3[7]=y[2]; mat3x3[8]=z[2];
    a4 = -1.0 * M3Determinante(mat3x3); 
		
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
    
/*	//Element Shape Functions */
	N1_1 = ((a1*1)+(b1*x[0])+(c1*y[0])+(d1*z[0]))/(6*volume);
	N1_2 = ((a1*1)+(b1*x[1])+(c1*y[1])+(d1*z[1]))/(6*volume);
	N1_3 = ((a1*1)+(b1*x[2])+(c1*y[2])+(d1*z[2]))/(6*volume);
	N1_4 = ((a1*1)+(b1*x[3])+(c1*y[3])+(d1*z[3]))/(6*volume);
	
	N2_1 = ((a2*1)+(b2*x[0])+(c2*y[0])+(d2*z[0]))/(6*volume);
	N2_2 = ((a2*1)+(b2*x[1])+(c2*y[1])+(d2*z[1]))/(6*volume);
	N2_3 = ((a2*1)+(b2*x[2])+(c2*y[2])+(d2*z[2]))/(6*volume);
	N2_4 = ((a2*1)+(b2*x[3])+(c2*y[3])+(d2*z[3]))/(6*volume);
		
	N3_1 = ((a3*1)+(b3*x[0])+(c3*y[0])+(d3*z[0]))/(6*volume);
	N3_2 = ((a3*1)+(b3*x[1])+(c3*y[1])+(d3*z[1]))/(6*volume);
	N3_3 = ((a3*1)+(b3*x[2])+(c3*y[2])+(d3*z[2]))/(6*volume);
	N3_4 = ((a3*1)+(b3*x[3])+(c3*y[3])+(d3*z[3]))/(6*volume);
		
	N4_1 = ((a4*1)+(b4*x[0])+(c4*y[0])+(d4*z[0]))/(6*volume);
	N4_2 = ((a4*1)+(b4*x[1])+(c4*y[1])+(d4*z[1]))/(6*volume);
	N4_3 = ((a4*1)+(b4*x[2])+(c4*y[2])+(d4*z[2]))/(6*volume);
	N4_4 = ((a4*1)+(b4*x[3])+(c4*y[3])+(d4*z[3]))/(6*volume);
//*/

    //Actual element shape function derivates
    dN1dx = b1/(6*volume);
    dN2dx = b2/(6*volume);
    dN3dx = b3/(6*volume);
    dN4dx = b4/(6*volume);

    dN1dy = c1/(6*volume);
    dN2dy = c2/(6*volume);
    dN3dy = c3/(6*volume);
    dN4dy = c4/(6*volume);

    dN1dz = d1/(6*volume);
    dN2dz = d2/(6*volume);
    dN3dz = d3/(6*volume);
    dN4dz = d4/(6*volume);

  GrdShpFct[0]=b1/6./volume;
  GrdShpFct[1]=b2/6./volume;
  GrdShpFct[2]=b3/6./volume;
  GrdShpFct[3]=b4/6./volume;
  GrdShpFct[4]=c1/6./volume;
  GrdShpFct[5]=c2/6./volume;
  GrdShpFct[6]=c3/6./volume;
  GrdShpFct[7]=c4/6./volume;
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


 
  /* --- Dispersionstensor */


  d_1[0] = molecular_diffusion; //mass_dispersion_tensor[0];
  d_1[1] = 0.0;
  d_1[2] = 0.0;
  d_1[3] = 0.0;
  d_1[4] = molecular_diffusion; //mass_dispersion_tensor[4];
  d_1[5] = 0.0;
  d_1[6] = 0.0;
  d_1[7] = 0.0;
  d_1[8] = molecular_diffusion; //mass_dispersion_tensor[8];

//  d = TensorDrehDich(d, v);
  /*--------------------------------------------------------------------------*/
  /* NUM - SUPG */
  MNulleVec(alpha,2);
  if (vg>MKleinsteZahl && alpha0>0.0) {
    /* Berechnung von zeta */
    for (k=0;k<2;k++)
      zeta[k] = -v[k] / vg;  /* reine Advektion */
    if (d_1[0]>MKleinsteZahl && d_1[3]>MKleinsteZahl) {
      /* halbe Peclet-Zahl mit ds=2.0 */
      /* pec = vt * tdt^-1 */
      for (k=0;k<4;k++)
        zwa[k] = d[k];
      M2Invertiere(zwa);
      MMultVecMat(v,2,zwa,2,2,pec,2);
      MNulleVec(zeta,2); /* fast reine Diffusion */
      for (k=0;k<2;k++)
        if (fabs(pec[k]) > 1.0)
          zeta[k] = (fabs(pec[k]) - 1.0) / pec[k];
    }
    /* Upwind-Faktoren */
    for (k=0;k<2;k++)
      alpha[k] = alpha0 * zeta[k];
#ifdef TESTSUPG
                 DisplayDoubleVector(alpha,2,"alpha");
#endif
  }
  /*==========================================================================*/
  /* --- Massenmatrix - Heat capacitance matrix ----------------------------- */
   /* Materials */
  fac_mat = porosity;
     /* 4x4 matrix */
    mass[0] = 2.0;
    mass[1] = 1.0;
    mass[2] = 1.0;
    mass[3] = 1.0;
    mass[4] = 1.0;
    mass[5] = 2.0;
    mass[6] = 1.0;
    mass[7] = 1.0;
    mass[8] = 1.0;
    mass[9] = 1.0;
    mass[10] = 2.0;
    mass[11] = 1.0;
    mass[12] = 1.0;
    mass[13] = 1.0;
    mass[14] = 1.0;
    mass[15] = 2.0;
   /* Volumetric element */
    fac_geo = (volume)/(20.);

    for (i=0;i<nn2;i++) {
        mass[i] *= fac_mat * fac_geo;
        }

  /* --- Decay matrix ------------------------------------------------------- */
//SB:todo
  fac_mat = porosity*0.0;
   /* 3x3 matrix */
   decay[0] = 2.0;
    decay[1] = 1.0;
    decay[2] = 1.0;
    decay[3] = 1.0;
    decay[4] = 1.0;
    decay[5] = 2.0;
    decay[6] = 1.0;
    decay[7] = 1.0;
    decay[8] = 1.0;
    decay[9] = 1.0;
    decay[10] = 2.0;
    decay[11] = 1.0;
    decay[12] = 1.0;
    decay[13] = 1.0;
    decay[14] = 1.0;
    decay[15] = 2.0;
   /* Volumetric element */
    fac_geo = (volume)/(20.);

    for (i=0;i<nn2;i++) {
        decay[i] *= fac_mat * fac_geo;
        }

  /* --- Dispersionsmatrix - Heat diffusion-dispersion matrix --------------- */
      /* Materials */
      fac_mat =  porosity;

   /* Heat dispersion matrix */
    /* 4 x 4 Matrix */

    disp[0]  = d[0]*b1*b1 + d[1]*c1*c1 + d[2]*d1*d1;
    disp[1]  = d[0]*b1*b2 + d[1]*c1*c2 + d[2]*d1*d2;
    disp[2]  = d[0]*b1*b3 + d[1]*c1*c3 + d[2]*d1*d3;
    disp[3]  = d[0]*b1*b4 + d[1]*c1*c4 + d[2]*d1*d4;
    disp[4]  = d[0]*b2*b1 + d[1]*c2*c1 + d[2]*d2*d1;
    disp[5]  = d[0]*b2*b2 + d[1]*c2*c2 + d[2]*d2*d2;
    disp[6]  = d[0]*b2*b3 + d[1]*c2*c3 + d[2]*d2*d3;
    disp[7]  = d[0]*b2*b4 + d[1]*c2*c4 + d[2]*d2*d4;
    disp[8]  = d[0]*b3*b1 + d[1]*c3*c1 + d[2]*d3*d1;
    disp[9]  = d[0]*b3*b2 + d[1]*c3*c2 + d[2]*d3*d2;
    disp[10] = d[0]*b3*b3 + d[1]*c3*c3 + d[2]*d3*d3;
    disp[11] = d[0]*b3*b4 + d[1]*c3*c4 + d[2]*d3*d4;
    disp[12] = d[0]*b4*b1 + d[1]*c4*c1 + d[2]*d4*d1; //OK
    disp[13] = d[0]*b4*b2 + d[1]*c4*c2 + d[2]*d4*d2;
    disp[14] = d[0]*b4*b3 + d[1]*c4*c3 + d[2]*d4*d3;
    disp[15] = d[0]*b4*b4 + d[1]*c4*c4 + d[2]*d4*d4;
    fac_geo = 1./(36.*volume);

  for (i=0;i<nn2;i++) {
		disp[i] *= fac_mat * fac_geo;//CMCD -> SB, is this correct?
        }

  MMultMatMat(GrdShpFctT,4,3,d_1,3,3,mat4x3,4,3);
  MMultMatMat(mat4x3,4,3,GrdShpFct,3,4,disp,4,4);
  for (i=0;i<nn2;i++) disp[i] *= fac_mat * (1.0); 


  /* --- Advektionsmatrix ---------------------------------------------------- */
      /* Materials */
    fac_mat = porosity;
    q_x = v[0];
    q_y = v[1];
    q_z = v[2];
    /* 4x4 matrix */
    adv[0] =  q_x*dN1dx + q_y*dN1dy + q_z*dN1dz;
    adv[1] =  q_x*dN2dx + q_y*dN2dy + q_z*dN2dz;
    adv[2] =  q_x*dN3dx + q_y*dN3dy + q_z*dN3dz;
    adv[3] =  q_x*dN4dx + q_y*dN4dy + q_z*dN4dz;
    adv[4] =  q_x*dN1dx + q_y*dN1dy + q_z*dN1dz;
    adv[5] =  q_x*dN2dx + q_y*dN2dy + q_z*dN2dz;
    adv[6] =  q_x*dN3dx + q_y*dN3dy + q_z*dN3dz;
    adv[7] =  q_x*dN4dx + q_y*dN4dy + q_z*dN4dz;
    adv[8] =  q_x*dN1dx + q_y*dN1dy + q_z*dN1dz;
    adv[9] =  q_x*dN2dx + q_y*dN2dy + q_z*dN2dz;
    adv[10] = q_x*dN3dx + q_y*dN3dy + q_z*dN3dz;
    adv[11] = q_x*dN4dx + q_y*dN4dy + q_z*dN4dz;
    adv[12] = q_x*dN1dx + q_y*dN1dy + q_z*dN1dz;
    adv[13] = q_x*dN2dx + q_y*dN2dy + q_z*dN2dz;
    adv[14] = q_x*dN3dx + q_y*dN3dy + q_z*dN3dz;
    adv[15] = q_x*dN4dx + q_y*dN4dy + q_z*dN4dz;
    
    /* Volumetric element */
    fac_geo = 0.25*volume ; //For the shape function contribution.
	  for (i=0;i<nn2;i++) {
		adv[i] *= fac_mat*fac_geo;
	  }


  /* --- Content matrix ------------------------------------------------------- */
  if(mtm_no_phases>1){ //OK
  //SB:todo
    fac_mat = porosity*0.0;
   /* 3x3 matrix */
    content[0] = 2.0;
    content[1] = 1.0;
    content[2] = 1.0;
    content[3] = 1.0;
    content[4] = 1.0;
    content[5] = 2.0;
    content[6] = 1.0;
    content[7] = 1.0;
    content[8] = 1.0;
    content[9] = 1.0;
    content[10] = 2.0;
    content[11] = 1.0;
    content[12] = 1.0;
    content[13] = 1.0;
    content[14] = 1.0;
    content[15] = 2.0;
   /* Volumetric element */
    fac_geo = (volume)/(20.);
    for (i=0;i<nn2;i++) {
      content[i] *= fac_mat * fac_geo;
    }
  }
   /* --------------------------------------------------- */
  /* Element-Matrizen speichern oder uebergeben */
  if (memory_opt == 0) {
    MTM2SetElementMassMatrixNew(index, mtm_pcs_number, mass);
    MTM2SetElementDispMatrixNew(index, mtm_pcs_number, disp);
    MTM2SetElementAdvMatrixNew(index, mtm_pcs_number, adv);
	MTM2SetElementDecayMatrixNew(index,mtm_pcs_number, decay);
    if(mtm_no_phases>1){ //OK
      MTM2SetElementFluidContentMatrixNew(index, mtm_pcs_number, content);
    }
  }

#ifdef TESTCEL_MTM2
if(index < 2){
  DisplayMsgLn("");DisplayLong(index);DisplayMsgLn(":");
  DisplayMsg(" v: "); DisplayDouble(v[0],0,0); DisplayMsg(", ");DisplayDouble(v[1],0,0); DisplayMsg(", ");DisplayDouble(v[2],0,0); DisplayMsgLn(" ");
  DisplayMsg(" volume: "); DisplayDouble(volume,0,0); DisplayMsgLn(" ");
//  MZeigMat(mass,nn,nn,"MTM2CalcElementMatrices3DTetrahedra: Mass capacitance matrix");
//  MZeigMat(disp,nn,nn,"MTM2CalcElementMatrices3DTetrahedra: Mass diffusion-dispersion matrix");
//  MZeigMat(adv,nn,nn, "MTM2CalcElementMatrices3DTetrahedra: Mass advection matrix");
}
#endif
}


/*
void MTM2SetElementDtNew ( long number, int phase, double zeitschritt )
{
//  ElSetElementVal(number,index_timestep_SB,zeitschritt); //SB:todo
}

void MTM2SetElementPecletNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3; //SB:todo
//	ElSetElementVal(number,index,value);
}

double MTM2GetElementPecletNumNew ( long number, int phase, int component)
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3;  //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}


void MTM2SetElementCourantNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3 +1; //SB:todo
//	ElSetElementVal(number,index,value);
}

double MTM2GetElementCourantNumNew ( long number, int phase, int component)
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3 +1; //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}


void MTM2SetElementDamkohlerNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	if(number < ElListSize()) {
//		index = index_TransportNumbers_SB + (component)*3 + 2; //SB:todo
//		ElSetElementVal(number,index,value);
//	}
}

double MTM2GetElementDamkohlerNumNew ( long number, int phase, int component)
{
//	int index;
//	if(number >= ElListSize()) return 0.0;
//	index = index_TransportNumbers_SB + (component)*3 +2; //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}
*/




/**************************************************************************
   ROCKFLOW - Funktion: MTM2DampOscillations

   Aufgabe:
   
   Daempft Oszillationen des Feldes oder begrenzt den Wertebereich.

   Ergebnis:
   - void -

   Programmaenderungen:
   07/2002   CT   Erste Version

  **************************************************************************/
void MTM2DampOscillations(int ndx1, int phase)
{
   activ_phase = phase;
   DampOscillations(ndx1, MTM2_oscil_damp_method, MTM2_oscil_damp_parameter, MTM2NodeCalcLumpedMass);
}


/**************************************************************************
   ROCKFLOW - Funktion:  MTM2NodeCalcLumpedMass

   Aufgabe:
   Liefert fuer einen Knoten die "gelumpte" Speichermatrix

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB

   Ergebnis:
     double mass : Rueckgabe, des Lumped-Wertes

   Programmaenderungen:
   7/2002 C.Thorenz Erste Version

 **************************************************************************/
double MTM2NodeCalcLumpedMass(long knoten)
{
  double *mass=NULL, lm=0.;
  long *elemente, *nodes;
  int anz_elemente, i, j, k, nn;
//  int component = 1; //SB:todo
  elemente = GetNodeElements(knoten, &anz_elemente);

  for (i = 0l; i < anz_elemente; i++) {
    nn = ElNumberOfNodes[ElGetElementType(elemente[i]) - 1];
    
//SB:todo    mass = MTM2GetElementMassMatrixNew(elemente[i], component, mtm_pcs_number);
    if (mass) {
      nodes = ElGetElementNodes(elemente[i]);
      for (j = 0; j < nn; j++) {
         if(nodes[j]==knoten)
           for (k = 0; k < nn; k++)
             lm += mass[j * nn + k];
      }
    }
  }

  elemente = (long *)Free(elemente);

  return lm;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: MTM2CreateELEMatricesPointer
                                                                          */
/* Aufgabe:
   Initialisiert und erzeugt interne Elementdaten fuer MTM2.                                                                          

   04/2003	   SB		  MTM2
                                                                     */
/**************************************************************************/
void *MTM2CreateELEMatricesPointer ( void )
{
  static InternElDataMTM2 *data;
  data = (InternElDataMTM2 *) Malloc(sizeof(InternElDataMTM2));

  data -> advectionmatrix = NULL;
  data -> dispersionmatrix = NULL;
  data -> massmatrix = NULL;
  data -> decaymatrix = NULL;
  data -> fluidcontentmatrix = NULL;

  return (void *) data;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: MTM2DestroyELEMatricesPointer
                                                                          */
/* Aufgabe:
   Entfernt interne Elementdaten fuer MTM2 aus dem Speicher.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *data: Zeiger auf zu loeschenden Datensatz
                                                                          */
/* Ergebnis:
   NULL
                                                                          */
/* Programmaenderungen:
   04/2003	  SB	  MTM2
                                                                          */
/**************************************************************************/
void *MTM2DestroyELEMatricesPointer ( void *data )
{

  if(data){
	if(((InternElDataMTM2 *) data) -> advectionmatrix)
		((InternElDataMTM2 *) data) -> advectionmatrix = (double *) Free(((InternElDataMTM2 *) data) -> advectionmatrix);
	if(((InternElDataMTM2 *) data) -> dispersionmatrix)
		((InternElDataMTM2 *) data) -> dispersionmatrix = (double *) Free(((InternElDataMTM2 *) data) -> dispersionmatrix);
	if(((InternElDataMTM2 *) data) -> massmatrix)
		((InternElDataMTM2 *) data) -> massmatrix = (double *) Free(((InternElDataMTM2 *) data) -> massmatrix);
	if(((InternElDataMTM2 *) data) -> decaymatrix)
		((InternElDataMTM2 *) data) -> decaymatrix = (double *) Free(((InternElDataMTM2 *) data) -> decaymatrix);
	if(((InternElDataMTM2 *) data) -> fluidcontentmatrix)
		((InternElDataMTM2 *) data) -> fluidcontentmatrix = (double *) Free(((InternElDataMTM2 *) data) -> fluidcontentmatrix);
  
  data = (void*) Free(data);                   /* sollte NULL sein */
  }
  return data;
}


/**************************************************************************
 ROCKFLOW - Funktion: MTM2GetSaturation

 Aufgabe:

 Liefert Saettigung

  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E: phase         Betrachtete phase
  E: index         Elementnummer
  E: r,s,t         Lokale Koordinaten
  E: theta         Zeitwichtung

  Ergebnis:
  Saettigung

  Programmaenderungen:
  8/1999 C.Thorenz                                           Erste Version
  02/2004   SB  angepasst für MTM2
11/2004 OK PCS2
**************************************************************************/
double MTM2GetSaturation(int phase, long index, double r, double s, double t, double theta)
{
phase = phase;
  static double satu_old, satu_new, satu = -1.0;
  string saturation_name = "SATURATION";
  char char_phase[1];
  sprintf(char_phase,"char_phase");
  saturation_name += char_phase;
  //OK satu_old = InterpolValue(index, PCSGetPhaseSaturationNodeIndex(phase,0), r, s, t);
  satu_old = InterpolValue(index,PCSGetNODValueIndex(saturation_name,0),r,s,t);
  //OK satu_new = InterpolValue(index, PCSGetPhaseSaturationNodeIndex(phase,1), r, s, t);
  satu_new = InterpolValue(index,PCSGetNODValueIndex(saturation_name,1),r,s,t);
  satu = (1. - theta) * satu_old + theta * satu_new;
  satu = min(max(satu, MKleinsteZahl), 1. - MKleinsteZahl);

  return satu;
}

/*************************************************************************
 ROCKFLOW - Funktion: MTM2Get- Set- DeleteElement...

 Aufgabe:

 Formalparameter: SET:
   E long number: Elementindex
(  E int phase  : Betrachtete Phase)
   X double *matrix: Zeiger auf die Matrix bzw. den Vektor

 Ergebnis:
   - void -

 Programmaenderungen:
   2003		SB
   02/2004  SB  Adapted to new structure and new CompProperties

*************************************************************************/

void MTM2SetElementAdvMatrixNew(long number,int pcs_number,double*matrix)
{
  static InternElDataMTM2 *element_matrices;
  element_matrices = (InternElDataMTM2 *) ELEGetElementMatrices(number,pcs_number);
  element_matrices->advectionmatrix = matrix;
}

void MTM2SetElementDecayMatrixNew(long number,int pcs_number,double*matrix)
{
  static InternElDataMTM2 *element_matrices;
  element_matrices = (InternElDataMTM2 *) ELEGetElementMatrices(number,pcs_number);
  element_matrices->decaymatrix = matrix;
}

void MTM2SetElementDispMatrixNew(long number,int pcs_number,double*matrix)
{
  static InternElDataMTM2 *element_matrices;
  element_matrices = (InternElDataMTM2 *) ELEGetElementMatrices(number,pcs_number);
  element_matrices->dispersionmatrix = matrix;
}

void MTM2SetElementMassMatrixNew(long number,int pcs_number,double*matrix)
{
  static InternElDataMTM2 *element_matrices;
  element_matrices = (InternElDataMTM2 *) ELEGetElementMatrices(number,pcs_number);
  element_matrices->massmatrix = matrix;
}

void MTM2SetElementFluidContentMatrixNew(long number,int pcs_number,double*matrix)
{
  static InternElDataMTM2 *element_matrices;
  element_matrices = (InternElDataMTM2 *) ELEGetElementMatrices(number,pcs_number);
  element_matrices->fluidcontentmatrix = matrix;
} 

double *MTM2GetElementAdvMatrixNew(long number,int mtm_pcs_number)
{
  return ((InternElDataMTM2 *) ELEGetElementMatrices(number,mtm_pcs_number)) -> advectionmatrix;
}

double *MTM2GetElementDecayMatrixNew(long number,  int mtm_pcs_number)
{
  return ((InternElDataMTM2 *) ELEGetElementMatrices(number,mtm_pcs_number)) -> decaymatrix;
}

double *MTM2GetElementDispMatrixNew(long number, int mtm_pcs_number)
{
  return ((InternElDataMTM2 *) ELEGetElementMatrices(number,mtm_pcs_number)) -> dispersionmatrix;
}

double *MTM2GetElementMassMatrixNew(long number, int mtm_pcs_number)
{
  return ((InternElDataMTM2 *) ELEGetElementMatrices(number,mtm_pcs_number)) -> massmatrix;
}

 double *MTM2GetElementFluidContentMatrixNew(long number,  int mtm_pcs_number)
{
  return ((InternElDataMTM2 *) ELEGetElementMatrices(number,mtm_pcs_number)) -> fluidcontentmatrix;
} 


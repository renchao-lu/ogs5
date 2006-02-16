/**************************************************************************
   ROCKFLOW - Modul: cel_mms.c

   Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul MMS.

   Programmaenderungen:
   9/1998   C.Thorenz       Erste Version
   5/1999   C.Thorenz       Umbau auf n-Phasen
   7/1999   C.Thorenz      3D-Elemente
   9/1999   C.Thorenz      2D-Elemente
   4/2000   C.Thorenz      SUPG auf alle Phasen erweitert
   9/2000   C.Thorenz      ML-FEM, Kernelkonfiguration
   7/2002   C.Thorenz      Daempfung fuer Feldoszillationen
   03/2003  RK             Quellcode bereinigt, Globalvariablen entfernt
   05/2003 OK Data access functions to element matrices

 **************************************************************************/
#include "stdafx.h"                    /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
#define noTESTSUPG
#define noTESTCEL_MMS
/* Header / Andere intern benutzte Module */
#include "cvel.h"
#include "rf_vel_new.h"
#include "cel_mms.h"
#include "int_mms.h"
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "femlib.h"
#include "rf_num_new.h"
#include "tools.h"
#include "cel_mmp.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_mmp_new.h"
#include "rf_mfp_new.h"
#include "rf_pcs.h"
/* Interne (statische) Deklarationen */
static void CalcEle1D(long index,CRFProcess*);
static void CalcEle2D(long index,CRFProcess*);
static void CalcEle3D(long index,CRFProcess*);
void MMSCalcElementMatrix(long index,CRFProcess*m_pcs);
//static int mms_timecollocation_array_size = 0;

/* Default-Werte fuer den Kernel */
static int mms_method = 1;
static int mms_calc_other_saturations_method = 0;
static double mms_time_collocation_global = 1.;
static double mms_time_collocation_cond_bc = 1.;
static double mms_time_collocation_source = 0.;
static double mms_time_collocation_upwind = 0.;
static double mms_time_collocation_open_boundary = 0.;

static int mms_upwind_method = 1;
static double mms_upwind_parameter = 1.;

static int mms_mass_lumping_method = 0;
//static double mms_mass_lumping_parameter = 0.;

static int mms_relaxation_method = 0;
static double mms_relaxation_parameter[16];

static int mms_predictor_method = 0;
static double mms_predictor_parameter = 0.;

//static int mms_artificial_diffusion = 0;
//static double mms_artificial_diffusion_param[16];
static int mms_nonlinear_coupling = 1;

static int mms_oscil_damp_method = 0;
static double mms_oscil_damp_parameter[16];

static int mms_pcs_number = 1;

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_MMS

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/
/**************************************************************************
 ROCKFLOW - Funktion: GetTimeCollocationXXX_MMS

 Aufgabe:
   Gibt Zeitkollokation zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   22.04.99   C.Thorenz    Erste Version

**************************************************************************/
int GetMethod_MMS(void)
{
  return mms_method;
}
double GetTimeCollocationGlobal_MMS(void)
{
  return mms_time_collocation_global;
}
double GetTimeCollocationCondBC_MMS(void)
{
  return mms_time_collocation_cond_bc;
}
double GetTimeCollocationSource_MMS(void)
{
  return mms_time_collocation_source;
}
double GetTimeCollocationupwind_MMS(void)
{
  return mms_time_collocation_upwind;
}
double GetTimeCollocationOpenBoundary_MMS(void)
{
  return mms_time_collocation_open_boundary;
}
int GetOtherSatuCalcMethod_MMS(void)
{
  return mms_calc_other_saturations_method;
}
int GetNonLinearCoupling_MMS(void)
{
  return mms_nonlinear_coupling;
}
int GetPredictorMethod_MMS(void)
{
  return mms_predictor_method;
}
double GetPredictorParam_MMS(void)
{
  return mms_predictor_parameter;
}
int GetRelaxationMethod_MMS(void)
{
  return mms_relaxation_method;
}
double *GetRelaxationParam_MMS(void)
{
  return mms_relaxation_parameter;
}
/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void MMSCalcElementMatrices(CRFProcess*m_pcs) 
{
  long i;
  for (i=0;i<ElListSize();i++) {
    MMSCalcElementMatrix(i,m_pcs);
  }
}
/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
**************************************************************************/
void MMSCalcElementMatrix(long index,CRFProcess*m_pcs)
{
  switch (ElGetElementType(index))
    {
    case 1:
      CalcEle1D(index,m_pcs);
      break;
    case 2:
      CalcEle2D(index,m_pcs);
      break;
    case 3:
      CalcEle3D(index,m_pcs);
      break;
    case 4:
      //MMPCalcElementMatrices2DTriangle(index, phase, matrixtyp,m_pcs); /* OK 3805 */
      break;
    }
}


/**************************************************************************
   ROCKFLOW - Funktion: CalcEle1D

   Aufgabe:
   Berechnet die Massenmatrix des angegebenen 1D - Elements
   ohne 1/dt fuer v=const (SUPG)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

   Ergebnis:
   - void -

   Programmaenderungen:
   9/97     C.Thorenz      Erste Version
   5/1999   C.Thorenz      Umbau auf n-Phasen
   4/2000   C.Thorenz      SUPG auf alle Phasen erweitert
   9/2000   C.Thorenz      ML-FEM
 08/2004 OK MFP implementation

 **************************************************************************/
static void CalcEle1D(long index,CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  static long i;
  /* Elementmatrix */
  static double *mass;          /* Massenmatrix */
  /* Jakobi */
  static double *invjac, detjac;
  /* Elementdaten */
  static double zeta;           /* SUPG */
  static double v[3],v_tot[3],l[3];
  static double vorfk;
  static long *element_nodes;
  /* Materialdaten */
  static double porosity;
  double gp[3]; 

#ifdef TESTSUPG
  DisplayMsg("1D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif
  int no_phases = (int)mfp_vector.size(); //OK
  /* Daten bereitstellen */
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  /* Speicherplatzreservierung fuer Elementmatrizen */
  mass = MMSGetElementSaturationMassMatrix(index);
  if (!mass)
    mass = (double *) Malloc(4 * sizeof(double));
  /* Initialisieren */
  MNulleMat(mass, 2, 2);
  /* Elementdaten und globale Modellparameter bereitstellen */
  //----------------------------------------------------------------------
  // NUM
  mms_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mms_upwind_parameter>0.0) 
    mms_upwind_method = 1;
  mms_mass_lumping_method = m_pcs->m_num->ele_mass_lumping;
  mms_time_collocation_global = m_pcs->m_num->ls_theta;
  mms_time_collocation_upwind = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // Medium properties 
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
         gp[0] = 0.0;
         gp[1] = 0.0; 
         gp[2] = 0.0;

//MX  porosity = m_mmp->Porosity(index,NULL,1.);
  porosity = m_mmp->Porosity(index,gp,1.);

  /* Numerik */
  zeta = 0.;
  /* SUPG */
  if (mms_upwind_method)
    {
      MNulleVec(v_tot,3);
      /* Abstandsgeschwindigkeit einer sich bewegenden Phase */
      for(i=0;i<no_phases;i++) {
         //CalcVelo1Dr(i,index,mms_time_collocation_upwind,0.,0.,0.,v);
         gp[0] = 0.0;
         gp[1] = 0.0; 
         gp[2] = 0.0;
         VELCalcGaussLineLocal(i, index, gp, m_pcs, v);

         v_tot[0] += v[0];
         v_tot[1] += v[1];
         v_tot[2] += v[2];
      }

      l[0] = GetNodeX(element_nodes[1]) - GetNodeX(element_nodes[0]);
      l[1] = GetNodeY(element_nodes[1]) - GetNodeY(element_nodes[0]);
      l[2] = GetNodeZ(element_nodes[1]) - GetNodeZ(element_nodes[0]);

      if (MBtrgVec(v_tot, 3) > MKleinsteZahl)
        {
          if (MSkalarprodukt(v_tot, l, 3) > 0.)
            zeta = 1.;
          else
            zeta = -1.;
        }
    }
  /* detjac = A*L/2 */
  vorfk = porosity * detjac * Mdrittel;

  /* Massenmatrix mit SUPG ohne Zeitanteile */
  mass[0] = (2.0 + 1.5 * mms_upwind_parameter * zeta) * vorfk;
  mass[1] = (1.0 + 1.5 * mms_upwind_parameter * zeta) * vorfk;
  mass[2] = (1.0 - 1.5 * mms_upwind_parameter * zeta) * vorfk;
  mass[3] = (2.0 - 1.5 * mms_upwind_parameter * zeta) * vorfk;


  /* Wenn die Speicherung per ML-FEM
     behandelt wird, wird nur die Diagonale gesetzt */
  if (mms_mass_lumping_method)
    {
      MNulleMat(mass, 2, 2);
      for (i = 0; i < 2; i++)
        mass[3 * i] = 0.5 * ElGetElementVolume(index) * porosity;
    }
  MMSSetElementSaturationMassMatrix(index, mass);

#ifdef TESTCEL_MMS
  MZeigMat(mass, 2, 2, "SaturationMassMatrix");
#endif
}

/**************************************************************************
   ROCKFLOW - Funktion: CalcEle2D

   Aufgabe:
   Berechnet die Massenmatrix ohne 1/dt (nur zeitunabhaengigen Anteil),
   die dispersive und die advektive Matrix ohne theta (nur zeitunabhaengige
   Anteile) fuer das angegebene Element

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

   Ergebnis:
   - void -

   Programmaenderungen:
   07/1995     Hans Herrmann        Erste Version
   11/1995     msr                  ueberarbeitet
   03/1996     cb                   Porositaet
   05/1996     cb                   zweite Version
   27.06.1997  R.Kaiser             Anpassung an die Funktion
                                    CalcElementJacobiMatrix (femlib.c)
   4/2000   C.Thorenz      SUPG auf alle Phasen erweitert
   9/2000   C.Thorenz      ML-FEM

 **************************************************************************/
static void CalcEle2D(long index,CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i, j, l;
  static long ind;
  static double r, s;
  /* Elementgeometriedaten */
  static double invjac[4], jacobi[4], detjac;
  static double phi[4], omega[4];
  /* Elementmatrizen */
  static double *mass;          /* Massenmatrix */
  /* Elementdaten */
  static int anzgp;
  static double alpha[2];       /* SUPG */
  static double v[2], v_tot[2], v_rs[2];
  static double fkt, area;
  /* Materialdaten */
  static double porosity;
  /* Hilfsfelder */
  static double zwi[16];
  //----------------------------------------------------------------------
  // NUM
  mms_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mms_upwind_parameter>0.0) 
    mms_upwind_method = 1;
  mms_mass_lumping_method = m_pcs->m_num->ele_mass_lumping;
  mms_time_collocation_global = m_pcs->m_num->ls_theta;
  mms_time_collocation_upwind = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
#ifdef TESTSUPG
  DisplayMsg("2D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif
  int no_phases = (int)mfp_vector.size(); //OK
  /* Speicherplatzreservierung fuer Elementmatrizen */
  mass = MMSGetElementSaturationMassMatrix(index);
  if(!mass)
    mass = (double *) Malloc(16 * sizeof(double));
  MNulleMat(mass, 4, 4);
  MNulleVec(alpha, 2);
  //----------------------------------------------------------------------
  // Geometry
  area = m_mmp->geo_area;
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  //----------------------------------------------------------------------
  // Medium properties 
  group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  double gp[3]={0.,0.,0.};
  porosity = m_mmp->Porosity(index,gp,mms_time_collocation_global);
  //----------------------------------------------------------------------
  // SUPG
  if(mms_upwind_method){
    MNulleVec(v_tot,2);
    // Abstandsgeschwindigkeit einer sich bewegenden Phase 
    for(i=0;i<no_phases;i++){
      
      //CalcVelo2Drs(i, index, mms_time_collocation_upwind, 0., 0., 0., v); MB
      VELCalcGaussQuadLocal(i, index, gp, m_pcs, v);
      
      v_tot[0] += v[0];
      v_tot[1] += v[1];
    }
    // Geschwindigkeitstransformation: a,b -> r,s
    Calc2DElementJacobiMatrix(index, 0., 0., invjac, &detjac);
    MKopierVec(invjac, jacobi, 4);
    M2InvertiereUndTransponiere(jacobi);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jacobi-Matrix */
    MMultMatVec(jacobi, 2, 2, v_tot, 2, v_rs, 2);
    if(MBtrgVec(v_rs, 2) > MKleinsteZahl){
      // Upwind-Faktoren
      for(l=0;l<2;l++)
        alpha[l] = -mms_upwind_parameter * v_rs[l] / (MBtrgVec(v_rs, 2) + MKleinsteZahl);
    }
  }
  //======================================================================
  // Schleife ueber Gauss-Punkte
  ind = -1;
  for(i=0;i<anzgp;i++){
    for(j=0;j<anzgp;j++){
      r = MXPGaussPkt(anzgp, i);
      s = MXPGaussPkt(anzgp, j);
      Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
      fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
      // Massenmatrix 
      // Ansatzfunktionen auswerten
      MOmega2D(omega, r, s);
      if(mms_mass_lumping_method){
        // Wenn die Speicherung per ML-FEM behandelt wird, wird nur die Diagonale gesetzt
        for(l=0;l<4;l++)
          mass[l * 5] += omega[l] * fkt;
      }
      else{
        // Normale FEM
        // phi * omega
        MPhi2D_SUPG(phi, r, s, alpha);
        MMultVecVec(phi, 4, omega, 4, zwi, 4, 4);
        for(l=0;l<16;l++)
          mass[l] += (zwi[l] * fkt);
      }
    } // Ende der Schleife ueber GaussPunkte
  } // Ende der Schleife ueber GaussPunkte
  for(l=0;l<16;l++)
    mass[l] *= porosity * area;
  MMSSetElementSaturationMassMatrix(index, mass);
#ifdef TESTCEL_MMS
  MZeigMat(mass, 4, 4, "SaturationMassMatrix");
#endif
}

/**************************************************************************
  ROCKFLOW - Funktion: CalcEle3D

  Aufgabe:
  Berechnet die Massenmatrix des angegebenen 1D - Elements
  ohne 1/dt fuer v=const (SUPG)

  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E long index: Index des Elements

  Ergebnis:
  - void -

  Programmaenderungen:
  5/99    C.Thorenz        Erste Version
  4/2000   C.Thorenz      SUPG auf alle Phasen erweitert
  9/2000   C.Thorenz      ML-FEM

**************************************************************************/
static void CalcEle3D(long index,CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i, j, k, l;
  static long ind;
  static double r, s, t;
  /* Elementgeometriedaten */
  static double invjac[9], jacobi[9], detjac;
  static double phi[8], omega[8];
  /* Elementmatrizen */
  static double *mass;          /* Massenmatrix */
  /* Elementdaten */
  static int anzgp;
  static double alpha[3];       /* SUPG */
  static double v[3], v_tot[3], v_rst[3];
  static double fkt;
  /* Materialdaten */
  static double porosity;
  /* Hilfsfelder */
  static double zwi[64];
  static double gp[]={0.,0.,0.};

#ifdef TESTSUPG
  DisplayMsg("3D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif
  //----------------------------------------------------------------------
  // NUM
  //----------------------------------------------------------------------
  // NUM
  mms_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mms_upwind_parameter>0.0) 
    mms_upwind_method = 1;
  mms_mass_lumping_method = m_pcs->m_num->ele_mass_lumping;
  mms_time_collocation_global = m_pcs->m_num->ls_theta;
  mms_time_collocation_upwind = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  //----------------------------------------------------------------------
  // MFP medium properties
  int no_phases = (int)mfp_vector.size(); //OK
  //----------------------------------------------------------------------
  /* Speicherplatzreservierung fuer Elementmatrizen */
  mass = MMSGetElementSaturationMassMatrix(index);
  if (!mass)
    mass = (double *) Malloc(64 * sizeof(double));
  /* Initialisieren */
  MNulleMat(mass, 8, 8);
  MNulleVec(alpha, 3);
  /* Elementdaten und globale Modellparameter bereitstellen */
  porosity = m_mmp->Porosity(index,NULL,mms_time_collocation_global);
  group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  porosity =  m_mmp->Porosity(index,gp,mms_time_collocation_global);
  /* Numerik */
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  /* SUPG */
  if (mms_upwind_method)
    {
      MNulleVec(v_tot,3);
      /* Abstandsgeschwindigkeit einer sich bewegenden Phase */
      for(i=0;i<no_phases;i++){
         
        //CalcVelo3Drst(i, index, mms_time_collocation_upwind, 0., 0., 0., v);
        gp[0]= 0.0;
        gp[1]= 0.0;
        gp[2]= 0.0;
        VELCalcGaussHexLocal(i, index, gp, m_pcs, v);
         
         v_tot[0] += v[0];
         v_tot[1] += v[1];
         v_tot[2] += v[2];
      }

      if (MBtrgVec(v_tot, 3) > MKleinsteZahl)
        {
          /* Geschwindigkeitstransformation: x,y,z -> r,s,t */
          Calc3DElementJacobiMatrix(index, 0., 0., 0., invjac, &detjac);
          MKopierVec(invjac, jacobi, 9);
          M3Invertiere(jacobi);        /* Jacobi-Matrix */
          MMultMatVec(jacobi, 3, 3, v_tot, 3, v_rst, 3);

          /* Upwind-Faktoren */
          for (l = 0; l < 3; l++)
            alpha[l] = -mms_upwind_parameter * v_rst[l] / (MBtrgVec(v_rst, 3) + MKleinsteZahl);;
        }
    }
/***************************************************************************/
/* Schleife ueber Gauss-Punkte
   ************************************************************************* */
  ind = -1;
  for (i = 0; i < anzgp; i++)
    {
      for (j = 0; j < anzgp; j++)
        {
          for (k = 0; k < anzgp; k++)
            {
              r = MXPGaussPkt(anzgp, i);
              s = MXPGaussPkt(anzgp, j);
              t = MXPGaussPkt(anzgp, k);

              /* Faktoren */
              Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
              fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * fabs(detjac);

              /* Massenmatrix */
              /* Ansatzfunktionen auswerten */
              MOmega3D(omega, r, s, t);
 
              if (mms_mass_lumping_method)
                {
                  /* Wenn die Speicherung per ML-FEM
                     behandelt wird, wird nur die Diagonale gesetzt */
                  for (l = 0; l < 8; l++)
                    mass[l * 9] += omega[l] * fkt;
                }
              else
                {
                  /* Normale FEM */
                  /* phi * omega */
                  MPhi3D_SUPG(phi, r, s, t, alpha);
                  MMultVecVec(phi, 8, omega, 8, zwi, 8, 8);
                  for (l = 0; l < 64; l++)
                    mass[l] += (zwi[l] * fkt);
                }
            }                          /* Ende der Schleife ueber GaussPunkte */
        }                              /* Ende der Schleife ueber GaussPunkte */
    }                                  /* Ende der Schleife ueber GaussPunkte */

  for (l = 0; l < 64; l++)
    mass[l] *= porosity;

  MMSSetElementSaturationMassMatrix(index, mass);

#ifdef TESTCEL_MMS
  MZeigMat(mass, 8, 8, "SaturationMassMatrix");
#endif

}



/**************************************************************************
   ROCKFLOW - Funktion: MMSDampOscillations

   Aufgabe:
   
   Daempft Oszillationen des Feldes oder begrenzt den Wertebereich.

   Ergebnis:
   - void -

   Programmaenderungen:
   07/2002   CT   Erste Version

  **************************************************************************/
void MMSDampOscillations(int ndx1)
{
   DampOscillations(ndx1, mms_oscil_damp_method, mms_oscil_damp_parameter, MMSNodeCalcLumpedMass);
}


/**************************************************************************
   ROCKFLOW - Funktion:  MMSNodeCalcLumpedMass

   Aufgabe:
   Liefert fuer einen Knoten die "gelumpte" Speichermatrix

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB

   Ergebnis:
     double mass : Rueckgabe, des Lumped-Wertes

   Programmaenderungen:
   7/2002 C.Thorenz Erste Version

 **************************************************************************/
double MMSNodeCalcLumpedMass(long knoten)
{
  double *mass=NULL, lm=0.;
  long *elemente, *nodes;
  int anz_elemente, i, j, k, nn;

  elemente = GetNodeElements(knoten, &anz_elemente);

  for (i = 0l; i < anz_elemente; i++) {
    nn = ElNumberOfNodes[ElGetElementType(elemente[i]) - 1];
    
    mass = MMSGetElementSaturationMassMatrix(elemente[i]);
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


/*------------------------------------------------------------------------*/
/* ElementSaturationMassMatrix */
double *MMSGetElementSaturationMassMatrix_MMS(long number)
{
  return ((MMSElementMatrices *) ELEGetElementMatrices(number,mms_pcs_number))->saturation_mass_matrix;
}
void MMSSetElementSaturationMassMatrix_MMS(long number, double *matrix)
{
  ((MMSElementMatrices *) ELEGetElementMatrices(number,mms_pcs_number))->saturation_mass_matrix = matrix;
}



double *MMSGetElementCapacitanceMatrix_MMS(long number,int phase)
{
  int process = 0;
  return ((MMPElementMatrices *) ELEGetElementMatricesPCS(number,process))->mmp_capacitance_matrix_phase[phase];
}

double *MMSGetElementCapillarityVector_MMS(long number,int phase)
{
  int process = 0;
  return ((MMPElementMatrices *) ELEGetElementMatricesPCS(number,process))->mmp_capillarity_vector_phase[phase];
}

double *MMSGetElementGravityVector_MMS(long number,int phase)
{
  int process = 0;
  return ((MMPElementMatrices *) ELEGetElementMatricesPCS(number,process))->mmp_gravity_vector_phase[phase];
}

double *MMSGetElementConductivityMatrixPhase_MMS(long number, int phase)
{
  int process = 0;
  return ((MMPElementMatrices *) ELEGetElementMatricesPCS(number,process))->mmp_conductivity_matrix_phase[phase];
}

/*************************************************************************
ROCKFLOW - Function: MMSDestroyELEMatricesPointer
Task: Create element matrices pointer
Programming: 06/2003 OK Implementation
last modified:
**************************************************************************/
void *MMSDestroyELEMatricesPointer(void *data)
{
  if (((MMSElementMatrices *) data)->saturation_mass_matrix)
       ((MMSElementMatrices *) data)->saturation_mass_matrix = \
         (double *) Free(((MMSElementMatrices *) data)->saturation_mass_matrix);
  data = (void*) Free(data);
  return data;
}

/*************************************************************************
ROCKFLOW - Function: MMSCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 06/2003 OK Implementation
last modified:
**************************************************************************/
void *MMSCreateELEMatricesPointer(void)
{ 
  MMSElementMatrices *data = NULL;

  data = (MMSElementMatrices *) Malloc(sizeof(MMSElementMatrices));

  data->saturation_mass_matrix = NULL;

  return (void *) data;
}

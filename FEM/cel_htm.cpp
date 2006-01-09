/**************************************************************************
   ROCKFLOW - Modul: cel_htm.c

   Aufgabe:
   Berechnung der Elementmatrizen fuer Waermetransport

   Programmaenderungen:
   10/1997   OK   Implementierung des Waermetransportmodells in ATM
   04/1999   OK   Kapselung des Waermetransportmodells (mod_0093)
   03/2000   RK   Anpassung an Gitteradaption
   11/2000   CT   Erste Version fuer Mehrphasen
   11/2000   CT   Kernel-Konfiguration
   01/2001   CT   Patch-Integriertes Masslumping, Zwischensumme sum => schneller!
   04/2002   OK   Dreieckselemente
                  neue MAT Funktionen eingebaut
   03/2003 RK model substituted by process control parameters
   05/2003 OK Access functions for element matrices
   05/2003 OK HTMCreateELEMatricesPointer, HTMDestroyELEMatricesPointer
   05/2003 OK CalcAllElementMatrices_HTM: only new keywords
   08/2004 MB Numerische Lösung für Dreiecksprismen 
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation        
11/2004 OK NUM Implementation
 **************************************************************************/
#include "stdafx.h"                    /* MFC */
/*------------------------------------------------------------------------*/
/* Preprozessor-Definitionen */
#define noTESTSUPG
#define noTESTCEL_HTM
#define noCOUNTER_CEL_HTM              /* Elementzahl-Ausgabe, fuer grosse Probleme */
/*------------------------------------------------------------------------*/
#include "makros.h"
#include "cel_htm.h"
#include "int_htm.h"
/* Allgemeine Methoden */
#include "tools.h"
#include "mathlib.h"
#include "cvel.h"                      /*CECalcGaussVelo Calc2DElementJacobiMatrix */
#include "rf_vel_new.h"     
#include "intrface.h"                  /*GetElementJacobiMatrix */
/* Objekte */
#include "nodes.h"
#include "elements.h"
#include "rf_num_new.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "rf_msp_new.h"
#include "rf_pcs.h"
using SolidProp::CSolidProperties;
/*------------------------------------------------------------------------*/
/* Interne (statische) Deklarationen */
void HTMCalcElementMatrices1D(long,double*,double*,double*,CRFProcess*);
void HTMCalcElementMatrices2D(long,double*,double*,double*,CRFProcess*);
void HTMCalcElementMatrices3D(long,double*,double*,double*,CRFProcess*);
void HTMCalcElementMatrices2DTriangle(long,double*,double*,double*,CRFProcess*);
void HTMCalcElementMatrices3DPrismNum(long,double*,double*,double*,CRFProcess*);
void HTMCalcElementMatrices3DTet(long,double*,double*,double*,CRFProcess*);

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

/* Bedingter Wiederaufbau */
static int htm_matrixrebuild_read = 0;
static int htm_number_rebuild_info;
static Rebuild_info *htm_rebuild_info;
//static int htm_rebuild_array_size = 0;

/* Vorkonfigurieren des Kernels */
static int htm_new_kernel = 0;
static int htm_method = 1;

//static int htm_timecollocation_array_size = 0;
static double htm_time_collocation_global = 1.;
//static double htm_time_collocation_cond_bc = 1.;
//static double htm_time_collocation_source = 1.;
static double htm_time_collocation_upwinding = 0.;
static int htm_nonlinear_coupling = 1;

static int htm_mass_lumping_method = 0;
//static double htm_mass_lumping_parameter = 0.;

static int htm_predictor_method = 0;
static double htm_predictor_parameter = 0.;

static int htm_relaxation_method = 0;
static double htm_relaxation_parameter[16];

//static int htm_upwind_method = 1;
static double htm_upwind_parameter = 1.;

static int htm_oscil_damp_method = 0;
static double htm_oscil_damp_parameter[16];

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_HTM

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version
**************************************************************************/
/**************************************************************************
 ROCKFLOW - Funktion: GetXXX_HTM

 Aufgabe:
   Gibt div. Kernelparameter zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   11/2000   C.Thorenz    Erste Version
   06/2006   MK           TEMPERATURE0 als ID fuer GetNumericalTimeCollocation
              
**************************************************************************/
double GetTimeCollocationGlobal_HTM(void)
{
  if (htm_new_kernel)
    return htm_time_collocation_global;
  else
    return GetNumericalTimeCollocation("TEMPERATURE0");
}
int GetMethod_HTM(void)
{
  return htm_method;
}
int GetNonLinearCoupling_HTM(void)
{
  return htm_nonlinear_coupling;
}
int GetPredictorMethod_HTM(void)
{
  return htm_predictor_method;
}
double GetPredictorParam_HTM(void)
{
  return htm_predictor_parameter;
}
int GetRelaxationMethod_HTM(void)
{
  return htm_relaxation_method;
}
double *GetRelaxationParam_HTM(void)
{
  return htm_relaxation_parameter;
}
/**************************************************************************
 ROCKFLOW - Funktion: ConditionalRebuildMatrices_HTM

 Aufgabe:
   Matrizen zur Berechnung kennzeichnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementindex

 Ergebnis:
   - void -

 Programmaenderungen:
    4/2002    C.Thorenz     Erste Version

**************************************************************************/
void ConditionalRebuildMatrices_HTM(int phase, int comp)
{
  long i, element;
  int *element_rebuild, e_index, e_index_ref, e_index_cur;
  double v[3], v_alt[3], error;
  Rebuild_info *rebuild_info;
  int num_rebuild_info = 0, found_velo = 0;

  /* Feld fuer das Markieren der Elemente */
  element_rebuild = (int *) Malloc(ElListSize() * sizeof(int));

  /* Eintraege setzen */
  for (i = 0; i < ElListSize(); i++)
    element_rebuild[i] = 0;            /* 0: Keine Info vorhanden */

  /* Die aktuellen Werte bestimmen und abspeichern */
  for (num_rebuild_info = 0; num_rebuild_info < htm_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &htm_rebuild_info[num_rebuild_info];

      /* Nicht NUll-Methode ? */
      if (rebuild_info -> method)
        {

          if (!strcmp("ELEMENT_VELOCITY_CHANGE", rebuild_info -> name))
            {
              if ((rebuild_info -> phase == -1) || (rebuild_info -> phase == phase))
                {
                  e_index = rebuild_info -> index_for_current_values[phase * 3];

                  for (i = 0; i < anz_active_elements; i++)
                    {
                      element = ActiveElements[i];

                      MNulleVec(v, 3);
                      CalcVeloXDrst(phase, element, 0, 0., 0., 0., v);

                      ElSetElementVal(ActiveElements[i], e_index, v[0]);
                      ElSetElementVal(ActiveElements[i], e_index + 1, v[1]);
                      ElSetElementVal(ActiveElements[i], e_index + 2, v[2]);
                    }
                }                      /* endif */
            }
        }
    }                                  /* endfor */



  /* Die gespeicherten Referenzwerte mit Ist-Werten und Limits vergleichen */
  for (num_rebuild_info = 0; num_rebuild_info < htm_number_rebuild_info; num_rebuild_info++)
    {

      /* Zeiger holen */
      rebuild_info = &htm_rebuild_info[num_rebuild_info];

      if (!strcmp("ELEMENT_VELOCITY_CHANGE", rebuild_info -> name))
        {
          if ((rebuild_info -> phase == -1) || (phase == rebuild_info -> phase))
            {
              if ((rebuild_info -> comp == -1) || (comp == rebuild_info -> comp))
                {
                  found_velo = 1;
                  e_index_ref = rebuild_info -> index_for_reference_values[phase * 3];
                  e_index_cur = rebuild_info -> index_for_current_values[phase * 3];

                  for (i = 0; i < anz_active_elements; i++)
                    {
                      element = ActiveElements[i];

                      v[0] = ElGetElementVal(element, e_index_cur);
                      v[1] = ElGetElementVal(element, e_index_cur + 1);
                      v[2] = ElGetElementVal(element, e_index_cur + 2);

                      v_alt[0] = ElGetElementVal(element, e_index_ref);
                      v_alt[1] = ElGetElementVal(element, e_index_ref + 1);
                      v_alt[2] = ElGetElementVal(element, e_index_ref + 2);

                      switch (rebuild_info -> method)
                        {
                        case 1:       /* Differenzen der Geschw. */
                          error = MVekDist(v, v_alt, 3);
                          if (error > rebuild_info -> param[0])
                            /* Neuaufbau noetig */
                            element_rebuild[element] = 1;
                          break;

                        case 2:       /* Rel. Differenzen der Geschw. */
                          error = MVekDist(v, v_alt, 3) / (MBtrgVec(v, 3) + MBtrgVec(v_alt, 3) + MKleinsteZahl);
                          if (error > rebuild_info -> param[0])
                            /* Neuaufbau noetig */
                            element_rebuild[element] = 1;
                          break;

                          /* Mit Zufallsaufbau! */
                        case 11:      /* Differenzen */
                          error = MVekDist(v, v_alt, 3);
                          if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                            /* Neuaufbau */
                            element_rebuild[element] = 1;
                          break;

                        case 12:      /* Rel. Differenzen  */
                          error = MVekDist(v, v_alt, 3) / (MBtrgVec(v, 3) + MBtrgVec(v_alt, 3) + MKleinsteZahl);
                          if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                            /* Neuaufbau */
                            element_rebuild[element] = 1;
                          break;

                        default:
                          break;
                        }              /* end-switch */
                    }                  /* end-for */
                }                      /* end-if */
            }                          /* end-if */
        }                              /* end-if */
    }                                  /* endfor */





  /* Jetzt liegt fuer jede Matrix ggf. ein Eintrag vor. Es werden fuer
     die neu aufzubauenden Matrizen alle zugehoerigen Referenzwerte abgelegt. */

  /* Ueber alle Eintraege gehen */
  for (num_rebuild_info = 0; num_rebuild_info < htm_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &htm_rebuild_info[num_rebuild_info];
      if (rebuild_info -> method)
        {
          if (!strcmp("ELEMENT_VELOCITY_CHANGE", rebuild_info -> name))
            {
              e_index_ref = rebuild_info -> index_for_reference_values[phase * 3];
              e_index_cur = rebuild_info -> index_for_current_values[phase * 3];
              for (i = 0; i < anz_active_elements; i++)
                {
                  element = ActiveElements[i];
                  /* Referenzdaten abspeichern */
                  if (element_rebuild[element])
                    {
                      ElSetElementVal(element, e_index_ref, ElGetElementVal(element, e_index_cur));
                      ElSetElementVal(element, e_index_ref + 1, ElGetElementVal(element, e_index_cur + 1));
                      ElSetElementVal(element, e_index_ref + 2, ElGetElementVal(element, e_index_cur + 2));
                    }
                }
            }

        }
    }                                  /* endfor */



  /* Jetzt werden die Matrizen wirklich aufgebaut */
  /* Auswerten der gekennzeichneten Elemente */
  for (i = 0; i < anz_active_elements; i++)
    {
      element = ActiveElements[i];
      /* Aufbauen: Wenn Eintraege fehlen, im ersten Zeitschritt ... */
      if (element_rebuild[element]
          || (aktueller_zeitschritt < 2)
          || (htm_matrixrebuild_read == 0))
        {
            //OK HTMCalcElementMatrices_Plus(element);
        }
    }

  element_rebuild = (int *) Free(element_rebuild);
}

/*=======================================================================*/
/* Element-Matrizen */
/*=======================================================================*/

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
04/1999 OK erste Implementierung
11/2000 CT Kernel-Konfiguration
05/2003 OK only new keywords
11/2004 OK ReImplementation
**************************************************************************/
void HTMCalcElementMatrix(long index,CRFProcess *m_pcs)
{
    switch(ElGetElementType(index)){
        case 1:
        HTMCalcElementMatrices1D(index,NULL,NULL,NULL,m_pcs);
        break;
    case 2:
        HTMCalcElementMatrices2D(index,NULL,NULL,NULL,m_pcs);
        break;
    case 3:
        HTMCalcElementMatrices3D(index,NULL,NULL,NULL,m_pcs);
        break;
    case 4:
        HTMCalcElementMatrices2DTriangle(index,NULL,NULL,NULL,m_pcs);
        break;
    case 5:
        HTMCalcElementMatrices3DTet(index,NULL,NULL,NULL,m_pcs);
        break;
    case 6:
        HTMCalcElementMatrices3DPrismNum(index,NULL,NULL,NULL,m_pcs);
        break;
    }
}

void HTMCalcElementMatrices(CRFProcess *m_pcs)
{
  long i;
  Element* element = NULL;
  for (i=0;i<ElListSize();i++){
    element = ElGetElement(i);
    if(element&&element->aktiv)
      HTMCalcElementMatrix(i,m_pcs);
  }
}
/**************************************************************************
 ROCKFLOW - Funktion: HTMCalcElementMatrices1D

 Aufgabe:
   1-D Elementmatrizen fuer Waermetransport

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   04/1999   OK   erste Implementierung
   01/2001   CT   Masslumping
   04/2002   OK   neue MAT Funktionen eingebaut (mehrphasenfaehig)
   06/2003 OK Bugfix: Memory allocation of element matrices
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation
11/2004 OK H2 compatibility
11/2004 OK NUM Implementation
**************************************************************************/
void HTMCalcElementMatrices1D(long index,double*mass,double*disp,double*adv,\
                              CRFProcess*m_pcs)
{
  /* Materialdaten */
  static double porosity;
  static double heat_capacity_fluids,heat_capacity_fluid;
  static double heat_capacity_porous_medium;
  static double *heat_dispersion_tensor;
  /* Geometriedaten */
  static double *invjac, detjac;
  /* Elementdaten */
  static double v[1], vt, q, qt;
  static double *velovec;
  static double vorfk;
  static double zeta, pec, alpha[3];    /* Upwind-Parameter */
  static int nn = 2;
  static int nn2 = 4;
  double gp[3]={0.,0.,0.};
  double theta=1.0;
  //----------------------------------------------------------------------
  /* Speicher-Modelle fuer Element-Matrizen */
  if (memory_opt==0) /* keine Speicheroptimierung */
    {        
      mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
      if (!mass)
        mass = (double *) Malloc(nn2 * sizeof(double));
      disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
      if (!disp)
        disp = (double *) Malloc(nn2 * sizeof(double));
      adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
      if (!adv)
        adv = (double *) Malloc(nn2 * sizeof(double));
    }
  MNulleMat(mass, nn, nn);
  MNulleMat(disp, nn, nn);
  MNulleMat(adv, nn, nn);
  //----------------------------------------------------------------------
  // GEO
  invjac = GetElementJacobiMatrix(index, &detjac);      /* detjac= AL/2 */
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  //--------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  porosity = m_mmp->Porosity(index,gp,theta);
  heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
  //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0);
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);//CMCD 4213
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // MFP fluid properties
  heat_capacity_fluids = porosity* MFPCalcFluidsHeatCapacity(index,gp,theta);
  CFluidProperties* m_mfp = NULL;
  int no_phases = (int)mfp_vector.size();
  int phase;
  // VEL
  velovec = ElGetVelocity(index);
  q = MBtrgVec(velovec,3);
  v[0] = q / porosity;
  qt = MSkalarprodukt(velovec, invjac, 3);
  vt = qt / porosity;
  /*---------------------------------------------------------------------*/
// ToDo encapsulate
  /* Numerische Verfahren */
   /* Taylor-Galerkin nach Donea */
#ifdef TESTTAYLOR
  printf("TESTTAYLOR, dt=%e\n", dt);
  alpha0 = 0.0;                        /* kein SU/PG */
  ElSetArtDiff(index, (-dt / 6.0 * vt * vt * porosity * (detjac * 0.5)));
#endif
   /* SUPG */
  alpha[0] = 0.0; /* alpha initialisieren */
  if (v[0] > MKleinsteZahl && htm_upwind_parameter > 0.0)
    {
      /* Berechnung von zeta */
      zeta = vt / fabs(vt);            /* reine Advektion */
      if (heat_dispersion_tensor[0] > MKleinsteZahl)
        {
          /* halbe Peclet-Zahl mit ds=2.0 */
          pec = vt / heat_dispersion_tensor[0];
          zeta = 0.0;                  /* reine Diffusion */
          if (fabs(pec) > 1.0)
            zeta = (fabs(pec) - 1.0) / pec;
        }
      /* Upwind-Faktor */
      alpha[0] = htm_upwind_parameter * zeta;
#ifdef TESTSUPG
      DisplayDoubleVector(alpha, 1, "alpha=");
#endif
    }
  /*alpha[0] = NUM_SUPG_1D(v,vt,heat_dispersion_tensor,tdt,htm_upwind_parameter);*/
  /*---------------------------------------------------------------------*/
  /* Elementmatrizen berechnen */
  /*---------------------------------------------------------------------*/
  /* Massenmatrix mit SUPG ohne Zeitanteile */
  vorfk = detjac * Mdrittel * heat_capacity_porous_medium; /* detjac=AL/2 */
  mass[0] = (2.0 - 1.5 * alpha[0]) * vorfk;
  mass[1] = (1.0 - 1.5 * alpha[0]) * vorfk;
  mass[2] = (1.0 + 1.5 * alpha[0]) * vorfk;
  mass[3] = (2.0 + 1.5 * alpha[0]) * vorfk;
  // Mass lumping FEM
  if (htm_mass_lumping_method == 1){
    mass[0] += mass[1];
    mass[1] = 0.;
    mass[3] += mass[2];
    mass[2] = 0.;
  }
#ifdef TESTCEL_HTM
  MZeigMat(mass,nn,nn,"HTMCalcElementMatrices1D: Heat capacitance mass");
#endif
  /*---------------------------------------------------------------------*/
  /* Dispersionsmatrix ohne Zeitanteile */
  vorfk = (detjac * 0.5) * heat_dispersion_tensor[0];
  disp[0] = disp[3] = vorfk;
  disp[1] = disp[2] = -vorfk;
#ifdef TESTCEL_HTM
  MZeigMat(disp,nn,nn,"HTMCalcElementMatrices1D: Heat diffusion-dispersion matrix");
#endif
  /*---------------------------------------------------------------------*/
  /* Advektive Matrix mit SUPG ohne Zeitanteile */
  //OK vorfk = heat_capacity_fluids * vt * detjac * 0.5; /* OK 3805 */
  vorfk = 0.0;
  char saturation_name[15];
  double saturation;
  for(phase=0;phase<no_phases;phase++){
    m_mfp = mfp_vector[phase];
    if(no_phases==1){
      saturation = 1.0;
    }
    else{
      sprintf(saturation_name,"SATURATION%i",phase+1);
      saturation = PCSGetELEValue(index,gp,theta,saturation_name);
    }
    heat_capacity_fluid = porosity * saturation \
                        * m_mfp->Density() \
                        * m_mfp->SpecificHeatCapacity();
    velovec = ElGetVelocityNew(index,phase);
    qt = MSkalarprodukt(velovec, invjac, 3);
    vt = qt / porosity;
    vorfk += heat_capacity_fluid * vt * detjac * 0.5;
  }
  alpha[0] = 0.0;
  adv[0] = vorfk * (-1.0 + alpha[0]);
  adv[1] = vorfk * (1.0 - alpha[0]);
  adv[2] = vorfk * (-1.0 - alpha[0]);
  adv[3] = vorfk * (1.0 + alpha[0]);
#ifdef TESTCEL_HTM
  MZeigMat(adv,nn,nn,"HTMCalcElementMatrices1D: Heat advection matrix");
#endif
  /*---------------------------------------------------------------------*/
  /* Element-Matrizen speichern oder uebergeben */
  if (memory_opt == 0){ 
    HTMSetElementHeatCapacitanceMatrix(index,mass,m_pcs->pcs_number);
    HTMSetElementHeatDiffusionMatrix(index,disp,m_pcs->pcs_number);
    HTMSetElementHeatAdvectionMatrix(index,adv,m_pcs->pcs_number);
  }
}


/**************************************************************************
 ROCKFLOW - Funktion: Calc2DElementMatrices_HTM

 Aufgabe:
   2-D Elementmatrizen fuer Waermetransport

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   04/1999   OK   erste Implementierung
   11/2000   OK   Speichermodelle (memory_opt)
   01/2001   CT   Patch-Integriertes Masslumping, Zwischensumme sum => schneller!
   04/2002   OK   neue MAT Funktionen eingebaut (mehrphasenfaehig)
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation                
11/2004 OK NUM Implementation
**************************************************************************/
void HTMCalcElementMatrices2D(long index, double *mass, double *disp, double *adv,\
                              CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i, j, k;
  static double r, s;           /* Gausspunkte */
  static int ind;
  /* Koordinatentransformationen */
  static double detjac,invjac[4],invjac_t[4];
  // static double dreh[4];
  /* Wichtungs- und Interpolationsfunktionen */
  static double zwa[8], zwi[16];
  static double omega[4], grad_ome[8];
  static double phi[4], grad_phi[8], grad_phi_T[8];
  /* Numerik */
  static int anzgp;
  static int supg = 1;          /* TODO */
  static double alpha[3];       /* SUPG */
  static double zeta[2];
  static double pec[2];
  static double fkt, sum;
  /* Materialdaten */
  static double tdt[4];
  static double *velovec, vg, v[2], vt[2];
  static double area, porosity;
  static double heat_capacity_porous_medium;
  static double *heat_dispersion_tensor;
  static int nn = 4;
  static int nn2 =16;
  /* Speicher-Modelle fuer Element-Matrizen */
  if (memory_opt == 0){ // keine Speicheroptimierung
    mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
    if (!mass)
      mass = (double *) Malloc(nn2 * sizeof(double));
    disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
    if (!disp)
      disp = (double *) Malloc(nn2 * sizeof(double));
    adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
    if (!adv)
      adv = (double *) Malloc(nn2 * sizeof(double));
  }
  MNulleMat(mass,nn,nn);
  MNulleMat(disp,nn,nn);
  MNulleMat(adv,nn,nn);
  /* --- Element-Daten ------------------------------------------------ */
  /* Material */
  static int dim = 2;
  //----------------------------------------------------------------------
  // NUM
  double gp[3]={0.,0.,0.};
  anzgp = m_pcs->m_num->ele_gauss_points;
  double theta = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  area = m_mmp->geo_area;
  porosity = m_mmp->Porosity(index,gp,theta);
  heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
//  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(index,gp,theta,0);//CMCD4213
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);
  //----------------------------------------------------------------------
  // MFP fluid properties
  double heat_capacity_fluids = porosity* MFPCalcFluidsHeatCapacity(index,gp,theta);
  //----------------------------------------------------------------------
  // VEL
  velovec = ElGetVelocity(index); // Darcy velocities in GP
  //======================================================================
  // GP loop
  ind = -1;
  for(i=0;i<anzgp;i++){
    for(j=0;j<anzgp;j++){
      r = MXPGaussPkt(anzgp, i);
      s = MXPGaussPkt(anzgp, j);
      //..................................................................
      // GP data
      Calc2DElementJacobiMatrix(index,r,s,invjac,&detjac);        // Jakobi Matrix im Gauss-Punkt
      fkt = MXPGaussFkt(anzgp,i) * MXPGaussFkt(anzgp,j) * detjac; // Gauss-Gewichte * Element-Volumen
      //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0); // in local coordinates
        /* --- SUPG --------------------------------------------------------------- */
//ToDo encapsulate
        MNulleVec(alpha, 2);
        if (vg > MKleinsteZahl && htm_upwind_parameter > 0.0)
          {
            /* Berechnung von zeta */
            for (k = 0; k < 2; k++)
              zeta[k] = -v[k] / vg;    /* reine Advektion */
            if (heat_dispersion_tensor[0] > MKleinsteZahl && heat_dispersion_tensor[3] > MKleinsteZahl)
              {
                /* halbe Peclet-Zahl mit ds=2.0 */
                /* pec = vt * tdt^-1 */
                for (k = 0; k < 4; k++)
                  zwa[k] = tdt[k];
                M2InvertiereUndTransponiere(zwa);  /* MK_bitte_pruefen!!! */
                MMultVecMat(vt, 2, zwa, 2, 2, pec, 2);
                MNulleVec(zeta, 2);    /* fast reine Diffusion */
                for (k = 0; k < 2; k++)
                  if (fabs(pec[k]) > 1.0)
                    zeta[k] = (fabs(pec[k]) - 1.0) / pec[k];
              }
            /* Upwind-Faktoren */
            for (k = 0; k < 2; k++)
              alpha[k] = htm_upwind_parameter * zeta[k];

#ifdef TESTSUPG
            DisplayDoubleVector(alpha, 2, "alpha");
#endif
          }
        /* --- Interpolationsfunktionen ------------------------------------------- */
        /* Wichtungsfunktion: phi */
        /* Fallunterscheidung fuer Standard-Galerkin oder SUPG-Verfahren */
        if (supg)
          {
            MPhi2D_SUPG(phi, r, s, alpha);
          }
        else
          {
            MPhi2D(phi, r, s);
          }
        /* Ansatzfunktion: omega */
        MOmega2D(omega, r, s);

      //..................................................................
      /* --- Massenmatrix - Heat capacitance matrix ------------------------------ */
      /* Zwischensumme, erspart viele Multiplikationen */
      sum = heat_capacity_porous_medium * fkt;
      if(htm_mass_lumping_method==0){ // Standard FEM
        MMultVecVec(phi,nn,omega,nn,zwi,nn,nn); // phi * omega^T
        for(k=0;k<nn2;k++)
          mass[k] += zwi[k] * sum;
      }
      else if(htm_mass_lumping_method==1){ // ML-FEM
        // Wenn die Speicherung per ML-FEM behandelt wird, wird nur die Diagonale gesetzt
        for(k=nn;k<nn2;k++)
          mass[k*5] += omega[k] * sum;
      }
      //..................................................................
      /* --- Dispersionsmatrix - Heat diffusion-dispersion matrix ---------------- */
      /* Dispersionstensor - tdt = (J^-1)^T * D * (J^-1) */
      MMultMatMat(invjac,dim,dim,heat_dispersion_tensor,dim,dim,zwa,dim,dim);
      MTranspoMat(invjac,dim,dim,zwi);
      MMultMatMat(zwa,dim,dim,zwi,dim,dim,tdt,dim,dim);
      /* grad(phi)^T */
      MGradPhi2D(grad_phi_T,r,s);  /* 2 Zeilen 4 Spalten */
      MTranspoMat(grad_phi_T,dim,nn,grad_phi);        /* 4 Zeilen 2 Spalten */
      /* grad(phi) * tdt */
      MMultMatMat(grad_phi,nn,dim,tdt,dim,dim,zwa,nn,dim);
      /* grad(omega)^T */
      MGradOmega2D(grad_ome,r,s);  /* 2 Zeilen 4 Spalten */
      /* grad(phi) * tdt * grad(omega)^T */
      MMultMatMat(zwa,nn,dim,grad_ome,dim,nn,zwi,nn,nn);
      for(k=0;k<nn2;k++)
        disp[k] += (zwi[k] * fkt);
      //..................................................................
      /* --- Advektionsmatrix ---------------------------------------------------- */
      // Geschwindigkeit in lokalen physikalischen (Element) Koordinaten
      for(k=0;k<2;k++)
        v[k] = velovec[++ind] / porosity;
      // vt = T * v = v * T^T 
      MTranspoMat(invjac,dim,dim,invjac_t);
      MMultVecMat(v,dim,invjac_t,dim,dim,vt,dim);
      /* phi * vt */
      MMultVecVec(phi,nn,vt,dim,zwa,nn,dim);
      /* phi * vt * grad(omega)^T */
      MMultMatMat(zwa,nn,dim,grad_ome,dim,nn,zwi,nn,nn);
      for(k=0;k<nn2;k++)
        adv[k] += heat_capacity_fluids * zwi[k] * fkt;
    }
  }
  //======================================================================
  for(i=0;i<nn2;i++){ // Elementdicke
    mass[i] *= (area);
    disp[i] *= (area);
    adv[i] *= (area);
  }
  /* --------------------------------------------------- */
  /* Element-Matrizen speichern oder uebergeben */
  if(memory_opt==0){
    HTMSetElementHeatCapacitanceMatrix(index, mass,m_pcs->pcs_number);
    HTMSetElementHeatDiffusionMatrix(index, disp,m_pcs->pcs_number);
    HTMSetElementHeatAdvectionMatrix(index, adv,m_pcs->pcs_number);
  }
#ifdef TESTCEL_HTM
  MZeigMat(mass,nn,nn,"HTMCalcElementMatrices2D: Heat capacitance matrix");
  MZeigMat(disp,nn,nn,"HTMCalcElementMatrices2D: Heat diffusion-dispersion matrix");
  MZeigMat(adv,nn,nn, "HTMCalcElementMatrices2D: Heat advection matrix");
#endif
}



/**************************************************************************
 ROCKFLOW - Funktion: HTMCalcElementMatrices3D

 Aufgabe:
   3-D Elementmatrizen fuer Waermetransport

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   04/1999   OK   Implementierung
   11/2000   OK   Speichermodelle (memory_opt)
   01/2001   CT   Patch-Integriertes Masslumping, Zwischensumme sum => schneller!
   01/2001   MK
   04/2002   OK   neue MAT Funktionen eingebaut (mehrphasenfaehig)
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation                
11/2004 OK NUM Implementation
**************************************************************************/
void HTMCalcElementMatrices3D(long index, double *mass, double *disp, double *adv,\
                              CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i, j, k, l;
  //  static long ind;
  int inde; //MB
  static double r, s, t;        /* Gausspunkte */
  /* Koordinatentransformationen */
  static double invjac[9], t_invjac[9], jacobi[9], detjac;
  /* Wichtungs- und Interpolationsfunktionen */
  static double phi[8], ome[24];
  /* Numerik */
  static int anzgp;
  static double alpha[3], zeta[3], pec[3];      /* SUPG */
  static double fkt, sum;
  /* Materialdaten */
  static double *velovec, v[3], vt[3], v_rst[3], vg;
  static double tdt[9];
  static double porosity;
  //  static double heat_dispersion_longitudinal, heat_dispersion_transverse;
  static double heat_capacity_fluids;
  static double heat_capacity_porous_medium;
  //  static double *heat_conductivity_porous_medium;
  /* Hilfsfelder */
  static double zwi[64];
  static double zwa[24], zwo[9];
  static int nn = 8;
  static int nn2 =64;
  double gp[3]={0.,0.,0.};
  //----------------------------------------------------------------------
  // Speicher-Modelle fuer Element-Matrizen
  if (memory_opt == 0)
    {                                  /* keine Speicheroptimierung */
      mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
      if (!mass)
        mass = (double *) Malloc(nn2 * sizeof(double));
      disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
      if (!disp)
        disp = (double *) Malloc(nn2 * sizeof(double));
      adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
      if (!adv)
        adv =  (double *) Malloc(nn2 * sizeof(double));
    }
  MNulleMat(mass,nn,nn);
  MNulleMat(disp,nn,nn);
  MNulleMat(adv, nn,nn);
  //----------------------------------------------------------------------
  static int dim = 3;
  //----------------------------------------------------------------------
  // NUM
  anzgp = m_pcs->m_num->ele_gauss_points;
  double theta = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  porosity = m_mmp->Porosity(index,gp,theta);
  heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
  double* heat_dispersion_tensor;
  //----------------------------------------------------------------------
  // MFP fluid properties
  heat_capacity_fluids = porosity*MFPCalcFluidsHeatCapacity(index,gp,theta);
  // Dispersion tensor in global coordinates
  //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0);
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);//CMCD 4213
  //----------------------------------------------------------------------
  // VEL - Filtergeschwindigkeiten in den Gauss-Punkten: 2*anzgp*anzgp - Werte
  velovec = ElGetVelocity(index); // Darcy velocities in GP
  int no_phases = (int)mfp_vector.size();
  double* velo_phase[2];
  for(i=0;i<no_phases;i++){
    velo_phase[i] = ElGetVelocityNew(index,i);
  }
  //======================================================================
  // GP loop
  inde = -1;
  for(i=0;i<anzgp;i++)
    for(j=0;j<anzgp;j++)
      for(k=0;k<anzgp;k++){
        r = MXPGaussPkt(anzgp, i);
        s = MXPGaussPkt(anzgp, j);
        t = MXPGaussPkt(anzgp, k);
        Calc3DElementJacobiMatrix(index, r, s, t, t_invjac, &detjac);
        fkt = MXPGaussFkt(anzgp,i)*MXPGaussFkt(anzgp,j)*MXPGaussFkt(anzgp,k)*fabs(detjac);
        MTranspoMat(t_invjac, 3, 3, invjac);  
        // Transformation into local element coordinates
        // tdt = J^-1 * D * (J^-1)T 
        MMultMatMat(invjac,3,3,heat_dispersion_tensor,3,3,zwa,3,3);
        MMultMatMat(zwa, 3, 3, t_invjac, 3, 3, tdt, 3, 3);
        // Pore velocity v_xyz in GP
        for (l = 0; l < 3; l++)
          //v[l] = velovec[++ind] / porosity; //Achtung mit dem long ind stimmt was nicht!!! MB
          v[l] = velovec[++inde] / porosity;
        vg = MBtrgVec(v, 3);
        /* MK_Calc3DElementJacobiMatrix_error_detected_14.01.2002                                           
           Die Funktion Calc3DElementJacobiMatrix berechnet leider nicht die 
           inverse Jakobimatrix, sondern die transponierte inverse J. (J^-T). 
           Das ist aber auch gut so, da im allgemeinen die transponierte inverse
           der Jakobimatrix J^-T benötigt wird!!!
           MK_Calc3DElementJacobiMatrix_error_detected_14.01.2002 */                                                                                   
//ToDo encapsulate
        /* --- SUPG --------------------------------------------------------------- */
        /* Geschwindigkeitstransformation: x,y,z -> r,s,t */
        Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
        MKopierVec(invjac, jacobi, 9);
        M3Invertiere(jacobi);        /* Jacobi-Matrix */
        MMultMatVec(jacobi, 3, 3, v, 3, v_rst, 3);
        MNulleVec(alpha, 3);
        if(MBtrgVec(v_rst,3)>MKleinsteZahl && htm_upwind_parameter>0.0){
          // Berechnung von zeta
          for(l=0;l<3;l++)
            zeta[l] = v_rst[l] / MBtrgVec(v_rst, 3); // reine Advektion
            if (heat_dispersion_tensor[0]>MKleinsteZahl&&heat_dispersion_tensor[4]>MKleinsteZahl&&heat_dispersion_tensor[8]>MKleinsteZahl)
            {
              /* halbe Peclet-Zahl mit ds=2.0 */
              /* pec = v_rst * (tdt)^-1 */
              for(l=0;l<9;l++)
                zwa[l] = tdt[l];
              M3Invertiere(zwa);
              MMultVecMat(v_rst,3,zwa,3,3,pec,3);
              MNulleVec(zeta, 3);  /* fast reine Diffusion */
              for(l=0;l<3;l++)
                if (fabs(pec[l]) > 1.0)
                  zeta[l] = ((fabs(pec[l]) - 1.0) / fabs(pec[l])) * (v_rst[l] / MBtrgVec(v_rst, 3));
            }
            // Upwind-Faktoren
            for(l=0;l<3;l++)
              alpha[l] = htm_upwind_parameter * zeta[l];
#ifdef TESTSUPG
              DisplayDoubleVector(alpha, 3, "alpha");
#endif
        }
        /* M3Invertiere(invjac);  Inverse Jacobi-Matrix */
        //..................................................................
        // Massenmatrix - Heat capacitance matrix
         // phi * omega
        MPhi3D_SUPG(phi,r,s,t,alpha);
        MOmega3D(zwo,r,s,t);
        // Zwischensumme, erspart viele Multiplikationen 
        sum = heat_capacity_porous_medium * fkt;
        if(htm_mass_lumping_method==0){ // Standard FEM
          // phi * omega^T
          MMultVecVec(phi,nn,zwo,nn,zwi,nn,nn);
          for(l=0;l<nn2;l++)
            mass[l] += sum * zwi[l];
        }
        else if (htm_mass_lumping_method == 1){ // ML-FEM
          for(l=0;l<nn;l++)
            mass[l*9] += zwo[l] * sum;
        }
        //..................................................................
        // Dispersionsmatrix - Heat diffusion-dispersion matrix
         // grad(phi)T
        MGradPhi3D(zwa,r,s,t); // 3 Zeilen 8 Spalten
        MTranspoMat(zwa,dim,nn,zwi); // jetzt: 8 Zeilen 3 Spalten 
         // grad(phi)T * tDt
        MMultMatMat(zwi,nn,dim,tdt,dim,dim,zwa,nn,dim);
         // grad(omega)
        MGradOmega3D(ome,r,s,t); // 3 Zeilen 8 Spalten
         // grad(phi)T * tDt * grad(omega)
        MMultMatMat(zwa,nn,dim,ome,dim,nn,zwi,nn,nn);
        for (l=0;l<nn2;l++)
          disp[l] += (zwi[l] * fkt);
        //..................................................................
        // Advektionsmatrix
        Calc3DElementJacobiMatrix(index,r,s,t,invjac,&detjac);
        MTranspoMat(invjac,dim,dim,t_invjac);    
        //for(l=0;l<dim;l++)
          //v[l] = velovec[++ind] / porosity;
        MMultMatVec(t_invjac,dim,dim,v,dim,vt,dim);
         // phi * vt
        MMultVecVec(phi,nn,vt,dim,zwa,nn,dim);
         // phi * vt * grad(omega)
        MMultMatMat(zwa,nn,dim,ome,dim,nn,zwi,nn,nn);
        for (l=0;l<nn2;l++)
          adv[l] += heat_capacity_fluids * (zwi[l] * fkt);
  }
  //======================================================================
  for(l=0;l<nn2;l++){
    mass[l] *= 1.;
    disp[l] *= 1.;
    adv[l] *= 1.;
  }
  /* --------------------------------------------------- */
  /* Element-Matrizen speichern oder uebergeben */
  if(memory_opt==0){
    HTMSetElementHeatCapacitanceMatrix(index,mass,m_pcs->pcs_number);
    HTMSetElementHeatDiffusionMatrix(index,disp,m_pcs->pcs_number);
    HTMSetElementHeatAdvectionMatrix(index,adv,m_pcs->pcs_number);
  }
#ifdef TESTCEL_HTM
  MZeigMat(mass,nn,nn, "HTMCalcElementMatrices3D: Heat capacitance matrix");
  MZeigMat(disp,nn,nn, "HTMCalcElementMatrices3D: Heat diffusion-dispersion matrix");
  MZeigMat(adv, nn,nn, "HTMCalcElementMatrices3D: Heat advection matrix");
#endif
}


/**************************************************************************
 ROCKFLOW - Funktion: HTMCalcElementMatrices2DTriangle

 Aufgabe:
   2-D Elementmatrizen fuer Waermetransport
   linear triangular elements

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Index des Elements

 Ergebnis:
   - void -

 Programmaenderungen:
   02/2002   OK/JdJ   erste Implementierung
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation                
11/2004 OK NUM Implementation
**************************************************************************/
void HTMCalcElementMatrices2DTriangle(long index, double *mass, double *disp, double *adv,\
                                      CRFProcess*m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */
  static long i;
  /* Geometry */
  static double x[3],y[3];
  static double volume;
  static int nn=3;
  static int nn2=9;
  static double fac_geo;
  long group;
  /* Materials */
  static double *velovec, vg, v[3];
  static double area, porosity;
  static double heat_capacity_fluids;
  static double heat_capacity_porous_medium;
  //  static double *heat_conductivity_porous_medium;
  //  static double heat_dispersion_longitudinal, heat_dispersion_transverse;
  static double fac_mat;
  static double d_x,d_y,q_x,q_y;
  double* heat_dispersion_tensor;
  double dircos[6],gp[3];
  double effective_heat_diffusion_coefficient;
  //----------------------------------------------------------------------
  // NUM
  double theta = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  /* Pointer to the elements CMCD*/ 
  Element *elem = NULL;
  elem = ElGetElement(index);
  /* Speicher-Modelle fuer Element-Matrizen */
  if (memory_opt == 0)
    {                                  /* keine Speicheroptimierung */
      mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
      if (!mass)
        mass = (double *) Malloc(nn2 * sizeof(double));
      disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
      if (!disp)
        disp = (double *) Malloc(nn2 * sizeof(double));
      adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
      if (!adv)
        adv = (double *) Malloc(nn2 * sizeof(double));
    }
  MNulleMat(mass,nn,nn);
  MNulleMat(disp,nn,nn);
  MNulleMat(adv,nn,nn);
  //----------------------------------------------------------------------
  // GEO
   /* Element-Koordinaten im lokalen System (x',y')==(a,b) */
  Calc2DElementCoordinatesTriangle(index,x,y,dircos);

  /* Element-Volumen: Flaeche * Dicke */    
  area = m_mmp->geo_area;
  volume = fabs(ElGetElementVolume(index))/area;

  //----------------------------------------------------------------------
  // MFP fluid properties
  bool FLOW = false; //
  for(i=0;i<(int)pcs_vector.size();i++){
     if(pcs_vector[i]->pcs_type_name.find("FLOW")!=string::npos)
       FLOW = true;
  }

  if(FLOW) //WW
  {
     porosity = m_mmp->Porosity(index,gp,theta);
     heat_capacity_fluids = porosity*MFPCalcFluidsHeatCapacity(index,gp,theta);
  }
  else 
  {
     porosity = 1.0;
     heat_capacity_fluids = 0.0; 
  }
  //----------------------------------------------------------------------
  // MMP medium properties
  heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
  //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0);
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);
  //----------------------------------------------------------------------
  // VEL -pore velocity
  velovec = ElGetVelocity(index); 
  v[0] = velovec[0]/porosity;
  v[1] = velovec[1]/porosity;
  v[2] = velovec[2]/porosity;
  vg = MBtrgVec(v, 3);
 
 /* Place real advective velocities in element data CMCD 05/2004*/
  elem->Advective_Vx = v[0];
  elem->Advective_Vy = v[1];
  elem->Advective_Vz = v[2];

 //Stability Criteria
 effective_heat_diffusion_coefficient = MMax(heat_dispersion_tensor[0],heat_dispersion_tensor[3]);
 CalculateStabilityCriteria(index, effective_heat_diffusion_coefficient, vg, dt);
 

  /* --- SUPG --------------------------------------------------------------- */
  /* --- Massenmatrix - Heat capacitance matrix ------------------------------ */
   /* Materials */
  fac_mat = heat_capacity_porous_medium;
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
      fac_geo = (volume)/(12.);

      for (i=0;i<nn2;i++) {
        mass[i] *= fac_mat * fac_geo;
      }

  /* --- Dispersionsmatrix - Heat diffusion-dispersion matrix ---------------- */
      /* Materials */
      fac_mat = 1.0;
      d_x = heat_dispersion_tensor[0];
      d_y = heat_dispersion_tensor[3];
       /* 3x3 matrix */
      disp[0] = d_x*(y[1]-y[2])*(y[1]-y[2]) + d_y*(x[2]-x[1])*(x[2]-x[1]);
      disp[1] = d_x*(y[1]-y[2])*(y[2]-y[0]) + d_y*(x[2]-x[1])*(x[0]-x[2]);
      disp[2] = d_x*(y[1]-y[2])*(y[0]-y[1]) + d_y*(x[2]-x[1])*(x[1]-x[0]);
      disp[3] = d_x*(y[2]-y[0])*(y[1]-y[2]) + d_y*(x[0]-x[2])*(x[2]-x[1]);
      disp[4] = d_x*(y[2]-y[0])*(y[2]-y[0]) + d_y*(x[0]-x[2])*(x[0]-x[2]);
      disp[5] = d_x*(y[2]-y[0])*(y[0]-y[1]) + d_y*(x[0]-x[2])*(x[1]-x[0]);
      disp[6] = d_x*(y[0]-y[1])*(y[1]-y[2]) + d_y*(x[1]-x[0])*(x[2]-x[1]);
      disp[7] = d_x*(y[0]-y[1])*(y[2]-y[0]) + d_y*(x[1]-x[0])*(x[0]-x[2]);
      disp[8] = d_x*(y[0]-y[1])*(y[0]-y[1]) + d_y*(x[1]-x[0])*(x[1]-x[0]);
       /* Volumetric element */
      fac_geo = 1./(4.*volume);

      for (i=0;i<nn2;i++) {
        disp[i] *= fac_mat * fac_geo;
      }

  /* --- Advektionsmatrix ---------------------------------------------------- */
      /* Materials */
      fac_mat = heat_capacity_fluids;
      q_x = v[0]*dircos[0]+v[1]*dircos[2]+v[2]*dircos[4];
      q_y = v[0]*dircos[1]+v[1]*dircos[3]+v[2]*dircos[5];

       /* 3x3 matrix */
      adv[0] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[1] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[2] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);
      adv[3] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[4] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[5] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);
      adv[6] = q_x*(y[1]-y[2]) + q_y*(x[2]-x[1]);
      adv[7] = q_x*(y[2]-y[0]) + q_y*(x[0]-x[2]);
      adv[8] = q_x*(y[0]-y[1]) + q_y*(x[1]-x[0]);

       /* Volumetric element */
      fac_geo = 1./6.;

      for (i=0;i<nn2;i++) {
        adv[i] *= fac_mat * fac_geo;
      }

  /* ------------------------------------------------------------------------ */
  for (i=0;i<nn2;i++)
  {                                  /* Elementdicke */
     mass[i] *= (area);
     disp[i] *= (area);
     adv[i] *= (area);
  }


  /* --------------------------------------------------- */
  /* Element-Matrizen speichern oder uebergeben */
  if (memory_opt == 0)
  {                                  /* Element-Matrizen in Element-Struktur speichern */
      HTMSetElementHeatCapacitanceMatrix(index,mass,m_pcs->pcs_number);
      HTMSetElementHeatDiffusionMatrix(index,disp,m_pcs->pcs_number);
      HTMSetElementHeatAdvectionMatrix(index,adv,m_pcs->pcs_number);
  }


#ifdef TESTCEL_HTM
  MZeigMat(mass,nn,nn,"HTMCalcElementMatrices2DTriangle: Heat capacitance matrix");
  MZeigMat(disp,nn,nn,"HTMCalcElementMatrices2DTriangle: Heat diffusion-dispersion matrix");
  MZeigMat(adv,nn,nn, "HTMCalcElementMatrices2DTriangle: Heat advection matrix");
#endif
}

/**************************************************************************/
/* ROCKFLOW - Funktion: HTMCalcElementMatrices3DTet
                                                                          */
/* Aufgabe:
   Vorbereitung, notwendig wegen:
   CalcXDElementMatrices_HTM[ElGetElementType(number) - 1]
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/2004   CMCD     Erste Version
11/2004 OK NUM Implementation
                                                                          */
/**************************************************************************/
void HTMCalcElementMatrices3DTet(long index, double *mass, double *disp, double *adv,\
                              CRFProcess*m_pcs)
{                            
  /* Datentypen der verwendeten Variablen */
  
    /* Geometry */
    double x[4],y[4],z[4]; //OK
    double volume;
    int nn=4;
    int nn2=16;
    double fac_geo, fac_mat;
    
    /* Numerik */
    int i;
    double gp[3];
	double a1,a2,a3,a4;
    double b1,b2,b3,b4;
    double c1,c2,c3,c4;
    double d1,d2,d3,d4;
    double dN1dx,dN2dx,dN3dx,dN4dx;
    double dN1dy,dN2dy,dN3dy,dN4dy;
    double dN1dz,dN2dz,dN3dz,dN4dz;
    double mat3x3[9];
//	double N1_1,N1_2,N1_3,N1_4;
//	double N2_1,N2_2,N2_3,N2_4;
//	double N3_1,N3_2,N3_3,N3_4;
//	double N4_1,N4_2,N4_3,N4_4;
  //----------------------------------------------------------------------
  // NUM
  double theta = m_pcs->m_num->ls_theta;
    /* Materials */
    long *element_nodes;
    double d[3];
    double *velovec, vg, v[3];
    double porosity;
    double heat_capacity_fluids;
    double heat_capacity_porous_medium;
    double q_x,q_y,q_z;
    double fluid_density;
    double rock_density;
	double effective_heat_diffusion_coefficient;
    long group = ElGetElementGroupNumber(index);
	int Phase = 0;
	double* heat_dispersion_tensor;

	/* Elements */ 
    Element *elem = NULL;
    elem = ElGetElement(index);

    //Set the required memory if no memory has already been allocated
    if (memory_opt == 0)
        {                                  /* keine Speicheroptimierung */
        mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
        if (!mass)
            mass = (double *) Malloc(nn2 * sizeof(double));
        disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
        if (!disp)
            disp = (double *) Malloc(nn2 * sizeof(double));
        adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
        if (!adv)
            adv =  (double *) Malloc(nn2 * sizeof(double));
        }
    //Assign zeros to the transport matricies 
    MNulleMat(mass,nn,nn);
    MNulleMat(disp,nn,nn);
    MNulleMat(adv, nn,nn);

    //Geometry nodes
    //  The tetrahedra do not need to be transformed into local co-ordintes. Therefore
    //  the value of the gp for the density funtion is given as the center point of the tetrahedra
	CalculateSimpleMiddelPointElement(index,gp);
    element_nodes = ElGetElementNodes(index);
    for(i=0;i<nn;i++) {
        x[i] = GetNodeX(element_nodes[i]);
        y[i] = GetNodeY(element_nodes[i]);
        z[i] = GetNodeZ(element_nodes[i]);
    }

    // Solid Properties 
	CSolidProperties *m_msp = NULL;
	m_msp = msp_vector[group];
	rock_density  = m_msp->Density(1);
    // MMP medium properties
    CMediumProperties *m_mmp = NULL;
    m_mmp = mmp_vector[group];
	porosity = m_mmp->Porosity(index,gp,theta);
    heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
	// Dispersion tensor in global coordinates
  //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0);//CMCD 4123
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);
	d[0] = heat_dispersion_tensor[0];
	d[1] = heat_dispersion_tensor[4];
	d[2] = heat_dispersion_tensor[8];

	// MFP fluid properties
	CFluidProperties *m_mfp = NULL;
    m_mfp = mfp_vector[Phase];
    heat_capacity_fluids =porosity*MFPCalcFluidsHeatCapacity(index,gp,theta);
	fluid_density = m_mfp->Density();

    //Element Data
    volume = (ElGetElementVolume(index));
 
	//Velocity Vector
    velovec = ElGetVelocity(index);
    v[0] = velovec[0]/porosity;  //Vx
    v[1] = velovec[1]/porosity;  //Vy
    v[2] = velovec[2]/porosity;  //Vz
    vg = MBtrgVec(v, 3);//Velocity from Vx,Vy,Vz vectors.

	//Stability Criteria
	effective_heat_diffusion_coefficient = MMax(d[0],d[1]);
	CalculateStabilityCriteria(index, effective_heat_diffusion_coefficient, vg, dt);

//	/* special stop CMCD*/		
//		if (index == 14306){
//			index = index;
//			/* Stop here*/
//			}		
    
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
    
/*	//Element Shape Functions
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
*/

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

    
    /*Heat capacitance matrix ------------------------------ */
    /* Materials */
    /* Consistent Formulation Approach */
    fac_mat = heat_capacity_porous_medium;
    
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
		disp[i] *= fac_geo;//CMCD fac_mat already included in dispersion tensor
        }

/*- Advektionsmatrix ---------------------------------------------------- */
    /* Materials */
    fac_mat = heat_capacity_fluids;
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
      adv[i] *= fac_mat * fac_geo;
	  //adv[i] *= fac_geo;
    }
 
   /* --------------------------------------------------- */
 
   /* Store Element-Matrixes */
  if (memory_opt == 0)
    {             /* Element-Matrizen in Element-Struktur speichern */
      HTMSetElementHeatCapacitanceMatrix(index, mass,m_pcs->pcs_number);
      HTMSetElementHeatDiffusionMatrix(index, disp,m_pcs->pcs_number);
      HTMSetElementHeatAdvectionMatrix(index, adv,m_pcs->pcs_number);
    }

#ifdef TESTCEL_HTM
  MZeigMat(mass,nn,nn,"HTMCalcElementMatrices3DTetrahedra: Heat capacitance matrix");
  MZeigMat(disp,nn,nn,"HTMCalcElementMatrices3DTetrahedra: Heat diffusion-dispersion matrix");
  MZeigMat(adv,nn,nn, "HTMCalcElementMatrices3DTetrahedra: Heat advection matrix");
#endif
}

/**************************************************************************/
/* ROCKFLOW - Funktion: HTM_numCalcEle3DPrism
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
08/2004   MB     Erste Version aufgebaut auf CalcEle3DPrism_ATM_num
09/2004 OK MFP Implementation                
09/2004 OK MMP Implementation                
09/2004 OK MSP Implementation                
11/2004 OK NUM Implementation
                                                                          */
/**************************************************************************/
void HTMCalcElementMatrices3DPrismNum(long index,\
                   double *mass,\
                   double *disp,\
                   double *adv,\
                   CRFProcess*m_pcs)
{
  /* Numerik */
  static double theta;
  static int anzgptri, anzgplin;
  static double r, s, t, fkt;
  static int nn=6;
  static int nn2=36;
  /* Material */
  static double porosity;
  static double heat_capacity_fluids,heat_capacity_porous_medium;
  //  static double *heat_conductivity_porous_medium;
  //  static double heat_dispersion_longitudinal, heat_dispersion_transverse;
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
  static double vg;
  static double tDt[9];
  static double* velo=NULL;
  static double velot[3];
  static int phase = 0;
  double gp[3]={0.,0.,0.};

  /* Speicherplatzreservierung  */
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    mass = (double *) Malloc(nn2 * sizeof(double));
    disp = (double *) Malloc(nn2 * sizeof(double));
    adv = (double *) Malloc(nn2 * sizeof(double));
  }
  /* Initialisieren */
  MNulleMat(mass, nn, nn);
  MNulleMat(disp, nn, nn); 
  MNulleMat(adv, nn, nn);
  /* Datenbereitstellung */
  //  static int dim = 3;
  //----------------------------------------------------------------------
  // MMP medium properties
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  porosity = m_mmp->Porosity(index,gp,theta);
  heat_capacity_porous_medium = m_mmp->HeatCapacity(index,gp,theta);
  double* heat_dispersion_tensor;
  //----------------------------------------------------------------------
  // MFP fluid properties
  heat_capacity_fluids = porosity*MFPCalcFluidsHeatCapacity(index,gp,theta);
  // Dispersion tensor in global coordinates
  //heat_dispersion_tensor = m_mmp->HeatDispersionTensor(index,gp,theta,0);CMCD 4213
  heat_dispersion_tensor = m_mmp->HeatDispersionTensorNew(0);
  //----------------------------------------------------------------------
  // VEL
  //----------------------------------------------------------------------
  // NUM
  theta = m_pcs->m_num->ls_theta;
  anzgptri = 3;
  anzgplin = 2;
  //======================================================================
  // GP loop
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
      //velo = CalcVelo3DPrismRST(phase, index, theta, r, s, t); //MB
      gp[0] = r;
      gp[1] = s;
      gp[2] = t;
      /* Speicher holen */
      velo = ElGetVelocityNew(index, phase); 
      VELCalcGaussPrisLocal(phase, index, gp, m_pcs, velo);

      velo[0] = velo[0] / porosity;
      velo[1] = velo[1] / porosity;
      velo[2] = velo[2] / porosity;
          
      /* Betrag des Vektors */
      vg = MBtrgVec(velo, 3);

      /* Dispersion im Gauss Punkt */
      /* Drehen des Tensors von stromlinienorietierten Koordinaten in lokale Element Koordinaten */
      heat_dispersion_tensor = TensorDrehDich(heat_dispersion_tensor, velo);

      /* vt = v * J^-1 */
      MMultVecMat(velo, 3, invjac, 3, 3, velot, 3);

      /* tdt = (J^-1)T * D * J^-1 */
      MMultMatMat(heat_dispersion_tensor, 3, 3, invjac, 3, 3, zwa, 3, 3);
      MTranspoMat(invjac, 3, 3, zwi);
      MMultMatMat(zwi, 3, 3, zwa, 3, 3, tDt, 3, 3);


      /*------------------------------------------------------------------------*/
      /*---- Heat Capacitance matrix -------------------------------------------*/
      /*------------------------------------------------------------------------*/
      
      /* Omega T * Omega * fkt */
      MOmegaPrism(OmPrism, r, s, t);
      MMultVecVec(OmPrism, nn, OmPrism, nn, zwi, nn, nn);

      for (l = 0; l < nn2; l++)  {
        mass[l] += heat_capacity_porous_medium * fkt * zwi[l];
      }

      
      /*------------------------------------------------------------------------*/
      /*---- Heat Diffusion-Dispersion Matrix ----------------------------------*/
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
      /*---- Heat Advection Matrix ---------------------------------------------*/
      /*------------------------------------------------------------------------*/

      /* OmPrism * velot */
      MMultVecVec(OmPrism, nn, velot, 3, zwa, nn, 3); 
      /* OmPrism * velot * GradOmPrism */
      MMultMatMat(zwa, nn, 3, GradOmPrism, 3, nn, zwu, nn, nn);
      /* OmPrism * velot * GradOmPrism * fkt */
      for (l = 0; l < nn2; l++) {
        adv[l] += heat_capacity_fluids * zwu[l] * fkt;        
      }
   
    }  /* Ende Schleife über Anzahl der GaussPunkte im Dreieck (xy-Richtung) */
  }    /* Ende Schleife über Anzahl der GaussPunkte in z Richtung */


  /*------------------------------------------------------------------------*/
  /* --- Einspeichern der ermittelten Matrizen ---------------------------- */
  /*------------------------------------------------------------------------*/
  if (memory_opt == 0)  {
    HTMSetElementHeatCapacitanceMatrix(index, mass,m_pcs->pcs_number);
    HTMSetElementHeatDiffusionMatrix(index, disp,m_pcs->pcs_number);
    HTMSetElementHeatAdvectionMatrix(index, adv,m_pcs->pcs_number);
  }
#ifdef TESTCEL_HTM
  MZeigMat(mass, nn, nn, "HTMCalcElementMatrices3DPrismNum: MASS MATRIX");
  MZeigMat(disp, nn, nn, "HTMCalcElementMatrices3DPrismNum: DISPERSION MATRIX");
  MZeigMat(adv, nn, nn, "HTMCalcElementMatrices3DPrismNum: ADVECTION MATRIX");
#endif
}


/*=======================================================================*/
/* Numerische Kennzahlen */
/*=======================================================================*/

/**************************************************************************
   ROCKFLOW - Funktion: CECalcPeclet_HTM

   Aufgabe:
   Berechnet die Element-Peclet-Zahl und den Courant- (Cr=1) bzw.
   Neumann-Zeitschritt (Ne=0.5) des angegebenen 1D-, 2D- oder 3D-Elements
   Pec = v * ds / D
   dt_Courant = ds / v
   dt_Neumann = (ds * ds) / (2 * D)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements

   Ergebnis:
   - void -

   Programmaenderungen:
   01/1996     cb        Erste Version
   09/1996     cb        genauer - stroemungsgerichtet
   09/1996     cb        nur Courant-Kriterim einhalten
   03/2000     RK        HTM-Anpassung

  **************************************************************************/
void CECalcPeclet_HTM(long index)
{
  static int et, anz, j;
  static long k, anzgp;
  static double *velovec, *invjac;
  static double vs[3], dx[3], dr[3], zwi[9];
  static double v, ds, d, el_dt, el_pec, detjac, a, b, c, fkt;
  static double art_diff;
  static double area, porosity;
  static double heat_conductivity_fluid;
  //  static double heat_conductivity_rock_xx, heat_conductivity_rock_yy, heat_conductivity_rock_zz;
  static double heat_conductivity_rock;
  static double heat_capacity_fluid, density_fluid;
  static double heat_dispersion_longitudinal, heat_dispersion_transverse;
  static double heat_dispersion;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  CSolidProperties *m_msp = NULL;
  m_msp = msp_vector[group];

  double theta = 1.; //OK htm_numerics->time_collocation;
  area = m_mmp->geo_area;
  porosity = m_mmp->Porosity(index,NULL,theta);
  heat_conductivity_fluid = mfp_vector[0]->HeatConductivity();
  heat_capacity_fluid = mfp_vector[0]->SpecificHeatCapacity(); //SB:m 
  density_fluid = mfp_vector[0]->Density();
  heat_dispersion_longitudinal = m_mmp->heat_dispersion_longitudinal;
  heat_dispersion_transverse = m_mmp->heat_dispersion_transverse;

  /* kuenstliche Diffusion auf Kinder uebertragen */
  k = ElGetElementPred(index);
  if (k > -1l)
    ElSetArtDiff(index, (art_diff = ElGetArtDiff(k)));
  else
    art_diff = ElGetArtDiff(index);

  /* Betrag der mittleren Geschwindigkeit */
  et = ElGetElementType(index);
  if (et == 2)
    anz = 2;
  else
    anz = 3;
  velovec = ElGetVelocity(index);
  anzgp = (long) pow((double) GetNumericsGaussPoints(ElGetElementType(index)), (double) et);

  for (j = 0; j < anz; j++)
    {
      vs[j] = 0.0;
      for (k = 0l; k < anzgp; k++)
        vs[j] += velovec[anz * k + j] / porosity;
      vs[j] /= (double) anzgp;
    }
  v = MBtrgVec(vs, anz);

  /* durchstroemte Elementlaenge */
  switch (et)
    {
    case 1:
      ds = ElGetElementVolume(index) / area;
      break;
    case 2:
      if (v > MKleinsteZahl)
        {
          a = fabs(vs[0]);
          b = fabs(vs[1]);
          fkt = max(a, b);
          dr[0] = 2.0 * a / fkt;
          dr[1] = 2.0 * b / fkt;
        }
      else
        {                              /* reine Diffusion */
          dr[0] = 2.0;
          dr[1] = 0.0;
        }
      /* dr(lokal) = J * dx(global) */
      invjac = GetElementJacobiMatrix(index, &detjac);
      for (j = 0; j < 4; j++)
        zwi[j] = invjac[j];
      M2InvertiereUndTransponiere(zwi);  /* MK_bitte_pruefen!!!: Transponierte der */  /* Jakobi-Matrix */
      MMultMatVec(zwi, 2, 2, dr, 2, dx, 2);
      ds = MBtrgVec(dx, 2);
      break;
    case 3:
      if (v > MKleinsteZahl)
        {
          a = fabs(vs[0]);
          b = fabs(vs[1]);
          c = fabs(vs[2]);
          fkt = max(max(a, b), c);
          dr[0] = 2.0 * a / fkt;
          dr[1] = 2.0 * b / fkt;
          dr[2] = 2.0 * c / fkt;
        }
      else
        {                              /* reine Diffusion */
          dr[0] = 2.0;
          dr[1] = 0.0;
          dr[2] = 0.0;
        }
      /* dr(lokal) = J * dx(global) */
      invjac = GetElementJacobiMatrix(index, &detjac);
      for (j = 0; j < 9; j++)
        zwi[j] = invjac[j];
      M3Invertiere(zwi);               /* Jakobi-Matrix */
      MMultMatVec(zwi, 3, 3, dr, 3, dx, 3);
      ds = MBtrgVec(dx, 3);
      break;
    }
  /* Diffusion */

//SB:m  heat_conductivity_rock = max(heat_conductivity_rock_xx, max(heat_conductivity_rock_yy, heat_conductivity_rock_zz));
  heat_conductivity_rock = m_msp->Heat_Conductivity(); //SB:m ?
  heat_dispersion = max(heat_dispersion_longitudinal, heat_dispersion_transverse);

  d = (porosity * heat_conductivity_fluid + (1.0 - porosity) * heat_conductivity_rock) / \
      (heat_capacity_fluid * density_fluid) + art_diff + \
      porosity * v * heat_dispersion;

  /* Element-Peclet-Zahl */
  if (d < MKleinsteZahl)
    d = MKleinsteZahl;
  el_pec = v * ds / d;
//OK  HTMSetElementPecletNum(index, el_pec);

  /* zugehoeriger Element-Zeitschritt */

#ifdef TESTCEL_HTM
  DisplayMsg("Dimension=");
  DisplayLong(et);
  DisplayMsg(" Peclet-Zahl=");
  DisplayDouble(el_pec, 0, 0);
#endif

  if (el_pec < 2.0)
    {
      el_dt = ds * ds * 0.5 / d;       /* Neumman */

#ifdef TESTCEL_HTM
      DisplayMsg("dt-Neumann=");
#endif
    }
  else
    {
      el_dt = ds / max(v, MKleinsteZahl);       /* Courant */

#ifdef TESTCEL_HTM
      DisplayMsg("dt-Courant=");
#endif
    }

#ifdef TESTCEL_HTM
  DisplayDouble(el_dt, 0, 0);
  DisplayMsgLn("");
#endif

//OK  HTMSetElementDt(index, el_dt);
}



/**************************************************************************
   ROCKFLOW - Funktion: CECalcPeclet_HTM_New

   Aufgabe:
   Berechnet die Element-Peclet-Zahl und den Courant- (Cr=1) bzw.
   Neumann-Zeitschritt (Ne=0.5) des angegebenen 1D-, 2D- oder 3D-Elements
   Pec = v * ds / D
   dt_Courant = ds / v
   dt_Neumann = (ds * ds) / (2 * D)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements

   Ergebnis:
   - void -

   Programmaenderungen:
   09/2000   CT   Erste Version (abgeleitet aus Einphasenversion und Mehrphasen-ATM)

  **************************************************************************/
void CECalcPeclet_HTM_New(long index)
{
  static int et, anz, i, j, phase;
  static long k;
  static double invjac[9], jacobi[9], detjac;
  static double vs[3], dx[3], dr[3], zwi[9];
  static double v[3], v_rs[3], vb, ds, d, el_dt, el_pec, a, b, c, fkt;
  static double art_diff, weight, weight_sum, sum_heat_capacity_fluid_density_fluid;
  static double area, soil_porosity, *fluid_content;
  static double *heat_conductivity_fluid;
  //  static double heat_conductivity_rock_xx, heat_conductivity_rock_yy, heat_conductivity_rock_zz;
  static double heat_conductivity_rock;
  static double *heat_capacity_fluid, *density_fluid, heat_capacity_rock, density_rock;
  static double heat_dispersion_longitudinal, heat_dispersion_transverse;
  static double heat_dispersion;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  // MSP solid properties 
  CSolidProperties *m_msp = NULL;
  m_msp = msp_vector[group];
  
  double theta = 1.0; //OK htm_numerics->time_collocation;

  area = m_mmp->geo_area;

  /* Materialdaten */
  heat_dispersion_longitudinal = m_mmp->heat_dispersion_longitudinal;
  heat_dispersion_transverse = m_mmp->heat_dispersion_transverse;
  soil_porosity = m_mmp->Porosity(index,NULL,theta);
  fluid_content = (double *) Malloc(GetRFProcessNumPhases() * sizeof(double));
  density_fluid = (double *) Malloc(GetRFProcessNumPhases() * sizeof(double));
  heat_capacity_fluid = (double *) Malloc(GetRFProcessNumPhases() * sizeof(double));
  heat_conductivity_fluid = (double *) Malloc(GetRFProcessNumPhases() * sizeof(double));
  for (phase = 0; phase < GetRFProcessNumPhases(); phase++)
    {
//OK      fluid_content[phase] = HTMGetFluidContent(phase, index, 0., 0., 0., htm_time_collocation_global);
      density_fluid[phase] = mfp_vector[phase]->Density();
	  heat_capacity_fluid[phase] = mfp_vector[phase]->SpecificHeatCapacity();
      heat_conductivity_fluid[phase] = mfp_vector[phase]->HeatConductivity();
    }
  density_rock  = m_msp->Density(1);
  heat_capacity_rock = m_msp->Heat_Capacity();
  heat_dispersion_longitudinal = m_mmp->heat_dispersion_longitudinal;
  heat_dispersion_transverse = m_mmp->heat_dispersion_transverse;
  /* kuenstliche Diffusion auf Kinder uebertragen */
  k = ElGetElementPred(index);
  if (k > -1l)
    ElSetArtDiff(index, (art_diff = ElGetArtDiff(k)));
  else
    art_diff = ElGetArtDiff(index);

  for (phase = 0; phase < GetRFProcessNumPhases(); phase++)
    {
      /* Mittlere Geschwindigkeit */
      CalcVeloXDrst(phase, index, htm_time_collocation_upwinding, 0., 0., 0., v);
      if (ElGetElementType(index) == 2)
        {
          /* Geschwindigkeitstransformation: a,b -> r,s */
          Calc2DElementJacobiMatrix(index, 0., 0., invjac, &detjac);
          MKopierVec(invjac, jacobi, 4);
          M2InvertiereUndTransponiere(jacobi);  /* MK_bitte_pruefen!!!: Transponierte der  */ /* Jacobi-Matrix */
          MMultMatVec(jacobi, 2, 2, v, 2, v_rs, 2);
        }
      else
        {
          MKopierVec(v, v_rs, 3);
        }

      weight = fluid_content[phase] * density_fluid[phase] * heat_capacity_fluid[phase];
      weight_sum += weight;
      /* Geschwindigkeiten wichten mit Hilfe des advektiven Waermeflusses der Phase */
      for (i = 0l; i < 3; i++)
        vs[i] += v_rs[i] / fluid_content[phase] * weight;
    }                                  /* endfor */

  /* Geschwindigkeiten wichten */
  for (i = 0l; i < 3; i++)
    vs[i] /= weight_sum;

  vb = MBtrgVec(vs, anz);

  /* durchstroemte Elementlaenge */
  switch (et)
    {
    case 1:
      ds = ElGetElementVolume(index) / area;
      break;
    case 2:
      if (vb > MKleinsteZahl)
        {
          a = fabs(vs[0]);
          b = fabs(vs[1]);
          fkt = max(a, b);
          dr[0] = 2.0 * a / fkt;
          dr[1] = 2.0 * b / fkt;
        }
      else
        {                              /* reine Diffusion */
          dr[0] = 2.0;
          dr[1] = 0.0;
        }
      /* dr(lokal) = J * dx(global) */
      CalcXDElementJacobiMatrix(index, 0., 0., 0., invjac, &detjac);
      for (j = 0; j < 4; j++)
        zwi[j] = invjac[j];
      M2InvertiereUndTransponiere(zwi);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jakobi-Matrix */
      MMultMatVec(zwi, 2, 2, dr, 2, dx, 2);
      ds = MBtrgVec(dx, 2);
      break;
    case 3:
      if (vb > MKleinsteZahl)
        {
          a = fabs(vs[0]);
          b = fabs(vs[1]);
          c = fabs(vs[2]);
          fkt = max(max(a, b), c);
          dr[0] = 2.0 * a / fkt;
          dr[1] = 2.0 * b / fkt;
          dr[2] = 2.0 * c / fkt;
        }
      else
        {                              /* reine Diffusion */
          dr[0] = 2.0;
          dr[1] = 0.0;
          dr[2] = 0.0;
        }
      /* dr(lokal) = J * dx(global) */
      CalcXDElementJacobiMatrix(index, 0., 0., 0., invjac, &detjac);
      for (j = 0; j < 9; j++)
        zwi[j] = invjac[j];
      M3Invertiere(zwi);               /* Jakobi-Matrix */
      MMultMatVec(zwi, 3, 3, dr, 3, dx, 3);
      ds = MBtrgVec(dx, 3);
      break;
    }
  /* Diffusion */

//SB:m  heat_conductivity_rock = max(heat_conductivity_rock_xx, max(heat_conductivity_rock_yy, heat_conductivity_rock_zz));
  
  heat_conductivity_rock = m_msp->Heat_Conductivity(); //SB:m ?
  heat_dispersion = max(heat_dispersion_longitudinal, heat_dispersion_transverse);

  sum_heat_capacity_fluid_density_fluid = 0.;
  for (phase = 0l; phase < GetRFProcessNumPhases(); phase++)
    sum_heat_capacity_fluid_density_fluid += heat_capacity_fluid[phase] * density_fluid[phase];


  d = ((1.0 - soil_porosity) * heat_conductivity_rock) / \
      (sum_heat_capacity_fluid_density_fluid) + art_diff;

  for (phase = 0; phase < GetRFProcessNumPhases(); phase++)
    d += (fluid_content[phase] * heat_conductivity_fluid[phase]) / (sum_heat_capacity_fluid_density_fluid) + fluid_content[phase] * vb * heat_dispersion;

  /* Element-Peclet-Zahl */
  if (d < MKleinsteZahl)
    d = MKleinsteZahl;
  el_pec = vb * ds / d;
//OK  HTMSetElementPecletNum(index, el_pec);

  /* zugehoeriger Element-Zeitschritt */

#ifdef TESTCEL_HTM
  DisplayMsg("Dimension=");
  DisplayLong(et);
  DisplayMsg(" Peclet-Zahl=");
  DisplayDouble(el_pec, 0, 0);
#endif

  if (el_pec < 2.0)
    {
      el_dt = ds * ds * 0.5 / d;       /* Neumman */

#ifdef TESTCEL_HTM
      DisplayMsg("dt-Neumann=");
#endif
    }
  else
    {
      el_dt = ds / max(vb, MKleinsteZahl);      /* Courant */

#ifdef TESTCEL_HTM
      DisplayMsg("dt-Courant=");
#endif
    }

#ifdef TESTCEL_HTM
  DisplayDouble(el_dt, 0, 0);
  DisplayMsgLn("");
#endif

//OK  HTMSetElementDt(index, el_dt);
}






/**************************************************************************
   ROCKFLOW - Funktion: HTMDampOscillations

   Aufgabe:
   
   Daempft Oszillationen des Feldes oder begrenzt den Wertebereich.

   Ergebnis:
   - void -

   Programmaenderungen:
   07/2002   CT   Erste Version

  **************************************************************************/
void HTMDampOscillations(int ndx1)
{
   DampOscillations(ndx1, htm_oscil_damp_method, htm_oscil_damp_parameter, HTMNodeCalcLumpedMass);
}


/**************************************************************************
   ROCKFLOW - Funktion:  HTMNodeCalcLumpedMass

   Aufgabe:
   Liefert fuer einen Knoten die "gelumpte" Speichermatrix

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB

   Ergebnis:
     double mass : Rueckgabe, des Lumped-Wertes

   Programmaenderungen:
   7/2002 C.Thorenz Erste Version

 **************************************************************************/
double HTMNodeCalcLumpedMass(long knoten)
{
  double *mass=NULL, lm=0.;
  long *elemente, *nodes;
  int anz_elemente, i, j, k, nn;
int pcs_number = -1; //OK
  elemente = GetNodeElements(knoten, &anz_elemente);

  for (i = 0l; i < anz_elemente; i++) {
    nn = ElNumberOfNodes[ElGetElementType(elemente[i]) - 1];
    
    mass = HTMGetElementHeatCapacitanceMatrix(elemente[i],pcs_number);
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


/*************************************************************************
ROCKFLOW - Function: HTMCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 05/2003 OK Implementation
last modified:
**************************************************************************/
void *HTMCreateELEMatricesPointer(void)
{ 
  HTMElementMatrices *data = NULL;
  data = (HTMElementMatrices *) Malloc(sizeof(HTMElementMatrices));
  data->heatadvectionmatrix = NULL;
  data->heatcapacitancematrix = NULL;
  data->heatdispersionmatrix = NULL;

  return (void *) data;
}

/*************************************************************************
ROCKFLOW - Function: HTMDestroyELEMatricesPointer
Task: Destruct element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *HTMDestroyELEMatricesPointer(void *data)
{
//OKnew
 if(data) {
  if(((HTMElementMatrices *) data)->heatadvectionmatrix)
     ((HTMElementMatrices *) data)->heatadvectionmatrix = \
       (double *) Free(((HTMElementMatrices *) data)->heatadvectionmatrix);
  if(((HTMElementMatrices *) data)->heatcapacitancematrix)
     ((HTMElementMatrices *) data)->heatcapacitancematrix = \
       (double *) Free(((HTMElementMatrices *) data)->heatcapacitancematrix);
  if(((HTMElementMatrices *) data)->heatdispersionmatrix)
     ((HTMElementMatrices *) data)->heatdispersionmatrix = \
       (double *) Free(((HTMElementMatrices *) data)->heatdispersionmatrix);

  data = (void*) Free(data);
 }
  return data;
}

/*------------------------------------------------------------------------*/
/* ElementHeatCapacitanceMatrix */
void HTMSetElementHeatCapacitanceMatrix(long number,double*matrix,int pcs_number)
{
  static HTMElementMatrices *data;
  data = (HTMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  /*
  //WW, will be removed if class is involved
  if(data->heatcapacitancematrix)
	data->heatcapacitancematrix  = (double *)Free(data->heatcapacitancematrix);
  */
  data->heatcapacitancematrix = matrix;
}

double *HTMGetElementHeatCapacitanceMatrix(long number,int pcs_number)
{
  return ((HTMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->heatcapacitancematrix;
}

/*------------------------------------------------------------------------*/
/* ElementHeatDiffusionMatrix */
void HTMSetElementHeatDiffusionMatrix(long number,double *matrix,int pcs_number)
{
  static HTMElementMatrices *data;
  data = (HTMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  /*
  //WW, will be removed if class is involved
  if(data->heatdispersionmatrix)
	  data->heatdispersionmatrix 
	    = (double *)Free(data->heatdispersionmatrix);
  */
  data->heatdispersionmatrix = matrix;
}

double *HTMGetElementHeatDiffusionMatrix(long number,int pcs_number)
{
  return ((HTMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->heatdispersionmatrix;
}

/*------------------------------------------------------------------------*/
/* ElementHeatAdvectionMatrix */
void HTMSetElementHeatAdvectionMatrix(long number,double *matrix,int pcs_number)
{
  static HTMElementMatrices *data;
  data = (HTMElementMatrices *) ELEGetElementMatrices(number,pcs_number);
  /*
    //WW, will be removed if class is involved
  if(data->heatadvectionmatrix) 
	  data->heatadvectionmatrix =(double *)Free(data->heatadvectionmatrix);
  */
  data->heatadvectionmatrix = matrix;
}

double *HTMGetElementHeatAdvectionMatrix(long number,int pcs_number)
{
  return ((HTMElementMatrices *) \
           ELEGetElementMatrices(number,pcs_number))->heatadvectionmatrix;
}


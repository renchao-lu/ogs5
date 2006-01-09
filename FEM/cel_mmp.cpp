/**************************************************************************
   ROCKFLOW - Modul: cel_mmp.c
   Aufgabe:
   Funktionen zur Berechnung der Druck-Elementmatrizen fuer das Modul MMP.

 Programmaenderungen:

 9/97     C.Thorenz      Erste Version
 11/1998  C.Thorenz      Erste rausgegangene Version
 5/1999   C.Thorenz      Umbau auf n-Phasen
 7/1999   C.Thorenz      3D-Elemente
 9/1999   C.Thorenz      2D-Elemente
 7/2000   C.Thorenz      Div. Korrekturen
            ..             ..
 7/2002   C.Thorenz      Div. Korrekturen
 09/2002  OK             MMPCalcCharacteristicNumbers
 03/2003  RK             Quellcode bereinigt, Globalvariablen entfernt
 05/2003 OK Data access functions to element matrices
 05/2003 OK MMPCalcElementMatrices()
 08/2004 OK MFP implementation

**************************************************************************/
#include "stdafx.h"                    /* MFC */

#include <iostream>
#include "rf_pcs.h"
#include "nodes.h"
#include "makros.h"
#include "int_mmp.h"
#include "elements.h"
#include "mathlib.h"
#include "femlib.h"
#include "adaptiv.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
#include "fem_ele.h"
#include "rf_vel_new.h"

extern double gravity_constant;

#define noTESTCEL_MMP
using FiniteElement::CElement;
/* Interne (statische) Deklarationen */
int InitializeMatrixtype_MMP(char *name, int phase, int component);
int GetMatrixtype_MMP(char *name);

/* Daten fuer den Matrixwiederaufbau */
typedef struct
 {
    char *name;
    int method;
    int mode;
    int curve;
    int group;
    int phase;
    int comp;
    double param[16];
    int number_reference_values;
    int *index_for_reference_values;
  }
Rebuild_info;

static int mmp_number_rebuild_info;
static Rebuild_info *mmp_rebuild_info;

/* Datenstruktur fuer das Verwalten der Matrizen-Infos */
typedef struct
  {
    char *name;
    int type;
    int phase;
    int comp;
  }
Matrix_info;

/* Default-Werte fuer den Kernel */
static int mmp_method = 1;
static int mmp_element_integration_method_maximum = 1;
static int mmp_element_integration_method_density = 0;
static int mmp_element_integration_method_viscosity = 0;
static int mmp_element_integration_method_rel_perm = 1;
static Matrix_info *Array_of_Matrixtypes = NULL;
static int Number_of_Matrixtypes = 0;
//static int mmp_rebuild_array_size = 0;
static int mmp_matrixrebuild_read = 0;
//static int mmp_integration_array_size = 0;
//static int mmp_timecollocation_array_size = 0;
static double mmp_time_collocation_global = 1.;
static double mmp_time_collocation_cond_bc = 1.;
static double mmp_time_collocation_source = 0.;
static double mmp_time_collocation_upwinding = 0.;
static double mmp_time_collocation_open_boundary = 0.;
static double mmp_time_collocation_rebuild = 1.;

static int mmp_upwind_method = 1;
static double mmp_upwind_parameter = 1.;

static int mmp_mass_lumping_method = 0;
//static double mmp_mass_lumping_parameter = 0.;

static int mmp_predictor_method = 0;
static double mmp_predictor_parameter = 0.;

static int mmp_relaxation_method = 0;
static double mmp_relaxation_parameter[16];

static int mmp_val_extraction_method = 0;
static int mmp_nonlinear_coupling = 1;

int mmp_reference_pressure_method = 0;

static int mmp_pcs_number;

#ifdef EFFORT
int mmp_effort_index = -1;
int mmp_effort_total_index = -1;
#endif

void DeleteElementMatrix_MMP(long index, int phase, int matrixtyp);

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_MMP

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/

/**************************************************************************
 ROCKFLOW - Funktion: DestroyKernel_MMP

 Aufgabe:
   Zerstoert die Kerneldaten

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   9/2000   C.Thorenz    Erste Version

**************************************************************************/
int DestroyKernel_MMP(void)
{
  int i;

  for (i = 0; i < Number_of_Matrixtypes; i++)
    Array_of_Matrixtypes[i].name = (char *) Free(Array_of_Matrixtypes[i].name);

  Array_of_Matrixtypes = (Matrix_info *) Free(Array_of_Matrixtypes);


  return 0;
}

/**************************************************************************
 ROCKFLOW - Funktion: GetXXX_MMP

 Aufgabe:
   Gibt diverse Kernelkonfigurationswerte zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   22.04.99   C.Thorenz    Erste Version

**************************************************************************/
double GetTimeCollocationGlobal_MMP(void)
{
  return mmp_time_collocation_global;
}
double GetTimeCollocationCondBC_MMP(void)
{
  return mmp_time_collocation_cond_bc;
}
double GetTimeCollocationSource_MMP(void)
{
  return mmp_time_collocation_source;
}
double GetTimeCollocationupwind_MMP(void)
{
  return mmp_time_collocation_upwinding;
}
double GetTimeCollocationOpenBoundary_MMP(void)
{
  return mmp_time_collocation_open_boundary;
}
int GetValuesExtractMethod_MMP(void)
{
  return mmp_val_extraction_method;
}
int GetMethod_MMP(void)
{
  return mmp_method;
}
int GetNonLinearCoupling_MMP(void)
{
  return mmp_nonlinear_coupling;
}
int GetPredictorMethod_MMP(void)
{
  return mmp_predictor_method;
}
double GetPredictorParam_MMP(void)
{
  return mmp_predictor_parameter;
}
int GetRelaxationMethod_MMP(void)
{
  return mmp_relaxation_method;
}
double *GetRelaxationParam_MMP(void)
{
  return mmp_relaxation_parameter;
}
int GetReferencePhase_MMP(void)
{
  return mmp_reference_pressure_method;
}




/**************************************************************************
 ROCKFLOW - Funktion: InitializeMatrixtype_MMP

 Aufgabe:
   Initialisiert die im Kern behandelten Matrixtypen.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char name: Name der zu bearbeitenden Matrix
   E: int phase: Phase der zu bearbeitenden Matrix
   E: int component: Nicht benutzt

 Ergebnis:
  0

 Programmaenderungen:
   7/2000   C.Thorenz    Erste Version

**************************************************************************/
int InitializeMatrixtype_MMP(char *name, int phase, int component)
{

  /* Speicher holen */
  Array_of_Matrixtypes = (Matrix_info *) Realloc(Array_of_Matrixtypes, (Number_of_Matrixtypes + 1) * sizeof(Matrix_info));

  /* Name eintragen */
  Array_of_Matrixtypes[Number_of_Matrixtypes].name = (char *) Malloc(((int)strlen(name) + 2) * sizeof(char));
  strcpy(Array_of_Matrixtypes[Number_of_Matrixtypes].name, name);

  /* Index des Typs eintragen */
  if (GetMatrixtype_MMP(name) < 0)
    {
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Panic in Kernel MMP: Unknown matrix-type initialized");
      abort();
    }                                  /* endif */
  Array_of_Matrixtypes[Number_of_Matrixtypes].type = GetMatrixtype_MMP(name);

  /* Phase eintragen */
  Array_of_Matrixtypes[Number_of_Matrixtypes].phase = phase;

  /* Komponente eintragen (fuer Mehrphasenmodell nicht noetig ) */
  Array_of_Matrixtypes[Number_of_Matrixtypes].comp = component;


  Number_of_Matrixtypes++;

  return 0;

}

/**************************************************************************
 ROCKFLOW - Funktion: GetMatrixIndex_MMP

 Aufgabe:

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char name: Name der zu bearbeitenden Matrix
   E: int phase: Phase der zu bearbeitenden Matrix

 Ergebnis:
  Index der Matrix

 Programmaenderungen:
   10/2000   C.Thorenz    Erste Version

**************************************************************************/
int GetMatrixIndex_MMP(char *name, int phase)
{
  int i=-1;
  int type = GetMatrixtype_MMP(name);

  /* Index des Typs holen */
  if (type < 0)
    {
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Panic in Kernel MMP: Unknown matrix-type");
      abort();
    }                                  /* endif */
  for (i = 0; i < Number_of_Matrixtypes; i++)
    {
      if ((Array_of_Matrixtypes[i].phase == phase) && (Array_of_Matrixtypes[i].type == type))
        {
          return i;
        }                              /* endif */
    }                                  /* endfor */

  return -1;

}



/**************************************************************************
 ROCKFLOW - Funktion: GetNumberOfMatrixtypes_MMP

 Aufgabe:
   Gibt die Anzahl der im Kern registrierten Matrizen zurueck.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   Anzhal der Matrixtypen

 Programmaenderungen:
   22.04.99   C.Thorenz    Erste Version

**************************************************************************/
int GetNumberOfRegisteredMatrixtypes_MMP(void)
{
  return Number_of_Matrixtypes;
}

/**************************************************************************
 ROCKFLOW - Funktion: ConditionalRebuildMatrices_MMP

 Aufgabe:
   Matrizen zur Berechnung kennzeichnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase: Nummer der Phase

 Ergebnis:
   - void -

 Programmaenderungen:
    9/2000    C.Thorenz     Erste Version

**************************************************************************/
void ConditionalRebuildMatrices_MMP(int phase)
{
  long i, element;
  int *element_rebuild, e_index, typ;
  double error, error1, error2;
  Rebuild_info *rebuild_info;
  int num_rebuild_info = 0;
  int matrix_condu, matrix_capac, matrix_gravi, matrix_capil, do_rebuild, do_delete;
  int found_density = 0, found_mobility = 0, found_saturation = 0;
  int matrixtyp;
  double val, ref_val;
  string nod_val_name;
  char char_phase[1];
  double gp[3] = {0.,0.,0.};

  #ifdef EFFORT
  long rebuild[16], notrebuild[16];
  #endif


  /* Zweidimensionales Feld: Element, Rebuild-Eintrag  */
  element_rebuild = (int *) Malloc(ElListSize() * GetNumberOfRegisteredMatrixtypes_MMP() * sizeof(int));

  /* Eintraege  loeschen */
  for (i = 0; i < ElListSize() * GetNumberOfRegisteredMatrixtypes_MMP(); i++)
    element_rebuild[i] = 0;

#ifdef EFFORT
  for (i = 0; i < GetNumberOfRegisteredMatrixtypes_MMP(); i++)
    {
      rebuild[i] = 0;
      notrebuild[i] = 0;
    }
#endif


  /* Index der Matrixtypen holen */
  matrix_condu = GetMatrixIndex_MMP("MMPCONDUCTIVITY0", phase);
  matrix_capac = GetMatrixIndex_MMP("MMPCAPACITANCE0", phase);
  matrix_gravi = GetMatrixIndex_MMP("MMPGRAVITYVECTOR0", phase);
  matrix_capil = GetMatrixIndex_MMP("MMPCAPILLARPRESSURE0", phase);

  /* Die aktuellen Werte bestimmen und abspeichern */

  /* Ueber alle Eintraege gehen */
  for (num_rebuild_info = 0; num_rebuild_info < mmp_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &mmp_rebuild_info[num_rebuild_info];

      /* Nicht NUll-Methode ? */
      if (rebuild_info -> method)
        {

          if (!strcmp("ELEMENT_MOBILITY_CHANGE", rebuild_info -> name))
            {
              if ((rebuild_info -> phase == -1) || (rebuild_info -> phase == phase))
                {
                  e_index = rebuild_info -> index_for_reference_values[phase] + GetRFProcessNumPhases();
                  for (i = 0; i < anz_active_elements; i++)
                    {
						val = MMPGetRelativePermeability(phase, ActiveElements[i], 0., 0., 0., mmp_time_collocation_rebuild) / mfp_vector[phase]->Viscosity();
                      ElSetElementVal(ActiveElements[i], e_index, val);
                    }
                }                      /* endif */
            }
          if (!strcmp("ELEMENT_DENSITY_CHANGE", rebuild_info -> name))
            {
              if ((rebuild_info -> phase == -1) || (rebuild_info -> phase == phase))
                {
                  e_index = rebuild_info -> index_for_reference_values[phase] + GetRFProcessNumPhases();
                  for (i = 0; i < anz_active_elements; i++)
                    {
                      val = mfp_vector[phase]->Density();
                      ElSetElementVal(ActiveElements[i], e_index, val);
                    }
                }
            }
          if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
            {
              if ((rebuild_info -> phase == -1) || (rebuild_info -> phase == phase))
                {
                  e_index = rebuild_info -> index_for_reference_values[phase] + GetRFProcessNumPhases();
                  for (i = 0; i < anz_active_elements; i++)
                    {
                      //OK val = MMPGetSaturation(phase, ActiveElements[i], 0., 0., 0., mmp_time_collocation_rebuild);
                      sprintf(char_phase,"%i",phase+1);
                      nod_val_name = "SATURATION";
                      nod_val_name += char_phase;
                      val = PCSGetELEValue(ActiveElements[i],gp,mmp_time_collocation_rebuild,nod_val_name);
                      ElSetElementVal(ActiveElements[i], e_index, val);
                    }
                }
            }
        }
    }                                  /* endfor */


  /* Die aktuellen Werte und Referenzwerte mit Sollwerten vergleichen und ggf. Matrizen markieren */

  /* Ueber alle Eintraege gehen */
  for (num_rebuild_info = 0; num_rebuild_info < mmp_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &mmp_rebuild_info[num_rebuild_info];

      if (!strcmp("ELEMENT_MOBILITY_CHANGE", rebuild_info -> name))
        {
          found_mobility = 1;
          if (rebuild_info -> method)
            if ((rebuild_info -> phase == -1) || (phase == rebuild_info -> phase))
              {

                /* Index der Werte holen */
                e_index = rebuild_info -> index_for_reference_values[phase];

                /* Alle Elemente */
                for (i = 0; i < anz_active_elements; i++)
                  {
                    element = ActiveElements[i];
                    ref_val = ElGetElementVal(element, e_index);
                    val = ElGetElementVal(element, e_index + GetRFProcessNumPhases());

                    do_rebuild = 0;
                    do_delete = 0;

                    switch (rebuild_info -> method)
                      {
                      case 1:          /* Differenzen */
                        error = fabs(val - ref_val);
                        if (error > rebuild_info -> param[0])
                          do_rebuild = 1;

                        break;

                      case 2:          /* Rel. Differenzen  */
                        error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error > rebuild_info -> param[0])
                          do_rebuild = 1;

                        break;

                      case 3:          /* Rel. Differenzen und Absolutwert */
                        error1 = fabs(val - ref_val);
                        error2 = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if ((error1 > rebuild_info -> param[0]) && (error2 > rebuild_info -> param[1]))
                          do_rebuild = 1;


                        /* Mit Zufallsaufbau! */
                      case 11:         /* Differenzen */
                        error = fabs(val - ref_val);
                        if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                          do_rebuild = 1;
                        break;

                      case 12:         /* Rel. Differenzen  */
                        error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                          do_rebuild = 1;
                        break;

                      case 13:         /* Rel. Differenzen und Absolutwert */
                        error1 = fabs(val - ref_val);
                        error2 = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error1 / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                          if (error2 / rebuild_info -> param[2] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[3]))
                            do_rebuild = 1;
                        break;

                      default:
                        break;
                      }                /* end-switch fuer do_rebuild */

                    /* Abschalten unnuetzer Teile der Differentialgleichung */
                    if (val < MKleinsteZahl)    /* Bisher noch nicht einstellbar ... */
                      do_delete = 1;

                    if (do_rebuild)
                      {
                        /* Neuaufbau noetig */
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_condu] = 1;
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capil] = 1;
                        /* Wird bei Gausspunktintegration mit erledigt, sonst extra */
                        if (!mmp_element_integration_method_density)
                          element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi] = 1;

                        /* Referenzwert ablegen */
                        ElSetElementVal(element, e_index, val);
                      }                /* endif */

                    if (do_delete)
                      {
                        /* Es kann geloescht werden (Mobilitaet ist Null) */
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_condu] = -1;
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capil] = -1;
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi] = -1;
                        /* Referenzwert ablegen */
                        ElSetElementVal(element, e_index, 0.);
                      }                /* endif */


                  }                    /* endfor */
              }                        /* endif */
        }                              /* endif */

      if (!strcmp("ELEMENT_DENSITY_CHANGE", rebuild_info -> name))
        {
          found_density = 1;
          if (rebuild_info -> method)
            if ((rebuild_info -> phase == -1) || (phase == rebuild_info -> phase))
              {

                /* Index der Werte holen */
                e_index = rebuild_info -> index_for_reference_values[phase];

                /* Alle Elemente */
                for (i = 0; i < anz_active_elements; i++)
                  {
                    element = ActiveElements[i];

                    /* Nur ausfuehren, wenn Mobilitaet > 0 !! */
                    if (element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi] > -1)
                      {

                        ref_val = ElGetElementVal(element, e_index);
                        val = ElGetElementVal(element, e_index + GetRFProcessNumPhases());

                        do_rebuild = 0;

                        switch (rebuild_info -> method)
                          {
                          case 1:     /* Differenzen */
                            error = fabs(val - ref_val);
                            if (error > rebuild_info -> param[0])
                              do_rebuild = 1;
                            break;

                          case 2:     /* Rel. Differenzen  */
                            error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                            if (error > rebuild_info -> param[0])
                              do_rebuild = 1;
                            break;

                            /* Mit Zufallsaufbau! */
                          case 11:    /* Differenzen */
                            error = fabs(val - ref_val);
                            if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                              do_rebuild = 1;
                            break;

                          case 12:    /* Rel. Differenzen  */
                            error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                            if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                              do_rebuild = 1;
                            break;

                          case 13:    /* Rel. Differenzen und Absolutwert */
                            error1 = fabs(val - ref_val);
                            error2 = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                            if (error1 / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[2]))
                              if (error2 / rebuild_info -> param[1] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[2]))
                                do_rebuild = 1;
                            break;

                          default:
                            break;
                          }            /* end-switch */

                        if (do_rebuild)
                          {
                            /* Neuaufbau noetig */
                            /* Nur bei Gaussintegration */
                            if (mmp_element_integration_method_density)
                              element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_condu] = 1;
                            else
                              element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi] = 1;

                            element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capac] = 1;
                            /* Referenzwert ablegen */
                            ElSetElementVal(element, e_index, val);
                          }            /* endif */
                      }                /* endfor */
                  }                    /* endif */
              }                        /* endif */

        }                              /* endif */
      if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
        {
          found_saturation = 1;

          if (rebuild_info -> method)
            if ((rebuild_info -> phase == -1) || (phase == rebuild_info -> phase))
              {

                /* Index der Werte holen */
                e_index = rebuild_info -> index_for_reference_values[phase];

                /* Index der Matrixtypen holen */
                matrix_capac = GetMatrixIndex_MMP("MMPCAPACITANCE0", phase);
                matrix_capil = GetMatrixIndex_MMP("MMPCAPILLARPRESSURE0", phase);

                /* Alle Elemente */
                for (i = 0; i < anz_active_elements; i++)
                  {
                    element = ActiveElements[i];
                    ref_val = ElGetElementVal(element, e_index);
                    val = ElGetElementVal(element, e_index + GetRFProcessNumPhases());

                    do_rebuild = 0;
                    switch (rebuild_info -> method)
                      {
                      case 1:          /* Differenzen */
                        error = fabs(val - ref_val);
                        if (error > rebuild_info -> param[0])
                          do_rebuild = 1;
                        break;

                      case 2:          /* Rel. Differenzen  */
                        error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error > rebuild_info -> param[0])
                          do_rebuild = 1;
                        break;

                        /* Mit Zufallsaufbau! */
                      case 11:         /* Differenzen */
                        error = fabs(val - ref_val);
                        if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                          do_rebuild = 1;
                        break;

                      case 12:         /* Rel. Differenzen  */
                        error = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                          do_rebuild = 1;
                        break;

                      case 13:         /* Rel. Differenzen und Absolutwert */
                        error1 = fabs(val - ref_val);
                        error2 = fabs((val - ref_val) / (val + ref_val + MKleinsteZahl) * 2.);
                        if (error1 / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[2]))
                          if (error2 / rebuild_info -> param[1] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[2]))
                            do_rebuild = 1;
                        break;

                      default:
                        break;
                      }                /* end-switch */

                    if (do_rebuild)
                      {
                        /* Neuaufbau noetig */
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capac] = 1;
                        element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capil] = 1;
                        /* Wird bei Gausspunktintegration mit erledigt, sonst extra */
                        if (!mmp_element_integration_method_density)
                          element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi] = 1;

                        /* Referenzwert ablegen */
                        ElSetElementVal(element, e_index, val);
                      }                /* endif */
                  }                    /* endfor */
              }                        /* endif */
        }                              /* endif */
    }                                  /* endfor */


#ifdef LAEUFT_NOCH_NICHT_SO_WIE_ES_SOLL

  /* Jetzt liegt fuer jede Matrix ggf. ein Eintrag vor. Es werden fuer die neu
     aufzubauenden Matrizen alle zugehoerigen Referenzwerte abgelegt. */

  /* Ueber alle Eintraege gehen */
  for (num_rebuild_info = 0; num_rebuild_info < mmp_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &mmp_rebuild_info[num_rebuild_info];
      if (rebuild_info -> method)
        {

          e_index = rebuild_info -> index_for_reference_values[phase];

          if (!strcmp("ELEMENT_MOBILITY_CHANGE", rebuild_info -> name))
            {
              for (i = 0; i < anz_active_elements; i++)
                {
                  element = ActiveElements[i];
                  if (element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_condu] || element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capil] || element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi])
                    ElSetElementVal(element, e_index, ElGetElementVal(element, e_index + GetRFProcessNumPhases()));
                }
            }
          if (!strcmp("ELEMENT_DENSITY_CHANGE", rebuild_info -> name))
            {
              for (i = 0; i < anz_active_elements; i++)
                {
                  element = ActiveElements[i];
                  if (element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_condu] || element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capac] || element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_gravi])
                    ElSetElementVal(element, e_index, ElGetElementVal(element, e_index + GetRFProcessNumPhases()));
                }
            }
          if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
            {
              for (i = 0; i < anz_active_elements; i++)
                {
                  element = ActiveElements[i];
                  if (element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capil] || element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrix_capac])
                    ElSetElementVal(element, e_index, ElGetElementVal(element, e_index + GetRFProcessNumPhases()));
                }
            }
        }
    }                                  /* endfor */

#endif


#ifdef EFFORT
  output = fopen("effort.mmp", "a");
  fprintf(output, "\n time:%f phase:%ld ", aktuelle_zeit, (long) phase);
#endif

  /* Jetzt werden die Matrizen wirklich aufgebaut */
  for (matrixtyp = 0; matrixtyp < GetNumberOfRegisteredMatrixtypes_MMP(); matrixtyp++)
    {
      if (phase == Array_of_Matrixtypes[matrixtyp].phase)
        {
          typ = Array_of_Matrixtypes[matrixtyp].type;
          for (i = 0; i < anz_active_elements; i++)
            {
              element = ActiveElements[i];
              /* Matrizen ggf. aufbauen, im ersten  Zeitschritt immer aufbauen,
                 wenn keine Infos da sind auch aufbauen !!! */
              if ((element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrixtyp] == 1)
                  || (aktueller_zeitschritt < 2)
                  || (mmp_matrixrebuild_read == 0))
                {
                  //OK CalcElementMatrix_MMP(element, phase, typ);

#ifdef EFFORT
                  rebuild[typ]++;
                  if (typ == 0)
                    {
                      ElSetElementVal(element, mmp_effort_index, ElGetElementVal(element, mmp_effort_index) + 1.);
                      ElSetElementVal(element, mmp_effort_total_index, ElGetElementVal(element, mmp_effort_total_index) + 1.);
                    }
#endif
                }
              else
                {
#ifdef EFFORT
                  notrebuild[typ]++;
#endif
                }

              /* Matrizen ggf. loeschen, im ersten  Zeitschritt immer aufbauen!!! */
              if ((element_rebuild[element * GetNumberOfRegisteredMatrixtypes_MMP() + matrixtyp] == -1) && (aktueller_zeitschritt > 1))
                {
                  DeleteElementMatrix_MMP(element, phase, typ);
                }

              /* endif */
            }                          /* endfor */
#ifdef EFFORT
          printf("Pressure (phase %ld, type %ld):%f\n", (long) phase, (long) typ, (double) rebuild[typ] / (double) (rebuild[typ] + notrebuild[typ]));
          fprintf(output, " %f ", (double) rebuild[typ] / (double) (rebuild[typ] + notrebuild[typ]));
#endif
        }                              /* endif */
    }  /* endfor */



#ifdef EFFORT
  fclose(output);
#endif

  element_rebuild = (int *) Free(element_rebuild);

}





/**************************************************************************
 ROCKFLOW - Funktion: GetMatrixtype_MMP

 Aufgabe:
   Gibt den Index eines vom Kern behandelten Matrixtypen zurueck.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   Anzhal der Matrixtypen

 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/
int GetMatrixtype_MMP(char *name)
{
  if (!strcmp("MMPCONDUCTIVITY0", name))
    return 0;
  if (!strcmp("MMPCAPACITANCE0", name))
    return 1;
  if (!strcmp("MMPGRAVITYVECTOR0", name))
    return 2;
  if (!strcmp("MMPCAPILLARPRESSURE0", name))
    return 3;
  if (!strcmp("MMPDEFORMATION0", name))
    return 4;

  return -1;
}

/**************************************************************************
 ROCKFLOW - Funktion: DeleteElementMatrix_MMP

 Aufgabe:
   Loescht die angeforderte Matrix fuer ein Element

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index     : Elementnummer
     int matrix     : Nummer des Matrixtyps
     int phase      : Nummer der Phase

 Ergebnis:
   - void -

   11/2000   C.Thorenz    Erste Version

**************************************************************************/
void DeleteElementMatrix_MMP(long index, int phase, int matrixtyp)
{
  double *pointer = NULL;

#ifdef TESTCEL_MMP
  DisplayMsgLn("");
  DisplayMsg("Element: ");
  DisplayLong(index);
  DisplayMsg(" Phase: ");
  DisplayLong(phase);
  DisplayMsg(" Type: ");
  DisplayLong(matrixtyp);
#endif

  /*  !!!! Wenn dieser Switch veraendert wird muss auch GetMatrixtype_MMP und DeleteElementMatrix_MMP angepasst werden !!! */
  switch (matrixtyp)
    {
    case 0:
      /* Durchlaessigkeitsmatrix */
      pointer = MMPGetElementConductivityMatrix(index, phase);
      pointer = (double *) Free(pointer);
      MMPSetElementConductivityMatrix(index, phase, NULL);
      break;

    case 1:
      /* Kompressibilitaet und Speicherterme */
      pointer = MMPGetElementCapacitanceMatrix(index, phase);
      pointer = (double *) Free(pointer);
      MMPSetElementCapacitanceMatrix(index, phase, NULL);
      break;

    case 2:
      /* Gravitationsvektor  */
      pointer = MMPGetElementGravityVector(index, phase);
      pointer = (double *) Free(pointer);
      MMPSetElementGravityVector(index, phase, NULL);
      break;

    case 3:
      /* Kapillardruckvektor */
      pointer = MMPGetElementCapillarityVector(index, phase);
      pointer = (double *) Free(pointer);
      MMPSetElementCapillarityVector(index, phase, NULL);
      break;

    case 4:
      /* Dilatation  */
      /* so nicht umsetzbar, da die Funktionen sind standardmaessig
         definiert sind (MK3808)     */ 
/*OK
      pointer = MMPGetElementStrainCouplingMatrixX(index);
      pointer = (double *) Free(pointer);
      MMPSetElementStrainCouplingMatrixX(index, NULL);
      pointer = MMPGetElementStrainCouplingMatrixY(index);
      pointer = (double *) Free(pointer);
      MMPSetElementStrainCouplingMatrixY(index, NULL);
*/
      break;

    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type deleted!");
      break;
    }                                  /* endswitch */
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcElementMatrix_MMP

 Aufgabe:
   Berechnet die angeforderte Matrix fuer ein Element

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index     : Elementnummer
     int matrix     : Nummer des Matrixtyps
     int phase      : Nummer der Phase

 Ergebnis:
   - void -

 09/2002   OK   MMP Triangles

**************************************************************************/
void CalcElementMatrix_MMP(long index, int phase, int matrixtyp,CRFProcess*m_pcs)
{

#ifdef TESTCEL_MMP
  DisplayMsgLn("");
  DisplayMsg("Element: ");
  DisplayLong(index);
  DisplayMsg(" Phase: ");
  DisplayLong(phase);
  DisplayMsg(" Type: ");
  DisplayLong(matrixtyp);
#endif

  switch (ElGetElementType(index))
    {
    case 1:
      CalcEle1D_MMP(index, phase, matrixtyp,m_pcs);
      break;
    case 2:
      CalcEle2D_MMP(index, phase, matrixtyp,m_pcs);
      break;
    case 3:
      CalcEle3D_MMP(index, phase, matrixtyp,m_pcs);
      break;
    case 4:
      MMPCalcElementMatrices2DTriangle(index, phase, matrixtyp,m_pcs); /* OK 3805 */
      break;
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle1D_MMP

 Aufgabe:
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 1D - Elements ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   E int phase: Fluidphase, mit 0 beginnend
   E int matrixtyp: Typ der auzubauenden Matrix

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz       Erste Version
 11/1998  C.Thorenz       Erste rausgegangene Version
 5/1999   C.Thorenz       Umbau auf n-Phasen
 1/2001   C.Thorenz       Zusatzterme Kompressibilitaet
 4/2001   C.Thorenz       Kapillardruckvektoren reduziert auf reine Werte, Multiplikation in CGS_MMP.C
 09/2001  OK   case 4 vorbereitet
 08/2004 OK MFP implementation

**************************************************************************/
void CalcEle1D_MMP(long index, int phase, int matrixtyp,CRFProcess*m_pcs)
{
  static double *invjac, detjac;
  static double A, L,  g, n,  fkt, r, l[3], v[3], mob_a, mob_b;
  static long *element_nodes;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *capillarity_vector;
  //  static double mmp_density;
  static double gp[3];
  static double* permeability;
  static double permeability_rel_l,permeability_rel_r;
  static double permeability_rel;
  //----------------------------------------------------------------------
  // NUM
  mmp_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mmp_upwind_parameter>0.0) 
    mmp_upwind_method = 1;
  //mmp_time_collocation_upwinding = m_pcs->m_num->ls_theta;
  mmp_time_collocation_global = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  /* Determinante und Inverse der Jacobi-Matrix bereitstellen */
  /* detjac = AL/2, invjac = 2/(AL) */
  invjac = GetElementJacobiMatrix(index, &detjac);
  A = m_mmp->geo_area;
  L = 2. * detjac / A;
  element_nodes = ElGetElementNodes(index);
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  //----------------------------------------------------------------------
  // Medium properties 
  group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  n = m_mmp->Porosity(index,gp,mmp_time_collocation_global);
  permeability = m_mmp->PermeabilityTensor(index);
  permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
  //----------------------------------------------------------------------
  // Fluid properties 
  g = gravity_constant;
  double density_gp;
  double density_node[2]; 
  double viscosity_gp;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  density_gp = m_mfp->Density();
  gp[0]=-1.0;
  density_node[0] = m_mfp->Density();
  gp[0]= 1.0;
  density_node[1] = m_mfp->Density();
  viscosity_gp = m_mfp->Viscosity();
  //----------------------------------------------------------------------
  // State functions
  char char_phase[2];
  sprintf(char_phase,"%i",phase+1);
  string nod_val_name = "SATURATION";
  nod_val_name += char_phase;
  double saturation;
  saturation = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
  //----------------------------------------------------------------------
  /*  !!!! Wenn dieser Switch veraendert wird muss auch GetMatrixtype_MMP angepasst werden !!! */
  switch (matrixtyp)
    {
    //---------------------------------------------------------------------------------------------
    case 0:
      /* Durchlaessigkeitsmatrix Beginn */
      /* Speicher holen */
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(4 * sizeof(double));

      /* Upwinding fuer Permeabilitaetsmatrizen */
      if ((mmp_upwind_method == 1) || (mmp_upwind_method == 2))
        {
          // CalcVelo1Dr(phase, index,mmp_time_collocation_upwinding, 0., 0., 0., v); MB
          gp[0]= 0.0;
          gp[1]= 0.0;
          gp[2]= 0.0;
          VELCalcGaussLineLocal(phase, index, gp, m_pcs, v);

          l[0] = GetNodeX(element_nodes[1]) - GetNodeX(element_nodes[0]);
          l[1] = GetNodeY(element_nodes[1]) - GetNodeY(element_nodes[0]);
          l[2] = GetNodeZ(element_nodes[1]) - GetNodeZ(element_nodes[0]);

          /* Verschiebungen auf -1<x<1 begrenzen */
          if (MBtrgVec(v, 3) > MKleinsteZahl) {
            if (MSkalarprodukt(v, l, 3) > 0.)
              r = MRange(-1., mmp_upwind_parameter, 1.);
            else
              r = MRange(-1., -mmp_upwind_parameter, 1.);
          } 
        }
      else
        {
          r = 0.;
        }
//OK_MFP
gp[0]=r;
density_gp = m_mfp->Density();
permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
      /* Maximum Mobility Upwinding */
      if (mmp_upwind_method == 3)
        {
          gp[0]=-1.;
          permeability_rel_l = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
          gp[0]=1.;
          permeability_rel_r = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
          fkt = MMax(permeability_rel_l,permeability_rel_r);
          fkt *= A / L * permeability[0] / viscosity_gp;
        }
      else
        {
          /* Fuer erstes Integral: grad(phi)*kr*k/mue*grad(ome) */
          fkt = A / L * permeability_rel * permeability[0] / viscosity_gp;
        }

      conductance_matrix[0] = fkt;
      conductance_matrix[1] = -fkt;
      conductance_matrix[2] = -fkt;
      conductance_matrix[3] = fkt;

      /* Fuer zweites Integral: -phi*grad(rho)*kr*k/mue*grad(ome) */
#define noGROUP_FEM
#ifdef GROUP_FEM
      mob_a = permeability_rel / viscosity_gp / density_node[0];
      mob_b = permeability_rel / viscosity_gp / density_node[1];
#else
      mob_a = permeability_rel / viscosity_gp / density_gp;
      mob_b = permeability_rel / viscosity_gp / density_gp;
#endif
      /* Maximum Mobility Upwinding */
      if (mmp_upwind_method == 3)
        {
          mob_a = mob_b = MMax(mob_a, mob_b);
        }
      fkt = permeability[0] * A / 6 / L * (density_node[1] - density_node[0]);
      conductance_matrix[0] += fkt * (2 * mob_a + mob_b);
      conductance_matrix[1] -= fkt * (2 * mob_a + mob_b);
      conductance_matrix[2] += fkt * (mob_a + 2 * mob_b);
      conductance_matrix[3] -= fkt * (mob_a + 2 * mob_b);
      MMPSetElementConductivityMatrix(index, phase, conductance_matrix);
#ifdef TESTCEL_MMP
      MZeigMat(conductance_matrix, 2, 2, "ConductivityMatrix");
#endif
      /* Hier gibt es keinen Unterschied zwischen Elementmittlerer- und Gausspunktintegration */
      if (mmp_element_integration_method_density)
        CalcEle1D_MMP(index, phase, 2,m_pcs);
      break;
    //---------------------------------------------------------------------------------------------
    case 1:
      /* Kompressibilitaet und Speicherterme */
      capacitance_matrix = MMPGetElementCapacitanceMatrix(index, phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(4 * sizeof(double));
      /* fkt = (1/rho*n*d_rho/d_p*S + Se*S  ) */
      fkt = (m_mmp->Porosity(index,gp,mmp_time_collocation_global) \
             * m_mfp->drho_dp \
             / density_gp \
             * MMax(0.,saturation) \
             + m_mmp->StorageFunction(index,gp,mmp_time_collocation_global) \
             * MMax(0.,saturation)
          ); /* * A * L / 6.; */
      fkt *= A * L / 6.;
      /* Richards'-Modell */
//SB      if ((GetMethod_MMP()==7)||(GetMethod_MMS()==7))
      if ((GetMethod_MMP()==7))
         fkt += m_mmp->Porosity(index,gp,mmp_time_collocation_global)
             * m_mfp->drho_dp
             * A * L / 6.;
      if (mmp_mass_lumping_method)
        {
          capacitance_matrix[0] = 3. * fkt;
          capacitance_matrix[1] = 0.;
          capacitance_matrix[2] = 0.;
          capacitance_matrix[3] = 3. * fkt;
        }
      else
        {
          capacitance_matrix[0] = 2. * fkt;
          capacitance_matrix[1] = 1. * fkt;
          capacitance_matrix[2] = 1. * fkt;
          capacitance_matrix[3] = 2. * fkt;
        }
      MMPSetElementCapacitanceMatrix(index, phase, capacitance_matrix);
#ifdef TESTCEL_MMP
      MZeigMat(capacitance_matrix, 2, 2, "CapacitanceMatrix");
#endif
      break;
    //---------------------------------------------------------------------------------------------
    case 2:
      /* Gravitationsvektor mit elementgemittelter Dichte */
      gravity_vector = MMPGetElementGravityVector(index, phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(2 * sizeof(double));
      /* Fuer erstes Integral: grad(phi)*kr*k/mue*rho*g */
      fkt = A / L * permeability_rel * permeability[0] / viscosity_gp
          * density_gp
          * g
          * ((GetNode(element_nodes[1])) -> z - (GetNode(element_nodes[0])) -> z);
      gravity_vector[0] = -fkt;
      gravity_vector[1] = fkt;
      /* Fuer zweites Integral: -phi*grad(rho)*kr*k/mue*rho*g */
      mob_a = permeability_rel / viscosity_gp;
      mob_b = permeability_rel / viscosity_gp;
      fkt = permeability[0] * A / 6 / L \
          * (density_node[1] - density_node[0]) \
          * ((GetNode(element_nodes[0])) -> z - (GetNode(element_nodes[1])) -> z);
      gravity_vector[0] += fkt * (2 * mob_a + mob_b);
      gravity_vector[1] += fkt * (mob_a + 2 * mob_b);
      MMPSetElementGravityVector(index, phase, gravity_vector);
#ifdef TESTCEL_MMP
      MZeigVec(gravity_vector, 2, "GravityVector");
#endif
      break;
    //---------------------------------------------------------------------------------------------
    case 3:
      capillarity_vector = MMPGetElementCapillarityVector(index, phase);
      if (!capillarity_vector)
        capillarity_vector = (double*)Malloc(2*sizeof(double));
      //OK capillarity_vector[0] = MMPGetNodeDifferenceToReferencePressure(phase, index, element_nodes[0], mmp_time_collocation_global);
      //OK capillarity_vector[1] = MMPGetNodeDifferenceToReferencePressure(phase, index, element_nodes[1], mmp_time_collocation_global);
      m_mmp->mode = 1;
      capillarity_vector[0] = m_mmp->CapillaryPressureFunction(element_nodes[0],NULL,mmp_time_collocation_global,phase,0.0);
      capillarity_vector[1] = m_mmp->CapillaryPressureFunction(element_nodes[1],NULL,mmp_time_collocation_global,phase,0.0);
      m_mmp->mode = 0;
      MMPSetElementCapillarityVector(index,phase,capillarity_vector);
#ifdef TESTCEL_MMP
      MZeigVec(capillarity_vector,2,"CapillarityVector");
#endif
      break;
    //---------------------------------------------------------------------------------------------
    case 4: /* ToDo*/
      /* Dilatation coupling matrix */
      break;
    //---------------------------------------------------------------------------------------------
    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */

}                                      /* of CalcEle1D_MMP */


/*************************************************************************
 ROCKFLOW - Funktion: CalcEle2D_MMP

 Aufgabe:
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 2D - Elementes ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   E int phase: Fluidphase, mit 0 beginnend
   E int matrixtyp: Typ der auzubauenden Matrix

 Ergebnis:
   - void -

 Programmaenderungen:
 09/1999 CT 2D-Elemente
 07/2000 OK Kopplungsmatrizen fuer Deformationsprozesse
 01/2001 CT Zusatzterme Kompressibilitaet
 04/2001 CT Kapillardruckvektoren reduziert auf reine Werte, Multiplikation in CGS_MMP.C
 11/2002 MB MATCalcFluidDensity
 09/2004 OK MFP/MMP implementation
**************************************************************************/
void CalcEle2D_MMP(long index, int phase, int matrixtyp,CRFProcess*m_pcs)
{
  static double *invjac, detjac;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *capillarity_vector;
  static double gravity_matrix[64];
  static long *element_nodes;
  static double elevation[8];
  static double g, mult;
  static double tkmyt[9], kmy[9], rho_n[4];
  static int anzgp;
  static long i, j, k, l;
  static double r, s, r_upw, s_upw, fkt;
  static double zwi[64];
  static double zwa[64];
  static double zwo[64];
  static double phi[64];
  static double phi_t[64];
  static double grad_phi[64];
  static double grad_phi_t[64];
  static double grad_rho[64];
  static double grad_ome[64];
  static double ome[64];

  static double kr, kr_max, k_xx, k_yy, rho, mue, n,area, *perm;
  static double alpha[2], scale;        /* SUPG */
  static double v[2], v_rs[2];
  static double jacobi[4];
  //  static double invjac_t[4];
  //  static double g_ome_t[8];
  //  static double g_ome_xy_t[8];
  static double *coupling_matrix_u_x, *coupling_matrix_u_y;
  //  static long kl;
  static double gp[3];
  //  static double invjac_9N[4],invjac_t_9N[4],detjac_9N;
  //  static double g_ome_t_9N[18];  
  //  static double g_ome_xy_t_9N[18]; 
  const int nd = 9;
  int nn=4;
  //int nn2=16;
  //----------------------------------------------------------------------
  // NUM
  mmp_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mmp_upwind_parameter>0.0) 
    mmp_upwind_method = 1;
  mmp_time_collocation_upwinding = m_pcs->m_num->ls_theta;
  mmp_time_collocation_global = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  //----------------------------------------------------------------------
  // Geometry
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  area = m_mmp->geo_area;
  //----------------------------------------------------------------------
  // Medium properties 
  n = m_mmp->Porosity(index,gp,mmp_time_collocation_global);
  perm = m_mmp->PermeabilityTensor(index);
  double permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
  //----------------------------------------------------------------------
  // Fluid properties 
  g = gravity_constant;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  double density_gp;
  double density_node[2]; 
  double viscosity_gp;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  density_gp = m_mfp->Density();
  gp[0]=-1.0;
  density_node[0] = m_mfp->Density();
  gp[0]= 1.0;
  density_node[1] = m_mfp->Density();
  viscosity_gp = m_mfp->Viscosity();
  //----------------------------------------------------------------------
  // State functions
  char char_phase[2];
  sprintf(char_phase,"%i",phase+1);
  string nod_val_name = "SATURATION";
  nod_val_name += char_phase;
  double saturation;
  saturation = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
  //======================================================================
  switch(matrixtyp){
    //--------------------------------------------------------------------
    // Durchlaessigkeitsmatrix
    case 0:
      // Speicher holen
      conductance_matrix = MMPGetElementConductivityMatrix(index,phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(16 * sizeof(double));
      // Schwerkraftterme ueber Gausspunkte
      if (mmp_element_integration_method_density){
        gravity_vector = MMPGetElementGravityVector(index, phase);
        if (!gravity_vector)
          gravity_vector = (double *) Malloc(4 * sizeof(double));
        MNulleMat(gravity_matrix, 4, 4);
        MNulleVec(gravity_vector, 4);
      }
      MNulleMat(conductance_matrix, 4, 4);
      MNulleMat(kmy, 3, 3);
      MNulleVec(alpha, 2);
      //..................................................................
      // Upwinding fuer Permeabilitaetsmatrizen 
      r_upw = s_upw = 0.;
      if ((mmp_upwind_method == 1)||(mmp_upwind_method == 2)){

        // CalcVelo2Drs(phase, index,mmp_time_collocation_upwinding, 0., 0., 0., v); MB
        gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
        VELCalcGaussQuadLocal(phase, index, gp, m_pcs, v);

        // Geschwindigkeitstransformation: a,b -> r,s 
        Calc2DElementJacobiMatrix(index,0.,0.,invjac,&detjac);
        MKopierVec(invjac,jacobi,4);
        M2InvertiereUndTransponiere(jacobi);  // MK_bitte_pruefen!!!: Transponierte der Jacobi-Matrix
        MMultMatVec(jacobi, 2, 2, v, 2, v_rs, 2);
        if (MBtrgVec(v_rs,2)>DBL_MIN){
          // Upwind-Faktoren
          for (l = 0; l < 2; l++)
            alpha[l] = -mmp_upwind_parameter * (v_rs[l] / MBtrgVec(v_rs, 2));
        }
//OK_CHECK
//        alpha[0] = 0.5;
//        alpha[1] = 0.0;
        if (mmp_upwind_method==1){
          // Verschiebungen der Gausspunkte auf Element begrenzen
          scale = 1.;
          if (fabs(alpha[0]) > 1.)
            scale = MMin(scale, 1. / fabs(alpha[0]));
          if (fabs(alpha[1]) > 1.)
            scale = MMin(scale, 1. / fabs(alpha[1]));
          r_upw = scale * alpha[0];
          s_upw = scale * alpha[1];
        }
        if (mmp_upwind_method==2){
          // Verschiebungen auf -1<x<1 begrenzen
          r_upw = MRange(-1., alpha[0], 1.);
          s_upw = MRange(-1., alpha[1], 1.);
        }
      }
      //..................................................................
      // Maximum Mobility Upwinding
      if(mmp_upwind_method==3){
        kr_max=0.;
        m_mmp->mode = 1;
        for(i=0;i<4;i++)
          permeability_rel = MMax(kr_max,m_mmp->PermeabilitySaturationFunction(element_nodes[i],gp,mmp_time_collocation_upwinding,phase));
          //kr_max = MMax(kr_max, MMPGetNodeRelativePermeability(phase, index, element_nodes[i], GetTimeCollocationupwind_MMP()));
        m_mmp->mode = 0;
      }
      //..................................................................
      // Permeabilitaet holen 
      k_xx = perm[0]; //GetSoilPermeability(index, 0);
      k_yy = perm[3]; //GetSoilPermeability(index, 1);
      // Alle Groessen nur in Elementmitte betrachten? Dann geht's so schneller.
      //kr = MMPGetRelativePermeability(phase, index, r_upw, s_upw, 0., mmp_time_collocation_global);
      gp[0]=r_upw; gp[1]=s_upw; gp[2]=0.0;
      kr = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
//OK
      if(kr<MKleinsteZahl) kr = MKleinsteZahl;
      gp[0]=r_upw; gp[1]=s_upw; gp[2]=0.0;
      mue = m_mfp->Viscosity();
      gp[0]=alpha[0]; gp[1]=alpha[1]; gp[2]=0.0;
      rho = m_mfp->Density();
      /* Maximum Mobility Upwinding */
      if (mmp_upwind_method == 3)
        kr = kr_max;
      kmy[0] = (k_xx) / mue * kr;
      kmy[3] = (k_yy) / mue * kr;
      m_mfp->mode=1;
      for(l=0;l<4;l++)
        rho_n[l] = m_mfp->Density();
      m_mfp->mode=0;
      //..................................................................
      // Schleife ueber GaussPunkte
      for(i=0;i<anzgp;i++){
        for(j=0;j<anzgp;j++){
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          // Leitfaehigkeitsmatrix ohne Zeitanteile
          // Integration ueber Gausspunkte
          if (mmp_element_integration_method_maximum){
            r_upw = r;
            s_upw = s;
            if(mmp_upwind_method==1){
              // Verschiebungen der Gausspunkte auf Element begrenzen
              scale = 1.;
              if (fabs(r + alpha[0]) > 1.)
                scale = MMin(scale, (1. - fabs(r)) / fabs(alpha[0]));
              if (fabs(s + alpha[1]) > 1.)
                scale = MMin(scale, (1. - fabs(s)) / fabs(alpha[1]));
              r_upw = r + scale * alpha[0];
              s_upw = s + scale * alpha[1];
            }
            if(mmp_upwind_method==2){
              // Verschiebungen auf -1<x<1 begrenzen 
              r_upw = MRange(-1., r + alpha[0], 1.);
              s_upw = MRange(-1., s + alpha[1], 1.);
            }
            gp[0]=r_upw; gp[1]=s_upw; gp[2]=0.0;
            if(mmp_element_integration_method_rel_perm)
              kr = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
            if(kr<MKleinsteZahl) kr = MKleinsteZahl;
            if(mmp_element_integration_method_viscosity)
              mue = m_mfp->Viscosity();
            if (mmp_element_integration_method_density)
              rho = m_mfp->Density();
            if (mmp_upwind_method == 3)
              kr = kr_max;
            kmy[0] = (k_xx) / mue * kr;
            kmy[3] = (k_yy) / mue * kr;
          }
          // J^-1 
          Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
          fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
          // tkmyt = J^-1 * (K/my) * (J^-1)T 
          MMultMatMat(invjac, 2, 2, kmy, 2, 2, zwa, 2, 2);
          MTranspoMat(invjac, 2, 2, zwi);
          MMultMatMat(zwa, 2, 2, zwi, 2, 2, tkmyt, 2, 2);
          // conductance matrix und gravity matrix
          // Ansatz- und Wichtungsfunktionen holen 
          // grad(phi)T 
          MGradPhi2D(grad_phi_t, r, s);     /* 2 Zeilen 4 Spalten */
          MTranspoMat(grad_phi_t, 2, 4, grad_phi);  /* jetzt: 4 Zeilen 2 Spalten */
          // (phi)T 
          MPhi2D(phi_t, r, s);     /* 1 Zeile 4 Spalten */
          MTranspoMat(phi_t, 1, 4, phi);    /* jetzt: 4 Zeilen 1 Spalte */
          // grad(omega) 
          MGradOmega2D(grad_ome, r, s);     /* 2 Zeilen 4 Spalten */
          // Fuer erstes Integral: grad(phi)*kr*k/mue*grad(ome)
          // grad(phi) * tkmyt
          MMultMatMat(grad_phi, 4, 2, tkmyt, 2, 2, zwa, 4, 2);
          // grad(phi) * tkmyt * grad(omega) */
          MMultMatMat(zwa, 4, 2, grad_ome, 2, 4, zwi, 4, 4);
          for(k=0;k<16;k++){
            conductance_matrix[k] += (zwi[k] * fkt);
          }
          // Schwerkraftterme ueber Gausspunkte
          if(mmp_element_integration_method_density){
            mult = fkt * rho * g;
            for(k=0;k<16;k++){
              gravity_matrix[k] += zwi[k] * mult;
            }
          }
          // Fuer zweites Integral: -phi*grad(rho)*kr*k/mue*grad(ome)
          // => grad(rho)
          MMultMatVec(grad_ome, 2, 4, rho_n, 4, grad_rho, 2);
          // => phi * grad(rho)
          MMultMatMat(grad_rho, 2, 1, phi, 1, 4, zwi, 2, 4);
          MTranspoMat(zwi, 2, 4, zwo);      /* jetzt: 4 Zeilen 2 Spalten */
          // => phi * grad(rho) * tkmyt
          MMultMatMat(zwo, 4, 2, tkmyt, 2, 2, zwa, 4, 2);
          // => phi * grad(rho) * tkmyt * grad(omega) 
          MMultMatMat(zwa, 4, 2, grad_ome, 2, 4, zwi, 4, 4);
          for(k=0;k<16;k++){
            mult = fkt / rho;
            conductance_matrix[k] -= (zwi[k] * mult);
          }
          // Schwerkraftterme ueber Gausspunkte 
          if(mmp_element_integration_method_density){
            mult = fkt * g;
            for(k=0;k<16;k++){
              gravity_matrix[k] -= zwi[k] * mult;
            }
          }
        } // Ende der Schleife ueber GaussPunkte 
      } // Ende der Schleife ueber GaussPunkte 
      for (i = 0; i < 16; i++)
        conductance_matrix[i] *= area;
      MMPSetElementConductivityMatrix(index, phase, conductance_matrix);
      // Schwerkraftterme ueber Gausspunkte
      if(mmp_element_integration_method_density){
        for(i=0;i<16;i++)
          gravity_matrix[i] *= area;
        // Schwerkraftterm berechnen
        for(i=0;i<4;i++)
          elevation[i] = ((GetNode(element_nodes[i])) -> z);
        MMultMatVec(gravity_matrix, 4, 4, elevation, 4, gravity_vector, 4);
        MMPSetElementGravityVector(index, phase, gravity_vector);
#ifdef TESTCEL_MMP
        MZeigVec(gravity_vector, 4, "Gauss-Integrated GravityVector");
#endif
      }
#ifdef TESTCEL_MMP
      MZeigMat(conductance_matrix, 4, 4, "ConductivityMatrix");
#endif
      break;
    //--------------------------------------------------------------------
    // Kompressibilitaet und Speicherterme
    case 1:
      capacitance_matrix = MMPGetElementCapacitanceMatrix(index, phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(16 * sizeof(double));
      MNulleMat(capacitance_matrix, 4, 4);
      //..................................................................
      for(i=0;i<anzgp;i++){
        for(j=0;j<anzgp;j++){
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
          // fkt = (1/rho*n*d_rho/d_p*S + Se*S + n*dS/dp)  Elementweise konstante Werte ausserhalb der Schleife
          fkt = m_mmp->Porosity(index,gp,mmp_time_collocation_global) \
              * m_mfp->drho_dp \
              / density_gp \
              * MMax(0.,saturation) \
              + m_mmp->StorageFunction(index,gp,mmp_time_collocation_global) \
              * MMax(0.,saturation);
          // Richards'-Modell 
//SB          if ((GetMethod_MMP()==7)||(GetMethod_MMS()==7))
      if ((GetMethod_MMP()==7))
            fkt += n * MMPGetSaturationPressureDependency(phase, index, r, s, 0., mmp_time_collocation_global);
          // Gauss-Faktoren g_i * g_j  * g_k * det[J_2D] 
          fkt *= MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;
          // Ansatzfunktion im Gauss-Punkt omega==N 
          MOmega2D(ome, r, s);
          if(mmp_mass_lumping_method){
            // Wenn die Speicherung per ML-FEM behandelt wird, wird nur die Diagonale gesetzt
            for(k=0;k<4;k++)
              capacitance_matrix[k * 5] += ome[k] * fkt;
          }
          else{
            // Normale FEM
            // Wichtungsfunktionen im Gauss-Punkt phi==N
            MPhi2D(phi, r, s);
            // Berechnen phi * omega
            MMultVecVec(phi, 4, ome, 4, zwi, 4, 4);
            for(k=0;k<16;k++)
              capacitance_matrix[k] += zwi[k] * fkt;
          }
        }
      } // Ende der Schleife ueber die Gauss-Punkte
      for(i=0;i<16;i++)
        capacitance_matrix[i] *= area;
      MMPSetElementCapacitanceMatrix(index,phase,capacitance_matrix);
#ifdef TESTCEL_MMP
      MZeigMat(capacitance_matrix, 4, 4, "CapacitanceMatrix");
#endif
      break;
    //--------------------------------------------------------------------
    // Gravitationsvektor mit elementgemittelter Dichte ?
    case 2:
      if(mmp_element_integration_method_density){
        // Nein, Gausspunkt-Integration !
        CalcEle2D_MMP(index, phase, 0,m_pcs);
        break;
      }
      // Gravitationsvektor mit elementgemittelter Dichte !
      gravity_vector = MMPGetElementGravityVector(index, phase);
      if(!gravity_vector)
        gravity_vector = (double *) Malloc(4 * sizeof(double));
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if(!conductance_matrix){
        CalcEle2D_MMP(index, phase, 0,m_pcs);
        conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      }
      MNulleMat(gravity_matrix, 4, 4);
      MNulleVec(gravity_vector, 4);
      // Schwerkraftterm berechnen (gravity vector)
      for(l=0;l<4;l++)
        elevation[l] = ((GetNode(element_nodes[l])) -> z) \
                     * density_gp \
                     * g;
      MMultMatVec(conductance_matrix, 4, 4, elevation, 4, gravity_vector, 4);
      MMPSetElementGravityVector(index, phase, gravity_vector);
#ifdef TESTCEL_MMP
      MZeigVec(gravity_vector, 4, "GravityVector");
#endif
      break;
    //--------------------------------------------------------------------
    // capillarity_vector
    case 3:
      capillarity_vector = MMPGetElementCapillarityVector(index, phase);
      if(!capillarity_vector)
        capillarity_vector = (double*) Malloc(4*sizeof(double));
      m_mmp->mode = 1;
      for(l=0;l<4;l++){
        //OK capillarity_vector[l] = MMPGetNodeDifferenceToReferencePressure(phase, index, element_nodes[l], mmp_time_collocation_global);
        capillarity_vector[l] = m_mmp->CapillaryPressureFunction(element_nodes[l],NULL,mmp_time_collocation_global,phase,0.0);
      }
      m_mmp->mode = 0;
      MMPSetElementCapillarityVector(index, phase, capillarity_vector);
#ifdef TESTCEL_MMP
      MZeigVec(capillarity_vector, 4, "CapillarityVector");
#endif
      break;
    //--------------------------------------------------------------------
    // Dilatation coupling matrix
    case 4: // based on MPC implementation by WW
    if(phase==1) {
    m_mfp = mfp_vector[phase];
    if(GetRFProcessProcessing("SD")) {
      Init_Quadratic_Elements();
      coupling_matrix_u_x = MMPGetElementStrainCouplingMatrixX(index,phase);
      if (!coupling_matrix_u_x)
        coupling_matrix_u_x = (double *) Malloc(nn*nd* sizeof(double));
      coupling_matrix_u_y = MMPGetElementStrainCouplingMatrixY(index,phase);
      if (!coupling_matrix_u_y)
        coupling_matrix_u_y = (double *) Malloc(nn*nd* sizeof(double));
      MNulleMat(coupling_matrix_u_x,nn,nd);
      MNulleMat(coupling_matrix_u_y,nn,nd);
/*
      elem_dm->ConfigElement(index, 2);
      elem_dm->ComputeStrainCouplingMatrix(0, coupling_matrix_u_x);
      elem_dm->ComputeStrainCouplingMatrix(1, coupling_matrix_u_y);

      fkt *= InterpolValue(index,PCSGetNODValueIndex("SATURATION2",1), 0., 0., 0.);
      fkt *= m_mfp->Density();

      for (i=0;i<(nn*nd);i++) {
        coupling_matrix_u_x[i] *= area*fkt;
        coupling_matrix_u_y[i] *= area*fkt;
      }
*/
     MMPSetElementStrainCouplingMatrixX(index,phase,coupling_matrix_u_x);
      MMPSetElementStrainCouplingMatrixY(index,phase,coupling_matrix_u_y);
#ifdef TEST_CEL_MMP
      MZeigMat(coupling_matrix_u_x,nn,nn, "MMPStrainCouplingMatrixX");
      MZeigMat(coupling_matrix_u_y,nn,nn, "MMPStrainCouplingMatrixY");
#endif
      Init_Linear_Elements();
    }
    }
      break;
    //--------------------------------------------------------------------
    default:
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type requested!");
      break;
    }
}


/*************************************************************************
 ROCKFLOW - Funktion: CalcEle3D_MMP

 Aufgabe:
   Berechnet die Speichermatrix (capacitance matrix),
   die Hydr. Leitfaehigkeitsmatrix (conductance matrix) und
   den Schwerkraftvektor (gravity vector)
   des angegebenen 3D - Elementes ohne 1/dt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   E int phase: Fluidphase, mit 0 beginnend
   E int matrixtyp: Typ der auzubauenden Matrix

 Ergebnis:
   - void -

 Programmaenderungen:
 01/1999   C.Thorenz       Erste Version
 05/1999   C.Thorenz       Umbau auf n-Phasen
 01/2001   C.Thorenz       Zusatzterme Kompressibilitaet
 04/2001   C.Thorenz       Kapillardruckvektoren reduziert auf reine Werte, Multiplikation in CGS_MMP.C
 08/2002   M.Kohlmeier     Kopplungsgroeßen DM -> MMP (coupling_matrix_pu)
 11/2002   MB   MATCalcFluidDensity
 10/2004   MX MFP/MMP implementation
*************************************************************************/
void CalcEle3D_MMP(long index, int phase, int matrixtyp,CRFProcess*m_pcs)
{
  static double *invjac, detjac;
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *capillarity_vector;
  static double gravity_matrix[64], rho_n[8];
  static long *element_nodes;
  static double elevation[8];
  static double g; //, mult;
  static double tkmyt[9], kmy[9];
  static int anzgp;
  static long i, j, k, l; //, m;
  static double r, s, t, r_upw, s_upw, t_upw, fkt;
  //  static double zwi[64];
  //  static double zwa[64];
  //  static double zwo[64];
  static double phi[64];
  //static double phi_t[64];
  // static double ome[64];
  static double grad_phi[64];
  //static double grad_phi_t[64];
  //static double grad_ome[64];
  static double grad_rho[3]; //WW [64];
  static double kr, kr_max, rho, mue, n, *perm, scale;
  static double alpha[3];       /* SUPG */
  static double v[3], v_rst[3];
  static double jacobi[9];
/* DM -> MMP - Kopplungsgroeßen */
  double *coupling_matrix_u_x, *coupling_matrix_u_y, *coupling_matrix_u_z;
  static double gp[3]={0.,0.,0.};
  //WW
  int ii,jj,kk,ll;
  int ele_dim, nnodes, nnodesHQ;
  double val1, val2; 
  ele_dim = 3;
  nnodes = 8;
  nnodesHQ = 20;
  //----------------------------------------------------------------------
  // NUM
  mmp_upwind_parameter = m_pcs->m_num->ele_upwinding;
  if(mmp_upwind_parameter>0.0) 
    mmp_upwind_method = 1;
  //OK mmp_time_collocation_upwinding = m_pcs->m_num->ls_theta;
  mmp_time_collocation_global = m_pcs->m_num->ls_theta;
  //----------------------------------------------------------------------
  // Geometry
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  anzgp = GetNumericsGaussPoints(ElGetElementType(index));
  //----------------------------------------------------------------------
  // Medium properties 
  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  n = m_mmp->Porosity(index,gp,mmp_time_collocation_global);
  perm = m_mmp->PermeabilityTensor(index);
  double permeability_rel = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
  double S = m_mmp->StorageFunction(index,gp,mmp_time_collocation_global);
  //----------------------------------------------------------------------
  // Fluid properties 
  g = gravity_constant;
  gp[0]=0.0; gp[1]=0.0; gp[2]=0.0;
  double density_gp;
  double density_node[2]; 
  double viscosity_gp;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  density_gp = m_mfp->Density();
  gp[0]=-1.0;
  density_node[0] = m_mfp->Density();
  gp[0]= 1.0;
  density_node[1] = m_mfp->Density();
  viscosity_gp = m_mfp->Viscosity();
  //----------------------------------------------------------------------
  // State functions
  char char_phase[2];
  sprintf(char_phase,"%i",phase+1);
  string nod_val_name = "SATURATION";
  nod_val_name += char_phase;
  double saturation;
  saturation = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
  double saturation_gp;
  //----------------------------------------------------------------------
  // 
  g = gravity_constant;
  //======================================================================
  /*  !!!! Wenn dieser Switch veraendert wird muss auch GetMatrixtype_MMP und DeleteElementMatrix_MMP angepasst werden !!! */
  switch (matrixtyp){
    //--------------------------------------------------------------------
    case 0:
      /* Durchlaessigkeitsmatrix Beginn */
      /* Speicher holen */
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(64 * sizeof(double));
      /* Schwerkraftterme ueber Gausspunkte */
      if (mmp_element_integration_method_density)
      {
        gravity_vector = MMPGetElementGravityVector(index, phase);
        if (!gravity_vector)
          gravity_vector = (double *) Malloc(8 * sizeof(double));
        /* Initialisieren */
        MNulleMat(gravity_matrix, 8, 8);
        MNulleVec(gravity_vector, 8);
      }
      /* Initialisieren */
      MNulleMat(conductance_matrix, 8, 8);
      MNulleMat(kmy, 3, 3);
      MNulleVec(alpha, 3);
      /* Upwinding fuer Permeabilitaetsmatrizen */
      r_upw = s_upw = t_upw;
      if((mmp_upwind_method==1)||(mmp_upwind_method==2)){

        //CalcVelo3Drst(phase,index,mmp_time_collocation_upwinding,0.,0.,0.,v); MB
        gp[0]= 0.0;
        gp[1]= 0.0;
        gp[2]= 0.0;
        VELCalcGaussHexLocal(phase, index, gp, m_pcs, v);

        /* Geschwindigkeitstransformation: x,y,z -> r,s,t */
        Calc3DElementJacobiMatrix(index, 0., 0., 0., invjac, &detjac);
        MKopierVec(invjac, jacobi, 9);
        M3Invertiere(jacobi);        /* zurueck zur Jacobi-Matrix */
        MMultMatVec(jacobi, 3, 3, v, 3, v_rst, 3);
        MNulleVec(alpha, 3);
        if (MBtrgVec(v_rst, 3) > MKleinsteZahl){
          /* Upwind-Faktoren */
          for (l = 0; l < 3; l++)
            alpha[l] = -mmp_upwind_parameter * v_rst[l] / MBtrgVec(v_rst, 3);
        }
        if(mmp_upwind_method==1){
          /* Verschiebungen der Gausspunkte auf Element begrenzen */
          scale = 1.;
          if (fabs(alpha[0]) > 1.)
            scale = MMin(scale, 1. / fabs(alpha[0]));
          if (fabs(alpha[1]) > 1.)
            scale = MMin(scale, 1. / fabs(alpha[1]));
          if (fabs(alpha[2]) > 1.)
            scale = MMin(scale, 1. / fabs(alpha[2]));
          r_upw = scale * alpha[0];
          s_upw = scale * alpha[1];
          t_upw = scale * alpha[2];
        }
        if(mmp_upwind_method==2){
          /* Verschiebungen auf -1<x<1 begrenzen */
          r_upw = MRange(-1., alpha[0], 1.);
          s_upw = MRange(-1., alpha[1], 1.);
          t_upw = MRange(-1., alpha[2], 1.);
        }
      }
      gp[0]=r_upw; gp[1]=s_upw; gp[2]=t_upw;
      /* Maximum Mobility Upwinding */
      m_mmp->mode=1;
      if(mmp_upwind_method==3){
        kr_max = 0.;
        for (i = 0; i < 8; i++)
          //OK kr_max = MMax(kr_max, MMPGetNodeRelativePermeability(phase, index, element_nodes[i], GetTimeCollocationupwind_MMP()));
          kr_max = MMax(kr_max,permeability_rel);
      }
      /* Permeabilitaet nur in Elementmitte ? */
      m_mmp->mode=0;
      gp[0]=r_upw; gp[1]=s_upw; gp[2]=t_upw; //OK 0.5;
      kr = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
      mue = m_mfp->Viscosity();
      gp[0]=alpha[0]; gp[1]=alpha[1]; gp[2]=alpha[2];
      rho = m_mfp->Density();
      if (mmp_upwind_method == 3)
        kr = kr_max;
      kmy[0] = (perm[0]) / mue * kr;
      kmy[4] = (perm[4]) / mue * kr;
      kmy[8] = (perm[8]) / mue * kr;
      m_mfp->mode=1;
      for(l=0;l<8;l++)
        rho_n[l] = m_mfp->Density();
      m_mfp->mode=0;
      /* Schleife ueber GaussPunkte */
      for (i = 0; i < anzgp; i++)
        {
          for (j = 0; j < anzgp; j++)
            {
              for (k = 0; k < anzgp; k++)
                {
                  r = MXPGaussPkt(anzgp, i);
                  s = MXPGaussPkt(anzgp, j);
                  t = MXPGaussPkt(anzgp, k);
                  r_upw = r;
                  s_upw = s;
                  t_upw = t;
                  /* Leitfaehigkeitsmatrix ohne Zeitanteile */
                  /* Integration ueber Gausspunkte */

                  /* Integration ueber Gausspunkte */
                  if (mmp_element_integration_method_maximum)
                    {
                      /* Verschieben der Integrationspunkte fuer Mobilitaeten ? */
                      r_upw = r;
                      s_upw = s;
                      t_upw = t;
                      if (mmp_upwind_method == 1)
                        {
                          /* Verschiebungen auf Element begrenzen */
                          scale = 1.;
                          if (fabs(r + alpha[0]) > 1.)
                            scale = MMin(scale, (1. - fabs(r)) / fabs(alpha[0]));
                          if (fabs(s + alpha[1]) > 1.)
                            scale = MMin(scale, (1. - fabs(s)) / fabs(alpha[1]));
                          if (fabs(t + alpha[2]) > 1.)
                            scale = MMin(scale, (1. - fabs(t)) / fabs(alpha[2]));
                          r_upw = r + scale * alpha[0];
                          s_upw = s + scale * alpha[1];
                          t_upw = t + scale * alpha[2];
                        }
                      if (mmp_upwind_method == 2)
                        {
                          /* Verschiebungen auf -1<x<1 begrenzen */
                          r_upw = MRange(-1., r + alpha[0], 1.);
                          s_upw = MRange(-1., s + alpha[1], 1.);
                          t_upw = MRange(-1., t + alpha[2], 1.);
                        }
						gp[0]= r_upw;
						gp[1]= s_upw;
						gp[2]= t_upw;
                      if (mmp_element_integration_method_rel_perm)
                        kr = m_mmp->PermeabilitySaturationFunction(index,gp,mmp_time_collocation_global,phase);
                      if (mmp_element_integration_method_viscosity)
						mue = m_mfp->Viscosity();
                      if (mmp_element_integration_method_density)
					  rho = m_mfp->Density();  //ToDo MX 2004

                      if (mmp_upwind_method == 3)
                        kr = kr_max;

                      kmy[0] = (perm[0]) / mue * kr;
                      kmy[4] = (perm[4]) / mue * kr;
                      kmy[8] = (perm[8]) / mue * kr;
                    }
                  Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
                  /* Faktor */
                  fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * fabs(detjac);

                  /* tkmyt = (J^-1)T * (K/my) * J^-1 */
                  //WW
                  for(ii=0; ii<ele_dim; ii++)
                  {
                     // Gradient rho

                     for(jj=0; jj<ele_dim; jj++)
                     {
                         tkmyt[ii*ele_dim+jj] = 0.0;
                         for(kk=0; kk<ele_dim; kk++)
                         {
                             for(ll=0; ll<ele_dim; ll++)
                                 tkmyt[ii*ele_dim+jj]
                                    += invjac[kk*ele_dim+ii]
                                       *kmy[kk*ele_dim+ll]* invjac[ll*ele_dim+jj];
                         } 
                     }
                  }

                  /* Ansatz- und Wichtungsfunktionen holen */
                  // Gradient of shape funtions
                  MGradPhi3D(grad_phi, r, s, t);      
                  // Shape funtions
                  MPhi3D(phi, r, s, t);     
                  // Gradient of the density
                  for(ii=0; ii<ele_dim; ii++)
                  {
                     grad_rho[ii] = 0.0;
                     rho = 0.0;
                     for(jj=0; jj<nnodes; jj++)
                     {
                         rho += phi[jj]*rho_n[jj];
                         grad_rho[ii] += grad_phi[ii*nnodes+jj]*rho_n[jj];
                     }
                  }

                  // Conductance and gravity vector
                  for(ii=0; ii<nnodes; ii++)
                  {
                     for(jj=0; jj<nnodes; jj++)
                     {
                         val1 = 0.0;
                         val2 = 0.0;
                         for(kk=0; kk<ele_dim; kk++)
                         {
                             for(ll=0; ll<ele_dim; ll++)
                             {
                                 val1 += grad_phi[kk*nnodes+ii]*tkmyt[ele_dim*kk+ll]*grad_phi[ll*nnodes+jj];
                                 val2 -= phi[ii]*grad_rho[kk]*tkmyt[ele_dim*kk+ll]*grad_phi[ll*nnodes+jj];
                             }
                         }
                         conductance_matrix[ii*nnodes+jj] += (val1+val2)*fkt;
                         // 
                         if (mmp_element_integration_method_density)
                             gravity_matrix[ii*nnodes+jj] += (rho *val1+ val2/rho)*g*fkt;
                     }
                  }

                }                      /* Ende der Schleife ueber GaussPunkte */
            }                          /* Ende der Schleife ueber GaussPunkte */
        }                              /* Ende der Schleife ueber GaussPunkte */

      /* Schwerkraftterme ueber Gausspunkte */
      if (mmp_element_integration_method_density)
        {
          for (l = 0; l < 8; l++)
            elevation[l] = ((GetNode(element_nodes[l])) -> z);
          MMultMatVec(gravity_matrix, 8, 8, elevation, 8, gravity_vector, 8);
          MMPSetElementGravityVector(index, phase, gravity_vector);

#ifdef TESTCEL_MMP
          MZeigVec(gravity_vector, 4, "Gauss-Integrated GravityVector");
#endif
        }
      MMPSetElementConductivityMatrix(index, phase, conductance_matrix);
      /* Durchlaessigkeitsmatrix Ende */

#ifdef TESTCEL_MMP
      MZeigMat(conductance_matrix, 8, 8, "ConductivityMatrix");
#endif
      break;
    //--------------------------------------------------------------------
    case 1:
      /* Kompressibilitaet und Speicherterme */
      capacitance_matrix = MMPGetElementCapacitanceMatrix(index, phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(64 * sizeof(double));

      /* Initialisieren */
      MNulleMat(capacitance_matrix, 8, 8);


      for (i = 0; i < anzgp; i++)
        {
          for (j = 0; j < anzgp; j++)
            {
              for (k = 0; k < anzgp; k++)
                {
                  r = MXPGaussPkt(anzgp, i);
                  s = MXPGaussPkt(anzgp, j);
                  t = MXPGaussPkt(anzgp, k);
                  gp[0]=r;gp[1]=s;gp[2]=t;
                  /* Berechnen der Element-Jacobi-Matrix [J_3D],
                     ihrer Determinante det[J_3D] sowie Inversen [J_3D^-1]
                     in den Gauss-Punkten */
                  Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);

                  /* fkt *= (1/rho*n*d_rho/d_p*S + Se*S + n*dS/dp)  Elementweise konstante Werte ausserhalb der Schleife */
//                  fkt = (n * GetFluidDensityPressureDependency(phase)
                  saturation_gp = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
                  fkt = (n * m_mfp->drho_dp
                           / m_mfp->Density()
                           * MMax(0.,saturation_gp)
                           + S
                           * MMax(0.,saturation_gp)
                         );

                  /* Richards'-Modell */
                  if (GetMethod_MMP()==7) //OK ||(GetMethod_MMS()==7))
                     fkt += n * MMPGetSaturationPressureDependency(phase, index, r, s, t, mmp_time_collocation_global);

                  /* Gauss-Faktoren g_i * g_j  * g_k * det[J_3D] */
                  fkt *= MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * MXPGaussFkt(anzgp, k) * detjac;

                  /* Ansatzfunktionen auswerten */
                  MPhi3D(phi, r, s, t);

                  if (mmp_mass_lumping_method)
                    {
                      /* Wenn die Speicherung per ML-FEM
                         behandelt wird, wird nur die Diagonale gesetzt */
                      for (l = 0; l < 8; l++)
                        capacitance_matrix[l * 9] += phi[l] * fkt;
                    }
                  else
                    {
                      /* Normale FEM */
                      /* Wichtungsfunktionen im Gauss-Punkt phi==N */
                      MPhi3D(phi, r, s, t);
                      //WW
                      for(ii=0; ii<nnodes; ii++)
                      {
                         for(jj=0; jj<nnodes; jj++)
                            capacitance_matrix[ii*nnodes+jj] = phi[ii]*phi[jj]*fkt;
                      }
                    }                  /* endif */
                }
            }
        }                              /* Ende der Schleife ueber die Gauss-Punkte */

      MMPSetElementCapacitanceMatrix(index, phase, capacitance_matrix);
      break;
    //--------------------------------------------------------------------
    case 2:
      /* Gravitationsvektor mit elementgemittelter Dichte? */
      if (mmp_element_integration_method_density)
        {
          /* Nein, Gausspunkt-Integration ! */
          CalcEle3D_MMP(index, phase, 0,m_pcs);
          break;
        }
      /* Gravitationsvektor mit elementgemittelter Dichte! */
      gravity_vector = MMPGetElementGravityVector(index, phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(8 * sizeof(double));
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if (!conductance_matrix)
        {
          CalcEle3D_MMP(index, phase, 0,m_pcs);
          conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
        }
      /* Initialisieren */
      MNulleMat(gravity_matrix, 8, 8);
      MNulleVec(gravity_vector, 8);

      /* Schwerkraftterm berechnen (gravity vector) */
      for (l = 0; l < 8; l++)
//        elevation[l] = ((GetNode(element_nodes[l])) -> z) * GetFluidDensity(phase, index, 0., 0., 0., mmp_time_collocation_global) * g;
        elevation[l] = ((GetNode(element_nodes[l])) -> z) * density_gp * g;
      MMultMatVec(conductance_matrix, 8, 8, elevation, 8, gravity_vector, 8);
      MMPSetElementGravityVector(index, phase, gravity_vector);

#ifdef TESTCEL_MMP
      MZeigVec(gravity_vector, 8, "GravityVector");
#endif
      break;
    //--------------------------------------------------------------------
    case 3:
      capillarity_vector = MMPGetElementCapillarityVector(index,phase);
      if (!capillarity_vector)
        capillarity_vector = (double*)Malloc(8* sizeof(double));
      m_mmp->mode = 1;
      for(l=0;l<8;l++){
        //OK capillarity_vector[l] = MMPGetNodeDifferenceToReferencePressure(phase, index, element_nodes[l], mmp_time_collocation_global);
        capillarity_vector[l] = m_mmp->CapillaryPressureFunction(element_nodes[l],NULL,mmp_time_collocation_global,phase,0.0);
      }
      m_mmp->mode = 0;
      MMPSetElementCapillarityVector(index,phase,capillarity_vector);
#ifdef TESTCEL_MMP
      MZeigVec(capillarity_vector,8,"CapillarityVector");
#endif
      break;
    //--------------------------------------------------------------------
    case 4:             
      // DM -> MMP - Coupling 
      if(GetRFProcessProcessing("SD")&&phase==1) { // By WW

          coupling_matrix_u_x = MMPGetElementStrainCouplingMatrixX(index, phase);
          if (!coupling_matrix_u_x)
             coupling_matrix_u_x = (double *) Malloc(nnodes*nnodesHQ*sizeof(double));
          coupling_matrix_u_y = MMPGetElementStrainCouplingMatrixY(index, phase);
          if (!coupling_matrix_u_y)
             coupling_matrix_u_y = (double *) Malloc(nnodes*nnodesHQ*sizeof(double));
          coupling_matrix_u_z = MMPGetElementStrainCouplingMatrixZ(index, phase);
          if (!coupling_matrix_u_z)
             coupling_matrix_u_z = (double *) Malloc(nnodes*nnodesHQ*sizeof(double));
/*
          MNulleMat(coupling_matrix_u_x,nnodes,nnodesHQ);
          MNulleMat(coupling_matrix_u_y,nnodes,nnodesHQ);
          MNulleMat(coupling_matrix_u_z,nnodes,nnodesHQ);

          Init_Quadratic_Elements();
          elem_dm->ConfigElement(index, 2);
          elem_dm->ComputeStrainCouplingMatrix(0, coupling_matrix_u_x);
          elem_dm->ComputeStrainCouplingMatrix(1, coupling_matrix_u_y);
          elem_dm->ComputeStrainCouplingMatrix(2, coupling_matrix_u_z);
*/
          /////////////////////////////////////////////////////////
          // This section will be inserted to elem_dm->ComputeStrainCouplingMatrix
          gp[0] = 0.0;
          gp[1] = 0.0;
          gp[2] = 0.0;
          saturation_gp = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
          for(ii=0; ii<nnodes*nnodesHQ; ii++)
          {
             coupling_matrix_u_x[ii] *= saturation_gp;
             coupling_matrix_u_y[ii] *= saturation_gp;
             coupling_matrix_u_z[ii] *= saturation_gp;
          }
          /////////////////////////////////////////////////////////

          MMPSetElementStrainCouplingMatrixX(index, phase, coupling_matrix_u_x);
          MMPSetElementStrainCouplingMatrixY(index, phase, coupling_matrix_u_y); 
          MMPSetElementStrainCouplingMatrixZ(index, phase, coupling_matrix_u_z);            
          Init_Linear_Elements();               


      } 
        
      break;
    //--------------------------------------------------------------------
    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}



/*OK
void BuildElementMatrices_MMP(long index)
{
  int i;
  // Schleife ueber alle Matrizen/Phasen 
  for (i = 0; i < Number_of_Matrixtypes; i++)
    CalcElementMatrix_MMP(index, Array_of_Matrixtypes[i].phase, Array_of_Matrixtypes[i].type);
}
*/


/*************************************************************************
 ROCKFLOW - Funktion: MMPCalcElementMatrices2DTriangle

 Aufgabe:
   Calcluation of MMP finite element matrices
   for linear triangular elements

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   E int phase: Fluidphase, mit 0 beginnend
   E int matrixtyp: Typ der auzubauenden Matrix

 Ergebnis:
   - void -

 Programmaenderungen:
   05/2002   OK   Implementation based on CalcEle2D_MMP (CT)

**************************************************************************/
void MMPCalcElementMatrices2DTriangle(long index, int phase, int matrixtyp,CRFProcess*m_pcs)
{

  /* Geometry */
  static int nn=3;
  static int nn2=9;
  static double x[3],y[3],x_gp,y_gp;
  static double thickness,area,volume;
  static double fac_geo;
  static double gp[3];
  /* Materials */
  static double fac_mat;
  static double kmy_xx,kmy_yy;

  /* Matrices */
  static double *conductance_matrix;
  static double *capacitance_matrix;
  static double *gravity_vector;
  static double *capillarity_vector;
  static double gravity_matrix[64];
  static long *element_nodes;
  static double elevation[8];
  static double g;
  static double rho_n[4];
  static long i,l;
  static double r,s, r_upw, s_upw;
  static double grad_rho[64];

  static double kr, kr_max, k_xx, k_yy, rho, mue, n, S; // *perm;
  static double alpha[2], scale;        /* SUPG */
  static double v[2];
  double* permeability;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  double theta = m_pcs->m_num->ls_theta;

  /* --- Element-Daten ------------------------------------------------ */
   /* Element-Koordinaten im lokalen System (x',y')==(a,b) */
/*  Calc2DElementCoordinatesTriangle(index,x,y); */
  element_nodes = ElGetElementNodes(index);
  for (i=0;i<nn;i++) {
    x[i]=GetNodeX(element_nodes[i]);
    y[i]=GetNodeY(element_nodes[i]);
  }

/*
        if (index<9) {
          x_gp = (x[0]+x[1])/2.;
          y_gp = (y[1]+y[2])/2.;
        } 
        else {
          x_gp = (x[2]+x[0])/2.;
          y_gp = (y[0]+y[1])/2.;
        }
*/
          x_gp = (x[0]+x[1]+x[2])/3.;
          y_gp = (y[0]+y[1]+y[2])/3.;

  /* Element-Volumen: Flaeche * Dicke */    
  volume = fabs(ElGetElementVolume(index));
  thickness = m_mmp->geo_area;
  area = volume/thickness;
  n = m_mmp->Porosity(index,NULL,theta);
  permeability = m_mmp->PermeabilityTensor(index);
  S = m_mmp->StorageFunction(index,gp,mmp_time_collocation_global);
  g = gravity_constant;
  MNulleVec(alpha, 2);
  //----------------------------------------------------------------------
  // State functions
  char char_phase[1];
  sprintf(char_phase,"%i",phase+1);
  string nod_val_name = "SATURATION";
  nod_val_name += char_phase;
  double saturation_gp;
  /*  !!!! Wenn dieser Switch veraendert wird muss auch GetMatrixtype_MMP und DeleteElementMatrix_MMP angepasst werden !!! */
  switch (matrixtyp)
    {
    /* ================================================================================ */
    case 0:
      /* ------------------------------------------------------------------------------ */
      /* Durchlaessigkeitsmatrix Beginn */
        /* Speicher holen */
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if (!conductance_matrix)
        conductance_matrix = (double *) Malloc(nn2 * sizeof(double));

      /* ------------------------------------------------------------------------------ */
      /* Upwinding fuer Permeabilitaetsmatrizen */
      r_upw = s_upw = 0.;
      if ((mmp_upwind_method == 1) || (mmp_upwind_method == 2))
        {
          CalcVelo2DTriangleRS(phase,index,GetTimeCollocationupwind_MMP(),x_gp,y_gp, 0., v);
          if (MBtrgVec(v,2) > MKleinsteZahl)
            {
              /* Upwind-Faktoren */
              for (l = 0; l < 2; l++) {
                v[l] = -v[l]; /* OK Vorzeichen */
                alpha[l] = -mmp_upwind_parameter * v[l] / (MBtrgVec(v, 2) + MKleinsteZahl);
              }
            }
          if (mmp_upwind_method == 1)
            {
              /* Verschiebungen der Gausspunkte auf Element begrenzen */
              scale = 1.;
              if (fabs(alpha[0]) > 1.)
                scale = MMin(scale, 1. / fabs(alpha[0]));
              if (fabs(alpha[1]) > 1.)
                scale = MMin(scale, 1. / fabs(alpha[1]));
              r_upw = scale * alpha[0];
              s_upw = scale * alpha[1];
            }
          if (mmp_upwind_method == 2)
            {
              /* Verschiebungen auf -1<x<1 begrenzen */
              r_upw = MRange(-1., alpha[0], 1.);
              s_upw = MRange(-1., alpha[1], 1.);
            }
        }
      /* Maximum Mobility Upwinding */
      if (mmp_upwind_method == 3)
        {
          kr_max = 0.;
          for (i = 0; i < 4; i++)
            kr_max = MMax(kr_max, MMPGetNodeRelativePermeability(phase, index, element_nodes[i], GetTimeCollocationupwind_MMP()));
        }

      /* Initialisieren */
      MNulleMat(conductance_matrix,nn,nn);
      /* ------------------------------------------------------------------------------ */
      /* Materials */
//      k_xx = GetSoilPermeability(index, 0);
	  k_xx = permeability[0];
//      k_yy = GetSoilPermeability(index, 1);
	  k_yy = permeability[3];
      /* Alle Groessen nur in Elementmitte betrachten? Dann geht's so schneller. */
      /* kr = MMPGetRelativePermeability(phase, index, r_upw, s_upw, 0., mmp_time_collocation_global);*/
      if(fabs(r_upw)< MKleinsteZahl) {
        if (index<9) {
          x_gp = (x[0]+x[1])/2.;
          y_gp = (y[1]+y[2])/2.;
        } 
        else {
          x_gp = (x[2]+x[0])/2.;
          y_gp = (y[0]+y[1])/2.;
        }
      }
      else if (r_upw<1.0) {
        x_gp = x[1];
        y_gp = y[1];
      }
      kr = MMPGetRelativePermeability(phase, index,x_gp,y_gp,0.0, mmp_time_collocation_global);
      gp[0]=r_upw; gp[1]=s_upw; gp[2]=0.0;
      mue = mfp_vector[phase]->Viscosity();
      kmy_xx = (k_xx) / mue * kr;
      kmy_yy = (k_yy) / mue * kr;
      /* ------------------------------------------------------------------------------ */
      /* Fuer erstes Integral: grad(phi)*kr*k/mue*grad(ome) */
                            /* grad(phi) * tkmyt */
      fac_mat = 1.0;
       /* 3x3 matrix */
      conductance_matrix[0] = kmy_xx*(y[1]-y[2])*(y[1]-y[2]) + kmy_yy*(x[2]-x[1])*(x[2]-x[1]);
      conductance_matrix[1] = kmy_xx*(y[1]-y[2])*(y[2]-y[0]) + kmy_yy*(x[2]-x[1])*(x[0]-x[2]);
      conductance_matrix[2] = kmy_xx*(y[1]-y[2])*(y[0]-y[1]) + kmy_yy*(x[2]-x[1])*(x[1]-x[0]);
      conductance_matrix[3] = kmy_xx*(y[2]-y[0])*(y[1]-y[2]) + kmy_yy*(x[0]-x[2])*(x[2]-x[1]);
      conductance_matrix[4] = kmy_xx*(y[2]-y[0])*(y[2]-y[0]) + kmy_yy*(x[0]-x[2])*(x[0]-x[2]);
      conductance_matrix[5] = kmy_xx*(y[2]-y[0])*(y[0]-y[1]) + kmy_yy*(x[0]-x[2])*(x[1]-x[0]);
      conductance_matrix[6] = kmy_xx*(y[0]-y[1])*(y[1]-y[2]) + kmy_yy*(x[1]-x[0])*(x[2]-x[1]);
      conductance_matrix[7] = kmy_xx*(y[0]-y[1])*(y[2]-y[0]) + kmy_yy*(x[1]-x[0])*(x[0]-x[2]);
      conductance_matrix[8] = kmy_xx*(y[0]-y[1])*(y[0]-y[1]) + kmy_yy*(x[1]-x[0])*(x[1]-x[0]);
       /* Volumetric element */
      fac_geo = 1./(4.*area) * thickness ; /* Prüfen */

      for (i=0;i<nn2;i++) {
        conductance_matrix[i] *= fac_mat * fac_geo;
      }
#ifdef TESTCEL_MMP
      MZeigMat(conductance_matrix, nn, nn, "ConductivityMatrix 1");
#endif
      /* ------------------------------------------------------------------------------ */
      /* Fuer zweites Integral: -phi*grad(rho)*kr*k/mue*grad(ome) */
      /* Materials */
      for (l = 0; l < nn; l++)
		rho_n[l] = mfp_vector[phase]->Density();
      grad_rho[0] = rho_n[0]*(y[1]-y[2]) + rho_n[1]*(y[2]-y[0]) + rho_n[2]*(y[0]-y[1]);
      grad_rho[1] = rho_n[0]*(x[2]-x[1]) + rho_n[1]*(x[0]-x[2]) + rho_n[2]*(x[1]-x[0]);
      gp[0]=x_gp; gp[1]=y_gp;
	  rho = mfp_vector[phase]->Density();
      fac_mat = (grad_rho[0] * k_xx + grad_rho[1] * k_yy)/rho;
      fac_geo = thickness/(2.*area); /* einfacher */
      for (i=0;i<nn2;i++) {
        conductance_matrix[i] -= fac_mat * fac_geo;
      }
      MMPSetElementConductivityMatrix(index, phase, conductance_matrix);
#ifdef TESTCEL_MMP
      MZeigMat(conductance_matrix, nn, nn, "ConductivityMatrix 2");
#endif
      break;
    /* ================================================================================ */
    case 1:
      /* Kompressibilitaet und Speicherterme */
      capacitance_matrix = MMPGetElementCapacitanceMatrix(index, phase);
      if (!capacitance_matrix)
        capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));

      /* Materials */
      /* fkt = (1/rho*n*d_rho/d_p*S + Se*S + n*dS/dp)  Elementweise konstante Werte ausserhalb der Schleife */
      r=0.0; s=0.0;
	  gp[0]=x_gp; gp[1]=y_gp; gp[2]=0.0;
//SB:m      fac_mat = (n * GetFluidDensityPressureDependency(phase)
      saturation_gp = PCSGetELEValue(index,gp,mmp_time_collocation_global,nod_val_name);
	  fac_mat = (n * mfp_vector[phase]->drho_dp
               / mfp_vector[phase]->Density()
               * MMax(0.,saturation_gp)
               + S
               * MMax(0.,saturation_gp)
            );

      /* Richards'-Modell */
//SB:      if ((GetMethod_MMP()==7)||(GetMethod_MMS()==7))
      if ((GetMethod_MMP()==7))
        fac_mat += n * MMPGetSaturationPressureDependency(phase, index,x_gp,y_gp, 0., mmp_time_collocation_global);

      if (mmp_mass_lumping_method)
      {
        /* 3x3 matrix */
        capacitance_matrix[0] = 4.0; capacitance_matrix[1] = 0.0; capacitance_matrix[2] = 0.0;
        capacitance_matrix[3] = 0.0; capacitance_matrix[4] = 4.0; capacitance_matrix[5] = 0.0;
        capacitance_matrix[6] = 0.0; capacitance_matrix[7] = 0.0; capacitance_matrix[8] = 4.0;
      }
      else
      {
        /* 3x3 matrix */
        capacitance_matrix[0] = 2.0; capacitance_matrix[1] = 1.0; capacitance_matrix[2] = 1.0;
        capacitance_matrix[3] = 1.0; capacitance_matrix[4] = 2.0; capacitance_matrix[5] = 1.0;
        capacitance_matrix[6] = 1.0; capacitance_matrix[7] = 1.0; capacitance_matrix[8] = 2.0;
      }

       /* Volumetric element */
      fac_geo = area/(12.) * thickness; /*OK_TODO */

      for (i=0;i<nn2;i++) {
        capacitance_matrix[i] *= fac_mat * fac_geo; /*OK_TODO*/
      }

      MMPSetElementCapacitanceMatrix(index,phase,capacitance_matrix);

#ifdef TESTCEL_MMP
      MZeigMat(capacitance_matrix,nn,nn,"CapacitanceMatrix");
#endif

      break;

    /* ================================================================================ */
    case 2:
      /* Gravitationsvektor mit elementgemittelter Dichte ? */
      if (mmp_element_integration_method_density)
        {
          /* Nein, Gausspunkt-Integration ! */
          MMPCalcElementMatrices2DTriangle(index, phase, 0,m_pcs);
          break;
        }
      /* Gravitationsvektor mit elementgemittelter Dichte ! */
      gravity_vector = MMPGetElementGravityVector(index, phase);
      if (!gravity_vector)
        gravity_vector = (double *) Malloc(nn * sizeof(double));
      conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
      if (!conductance_matrix)
        {
          MMPCalcElementMatrices2DTriangle(index, phase, 0,m_pcs);
          conductance_matrix = MMPGetElementConductivityMatrix(index, phase);
        }
      /* Initialisieren */
      MNulleMat(gravity_matrix, nn, nn);
      MNulleVec(gravity_vector, nn);
      /* Schwerkraftterm berechnen (gravity vector) */
      for(l=0;l<nn;l++)
        elevation[l] = ((GetNode(element_nodes[l]))->z) * mfp_vector[phase]->Density() * g;
      MMultMatVec(conductance_matrix, nn, nn, elevation, nn, gravity_vector, nn);
      MMPSetElementGravityVector(index, phase, gravity_vector);

#ifdef TESTCEL_MMP
      MZeigVec(gravity_vector, nn, "GravityVector");
#endif

      break;

    /* ================================================================================ */
    case 3:
          capillarity_vector = MMPGetElementCapillarityVector(index, phase);
          if (!capillarity_vector)
            capillarity_vector = (double *) Malloc(nn * sizeof(double));

          for (l = 0; l < nn; l++)
            capillarity_vector[l] = MMPGetNodeDifferenceToReferencePressure(phase, index, element_nodes[l], mmp_time_collocation_global);

          MMPSetElementCapillarityVector(index, phase, capillarity_vector);

#ifdef TESTCEL_MMP
          MZeigVec(capillarity_vector, nn, "CapillarityVector");
#endif
      break;


    /* ================================================================================ */
    case 4: /* s. Vierecke */
      /* Dilatation coupling matrix */
      break;

    /* ================================================================================ */
    default:
      /* Fehlermeldung */
      DisplayMsgLn("cel_mmp.c: Unknown matrix-type requested!");
      break;
    }                                  /* endswitch */
}




/**************************************************************************
 ROCKFLOW - Funktion: MMPCalcCharacteristicNumbers

 Task: 

 Parameters: (I: Input; R: Return; X: Both)
   I long index:    element number
   I int phase:     fluid phase 
   I int comp:      component number

 Return:
   - void -

 Programming:
 06/2002   OK   Implementation
 04/4002   WW   Bug removing

**************************************************************************/
void MMPCalcCharacteristicNumbers(long index,int phase)
{
  static double dt,dx;
  static double *invjac, detjac;
  static double A, L, r, *permeability; // *perm
  static long *element_nodes;
  static double mmp_permeability_ref,mmp_permeability_rel,mmp_viscosity,mmp_porosity;
  static double mmp_dS_dp;
  static double neumann_number;
  double Gauss[]={0.0,0.0,0.0};
  extern VoidFuncLIID MPCSetElementConductivityNumber;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];

  double theta = 1.0; //OK m_pcs->m_num->ls_theta;

  /*--------------------------------------------------------------------------*/
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcCharacteristicNumbers: no time discretization data !" << endl;
  dt = m_tim->CalcTimeStep();
//OK_TIM  dt = GetDt(aktueller_zeitschritt-1l);
  invjac = GetElementJacobiMatrix(index, &detjac);
  element_nodes = ElGetElementNodes(index);
  A = m_mmp->geo_area;
  L = 2. * detjac / A;
  dx = L;
  r=0.0;
  /*--------------------------------------------------------------------------*/
  /* Materials */
  permeability = m_mmp->PermeabilityTensor(index);
  mmp_permeability_ref = permeability[0]; //SB:m
  Gauss[0]=r; Gauss[1]=0.0; Gauss[2]=0.0;
  mmp_permeability_rel = m_mmp->PermeabilitySaturationFunction(index,Gauss,theta,phase);
  mmp_viscosity=mfp_vector[phase]->Viscosity();   //??YD
  mmp_porosity = m_mmp->Porosity(index,NULL,theta);
  mmp_dS_dp = MMPGetSaturationPressureDependency(phase,index,0.,0.,0.,mmp_time_collocation_global);
  /*--------------------------------------------------------------------------*/
  /* Compressibility number */
  /*--------------------------------------------------------------------------*/
  /* Conductivity number */
  neumann_number = mmp_permeability_rel * mmp_permeability_ref \
                 / mmp_viscosity \
                 / mmp_porosity \
                 / mmp_dS_dp \
                 * dt / (dx*dx);
  MPCSetElementConductivityNumber(index,phase,0,neumann_number);
  /*--------------------------------------------------------------------------*/
  /* Diffusivity number */
}

/*** KERNEL MMP MMS *******************************************************/
/*------------------------------------------------------------------------*/
/* ElementCapacitanceMatrix */
void MMPSetElementCapacitanceMatrixPhase_MMP(long number, int phase, double *matrix)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_capacitance_matrix_phase[phase] = matrix;
}

double *MMPGetElementCapacitanceMatrixPhase_MMP(long number, int phase)
{
    return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_capacitance_matrix_phase[phase];
}

/*------------------------------------------------------------------------*/
/* ElementConductanceMatrix */
void MMPSetElementConductivityMatrixPhase_MMP(long number, int phase, double *matrix)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_conductivity_matrix_phase[phase] = matrix;
}

double *MMPGetElementConductivityMatrixPhase_MMP(long number, int phase)
{
  return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_conductivity_matrix_phase[phase];
  //return ((MMPElementMatrices *) ELEGetElementMatricesPCS(number,phase))->mmp_capacitance_matrix_phase[phase];
}


/*------------------------------------------------------------------------*/
/* ElementGravityVector */
void MMPSetElementGravityVectorPhase_MMP(long number, int phase, double *vec)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_gravity_vector_phase[phase] = vec;
}

double *MMPGetElementGravityVectorPhase_MMP(long number, int phase)
{
    return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_gravity_vector_phase[phase];
}

/*------------------------------------------------------------------------*/
/* ElementCapillarityVector */
double *MMPGetElementCapillarityVectorPhase_MMP(long number, int phase)
{
  return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_capillarity_matrix[phase];
}
void MMPSetElementCapillarityVectorPhase_MMP(long number, int phase, double *vec)
{
  ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_capillarity_matrix[phase] = vec;
}

/*------------------------------------------------------------------------*/
/* ElementStrainCouplingMatrix */
void MMPSetElementStrainCouplingMatrixX(long number, int phase, double *matrix)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_strain_coupling_matrix_x[phase] = matrix;
}

double *MMPGetElementStrainCouplingMatrixX(long number,int phase)
{
    return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_strain_coupling_matrix_x[phase];
}

void MMPSetElementStrainCouplingMatrixY(long number, int phase, double *matrix)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_strain_coupling_matrix_y[phase] = matrix;
}

double *MMPGetElementStrainCouplingMatrixY(long number, int phase)
{
    return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_strain_coupling_matrix_y[phase];
}

void MMPSetElementStrainCouplingMatrixZ(long number, int phase, double *matrix)
{
    static MMPElementMatrices *data;
    data = (MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number);
    data->mmp_strain_coupling_matrix_z[phase] = matrix;
}

double *MMPGetElementStrainCouplingMatrixZ(long number, int phase)
{
    return ((MMPElementMatrices *) ELEGetElementMatrices(number,mmp_pcs_number))->mmp_strain_coupling_matrix_z[phase];
}

/*************************************************************************
ROCKFLOW - Function: MMPCreateELEMatricesPointer
Task: Create element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *MMPCreateELEMatricesPointer(void)
{ 
  MMPElementMatrices *data = NULL;
  int i;
  int no_fluid_phases = 2;
  data = (MMPElementMatrices *) Malloc(sizeof(MMPElementMatrices));

  data->mmp_conductivity_matrix_phase = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_capacitance_matrix_phase = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_gravity_vector_phase = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_capillarity_matrix = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_capillarity_vector_phase = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_strain_coupling_matrix_x = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_strain_coupling_matrix_y = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  data->mmp_strain_coupling_matrix_z = \
       (double **) Malloc(sizeof(double *) * no_fluid_phases);
  for (i=0;i<no_fluid_phases;i++) {
    data->mmp_conductivity_matrix_phase[i] = NULL;
    data->mmp_capacitance_matrix_phase[i] = NULL;
    data->mmp_gravity_vector_phase[i] = NULL;
    data->mmp_capillarity_matrix[i] = NULL;
    data->mmp_capillarity_vector_phase[i] = NULL;
    data->mmp_strain_coupling_matrix_x[i] = NULL;
    data->mmp_strain_coupling_matrix_y[i] = NULL;
    data->mmp_strain_coupling_matrix_z[i] = NULL;
  }

  return (void *) data;
}

/*************************************************************************
ROCKFLOW - Function: MMPDestroyELEMatricesPointer
Task: Create element matrices pointer
Programming: 04/2003 OK Implementation
last modified:
**************************************************************************/
void *MMPDestroyELEMatricesPointer(void *data)
{
  int i;
  /*--------------------------------------------------------------------------*/
  /* Kernel MPC */
  for (i=0;i<GetRFProcessNumPhases();i++) {
    if (((MMPElementMatrices *) data)->mmp_conductivity_matrix_phase[i])
        ((MMPElementMatrices *) data)->mmp_conductivity_matrix_phase[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_conductivity_matrix_phase[i]);
    if (((MMPElementMatrices *) data)->mmp_capacitance_matrix_phase[i])
        ((MMPElementMatrices *) data)->mmp_capacitance_matrix_phase[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_capacitance_matrix_phase[i]);
    if (((MMPElementMatrices *) data)->mmp_gravity_vector_phase[i])
        ((MMPElementMatrices *) data)->mmp_gravity_vector_phase[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_gravity_vector_phase[i]);
    if (((MMPElementMatrices *) data)->mmp_capillarity_matrix[i])
        ((MMPElementMatrices *) data)->mmp_capillarity_matrix[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_capillarity_matrix[i]);
    if (((MMPElementMatrices *) data)->mmp_capillarity_vector_phase[i])
        ((MMPElementMatrices *) data)->mmp_capillarity_vector_phase[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_capillarity_vector_phase[i]);
    if (((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_x[i])
        ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_x[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_x[i]);
    if (((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_y[i])
        ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_y[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_y[i]);
    if (((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_z[i])
        ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_z[i] = \
         (double *) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_z[i]);
  }
  ((MMPElementMatrices *) data)->mmp_conductivity_matrix_phase = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_conductivity_matrix_phase);
  ((MMPElementMatrices *) data)->mmp_capacitance_matrix_phase = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_capacitance_matrix_phase);
  ((MMPElementMatrices *) data)->mmp_gravity_vector_phase = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_gravity_vector_phase);
  ((MMPElementMatrices *) data)->mmp_capillarity_matrix = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_capillarity_matrix);
  ((MMPElementMatrices *) data)->mmp_capillarity_vector_phase = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_capillarity_vector_phase);
  ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_x = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_x);
  ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_y = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_y);
  ((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_z = \
                     (double **) Free(((MMPElementMatrices *) data)->mmp_strain_coupling_matrix_z);

  data = (void*) Free(data);  /* sollte NULL sein */
  return data;
}


/*************************************************************************
ROCKFLOW - Function: MMPCalcElementMatrices
Task: Calc element matrices
Programming: 05/2003 OK Implementation
last modified:
**************************************************************************/
void MMPCalcElementMatrices(CRFProcess*m_pcs) 
{
  long i;
  int j,k;
  int Number_of_Matrixtypes=5;

  for (i=0;i<ElListSize();i++) {
    //BuildElementMatrices_MMP(i);
    for (j=0;j<GetRFProcessNumPhases();j++) {
      for (k=0;k<Number_of_Matrixtypes;k++) {
        CalcElementMatrix_MMP(i,j,k,m_pcs);
      }
    }
  }
}

/*************************************************************************
ROCKFLOW - Function: MMPCalcElementMatrices
Task: Calc element matrices
Programming: 05/2003 OK Implementation
last modified:
**************************************************************************/
void MMPCalcElementMatricesTypes(CRFProcess*m_pcs) 
{
  long i;
  int j,k;
  int Number_of_Matrixtypes=4; //OK 5
  for (j=0;j<GetRFProcessNumPhases();j++) {
    for (k=0;k<Number_of_Matrixtypes;k++) {
      for (i=0;i<ElListSize();i++) {
        CalcElementMatrix_MMP(i,j,k,m_pcs);
      }
    }
  }
}

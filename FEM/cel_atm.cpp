/**************************************************************************
   ROCKFLOW - Modul: cel_atm.c

   Aufgabe:
   Funktionen zur Berechnung der Elementmatrizen und anderer
   elementbezogener Daten fuer Modul aTM.

   Programmaenderungen:
   09/1994     MSR/hh        Erste Version
   05/1995     hh            2D-Berechnungen
   07/1995     hh            Umbau
   11/1995     msr           ueberarbeitet
   01/1996     cb            Peclet/Courant
   02/1996     cb            Transformationsmatrix 2D
   05/1996     cb            zweite Version
   08/1996     cb            Jakobi-Matrix im Zentrum
   06/1997     RK            Funktionen zur Berechnung der Inversen
                             Jakobi-Matrix und der Determinante der
                             Jakobi-Matrix entfernt (-> femlib.c)
   07/1997     RK            Korrekturen und Aenderungen aus dem aTM
                             uebertragen
   10/1997     OK            Implementierung des Waermetransportmodells
   06/1998     CT            Lagrangeverfahren fuer 2D
   10/1998     CT/OK         Korrektur 3D-Elementmatrizen
   10/1998     CT            Zeitliche Aenderung der Porositaet
   06/1999     CT            Globalvariable halbwertszeit_1 entfernt
   10/1999     CT            Zugriff auf Saettigung wiederhergestellt (!)
    2/2000     CT            Phasennummer eingefuehrt
    7/2000     CT            Aenderung 10/1998 wieder zurueckgenommen.
    7/2000     CT            Upwinding 2d/3d auf r/s/t umgestellt. Aus Gaussschleife entfernt
    8/2000     CT            Zugriff auf Elementpecletzahl
    9/2000     RK            Upwinding-Korrektur
    9/2000     CT            Kernel-Konfigurration, Mehrphasenbetrieb
   10/2000     CT            ML-FEM, total-upwinding<=>advection-only-upwinding
    7/2002     CT            Daempfung fuer Feldoszillationen
   03/2003     RK            Quellcode bereinigt, Globalvariablen entfernt 
   10/2003     MB            Numerische und analytische Lösung für Dreiecksprismen                                                                        

 **************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTSM
#define noTESTSUPG
#define noTESTCEL_ATM

/* noNUMERIC_ATM --> analytische Lösung für EMatrizen (für Dreiecksprismen) */
#define noNUMERIC_ATM

/* Header / Andere intern benutzte Module */
#include "cvel.h"
#include "cel_atm.h"
#include "int_atm.h"
#include "rfmodel.h"
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "femlib.h"
#include "material.h"
#include "numerics.h"
#include "tools.h"
#include "rf_tim_new.h"

/* Interne (statische) Deklarationen */
void CalcEle1D(int phase, int component, long index);
void CalcEle2D(int phase, int component, long index);
void CalcEle3D(int phase, int component, long index);
void CalcEle3DPrism_ATM_ana(int phase, int component, long index, double *, double *, double *);
void CalcEle3DPrism_ATM_num(int phase, int component, long index, double *, double *, double *);

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

static int atm_matrixrebuild_read = 0;
static int atm_number_rebuild_info;
static Rebuild_info *atm_rebuild_info;
static int atm_rebuild_array_size = 0;
static MATRIXREBUILD *atm_rebuild_array = NULL;


/* Vorkonfigurieren des Kernels */
static int atm_new_kernel = 0;
static int atm_method = 1;
static int atm_use_lagrange_for_2D = 0;
static int atm_transport_in_phase = -1;
static int atm_nonlinear_coupling = 1;

static int atm_timecollocation_array_size = 0;
static TIMECOLLOCATION *atm_timecollocation_array = NULL;
static double atm_time_collocation_global = 1.;
static double atm_time_collocation_cond_bc = 1.;
static double atm_time_collocation_source = 1.;
static double atm_time_collocation_upwinding = 0.;
static double atm_time_collocation_rebuild = 1.;

static int atm_upwind_method = 2;
static double atm_upwind_parameter = 1.;

static int atm_mass_lumping_method = 0;
static double atm_mass_lumping_parameter = 0.;

static int atm_predictor_method = 0;
static double atm_predictor_parameter = 0.;

static int atm_relaxation_method = 0;
static double atm_relaxation_parameter[16];

static int atm_artificial_diffusion = 0;
static double atm_artificial_diffusion_param[16];

static int atm_oscil_damp_method = 0;
static double atm_oscil_damp_parameter[16];

#ifdef EFFORT
int atm_effort_index = -1;
int atm_effort_total_index = -1;
#endif

/**************************************************************************
 ROCKFLOW - Funktion: InitializeKernel_ATM

 Aufgabe:
   Initialisiert den Kern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
      0
 Programmaenderungen:
   7.2000   C.Thorenz    Erste Version

**************************************************************************/
int InitializeKernel_ATM(void)
{
  int i, j, found;
  NUMERICALPARAMETER *atm_numerics = NumericsGetPointer("CONCENTRATION0");

  atm_new_kernel = 1;

  if (atm_numerics)
    {

      /* Numerische Methode */
      if (atm_numerics -> method_read)
        {
          atm_method = atm_numerics -> method;

          /* Lagrangeverfahren */
          if ((atm_numerics -> method == 3) || (atm_numerics -> method == 4) || (atm_numerics -> method == 5))
            atm_use_lagrange_for_2D = 1;

          /* Kuenstliche Diffusion */
          if ((atm_numerics -> method == 9) || (atm_numerics -> method == 11))
            {
              atm_artificial_diffusion = 1;
              atm_artificial_diffusion_param[0] = atm_numerics -> method_param[0];
              atm_artificial_diffusion_param[1] = atm_numerics -> method_param[1];
            }
        }

      /* Konfigurieren der Zeitwichtung */
      atm_timecollocation_array_size = NumericsGetTimecollocationArraySize("CONCENTRATION0");
      atm_timecollocation_array = NumericsGetPointerTimecollocationArray("CONCENTRATION0");

      for (i = 0; i < atm_timecollocation_array_size; i++)
        {
          /* Gibt es den Namenseintrag ? Ansonsten werden Defaults verwendet */
          if (!strcmp(atm_timecollocation_array[i].name, "GLOBAL"))
            atm_time_collocation_global = (double) atm_timecollocation_array[i].param[0];
          if (!strcmp(atm_timecollocation_array[i].name, "COND_BC"))
            atm_time_collocation_cond_bc = (double) atm_timecollocation_array[i].param[0];
          if (!strcmp(atm_timecollocation_array[i].name, "SOURCE"))
            atm_time_collocation_source = (double) atm_timecollocation_array[i].param[0];
          if (!strcmp(atm_timecollocation_array[i].name, "UPWINDING"))
            atm_time_collocation_upwinding = (double) atm_timecollocation_array[i].param[0];
          if (!strcmp(atm_timecollocation_array[i].name, "REBUILD"))
            atm_time_collocation_rebuild = (double) atm_timecollocation_array[i].param[0];
        }

      /* Masslumping */
      if (atm_numerics -> mass_lumping_method_read)
        {
          atm_mass_lumping_method = atm_numerics -> mass_lumping_method;
          atm_mass_lumping_parameter = atm_numerics -> mass_lumping_parameter;
        }

      /* Prediktor */
      if (atm_numerics -> predictor_method_read)
        {
          atm_predictor_method = atm_numerics -> predictor_method;
          atm_predictor_parameter = atm_numerics -> predictor_parameter;
        }

      /* Relaxation */
      if (atm_numerics -> relaxation_method_read)
        {
          atm_relaxation_method = atm_numerics -> relaxation_method;
          for (i = 0; i < 16; i++)
            atm_relaxation_parameter[i] = atm_numerics -> relaxation_parameter[i];
        }

      /* Transportphase */
      if (atm_numerics -> transport_in_phase_read)
        {
          atm_transport_in_phase = atm_numerics -> transport_in_phase;
        }

      /* Upwinding */
      if (atm_numerics -> upwind_method_read)
        {
          atm_upwind_method = atm_numerics -> upwind_method;
          atm_upwind_parameter = atm_numerics -> upwind_parameter;
        }

      /* Nichtlineare Kopplung */
      if (atm_numerics -> nonlinear_coupling_read)
        {
          atm_nonlinear_coupling = atm_numerics -> nonlinear_coupling;
        }

      /* Daempfung */
      if (atm_numerics -> oscil_damp_method_read)
        {
          atm_oscil_damp_method = atm_numerics -> oscil_damp_method;
          for (i = 0; i < 16; i++)
            atm_oscil_damp_parameter[i] = atm_numerics -> oscil_damp_parameter[i];
        }

      /* Konfigurieren des Matrixwiederaufbaus */
      if (atm_numerics -> matrixrebuild_read)
        {
          atm_matrixrebuild_read = 1;
          /* Konfigurieren des Matrixwiederaufbaus */
          atm_rebuild_array_size = NumericsGetMatrixRebuildArraySize("CONCENTRATION0");
          atm_rebuild_array = NumericsGetPointerMatrixRebuildArray("CONCENTRATION0");

          /* Umkopieren und Einsortieren der Rebuildinformationen */
          for (i = 0; i < atm_rebuild_array_size; i++)
            {
              found = 0;

              if (!strcmp("ELEMENT_SATURATION_CHANGE", atm_rebuild_array[i].name))
                {
                  found = 1;
                  /* Speicher holen */
                  atm_number_rebuild_info++;
                  atm_rebuild_info = (Rebuild_info *) Realloc(atm_rebuild_info, atm_number_rebuild_info * sizeof(Rebuild_info));
                  /* Name eintragen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].name = (char *) Malloc(((int)strlen("ELEMENT_SATURATION_CHANGE") + 1) * sizeof(char));
                  strcpy(atm_rebuild_info[atm_number_rebuild_info - 1].name, "ELEMENT_SATURATION_CHANGE");
                  /* Speicher fuer Referenzdaten holen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].number_reference_values = 1;
                  atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values = (int *) Malloc(GetRFProcessNumPhases() * sizeof(int));
                  for (j = 0; j < GetRFProcessNumPhases(); j++)
                    atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values[j] = ModelsAddElementValInfoStructure("Satu_ref", "-", 0, 0, 0, 0, 1.23456789e10);

                  /* Speicher fuer Aktualdaten holen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values = (int *) Malloc(GetRFProcessNumPhases() * sizeof(int));
                  for (j = 0; j < GetRFProcessNumPhases(); j++)
                    atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values[j] = ModelsAddElementValInfoStructure("Satu_cur", "-", 0, 0, 0, 0, 1.23456789e10);

                }                      /* endif */
              if (!strcmp("ELEMENT_VELOCITY_CHANGE", atm_rebuild_array[i].name))
                {
                  found = 1;
                  /* Speicher holen */
                  atm_number_rebuild_info++;
                  atm_rebuild_info = (Rebuild_info *) Realloc(atm_rebuild_info, atm_number_rebuild_info * sizeof(Rebuild_info));
                  /* Name eintragen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].name = (char *) Malloc(((int)strlen("ELEMENT_VELOCITY_CHANGE") + 1) * sizeof(char));
                  strcpy(atm_rebuild_info[atm_number_rebuild_info - 1].name, "ELEMENT_VELOCITY_CHANGE");

                  /* Speicher fuer Referenzdaten holen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].number_reference_values = 3 * GetRFProcessNumPhases();
                  atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values = (int *) Malloc(3 * GetRFProcessNumPhases() * sizeof(int));
                  for (j = 0; j < GetRFProcessNumPhases(); j++)
                    {
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values[j * 3 + 0] = ModelsAddElementValInfoStructure("v_x_ref", "m/s", 0, 0, 0, 0, 1.23456789e10);
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values[j * 3 + 1] = ModelsAddElementValInfoStructure("v_y_ref", "m/s", 0, 0, 0, 0, 1.23456789e10);
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_reference_values[j * 3 + 2] = ModelsAddElementValInfoStructure("v_z_ref", "m/s", 0, 0, 0, 0, 1.23456789e10);
                    }
                  /* Speicher fuer Aktualdaten holen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values = (int *) Malloc(3 * GetRFProcessNumPhases() * sizeof(int));
                  for (j = 0; j < GetRFProcessNumPhases(); j++)
                    {
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values[j * 3 + 0] = ModelsAddElementValInfoStructure("v_x_cur", "m/s", 0, 0, 0, 0, 1.23456789e10);
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values[j * 3 + 1] = ModelsAddElementValInfoStructure("v_y_cur", "m/s", 0, 0, 0, 0, 1.23456789e10);
                      atm_rebuild_info[atm_number_rebuild_info - 1].index_for_current_values[j * 3 + 2] = ModelsAddElementValInfoStructure("v_z_cur", "m/s", 0, 0, 0, 0, 1.23456789e10);
                    }
                }                      /* endif */
              if (found)
                {
                  /* Die Daten aus der Eingabe uebertragen */
                  atm_rebuild_info[atm_number_rebuild_info - 1].method = atm_rebuild_array[i].method;
                  atm_rebuild_info[atm_number_rebuild_info - 1].mode = atm_rebuild_array[i].mode;
                  atm_rebuild_info[atm_number_rebuild_info - 1].curve = atm_rebuild_array[i].curve;
                  atm_rebuild_info[atm_number_rebuild_info - 1].phase = atm_rebuild_array[i].phase;
                  atm_rebuild_info[atm_number_rebuild_info - 1].comp = atm_rebuild_array[i].comp;
                  atm_rebuild_info[atm_number_rebuild_info - 1].group = atm_rebuild_array[i].group;

                  for (j = 0; j < 16; j++)
                    atm_rebuild_info[atm_number_rebuild_info - 1].param[j] = atm_rebuild_array[i].param[j];
                }
              else
                {
                  DisplayMsg("Unknown identifier &");
                  DisplayMsg(atm_rebuild_array[i].name);
                  DisplayMsgLn(" found.");
                }
            }                          /* endfor */
        }
    }                                  /* endif */

#ifdef EFFORT
  atm_effort_index = ModelsAddElementValInfoStructure("Effort_ATM", "-", 1, 0, 0, 0, 0.);;
  atm_effort_total_index = ModelsAddElementValInfoStructure("Effort_Total_ATM", "-", 1, 0, 0, 0, 0.);;
#endif

  return 0;
}





/**************************************************************************
 ROCKFLOW - Funktion: GetXXX_ATM

 Aufgabe:
   Gibt div. Kernelparameter zurueck

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   double

 Programmaenderungen:
   22.04.99   C.Thorenz    Erste Version

**************************************************************************/
double GetTimeCollocationGlobal_ATM(void)
{
  return atm_time_collocation_global;
}
double GetTimeCollocationCondBC_ATM(void)
{
  return atm_time_collocation_cond_bc;
}
double GetTimeCollocationSource_ATM(void)
{
  return atm_time_collocation_source;
}
double GetTimeCollocationUpwinding_ATM(void)
{
  return atm_time_collocation_upwinding;
}
int GetArtificialDiffusion_ATM(void)
{
  return atm_artificial_diffusion;
}
double *GetArtificialDiffusionParam_ATM(void)
{
  return atm_artificial_diffusion_param;
}
int GetOscillationDampingMethod_ATM(void)
{
  return atm_oscil_damp_method;
}
double *GetOscillationDampingParam_ATM(void)
{
  return atm_oscil_damp_parameter;
}
int GetMethod_ATM(void)
{
  return atm_method;
}
int GetUseLagrange_ATM(void)
{
  return atm_use_lagrange_for_2D;
}
int GetTransportInPhase_ATM(void)
{
  return atm_transport_in_phase;
}
int GetNonLinearCoupling_ATM(void)
{
  return atm_nonlinear_coupling;
}
int GetPredictorMethod_ATM(void)
{
  return atm_predictor_method;
}
double GetPredictorParam_ATM(void)
{
  return atm_predictor_parameter;
}
int GetRelaxationMethod_ATM(void)
{
  return atm_relaxation_method;
}
double *GetRelaxationParam_ATM(void)
{
  return atm_relaxation_parameter;
}



/**************************************************************************
 ROCKFLOW - Funktion: ConditionalRebuildMatrices_ATM

 Aufgabe:
   Matrizen zur Berechnung kennzeichnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementindex

 Ergebnis:
   - void -

 Programmaenderungen:
    9/2000    C.Thorenz     Erste Version

**************************************************************************/
void ConditionalRebuildMatrices_ATM(int phase, int comp)
{
  long i, element, number_rebuild = 0;
  int *element_rebuild, e_index, e_index_ref, e_index_cur;
  double v[3], v_alt[3], error, s, s_ref, val;
  Rebuild_info *rebuild_info;
  int num_rebuild_info = 0, found_velo = 0, found_satu = 0;

  /* Feld fuer das Markieren der Elemente */
  element_rebuild = (int *) Malloc(ElListSize() * sizeof(int));

  /* Eintraege setzen */
  for (i = 0; i < ElListSize(); i++)
    element_rebuild[i] = 0;            /* 0: Keine Info vorhanden */

  /* Die aktuellen Werte bestimmen und abspeichern */
  for (num_rebuild_info = 0; num_rebuild_info < atm_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &atm_rebuild_info[num_rebuild_info];

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
                      CalcVeloXDrst(phase, element, atm_time_collocation_rebuild, 0., 0., 0., v);

                      ElSetElementVal(ActiveElements[i], e_index, v[0]);
                      ElSetElementVal(ActiveElements[i], e_index + 1, v[1]);
                      ElSetElementVal(ActiveElements[i], e_index + 2, v[2]);
                    }
                }                      /* endif */
            }

          if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
            {
              if ((rebuild_info -> phase == -1) || (rebuild_info -> phase == phase))
                {
                  e_index = rebuild_info -> index_for_current_values[phase];
                  for (i = 0; i < anz_active_elements; i++)
                    {
                      val = ATMGetFluidContent(phase, ActiveElements[i], 0., 0., 0., atm_time_collocation_rebuild);
                      ElSetElementVal(ActiveElements[i], e_index, val);
                    }
                }
            }
        }
    }                                  /* endfor */



  /* Die gespeicherten Referenzwerte mit Ist-Werten und Limits vergleichen */
  for (num_rebuild_info = 0; num_rebuild_info < atm_number_rebuild_info; num_rebuild_info++)
    {

      /* Zeiger holen */
      rebuild_info = &atm_rebuild_info[num_rebuild_info];

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

      if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
        {
          if ((rebuild_info -> phase == -1) || (phase == rebuild_info -> phase))
            {
              if ((rebuild_info -> comp == -1) || (comp == rebuild_info -> comp))
                {
                  found_satu = 1;
                  e_index_ref = rebuild_info -> index_for_reference_values[phase];
                  e_index_cur = rebuild_info -> index_for_current_values[phase];

                  for (i = 0; i < anz_active_elements; i++)
                    {
                      element = ActiveElements[i];

                      s_ref = ElGetElementVal(element, e_index_ref);
                      s = ElGetElementVal(element, e_index_cur);

                      switch (rebuild_info -> method)
                        {
                        case 1:       /* Differenzen */
                          error = fabs(s - s_ref);
                          if (error > rebuild_info -> param[0])
                            /* Neuaufbau */
                            element_rebuild[element] = 1;
                          break;

                        case 2:       /* Rel. Differenzen */
                          error = fabs((s - s_ref) / (s + s_ref + MKleinsteZahl) * 2.);
                          if (error > rebuild_info -> param[0])
                            /* Neuaufbau */
                            element_rebuild[element] = 1;
                          break;


                          /* Mit Zufallsaufbau! */
                        case 11:      /* Differenzen */
                          error = fabs(s - s_ref);
                          if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
                            /* Neuaufbau */
                            element_rebuild[element] = 1;
                          break;

                        case 12:      /* Rel. Differenzen  */
                          error = fabs((s - s_ref) / (s + s_ref + MKleinsteZahl) * 2.);
                          if (error / rebuild_info -> param[0] > pow((double) rand() / (double) RAND_MAX, rebuild_info -> param[1]))
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
  for (num_rebuild_info = 0; num_rebuild_info < atm_number_rebuild_info; num_rebuild_info++)
    {
      /* Zeiger holen */
      rebuild_info = &atm_rebuild_info[num_rebuild_info];
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

          if (!strcmp("ELEMENT_SATURATION_CHANGE", rebuild_info -> name))
            {
              e_index_ref = rebuild_info -> index_for_reference_values[phase];
              e_index_cur = rebuild_info -> index_for_current_values[phase];
              for (i = 0; i < anz_active_elements; i++)
                {
                  element = ActiveElements[i];
                  /* Referenzdaten abspeichern */
                  if (element_rebuild[element])
                    {
                      ElSetElementVal(element, e_index_ref, ElGetElementVal(element, e_index_cur));
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
          || (atm_matrixrebuild_read == 0))
        {
          number_rebuild++;
          CECalcGaussVelo(phase, element, GetTimeCollocationGlobal_ATM());
          CECalcPeclet_ATM_NEW(phase, 0, element);
          CECalcRetard_ATM(element);
          CECalcEleMatrix_ATM_NEW(phase, 0, element);

#ifdef EFFORT
          ElSetElementVal(element, atm_effort_index, ElGetElementVal(element, atm_effort_index) + 1.);
          ElSetElementVal(element, atm_effort_total_index, ElGetElementVal(element, atm_effort_total_index) + 1.);
#endif
        }
    }

#ifdef EFFORT
  printf("\nTransport: %f \n", (double) number_rebuild / (double) anz_active_elements);
  output = fopen("effort.atm", "a");
  fprintf(output, "\n %f %ld  %ld ", aktuelle_zeit, number_rebuild, anz_active_elements);
  fclose(output);
#endif


  element_rebuild = (int *) Free(element_rebuild);

}


/**************************************************************************
 ROCKFLOW - Funktion: BuildMatrices_ATM

 Aufgabe:
   
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int phase, int comp

 Ergebnis:
   - void -

 Programmaenderungen:
    7/2003    M.Beinhorn     Erste Version

**************************************************************************/
void BuildMatrices_ATM(int phase, int comp)
{
  long i, element;
  comp=comp; /*TODO MB*/ 

  for (i = 0; i < anz_active_elements; i++)  {
    element = ActiveElements[i];
    CECalcRetard_ATM(element);
    CECalcEleMatrix_ATM_NEW(phase, 0, element);
  }
}


/**************************************************************************
 ROCKFLOW - Funktion: StatMat_ATM

 Aufgabe:
   Stationaere Matrizenanteile berechnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementindex

 Ergebnis:
   - void -

 Programmaenderungen:
   08/1996     cb         Erste Version
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
   femlib.c  (-> CalcElementJacobiMatrix)

   letzte Aenderung   R.Kaiser  27.06.1997
**************************************************************************/
void StatMat_ATM(long index)
{
  CECalcRetard_ATM(index);
}


/**************************************************************************
 ROCKFLOW - Funktion: MakeMat_ATM

 Aufgabe:
   Instationaere Matrizenanteile berechnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementindex

 Ergebnis:
   - void -

 Programmaenderungen:
   01/1996     cb         Erste Version

**************************************************************************/
void MakeMat_ATM(long index)
{
  CECalcGaussVelo(0, index, GetTimeCollocationGlobal_ATM());
  CECalcPeclet_ATM(index);
  CECalcEleMatrix_ATM(index);
}


/**************************************************************************
   ROCKFLOW - Funktion: CECalcRetard_ATM

   Aufgabe:
   Berechnet den Retardationsfaktor des angegebenen Elements (fuer
   1D, 2D und 3D identisch)

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Index des Elements, dessen Retardationsfaktor berechnet
                  werden soll

   Ergebnis:
   - void -

   Programmaenderungen:

 **************************************************************************/
void CECalcRetard_ATM(long index)
{
  static double porosity, density_rock, partitioning_coefficient;


  porosity = GetSoilPorosity(index);
  density_rock = GetSolidDensity(index);
//SB:GS4  partitioning_coefficient = GetTracerPartitioningCoefficient(index, 0, 0);     /* CT3248 */
partitioning_coefficient = 0.0; //SB:GS4
  ATMSetElementRetardFac(index, (1. + (1. - porosity) * density_rock * \
                                 partitioning_coefficient / porosity));
}


/**************************************************************************
   ROCKFLOW - Funktion: CECalcPeclet_ATM_NEW

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
void CECalcPeclet_ATM(long index)
{
  CECalcPeclet_ATM_NEW(0, 0, index);
}

void CECalcPeclet_ATM_NEW(int phase, int component, long index)
{
  static int et, anz, j;
  static long k, anzgp;
  static double *velovec, *invjac;
  static double vs[3], dx[3], dr[3], zwi[9];
  static double v, ds, d, el_dt, el_pec, detjac, a, b, c, fkt, art_diff;
  static double area, porosity;
  static double molecular_diffusion, mass_dispersion_longitudinal, mass_dispersion_transverse;


  area = GetElementExchangeArea(index);

  /* Gehalt der Phase holen, auf der das Transportmodell arbeitet */
  if (ATMGetFluidContent)
    porosity = ATMGetFluidContent(phase, index, 0., 0., 0., atm_time_collocation_global);
  else
    porosity = GetSoilPorosity(index);

  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);
  mass_dispersion_transverse = GetSoilMassDispersionT(index);

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
/*        ds = ATMGetElementVolume(index) / area; OK rf3230 */
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
      M2InvertiereUndTransponiere(zwi);  /* MK_bitte_pruefen!!!: Transponierte der */ /* Jakobi-Matrix */
      MMultMatVec(zwi, 2, 2, dr, 2, dx, 2);
      ds = MBtrgVec(dx, 2);
      /* printf("dx=%e %e ds=%e\n",dx[0],dx[1],ds); */
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
      /* printf("dx=%e %e %e ds=%e\n",dx[0],dx[1],dx[2],ds); */
      break;
    }
  /* Diffusion */
  d = (molecular_diffusion + art_diff) + v * max(mass_dispersion_longitudinal, mass_dispersion_transverse);

  /* Element-Peclet-Zahl */
  if (d < MKleinsteZahl)
    d = MKleinsteZahl;
  el_pec = v * ds / d;


  /* zugehoeriger Element-Zeitschritt */

#ifdef TESTCEL_ATM
  DisplayMsg("Dimension=");
  DisplayLong(et);
  DisplayMsg(" Peclet-Zahl=");
  DisplayDouble(el_pec, 0, 0);
#endif

  if (el_pec < 2.0)
    {
      el_dt = ds * ds * 0.5 / d;       /* Neumman */

#ifdef TESTCEL_ATM
      DisplayMsg("dt-Neumann=");
#endif
    }
  else
    {
      el_dt = ds / max(v, MKleinsteZahl);       /* Courant */

#ifdef TESTCEL_ATM
      DisplayMsg("dt-Courant=");
#endif
    }

#ifdef TESTCEL_ATM
  DisplayDouble(el_dt, 0, 0);
  DisplayMsgLn("");
#endif

  if (atm_new_kernel)
    {
      ATMSetElementPecletNumNew(index, phase, el_pec);
      ATMSetElementDtNew(index, phase, el_dt);
    }
  else
    {
      ATMSetElementPecletNum(index, el_pec);
      ATMSetElementDt(index, el_dt);
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: CECalcEleMatrix_ATM

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
void CECalcEleMatrix_ATM(long number)
{
  /* Alte Version nur fuer Modell 10099 */
  atm_upwind_parameter = GetNumericalUpwindParameter("TRANSPORT");
  atm_time_collocation_global = GetNumericalTimeCollocation("TRANSPORT");
  CECalcEleMatrix_ATM_NEW(0, 0, number);
}

void CECalcEleMatrix_ATM_NEW(int phase, int component, long index)
{
  switch (ElGetElementType(index))
    {
    case 1:
      CalcEle1D(phase, component, index);
      break;
    case 2:
      CalcEle2D(phase, component, index);
      break;
    case 3:
      CalcEle3D(phase, component, index);
      break;
    case 6:
      #ifdef NUMERIC_ATM /* Numerische Lösung */
      CalcEle3DPrism_ATM_num(phase, component, index,NULL,NULL,NULL);
      #else              /* Analytische Lösung */
      CalcEle3DPrism_ATM_ana(phase, component, index,NULL,NULL,NULL);
      #endif
      break;
    default:
      DisplayMsg("Kernel ATM: Unknown element type!");
      abort();
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle1D

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

**************************************************************************/
void CalcEle1D(int phase, int component, long index)
{
  /* Datentypen der verwendeten Variablen */
  /* Modelldaten */

  static double retard;
  /* Materialdaten */
  static double porosity;
  static double molecular_diffusion, mass_dispersion_longitudinal;
  static double *dispersion_tensor;
  /* Elementgeometriedaten */
  static double *invjac, detjac;
  /* Elementdaten */
  static double d, tdt, v, vt, q, qt;
  static double *velovec;
  static double vorfk;
  static double zeta, pec, alpha[3], alpha_adv[3];      /* Upwind-Parameter */
  /* Elementmatrizen */
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;

#ifdef TESTSUPG
  DisplayMsg("1D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
  if (atm_new_kernel)
    {
      /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = ATMGetElementMassMatrixNew(index, phase);
      if (!mass)
        mass = (double *) Malloc(4 * sizeof(double));

      disp = ATMGetElementDispMatrixNew(index, phase);
      if (!disp)
        disp = (double *) Malloc(4 * sizeof(double));

      adv = ATMGetElementAdvMatrixNew(index, phase);
      if (!adv)
        adv = (double *) Malloc(4 * sizeof(double));

      content = ATMGetElementFluidContentMatrixNew(index, phase);
      if (!content)
        content = (double *) Malloc(4 * sizeof(double));

    }
  else
    {
      /* Alte Methode ATM, Speicher der alten Matrix wird beim
         Setzen der neuen Matrix freigegeben -> langsamer */
      mass = (double *) Malloc(4 * sizeof(double));
      disp = (double *) Malloc(4 * sizeof(double));
      adv = (double *) Malloc(4 * sizeof(double));
    }

  /* Initialisieren */
  MNulleMat(mass, 2, 2);
  MNulleMat(disp, 2, 2);
  MNulleMat(adv, 2, 2);
  if (ATMSetElementFluidContentMatrixNew)
    MNulleMat(content, 2, 2);


  /* Elementdaten bereitstellen */
  /* cb: CalcJac1D(index,invjac,&detjac); */
  invjac = GetElementJacobiMatrix(index, &detjac);      /* detjac= AL/2 */

  /* Fuer teilgesaettigte Verhaeltnisse: */
  if (ATMGetFluidContent)
    porosity = ATMGetFluidContent(phase, index, 0., 0., 0., atm_time_collocation_global);
  else
    porosity = GetSoilPorosity(index);

  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);

  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);

  retard = ATMGetElementRetardFac(index);
  if (atm_new_kernel)
    pec = ATMGetElementPecletNumNew(index, phase);
  else
    pec = ATMGetElementPecletNum(index);

  q = MBtrgVec(velovec, 3);
  v = q / porosity;
  qt = MSkalarprodukt(velovec, invjac, 3);

  vt = qt / porosity;

  /* Materialparameter berechnen */
  /* hydrodynamischer Dispersionsterm */
  dispersion_tensor = GetMassDispersionTensor(index, phase, component, &v);
  tdt = dispersion_tensor[0] * MSkalarprodukt(invjac, invjac, 3);
  d = dispersion_tensor[0];
  dispersion_tensor = (double *) Free(dispersion_tensor);       /* Effizienz OK */

#ifdef TESTTAYLOR
  /* Taylor-Galerkin nach Donea */
  printf("TESTTAYLOR, dt=%e\n", dt);
  atm_upwind_parameter = 0.0;          /* kein SU/PG */
  ElSetArtDiff(index, (-dt / 6.0 * vt * vt * porosity * (detjac * 0.5)));
#endif


/************************************************************************/
  /* SUPG                                                                 */
/************************************************************************/

  /* alpha initialisieren */
  alpha[0] = 0.0;
  alpha_adv[0] = 0.0;
  if (atm_upwind_method && (v > MKleinsteZahl) && (atm_upwind_parameter > 0.0))
    {
      /* Berechnung von zeta */
      zeta = vt / fabs(vt);            /* reine Advektion */
      if (d > MKleinsteZahl)
        {
          /* halbe Peclet-Zahl mit ds=2.0 */
          pec = vt / tdt;
          zeta = 0.0;                  /* reine Diffusion */
          if (fabs(pec) > 1.0)
            zeta = vt / fabs(vt) * (fabs(pec) - 1.0) / fabs(pec);
        }
      /* Upwind-Faktor */
      if (atm_upwind_method == 1)
        {
          /* Fully upwinding */
          alpha[0] = atm_upwind_parameter * zeta;
          alpha_adv[0] = atm_upwind_parameter * zeta;
        }
      if (atm_upwind_method == 2)
        {
          /* Advection upwinding */
          alpha_adv[0] = atm_upwind_parameter * zeta;
        }

#ifdef TESTSUPG
      DisplayDoubleVector(alpha, 1, "alpha=");
#endif
    }
/************************************************************************/
  /* Elementmatrizen berechnen                                            */
/************************************************************************/


  if (atm_mass_lumping_method == 0)
    {
      /* Massenmatrix mit SUPG ohne Zeitanteile */
      vorfk = retard * detjac * Mdrittel * porosity;
      mass[0] = (2.0 - 1.5 * alpha[0]) * vorfk;
      mass[1] = (1.0 - 1.5 * alpha[0]) * vorfk;
      mass[2] = (1.0 + 1.5 * alpha[0]) * vorfk;
      mass[3] = (2.0 + 1.5 * alpha[0]) * vorfk;
    }
  else if (atm_mass_lumping_method == 1)
    {
      mass[0] = mass[3] = 0.5 * ElGetElementVolume(index) * porosity * retard;
    }

#ifdef TESTCEL_ATM
  MZeigMat(mass, 2, 2, "CalcEle1D_ATM: mass capacitance matrix");
#endif

  /* Dispersionsmatrix ohne Zeitanteile */
  vorfk = porosity * (detjac * 0.5) * tdt;
  disp[0] = disp[3] = vorfk;
  disp[1] = disp[2] = -vorfk;

#ifdef TESTCEL_ATM
  MZeigMat(disp, 2, 2, "CalcEle1D_ATM: mass diffusion-dispersion matrix");
#endif

  /* Advektive Matrix mit SUPG ohne Zeitanteile */
  vorfk = porosity * detjac * 0.5 * vt;
  adv[0] = vorfk * (-1.0 + alpha_adv[0]);
  adv[1] = vorfk * (1.0 - alpha_adv[0]);
  adv[2] = vorfk * (-1.0 - alpha_adv[0]);
  adv[3] = vorfk * (1.0 + alpha_adv[0]);

#ifdef TESTCEL_ATM
  MZeigMat(adv, 2, 2, "CalcEle1D_ATM: heat advection matrix");
#endif


/* Int(phi*(dn_dt*c))dV */
/* vorfk = 1/6 * A * L * dn_dt;  das 1/dt wird in der cgs_atm.c beruecksichtigt */
/* detjac= AL/2 */
/* dn_dt = (ATMGetFluidContent(phase,index,0.,0.,0.,1.) - ATMGetFluidContent(phase,index,0.,0.,0.,0.)) / dt */

  if (ATMSetElementFluidContentMatrixNew)
    {
      vorfk = retard * detjac * Mdrittel * (ATMGetFluidContent(phase, index, 0., 0., 0., 1.) - ATMGetFluidContent(phase, index, 0., 0., 0., 0.));
      content[0] = (2.0 - 1.5 * alpha[0]) * vorfk;
      content[1] = (1.0 - 1.5 * alpha[0]) * vorfk;
      content[2] = (1.0 + 1.5 * alpha[0]) * vorfk;
      content[3] = (2.0 + 1.5 * alpha[0]) * vorfk;
    }
  /* Einspeichern der ermittelten Matrizen */
  if (atm_new_kernel)
    {
      ATMSetElementMassMatrixNew(index, phase, mass);
      ATMSetElementDispMatrixNew(index, phase, disp);
      ATMSetElementAdvMatrixNew(index, phase, adv);
      if (ATMSetElementFluidContentMatrixNew)
        ATMSetElementFluidContentMatrixNew(index, phase, content);
    }
  else
    {
      ATMSetElementMassMatrix(index, mass);
      ATMSetElementDispMatrix(index, disp);
      ATMSetElementAdvMatrix(index, adv);
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle2D

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

**************************************************************************/
void CalcEle2D(int phase, int component, long index)
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
  static double d[4], tdt[4];
  static double dreh[4];
  /* Elementmatrizen */
  static double *mass;
  static double *content;
  static double *disp;
  static double *adv;
  static double zwi[16];
  /* Elementdaten */
  static int anzgp;
  static double art_diff;       /* ETG */
  static double alpha[3], alpha_adv[3];         /* SUPG */
  static double pec;
  static double *velovec, vg, v[2], vt[2], v_rs[2];
  static double fkt, sum;
  static double retard;
  /* Materialdaten */
  static double area, porosity;
  static double mass_dispersion_longitudinal, mass_dispersion_transverse, molecular_diffusion;

  static double dC;

#ifdef TESTSUPG
  DisplayMsg("2D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
  if (atm_new_kernel)
    {
      /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = ATMGetElementMassMatrixNew(index, phase);
      if (!mass)
        mass = (double *) Malloc(16 * sizeof(double));

      disp = ATMGetElementDispMatrixNew(index, phase);
      if (!disp)
        disp = (double *) Malloc(16 * sizeof(double));

      adv = ATMGetElementAdvMatrixNew(index, phase);
      if (!adv)
        adv = (double *) Malloc(16 * sizeof(double));

      content = ATMGetElementFluidContentMatrixNew(index, phase);
      if (!content)
        content = (double *) Malloc(16 * sizeof(double));
    }
  else
    {
      /* Alte Methode ATM, Speicher der alten Matrix wird beim
         Setzen der neuen Matrix freigegeben -> langsamer */
      mass = (double *) Malloc(16 * sizeof(double));
      disp = (double *) Malloc(16 * sizeof(double));
      adv = (double *) Malloc(16 * sizeof(double));
    }

  /* Initialisieren */
  MNulleMat(mass, 4, 4);
  MNulleMat(disp, 4, 4);
  MNulleMat(adv, 4, 4);
  if (ATMSetElementFluidContentMatrixNew)
    MNulleMat(content, 4, 4);

  /* Elementdaten und globale Modellparameter bereitstellen */
  area = GetElementExchangeArea(index);

  /* Fuer teilgesaettigte Verhaeltnisse: */
  if (ATMGetFluidContent)
    porosity = ATMGetFluidContent(phase, index, 0., 0., 0., atm_time_collocation_global);
  else
    porosity = GetSoilPorosity(index);

  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);
  mass_dispersion_transverse = GetSoilMassDispersionT(index);

  retard = ATMGetElementRetardFac(index);       /* Retardierungsfaktor */
  if (atm_new_kernel)
    pec = ATMGetElementPecletNumNew(index, phase);
  else
    pec = ATMGetElementPecletNum(index);

  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);

  /* SUPG */
  MNulleVec(alpha, 2);
  MNulleVec(alpha_adv, 2);

  if (atm_upwind_method && (atm_upwind_parameter > 0.0))
    {
      CalcVeloXDrst(phase, index, atm_time_collocation_upwinding, 0., 0., 0., v);

      /* Geschwindigkeitstransformation: a,b -> r,s */
      Calc2DElementJacobiMatrix(index, 0., 0., invjac, &detjac);
      MKopierVec(invjac, jacobi, 4);
      M2InvertiereUndTransponiere(jacobi); /* MK_bitte_pruefen!!!: Transponierte der */ /* Jacobi-Matrix */
      MMultMatVec(jacobi, 2, 2, v, 2, v_rs, 2);

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
                alpha[k] = alpha_adv[k] = v[k] / vg * atm_upwind_parameter * (fabs(pec) - 1.0) / fabs(pec);

#ifdef TESTSUPG
              DisplayDoubleVector(alpha, 2, "alpha");
#endif

              if (atm_upwind_method == 2)
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

        /* Allgemeines */
        /* J^-1 */
        Calc2DElementJacobiMatrix(index, r, s, invjac, &detjac);
        /* Faktor */
        fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * detjac;

        /* Veraenderliche Porositaet? */
        if (ATMGetFluidContent)
          porosity = ATMGetFluidContent(phase, index, r, s, 0., atm_time_collocation_global);

        sum = fkt * porosity;

        /* Abstandsgeschwindigkeit im Gauss-Punkt */
        for (k = 0; k < 2; k++)
          v[k] = velovec[++ind] / porosity;
        vg = MBtrgVec(v, 2);

        /* hydrodynamischen Dispersionstensor berechnen */
        MNulleMat(d, 2, 2);
        /* Stofftransport */
        art_diff = ElGetArtDiff(index);
        d[0] = (molecular_diffusion + art_diff) + mass_dispersion_longitudinal * vg;
        d[3] = (molecular_diffusion + art_diff) + mass_dispersion_transverse * vg;
        /* Drehen in r,s-Richtung ???
           OK offensichtlich Drehen des Tensors von stromlinienorietierten
           in lokale physikalische (Element) Koordinaten
           wird benoetigt, wenn Dispersionstensor in Hauptachsen angegeben ist */
        if ((vg > MKleinsteZahl) && (d[0] > MKleinsteZahl || d[3] > MKleinsteZahl))
          {
            dreh[0] = dreh[3] = v[0] / vg;
            dreh[1] = v[1] / vg;
            dreh[2] = -dreh[1];
            /* dreh^T x D x dreh */
            MMultMatMat(d, 2, 2, dreh, 2, 2, zwa, 2, 2);
            MTranspoMat(dreh, 2, 2, zwi);
            MMultMatMat(zwi, 2, 2, zwa, 2, 2, d, 2, 2);
          }
        /* tdt = J^-1 * D * (J^-1)T */
        MMultMatMat(invjac, 2, 2, d, 2, 2, zwa, 2, 2);
        MTranspoMat(invjac, 2, 2, zwi);
        MMultMatMat(zwa, 2, 2, zwi, 2, 2, tdt, 2, 2);
        /* vt = v * (J^-1)T */
        MMultVecMat(v, 2, zwi, 2, 2, vt, 2);

/********************************************************************/
/* Massenmatrix                                                     */
/********************************************************************/
        /* Wichtungsfunktion: phi */
        /* Fallunterscheidung fuer Standard-Galerkin oder SUPG-Verfahren */
        if (atm_upwind_method == 1)
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

        if (atm_mass_lumping_method == 0)
          {
            /* Normale FEM */
            /* phi * omega */
            MMultVecVec(phi, 4, zwo, 4, zwi, 4, 4);

            /* Stofftransport - sorption matrix */
            for (k = 0; k < 16; k++)
              mass[k] += (zwi[k] * sum);
          }
        else if (atm_mass_lumping_method == 1)
          {
            /* Mass-Lumping */
            for (k = 0; k < 4; k++)
              mass[k * 5] += zwo[k] * sum;
          }
/********************************************************************/
/*  Fluidgehaltsmatrix                                              */
/********************************************************************/
        if (ATMSetElementFluidContentMatrixNew)
          {
            dC = (ATMGetFluidContent(phase, index, r, s, 0., 1.) - ATMGetFluidContent(phase, index, r, s, 0., 0.));
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
        if (atm_upwind_method == 2)
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
      }                                /* Ende der Schleife ueber GaussPunkte */

  /* Stofftransport */
  for (i = 0; i < 16; i++)
    {
      mass[i] *= area * retard;
      if (ATMSetElementFluidContentMatrixNew)
        content[i] *= area * retard;
      disp[i] *= area;
      adv[i] *= area;
    }

  /* Wenn der advektive Anteil per Lagrangeverfahren
     behandelt wird, muss er hier zu Null gesetzt werden */
  if (atm_use_lagrange_for_2D)
    for (i = 0; i < 16; i++)
      adv[i] = 0.;

  /* Einspeichern der ermittelten Matrizen */
  if (atm_new_kernel)
    {
      ATMSetElementMassMatrixNew(index, phase, mass);
      ATMSetElementDispMatrixNew(index, phase, disp);
      ATMSetElementAdvMatrixNew(index, phase, adv);
      if (ATMSetElementFluidContentMatrixNew)
        ATMSetElementFluidContentMatrixNew(index, phase, content);
    }
  else
    {
      ATMSetElementMassMatrix(index, mass);
      ATMSetElementDispMatrix(index, disp);
      ATMSetElementAdvMatrix(index, adv);
    }



#ifdef TESTCEL_ATM
  MZeigMat(mass, 4, 4, "CalcEle2D_ATM: heat capacitance matrix");
  MZeigMat(disp, 4, 4, "CalcEle2D_ATM: heat diffusion-dispersion matrix");
  MZeigMat(adv, 4, 4, "CalcEle2D_ATM: heat advection matrix");
#endif
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle3D

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

**************************************************************************/
void CalcEle3D(int phase, int component, long index)
{
  /* Datentypen der verwendeten Variablen */
  /* Laufvariablen */

  static long i, j, k, l, ii;
  static long ind;
  static double r, s, t;
  /* Elementgeometriedaten */
  static double invjac[9], jacobi[9], detjac;
  static double phi[8], ome[24];
  /* Elementmatrizen */
  static double *mass;          /* Massenmatrix */
  static double *content;       /* Aenderung des Feuchtegehalts */
  static double *disp;          /* Dispersionsmatrix */
  static double *adv;           /* Advektionsmatrix */
  /* Elementdaten */
  static int anzgp;
  static double alpha[3], alpha_adv[3], pec;    /* SUPG */
  static double art_diff;       /* ETG */
  static double *velovec, v[3], vt[3], v_rst[3], vg;
  static double d[9], tdt[9];
  static double retard;
  static double fkt, sum;
  /* Materialdaten */
  static double porosity, dC;
  static double mass_dispersion_longitudinal, mass_dispersion_transverse, molecular_diffusion;
  /* Hilfsfelder */
  static double zwi[64];
  static double zwa[24], zwo[9];

#ifdef TESTSUPG
  DisplayMsg("3D: Element-Nr=");
  DisplayLong(index);
  DisplayMsgLn("");
#endif

  /* Speicher holen */
  if (atm_new_kernel)
    {
      /* Neue Methode fuer Mehrphasenmodell, Speicher wird nur einmal allokiert */
      mass = ATMGetElementMassMatrixNew(index, phase);
      if (!mass)
        mass = (double *) Malloc(64 * sizeof(double));

      disp = ATMGetElementDispMatrixNew(index, phase);
      if (!disp)
        disp = (double *) Malloc(64 * sizeof(double));

      adv = ATMGetElementAdvMatrixNew(index, phase);
      if (!adv)
        adv = (double *) Malloc(64 * sizeof(double));

      content = ATMGetElementFluidContentMatrixNew(index, phase);
      if (!content)
        content = (double *) Malloc(64 * sizeof(double));
    }
  else
    {
      /* Alte Methode ATM, Speicher der alten Matrix wird beim
         Setzen der neuen Matrix freigegeben -> langsamer */
      mass = (double *) Malloc(64 * sizeof(double));
      disp = (double *) Malloc(64 * sizeof(double));
      adv = (double *) Malloc(64 * sizeof(double));
    }

  /* Initialisieren */
  MNulleMat(mass, 8, 8);
  MNulleMat(disp, 8, 8);
  MNulleMat(adv, 8, 8);
  if (ATMSetElementFluidContentMatrixNew)
    MNulleMat(content, 8, 8);

  /* Elementdaten und globale Modellparameter bereitstellen */
  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);
  mass_dispersion_transverse = GetSoilMassDispersionT(index);

  retard = ATMGetElementRetardFac(index);       /* Retardierungsfaktor */
  if (atm_new_kernel)
    pec = ATMGetElementPecletNumNew(index, phase);
  else
    pec = ATMGetElementPecletNum(index);

  /* Fuer teilgesaettigte Verhaeltnisse: */
  if (ATMGetFluidContent)
    porosity = ATMGetFluidContent(phase, index, 0., 0., 0., atm_time_collocation_global);
  else
    porosity = GetSoilPorosity(index);

  anzgp = GetNumericsGaussPoints(ElGetElementType(index));

  /* Filtergeschwindigkeit in den Gausspunkten */
  velovec = ElGetVelocityNew(index, phase);

  /* SUPG */
  MNulleVec(alpha, 3);
  MNulleVec(alpha_adv, 3);

  if (atm_upwind_method && (atm_upwind_parameter > 0.0))
    {
      CalcVeloXDrst(phase, index, atm_time_collocation_upwinding, 0., 0., 0., v);

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
                alpha[k] = alpha_adv[k] = v[k] / vg * atm_upwind_parameter * (fabs(pec) - 1.0) / pec;
              if (atm_upwind_method == 2)
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
  ind = -1;
  for (i = 0; i < anzgp; i++)
    for (j = 0; j < anzgp; j++)
      for (k = 0; k < anzgp; k++)
        {
          r = MXPGaussPkt(anzgp, i);
          s = MXPGaussPkt(anzgp, j);
          t = MXPGaussPkt(anzgp, k);

          /* Faktoren */
          Calc3DElementJacobiMatrix(index, r, s, t, invjac, &detjac);
          fkt = MXPGaussFkt(anzgp, i) * MXPGaussFkt(anzgp, j) * \
              MXPGaussFkt(anzgp, k) * detjac;

          /* Veraenderliche Porositaet? */
          if (ATMGetFluidContent)
            porosity = ATMGetFluidContent(phase, index, r, s, t, atm_time_collocation_global);

          sum = fkt * porosity;

          /* Abstandsgeschwindigkeit im Gauss-Punkt */
          for (l = 0; l < 3; l++)
            v[l] = velovec[++ind] / porosity;
          vg = MBtrgVec(v, 3);

          /* hydrodynamischen Dispersionstensor im Gauss-Punkt berechnen */
          MNulleMat(d, 3, 3);
          /* Stofftransport */
          art_diff = ElGetArtDiff(index);
          d[0] = (molecular_diffusion + art_diff) + mass_dispersion_longitudinal * vg;
          d[4] = d[8] = (molecular_diffusion + art_diff) + mass_dispersion_transverse * vg;
          /* Drehen in r,s-Richtung ???
             OK offensichtlich Drehen des Tensors von stromlinienorietierten
             in lokale physikalische (Element) Koordinaten
             wird benoetigt, wenn Dispersionstensor in Hauptachsen angegeben ist */
          if (d[0] > MKleinsteZahl || d[4] > MKleinsteZahl || d[8] > MKleinsteZahl)
            {
              /* Drehen: Stromrichtung - r,s,t */
              if (vg > MKleinsteZahl)
                {
                  /* 1. Zeile */
                  for (l = 0; l < 3; l++)
                    zwa[l] = v[l];
                  MNormiere(zwa, 3);
                  /* 2. Zeile */
                  fkt = fabs(v[0]);
                  ii = 0;
                  for (l = 1; l < 3; l++)
                    if (fabs(v[l]) < fkt)
                      {
                        fkt = fabs(v[l]);
                        ii = l;
                      }
                  zwo[ii] = 0.0;
                  zwo[(ii + 1) % 3] = v[(ii + 2) % 3];
                  zwo[(ii + 2) % 3] = -v[(ii + 1) % 3];
                  MNormiere(zwo, 3);
                  /* 3. Zeile */
                  M3KreuzProdukt(zwa, zwo, zwi);
                  MNormiere(zwi, 3);
                  for (l = 0; l < 3; l++)
                    {
                      zwa[3 + l] = zwo[l];
                      zwa[6 + l] = zwi[l];
                    }
                  /* dreh^T * D * dreh */
                  MMultMatMat(d, 3, 3, zwa, 3, 3, zwi, 3, 3);
                  MTranspoMat(zwa, 3, 3, zwo);
                  MMultMatMat(zwo, 3, 3, zwi, 3, 3, d, 3, 3);
                }
            }
          /* vt = v * J^-1 */
          MMultVecMat(v, 3, invjac, 3, 3, vt, 3);
          /* tdt = (J^-1)T * D * J^-1 */
          MMultMatMat(d, 3, 3, invjac, 3, 3, zwa, 3, 3);
          MTranspoMat(invjac, 3, 3, zwi);
          MMultMatMat(zwi, 3, 3, zwa, 3, 3, tdt, 3, 3);


/************************************************************************/
/* Massenmatrix                                                         */
/************************************************************************/
          /* phi * omega */
          MPhi3D_SUPG(phi, r, s, t, alpha);
          MOmega3D(zwo, r, s, t);

          if (atm_mass_lumping_method == 0)
            {
              /* Normale FEM */
              /* phi * omega */
              MMultVecVec(phi, 8, zwo, 8, zwi, 8, 8);
              /* Stofftransport - sorption matrix */
              for (l = 0; l < 64; l++)
                mass[l] += (zwi[l] * sum);
            }
          else if (atm_mass_lumping_method == 1)
            {
              /* Wenn die Speicherung per ML-FEM
                 behandelt wird, wird nur die Diagonale gesetzt */
              for (l = 0; l < 8; l++)
                mass[l * 9] += zwo[l] * sum;
            }

/********************************************************************/
/*  Fluidgehaltsmatrix                                              */
/********************************************************************/
          if (ATMSetElementFluidContentMatrixNew)
            {
              dC = (ATMGetFluidContent(phase, index, r, s, t, 1.) - ATMGetFluidContent(phase, index, r, s, t, 0.));
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
          if (atm_upwind_method == 2)
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

        }                              /* Ende der Schleife ueber GaussPunkte */


  /* Stofftransport */
  for (l = 0; l < 64; l++)
    {
      mass[l] *= retard;
      if (ATMSetElementFluidContentMatrixNew)
        content[i] *= retard;
    }

  /* Einspeichern der ermittelten Matrizen */
  if (atm_new_kernel)
    {
      ATMSetElementMassMatrixNew(index, phase, mass);
      ATMSetElementDispMatrixNew(index, phase, disp);
      ATMSetElementAdvMatrixNew(index, phase, adv);
      if (ATMSetElementFluidContentMatrixNew)
        ATMSetElementFluidContentMatrixNew(index, phase, content);
    }
  else
    {
      ATMSetElementMassMatrix(index, mass);
      ATMSetElementDispMatrix(index, disp);
      ATMSetElementAdvMatrix(index, adv);
    }

#ifdef TESTCEL_ATM
  MZeigMat(mass, 8, 8, "CalcEle3D_ATM");
  MZeigMat(disp, 8, 8, "CalcEle3D_ATM");
  MZeigMat(adv, 8, 8, "CalcEle3D_ATM");
#endif
}


/**************************************************************************
 ROCKFLOW - Funktion: CalcEle3DPrism_ATM_ana

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
void CalcEle3DPrism_ATM_ana(int phase, int component,long index,\
                   double *tracer_capacitance_matrix,\
                   double *tracer_diffusion_matrix,\
                   double *tracer_advection_matrix)
{
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
  static double vg;
  static double v[3];
  static double fac1;
  static double* d=NULL;
  /* Laufvariablen */
  long i;


  /* Speicherplatzreservierung fuer die Elementmatrizen und -vektoren */
  if (memory_opt == 0) {                /* keine Speicheroptimierung */
    tracer_capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
    tracer_diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
    tracer_advection_matrix = (double *) Malloc(nn2 * sizeof(double));
  }
  d = (double *) Malloc(9 * sizeof(double));

//  element_nodes = (long *) Malloc(6 * sizeof(long));

  /* Initialisieren */
  MNulleMat(tracer_capacitance_matrix,nn,nn);
  MNulleMat(tracer_diffusion_matrix,nn,nn);
  MNulleMat(tracer_advection_matrix,nn,nn);
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
  GetPriMatFromTriMat(Tri_x_Tri, tracer_capacitance_matrix);
  MMultMatMat2(tracer_capacitance_matrix, 6, 6, Ct, tracer_capacitance_matrix);


  /*------------------------------------------------------------------------*/ 
  /* --- Tracer diffusion/dispersion matrix ------------------------------- */
  /*------------------------------------------------------------------------*/

  /* Materialparameter */
  porosity = GetSoilPorosity(index);
  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);
  mass_dispersion_transverse = GetSoilMassDispersionT(index);

    /* Abstandsgeschwindigkeiten im Elementmittelpunkt */
  v[0] = VELGetElementVelocityX(index, 0) / porosity;
  v[1] = VELGetElementVelocityY(index, 0) / porosity;
  v[2] = VELGetElementVelocityZ(index, 0) / porosity;

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
    tracer_diffusion_matrix[i] =  Dijxx[i] + Dijzz[i] + Dijyy[i] \
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
    tracer_advection_matrix[i] = Bijxx[i] + Bijyy[i] + Bijzz[i];
  }

  /*------------------------------------------------------------------------*/
  /* --- Einspeichern der ermittelten Matrizen ---------------------------- */
  /*------------------------------------------------------------------------*/
  if (atm_new_kernel)
  {
    ATMSetElementMassMatrixNew(index, phase, tracer_capacitance_matrix);
    ATMSetElementDispMatrixNew(index, phase, tracer_diffusion_matrix);
    ATMSetElementAdvMatrixNew(index, phase, tracer_advection_matrix);
  }
  else
  {
    ATMSetElementMassMatrix(index, tracer_capacitance_matrix);
    ATMSetElementDispMatrix(index, tracer_diffusion_matrix);
    ATMSetElementAdvMatrix(index, tracer_advection_matrix);
  }

  /*------------------------------------------------------------------------*/
  /* Speicherfreigabe */
  d = (double *) Free(d);


#ifdef TESTCEL_ATM
DisplayMsgLn(" "); DisplayDouble(v[0],0,0); DisplayMsg(", "); DisplayDouble(v[1],0,0); DisplayMsg(", "); DisplayDouble(v[2],0,0); DisplayMsgLn(", ");
  MZeigMat(tracer_capacitance_matrix, nn, nn, "CalcEle3DPrism_ATM_ana: MASS MATRIX");
  MZeigMat(tracer_diffusion_matrix, nn, nn, "CalcEle3DPrism_ATM_ana: DISPERSION MATRIX");
  MZeigMat(tracer_advection_matrix, nn, nn, "CalcEle3DPrism_ATM_ana: ADVECTION MATRIX");
#endif

}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcEle3DPrism_ATM_num
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
void CalcEle3DPrism_ATM_num(int phase, int component,long index,\
                   double *tracer_capacitance_matrix,\
                   double *tracer_diffusion_matrix,\
                   double *tracer_advection_matrix)

{
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
  static double vg;
  static double* d;
  static double tDt[9];
  static double* velo=NULL;
  static double velot[3];

  /* Speicherplatzreservierung  */
  if(memory_opt==0) { /* schnelle, speicherintensive Variante */
    tracer_capacitance_matrix = (double *) Malloc(nn2 * sizeof(double));
    tracer_diffusion_matrix = (double *) Malloc(nn2 * sizeof(double));
    tracer_advection_matrix = (double *) Malloc(nn2 * sizeof(double));
  }
  d = (double *) Malloc(9 * sizeof(double));

  /* Initialisieren */
  MNulleMat(tracer_capacitance_matrix, nn, nn);
  MNulleMat(tracer_diffusion_matrix, nn, nn); 
  MNulleMat(tracer_advection_matrix, nn, nn);

  /* Datenbereitstellung */
  /* Numerische Parameter */
  NUMERICALPARAMETER *numerics;
  numerics = NumericsGetPointer("PRESSURE0");
  theta = numerics->time_collocation;
  anzgptri = 3;
  anzgplin = 2;
  /* Materialparameter */
  porosity = GetSoilPorosity(index);
  molecular_diffusion = GetTracerDiffusionCoefficient(index, phase, component) * GetSoilTortuosity(index);
  mass_dispersion_longitudinal = GetSoilMassDispersionL(index);
  mass_dispersion_transverse = GetSoilMassDispersionT(index);


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
      velo = CalcVelo3DPrismRST(phase, index, theta, r, s, t);

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
        tracer_capacitance_matrix[l] += (zwi[l] * fkt);
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
        tracer_diffusion_matrix[l] += (zwu[l] * fkt);        
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
        tracer_advection_matrix[l] += (zwu[l] * fkt);        
      }
   
    }  /* Ende Schleife über Anzahl der GaussPunkte im Dreieck (xy-Richtung) */
  }    /* Ende Schleife über Anzahl der GaussPunkte in z Richtung */


  /*------------------------------------------------------------------------*/
  /* --- Einspeichern der ermittelten Matrizen ---------------------------- */
  /*------------------------------------------------------------------------*/
  if (atm_new_kernel)  {
    ATMSetElementMassMatrixNew(index, phase, tracer_capacitance_matrix);
    ATMSetElementDispMatrixNew(index, phase, tracer_diffusion_matrix);
    ATMSetElementAdvMatrixNew(index, phase, tracer_advection_matrix);
  }
  else  {
    ATMSetElementMassMatrix(index, tracer_capacitance_matrix);
    ATMSetElementDispMatrix(index, tracer_diffusion_matrix);
    ATMSetElementAdvMatrix(index, tracer_advection_matrix);
  }

#ifdef TESTCEL_ATM
  MZeigMat(tracer_capacitance_matrix, nn, nn, "CalcEle3DPrism_ATM_num: MASS MATRIX");
  MZeigMat(tracer_diffusion_matrix, nn, nn, "CalcEle3DPrism_ATM_num: DISPERSION MATRIX");
  MZeigMat(tracer_advection_matrix, nn, nn, "CalcEle3DPrism_ATM_num: ADVECTION MATRIX");
#endif

  /* Speicherfreigabe */
  d = (double *) Free(d);
}


/**************************************************************************
   ROCKFLOW - Funktion: ATMDampOscillations

   Aufgabe:
   
   Daempft Oszillationen des Feldes oder begrenzt den Wertebereich.

   Ergebnis:
   - void -

   Programmaenderungen:
   07/2002   CT   Erste Version

  **************************************************************************/
void ATMDampOscillations(int ndx1, int phase)
{
   activ_phase = phase;
   DampOscillations(ndx1, atm_oscil_damp_method, atm_oscil_damp_parameter, ATMNodeCalcLumpedMass);
}


/**************************************************************************
   ROCKFLOW - Funktion:  ATMNodeCalcLumpedMass

   Aufgabe:
   Liefert fuer einen Knoten die "gelumpte" Speichermatrix

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB

   Ergebnis:
     double mass : Rueckgabe, des Lumped-Wertes

   Programmaenderungen:
   7/2002 C.Thorenz Erste Version

 **************************************************************************/
double ATMNodeCalcLumpedMass(long knoten)
{
  double *mass=NULL, lm=0.;
  long *elemente, *nodes;
  int anz_elemente, i, j, k, nn;

  elemente = GetNodeElements(knoten, &anz_elemente);

  for (i = 0l; i < anz_elemente; i++) {
    nn = ElNumberOfNodes[ElGetElementType(elemente[i]) - 1];
    
    mass = ATMGetElementMassMatrixNew(elemente[i], activ_phase);
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


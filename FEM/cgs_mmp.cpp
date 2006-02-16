/**************************************************************************
  ROCKFLOW - Modul: cgs_mmp.c

  Aufgabe:
  Aufstellen des Gleichungssystems fuer das Modul MMP.

  Programmaenderungen:

   9/97     C.Thorenz       Erste Version
  11/1998   C. Thorenz      Erste rausgegangene Version
   7/1999   C.Thorenz       3D-Elemente
   9/1999   C.Thorenz       2D-Elemente
  11/1999   RK              EliminateIrrNodes()
  12/1999   C.Thorenz       Irr. Knoten _vor_ Dirichlet-BC
  07/2000   OK              MakeGS_RichardsV1
  11/2000   C.Thorenz       Div. Korrekturen
   1/2001   C.Thorenz       Druck-Extraktion fuer Richards-Modell
  09/2001   OK              MakeGS_MMPV1 (fuer LES-Objekte)
  09/2002   OK              MMPMakeElementEntryPhase
  03/2003   RK              Quellcode bereinigt, Globalvariablen entfernt
 08/2004 OK MFP implementation
 08/2004 OK MMP implementation
**************************************************************************/
#include "stdafx.h"
#include "makros.h"
#include "int_mmp.h"
#include "int_mms.h"
#include "elements.h"
#include "nodes.h"
#include "mathlib.h"
#include "matrix.h"
#include "edges.h"
#include "plains.h"
#include "femlib.h"
#include "adaptiv.h"
#include "rf_bc_new.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "pcs_dm.h"
// Elem object
#include "fem_ele_std.h"

extern double CECalcSatuFromCap_AMM(long element, double kap12);
extern double GetSoilFluidContinuumExchange(long index, double p_up, double p, double p_do, double *Q_up, double *R_up, double *c_up, double *c_up2, double *Q_do, double *R_do, double *c_do, double *c_do2);

#define noTEST_CGS_MMP_CPL

/* Interne (statische) Deklarationen */
void MMPAssembleMatrices(int, double *, double *);
void MMPAssembleMatricesRichards(int, double *, double *);
void MMPCalcFluxes(int, double *, double *);
void MMPDeleteFluxes(int);
void MMPIncorporateSourceBoundaryConditions(int phasen, double *rechts, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name);
void MMPIncorporateSourceBoundaryCondition(int phase, double *rechts, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name);
void MMPMakeElementEntry(int, long, double *, double *);
void MMPMakeElementEntryPressureExtract(int phasen, long element, double *ergebnis_MMP, double *rechts);
void MMPMakeMassMatrixEntry(int phase, long element, double *rechts);
double MMPGetSinkMultiplicator(long i);
double MMPGetSourceMultiplicator(long i);
void MMPIncorporateFluidExchange(double *rechts);

double MMPBoundary(long knoten, int *gueltig, double val);
double MMPRichardsOpenBoundary(long knoten, int *gueltig, double val);
double MMPRichardsMassSourceSinkFunction(long knoten, int *gueltig, double val);
double MMPRichardsVolumeSourceSinkFunction(long knoten, int *gueltig, double val);
double MMPCalcNotSaturatedPatchVolume(int phase, long element, long node);
double MMPCalcPatchVolume(long number);

/* Data access to nodal data */
IntFuncII MMPGetPhasePressureNodeIndex;

static int activ_phase;
static double *rechte_seite;

extern int mmp_reference_pressure_method;

/**************************************************************************
 ROCKFLOW - Funktion: MakeGS_MMP

 Aufgabe:
   Gleichungssystem aufstellen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz       Erste Version
 11/1998  C. Thorenz      Erste rausgegangene Version

**************************************************************************/
void MakeGS_MMP(int phasen, double *ergebnis_MMP, double *rechts, char *bc_name, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name)
{
bc_name = bc_name;
  /* Fluesse der Phasen loeschen, bei Mehrkontinua nur beim ersten Durchlauf */
//SB:m  if(MaterialGetActiveContinuum()==0) //SB auskommentiert, da immer = 0 bei nur einem Continuum
     MMPDeleteFluxes(phasen);

  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren */
  MMPAssembleMatrices(phasen, ergebnis_MMP, rechts);

  /* Quellterme einbauen */
//SB:m  if(MaterialGetActiveContinuum() == 0)
    MMPIncorporateSourceBoundaryConditions(phasen, rechts, source_mass_flow_name, source_volume_flow_name, sink_volume_mixture_name);

  /* Austauschterme fuer Multikontinuumansaetze einbauen */
  MMPIncorporateFluidExchange(rechts);

  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  /* Dirichlet-Randbedingungen eintragen */
  //OK_BC IncorporateBoundaryConditionsEx2(bc_name, rechts, MMPBoundary);
}

/**************************************************************************
 ROCKFLOW - Funktion: MakeGS_MMPV1

 Aufgabe:
   System-Matrix assemblieren

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:
  05/2001   OK   Ableitung von MakeGS_MMP (CT)

**************************************************************************/
/*AssembleSystemMatrix_MMP*/
void MakeGS_MMPV1(double *rechts, double *ergebnis_MMP, double dummy)
{
  int phasen=2; /* ToDo - Wert übergeben ?*/

  dummy=dummy;

  /* 1 ------------------------------------------------------------------*/
  /* Matrizen ggf. neu aufbauen */
    /* Standart Mehrphasen-FEM */
  if (GetMethod_MMP() == 1)
    {
      /*  Matrizen ggf. neu aufbauen */
      //OK_MOD MMTM0699RebuildMatricesPressure(0);
    }

  /* Fluesse der Phasen loeschen, bei Mehrkontinua nur beim ersten Durchlauf */
//SB:m  if(MaterialGetActiveContinuum()==0)
     MMPDeleteFluxes(phasen);

  /* 2 ------------------------------------------------------------------*/
  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren */
  MMPAssembleMatrices(phasen, ergebnis_MMP, rechts);

  /* 3 ------------------------------------------------------------------*/
  /* incorporate source terms        - ExecuteLinearSolver */
  //OK_ST MMPIncorporateSourceBoundaryConditions(phasen, rechts, 
  //   name_source_mass_fluid_phase, name_source_volume_fluid_phase, name_sink_volume_fluid_mixture);

  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  /* 4 ------------------------------------------------------------------*/
  /* incorporate boundary conditions - ExecuteLinearSolver */
  //OK_BC IncorporateBoundaryConditionsEx2("PRESSURE1", rechts, MMPBoundary);

}


/**************************************************************************
 ROCKFLOW - Funktion: MakeGS_Richards
 Aufgabe:
   Gleichungssystem aufstellen fuer druckformulierte Richardsgleichung

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 12/1999     C.Thorenz       Erste Version

**************************************************************************/
void MakeGS_Richards(int phase, double *ergebnis_MMP, double *rechts, char *bc_name, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name)
{
bc_name = bc_name;
  char name[512];

  sink_volume_mixture_name = sink_volume_mixture_name;

  /* Wird bei der Behandlung gemischter RB benoetigt */
  rechte_seite = rechts;

  /* Fluesse der Phasen(1) loeschen, bei Mehrkontinua nur beim ersten Durchlauf */
//SB:m  if(MaterialGetActiveContinuum()==0)
    MMPDeleteFluxes(1);

  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren */
  MMPAssembleMatricesRichards(phase, ergebnis_MMP, rechts);

  /* Fluesse der Phasen(1) ermitteln */
  MMPCalcFluxes(1, ergebnis_MMP, rechts);

  /* Massen-Quellen einarbeiten, werden in Volumen umgerechnet */
  sprintf(name, "%s%i", source_mass_flow_name, phase + 1);
  //OK_ST IncorporateSourceSinkEx(name, rechts, MMPRichardsMassSourceSinkFunction);

  /* Volumen-Quellen einarbeiten */
  sprintf(name, "%s%i", source_volume_flow_name, phase + 1);
  //OK_ST IncorporateSourceSinkEx(name, rechts, MMPRichardsVolumeSourceSinkFunction);

  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  /* Dirichlet-Randbedingungen eintragen */
  //OK_BC IncorporateBoundaryConditionsEx2(bc_name, rechts, MMPBoundary);
  //OK_BC IncorporateBoundaryConditionsEx2("FREE_OUTFLOW1", rechts, MMPRichardsOpenBoundary);

}

/**************************************************************************
  ROCKFLOW - Funktion: MakeGS_RichardsV1

  Aufgabe:
  Systemmatrix und RHS-Term (ohne SS-Terme) aufstellen

  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  R double *rechts: rechte Seite GS (Vektor)
  R double *ergebnis: Ergebnis des GS, bei Rueckgabe Nullvektor

  Ergebnis:
  - void -

  Programmaenderungen:
  06/2000   OK   Ableitung von MakeGS_Richards (CT)
   9/2000   CT   Aufruf in Modell 0699 auskommentiert

***************************************************************************/
void MakeGS_RichardsV1(double *rechts, double *ergebnis_MMP, double dummy)
{
  int phase = 0;

  dummy=dummy;

  /* Alle Matrizen (-1) neu aufbauen */
  //OK_MOD MMTM0699RebuildMatricesPressureRichards();
/*    MMTM0699RebuildMatricesSaturation(-1, 0); */

  /* Fluesse der Phasen(1) loeschen, bei Mehrkontinua nur beim ersten Durchlauf */
//SB:m  if(MaterialGetActiveContinuum()==0)
    MMPDeleteFluxes(1);

  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren */
  MMPAssembleMatricesRichards(phase, ergebnis_MMP, rechts);
}



/**************************************************************************
 ROCKFLOW - Funktion: MMPRichardsOpenBoundary
 Aufgabe:
   RB-Funktion fuer Richards-Modell: Offene Raender, an- und abschalten von RB
   Fuer "seepage" RB.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
 E: long knoten: Knotennummer
    double val : Vorgesehener Wert fuer Randbedingung
 R: int gueltig: Flag, gibt an ob RB beachtet werden soll

 Ergebnis:
  double: neuer Wert fuer RB

 Programmaenderungen:

 12/1999     C.Thorenz       Erste Version
  1/2001     C.Thorenz       Druckextraktion dazu

**************************************************************************/
double MMPRichardsOpenBoundary(long knoten, int *gueltig, double val)
{
  double p, p_offset, flux, theta;
  static double p_max = 0.;

  val = val;

  /* Mit Druckextraktion rechnen ? */
  if (GetValuesExtractMethod_MMP() == 1)
    {
      p_offset = MMPGetNodePress(0, knoten, -1, 0);
    }
  else
    {
      p_offset = 0.;
    }

  theta = GetTimeCollocationCondBC_MMP();

  p = (1. - theta) * MMPGetNodePress(0, knoten, -1, 0) + theta * MMPGetNodePress(0, knoten, -1, 1);
  flux = (1. - theta) * MMPGetNodeFlux(0, knoten, -1, 0) + theta * MMPGetNodeFlux(0, knoten, -1, 1);

  p_max = max(p_max, p);

  /* Negativer Druck: Abschalten */
  if (p / (p_max + MKleinsteZahl) < -MKleinsteZahl)
    {
      *gueltig = 0;
      return 0.;
    }
  /* Positiver Druck: Ausfuehren */
  if (p / (p_max + MKleinsteZahl) > MKleinsteZahl)
    {
      *gueltig = 1;
      return -p_offset;
    }

  /* Druck ungefaehr Null: Fluss entscheidet */
  if (flux < 0.)
    {
      /* Aus den Gebiet heraus: gueltig */
      *gueltig = 1;
      return -p_offset;
    }
  else
    {
      /* In das Gebiet hinein: ungueltig */
      *gueltig = 0;
      return 0.;
    }

}


/**************************************************************************
 ROCKFLOW - Funktion: MMPRichardsBoundary
 Aufgabe:

   RB-Funktion um bei Druckextraktion den RB-Wert veraendern zu koennen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
 E: long knoten: Knotennummer
    double val : Vorgesehener Wert fuer Randbedingung

 R: int gueltig: Flag, gibt an ob RB beachtet werden soll

 Ergebnis:
  double: neuer Wert fuer RB

 Programmaenderungen:

  1/2001     C.Thorenz       Druckextraktion

**************************************************************************/
double MMPBoundary(long knoten, int *gueltig, double val)
{
  double p_offset;

  val = val;

  /* Mit Druckextraktion rechnen ? */
  if (GetValuesExtractMethod_MMP() == 1)
    {
      p_offset = MMPGetNodePress(0, knoten, -1, 0);
    }
  else
    {
      p_offset = 0.;
    }

  *gueltig = 1;

  return val - p_offset;

}

/**************************************************************************
 ROCKFLOW - Funktion: MMPRichardsMassSourceSinkFunction
 Aufgabe:
   Quellen und Senken fuer Richards-Modell.
   Umrechnen von Massen- auf Volumenstrom, ggf. an offenen Raendern
   oder Raendern mit Dirichlet-RB abschalten.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
 E: long knoten: Knotennummer
    double val : Vorgesehener Wert fuer Randbedingung
 R: int gueltig: Flag, gibt an ob Quelle beachtet werden soll

 Ergebnis:
  double: neuer Wert fuer Quelle

 Programmaenderungen:

 12/1999     C.Thorenz       Erste Version
**************************************************************************/
double MMPRichardsMassSourceSinkFunction(long knoten, int *gueltig, double val)
{
  /* Fuehrt zu Oszillationen bei kompressiblen Fluiden bei theta>0. !!! */
	val /= MMax(mfp_vector[0]->Density(), MKleinsteZahl);
  return MMPRichardsVolumeSourceSinkFunction(knoten, gueltig, val);
}

double MMPRichardsVolumeSourceSinkFunction(long knoten, int *gueltig, double val)
{
  double volume = 0.;

  gueltig=gueltig;

  /* Soll bedingte RB gesetzt werden? Dann nur beschraenkte Zufluesse! */
//OK_IC  if (IsNodeBoundaryCondition("FREE_OUTFLOW1", knoten))
    {
      /* Bestimmen des maximalen Zuflusses */
      volume = MMPCalcPatchVolume(knoten) * (1. - MMPGetNodeSatu(0, knoten, -1, 1));

#ifdef der_fluss_wird_beschraenkt
      val = MMin(val, volume / dt);
#else
      /* Knoten erreicht Vollsaettigung -> RB mit P=0  */
      if (val > (volume / dt))
        {
          MXRandbed(GetNodeIndex(knoten), 0., rechte_seite);
          val = 0.;
        }
#endif

    }

  return val;
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPAssembleMatrices

 Aufgabe:
   Gleichungssystem aus Einzelmatrizen aufstellen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 3/1999  C. Thorenz      Erste Version

**************************************************************************/
void MMPAssembleMatrices(int phasen, double *ergebnis_MMP, double *rechts)
{
  /* Variablen */
  static long i;
  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);
  /* Linke und rechte Seite des Gleichungssystems aufbauen */
  /* Mit Ergebnisextraktion ? */
  if (!GetValuesExtractMethod_MMP() == 1)
    {
      for (i = 0; i < anz_active_elements; i++)
        MMPMakeElementEntry(phasen, ActiveElements[i], ergebnis_MMP, rechts);
    }
  else
    {
      for (i = 0; i < anz_active_elements; i++)
        MMPMakeElementEntryPressureExtract(phasen, ActiveElements[i], ergebnis_MMP, rechts);
    }
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPAssembleMatricesRichards

 Aufgabe:
   Gleichungssystem aus Einzelmatrizen aufstellen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 12/1999  C. Thorenz      Erste Version

**************************************************************************/
void MMPAssembleMatricesRichards(int phase, double *ergebnis_MMP, double *rechts)
{
  /* Variablen */
  static long i;
  int phasen = 1;

  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);

  /* Beginn der Elementschleife */
  /* Linke und rechte Seite aufbauen aus Flussmatrizen aller Elemente */

  /* Mit Ergebnisextraktion ? */
  if (!GetValuesExtractMethod_MMP() == 1)
    {
      for (i = 0; i < anz_active_elements; i++)
        MMPMakeElementEntry(phasen, ActiveElements[i], ergebnis_MMP, rechts);
    }
  else
    {
      for (i = 0; i < anz_active_elements; i++)
        MMPMakeElementEntryPressureExtract(phasen, ActiveElements[i], ergebnis_MMP, rechts);
    }

  /* Linke und rechte Seite aufbauen aus Massenmatrizen aller Elemente.
     Nur bei Formulierung nach Celia sind diese Matrizen vorhanden. */
  for (i = 0; i < anz_active_elements; i++)
    MMPMakeMassMatrixEntry(phase, ActiveElements[i], rechts);
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPCalcFluxes

 Aufgabe:
   Ermittelt Volumen-Fluesse der einzelnen Phasen ueber Knoten.
   Wird fuer Behandlung der offenen Raender in der Saettigungsgleichung
   und fuer die Ausgabe benutzt. Nicht korrekt wenn mehrere Material-
   gruppen an einem Knoten angrenzen und echt mehrphasig gerechnet
   wird. Dann muss die Berechnung auf Elementebene erfolgen.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 3/1999  C. Thorenz      Erste Version

**************************************************************************/
void MMPCalcFluxes(int phasen, double *ergebnis_MMP, double *rechts)
{
  int phase;
  long i;
  double *residuum, total_mobility, fractional_mobility, fractional_flux;
  long index = 0;
  double theta = GetTimeCollocationGlobal_MMP();

  residuum = (double *) Malloc(NodeListLength * sizeof(double));
  MXResiduum(ergebnis_MMP, rechts, residuum);

  /* Der Fluss einer einzelnen Phase ergibt sich aus Gesamtfluss und Fractional Flow */
  for (i = 0; i < NodeListLength; i++)
    {
      total_mobility = 0.;
      for (phase = 0; phase < phasen; phase++)
        {
          total_mobility += MMPGetNodeRelativePermeability(phase, index, NodeNumber[i], theta)
              / mfp_vector[phase]->Viscosity() + MKleinsteZahl;
        }
      for (phase = 0; phase < phasen; phase++)
        {
          fractional_mobility = (MMPGetNodeRelativePermeability(phase, index, NodeNumber[i], theta) + MKleinsteZahl)
              / mfp_vector[phase]->Viscosity()/ (total_mobility + MKleinsteZahl); //SB:m
          fractional_flux = residuum[i] * fractional_mobility;
          MMPSetNodeFlux(phase, NodeNumber[i], 1, -fractional_flux);
        }
    }
  residuum = (double *) Free(residuum);
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPDeleteFluxes

 Aufgabe:
   Loescht die Eintraege fuer  Volumen-Fluesse der einzelnen Phasen
   ueber Knoten.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 12/1999  C. Thorenz      Erste Version

**************************************************************************/
void MMPDeleteFluxes(int phasen)
{
  int phase;
  long i;

  for (i = 0; i < NodeListLength; i++)
    for (phase = 0; phase < phasen; phase++)
      MMPSetNodeFlux(phase, NodeNumber[i], 1, 0.);
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPIncorporateSourceBoundaryConditions

 Aufgabe:
   RB in Gleichungssystem einbauen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz      Erste Version
 11/1998  C.Thorenz      Erste rausgegangene Version

**************************************************************************/
void MMPIncorporateSourceBoundaryConditions(int phasen, double *rechts, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name)
{

  for (activ_phase = 0; activ_phase < phasen; activ_phase++)
    MMPIncorporateSourceBoundaryCondition(activ_phase, rechts, source_mass_flow_name, source_volume_flow_name, sink_volume_mixture_name);

  /* Volumen-Punktsenken einarbeiten */
  //OK_ST IncorporateSourceSink(sink_volume_mixture_name, rechts, MMPGetSinkMultiplicator);
}

/**************************************************************************
 ROCKFLOW - Funktion: MMPIncorporateSourceBoundaryCondition

 Aufgabe:
   Massen- und Volumenquellen fuer eine Phase in Gleichungssystem einbauen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz      Erste Version
 11/1998  C.Thorenz      Erste rausgegangene Version

**************************************************************************/
void MMPIncorporateSourceBoundaryCondition(int phase, double *rechts, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name)
{
rechts = rechts;
  char flux[80];

  sink_volume_mixture_name = sink_volume_mixture_name;

  /* Massen-Punktquellen einarbeiten, werden in Volumen umgerechnet */
  sprintf(flux, "%s%i", source_mass_flow_name, phase + 1);
  //OK_ST IncorporateSourceSink(flux, rechts, MMPGetSourceMultiplicator);

  /* Volumen-Punktquellen einarbeiten */
  sprintf(flux, "%s%i", source_volume_flow_name, phase + 1);
  //OK_ST IncorporateSourceSink(flux, rechts, NULL);
}
/**************************************************************************
 ROCKFLOW - Funktion: MMPMakeElementEntry

 Aufgabe:
   Alle Beitraege der Elemente fuer alle Phasen ermitteln
   und in das Gleichungssytem einarbeiten

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz       Erste Version
 11/1998  C. Thorenz      Erste rausgegangene Version
 08/2002  M.Kohlmeier     Iterative Kopplungsgroeßen DM -> MMP (coupling_matrix_pu)
                          für hex elements
 09/2004 OK Strain coupling matrices for quad elements 
         based on WWs MPC implementation
 12/2004 PU Coupling
**************************************************************************/
void MMPMakeElementEntry(int phasen, long element, double *ergebnis_MMP, double *rechts)
{
  /* Variablen */
  static long i, j;
  static int nn, nn2, phase;
  static double theta, dt_inverse, total_mobility, fractional_mobility, fractional_flux;
  static long *element_nodes;

  static double *conductivity_matrix;
  static double *capacitance_matrix;
  static double *cap_pressure;
  static double *gravity_vector;

  static double left_matrix[64];
  static double right_matrix[64];
  static double right_vector[8];
  static double cap_pressure_change[8];
  static double cap_pressure_change2[8];
  static double capillarity_vector[8];

  static double press_old[8], press_new[8], flux[8], zwiebel[8], zwickau[8];

 /* DM -> MMP - Kopplungsgroeßen */
  //  static double d_strain[8];
  double *mmp_strain_coupling_matrix_x = NULL;
  double *mmp_strain_coupling_matrix_y = NULL;
  double *mmp_strain_coupling_matrix_z = NULL;
  static double du_x[20];
  static double du_y[20];
  static double du_z[20];
  static double edt = 1.0 / dt;
  static double right_vector_x[9];
  static double right_vector_y[9];
  static double right_vector_z[9];

  
  // Number of quadratic element. WW
  int nd = NumbersOfElementNodeHQ(element);

  ergebnis_MMP = ergebnis_MMP;

  dt_inverse = 1. / dt;
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  nn2 = nn * nn;

  /* Matrizen, Vektoren initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(right_vector, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(zwickau, nn);
  MNulleVec(cap_pressure_change, nn);
  MNulleVec(cap_pressure_change2, nn);

  element_nodes = ElGetElementNodes(element);
  theta = GetTimeCollocationGlobal_MMP();
  //----------------------------------------------------------------------
  // Medium properties 
  long group = ElGetElementGroupNumber(element);
  CMediumProperties* m_mmp = NULL;
  m_mmp = mmp_vector[group];
  /* Druecke holen */
  char pressure_name[10];
  sprintf(pressure_name,"%s%i","PRESSURE",mmp_reference_pressure_method+1);
  for(i=0;i<nn;i++){
    //OK press_old[i] = MMPGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 0);
    press_old[i] = PCSGetNODValue(element_nodes[i],pressure_name,0);
    //OK press_new[i] = MMPGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 1);
    press_new[i] = PCSGetNODValue(element_nodes[i],pressure_name,1);
  }
  /* Eintraege fuer alle Phasen */
  for (phase = 0; phase < phasen; phase++)
    {
      /* Leitfaehigkeit */
      conductivity_matrix = MMPGetElementConductivityMatrix(element, phase);
      if (conductivity_matrix)
        {
          for (j = 0; j < nn2; j++)
            {
              left_matrix[j] += theta * conductivity_matrix[j];
              right_matrix[j] -= (1. - theta) * conductivity_matrix[j];
            }
        }

      /* Kompressibilitaet */
      capacitance_matrix = MMPGetElementCapacitanceMatrix(element, phase);
      if (capacitance_matrix)
        {
          for (j = 0; j < nn2; j++)
            {
              left_matrix[j] += capacitance_matrix[j] * dt_inverse;
              right_matrix[j] += capacitance_matrix[j] * dt_inverse;
            }
        }

      /* Kapillaritaet */
      cap_pressure = MMPGetElementCapillarityVector(element, phase);
      if (cap_pressure && conductivity_matrix)
        {
          MMultMatVec(conductivity_matrix, nn, nn, cap_pressure, nn, capillarity_vector, nn);
          for (j = 0; j < nn; j++)
            right_vector[j] += capillarity_vector[j];
        }
      //------------------------------------------------------------------
      /* Gravitation */
      gravity_vector = MMPGetElementGravityVector(element, phase);
      if (gravity_vector)
        for (j = 0; j < nn; j++)
          right_vector[j] -= gravity_vector[j];
      //------------------------------------------------------------------
      /* Kompressibilitaet aus Kapillardruecken */
      if(cap_pressure&&capacitance_matrix){
        m_mmp->mode = 1;
        for(j=0;j<nn;j++){
          cap_pressure_change[j] = dt_inverse * \
                                   (m_mmp->CapillaryPressureFunction(element_nodes[j],NULL,1.,phase,0.0) - \
                                    m_mmp->CapillaryPressureFunction(element_nodes[j],NULL,0.,phase,0.0));
                                   //OK (MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 1.) 
                                   //OK - MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 0.));
        }
        MMultMatVec(capacitance_matrix, nn, nn, cap_pressure_change, nn, cap_pressure_change2, nn);
        for (j = 0; j < nn; j++)
          right_vector[j] += cap_pressure_change2[j];
        m_mmp->mode = 0;
      }
      //------------------------------------------------------------------
    }
  //----------------------------------------------------------------------
  // RHS7: Strain coupling term: B (S^g rho^g + S^l rho^l) 1/dt ([u^s]^n+1 - [u^s]^n)
  if(GetRFProcessProcessing("SD")) { 
    int timelevel = 1;
       phase=1;
        mmp_strain_coupling_matrix_x = MMPGetElementStrainCouplingMatrixX(element,phase);
        mmp_strain_coupling_matrix_y = MMPGetElementStrainCouplingMatrixY(element,phase);
        if(problem_dimension_dm==3) //WW
            mmp_strain_coupling_matrix_z = MMPGetElementStrainCouplingMatrixZ(element,phase);
        // [u^s]^n+1 - [u^s]^n
        for (i=0;i<nd;i++){
          du_x[i] = - GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_X1",0));
          du_y[i] = - GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_Y1",0));
          if(problem_dimension_dm==3)
            du_z[i] = - GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_Z1",0));
      if(pcs_deformation%11 != 0){ //WW. if not monolithic scheme. Important
            du_x[i] += GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_X1",timelevel));
                       //DMGetNodeDisplacementX1(element_nodes[i]); 
            du_y[i] += GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_Y1",timelevel));
                       //DMGetNodeDisplacementY1(element_nodes[i]); 
            if(problem_dimension_dm==3)
               du_z[i] += GetNodeVal(element_nodes[i],PCSGetNODValueIndex("DISPLACEMENT_Z1",timelevel));
	      }
 //WW         du_x[i] *= 1e-3;
 //WW         du_y[i] *= 1e-3;
        }
        MMultMatVec(mmp_strain_coupling_matrix_x,nn,nd,du_x,nd,right_vector_x,nn);
        MMultMatVec(mmp_strain_coupling_matrix_y,nn,nd,du_y,nd,right_vector_y,nn);
        if(problem_dimension_dm==3)
           MMultMatVec(mmp_strain_coupling_matrix_z,nn,nd,du_z,nd,right_vector_z,nn);
        for (i=0;i<nn;i++) {
           right_vector[i] = -(right_vector_x[i]+right_vector_y[i])*edt; 
        if(problem_dimension_dm==3)
           right_vector[i] -= right_vector_z[i]*edt; 
        }
#ifdef TEST_CGS_MMP_CPL
        MZeigVec(du_x,nd,"du_x");
        MZeigVec(du_y,nd,"du_y");
        MZeigVec(right_vector,nn,"RHS7: B (S^g rho^g + S^l rho^l) 1/dt ([u^s]^n+1 - [u^s]^n))");
#endif
  } // DM -> MMP - Kopplungsgroeßen 


/* Ermitteln des expliziten Anteils */
  MMultMatVec(right_matrix, nn, nn, press_old, nn, zwickau, nn);

  /* Einspeichern in rechts */
  for (i = 0; i < nn; i++)
    right_vector[i] += zwickau[i];

  /* Ermitteln der Knotenfluesse. Wird fuer Kontrolle der Genauigkeit (alle Knoten) und fuer die
     Randterme der Saettigungsgleichungen (nur Randknoten) gebraucht */
  /* Ermitteln des Flusses ueber den Knoten fuer dieses Element: */
  MMultMatVec(left_matrix, nn, nn, press_new, nn, zwickau, nn);
  for (i = 0; i < nn; i++)
    flux[i] = right_vector[i] - zwickau[i];
//OK_MFP
//  double gp[3]={0.0,0.0,0.0};
  double viscosity_gp;
  CFluidProperties *m_mfp = NULL;
  double permeability_rel;
  double node_flux;
  /* Der Fluss einer einzelnen Phase ergibt sich aus Gesamtfluss und Fractional Flow */
  for (i = 0; i < nn; i++)
    {
      total_mobility = MKleinsteZahl;
      for (phase = 0; phase < phasen; phase++)
        {
          m_mfp = mfp_vector[phase];
          viscosity_gp = m_mfp->Viscosity();
          m_mmp->mode = 1;
          permeability_rel = m_mmp->PermeabilitySaturationFunction(element_nodes[i],NULL,theta,phase);
          m_mmp->mode = 0;
          //total_mobility += MMPGetNodeRelativePermeability(phase,element,element_nodes[i],theta) / viscosity_gp;
          total_mobility += permeability_rel / viscosity_gp;
        }
      for (phase = 0; phase < phasen; phase++)
        {
          m_mfp = mfp_vector[phase];
          viscosity_gp = m_mfp->Viscosity();
          m_mmp->mode = 1;
          permeability_rel = m_mmp->PermeabilitySaturationFunction(element_nodes[i],NULL,theta,phase);
          m_mmp->mode = 0;
          //fractional_mobility = MMPGetNodeRelativePermeability(phase, element, element_nodes[i], theta)
          //    / viscosity_gp
          //    / total_mobility;
          fractional_mobility = permeability_rel / viscosity_gp / total_mobility;
          /* Der Anteil der betrachteten Phase am Gesamtfluss, es wird ueber alle Elementeintraege aufaddiert */
          fractional_flux = flux[i] * fractional_mobility;
          //OK MMPSetNodeFlux(phase, element_nodes[i], 1, MMPGetNodeFlux(phase, element_nodes[i], -1, 1) - fractional_flux);
          node_flux = PCSGetNODValue(element_nodes[i],"FLUX",1);
          SetNodeVal(element_nodes[i],PCSGetNODValueIndex("FLUX",1),node_flux-fractional_flux);
        }
    }
#ifdef TESTCGS_MMP
    DisplayMsgLn("");
    DisplayMsg("Element: "); DisplayLong (element);
    MZeigMat(left_matrix,nn,nn,"MMPMakeElementEntry: Left matrix");
    MZeigMat(right_matrix,nn,nn,"MMPMakeElementEntry: Right matrix");
    MZeigVec(right_vector,nn,"MMPMakeElementEntry: RHS vector");
    MZeigVec(rechts,nn,"MMPMakeElementEntry: rechts");
#endif
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}


/**************************************************************************
 ROCKFLOW - Funktion: MMPMakeElementEntryPhase

 Aufgabe:
   Alle Beitraege der Elemente eine Phasen ermitteln
   und in das Gleichungssytem einarbeiten

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 9/97     C.Thorenz       Erste Version
 11/1998  C. Thorenz      Erste rausgegangene Version
 07/2002   OK   specified for a defined phase

**************************************************************************/
void MMPMakeElementEntryPhase(int phase, long element, double *rechts)
{
  /* Variablen */
  static long i, j;
  static int nn, nn2;
  static double theta, dt_inverse, total_mobility, fractional_mobility, fractional_flux;
  static long *element_nodes;

  static double *conductivity_matrix;
  static double *capacitance_matrix;
  static double *cap_pressure;
  static double *gravity_vector;

  static double left_matrix[64];
  static double right_matrix[64];
  static double right_vector[8];
  static double cap_pressure_change[8];
  static double cap_pressure_change2[8];
  static double capillarity_vector[8];
  static double press_old[8], press_new[8], flux[8], zwiebel[8], zwickau[8];
  static double this_left_matrix[64];
  static double this_right_matrix[64];


#ifdef TESTCGS_MMP
    DisplayMsg("Element: "); DisplayLong(element); DisplayMsgLn("");
#endif

  dt_inverse = 1. / dt;
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  nn2 = nn * nn;

  /* Matrizen, Vektoren initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(right_vector, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(zwickau, nn);
  MNulleVec(cap_pressure_change, nn);
  MNulleVec(cap_pressure_change2, nn);
  MNulleMat(this_left_matrix, nn, nn);

  element_nodes = ElGetElementNodes(element);
  theta = GetTimeCollocationGlobal_MMP();

  /* Druecke holen */
  for (i = 0; i < nn; i++) {
    press_old[i] = MMPGetNodePress(phase, element_nodes[i], element, 0);
    press_new[i] = MMPGetNodePress(phase, element_nodes[i], element, 1);
  }

#ifdef TESTCGS_MMP
  DisplayMsg("Phase: "); DisplayLong(phase); DisplayMsgLn("");
#endif

  /*--------------------------------------------------------------------------*/
  /* Leitfaehigkeit */
  conductivity_matrix = MMPGetElementConductivityMatrix(element, phase);
#ifdef TESTCGS_MMP
  MZeigMat(conductivity_matrix,nn,nn,"MMPMakeElementEntryPhase: KP^p");
#endif
  if (conductivity_matrix) {
    for (j = 0; j < nn2; j++) {
      this_left_matrix[j] = theta * conductivity_matrix[j];
      this_right_matrix[j] = -1.*(1. - theta) * conductivity_matrix[j];
      left_matrix[j] += theta * conductivity_matrix[j];
      right_matrix[j] -= (1. - theta) * conductivity_matrix[j];
    }
  }
#ifdef TESTCGS_MMP
  MZeigMat(this_left_matrix,nn,nn,"MMPMakeElementEntryPhase: theta KP^p");
  MZeigMat(this_right_matrix,nn,nn,"MMPMakeElementEntryPhase: -(1-theta) KP^p");
#endif

  /*--------------------------------------------------------------------------*/
  /* Kompressibilitaet */
  capacitance_matrix = MMPGetElementCapacitanceMatrix(element, phase);
#ifdef TESTCGS_MMP
  MZeigMat(capacitance_matrix,nn,nn,"MMPMakeElementEntryPhase: CP^p");
#endif
  if (capacitance_matrix) {
    for (j = 0; j < nn2; j++) {
      this_left_matrix[j] = capacitance_matrix[j] * dt_inverse;
      this_right_matrix[j] = capacitance_matrix[j] * dt_inverse;
      left_matrix[j] += capacitance_matrix[j] * dt_inverse;
      right_matrix[j] += capacitance_matrix[j] * dt_inverse;
    }
  }
#ifdef TESTCGS_MMP
    MZeigMat(this_left_matrix,nn,nn,"MMPMakeElementEntryPhase: 1/dt CP^p");
#endif

  /*--------------------------------------------------------------------------*/
  /* Kapillaritaet */
  cap_pressure = MMPGetElementCapillarityVector(element, phase);
#ifdef TESTCGS_MMP
  MZeigVec(cap_pressure,nn,"MMPMakeElementEntryPhase: pc");
#endif
  if (cap_pressure && conductivity_matrix) {
    MMultMatVec(conductivity_matrix, nn, nn, cap_pressure, nn, capillarity_vector, nn);
    for (j = 0; j < nn; j++) {
      right_vector[j] += capillarity_vector[j];
    }
  }
#ifdef TESTCGS_MMP
  MZeigVec(capillarity_vector,nn,"MMPMakeElementEntryPhase: RHS1 - KP^phase * pc");
#endif

  /*--------------------------------------------------------------------------*/
  /* Gravitation */
  gravity_vector = MMPGetElementGravityVector(element, phase);
#ifdef TESTCGS_MMP
  MZeigVec(gravity_vector,nn,"MMPMakeElementEntryPhase: gravity_vector");
#endif
  if (gravity_vector) {
        for (j = 0; j < nn; j++)
          right_vector[j] -= gravity_vector[j];
  }
#ifdef TESTCGS_MMP
    MZeigVec(gravity_vector,nn,"MMPMakeElementEntryPhase: RHS2 - gravity_vector");
#endif

  /*--------------------------------------------------------------------------*/
  /* Kompressibilitaet aus Kapillardruecken */
  if (cap_pressure && capacitance_matrix) {
    for (j = 0; j < nn; j++)
      cap_pressure_change[j] = dt_inverse * (MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 1.) - MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 0.));
#ifdef TESTCGS_MMP
  MZeigVec(cap_pressure_change2,nn,"MMPMakeElementEntryPhase: cap_pressure_change2");
#endif
    MMultMatVec(capacitance_matrix, nn, nn, cap_pressure_change, nn, cap_pressure_change2, nn);
    for (j = 0; j < nn; j++) {
      right_vector[j] += cap_pressure_change2[j];
    }
  }
#ifdef TESTCGS_MMP
  MZeigVec(cap_pressure_change2,nn,"MMPMakeElementEntryPhase: RHS3 - 1/dt CP^phase dpc");
#endif

  /*--------------------------------------------------------------------------*/
  /* Ermitteln des expliziten Anteils */
  MMultMatVec(right_matrix, nn, nn, press_old, nn, zwickau, nn);
#ifdef TESTCGS_MMP
  MZeigVec(zwickau,nn,"MMPMakeElementEntryPhase: RHS4 - RHM * p^n");
#endif

  /*--------------------------------------------------------------------------*/
  /* Einspeichern in rechts */
  for (i = 0; i < nn; i++)
    right_vector[i] += zwickau[i];


  /*--------------------------------------------------------------------------*/
  /* Ermitteln der Knotenfluesse. Wird fuer Kontrolle der Genauigkeit (alle Knoten) und fuer die
     Randterme der Saettigungsgleichungen (nur Randknoten) gebraucht */
  /* Ermitteln des Flusses ueber den Knoten fuer dieses Element: */
  MMultMatVec(left_matrix, nn, nn, press_new, nn, zwickau, nn);
  for (i = 0; i < nn; i++)
    flux[i] = right_vector[i] - zwickau[i];

  /* Der Fluss einer einzelnen Phase ergibt sich aus Gesamtfluss und Fractional Flow */
  for (i = 0; i < nn; i++) {
    total_mobility = MKleinsteZahl;
    for (phase = 0; phase <GetRFProcessNumPhases(); phase++) {
          total_mobility += MMPGetNodeRelativePermeability(phase, element, element_nodes[i], theta)
              / mfp_vector[phase]->Viscosity();
        }
    for (phase = 0; phase < GetRFProcessNumPhases(); phase++) {
          fractional_mobility = MMPGetNodeRelativePermeability(phase, element, element_nodes[i], theta)
              / mfp_vector[phase]->Viscosity()/ total_mobility;
          /* Der Anteil der betrachteten Phase am Gesamtfluss, es wird ueber alle Elementeintraege aufaddiert */
          fractional_flux = flux[i] * fractional_mobility;
          MMPSetNodeFlux(phase, element_nodes[i], 1, MMPGetNodeFlux(phase, element_nodes[i], -1, 1) - fractional_flux);
        }
  }

#ifdef TESTCGS_MMP
    DisplayMsgLn("");
    DisplayMsg("Element: "); DisplayLong (element);
    MZeigMat(left_matrix,nn,nn,"MMPMakeElementEntryPhase: Left matrix");
    MZeigMat(right_matrix,nn,nn,"MMPMakeElementEntryPhase: Right matrix");
    MZeigVec(right_vector,nn,"MMPMakeElementEntryPhase: RHS vector");
    MZeigMat(this_left_matrix,nn,nn,"MMPMakeElementEntryPhase: This left matrix");
#endif

  /*--------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
#ifdef TESTCGS_MMP
    MZeigVec(rechts,nn,"MMPMakeElementEntryPhase: rechts");
#endif
}




/**************************************************************************
 ROCKFLOW - Funktion: MMPMakeElementEntryPressureExtract

 Aufgabe:
   Alle Beitraege der Elemente fuer alle Phasen ermitteln
   und in das Gleichungssytem einarbeiten. Andere Formulierung, bei
   der der Druck des alten Zeitschritts aus dem GLS extrahiert wurde.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:

 01/2001    C.Thorenz       Erste Version
 08/2002    M.Kohlmeier     Kopplungsgroeßen DM -> MMP (coupling_matrix_pu)
**************************************************************************/
void MMPMakeElementEntryPressureExtract(int phasen, long element, double *ergebnis_MMP, double *rechts)
{
  /* Variablen */
  static long i, j;
  static int nn, nn2, phase;
  static double theta, dt_inverse, total_mobility, fractional_mobility, fractional_flux;
  static long *element_nodes;

  static double *conductivity_matrix;
  static double *capacitance_matrix;
  static double *cap_pressure;
  static double *gravity_vector;

  static double left_matrix[64];
  static double right_matrix[64];
  static double right_vector[8];
  static double cap_pressure_change[8];
  static double cap_pressure_change2[8];
  static double capillarity_vector[8];

  static double press_old[8], press_new[8], delta_p[8], flux[8], zwiebel[8], zwickau[8];

  ergebnis_MMP = ergebnis_MMP;

  dt_inverse = 1. / dt;
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  nn2 = nn * nn;

  /* Matrizen, Vektoren initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(right_vector, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(zwickau, nn);
  MNulleVec(cap_pressure_change, nn);
  MNulleVec(cap_pressure_change2, nn);

  element_nodes = ElGetElementNodes(element);
  theta = GetTimeCollocationGlobal_MMP();

  /* Druecke holen */
  for (i = 0; i < nn; i++)
    {
      press_old[i] = MMPGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 0);
      press_new[i] = MMPGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 1);
      delta_p[i] = press_new[i] - press_old[i];
    }

  /* Eintraege fuer alle Phasen */
  for (phase = 0; phase < phasen; phase++)
    {
      /* Leitfaehigkeit: Integral(grad(phi)*(kr1*K/mue1*grad(p1)))dV  */
      conductivity_matrix = MMPGetElementConductivityMatrix(element, phase);
      if (conductivity_matrix)
        {
          for (j = 0; j < nn2; j++)
            {
              left_matrix[j] += theta * conductivity_matrix[j];
              right_matrix[j] -= conductivity_matrix[j];
            }
        }
      /* Kompressibilitaet: Integral(phi*(1/(c0+c1*p1)*n*S1*c1*dp1/dt))dV */
      capacitance_matrix = MMPGetElementCapacitanceMatrix(element, phase);
      if (capacitance_matrix)
        for (j = 0; j < nn2; j++)
          left_matrix[j] += capacitance_matrix[j] * dt_inverse;

      /* Kapillaritaet: Integral(grad(phi)*(kr1*K/mue1*grad(p_cap)))dV  */
      cap_pressure = MMPGetElementCapillarityVector(element, phase);
      if (cap_pressure && conductivity_matrix)
        {
          MMultMatVec(conductivity_matrix, nn, nn, cap_pressure, nn, capillarity_vector, nn);
          for (j = 0; j < nn; j++)
            right_vector[j] += capillarity_vector[j];
        }

      /* Gravitation: Integral(grad(phi)*(kr*K/mue*rho*g))dV */
      gravity_vector = MMPGetElementGravityVector(element, phase);
      if (gravity_vector)
        for (j = 0; j < nn; j++)
          right_vector[j] -= gravity_vector[j];

      /* Kompressibilitaet aus Kapillardruecken */
      if (cap_pressure && capacitance_matrix)
        {
          for (j = 0; j < nn; j++)
            cap_pressure_change[j] = dt_inverse * (MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 1.) - MMPGetNodeDifferenceToReferencePressure(phase, element, element_nodes[j], 0.));
          MMultMatVec(capacitance_matrix, nn, nn, cap_pressure_change, nn, cap_pressure_change2, nn);
          for (j = 0; j < nn; j++)
            right_vector[j] += cap_pressure_change2[j];
        }
    }

    /* Ermitteln des expliziten Anteils */
  MMultMatVec(right_matrix, nn, nn, press_old, nn, zwickau, nn);

  /* Einspeichern in rechts */
  for (i = 0; i < nn; i++)
    right_vector[i] += zwickau[i];

  /* Ermitteln der Knotenfluesse. Wird fuer Kontrolle der Genauigkeit 
     (alle Knoten) und fuer die Randterme der Saettigungsgleichungen
     (nur Randknoten) gebraucht, geht bei Extraktion nicht !?! */

  /* Ermitteln des Flusses ueber den Knoten fuer dieses Element: */
  MMultMatVec(left_matrix, nn, nn, delta_p, nn, zwickau, nn);
  for (i = 0; i < nn; i++)
    flux[i] = right_vector[i] - zwickau[i];

  /* Der Fluss einer einzelnen Phase ergibt sich aus Gesamtfluss und Fractional Flow */
  for (i = 0; i < nn; i++)
    {
      total_mobility = MKleinsteZahl;
      for (phase = 0; phase < phasen; phase++)
        {
          total_mobility += MMPGetNodeRelativePermeability(phase, element, element_nodes[i], theta)
              / mfp_vector[phase]->Viscosity();
        }
      for (phase = 0; phase < phasen; phase++)
        {
          fractional_mobility = MMPGetNodeRelativePermeability(phase, element, element_nodes[i], theta)
              / mfp_vector[phase]->Viscosity()/ total_mobility;
          /* Der Anteil der betrachteten Phase am Gesamtfluss, es wird ueber alle Elementeintraege aufaddiert */
          fractional_flux = flux[i] * fractional_mobility;
          MMPSetNodeFlux(phase, element_nodes[i], 1, MMPGetNodeFlux(phase, element_nodes[i], -1, 1) - fractional_flux);
        }
    }
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}

/**************************************************************************
   ROCKFLOW - Funktion: MMPMakeMassMatrixEntry

   Aufgabe:
   Gleichungssystemeintrag fuer die Massenmatrix eines Elements
   (Saettigungsaenderung in Richards-Celia-Ansatz)

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   12/1999  C.Thorenz     Erste Version

 **************************************************************************/
void MMPMakeMassMatrixEntry(int phase, long element, double *rechts)
{
  static double *mass, edt;
  static long nn, *element_nodes;
  static int i;
  static double left_matrix[64];
  static double zwickau[8];
  static double zwiebel[8];
  static double right_vector[8];

  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;

  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);

  /* Initialisieren */
  MNulleVec(zwickau, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(right_vector, nn);

  /* Linke Seite aus Massenmatrix */
  mass = MMPGetElementSaturationMassMatrix(element);

  if (mass)
    {
      /* dS/dt  */
      for (i = 0; i < nn; i++)
        zwiebel[i] = edt * (MMPGetNodeSatu(phase, element_nodes[i], element, 1) - MMPGetNodeSatu(phase, element_nodes[i], element, 0));

      /* Rechte Seite (Vektor) aus Massenmatrix */
      MMultMatVec(mass, nn, nn, zwiebel, nn, zwickau, nn);
      for (i = 0; i < nn; i++)
        right_vector[i] -= zwickau[i];
    }
  MNulleMat(left_matrix, nn, nn);

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}





/**************************************************************************
   ROCKFLOW - Funktion: MMPCalcNotSaturatedPatchVolume

   Aufgabe:
   Ermittelt das Volumen des nicht geaettigten Porenraums

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   12/2000  C.Thorenz     Erste Version

 **************************************************************************/
double MMPCalcNotSaturatedPatchVolume(int phase, long element, long node)
{
  double *mass, sum = 0.;
  long nn, *element_nodes;
  int i, node_index;
  static double zwickau[8];
  static double zwiebel[8];

  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);
  for (node_index = 0; node_index < nn; node_index++)
    if (element_nodes[node_index] == node)
      break;

  MNulleVec(zwickau, nn);
  MNulleVec(zwiebel, nn);

  /* Linke Seite aus Massenmatrix */
  mass = MMPGetElementSaturationMassMatrix(element);

  zwiebel[node_index] = 1. - MMPGetNodeSatu(phase, node, element, 0);

  MMultMatVec(mass, nn, nn, zwiebel, nn, zwickau, nn);

  for (i = 0; i < nn; i++)
    sum += zwickau[i];

  return sum;
}


double MMPCalcPatchVolume(long number)
{
  long *elemente;
  long i;
  int anzahl1d, anzahl2d, anzahl3d;
  double volume = 0.;
  double theta = 1.; //OK mmp_numerics->time_collocation;
  elemente = GetNode1DElems(number, &anzahl1d);
  for (i = 0l; i < anzahl1d; i++)
    volume += 0.5 * ElGetElementVolume(elemente[i]) * mmp_vector[ElGetElementGroupNumber(i)]->Porosity(elemente[i],NULL,theta);
  elemente = GetNode2DElems(number, &anzahl2d);
  for (i = 0l; i < anzahl2d; i++)
    volume += 0.25 * ElGetElementVolume(elemente[i]) * mmp_vector[ElGetElementGroupNumber(i)]->Porosity(elemente[i],NULL,theta);
  elemente = GetNode3DElems(number, &anzahl3d);
  for (i = 0l; i < anzahl3d; i++)
    volume += 0.125 * ElGetElementVolume(elemente[i]) * mmp_vector[ElGetElementGroupNumber(i)]->Porosity(elemente[i],NULL,theta);
  return volume;
}



/**************************************************************************
   ROCKFLOW - Funktion: MMPIncorporateFluidExchange

   Aufgabe:
     Baut die aus dem Mehrfachkontinuumansatz resultierenden 
     Fluesse ein.

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   6/2001  C.Thorenz     Erste Version
**************************************************************************/
void MMPIncorporateFluidExchange(double *rechts)
{
rechts = rechts;
/*
  long *elemente;
  long i, j, node;
  int anzahl1d, anzahl2d, anzahl3d;
  double p, p_up, p_do;
  double q, Q_up, Q_do;
  double R, R_up, R_do;
  double c_up, c_do;
  double c_up2, c_do2;

  for (i = 0l; i < NodeListLength; i++)
    {
      node = NodeNumber[i];
      // Hoeherwertiges Kontinuum 
      p_up = (1.-GetTimeCollocationGlobal_MMP())*MMPGetNodePressHigherOrderContinuum(0, node, -1, 0)+GetTimeCollocationGlobal_MMP()*MMPGetNodePressHigherOrderContinuum(0, node, -1, 1);
      // Aktuelles Kontinuum
      p    = (1.-GetTimeCollocationGlobal_MMP())*MMPGetNodePress(0, node, -1, 0)+GetTimeCollocationGlobal_MMP()*MMPGetNodePress(0, node, -1, 1);
      // Niederwertiges Kontinuum
      p_do = (1.-GetTimeCollocationGlobal_MMP())*MMPGetNodePressLowerOrderContinuum(0, node, -1, 0)+GetTimeCollocationGlobal_MMP()*MMPGetNodePressLowerOrderContinuum(0, node, -1, 1);
      elemente = GetNode1DElems(node, &anzahl1d);
      for (j = 0l; j < anzahl1d; j++) {
        q = GetSoilFluidContinuumExchange(elemente[j], p_up, p, p_do, &Q_up, &R_up, &c_up, &c_up2, &Q_do, &R_do, &c_do, &c_do2);
        R = p_up * c_up2 + p_do * c_do2 + R_up + R_do; 
        MXInc(i,i, -0.5 * ElGetElementVolume(elemente[j]) * (c_up + c_do));  
        rechts[i] += 0.5 * ElGetElementVolume(elemente[j]) * R;
      }
      elemente = GetNode2DElems(node, &anzahl2d);
      for (j = 0l; j < anzahl2d; j++) {
        q = GetSoilFluidContinuumExchange(elemente[j], p_up, p, p_do, &Q_up, &R_up, &c_up, &c_up2, &Q_do, &R_do, &c_do, &c_do2);
        R = p_up * c_up2 + p_do * c_do2 + R_up + R_do; 
        MXInc(i,i, -0.25 * ElGetElementVolume(elemente[j]) * (c_up + c_do));  
        rechts[i] += 0.25 * ElGetElementVolume(elemente[j]) * R;
      }
      elemente = GetNode3DElems(node, &anzahl3d);
      for (j = 0l; j < anzahl3d; j++) {
        q = GetSoilFluidContinuumExchange(elemente[j], p_up, p, p_do, &Q_up, &R_up, &c_up, &c_up2, &Q_do, &R_do, &c_do, &c_do2);
        R = p_up * c_up2 + p_do * c_do2 + R_up + R_do; 
        MXInc(i,i, -0.125 * ElGetElementVolume(elemente[j]) * (c_up + c_do));  
        rechts[i] += 0.125 * ElGetElementVolume(elemente[j]) * R;
      }
    }
*/
}

/*************************************************************************
 ROCKFLOW - Funktion MMPCalcPhasePressure
 Aufgabe:
   Berechnet alle Knotendruecke neu, bei Knoten die an mehreren
   Elementen angrenzem wird ueber die Elemente gemittelt.
   Problematisch an Materialgrenzen!!!
 Programmaenderungen:
   02/1999   CT   Erste Version
   05/2001   OK   Verallgemeinerung von MMTM0699CalcPhasePressure
*************************************************************************/
void MMPCalcPhasePressure(int timelevel)
{
  double p_cap_sum, p, p_cap;
  long *elemente, i, number;
  int anzahl1d, anzahl2d, anzahl3d, j, activ_phase;
  double gp[3]={0.,0.,0.};
  CMediumProperties* m_mmp = NULL;

  for (activ_phase = 1; activ_phase < GetRFProcessNumPhases(); activ_phase++)
    {
      for (i = 0; i < NodeListLength; i++)
        {
          number = NodeNumber[i];
          p = MMPGetNodePress(0,number,9999,timelevel);
          p_cap_sum = 0.;

          elemente = GetNode1DElems(number, &anzahl1d);
          for (j = 0l; j < anzahl1d; j++)
            {
              //OK p_cap = MATCalcNodeCapillaryPressure(activ_phase, elemente[j], number, (double) timelevel);
              m_mmp->mode = 1;
              p_cap = m_mmp->CapillaryPressureFunction(number,gp,(double)timelevel,activ_phase,0.0);
              p_cap_sum += p_cap;
            }
          elemente = GetNode2DElems(number, &anzahl2d);
          for (j = 0l; j < anzahl2d; j++)
            {
              //OK p_cap = MATCalcNodeCapillaryPressure(activ_phase, elemente[j], number, (double) timelevel);
              m_mmp->mode = 1;
              p_cap = m_mmp->CapillaryPressureFunction(number,gp,(double)timelevel,activ_phase,0.0);
              p_cap_sum += p_cap;
            }
          elemente = GetNode3DElems(number, &anzahl3d);
          for (j = 0l; j < anzahl3d; j++)
            {
              //OK p_cap = MATCalcNodeCapillaryPressure(activ_phase, elemente[j], number, (double) timelevel);
              m_mmp->mode = 1;
              p_cap = m_mmp->CapillaryPressureFunction(number,gp,(double)timelevel,activ_phase,0.0);
              p_cap_sum += p_cap;
            }

          p_cap_sum /= (double) (anzahl1d + anzahl2d + anzahl3d);
          MMPSetNodePress(activ_phase,number,timelevel,p - p_cap_sum);
        }
    }
}


/**************************************************************************
 ROCKFLOW - Funktion: MMPCalcSaturationPressureDependency

 Aufgabe:

 Liefert Saettigungs-Druckbeziehung fuer Richardsmodell

  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E: phase         Betrachtete phase
  E: index         Elementnummer
  E: r,s,t         Lokale Koordinaten
  E: theta         Zeitwichtung

  Ergebnis:
  Saettigung

  Programmaenderungen:
  08/1999   CT   Erste Version (MMTM0699GetSaturationPressureDependency)
  05/2001   OK   Verallgemeinerung

**************************************************************************/
double MMPCalcSaturationPressureDependency(int phase, long index, double r, double s, double t, double theta)
{
  static double satu0, satu1, dS_dp, ds, satu[16], kap_press1[16], kap_press2[16];
  int phasen;
  static double pres_old, pres_new;
  static double water_pressure,capillary_pressure;

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];


  /* Nur fuer Druck-Formulierung der Richards-Methode! */
/*OK
  if ((GetMethod_MMS() != 7))
    return 0.;
*/

  phasen = 2;

    /* Achtung! Das RichardsModell ist ein fieser Hybrid; obwohl nur eine Phase
       vorhanden ist, wird auf die Materialparameter des Mehrphasensystems
       zugegriffen. Also virtuell die anderen Phasensaettigungen ausrechnen ... */
/* OK/WW rf3812
    pres_old = InterpolValue(index, GetNodeIndexForPhasePressure0010(phase, 0), r, s, t);
    pres_new = InterpolValue(index, GetNodeIndexForPhasePressure0010(phase, 1), r, s, t);
*/
    pres_old = InterpolValue(index, MMPGetPhasePressureNodeIndex(phase, 0), r, s, t);
    pres_new = InterpolValue(index, MMPGetPhasePressureNodeIndex(phase, 1), r, s, t);

    water_pressure = (1. - theta) * pres_old + theta * pres_new;
    capillary_pressure = - water_pressure;
    //OK satu1 = CECalcSatuFromCap_AMM(index,capillary_pressure);

  /* Gassaettigung, wird eigentlich nicht gebraucht. */
  satu0 = 1. - satu1;

  /* Vollsaettigung */
  if (satu0 < MKleinsteZahl)
    return 0.;

  satu[0] = satu0;
  satu[1] = satu1;
//SB:m  CECalcCap_AMM(index, phasen, satu, kap_press1);
  kap_press1[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
  kap_press1[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);

  /* Vollsaettigung? */
  if (kap_press1[1] < MKleinsteZahl)
    return 0.;

  ds = 1.e-2;

  /* Wenn wir nah an der Vollsaettigung, ggf. Schrittweite verkleinern */
  do
    {
      ds /= 10.;
      satu[1] = satu1 - ds;
//SB:m      CECalcCap_AMM(index, phasen, satu, kap_press1);
		kap_press1[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
		kap_press1[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);

      satu[1] = satu1 + ds;
//SB:m      CECalcCap_AMM(index, phasen, satu, kap_press2);
	  kap_press2[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
	  kap_press2[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);
    }
  while ((ds > MKleinsteZahl) && (kap_press2[1] < MKleinsteZahl / 100.));


  if (((kap_press1[1] > MKleinsteZahl) || (kap_press2[1] > MKleinsteZahl)) && (kap_press1[1] - kap_press2[1] > MKleinsteZahl))
    dS_dp = 2. * ds / (kap_press1[1] - kap_press2[1]);
  else
    dS_dp = 0.;

  return dS_dp;
}

double MATCalcSaturationPressureDependency(int phase, long index, double r, double s, double t, double theta)
{
  static double satu0, satu1, dS_dp, ds, satu[16], kap_press1[16], kap_press2[16];
  int phasen;
  static double pres_old, pres_new;
  static double water_pressure,capillary_pressure;
  char name[80];

  CMediumProperties *m_mmp = NULL;
  long group = ElGetElementGroupNumber(index);
  m_mmp = mmp_vector[group];
  phasen = 2;

  sprintf(name,"PRESSURE%i",phase+1);

  pres_old = InterpolValue(index,PCSGetNODValueIndex(name,0), r, s, t);
  pres_new = InterpolValue(index,PCSGetNODValueIndex(name,1), r, s, t);

  water_pressure = (1. - theta) * pres_old + theta * pres_new;
  capillary_pressure = - water_pressure;
  //OK satu1 = CECalcSatuFromCap_AMM(index,capillary_pressure);

  /* Gassaettigung, wird eigentlich nicht gebraucht. */
  satu0 = 1. - satu1;

  /* Vollsaettigung */
  if (satu0 < MKleinsteZahl)
    return 0.;

  satu[0] = satu0;
  satu[1] = satu1;
//SB:m  CECalcCap_AMM(index, phasen, satu, kap_press1);
  kap_press1[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
  kap_press1[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);

  /* Vollsaettigung? */
  if (kap_press1[1] < MKleinsteZahl)
    return 0.;

  ds = 1.e-2;

  /* Wenn wir nah an der Vollsaettigung, ggf. Schrittweite verkleinern */
  do
    {
      ds /= 10.;
      satu[1] = satu1 - ds;
//SB:m      CECalcCap_AMM(index, phasen, satu, kap_press1);
		kap_press1[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
		kap_press1[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);

		satu[1] = satu1 + ds;
//SB:m      CECalcCap_AMM(index, phasen, satu, kap_press2);
		kap_press2[0] = m_mmp->CapillaryPressureFunction(index,NULL,theta,0,0.0);
		kap_press2[1] = m_mmp->CapillaryPressureFunction(index,NULL,theta,1,0.0);

    }
  while ((ds > MKleinsteZahl) && (kap_press2[1] < MKleinsteZahl / 100.));


  if (((kap_press1[1] > MKleinsteZahl) || (kap_press2[1] > MKleinsteZahl)) && (kap_press1[1] - kap_press2[1] > MKleinsteZahl))
    dS_dp = 2. * ds / (kap_press1[1] - kap_press2[1]);
  else
    dS_dp = 0.;

  return dS_dp;
}

/*************************************************************************
ROCKFLOW - Function: MMPCalcSecondaryVariables
Task: Calc secondary variables for MMP
Programming: 
05/2003 OK Implementation
09/2004 OK Density
09/2004 OK Viscosity
last modified:
**************************************************************************/
void MMPCalcSecondaryVariables(void)
{
  int phase,timelevel,index;
  long i;
  double theta;
  double p_cap,p_liquid,p_gas;
  double s_liquid,s_gas;
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet("TWO_PHASE_FLOW");
  if(!m_pcs){
    cout << "MMPCalcSecondaryVariables: no PCS data" << endl;
    return;
  }
  /*---------------------------------------------------------------------*/
  /* 5.1 Anfangsbedingungen mit vorhandenen Randbedingungen ueberschreiben */
  phase=0;
  timelevel=0;
//OK_BC
  NodeSetBoundaryConditions("PRESSURE1",\
                            PCSGetNODValueIndex("PRESSURE1",timelevel),\
                            m_pcs->pcs_type_name);
  NodeSetBoundaryConditions("SATURATION2",\
                            PCSGetNODValueIndex("SATURATION2",timelevel),\
                            m_pcs->pcs_type_name);
  timelevel=1;
  NodeSetBoundaryConditions("PRESSURE1",\
                            PCSGetNODValueIndex("PRESSURE1",timelevel),\
                            m_pcs->pcs_type_name);
  NodeSetBoundaryConditions("SATURATION2",\
                            PCSGetNODValueIndex("SATURATION2",timelevel),\
                            m_pcs->pcs_type_name);

  /*---------------------------------------------------------------------*/
  /* 5.2 Anfangsbedingungen fuer zweite Phase berechnen */
   /* Calc secondary variable saturation S^g=1-S^l */
  timelevel=0; phase=1; index=0; theta=0.0;
  for (i=0;i<NodeListLength;i++) {
    s_liquid = GetNodeVal(i,PCSGetNODValueIndex("SATURATION2",timelevel));
    s_gas = MRange(0.0,1.0-s_liquid,1.0);
    SetNodeVal(i,PCSGetNODValueIndex("SATURATION1",timelevel),s_gas);
  }
  CopyNodeVals(PCSGetNODValueIndex("SATURATION1",0),\
               PCSGetNODValueIndex("SATURATION1",1));
  /*---------------------------------------------------------------------*/
  /* 5.3.1 Capillary pressure - p_c (S) */
  timelevel=0; phase=1; index=0; theta=0.0;
  CMediumProperties *m_mmp = NULL;
  for (i=0;i<NodeListLength;i++) {
    m_mmp = mmp_vector[0];
    m_mmp->mode = 1;
    p_cap = 0.0; //m_mmp->CapillaryPressure(i,NULL,theta,phase); //MATCalcNodeCapillaryPressure(phase,index,i,theta);
    m_mmp->mode = 0;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
  }
  CopyNodeVals(PCSGetNODValueIndex("PRESSURE_CAP",0),\
               PCSGetNODValueIndex("PRESSURE_CAP",1));
  /*---------------------------------------------------------------------*/
  /* 5.3.2 Phasendruck fuer 2. Phase: p^l = p^g - p_c(S)*/
  timelevel=0; index=0; theta=0.0;
  for (i=0;i<NodeListLength;i++) {
    phase=0;
    p_gas = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE1",timelevel));
    p_cap = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel));
    p_liquid = p_gas - p_cap;
    phase=1;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel),p_liquid);
  }
  timelevel=1; phase=1;
  CopyNodeVals(PCSGetNODValueIndex("PRESSURE2",0),\
               PCSGetNODValueIndex("PRESSURE2",1));
  //----------------------------------------------------------------------
  // Fluid properties
  double density;
  double viscosity;
  timelevel=1;
  CFluidProperties* m_mfp = NULL;
  //......................................................................
  phase=0;
  m_mfp = mfp_vector[phase];
  m_mfp->mode = 1;
  for (i=0;i<NodeListLength;i++) {
    density = m_mfp->Density();
    SetNodeVal(i,PCSGetNODValueIndex("DENSITY1",timelevel),density);
    viscosity = m_mfp->Viscosity();
    SetNodeVal(i,PCSGetNODValueIndex("VISCOSITY1",timelevel),viscosity);
  }
  m_mfp->mode = 0;
  //......................................................................
  phase=1;
  m_mfp = mfp_vector[phase];
  m_mfp->mode = 1;
  for (i=0;i<NodeListLength;i++) {
    density = m_mfp->Density();
    SetNodeVal(i,PCSGetNODValueIndex("DENSITY2",timelevel),density);
    viscosity = m_mfp->Viscosity();
    SetNodeVal(i,PCSGetNODValueIndex("VISCOSITY2",timelevel),viscosity);
  }
  m_mfp->mode = 0;
  //----------------------------------------------------------------------
  //OK InitializeKernel_MMS();  
  //----------------------------------------------------------------------
  CalcElementsGeometry();
  int no_processes = (int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare("TWO_PHASE_FLOW")==0)
      m_pcs->CalculateElementMatrices();
  }
}

/*************************************************************************
GeoSys-FEM Function:
Task: 
Programming: 
02/2005 OK/YD Implementation
05/2005 WW  Generalization for all element types
05/2005 OK MSH
08/2005 WW Changes for new mesh
last modified:
**************************************************************************/
//using Mesh_Group::CFEMesh;
//using Mesh_Group::CElem;
//using Mesh_Group::CNode;
using FiniteElement::CFiniteElementStd;
void CRFProcess::MMPCalcSecondaryVariablesRichards(int timelevel, bool update) //MB
//void MMPCalcSecondaryVariablesRichards(int timelevel, bool update)
{
  int j, EleType;
  long i, enode;
  long group;
  double p_cap;
  double saturation,saturation_sum = 0.0;
  double GP[3];
  static double Node_Cap[8];
  int idxp,idxcp,idxS;
  CMediumProperties* m_mmp = NULL;
  CElem* elem =NULL;
  //----------------------------------------------------------------------
  // PCS
  /*CRFProcess*pcs = NULL;
  CRFProcess*m_pcs_mmp = NULL;
  for(j=0;j<(int)pcs_vector.size();j++){
    pcs = pcs_vector[j];
    if(pcs->pcs_type_name.find("RICHARDS")!=string::npos)
      m_pcs_mmp = pcs;
  }*/
  

  //CFEMesh* m_msh = NULL; //MB
  //CFEMesh* m_msh = m_pcs_mmp->m_msh; 
  ///CFiniteElementStd* fem = m_pcs_mmp->GetAssembler();
  GetAssembler();
  //----------------------------------------------------------------------
  //WW
  idxp  = GetNodeValueIndex("PRESSURE1") + timelevel;
  idxcp = GetNodeValueIndex("PRESSURE_CAP") + timelevel;
  idxS  = GetNodeValueIndex("SATURATION1") + timelevel;
  //WW
  //----------------------------------------------------------------------
  // Capillary pressure
  for(i=0;i<(long)m_msh->GetNodesNumber(false);i++){
     p_cap = -GetNodeValue(i,idxp);
     if(timelevel==1&&update)  SetNodeValue(i,idxcp-1,GetNodeValue(i,idxcp));
     SetNodeValue(i,idxcp,p_cap);
	  if(timelevel==1&&update) SetNodeValue(i,idxS-1,GetNodeValue(i,idxS));
  }
  //----------------------------------------------------------------------
  // Liquid saturation
  //WW
  for (i = 0; i < m_msh->GetNodesNumber(false); i++)
      SetNodeValue(i,idxS, 0.0);
  // 
  for (i = 0; i < (long)m_msh->ele_vector.size(); i++)  
  {
     elem = m_msh->ele_vector[i];

     //if (elem->GetMark())     // for initial Conditions the complete ElementList !
                                // umständlich !!! nach jeder it. für jeden Richards Spalte !!
     //{
          // Activated Element 
          group = elem->GetPatchIndex();
          m_mmp = mmp_vector[group];
          m_mmp->m_pcs=this;  //m_pcs_mmp
          EleType = elem->GetElementType();
          if(EleType==4) // Traingle
          {
             GP[0] = GP[1] = 0.1/0.3; 
             GP[2] = 0.0;
          }
          else if(EleType==5) 
		     GP[0] = GP[1] = GP[2] = 0.25;
          else
		     GP[0] = GP[1] = GP[2] = 0.0;  

          fem->ConfigElement(elem);
		  fem->setUnitCoordinates(GP);
          fem->ComputeShapefct(1); // Linear
		  for(j=0; j<elem->GetVertexNumber(); j++)
		  {
             enode = elem->GetNodeIndex(j);
             Node_Cap[j] =  GetNodeValue(enode,idxcp); //m_pcs_mmp
		  }
		  p_cap = fem->interpolate(Node_Cap);
          saturation = m_mmp->SaturationCapillaryPressureFunction(p_cap,(int)mfp_vector.size()-1);  //YD
		  for(j=0; j<elem->GetVertexNumber(); j++)
          {
			  enode = elem->GetNodeIndex(j);
              saturation_sum = GetNodeValue(enode, idxS);
			  SetNodeValue(enode,idxS, saturation_sum+saturation);
          }
      //}
  }
  // Average 
  for (i = 0; i <(long)m_msh->GetNodesNumber(false); i++) {       
    //if (elem->GetMark()){     // Element selected	  
	  saturation_sum = GetNodeValue(i, idxS);
      p_cap = m_msh->nod_vector[i]->connected_elements.size();
      if(p_cap==0) p_cap =1;
	  saturation_sum /= (double)p_cap;
      SetNodeValue(i,idxS, saturation_sum);
    //}
  }
}

/*************************************************************************
GeoSys-FEM Function:
Task: 
Programming: 
02/2005 OK/YD Implementation
08/2005 WW 
last modified:
**************************************************************************/
void CRFProcess::MMPSetICRichards(int dummy)
{
  dummy=dummy;
  int timelevel;

  int idxp0 = GetNodeValueIndex("PRESSURE1");
  int idxcp0 = GetNodeValueIndex("PRESSURE_CAP");

  int idxp1 = idxp0+1;
  int idxcp1 = idxcp0+1;

  //----------------------------------------------------------------------
  // Capillary pressure
  double p_cap;
  for(int i=0;i<m_msh->GetNodesNumber(false);i++){
    p_cap = -GetNodeValue(i,idxp0);
    SetNodeValue(i,idxcp0,p_cap);
    p_cap = -GetNodeValue(i,idxp1);
    SetNodeValue(i,idxcp1,p_cap);

  }

  timelevel = 0;
  // Node valus of Both levels are needed by the following line
  MMPCalcSecondaryVariablesRichards(timelevel, false);
  timelevel = 1;
  MMPCalcSecondaryVariablesRichards(timelevel, false);
}


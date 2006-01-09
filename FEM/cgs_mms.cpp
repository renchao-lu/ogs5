/**************************************************************************
 ROCKFLOW - Modul: cgs_mms.c

 Aufgabe:
 Aufstellen des Gesamtgleichungssystems und Berechnen der
 Elementmatrizen etc. fuer das Modul MMS.

 Programmaenderungen:

  9/97     C.Thorenz       Erste Version
 11/1998   C.Thorenz       Erste rausgegangene Version
  5/1999   C.Thorenz       Multiphasen
 11/1999   RK              EliminateIrrNodes()
 12/1999   C.Thorenz       Irr. Knoten _vor_ Dirichlet-BC
  2/2000   C.Thorenz       Explicit handling of boundary fluxes, reduces oscillations during iteration
 09/2001   OK              MakeGS_MMSV1
  7/2002   C.Thorenz       Zugriff auf GetNodeElements statt GetNode1DElems, GetNode2DElems, ...
 03/2003   RK              Quellcode bereinigt, Globalvariablen entfernt
   

**************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"
#include "cel_mmp.h"

#define noTESTCGS_MMS

/* Header / Andere intern benutzte Module */
#include "int_mms.h"
#include "elements.h"
#include "nodes.h"
#include "mathlib.h"
#include "matrix.h"
#include "edges.h"
#include "plains.h"
#include "femlib.h"
#include "adaptiv.h"
#include "cvel.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_tim_new.h"
#include "loop_pcs.h"
#include "rf_pcs.h"

/* Interne (statische) Deklarationen */
void MMSInitializeSystemOfEquations(double *);
void MMSAssembleFluxMatrices(int, double *);
void MMSAssembleMassMatrices(int, double *);
void MMSTreatOpenBoundarys(int, double *);
void MMSIncorporateSourceBoundaryConditions(int, double *, char *, char *);
double MMSGetSourceMultiplicator(long i);
void MMSIncorporateDirichletBoundaryCondition(int, double *, char *);
double MMSBoundaryConditionsFunction(long knoten, int *gueltig, double val);
double MMSPcapBoundaryConditionsFunction(long node, int *gueltig, double val);

void MMSMakeMassMatrixEntry(int phase, long element, double *rechts);
void MMSMakeFluxMatrixEntry(int phase, long element, double *rechts);
void MMSMakeMassMatrixEntryPcap(int phase, long element, double *rechts);
void MMSMakeFluxMatrixEntryPcap(int phase, long element, double *rechts);

static int activ_phase;      
static long i,j;

extern int mmp_reference_pressure_method;

/**************************************************************************
   ROCKFLOW - Funktion: MakeGS_MMS

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *links: linke Seite Gleichungssystem (Matrix)
                    (nur alte Loeser !!!)
   R double *rechts: rechte Seite GS (Vektor)
   R double *ergebnis: Ergebnis des GS, bei Rueckgabe Nullvektor
   E double aktuelle_zeit: aktueller Zeitpunkt

   Ergebnis:
   - void -

   Programmaenderungen:
   8/1998        C. Thorenz Mehrphasenmodell

 **************************************************************************/
void MakeGS_MMS(int phase, double *rechts, char *bc_name, char *ss_mass_flow_name, char *ss_volume_flow_name)
{
bc_name = bc_name;
  activ_phase = phase;

  /* Gleichungssystem mit Nullen vorbelegen */
  MMSInitializeSystemOfEquations(rechts);

  if(GetMethod_MMS()!=10) {
    /* Saettigungsformulierung */
    /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 1) */
    for (i = 0; i < anz_active_elements; i++)
      MMSMakeMassMatrixEntry(phase, ActiveElements[i], rechts);

    /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 2) */
    for (i = 0; i < anz_active_elements; i++)
      MMSMakeFluxMatrixEntry(phase, ActiveElements[i], rechts);
  } else {
    /* Kapillardruckformulierung */
    /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 1) */
    for (i = 0; i < anz_active_elements; i++)
      MMSMakeMassMatrixEntryPcap(phase, ActiveElements[i], rechts);

    /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 2) */
    for (i = 0; i < anz_active_elements; i++)
      MMSMakeFluxMatrixEntryPcap(phase, ActiveElements[i], rechts);
  }

  /* Offene Raender behandeln */
  MMSTreatOpenBoundarys(phase, rechts);

  /* Quellterme einbauen  */
  MMSIncorporateSourceBoundaryConditions(phase, rechts, ss_mass_flow_name, ss_volume_flow_name);

  /* Irregulaere Knoten eliminieren */
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  if(GetMethod_MMS()!=10) {
    /* Saettigungsformulierung */
    /* Dirichlet-Randbedingungen eintragen */
    //OK_BC sprintf(name, "%s%i", bc_name, phase + 1);
    //OK_BC IncorporateBoundaryConditions(name, rechts);

    /* Bedingte Dirichlet-Randbedingungen eintragen */
    //OK_BC sprintf(name, "%s%i", name_boundary_condition_conditional_saturation, phase + 1);
    //OK_BC IncorporateBoundaryConditionsEx2(name, rechts, MMSBoundaryConditionsFunction);
  } else {
    /* pcap-Formulierung */
    //OK_BC sprintf(name, "%s%i", bc_name, phase + 1);
    //OK_BC IncorporateBoundaryConditionsEx2(name, rechts, MMSPcapBoundaryConditionsFunction);
  }
}

/**************************************************************************
   ROCKFLOW - Funktion: MakeGS_MMSV1

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *links: linke Seite Gleichungssystem (Matrix)
                    (nur alte Loeser !!!)
   R double *rechts: rechte Seite GS (Vektor)
   R double *ergebnis: Ergebnis des GS, bei Rueckgabe Nullvektor
   E double aktuelle_zeit: aktueller Zeitpunkt

   Ergebnis:
   - void -

   Programmaenderungen:
   8/1998    CT   Mehrphasenmodell
   05/2001   OK   Variante fuer LES-Objekte

 **************************************************************************/
void MakeGS_MMSV1(double *rechts, double *ergebnis_MMS, double dummy)
{
  int phase = 1; /* ToDo Schleife ueber Phasen*/
  dummy=dummy;
  ergebnis_MMS=ergebnis_MMS;
  activ_phase = phase;
  /* Gleichungssystem mit Nullen vorbelegen */
  MMSInitializeSystemOfEquations(rechts);
  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 1) */
  for (i = 0; i < anz_active_elements; i++)
    MMSMakeMassMatrixEntry(phase, ActiveElements[i], rechts);
  /* Einzelmatrizen zu Gesamtgleichungssystem assemblieren 2) */
  for (i = 0; i < anz_active_elements; i++)
    MMSMakeFluxMatrixEntry(phase, ActiveElements[i], rechts);
  /* Offene Raender behandeln */
//OK_ToDo  MMSTreatOpenBoundarys(phase, rechts);
  /* Quellterme einbauen */
  //OK_ST MMSIncorporateSourceBoundaryConditions(phase,rechts,name_source_mass_fluid_phase,name_source_volume_fluid_phase);
  /* Irregulaere Knoten eliminieren */
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);
  /* Dirichlet-Randbedingungen eintragen */
  //OK_BC sprintf(name, "%s%i", name_boundary_condition_saturation, phase+1);
  //OK_BC IncorporateBoundaryConditions(name,rechts);
  /* Bedingte Dirichlet-Randbedingungen eintragen */
  //OK_BC sprintf(name, "%s%i", name_boundary_condition_conditional_saturation, phase+1);
  //OK_BC IncorporateBoundaryConditionsEx2(name, rechts, MMSBoundaryConditionsFunction);
}


/**************************************************************************
   ROCKFLOW - Funktion:  MMSBoundaryConditionsFunction

   Aufgabe:
   Bedingte RB: Bei Ausfluss einer Phase RB abschalten

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long node : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   2/2001 C.Thorenz Erste Version

 **************************************************************************/
double MMSBoundaryConditionsFunction(long node, int *gueltig, double val)
{
  double flux = 0.;
  int phase;

  *gueltig = 1;

  for (phase = 0; phase < GetRFProcessNumPhases(); phase++) {
    flux = MMin(flux, (1.-GetTimeCollocationOpenBoundary_MMS())*MMSGetNodeFlux(phase, node, -1, 0)+ GetTimeCollocationOpenBoundary_MMS()*MMSGetNodeFlux(phase, node, -1, 1));
  }

  if (flux < -MKleinsteZahl)
       *gueltig = 0;

  return val;
}

/**************************************************************************
   ROCKFLOW - Funktion:  MMSPcapBoundaryConditionsFunction

   Aufgabe:
   Errechnete RB: Ermittelt die zugehoerigen Kapillardruecke zur
   Saettigungs-RB. Sehr haesslich, da nicht die RB-Werte fuer alle
   Phasen vorliegen.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long node : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   2/2002 C.Thorenz Erste Version

 **************************************************************************/
double MMSPcapBoundaryConditionsFunction(long node, int *gueltig, double val)
{
//SB:m  double kap_press[16]; 
	double satu[16], kap = 0.;
  int anzahl;
  long *elemente;
  double theta = 1.; //OK mms_numerics->time_collocation;

  *gueltig = 1;

  elemente = GetNodeElements(node, &anzahl);

  for (i = 0; i < GetRFProcessNumPhases(); i++) 
    satu[i] = 0.;

  for (i = 0; i < GetRFProcessNumPhases(); i++) {
     for (j = 0l; j < anzahl; j++) 
       satu[i] += MMSGetNodeSatu(i, node, elemente[j], 1);
     satu[i] /=(double)anzahl;
  } 

  satu[activ_phase] = val;

  for (j = 0l; j < anzahl; j++) {
//SB:m     CECalcCap_AMM(elemente[j], GetRFProcessNumPhases(), satu, kap_press);
//SB:m     kap += kap_press[activ_phase];
	  kap += mmp_vector[ElGetElementGroupNumber(elemente[j])]->CapillaryPressureFunction(elemente[j],NULL,theta,activ_phase,0.0);
  } 

  elemente = (long *)Free(elemente);

  return kap/(double)anzahl;
}

/**************************************************************************
   ROCKFLOW - Funktion: MMSInitializeSystemOfEquations

   Aufgabe:
   Gleichungssystem initialisieren

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *rechts: rechte Seite GS (Vektor)

   Ergebnis:
   - void -

   Programmaenderungen:
   8/1998        C. Thorenz Mehrphasenmodell

 **************************************************************************/
void MMSInitializeSystemOfEquations(double *rechts)
{
  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);
}

/**************************************************************************
   ROCKFLOW - Funktion:  MMSTreatOpenBoundarya(int, double *);

   Aufgabe:
   Behandeln der offenen Raender

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   Ergebnis:
   - void -

   Programmaenderungen:
   11/1999        C. Thorenz  Erste Version

 **************************************************************************/
void MMSTreatOpenBoundarys(int phase, double *rechts)
{
  long i, node;
  int set;
  double flux;

  //OK_BC sprintf(name, "%s%i", name_boundary_condition_conditional_saturation, phase + 1);

  /* Die Fluesse der einzelnen Phasen werden als Knotenlast behandelt */
  for (i = 0; i < NodeListLength; i++) {
    set = 0;
    node = NodeNumber[i];
    //OK_BC if(IsNodeBoundaryCondition(bc_free_outflow_name, node)) set=1;
    //OK_ST if(IsNodeSourceSink(name_sink_mass_fluid_mixture, node)) set=1;
    //OK_ST if(IsNodeSourceSink(name_sink_volume_fluid_mixture, node)) set=1;
//OK_IC    if(IsNodeBoundaryCondition(name, node)) {
       flux = (1.-GetTimeCollocationOpenBoundary_MMS())*MMSGetNodeFlux(phase, node, -1, 0)+ GetTimeCollocationOpenBoundary_MMS()*MMSGetNodeFlux(phase, node, -1, 1);
       if(flux < 0.) set=1;
//OK_IC    }

    if(set) {
      flux = (1.-GetTimeCollocationOpenBoundary_MMS())*MMSGetNodeFlux(phase, node, -1, 0)+ GetTimeCollocationOpenBoundary_MMS()*MMSGetNodeFlux(phase, node, -1, 1);
      rechts[i] += flux;
    } 
  }
}

/**************************************************************************
   ROCKFLOW - Funktion: MMSIncorporateSourceBoundaryConditions

   Aufgabe:
   RB in Gleichungssystem einbauen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   Ergebnis:
   - void -

   Programmaenderungen:
   3/1999        C. Thorenz

 **************************************************************************/
void MMSIncorporateSourceBoundaryConditions(int phase, double *rechts, char *source_mass_flow_name, char *source_volume_flow_name)
{
rechts = rechts;
  char flux[80];

  activ_phase = phase;

  /* Massen-Punktquelle */
  sprintf(flux, "%s%i", source_mass_flow_name, phase + 1);
  //OK_ST IncorporateSourceSink(flux, rechts, MMSGetSourceMultiplicator);

  /* Volumen-Punktquelle einarbeiten */
  sprintf(flux, "%s%i", source_volume_flow_name, phase + 1);
  //OK_ST IncorporateSourceSink(flux, rechts, NULL);

}

double MMSGetSourceMultiplicator(long i)
{
  return 1. / MMax(mfp_vector[activ_phase]->Density(),MKleinsteZahl);
}


/**************************************************************************
   ROCKFLOW - Funktion: MMSMakeMassMatrixEntry

   Aufgabe:
   Gleichungssystemeintrag fuer ein Element

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   5/1999  C.Thorenz     Erste Version

 **************************************************************************/
void MMSMakeMassMatrixEntry(int phase, long element, double *rechts)
{
  static double *mass, edt;
  static long nn, *element_nodes;
  static int i;
  static long element_node_index[8];
  static double left_matrix[64];
  static double zwickau[64];
  static double zwiebel[8];
  static double right_vector[8];
  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;
  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);
  /* Knotennummern in element_node_index geben an, wo sich die Knoten des Elements
     als Eintraege im Gesamtgleichungssystem finden lassen */
  for (i = 0; i < nn; i++)
    element_node_index[i] = GetNodeIndex(element_nodes[i]);
  /* Initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(zwickau, nn, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(right_vector, nn);
  char saturation_name[15];
  sprintf(saturation_name,"SATURATION%i",phase+1);
  /* Linke Seite aus Massenmatrix */
  mass = MMSGetElementSaturationMassMatrix(element);
  if (mass)
    {
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += edt * mass[i];

      /* Randbedingungen und Ergebnisse aus letztem Zeitschritt uebertragen */
      for (i = 0; i < nn; i++)
        zwiebel[i] = edt * PCSGetNODValue(element_nodes[i],saturation_name,0); //OK MMSGetNodeSatu(phase, element_nodes[i], element, 0);

      /* Rechte Seite (Vektor) aus Massenmatrix */
      MMultMatVec(mass, nn, nn, zwiebel, nn, zwickau, nn);
      for (i = 0; i < nn; i++)
        right_vector[i] += zwickau[i];
    }

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);

}

/**************************************************************************
   ROCKFLOW - Funktion: MMSMakeFluxMatrixEntry

   Aufgabe:
   Gleichungssystemeintrag fuer ein Element

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   5/1999  C.Thorenz     Erste Version
 **************************************************************************/
void MMSMakeFluxMatrixEntry(int phase, long element, double *rechts)
{
  static double *conductivity_matrix, *capacitance_matrix, *gravity_vector, *cap_pressure, edt, dp_dS[8];
  static long nn, *element_nodes;
  static int i;
  static long element_node_index[8];
  static double left_matrix[64];
  static double zwickau[64];
  static double zwiebel[8];
  static double right_vector[8], cap_pressure_change[8];
  /* damit keine Warnung auftritt */
  edt = aktuelle_zeit;
  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;
  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);
  /* Knotennummern in element_node_index geben an, wo sich die Knoten des Elements
     als Eintraege im Gesamtgleichungssystem finden lassen */
  for (i = 0; i < nn; i++)
    element_node_index[i] = GetNodeIndex(element_nodes[i]);
  /* Initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(zwickau, nn, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(right_vector, nn);
  char pressure_name[10];
  sprintf(pressure_name,"PRESSURE%i",mmp_reference_pressure_method+1);
  /* Rechte Seite (Vektor) aus Flussmatrix*Druck */
  conductivity_matrix = MMSGetElementConductivityMatrix(element, phase);
  if (conductivity_matrix)
    {
      /* Hier wird der Druck der Referenz-Phase verwendet, da die Kapillardruecke separat behandelt werden */
      for (i = 0; i < nn; i++)
/*OK
        zwiebel[i] = (1.-GetTimeCollocationGlobal_MMS()) \
                   * MMSGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 0) \
                   + GetTimeCollocationGlobal_MMS() \
                   * MMSGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 1);
*/
        zwiebel[i] = (1.-GetTimeCollocationGlobal_MMS()) \
                   * PCSGetNODValue(element_nodes[i],pressure_name,0) \
                   + GetTimeCollocationGlobal_MMS() \
                   * PCSGetNODValue(element_nodes[i],pressure_name,1);
      MMultMatVec(conductivity_matrix, nn, nn, zwiebel, nn, zwickau, nn);
      for (i = 0; i < nn; i++)
        right_vector[i] -= zwickau[i];
      if(GetMethod_MMS()==6) {
         /* Erstmal zum Probieren in Elementmitte, spaeter richtig integrieren */ 
         dp_dS[0] = 1./MMSGetSaturationPressureDependency(phase, element, 0., 0., 0., GetTimeCollocationGlobal_MMS());
         for (i = 0; i < nn*nn; i++)
           left_matrix[i] -= dp_dS[0] * conductivity_matrix[i];
      } 
    }
  /* Rechte Seite (Vektor) aus Kapillarduecken */
  if(GetMethod_MMS()!=6) {
    cap_pressure = MMSGetElementCapillarityVector(element, phase);
    if (cap_pressure && conductivity_matrix) {
      MMultMatVec(conductivity_matrix, nn, nn, cap_pressure, nn, zwiebel, nn);
      for (i = 0; i < nn; i++)
        right_vector[i] += zwiebel[i];
    }
  }

  /* Rechte Seite (Vektor) aus Kompressibilitaet*Druckaenderung */
  capacitance_matrix = MMSGetElementCapacitanceMatrix(element, phase);
  if (capacitance_matrix) {
    for (i = 0; i < nn; i++){
      //OK cap_pressure_change[i] = edt * (MMSGetNodePress(phase,element_nodes[i],element,1)-MMSGetNodePress(phase,element_nodes[i],element,0));
      cap_pressure_change[i] = edt * \
        (GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",1))-GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",0)));
    }
     MMultMatVec(capacitance_matrix, nn, nn, cap_pressure_change, nn, zwiebel, nn);
      
     for (i = 0; i < nn; i++)
        right_vector[i] -= zwiebel[i];
  } 

  /* Rechte Seite (Vektor) aus Gravitationsterm   */
  gravity_vector = MMSGetElementGravityVector(element, phase);
  if (gravity_vector)
    for (i = 0; i < nn; i++)
        right_vector[i] -= gravity_vector[i];

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}


/**************************************************************************
   ROCKFLOW - Funktion: MMSMakeMassMatrixEntryPcap

   Aufgabe:
   Massenmatrix-Gleichungssystemeintrag fuer ein Element  
   im Pcap-Formulierung

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   2/2002  C.Thorenz     Erste Version

 **************************************************************************/
void MMSMakeMassMatrixEntryPcap(int phase, long element, double *rechts)
{
  static double *mass, edt;
  static long nn, *element_nodes;
  static int i;
  static long element_node_index[8];
  static double left_matrix[64];
  static double zwickau[64];
  static double zwiebel[8];
  static double right_vector[8];
  static double dS_dp[16];

  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;

  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);
  /* Knotennummern in element_node_index geben an, wo sich die Knoten des Elements
     als Eintraege im Gesamtgleichungssystem finden lassen */
  for (i = 0; i < nn; i++)
    element_node_index[i] = GetNodeIndex(element_nodes[i]);

  /* Initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(zwickau, nn, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(right_vector, nn);

  /* Linke Seite aus Massenmatrix */
  mass = MMSGetElementSaturationMassMatrix(element);
  if (mass)
    {
      for (i = 0; i < nn; i++)
        dS_dp[i] =  MMSGetNodeSaturationPressureDependency(phase, element, element_nodes[i], GetTimeCollocationGlobal_MMS());

      for (i = 0; i < nn; i++)
        for (j = 0; j < nn; j++)
          left_matrix[i*nn+j] += dS_dp[j] * edt * mass[i*nn+j];

      /* Expliziter Anteil */
      for (i = 0; i < nn; i++)
        zwiebel[i] = dS_dp[i] * edt * (MMSGetNodePress(0, element_nodes[i], element, 0)-MMSGetNodePress(phase, element_nodes[i], element, 0));

      /* Rechte Seite (Vektor) aus Massenmatrix */
      MMultMatVec(mass, nn, nn, zwiebel, nn, zwickau, nn);
      for (i = 0; i < nn; i++)
        right_vector[i] += zwickau[i];
    }

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}

/**************************************************************************
   ROCKFLOW - Funktion: MMSMakeFluxMatrixEntryPcap

   Aufgabe:
   Gleichungssystemeintrag fuer ein Element

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:

   2/2002  C.Thorenz     Erste Version

 **************************************************************************/
void MMSMakeFluxMatrixEntryPcap(int phase, long element, double *rechts)
{
  static double *conductivity_matrix, *capacitance_matrix, *gravity_vector, edt;
  static long nn, *element_nodes;
  static int i;
  static long element_node_index[8];
  static double left_matrix[64];
  static double zwickau[64];
  static double zwiebel[8];
  static double right_vector[8];
  static double cap_pressure_change[8];

  /* damit keine Warnung auftritt */
  edt = aktuelle_zeit;
  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;

  /* Geometrie */
  nn = ElNumberOfNodes[ElGetElementType(element) - 1];
  element_nodes = ElGetElementNodes(element);
  /* Knotennummern in element_node_index geben an, wo sich die Knoten des Elements
     als Eintraege im Gesamtgleichungssystem finden lassen */
  for (i = 0; i < nn; i++)
    element_node_index[i] = GetNodeIndex(element_nodes[i]);

  /* Initialisieren */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(zwickau, nn, nn);
  MNulleVec(zwiebel, nn);
  MNulleVec(right_vector, nn);

  /* Rechte Seite (Vektor) aus Flussmatrix*Druck */
  conductivity_matrix = MMSGetElementConductivityMatrix(element, phase);
  if (conductivity_matrix)
    {
      for (i = 0; i < nn; i++)
        zwiebel[i] = (1. - GetTimeCollocationGlobal_MMS()) * MMSGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 0) + GetTimeCollocationGlobal_MMS() * MMSGetNodePress(GetReferencePhase_MMP(), element_nodes[i], element, 1);

      MMultMatVec(conductivity_matrix, nn, nn, zwiebel, nn, zwickau, nn);

      for (i = 0; i < nn; i++)
        right_vector[i] -= zwickau[i];
    
      for (i = 0; i < nn*nn; i++)
        left_matrix[i] -= conductivity_matrix[i];

    }

  /* Rechte Seite (Vektor) aus Kompressibilitaet*Druckaenderung */
  capacitance_matrix = MMSGetElementCapacitanceMatrix(element, phase);
  if (capacitance_matrix) {
     for (i = 0; i < nn; i++)
        cap_pressure_change[i] = edt * (MMSGetNodePress(phase, element_nodes[i], element, 1) - MMSGetNodePress(phase, element_nodes[i], element, 0));
     MMultMatVec(capacitance_matrix, nn, nn, cap_pressure_change, nn, zwiebel, nn);
      
     for (i = 0; i < nn; i++)
        right_vector[i] -= zwiebel[i];
  } 

  /* Rechte Seite (Vektor) aus Gravitationsterm   */
  gravity_vector = MMSGetElementGravityVector(element, phase);
  if (gravity_vector)
    for (i = 0; i < nn; i++)
        right_vector[i] -= gravity_vector[i];

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(element, left_matrix, right_vector, rechts);
}




/**************************************************************************
ROCKFLOW - Funktion: MMSCalcSaturations

Aufgabe:
  Berechnet die Phasensaettigungen der restlichen Phasen

Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  E int timelevel 0: alte Zeitebene, 1: neue Zeitebene

Ergebnis:
   -void-

Programmaenderungen:
  ??/????   CT   Erste Version (Loop0699MakeSumOfSaturationsEqualToOne)
  05/2001   OK   Verallgemeinerung

**************************************************************************/
void MMSCalcSaturations(int timelevel)
{
  double summe, value;
  long i;
  int activ_phase;

  if (GetOtherSatuCalcMethod_MMS() >= 0)
    {
      /* Die nichtberechnete Saettigung wird aus den anderen Saettigungen ermittelt */
      for (i = 0; i < NodeListLength; i++)
        {
          summe = 1.;
          for (activ_phase = 0; activ_phase < GetRFProcessNumPhases(); activ_phase++)
            if (activ_phase != GetOtherSatuCalcMethod_MMS())
              {
                summe -= MMSGetNodeSatu(activ_phase,NodeNumber[i],9999,timelevel);
              }
          MMSSetNodeSatu(GetOtherSatuCalcMethod_MMS(),NodeNumber[i],timelevel,summe);
        }
    }
  if (GetOtherSatuCalcMethod_MMS() == -1)
    {
      /* Es werden alle Ergebnisse so angepasst, dass die Summe
         wieder 1 ist. */
      for (i = 0; i < NodeListLength; i++)
        {
          summe = 1.;
          for (activ_phase = 0; activ_phase < GetRFProcessNumPhases(); activ_phase++)
            {
              summe -= MMSGetNodeSatu(activ_phase,NodeNumber[i],9999,timelevel);
            }
          /* summe ist der zu verteilende Rest */
          for (activ_phase = 0; activ_phase < GetRFProcessNumPhases(); activ_phase++)
            {
              value = MMSGetNodeSatu(activ_phase,NodeNumber[i],9999,timelevel) + summe / (double) GetRFProcessNumPhases();
              MMSSetNodeSatu(activ_phase,NodeNumber[i],timelevel,summe);
            }
        }
    }
}





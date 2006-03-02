/**************************************************************************
   ROCKFLOW - Modul: cgs_atm.c
 
   Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul aTM.

 
   Programmaenderungen:
   09/1994     MSR/hh        Erste Version
   03/1995     cb            SUPG
   05/1996     cb            3D
   08/1996     cb            MTranspoMat
   01.07.1997  R.Kaiser      Korrekturen und Aenderungen aus dem aTM
                             uebertragen
   16.07.1997  R.Kaiser      Interpol_q_XD -> adaptiv.c
   25.07.1997  R.Kaiser      CalcIrrNodes_ASM umbenannt zu
                             CalcIrrNodeVals (erweitert) -> adaptiv.c
   25.07.1997  R.Kaiser      Adaptivitaet aus dem Modul herausgenommen
                             (DelIrrNodesXD -> adaptiv.c)
   14.06.1999  OK            MakeGS_ATM_NEW ! unterschiedlich
   10/1999     CT            Zugriff auf Saettigung wiederhergestellt (!)
   11/1999     RK            EliminateIrrNodes()
   12/1999     CT            Irr. Knoten _vor_ Dirichlet-BC
   02/2000     CT            Verallgemeinert, Dimensionen zusammengefasst,
                             Porositaets- und Saettigungsaenderung als eigene
                             Matrix
   7/2000      CT            Bedingte RB an Zuflussraendern
   03/2003     RK            Quellcode bereinigt, Globalvariablen entfernt
 
 
**************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTCGS_ATM


/* Intern benutzte Module/Objekte */
#include "int_atm.h"
#include "elements.h"
#include "nodes.h"
#include "mathlib.h"
#include "matrix.h"
#include "edges.h"
#include "plains.h"
#include "femlib.h"
#include "adaptiv.h"
//OK_IC #include "rfbc.h"
//OK_IC #include "rfsousin.h"
#include "numerics.h"
#include "rfmodel.h"
#include "rf_tim_new.h"

/* Tools */
#include "tools.h"


/* Definitionen */
double ATMBoundaryConditionsFunction(long knoten, int *gueltig, double val);
double ATMSourceSinkFunction(long knoten, int *gueltig, double val);
void MakeGSxD(long index, int phase, double *rechts, double *ergebnis, double theta);

static int activ_phase = -1, activ_component = -1;
static double atm_c_max_min;

/**************************************************************************/
/* ROCKFLOW - Funktion: MakeStatMat_ATM
 */
/* Aufgabe:
   alle stationaeren Matrizen etc. berechnen (vor Verfeinerung)
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   10/1994     MSR        Erste Version
   01/1996     cb         MakeMat
   08/1996     cb         StatMat
 */
/**************************************************************************/
void MakeStatMat_ATM(void)
{
  static long i;
  /* alle stationaeren Matrizen etc. berechnen */
  for (i = 0; i < ElListSize(); i++)
    if (ElGetElement(i) != NULL)       /* Element existiert */
      if (ElGetElementActiveState(i))
        {                              /* aktives Element */
          StatMat_ATM(i);
          if (GetRFControlModel() == 99)
            MakeMat_ATM(i);
          /* nur sinnvoll, wenn Druckverteilung eingelesen wird */
        }
}



/**************************************************************************
   ROCKFLOW - Funktion: MakeGSxD

   Aufgabe:
    Gleichungssystem-Eintraege der Matrizen

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:
   05/1996     cb            erste Version
   25.07.1997  R.Kaiser      Adaptivitaet herausgenommen
   02/2000     C.Thorenz     Verallgemeinert, Dimensionen zusammengefasst,
                             Porositaets- und Saettigungsaenderung.
   09/2000     C.Thorenz     Mehrphasen, kuenstlich Diffusion

 **************************************************************************/
void MakeGSxD(long index, int phase, double *rechts, double *ergebnis, double theta)
{
  double *mass = NULL, *adv = NULL, *disp = NULL;
  double edt, c_max = -1.e99, c_min = 1.e99, c_max_min, art_diff = 1.;
  long nn;
  int i;
  static double left_matrix[64];
  static double right_matrix[64];
  static double explizit[8];
  static double right_vector[8];

#ifdef CONTENT_CHANGE
  double *content = NULL;
#endif

  nn = ElNumberOfNodes[ElGetElementType(index) - 1];

#ifdef TESTTAYLOR
  theta = 0.5;
#endif

  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
  edt = 1.0 / dt;

  /* Initialisierung */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(explizit, nn);
  MNulleVec(right_vector, nn);

  /* Randbedingungen und Ergebnisse aus letztem Zeitschritt uebertragen */
  for (i = 0; i < nn; i++)
    explizit[i] = ergebnis[GetNodeIndex(ElGetElementNodes(index)[i])];


  /* Kuenstliche Diffusion? */
  if (GetArtificialDiffusion_ATM())
    {
      for (i = 0; i < nn; i++)
        {
          c_max = MMax(c_max, explizit[i]);
          c_min = MMin(c_min, explizit[i]);
        }
      c_max_min = c_max - c_min;

      if (GetArtificialDiffusionParam_ATM()[0] < 0.)
        {
          /* Relative Werte */
          c_max_min /= atm_c_max_min;
        }
      if (c_max_min > fabs(GetArtificialDiffusionParam_ATM()[0]))
        art_diff = pow(GetArtificialDiffusionParam_ATM()[1], (c_max_min - fabs(GetArtificialDiffusionParam_ATM()[0])) / fabs(GetArtificialDiffusionParam_ATM()[0] + MKleinsteZahl));
    }
  /* Matrizen zusammenbauen */

  /* Massenmatrix nS*dc/dt */
  if (ATMGetElementMassMatrixNew)
    mass = ATMGetElementMassMatrixNew(index, phase);
  if (ATMGetElementMassMatrix)
    mass = ATMGetElementMassMatrix(index);
  if (mass)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += edt * mass[i];
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += edt * mass[i];
    }

#ifdef CONTENT_CHANGE
  /* Fluidgehaltmatrix c*d(nS)/dt */
  if (ATMGetElementFluidContentMatrixNew)
    content = ATMGetElementFluidContentMatrixNew(index, phase);
  if (ATMGetElementFluidContentMatrix)
    content = ATMGetElementFluidContentMatrix(index);
  if (content)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * edt * content[i];
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1. - theta) * edt * content[i];
    }
#endif

  /* Advektionsmatrix */
  if (ATMGetElementAdvMatrixNew)
    adv = ATMGetElementAdvMatrixNew(index, phase);
  if (ATMGetElementAdvMatrix)
    adv = ATMGetElementAdvMatrix(index);
  if (adv)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * adv[i];
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1.0 - theta) * adv[i];
    }

  /* Diffusionsmatrix */
  if (ATMGetElementDispMatrixNew)
    disp = ATMGetElementDispMatrixNew(index, phase);
  if (ATMGetElementDispMatrix)
    disp = ATMGetElementDispMatrix(index);
  if (disp)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * disp[i] * art_diff;
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1.0 - theta) * disp[i] * art_diff;
    }

  /* Negativ-Diffusion fuer Taylor-Galerkin, entfernt ab Version 3.4.23 */

  /* Rechte-Seite-Vektor aus explizitem Anteil ermitteln */
  MMultMatVec(right_matrix, nn, nn, explizit, nn, right_vector, nn);

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index, left_matrix, right_vector, rechts);

}



/**************************************************************************
   ROCKFLOW - Funktion: MakeGS_ATM

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *links: linke Seite Gleichungssystem (Matrix)
                    (nur alte Loeser !!!)
   R double *rechts: rechte Seite GS (Vektor)
   R double *ergebnis: Ergebnis des GS, bei Rueckgabe Nullvektor

   Ergebnis:
   - void -

   Programmaenderungen:
   10/1994     hh/MSR        Erste Version
   03/1995     cb            SUPG
   03/1995     MSR           Randbedingungs-Zeitkurven eingebaut
   08/1995     cb            irregulaere Knoten eliminieren
   11/1995     msr           alte und neue Loeser
   04/1996     cb            RB nur an Zuflussknoten
    6/1999     OK            ohne Namen-String
    7/2000     CT            Bedingte RB am Zuflussrand
   09/2000     C.Thorenz     Mehrphasen

 **************************************************************************/
void MakeGS_ATM(double *rechts, double *ergebnis, double zeit)
{
  double theta = GetNumericalTimeCollocation("TRANSPORT");
  MakeGS_ATM_New(rechts, ergebnis, zeit, 0, 1, 0, theta);
}

void MakeGS_ATM_New(double *rechts, double *ergebnis, double zeit, int phase, int anz_phasen, int component, double theta)
{
  anz_phasen = anz_phasen;
  /* Variablen */
  long i, index, elanz;
  double c_max = -1.e99, c_min = 1.e99;

  double *residuum;
  residuum = (double *) Malloc(NodeListLength * sizeof(double));


  zeit = zeit;

  activ_phase = phase;
  activ_component = component;


  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);
  /* Elementunabhaengige Berechnungen */
  /* Ergebnisse aus letztem Zeitschritt holen */

  for (i = 0l; i < NodeListLength; i++)
    {
      if (ATMGetNodeConc)
        {                              /* Neu */
          ergebnis[i] = ATMGetNodeConc(NodeNumber[i], phase, component, 0);
        }
      else
        {                              /* Alte Variante */
          ergebnis[i] = ATMGetNodeConc0(NodeNumber[i]);
        }
      c_max = MMax(c_max, ergebnis[i]);
      c_min = MMin(c_min, ergebnis[i]);
    }

  /* Groesste Spanne im System, fuer eventuelle Glaettung */
  atm_c_max_min = c_max - c_min;

  /* Linke und rechte Seite aufbauen aus Elementmatrizen aller Elemente */
  elanz = ElListSize();
  for (index = 0; index < elanz; index++)
    {
      if (ElGetElement(index) != NULL)
        {                              /* wenn Element existiert */
          if (ElGetElementActiveState(index))
            {                          /* nur aktive Elemente */
              MakeGSxD(index, phase, rechts, ergebnis, theta);
            }                          /* if = nur aktive Elemente */
        }                              /* if = Element existiert */
    }                                  /* for = Schleife ueber die Elemente */

#ifdef TESTCGS_ATM
  DisplayMsgLn("MakeGS vor Punktquellen");
#endif

  MXResiduum(ergebnis, rechts, residuum);


  /* Punktquellen einarbeiten */
  //OK_ST sprintf(name, "%s%i", name_source_mass_tracer_component, component * anz_phasen + phase + 1);
  //OK_ST IncorporateSourceSinkEx(name, rechts, ATMSourceSinkFunction);

  MXResiduum(ergebnis, rechts, residuum);

  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  MXResiduum(ergebnis, rechts, residuum);


  /* Dirichlet-Randbedingungen eintragen */
  //OK_BC sprintf(name, "%s%d", name_boundary_condition_tracer_component, component * anz_phasen + phase + 1);
  //OK_BC IncorporateBoundaryConditions(name, rechts);

  MXResiduum(ergebnis, rechts, residuum);

  /* Bedingte Dirichlet-Randbedingungen eintragen */
  //OK_BC sprintf(name, "%s%i", name_boundary_condition_conditional_tracer_component, component * anz_phasen + phase + 1);
  //OK_BC IncorporateBoundaryConditionsEx2(name, rechts, ATMBoundaryConditionsFunction);

  MXResiduum(ergebnis, rechts, residuum);

  residuum = (double *) Free(residuum);

}


/**************************************************************************
   ROCKFLOW - Funktion: CalcFlux_ATM

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *rechts  : Vektor fuer rechte Seite GS (Vektor)
   E double *ergebnis: Vektor fuer Ergebnis des GS
   X double *residuum: Vektor fuer Residuum und die Fluesse des GS
   E int phase       : Zu betrachtende Phase
   E int component   : Zu betrachtende Komponente in der Phase

   Ergebnis:
   - void -

   Programmaenderungen:
   05/2001     C.Thorenz     Erste Version
 **************************************************************************/
void CalcFlux_ATM(double *rechts, double *ergebnis, double *residuum, int phase, int component)
{
  /* Variablen */
  long i, index, elanz;

  activ_phase = phase;
  activ_component = component;


  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);
  /* Elementunabhaengige Berechnungen */
  /* Ergebnisse aus letztem Zeitschritt holen */

  /* Linke und rechte Seite aufbauen aus Elementmatrizen aller Elemente */
  elanz = ElListSize();

  for (i = 0l; i < NodeListLength; i++)
     ergebnis[i] = ATMGetNodeConc(NodeNumber[i], phase, component, 0);

  for (index = 0; index < elanz; index++)
    {
      if (ElGetElement(index) != NULL)
        {                              /* wenn Element existiert */
          if (ElGetElementActiveState(index))
            {                          /* nur aktive Elemente */
              MakeGSxD(index, phase, rechts, ergebnis, GetTimeCollocationGlobal_ATM());
            }                          /* if = nur aktive Elemente */
        }                              /* if = Element existiert */
    }                                  /* for = Schleife ueber die Elemente */

  for (i = 0l; i < NodeListLength; i++)
     ergebnis[i] = ATMGetNodeConc(NodeNumber[i], phase, component, 1);
  
  /* Das Residuum enthaelt diffusive Fluesse und Quellen und Senken */
  MXResiduum(ergebnis, rechts, residuum);

  /* Auch advektive Fluesse beruecksichtigen */
  for (i = 0l; i < NodeListLength; i++)
    residuum[i] -= ergebnis[i] * ATMGetNodeFlux(activ_phase, NodeNumber[i], -1, 1); 


  /* Das Vorzeichen wird umgedreht, damit negative Fluesse aus dem Gebiet heraus gehen */
  for (i = 0l; i < NodeListLength; i++)
    residuum[i] *= -1.; 
}



/**************************************************************************
   ROCKFLOW - Funktion:  ATMSourceSinkFunction

   Aufgabe:
   Ermoeglicht Fluidzufluesse mit 0-Konzentration

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   7/2000 C.Thorenz Erste Version
   3/2001 C.Thorenz Behandlung der Raender verbessert

 **************************************************************************/
double ATMSourceSinkFunction(long knoten, int *gueltig, double val)
{
  double flux = 0.;

  *gueltig = 1;

  if (ATMGetNodeFlux)
    {
      flux = (1.-GetTimeCollocationSource_ATM()) * ATMGetNodeFlux(activ_phase, knoten, -1, 0) +
             (GetTimeCollocationSource_ATM()) * ATMGetNodeFlux(activ_phase, knoten, -1, 1);

      if (flux > 0.)
        {
/*
          conc = (1.-GetTimeCollocationSource_ATM()) * ATMGetNodeConc(knoten, activ_phase, activ_component, 0) +
                 (GetTimeCollocationSource_ATM()) * ATMGetNodeConc(knoten, activ_phase, activ_component, 1); 
          val -= conc * flux;
*/
          /* Besser: Implizit im GLS */  
          MXInc(GetNodeIndex(knoten),GetNodeIndex(knoten),flux);

        }
    }
  return val;
}

/**************************************************************************
   ROCKFLOW - Funktion:  ATMBoundaryConditionsFunction

   Aufgabe:
   Bedingte RB: Nur bei Zufluss angeschaltet

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   7/2000 C.Thorenz Erste Version

 **************************************************************************/

double ATMBoundaryConditionsFunction(long knoten, int *gueltig, double val)
{
  double flux = -1.;

  *gueltig = 0;

  if (ATMGetNodeFlux)
    {
      flux = ATMGetNodeFlux(activ_phase, knoten, -1, 0);
      if (flux > MKleinsteZahl)
        {
          *gueltig = 1;
        }
    }
  return val;
}

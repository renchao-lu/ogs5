/**************************************************************************
   ROCKFLOW - Modul: lag_atm.c

   Aufgabe:
   Funktionen zur Berechnung des Lagrange'schen Anteils der
   Transportgleichung

   Programmaenderungen:
    5/1998     C. Thorenz       Erste Version
   03/2003     RK               Quellcode bereinigt, Globalvariablen entfernt   

 **************************************************************************/

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "lag_atm.h"
#include "cvel.h"
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "matrix.h"
#include "femlib.h"
#include "adaptiv.h"
//OK_IC #include "rfbc.h"
#include "tools.h"
#include "material.h"
#include "numerics.h"
#include "rf_tim_new.h"

struct
  {
    long startknoten;
    double dt;
    double konzsum;
    double weightsum;
    int phase;
    int c1;
    /*    int modifyvelocity; */
    int timeweighting;
    long number_of_steps;
    long maximal_steps;
    int velocity_calculation;
    int use_matrix;
    double local_eps;
    double minimalweight;
    double quality;
    double theta;
  }
lagrangecommon;

typedef struct
  {
    double koor[3];
    int position;
    long knoten;
    long kante;
    long flaeche;
    long volumen;
    double restdt;
    double ownweight;
    int slip;
  }
LAGRANGETRACE;

typedef struct
  {
    LAGRANGETRACE trace;
    int activ;
  }
LAGRANGESTACK;

static LAGRANGESTACK *lagrange_stack = NULL;
static long lagrange_stack_size = 0;

static void *lagrange_matrix;   /* Wurzel der Advektionsmatrix fuer Lagrange-Verfahren */
static void *other_matrix;      /* Wurzel der vorher gueltigen Matrix */
static double *ergebnis_LAG = NULL;     /* Ergebnis = Konzentrationen an den Knoten fuer Lagrange-Verfahren */
static double *ergebnis_LAG2 = NULL;    /* Ergebnis = Konzentrationen an den Knoten fuer Lagrange-Verfahren */

/* Berechnet den advektiven Transport auf der Basis von c1 und speichert die Ergebnisse in c2 */
void LAGCalcLagrangeConcentration(int, int, char *, char *);
/* verfolgt eine Stromlinie stromauf */
double LAGTraceStreamline2DRecursive(LAGRANGETRACE);
double LAGTraceStreamlineThrough2DElement(LAGRANGETRACE, LAGRANGETRACE *);
void LAGCopyTraceStructure(LAGRANGETRACE, LAGRANGETRACE *);
void LAGSetResults(LAGRANGETRACE);

/* Lege Struktur auf den Stack */
void LAGAddTraceStructureToStack(LAGRANGETRACE);
/* Hole Struktur vom Stack */
int LAGGetTraceStructureFromStack(LAGRANGETRACE *);

/* berechnet den Schnittpunkt zweier Geraden im Raum */
int LAGCalcVectorDependence(double *, double *, double *, double *, double *);
/* Liefert alle aktiven 2D-Elemente an einer Kante, die moegliche Folgeelemente fuer die Verfolgung sind */
long *LAGGetEdgeNext2DElements(long, int *, double *);
/* Liefert alle aktiven 2D-Elemente neben einem Knoten */
long *LAGGetNode2DElements(long, int *);
/* Liefert alle aktiven 2D-Elemente an einer Kante */
long *LAGGetEdge2DElements(long edge, int *anz_alle_elem);
/* Liefert alle Kanten an einem Knoten */
long *LAGGetNode2DEdges(long, int *);
/* Liefert den Volumenstrom parallel zu einer Kante, aus der Sicht eines Elements */
double LAGCalcEdgeParallelFlow(long, long, long, double *);
/* Liefert den Volumenstrom parallel zu einer Kante, aus der mittleren Sicht der Elemente an dieser Kante */
double LAGCalcMiddleEdgeParallelFlow(long, long, double *);
double LAGCalcEdgeFlow(long, long, double *);
double LAGCalcNodeFlow(long, long);
long LAGFindNextEdgeFromNode(long);
void LAGGet2DNegativeEdgeParallelVelocity(long, double *, double *);
void LAGGet2DElementFilterVelocity(long, double, double *, double *);

/************************************************************************
   ROCKFLOW - Funktion: LAGInitialize

   Aufgabe:
   Initilisiert das Lagrange-Verfahren

   Formalparameter: ()
   Ergebnis:

   Programmaenderungen:
   3/2000    C. Thorenz  Erste Version
************************************************************************/
void LAGInitialize()
{
  LAGRANGE *lag = NumericsGetPointerLagrangeArray("CONCENTRATION0");

  if (lag)
    {
      lagrangecommon.local_eps = lag -> local_eps;
      lagrangecommon.minimalweight = lag -> minimalweight;
      lagrangecommon.maximal_steps = lag -> maximal_steps;
      lagrangecommon.timeweighting = lag -> timeweighting;
      lagrangecommon.theta = 1.;
      lagrangecommon.use_matrix = lag -> use_matrix;
      lagrangecommon.velocity_calculation = lag -> velocity_calculation;

      ergebnis_LAG = (double *) Malloc(sizeof(double) * NodeListLength);
      ergebnis_LAG2 = (double *) Malloc(sizeof(double) * NodeListLength);

      if (lagrangecommon.use_matrix)
        {
          other_matrix = (void *) MXGetMatrixPointer();
          lagrange_matrix = MXSetupMatrix(NodeListLength, speichertechnik_tran, 0l);
          MXSetMatrixPointer(other_matrix);
        }
    }
}

/************************************************************************
   ROCKFLOW - Funktion: LAGResize

   Aufgabe:
   Passt Matrix und Vektoren an

   Formalparameter: ()
   Ergebnis:

   Programmaenderungen:
   3/2000    C. Thorenz  Erste Version
************************************************************************/
void LAGResize()
{

  ergebnis_LAG = (double *) Free(ergebnis_LAG);
  ergebnis_LAG = (double *) Malloc(sizeof(double) * NodeListLength);
  ergebnis_LAG2 = (double *) Free(ergebnis_LAG2);
  ergebnis_LAG2 = (double *) Malloc(sizeof(double) * NodeListLength);

  if (lagrangecommon.use_matrix)
    {
      other_matrix = (void *) MXGetMatrixPointer();
      MXSetMatrixPointer(lagrange_matrix);
      MXResizeMatrix(NodeListLength);

      MXSetMatrixPointer(other_matrix);
    }
}

/************************************************************************
   ROCKFLOW - Funktion: LAGDestroy

   Aufgabe:
   Zerstoert das Lagrange-Verfahren

   Formalparameter: ()
   Ergebnis:

   Programmaenderungen:
   3/2000    C. Thorenz  Erste Version
************************************************************************/
void LAGDestroy()
{

  ergebnis_LAG = (double *) Free(ergebnis_LAG);
  ergebnis_LAG2 = (double *) Free(ergebnis_LAG2);

  if (lagrangecommon.use_matrix)
    {
      other_matrix = (void *) MXGetMatrixPointer();
      MXSetMatrixPointer(lagrange_matrix);
      MXDestroyMatrix();
      MXSetMatrixPointer(other_matrix);
    }
}

/************************************************************************
   ROCKFLOW - Funktion: LAGCalcLagrangeAdvection

   Aufgabe:
   Ermittelt den advektiven Schritt fuer eine Knotengroesse.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int c1:   Alte Zeitebene in Knotenindex c1
   E int c2:   Neue Zeitebene in Knotenindex c2
   E string:   Name der Konz.-RB
   E modus :   Modus 0=alte Werte holen; 1=neu berechnete Werte holen.

   Ergebnis:

   Programmaenderungen:
   3/2000    C. Thorenz  Erste Version
************************************************************************/
void LAGCalcLagrangeAdvection(int phase, int c1, int c2, char *bc_name1, char *bc_name2, int modus)
{
  long i;

  lagrangecommon.phase = phase;
  lagrangecommon.dt = dt;


  if (lagrangecommon.use_matrix)
    {
      other_matrix = (void *) MXGetMatrixPointer();
      MXSetMatrixPointer(lagrange_matrix);      /* Matrix, in der die Beziehungen des advektiven Transports stehen */
    }

  if (modus)
    {
      LAGCalcLagrangeConcentration(c1, c2, bc_name1, bc_name2);
    }
  else
    {
      if (lagrangecommon.use_matrix)
        {
          PresetErgebnis(ergebnis_LAG2, c1);
          MXMatVek(ergebnis_LAG2, ergebnis_LAG);
          TransferNodeVals(ergebnis_LAG, c2);
        }
      else
        {
          LAGCalcLagrangeConcentration(c1, c2, bc_name1, bc_name2);
        }

    }

  /* RB eintragen */
  for (i = 0; i < NodeListLength; i++)
    {
      //OK_BC if (IsNodeBoundaryCondition(bc_name1, NodeNumber[i]))     /* RB */
        //OK_BC SetNodeVal(NodeNumber[i], c2, GetBoundaryConditionsValue(bc_name1, NodeNumber[i]));
      //OK_BC if (IsNodeBoundaryCondition(bc_name2, NodeNumber[i]))     /* RB */
        //OK_BC SetNodeVal(NodeNumber[i], c2, GetBoundaryConditionsValue(bc_name2, NodeNumber[i]));
    }

  if (lagrangecommon.use_matrix)
    MXSetMatrixPointer(other_matrix);  /* Alte Matrix wieder herstellen */

}

/************************************************************************
   ROCKFLOW - Funktion: LAGCalcLagrangeConcentration

   Aufgabe:
   Ermittelt den advektiven Schritt fuer eine Knotengroesse.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int c1:   Alte Zeitebene in Knotenindex c1
   E int c2:   Neue Zeitebene in Knotenindex c2

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
void LAGCalcLagrangeConcentration(int c1, int c2, char *bc_name1, char *bc_name2)
{
bc_name1 = bc_name1;
bc_name2 = bc_name2;
  /* Berechnet den advektiven Transport auf der Basis von c1 und
     speichert die Ergebnisse nach c2 */
  static long i;
//OK_BC  double konz;
  LAGRANGETRACE trace;

  lagrangecommon.c1 = c1;

  /* Schleife ueber alle Knoten */
  for (i = 0; i < NodeListLength; i++)
    {
//OK_IC      if (IsNodeBoundaryCondition(bc_name1, NodeNumber[i]))
        {
          //OK_BC konz = GetBoundaryConditionsValue(bc_name1, NodeNumber[i]);
          //OK_BC SetNodeVal(NodeNumber[i], c2, konz);
          if (lagrangecommon.use_matrix)
            {
              MXSet(i, i, 1.);         /* Randbedingungen haben sich selbst als Einfluss */
            }
        }
//OK_IC      if (IsNodeBoundaryCondition(bc_name2, NodeNumber[i]))
        {
          //OK_BC konz = GetBoundaryConditionsValue(bc_name2, NodeNumber[i]);
          //OK_BC SetNodeVal(NodeNumber[i], c2, konz);
          if (lagrangecommon.use_matrix)
            {
              MXSet(i, i, 1.);         /* Randbedingungen haben sich selbst als Einfluss */
            }
        }
    }

  for (i = 0; i < NodeListLength; i++)
    {
//OK_IC      if (!IsNodeBoundaryCondition(bc_name1, NodeNumber[i]) && !IsNodeBoundaryCondition(bc_name2, NodeNumber[i]))
        {                              /* regulaerer Innenknoten */
          lagrangecommon.konzsum = 0.;
          lagrangecommon.weightsum = 0.;
          lagrangecommon.startknoten = NodeNumber[i];
          lagrangecommon.number_of_steps = 0;

          trace.knoten = NodeNumber[i];
          trace.position = 0;          /* Auf Knoten */
          trace.restdt = dt;
          trace.ownweight = 1.;
          trace.slip = 0;

          LAGTraceStreamline2DRecursive(trace);
          /*          printf(" : %ld Schritte", lagrangecommon.number_of_steps); */
          if ((lagrangecommon.weightsum < lagrangecommon.quality) || (lagrangecommon.weightsum > (2. - lagrangecommon.quality)))
            printf("\nWARNUNG: Knoten %ld Gewichtssumme %f Konzsumme %f", NodeNumber[i], lagrangecommon.weightsum, lagrangecommon.konzsum);
          if (lagrangecommon.number_of_steps >= lagrangecommon.maximal_steps)
            printf("\nWARNUNG: Knoten %ld MAXSTEPS! Gewichtssumme %f Konzsumme %f", NodeNumber[i], lagrangecommon.weightsum, lagrangecommon.konzsum);
          ergebnis_LAG[i] = lagrangecommon.konzsum / (lagrangecommon.weightsum + MKleinsteZahl);

        }
    }

  /* Eintragen der Ergebnisse */
  TransferNodeVals(ergebnis_LAG, c2);

  return;
}

/************************************************************************
   ROCKFLOW - Funktion: LAGTraceStreamline2DRecursive

   Aufgabe:
   Rekursive Funktion zur Verfolgung einer Bahnlinie

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE trace:   Informationen ueber die aktuelle Position

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
double LAGTraceStreamline2DRecursive(LAGRANGETRACE trace)
{
  double q, q_sum, konz;
  long anz_aeste, i, j, k;
  int anz_elem, anz_alle_elem, anz_alle_kanten;
  long *elemente, *alle_elemente;
  long *kanten, *alle_kanten;

  LAGRANGETRACE endposition;
  LAGRANGETRACE recursivetrace;
  Knoten *knoten;
  Kante *kante;

  do
    {
      if (lagrangecommon.timeweighting)
        lagrangecommon.theta = 1. - trace.restdt / lagrangecommon.dt;
      else
        lagrangecommon.theta = 1.;
      lagrangecommon.number_of_steps++;
      LAGCopyTraceStructure(trace, &endposition);

      switch (trace.position)
        {

        case 0:                /* Auf Knoten */

          /* Startkoordinaten */
          knoten = GetNode(trace.knoten);

          trace.koor[0] = knoten -> x;
          trace.koor[1] = knoten -> y;
          trace.koor[2] = knoten -> z;

          q_sum = 0.;
          anz_aeste = 0;

          alle_kanten = LAGGetNode2DEdges(trace.knoten, &anz_alle_kanten);      /* Speicher spaeter wieder freigeben! */
          alle_elemente = LAGGetNode2DElements(trace.knoten, &anz_alle_elem);   /* Speicher spaeter wieder freigeben! */

          for (i = 0; i < anz_alle_elem; i++)
            {
              q = LAGCalcNodeFlow(alle_elemente[i], trace.knoten);      /* Kommt ein Fluss aus diesem Element ? */
              if (q > MKleinsteZahl)
                {                      /* Jo! */
                  q_sum += q;
                  anz_aeste++;

                  /* Die jetzt mit dem Element betrachteten Kanten koennen aus dem Kantenarray geloescht werden */
                  kanten = ElGetElementEdges(alle_elemente[i]);
                  for (j = 0; j < anz_alle_kanten; j++)
                    {
                      for (k = 0; k < 4; k++)
                        {
                          kante = GetEdge(kanten[k]);
                          if (alle_kanten[j] == kanten[k])
                            alle_kanten[j] = -1;
                          if (alle_kanten[j] == kante -> kinder[0])
                            alle_kanten[j] = -1;
                          if (alle_kanten[j] == kante -> kinder[1])
                            alle_kanten[j] = -1;
                        }              /* endfor */
                    }                  /* endfor */
                }                      /* endif */
            }                          /* endfor */

          for (i = 0; i < anz_alle_kanten; i++)
            {
              if (alle_kanten[i] >= 0)
                {
                  q = LAGCalcMiddleEdgeParallelFlow(trace.knoten, alle_kanten[i], trace.koor);  /* Kommt ein Fluss entlang dieser Kante ? */
                  if (q > MKleinsteZahl)
                    {
                      q_sum += q;
                      anz_aeste++;
                    }                  /* endif */
                }                      /* endif */
            }                          /* endfor */

          Free(alle_elemente);
          Free(alle_kanten);

          switch (anz_aeste)
            {
            case 0:
              /*              printf("\nKein Oberstromelelement!: x=%f y=%f z=%f restdt=%f", trace.koor[0], trace.koor[1], trace.koor[2], endposition.restdt); */
              endposition.restdt = 0.;
              endposition.position = 2;
              elemente = LAGGetNode2DElements(trace.knoten, &anz_elem);
              endposition.flaeche = elemente[0];
              Free(elemente);
              break;

            default:
              alle_kanten = LAGGetNode2DEdges(trace.knoten, &anz_alle_kanten);  /* Speicher spaeter wieder freigeben! */
              alle_elemente = LAGGetNode2DElements(trace.knoten, &anz_alle_elem);       /* Speicher spaeter wieder freigeben! */

              for (i = 0; i < anz_alle_elem; i++)
                {
                  q = LAGCalcNodeFlow(alle_elemente[i], trace.knoten);  /* Kommt ein Fluss aus diesem Element ? */
                  if (q > MKleinsteZahl)
                    {                  /* Jo! */
                      kanten = ElGetElementEdges(alle_elemente[i]);
                      LAGCopyTraceStructure(trace, &recursivetrace);

                      recursivetrace.position = 2;
                      recursivetrace.kante = -1;
                      recursivetrace.flaeche = alle_elemente[i];
                      recursivetrace.ownweight = q / q_sum * trace.ownweight;

                      if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                        LAGTraceStreamline2DRecursive(recursivetrace);

                      /* Die jetzt mit dem Element betrachteten Kanten koennen aus dem Kantenarray geloescht werden */
                      for (j = 0; j < anz_alle_kanten; j++)
                        {
                          for (k = 0; k < 4; k++)
                            {
                              kante = GetEdge(kanten[k]);
                              if (alle_kanten[j] == kanten[k])
                                alle_kanten[j] = -1;
                              if (alle_kanten[j] == kante -> kinder[0])
                                alle_kanten[j] = -1;
                              if (alle_kanten[j] == kante -> kinder[1])
                                alle_kanten[j] = -1;
                            }          /* endfor */
                        }              /* endfor */
                    }                  /* endif */
                }                      /* endfor */

              for (i = 0; i < anz_alle_kanten; i++)
                {                      /* Jetzt kommen noch die Krueppelkanten ... */
                  if (alle_kanten[i] >= 0)
                    {
                      q = LAGCalcMiddleEdgeParallelFlow(trace.knoten, alle_kanten[i], trace.koor);
                      if (q > MKleinsteZahl)
                        {
                          elemente = LAGGetEdge2DElements(alle_kanten[i], &anz_elem);
                          trace.position = 1;
                          trace.kante = alle_kanten[i];
                          trace.flaeche = elemente[0];
                          Free(elemente);
                          LAGCopyTraceStructure(trace, &recursivetrace);
                          recursivetrace.ownweight = q / q_sum * trace.ownweight;
                          recursivetrace.slip = 1;
                          if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                            LAGTraceStreamline2DRecursive(recursivetrace);
                          recursivetrace.slip = 0;
                        }
                    }
                }                      /* endfor */
              endposition.ownweight = 0.;       /* Das Gewicht wird durch die Kinder uebernommen, die Kinder  speisen ihre Ergebnisse in *konz_sum und *weight_sum ein */
              endposition.restdt = 0.; /* Die Laufzeit wird durch die Kinder uebernommen */
              Free(alle_elemente);
              Free(alle_kanten);
              break;
            }

          break;

        case 1:                /* Auf Kante */
          alle_elemente = LAGGetEdgeNext2DElements(trace.kante, &anz_alle_elem, trace.koor);    /* Speicher spaeter wieder freigeben! */
          q_sum = 0.;
          anz_aeste = 0;
          trace.flaeche = alle_elemente[0];     /* Einen sinnvollen Wert zuweisen */
          for (i = 0; i < anz_alle_elem; i++)
            {
              q = LAGCalcEdgeFlow(alle_elemente[i], trace.kante, trace.koor);
              if (q > MKleinsteZahl)
                {
                  trace.slip = 0;
                  trace.flaeche = alle_elemente[i];     /* Dieses Element ist ein moegliches Startelement */
                  q_sum += q;
                  anz_aeste++;
                }
            }

          switch (anz_aeste)
            {
            case 0:                    /* Kein Ast */
              /*              printf("\nKein Oberstromelelement!: x=%f y=%f z=%f restdt=%f", trace.koor[0], trace.koor[1], trace.koor[2], endposition.restdt); */
              trace.slip = 1;
            case 1:                    /* Ein Ast */
              endposition.ownweight = trace.ownweight;
              konz = LAGTraceStreamlineThrough2DElement(trace, &endposition);
              trace.slip = 0;
              endposition.slip = 0;
              break;

            default:                   /* Viele Aeste */
              for (i = 0; i < anz_alle_elem; i++)
                {
                  q = LAGCalcEdgeFlow(alle_elemente[i], trace.kante, trace.koor);
                  if (q > MKleinsteZahl)
                    {
                      LAGCopyTraceStructure(trace, &recursivetrace);
                      recursivetrace.position = 2;
                      recursivetrace.kante = -1;
                      recursivetrace.flaeche = alle_elemente[i];
                      recursivetrace.ownweight = q / q_sum * trace.ownweight;
                      if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                        LAGTraceStreamline2DRecursive(recursivetrace);
                    }
                }
              endposition.ownweight = 0.;       /* Das Gewicht wird durch die Kinder uebernommen, die Kinder  speisen ihre Ergebnisse in *konz_sum und *weight_sum ein */
              endposition.restdt = 0.; /* Die Laufzeit wird durch die Kinder uebernommen */
              break;
            }
          Free(alle_elemente);
          break;

        case 2:                /* Auf Flaeche -> es wird immer nur _eine_ Kante gefunden */
          konz = LAGTraceStreamlineThrough2DElement(trace, &endposition);
          break;

        default:
          LAGCopyTraceStructure(trace, &endposition);
          printf("\nPanic! Invalid position!");
          return 0.;
        }

      if (lagrangecommon.number_of_steps > lagrangecommon.maximal_steps)
        {
          /*          printf("\n Zu viele Schritte!"); */
          endposition.restdt = 0.;
        }
      LAGCopyTraceStructure(endposition, &trace);
    }
  while ((trace.restdt / lagrangecommon.dt) > MKleinsteZahl);   /* Gehe weiter bis Laufzeit verbraucht ist */

  if (endposition.ownweight > lagrangecommon.minimalweight)
    LAGSetResults(endposition);

  return 0.;
}

/************************************************************************
   ROCKFLOW - Funktion: LAGTraceStreamline2DStackBased

   Aufgabe:
   Funktion zur Verfolgung einer Bahnlinie, basier auf Stack

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE trace:   Informationen ueber die aktuelle Position

   Ergebnis:

   Programmaenderungen:
   4/2000    C. Thorenz  Erste Version

************************************************************************/
double LAGTraceStreamline2DStackBased(LAGRANGETRACE trace)
{
  double q, q_sum, konz;
  long anz_aeste, i, j, k;
  int anz_elem, anz_alle_elem, anz_alle_kanten;
  long *elemente, *alle_elemente;
  long *kanten, *alle_kanten;

  LAGRANGETRACE endposition;
  LAGRANGETRACE recursivetrace;
  Knoten *knoten;
  Kante *kante;

  LAGAddTraceStructureToStack(trace);
  LAGCopyTraceStructure(trace, &endposition);

  while (LAGGetTraceStructureFromStack(&trace))
    {

      if (lagrangecommon.timeweighting)
        lagrangecommon.theta = 1. - trace.restdt / lagrangecommon.dt;
      else
        lagrangecommon.theta = 1.;
      lagrangecommon.number_of_steps++;
      LAGCopyTraceStructure(trace, &endposition);

      switch (trace.position)
        {

        case 0:                /* Auf Knoten */

          /* Startkoordinaten */
          knoten = GetNode(trace.knoten);

          trace.koor[0] = knoten -> x;
          trace.koor[1] = knoten -> y;
          trace.koor[2] = knoten -> z;

          q_sum = 0.;
          anz_aeste = 0;

          alle_kanten = LAGGetNode2DEdges(trace.knoten, &anz_alle_kanten);      /* Speicher spaeter wieder freigeben! */
          alle_elemente = LAGGetNode2DElements(trace.knoten, &anz_alle_elem);   /* Speicher spaeter wieder freigeben! */

          for (i = 0; i < anz_alle_elem; i++)
            {
              q = LAGCalcNodeFlow(alle_elemente[i], trace.knoten);      /* Kommt ein Fluss aus diesem Element ? */
              if (q > MKleinsteZahl)
                {                      /* Jo! */
                  q_sum += q;
                  anz_aeste++;

                  /* Die jetzt mit dem Element betrachteten Kanten koennen aus dem Kantenarray geloescht werden */
                  kanten = ElGetElementEdges(alle_elemente[i]);
                  for (j = 0; j < anz_alle_kanten; j++)
                    {
                      for (k = 0; k < 4; k++)
                        {
                          kante = GetEdge(kanten[k]);
                          if (alle_kanten[j] == kanten[k])
                            alle_kanten[j] = -1;
                          if (alle_kanten[j] == kante -> kinder[0])
                            alle_kanten[j] = -1;
                          if (alle_kanten[j] == kante -> kinder[1])
                            alle_kanten[j] = -1;
                        }              /* endfor */
                    }                  /* endfor */
                }                      /* endif */
            }                          /* endfor */

          for (i = 0; i < anz_alle_kanten; i++)
            {
              if (alle_kanten[i] >= 0)
                {
                  q = LAGCalcMiddleEdgeParallelFlow(trace.knoten, alle_kanten[i], trace.koor);  /* Kommt ein Fluss entlang dieser Kante ? */
                  if (q > MKleinsteZahl)
                    {
                      q_sum += q;
                      anz_aeste++;
                    }                  /* endif */
                }                      /* endif */
            }                          /* endfor */

          Free(alle_elemente);
          Free(alle_kanten);

          switch (anz_aeste)
            {
            case 0:
              /*              printf("\nKein Oberstromelelement!: x=%f y=%f z=%f restdt=%f", trace.koor[0], trace.koor[1], trace.koor[2], endposition.restdt); */
              endposition.restdt = 0.;
              endposition.position = 2;
              elemente = LAGGetNode2DElements(trace.knoten, &anz_elem);
              endposition.flaeche = elemente[0];
              Free(elemente);
              break;

            default:
              alle_kanten = LAGGetNode2DEdges(trace.knoten, &anz_alle_kanten);  /* Speicher spaeter wieder freigeben! */
              alle_elemente = LAGGetNode2DElements(trace.knoten, &anz_alle_elem);       /* Speicher spaeter wieder freigeben! */

              for (i = 0; i < anz_alle_elem; i++)
                {
                  q = LAGCalcNodeFlow(alle_elemente[i], trace.knoten);  /* Kommt ein Fluss aus diesem Element ? */
                  if (q > MKleinsteZahl)
                    {                  /* Jo! */
                      kanten = ElGetElementEdges(alle_elemente[i]);
                      LAGCopyTraceStructure(trace, &recursivetrace);

                      recursivetrace.position = 2;
                      recursivetrace.kante = -1;
                      recursivetrace.flaeche = alle_elemente[i];
                      recursivetrace.ownweight = q / q_sum * trace.ownweight;

                      if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                        LAGAddTraceStructureToStack(recursivetrace);

                      /* Die jetzt mit dem Element betrachteten Kanten koennen aus dem Kantenarray geloescht werden */
                      for (j = 0; j < anz_alle_kanten; j++)
                        {
                          for (k = 0; k < 4; k++)
                            {
                              kante = GetEdge(kanten[k]);
                              if (alle_kanten[j] == kanten[k])
                                alle_kanten[j] = -1;
                              if (alle_kanten[j] == kante -> kinder[0])
                                alle_kanten[j] = -1;
                              if (alle_kanten[j] == kante -> kinder[1])
                                alle_kanten[j] = -1;
                            }          /* endfor */
                        }              /* endfor */
                    }                  /* endif */
                }                      /* endfor */

              for (i = 0; i < anz_alle_kanten; i++)
                {                      /* Jetzt kommen noch die Krueppelkanten ... */
                  if (alle_kanten[i] >= 0)
                    {
                      q = LAGCalcMiddleEdgeParallelFlow(trace.knoten, alle_kanten[i], trace.koor);
                      if (q > MKleinsteZahl)
                        {
                          elemente = LAGGetEdge2DElements(alle_kanten[i], &anz_elem);
                          trace.position = 1;
                          trace.kante = alle_kanten[i];
                          trace.flaeche = elemente[0];
                          Free(elemente);
                          LAGCopyTraceStructure(trace, &recursivetrace);
                          recursivetrace.ownweight = q / q_sum * trace.ownweight;
                          recursivetrace.slip = 1;
                          if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                            LAGAddTraceStructureToStack(recursivetrace);

                        }
                    }
                }                      /* endfor */
              endposition.ownweight = 0.;       /* Das Gewicht wird durch die Kinder uebernommen, die Kinder  speisen ihre Ergebnisse in *konz_sum und *weight_sum ein */
              endposition.restdt = 0.; /* Die Laufzeit wird durch die Kinder uebernommen */
              Free(alle_elemente);
              Free(alle_kanten);
              break;
            }

          break;

        case 1:                /* Auf Kante */
          alle_elemente = LAGGetEdgeNext2DElements(trace.kante, &anz_alle_elem, trace.koor);    /* Speicher spaeter wieder freigeben! */
          q_sum = 0.;
          anz_aeste = 0;
          trace.flaeche = alle_elemente[0];     /* Einen sinnvollen Wert zuweisen */
          for (i = 0; i < anz_alle_elem; i++)
            {
              q = LAGCalcEdgeFlow(alle_elemente[i], trace.kante, trace.koor);
              if (q > MKleinsteZahl)
                {
                  trace.slip = 0;
                  trace.flaeche = alle_elemente[i];     /* Dieses Element ist ein moegliches Startelement */
                  q_sum += q;
                  anz_aeste++;
                }
            }

          switch (anz_aeste)
            {
            case 0:                    /* Kein Ast */
              /*              printf("\nKein Oberstromelelement!: x=%f y=%f z=%f restdt=%f", trace.koor[0], trace.koor[1], trace.koor[2], endposition.restdt); */
              trace.slip = 1;
            case 1:                    /* Ein Ast */
              endposition.ownweight = trace.ownweight;
              konz = LAGTraceStreamlineThrough2DElement(trace, &endposition);
              trace.slip = 0;
              break;

            default:                   /* Viele Aeste */
              for (i = 0; i < anz_alle_elem; i++)
                {
                  q = LAGCalcEdgeFlow(alle_elemente[i], trace.kante, trace.koor);
                  if (q > MKleinsteZahl)
                    {
                      LAGCopyTraceStructure(trace, &recursivetrace);
                      recursivetrace.position = 2;
                      recursivetrace.kante = -1;
                      recursivetrace.flaeche = alle_elemente[i];
                      recursivetrace.ownweight = q / q_sum * trace.ownweight;
                      if (recursivetrace.ownweight > lagrangecommon.minimalweight)
                        LAGAddTraceStructureToStack(recursivetrace);
                    }
                }
              endposition.ownweight = 0.;       /* Das Gewicht wird durch die Kinder uebernommen, die Kinder  speisen ihre Ergebnisse in *konz_sum und *weight_sum ein */
              endposition.restdt = 0.; /* Die Laufzeit wird durch die Kinder uebernommen */
              break;
            }
          Free(alle_elemente);
          break;

        case 2:                /* Auf Flaeche -> es wird immer nur _eine_ Kante gefunden */
          konz = LAGTraceStreamlineThrough2DElement(trace, &endposition);
          break;

        default:
          LAGCopyTraceStructure(trace, &endposition);
          printf("\nPanic! Invalid position!");
          return 0.;
        }

      if (lagrangecommon.number_of_steps > lagrangecommon.maximal_steps)
        {
          /*          printf("\n Zu viele Schritte!"); */
          endposition.restdt = 0.;
        }
      LAGCopyTraceStructure(endposition, &trace);
    }                                  /* Gehe weiter bis der Stack leer ist */

  if (endposition.ownweight > lagrangecommon.minimalweight)
    LAGSetResults(endposition);

  return 0.;
}

/************************************************************************
   ROCKFLOW - Funktion: LAGCalcVectorDependence

   Aufgabe:
   Ermittelt die lineare Abhaengigkeit von drei Vektoren im Raum.

   Geloest wird: a*vec1 + b*vec2 = vec3

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *vec1: Zeiger auf ersten Vektor
   E double *vec2: Zeiger auf zweiten Vektor
   E double *vec3: Zeiger auf dritten Vektor
   E double a: Ermittelter Multiplikator
   E double b: Ermittelter Multiplikator

   Ergebnis:

   0: Es ist ein Fehler aufgetreten
   1: Alles klar

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
int LAGCalcVectorDependence(double *vec1, double *vec2, double *vec3, double *a, double *b)
{
  static double det1_n, det2_n, det3_n, D;      /* Determinanten eines 2d-Hilfssystems */

  /* Eine der Gleichungen sollte ueberfluessig sein ... */

  *a = 0.;
  *b = 0.;

  det1_n = vec1[1] * vec2[2] - vec1[2] * vec2[1];
  det2_n = vec1[0] * vec2[2] - vec1[2] * vec2[0];
  det3_n = vec1[0] * vec2[1] - vec1[1] * vec2[0];

  /* Erste Gleichung weggelassen */
  if ((fabs(det1_n) >= fabs(det2_n))
      && (fabs(det1_n) >= fabs(det3_n) && (fabs(det1_n) > 0.)))
    {
      D = vec3[1] * vec2[2] - vec3[2] * vec2[1];
      *a = D / det1_n;
      D = vec1[1] * vec3[2] - vec1[2] * vec3[1];
      *b = D / det1_n;
      return 1;
    }
  /* Zweite Gleichung weggelassen */
  if ((fabs(det2_n) >= fabs(det1_n))
      && (fabs(det2_n) >= fabs(det3_n) && (fabs(det2_n) > 0.)))
    {
      D = vec3[0] * vec2[2] - vec3[2] * vec2[0];
      *a = D / det2_n;
      D = vec1[0] * vec3[2] - vec1[2] * vec3[0];
      *b = D / det2_n;
      return 1;
    }
  /* Dritte Gleichung weggelassen */
  if ((fabs(det3_n) >= fabs(det1_n))
      && (fabs(det3_n) >= fabs(det2_n) && (fabs(det3_n) > 0.)))
    {
      D = vec3[0] * vec2[1] - vec3[1] * vec2[0];
      *a = D / det3_n;
      D = vec1[0] * vec3[1] - vec1[1] * vec3[0];
      *b = D / det3_n;
      return 1;
    }
  /* Irgendwie passt gar nix ... */
  return 0;
}

/************************************************************************
   ROCKFLOW - Funktion: LAGGet2DElementFilterVelocity

   Aufgabe:
   Ermittelt die Filtergeschwindigkeit fuer ein Element.

   Die Globalvariable
     lagrangecommon.use_velocity_in_gauss_points
   steuert, ob die Geschwindigkeiten neu berechnet oder aus den
   bereits vorhandenen Gauss-Geschwindigkeiten ermittelt werden soll.

   Die Globalvariable
      lagrangecommon.for_velocity_calculation_use_real_koordinates
   steuert, ob die Geschwindigkeiten auf den aktuellen Koordinaten
   oder in der Elementmitte berechnet werden sollen.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long element: Elementnummer
   E double theta: Wichtungsfaktor zwischen Zeitebenen
   E double *koor: Aktuelle Koordinaten (xyz)

   R double *velocity: Ermittelte Geschwindigkeit (xyz)
                       Speicher muss von der aufrufenden Funktion allokiert werden.

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
void LAGGet2DElementFilterVelocity(long element, double theta, double *koor, double *velocity)
{
  static long i;
  static int anzgp;
  static double trans[6];       /* Drehmatrix */
  static double vector[12];     /* Vektorfeld */
  static double *velovec, veloab[2];
  static double koor_ab[2], koor_rs[2];

  theta = theta;

  switch (lagrangecommon.velocity_calculation)
    {

    case 0:
      anzgp = GetNumericsGaussPoints(ElGetElementType(element));

      /* Abstandsgeschwindigkeiten in den Gausspunkten */
      velovec = ElGetVelocity(element);
      veloab[0] = 0.;
      veloab[1] = 0.;
      /* Mittelwert bilden */
      for (i = 0; i < (anzgp * anzgp); i++)
        {
          veloab[0] += velovec[i * 2];
          veloab[1] += velovec[i * 2 + 1];
        }
      veloab[0] /= (double) (anzgp * anzgp);
      veloab[1] /= (double) (anzgp * anzgp);
      break;

    case 1:
      /* Koordinaten des tatsaechlichen Aufenthaltsortes */
      Calc2DElement_xyz2ab_Coord(element, koor_ab, koor[0], koor[1], koor[2]);
      Calc2DElement_ab2rs_Coord(element, koor_rs, koor_ab[0], koor_ab[1]);

      CalcVelo2Drs(lagrangecommon.phase, element, lagrangecommon.theta, MRange(-1.,koor_rs[0],1.), MRange(-1.,koor_rs[1],1.), 0, veloab);
      break;
    default:
    case 2:

      CalcVelo2Drs(lagrangecommon.phase, element, lagrangecommon.theta, 0., 0., 0, veloab);
      break;

    }
  /* Transformation der Geschwindigkeitsvektoren in das globale physikalische Koordinatensystem (x,y,z) */
  CalcTransformationsMatrix(element, vector, trans);
  MMultMatVec(trans, 3, 2, veloab, 2, velocity, 3);

  return;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGTraceStreamlineThrough2DElement

   Aufgabe:
   Ermittelt den Weg durch ein Element

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE startposition: Vorheriger Aufenthaltsort.
   R LAGRANGETRACE *endposition:  Neuer Aufenthaltsort.
   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/

double LAGTraceStreamlineThrough2DElement(LAGRANGETRACE startposition, LAGRANGETRACE * endposition)
{
  static long i, j;
  static long startknoten0, startknoten1;
  static long *elekanten;       /* Feld mit den Elementkanten */
  static long massgeb_knoten, massgeb_kante, massgeb_element;
  static double massgeb_dt, massgeb_b;
  static double massgeb_qualitaet, qualitaet;
  static double a, b, konz;
  static double velo[3], vec2[3], vec3[3];
  static Knoten *knoten0 = NULL, *knoten1 = NULL;       /* Zeiger auf Knoten */
  static Kante *kante = NULL;   /* Zeiger auf Kante */

  static double porosity;

  LAGCopyTraceStructure(startposition, endposition);

  massgeb_dt = -1.e99;
  massgeb_kante = -1;
  massgeb_qualitaet = -1;
  massgeb_element = startposition.flaeche;

  if (startposition.kante != -1)
    {
      kante = GetEdge(startposition.kante);
      startknoten0 = kante -> knoten[0];
      startknoten1 = kante -> knoten[1];
    }
  else
    {
      startknoten0 = -1;
      startknoten1 = -1;
    }                                  /* endif */

  if (startposition.slip)
    {
      /* Der Geschwindigkeitsvektor "rutscht" an der Kante entlang. Diese Kante ist vermutlich ein Aussenrand. */
      /* Die "Berechnung" wird ziemlich einfach  */
      LAGGet2DNegativeEdgeParallelVelocity(startposition.kante, startposition.koor, &(velo[0]));

      /* Vektoren vom Untersuchungspunkt zu den Knoten */

      knoten0 = GetNode(kante -> knoten[0]);
      knoten1 = GetNode(kante -> knoten[1]);

      vec2[0] = knoten0 -> x - startposition.koor[0];
      vec2[1] = knoten0 -> y - startposition.koor[1];
      vec2[2] = knoten0 -> z - startposition.koor[2];

      vec3[0] = knoten1 -> x - startposition.koor[0];
      vec3[1] = knoten1 -> y - startposition.koor[1];
      vec3[2] = knoten1 -> z - startposition.koor[2];

      if (MSkalarprodukt(vec2, velo, 3) > MSkalarprodukt(vec3, velo, 3))
        massgeb_knoten = startknoten0;
      else
        massgeb_knoten = startknoten1;

      knoten0 = GetNode(massgeb_knoten);

      vec2[0] = knoten0 -> x - startposition.koor[0];
      vec2[1] = knoten0 -> y - startposition.koor[1];
      vec2[2] = knoten0 -> z - startposition.koor[2];

      massgeb_dt = MBtrgVec(vec2, 3) / MBtrgVec(velo, 3);

      if (massgeb_dt > startposition.restdt)
        {

          (*endposition).koor[0] = startposition.koor[0] + startposition.restdt * velo[0];
          (*endposition).koor[1] = startposition.koor[1] + startposition.restdt * velo[1];
          (*endposition).koor[2] = startposition.koor[2] + startposition.restdt * velo[2];
          (*endposition).restdt = 0.;
        }
      else
        {
          (*endposition).knoten = massgeb_knoten;
          (*endposition).koor[0] = knoten0 -> x;
          (*endposition).koor[1] = knoten0 -> y;
          (*endposition).koor[2] = knoten0 -> z;
          (*endposition).position = 0;
          (*endposition).restdt = startposition.restdt - massgeb_dt;
        }                              /* endif */

      return 0;
    }
  /* Ab hier kommt der Standardfall: */

  LAGGet2DElementFilterVelocity(startposition.flaeche, lagrangecommon.theta, startposition.koor, &(velo[0]));
  /*  Auf negative Abstandsgeschw. umrechnen */
  porosity = GetSoilPorosity(startposition.flaeche);

  for (i = 0; i < 3; i++)
    velo[i] /= -porosity;

  elekanten = ElGetElementEdges(startposition.flaeche);

  for (j = 0; j < 4; j++)
    {                                  /* Schleife ueber alle Kanten */
      if (elekanten[j] != startposition.kante)
        {                              /* Nur die anderen Kanten zulassen */
          kante = GetEdge(elekanten[j]);
          knoten0 = GetNode(kante -> knoten[0]);
          knoten1 = GetNode(kante -> knoten[1]);

          /* Ueber die Vektoren 2 und 3 wird ein Koordinatensystem aufgestellt */

          /* Vektor zur Kantenbeschreibung */
          vec2[0] = knoten0 -> x - knoten1 -> x;
          vec2[1] = knoten0 -> y - knoten1 -> y;
          vec2[2] = knoten0 -> z - knoten1 -> z;

          /* Vektor vom Untersuchungspunkt zur Kante */
          vec3[0] = knoten0 -> x - startposition.koor[0];
          vec3[1] = knoten0 -> y - startposition.koor[1];
          vec3[2] = knoten0 -> z - startposition.koor[2];
          if (LAGCalcVectorDependence(velo, vec2, vec3, &a, &b))
            {                          /* Zu loesen: a*veloglob + b*vec2  = vec3  */
              qualitaet = 1. - 1. / (0.5 + lagrangecommon.local_eps) * fabs(b - 0.5);

              /* Schnittpunkt liegt eigentlich ausserhalb des Elements, wir ziehen ihn auf den Knoten und markieren die schlechte "Qualitaet" */
              if ((fabs(b) < lagrangecommon.local_eps) && (b < 0.))
                b = 0.;
              if ((fabs(b - 1.) < lagrangecommon.local_eps) && (b > 1.))
                b = 1.;

              if ((b >= -lagrangecommon.local_eps) && (b <= 1.) &&      /* Schnittpunkt liegt auf Elementkante, nicht ausserhalb */
                  (qualitaet * a > massgeb_qualitaet))
                {                      /* Es gibt noch nichts besseres */
                  massgeb_dt = a;
                  massgeb_b = b;
                  massgeb_kante = j;
                  massgeb_qualitaet = qualitaet * a;
                  massgeb_element = startposition.flaeche;
                  (*endposition).position = 1;  /* Auf Kante */
                }
            }
        }
    }                                  /* Ende Schleife ueber alle Kanten */

  if (massgeb_dt < 0.)
    {                                  /* Es wurde keine Schnittkante gefunden */
      /*      printf("\nBeende Verfolgung von Knoten %ld in Element %ld x=%f y=%f z=%f", lagrangecommon.startknoten, startposition.flaeche, startposition.koor[0], startposition.koor[1], startposition.koor[2]); */
      massgeb_dt = 0.;
      (*endposition).restdt = 0.;
      (*endposition).kante = startposition.kante;
      (*endposition).flaeche = startposition.flaeche;
      (*endposition).position = 1;     /* Auf Kante */
      (*endposition).ownweight = 0.;
      return 0;
    }
  else
    {
      /* Naechstes Element setzen */
      elekanten = ElGetElementEdges(massgeb_element);
      (*endposition).kante = elekanten[massgeb_kante];
      (*endposition).flaeche = massgeb_element;

    }
  if (startposition.restdt <= massgeb_dt)
    {
      (*endposition).position = 2;     /* Auf der Flaeche */

      for (i = 0; i < 3; i++)
        (*endposition).koor[i] = startposition.koor[i] + startposition.restdt * velo[i];
      (*endposition).restdt = 0.;
    }
  else
    {
      (*endposition).position = 1;     /* Auf Kante */

      /* Reisezeit bis zur naechsten Kante von Restzeit abziehen */
      (*endposition).restdt = startposition.restdt - massgeb_dt;

      kante = GetEdge((*endposition).kante);

      knoten0 = GetNode(kante -> knoten[0]);
      knoten1 = GetNode(kante -> knoten[1]);

      /* Vektor zur Kantenbeschreibung */
      vec2[0] = knoten0 -> x - knoten1 -> x;
      vec2[1] = knoten0 -> y - knoten1 -> y;
      vec2[2] = knoten0 -> z - knoten1 -> z;

      /* Neue Position */
      (*endposition).koor[0] = knoten0 -> x - massgeb_b * vec2[0];
      (*endposition).koor[1] = knoten0 -> y - massgeb_b * vec2[1];
      (*endposition).koor[2] = knoten0 -> z - massgeb_b * vec2[2];

      if ((massgeb_b < MKleinsteZahl) && (kante -> knoten[0] != startknoten0)
          && (kante -> knoten[0] != startknoten1))
        {
          (*endposition).knoten = kante -> knoten[0];
          (*endposition).position = 0; /* Auf Knoten */
        }
      if ((massgeb_b > 1.-MKleinsteZahl) && (kante -> knoten[1] != startknoten0)
          && (kante -> knoten[1] != startknoten1))
        {
          (*endposition).knoten = kante -> knoten[1];
          (*endposition).position = 0; /* Auf Knoten */
        }
    }                                  /* endif */
  /* Den naechsten auswaehlen */

  return konz;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGSetResults
   Aufgabe:
   Ermittelt die Konzentration fuer den Aufenthaltsort
   und speichert die ermittelten Ergebnisse fuer einen Knoten
   ab.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE endposition: Position

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/

void LAGSetResults(LAGRANGETRACE endposition)
{
  double koor_ab[2], koor_rs[2];
  long i, *nodes;
  double c[4];

  /* Lokale Koordinaten bestimmen */
  Calc2DElement_xyz2ab_Coord(endposition.flaeche, koor_ab, endposition.koor[0], endposition.koor[1], endposition.koor[2]);
  Calc2DElement_ab2rs_Coord(endposition.flaeche, koor_rs, koor_ab[0], koor_ab[1]);

  /* Konzentration mit dem Gewicht fuer den Knoten anschreiben */
  lagrangecommon.konzsum += InterpolValue(endposition.flaeche, lagrangecommon.c1, koor_rs[0], koor_rs[1], 0.) * endposition.ownweight;

  /* Als Kontrolle die Gewichte auch summieren */
  lagrangecommon.weightsum += endposition.ownweight;

  if (lagrangecommon.use_matrix)
    {
      /* Die "Einflussfaktoren" (Wert der Ansatzfunktionen*Gewicht) der einzelnen Knoten auf das Ergebis werden in einer Matrix abgelegt */
      nodes = ElGetElementNodes(endposition.flaeche);
      for (i = 0; i < 4; i++)
        {
          MOmega2D(&c[0], koor_rs[0], koor_rs[1]);
          MXInc(GetNodeIndex(lagrangecommon.startknoten), GetNodeIndex(nodes[i]), c[i] * endposition.ownweight);
        }
    }

  return;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGCalcEdgeFlow

   Aufgabe:
   Ermittelt den Volumenstrom pro Laengenmeter ueber eine Kante.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long element: betrachtetes Element
   E long edge:    betrachtete Kante
   E double *koor: aktuelle Position

   Ergebnis:
   double : Volumenstrom pro Laengenmeter

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
double LAGCalcEdgeFlow(long element, long edge, double *koor)
{
  static double vel[3], vel_n;
  static double n[3];
  static Kante *kante;
  static long massgeb_kante, i;
  static long *edges;
  static double area;

  area = GetElementExchangeArea(element);

  kante = GetEdge(edge);
  edges = ElGetElementEdges(element);

  massgeb_kante = -1;

  /* Liegt die Kante an diesem Element ? */
  for (i = 0; i < 4; i++)
    {
      if (edge == edges[i])
        massgeb_kante = edge;
    }                                  /* endfor */

  /* Die Kante scheint nicht am Element zu liegen. Jetzt probieren wir die Kinder/Eltern */
  if (massgeb_kante == -1)
    {
      for (i = 0; i < 4; i++)
        {
          if (kante -> kinder[0] == edges[i])
            massgeb_kante = kante -> kinder[0];
          if (kante -> kinder[1] == edges[i])
            massgeb_kante = kante -> kinder[1];
          if (kante -> vorgaenger == edges[i])
            massgeb_kante = kante -> vorgaenger;
        }                              /* endfor */
    }
  /* endif */
  /* Einheitsnormalenvektor fuer diese Kante bestimmen */
  Calc2DEdgeUnitNormalVec(element, massgeb_kante, n);

  LAGGet2DElementFilterVelocity(element, lagrangecommon.theta, koor, &(vel[0]));

  /* Geschwindigkeit in Normalenrichtung berechnen */
  vel_n = MSkalarprodukt(vel, n, 3);

  /* q (Durchfluss pro Laengenmeter der Kante) auf Kante berechnen */
  return area * vel_n;                 /* mat->area ist die Dicke! */
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGCalcNodeFlow

   Aufgabe:
   Ermittelt den Volumenstrom pro Breitenmeter zu einem Knoten

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long element: betrachtetes Element
   E long node: betrachteter Knoten

   Ergebnis:
   double : Volumenstrom pro Breitenmeter

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
double LAGCalcNodeFlow(long element, long node)
{
  static double vel[3], vel_n, a, b;
  static double vec1[3], vec2[3], koor[3];
  static int int_c;
  static long i, j, n1, n2, *nodes, *edges, edge;
  static Knoten *knoten;
  static Kante *kante, *kinderkante;
  static double area;

  area = GetElementExchangeArea(element);

  koor[0] = GetNodeX(node);
  koor[1] = GetNodeY(node);
  koor[2] = GetNodeZ(node);

  /* Filtergeschwindigkeit in der Elementmitte ausrechnen */
  LAGGet2DElementFilterVelocity(element, lagrangecommon.theta, koor, &(vel[0]));

  vel_n = MBtrgVec(vel, 3);            /* Betrag (Laenge)  des Vektors */

  knoten = GetNode(node);

  nodes = ElGetElementNodes(element);
  /* Der untersuchte Knoten ist der j_te Knoten des Elements */
  for (j = 0; ((j < 4) && (nodes[j] != node)); j++)
    {
    };

  if (j < 4)
    {                                  /* Der Knoten liegt auf der Ecke des Elements. Es handelt sich um einen regulaeren Knoten */
      /* Vektoren zu den Nachbarknoten ermitteln */
      if (j > 0)
        n1 = nodes[j - 1];
      else
        n1 = nodes[3];
      if (j < 3)
        n2 = nodes[j + 1];
      else
        n2 = nodes[0];

      /* Bestimmen des Oeffnungswinkels des aufgespannten Systems */
      vec1[0] = GetNodeX(n1) - GetNodeX(node);
      vec1[1] = GetNodeY(n1) - GetNodeY(node);
      vec1[2] = GetNodeZ(n1) - GetNodeZ(node);
      vec2[0] = GetNodeX(n2) - GetNodeX(node);
      vec2[1] = GetNodeY(n2) - GetNodeY(node);
      vec2[2] = GetNodeZ(n2) - GetNodeZ(node);

      int_c = LAGCalcVectorDependence(vec1, vec2, vel, &a, &b);         /* a*vec1+b*vec2 = vel */
      if ((int_c) && (a < -lagrangecommon.local_eps)
          && (b < -lagrangecommon.local_eps))
        {
          /* Der Geschw.vektor laesst sich als Linearkombination der Randvektoren fuer dieses Element darstellen,
             also kommt er aus diesem Element */
          return area * vel_n;         /* mat->area ist die Dicke! */
        }
      else
        return 0.;                     /* Kein Volumenstrom aus diesem Element */
    }
  else
    {                                  /* Es handelt sich vermutlich um einen irreguelaeren Knoten */
      /* Hole alle Kanten des Elements */
      edges = ElGetElementEdges(element);
      /* Finde die richtige Kante */
      for (i = 0; i < 4; i++)
        {
          edge = edges[i];
          kante = GetEdge(edge);

          if (kante -> kinder[0] > -1)
            {                          /* Es gibt Kinder */
              kinderkante = GetEdge(kante -> kinder[0]);
              if ((kinderkante -> knoten[0] == node)
                  || (kinderkante -> knoten[1] == node))
                {
                  /* Der irregulaere Knoten liegt auf dieser Vater-Kante */
                  i = 99;              /* Schleife beenden */
                }                      /* endif */
            }                          /* endif */
        }                              /* endfor */

      /* Ueberpruefe, ob der Knoten an einem der Kinder haengt, ggf Fehler */
      if (i > 5)
        {
          return LAGCalcEdgeFlow(element, edge, koor);
        }
      else
        {
          printf("\n PANIC in LAGCalcNodeFlow!");
          return 0.;
        }                              /* endif */
    }
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGCopyTraceStructure

   Aufgabe:

   Umkopieren des Strukturinhalts

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE trace           : Ursprung der Kopie
   R LAGRANGETRACE *recursivetrace : Ziel der Kopie

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
void LAGCopyTraceStructure(LAGRANGETRACE trace, LAGRANGETRACE * recursivetrace)
{
  (*recursivetrace).koor[0] = trace.koor[0];
  (*recursivetrace).koor[1] = trace.koor[1];
  (*recursivetrace).koor[2] = trace.koor[2];
  (*recursivetrace).position = trace.position;
  (*recursivetrace).knoten = trace.knoten;
  (*recursivetrace).kante = trace.kante;
  (*recursivetrace).flaeche = trace.flaeche;
  (*recursivetrace).volumen = trace.volumen;
  (*recursivetrace).restdt = trace.restdt;
  (*recursivetrace).ownweight = trace.ownweight;
  (*recursivetrace).slip = trace.slip;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGAddTraceStructureToStack

   Aufgabe:

   Ablegen des Strukturinhalts auf Stack

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E LAGRANGETRACE trace : Datenstruktur zum Ablegen

   Ergebnis:

   Programmaenderungen:
   4/2000    C. Thorenz  Erste Version
************************************************************************/
void LAGAddTraceStructureToStack(LAGRANGETRACE trace)
{
  long i;

  /* Freies Element im Stack suchen */
  for (i = 0; i < lagrange_stack_size; i++)
    {
      if (lagrange_stack[i].activ == 0)
        break;
    }

  /* Keinen freien Platz gefunden, neuen Allozieren */
  if (i == lagrange_stack_size)
    {
      lagrange_stack_size++;
      lagrange_stack = (LAGRANGESTACK *) Realloc(lagrange_stack, lagrange_stack_size * sizeof(LAGRANGESTACK));
    }

  LAGCopyTraceStructure(trace, &lagrange_stack[i].trace);
  lagrange_stack[i].activ = 1;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGGetTraceStructureFromStack

   Aufgabe:

   Abrufen einer Struktur vom Stack

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   Ergebnis:
      Zeiger auf Struktur

   Programmaenderungen:
   4/2000    C. Thorenz  Erste Version
************************************************************************/
int LAGGetTraceStructureFromStack(LAGRANGETRACE * trace)
{
  long i, index = -1;
  double max_weight = 0.;

  /* Element mit dem hoechsten Gewicht im Stack suchen */
  for (i = 0; i < lagrange_stack_size; i++)
    {
      if (lagrange_stack[i].activ)
        {
          if (lagrange_stack[i].trace.ownweight > max_weight)
            {
              max_weight = lagrange_stack[i].trace.ownweight;
              index = i;
            }
        }
    }

  /* Keine Daten mehr vorhanden */
  if (index == -1)
    return 0;

  LAGCopyTraceStructure(lagrange_stack[index].trace, trace);
  lagrange_stack[index].activ = 0;
  return 1;
}

/************************************************************************
   ROCKFLOW - Funktion:   LAGFindNextEdgeFromNode

   Aufgabe:

   Es wird versucht eine Kante zu finden, fuer die die gemittelten
   projizierten Geschwindigkeitsvektoren der anliegenden Elemente
   zu diesem Knoten fuehren.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long node: Startknoten

   Ergebnis:
   long kante

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
long LAGFindNextEdgeFromNode(long knoten)
{
  long *elemente, *kanten, *alle_kanten;
  int anz_elem, anz_alle_kanten, i, j;
  Kante *kante;
  Knoten *knoten0, *knoten1;
  static double vec2[3], velo[3];

  double betrag_v, help, koor[3];
  long massgeb_kante;

  anz_alle_kanten = 0;
  elemente = LAGGetNode2DElements(knoten, &anz_elem);
  alle_kanten = (long *) Malloc(sizeof(long) * anz_elem * 4);

  /* Suche alle Kanten an diesem Knoten */
  for (i = 0; i < anz_elem; i++)
    {
      kanten = ElGetElementEdges(elemente[i]);
      for (j = 0; j < 4; j++)
        {
          kante = GetEdge(kanten[j]);
          if ((kante -> knoten[0] == knoten) || (kante -> knoten[1] == knoten))
            {                          /* Nur Kanten mit "unserem" Knoten betrachten */
              alle_kanten[anz_alle_kanten] = kanten[j];
              anz_alle_kanten++;
            }                          /* endif */
        }                              /* Schleife ueber Kanten */
    }                                  /* Schleife ueber Elemente */
  /* Jetzt sollten alle Kanten vorhanden sein */

  Free(elemente);

  betrag_v = -1.;
  massgeb_kante = -1;

  for (i = 0; i < anz_alle_kanten; i++)
    {
      kante = GetEdge(alle_kanten[i]);

      knoten0 = GetNode(kante -> knoten[0]);
      knoten1 = GetNode(kante -> knoten[1]);

      if (knoten == kante -> knoten[0])
        {
          vec2[0] = knoten1 -> x - knoten0 -> x;
          vec2[1] = knoten1 -> y - knoten0 -> y;
          vec2[2] = knoten1 -> z - knoten0 -> z;
        }
      else
        {
          vec2[0] = knoten0 -> x - knoten1 -> x;
          vec2[1] = knoten0 -> y - knoten1 -> y;
          vec2[2] = knoten0 -> z - knoten1 -> z;
        }                              /* endif */
      /* Vektor zur Kantenbeschreibung, vom Knoten wegzeigend */

      help = MBtrgVec(vec2, 3);
      for (j = 0; j < 3; j++)          /* Normieren des Kantenvektors */
        vec2[j] /= help;

      knoten0 = GetNode(knoten);
      koor[0] = knoten0 -> x;
      koor[1] = knoten0 -> y;
      koor[2] = knoten0 -> z;

      LAGGet2DNegativeEdgeParallelVelocity(alle_kanten[i], koor, &(velo[0]));

      help = MSkalarprodukt(vec2, velo, 3);

      if ((help > betrag_v) && (help > 0.))
        {
          massgeb_kante = alle_kanten[i];
          betrag_v = help;
        }                              /* endif */
    }

  Free(alle_kanten);

  if (massgeb_kante >= 0)
    return massgeb_kante;
  else
    return -1;

}

/************************************************************************
   ROCKFLOW - Funktion:    LAGGet2DNegativeEdgeParallelVelocity

   Aufgabe:

   Es wird der gemittelte projizierte Geschwindigkeitsvektor
   der anliegenden Elemente fuer diese Kante ermittelt.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long edge: Kante
   E double *koor: Position

   R double *velo: Ermittelte Geschwindigkeit

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
void LAGGet2DNegativeEdgeParallelVelocity(long edge, double *koor, double *velo)
{
  static long i, j;

  static double veloglob[3], vec2[3];
  static Knoten *knoten0, *knoten1 = NULL;      /* Zeiger auf Knoten */
  static Kante *kante = NULL;   /* Zeiger auf Kante */
  static long *elemente;        /* Feld mit den Elementen einer Kante */
  static int anz_elem;
  static double help;
  static double porosity;

  /* Alles Elemente an dieser Kante */
  elemente = LAGGetEdge2DElements(edge, &anz_elem);

  velo[0] = 0.;
  velo[1] = 0.;
  velo[2] = 0.;

  for (i = 0; i < anz_elem; i++)
    {                                  /* Schleife ueber alle Elemente an dieser Kante, zum Feststellen der mittl. Geschw. */
      /*  Fuer den ermittelten Ort die Geschwindigkeit bestimmen */
      LAGGet2DElementFilterVelocity(elemente[i], lagrangecommon.theta, koor, &(veloglob[0]));

      /*  Auf negative Abstandsgeschw. umrechnen */
      porosity = GetSoilPorosity(elemente[i]);

      for (j = 0; j < 3; j++)
        veloglob[j] /= -porosity;

      /* Aufsummieren der Einzelvektoren und Mittelwertbildung */
      for (j = 0; j < 3; j++)
        velo[j] += veloglob[j] / (double) anz_elem;
    }

  kante = GetEdge(edge);
  knoten0 = GetNode(kante -> knoten[0]);
  knoten1 = GetNode(kante -> knoten[1]);

  /* Vektor zur Kantenbeschreibung */
  vec2[0] = knoten0 -> x - knoten1 -> x;
  vec2[1] = knoten0 -> y - knoten1 -> y;
  vec2[2] = knoten0 -> z - knoten1 -> z;

  help = MBtrgVec(vec2, 3);
  for (i = 0; i < 3; i++)              /* Normieren des Kantenvektors */
    vec2[i] /= help;

  help = MSkalarprodukt(vec2, velo, 3);         /* Betrag der Laenge in Richtung der Kante */
  if (fabs(help) < MKleinsteZahl)
    help = MKleinsteZahl;

  for (i = 0; i < 3; i++)              /* Komponente des Geschwindigkeitsvektors in Richtung der Kante */
    velo[i] = help * vec2[i];

  Free(elemente);

  return;
}

/************************************************************************
   ROCKFLOW - Funktion: LAGGetEdgeNext2DElements

   Aufgabe:
   Ausgehend von einer Kante, suche alle Elemente, die als
   naechste Elemente auf der Bahnlinie in Frage kommen.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long edge: Kante
   E double *koor: Position

   R int *anz_alle_elemente: Anzahl der gefundenen Elemente

   Ergebnis:
   long *  Zeiger auf Feld mit den gefundenen Elementen. Spaeter freigeben!!!

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
long *LAGGetEdgeNext2DElements(long edge, int *anz_alle_elem, double *koor)
{
  static long parent_edge;
  static long child_edge;
  static long *elemente;
  static long *alle_elemente;
  static int anz_elem;
  static long i, j;
  static double childvec[3], koorvec[3];
  static Kante *kante, *kinderkante;

  *anz_alle_elem = 0;

  /* Zuerst alle Elemente suchen, die an der urspruenglich uebergebenen Kante haengen */
  elemente = GetEdge2DElems(edge, &anz_elem);

  alle_elemente = (long *) Malloc(anz_elem);

  /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
  for (i = 0; i < anz_elem; i++)
    {
      if (ElGetElementActiveState(elemente[i]))
        {
          (*anz_alle_elem)++;
          alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
          alle_elemente[*anz_alle_elem - 1] = elemente[i];
        }
    }

  /* Hat die Kante einen Vorgaenger ? */
  kante = GetEdge(edge);
  parent_edge = kante -> vorgaenger;
  if (parent_edge > -1)
    {                                  /* Es gibt eine Vorgaengerkante */
      /* Alle Elemente suchen, die an dieser Kante haengen */
      elemente = GetEdge2DElems(parent_edge, &anz_elem);
      /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
      for (i = 0; i < anz_elem; i++)
        {
          if (ElGetElementActiveState(elemente[i]))
            {
              (*anz_alle_elem)++;
              alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
              alle_elemente[*anz_alle_elem - 1] = elemente[i];
            }
        }
    }
  /* Hat die Kante Nachfolger ? */
  kante = GetEdge(edge);
  child_edge = kante -> kinder[0];
  if (child_edge > -1)
    {                                  /* Es gibt eine Nachfolgerkante */
      /* Suche den neuen Mittelknoten */
      kinderkante = GetEdge(child_edge);

      if ((kinderkante -> knoten[0] == kante -> knoten[0])
          || (kinderkante -> knoten[0] == kante -> knoten[1]))
        {
          /* Vektor vom Mittelknoten zum Randknoten */
          childvec[0] = GetNodeX(kinderkante -> knoten[0]) - GetNodeX(kinderkante -> knoten[1]);
          childvec[1] = GetNodeY(kinderkante -> knoten[0]) - GetNodeY(kinderkante -> knoten[1]);
          childvec[2] = GetNodeZ(kinderkante -> knoten[0]) - GetNodeZ(kinderkante -> knoten[1]);
          koorvec[0] = koor[0] - GetNodeX(kinderkante -> knoten[1]);
          koorvec[1] = koor[1] - GetNodeY(kinderkante -> knoten[1]);
          koorvec[2] = koor[2] - GetNodeZ(kinderkante -> knoten[1]);
        }
      else
        {
          childvec[0] = GetNodeX(kinderkante -> knoten[1]) - GetNodeX(kinderkante -> knoten[0]);
          childvec[1] = GetNodeY(kinderkante -> knoten[1]) - GetNodeY(kinderkante -> knoten[0]);
          childvec[2] = GetNodeZ(kinderkante -> knoten[1]) - GetNodeZ(kinderkante -> knoten[0]);
          koorvec[0] = koor[0] - GetNodeX(kinderkante -> knoten[0]);
          koorvec[1] = koor[1] - GetNodeY(kinderkante -> knoten[0]);
          koorvec[2] = koor[2] - GetNodeZ(kinderkante -> knoten[0]);
        }

      if (MSkalarprodukt(koorvec, childvec, 3) <= 0.)
        {                              /* Das war noch nicht die richtige Kante! */
          child_edge = kante -> kinder[1];
        }
      /* endif */
      /* Alle Elemente suchen, die an dieser Kante haengen */
      elemente = GetEdge2DElems(child_edge, &anz_elem);
      /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
      for (j = 0; j < anz_elem; j++)
        {
          if (ElGetElementActiveState(elemente[j]))
            {
              (*anz_alle_elem)++;
              alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
              alle_elemente[*anz_alle_elem - 1] = elemente[j];
            }                          /* endif */
        }                              /* endfor */
    }                                  /* endif */
  return alle_elemente;

}

/************************************************************************
   ROCKFLOW - Funktion: LAGGetEdge2DElements

   Aufgabe:
   Liefert alle aktiven 2D-Elemente, die an einer Kante haengen.
   Dies gilt auch wenn sich die die Elemente auf unterschiedlichen
   Verfeinerungslevels befinden (irregulaere Kanten).

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long edge          : betrachtete Kante
   E int *anz_alle_elem : Anzahl der gefundenen Elemente
   E long *alle_elemente: Feld der gefundenen Elemente

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
************************************************************************/
long *LAGGetEdge2DElements(long edge, int *anz_alle_elem)
{
  static long parent_edge;
  static long child_edge;
  static long *elemente;
  static long *alle_elemente;
  static int anz_elem;
  static long i, j;
  static Kante *kante;

  *anz_alle_elem = 0;

  /* Zuerst alle Elemente suchen, die an der urspruenglich uebergebenen Kante haengen */
  elemente = GetEdge2DElems(edge, &anz_elem);

  alle_elemente = (long *) Malloc(4);

  /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
  for (i = 0; i < anz_elem; i++)
    {
      if (ElGetElementActiveState(elemente[i]))
        {
          (*anz_alle_elem)++;
          alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
          alle_elemente[*anz_alle_elem - 1] = elemente[i];
        }
    }

  /* Hat die Kante einen Vorgaenger ? */
  kante = GetEdge(edge);
  parent_edge = kante -> vorgaenger;
  if (parent_edge > -1)
    {                                  /* Es gibt eine Vorgaengerkante */
      /* Alle Elemente suchen, die an dieser Kante haengen */
      elemente = GetEdge2DElems(parent_edge, &anz_elem);
      /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
      for (i = 0; i < anz_elem; i++)
        {
          if (ElGetElementActiveState(elemente[i]))
            {
              (*anz_alle_elem)++;
              alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
              alle_elemente[*anz_alle_elem - 1] = elemente[i];
            }
        }
    }
  /* Hat die Kante Nachfolger ? */
  kante = GetEdge(edge);
  for (i = 0; i < 2; i++)
    {                                  /* Hoechstens zwei Nachfolger */
      child_edge = kante -> kinder[i];
      if (child_edge > -1)
        {                              /* Es gibt eine Vorgaengerkante */
          /* Alle Elemente suchen, die an dieser Kante haengen */
          elemente = GetEdge2DElems(child_edge, &anz_elem);
          /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */
          for (j = 0; j < anz_elem; j++)
            {
              if (ElGetElementActiveState(elemente[j]))
                {
                  (*anz_alle_elem)++;
                  alle_elemente = (long *) Realloc(alle_elemente, *anz_alle_elem * sizeof(long));
                  alle_elemente[*anz_alle_elem - 1] = elemente[j];
                }                      /* endif */
            }                          /* endfor */
        }                              /* endif */
    }                                  /* endfor */

  return alle_elemente;
}

/************************************************************************
   ROCKFLOW - Funktion: GetNode2DActiveElements

   Aufgabe:
   Liefert alle aktiven 2D-Elemente, die an einem (eventuell irregulaeren!)
   Knoten haengen. Dies gilt auch wenn sich die die Elemente auf unterschiedlichen
   Verfeinerungslevels befinden (irregulaere Knoten).

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long node          : betrachtetr Knoten
   R int *anz_alle_elem : Anzahl der gefundenen Elemente
   R long *alle_elemente: Feld der gefundenen Elemente

   Ergebnis:

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
************************************************************************/

long *LAGGetNode2DElements(long node, int *anz_alle_elemente)
{
  /* Liefert alle 2D-Elemente, die an einem Knoten liegen. Bei irregulaeren Knoten
     auch das grobe Nachbarelemente */

  static long *elemente;
  static long *alle_kanten;
  static long *alle_elemente;
  static int anz_elem, anz_kanten, anz_alle_kanten;
  static long i, j, k;

  (*anz_alle_elemente) = 0;
  anz_kanten = 0;

  alle_kanten = LAGGetNode2DEdges(node, &anz_alle_kanten);
  alle_elemente = (long *) Malloc(100 * sizeof(long));
  /* erstmal Platz fuer 100 Elemente, sollte reichen .... */

  /* Diese Elemente ggf. in die Gesamt-Liste aufnehmen */

  for (i = 0; i < anz_alle_kanten; i++)
    {                                  /* Schleife ueber alle Kanten */
      elemente = LAGGetEdge2DElements(alle_kanten[i], &anz_elem);
      for (j = 0; j < anz_elem; j++)
        {
          if (ElGetElementActiveState(elemente[j]))
            {
              for (k = 0; ((k < (*anz_alle_elemente))
                           && (alle_elemente[k] != elemente[j])); k++)
                {
                };                     /* Solange erhoehen, bis entweder das Elemente gefunden wurde oder die Liste durch ist */
              if (k == (*anz_alle_elemente))
                {                      /* ggf. zur Liste hinzufuegen */
                  alle_elemente[(*anz_alle_elemente)] = elemente[j];
                  (*anz_alle_elemente)++;
                  if ((*anz_alle_elemente) > 99)
                    alle_elemente = (long *) Realloc(alle_elemente, (*anz_alle_elemente + 1) * sizeof(long));
                }                      /* endif */
            }                          /* endif */
        }                              /* endfor */
      Free(elemente);
    }                                  /* endfor */
  Free(alle_kanten);

  return alle_elemente;
}

/************************************************************************
   ROCKFLOW - Funktion:  LAGGetNode2DEdges

   Aufgabe:
   Ausgehend von eine Knoten werden alle Kanten gesucht.

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long node: Knoten

   R int *anz_kanten: Anzahl der gefundenen Kanten

   Ergebnis:
   long *  Zeiger auf Feld mit den gefundenen Kanten. Spaeter freigeben!!!

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/

long *LAGGetNode2DEdges(long node, int *anz_kanten)
{
  /* Liefert alle Kanten an einem Knoten, aber nicht die irregulaeren Elternkanten */
  int anz_elem;
  int i, j, k;
  long *alle_kanten = NULL;
  long *kanten;
  long *elemente;
  Kante *kante;

  /* Alle 2-D Elemente holen, die an diesem Knoten haengen */
  elemente = GetNode2DElems(node, &anz_elem);

  *anz_kanten = 0;

  if (anz_elem)
    {
      /* Alle Kanten holen, die an diesen Elementen haengen */
      alle_kanten = (long *) Malloc(4 * anz_elem * sizeof(long));
      alle_kanten[0] = -1;

      for (i = 0; i < anz_elem; i++)
        {
          if (ElGetElementActiveState(elemente[i]))
            {
              kanten = ElGetElementEdges(elemente[i]);
              for (j = 0; j < 4; j++)
                {
                  for (k = 0; ((k < *anz_kanten) && (alle_kanten[k] != kanten[j])); k++)
                    {
                    };
                  if (k == *anz_kanten)
                    {
                      kante = GetEdge(kanten[j]);
                      if (((kante -> knoten[0] == node)
                           || (kante -> knoten[1] == node))
                          && (kante -> kinder[0] == -1))
                        {
                          /* Alle Kanten aussortieren, die nicht an diesem Knoten sind */
                          /* Alle Kanten aussortieren, die Kinder haben */
                          alle_kanten[*anz_kanten] = kanten[j];
                          (*anz_kanten)++;
                        }              /* endif */
                    }                  /* endif */
                }                      /* endfor */
            }                          /* endif */
        }                              /* endfor */
    }

  return alle_kanten;

}

/************************************************************************
   ROCKFLOW - Funktion:   LAGCalcMiddleEdgeParallelFlow

   Aufgabe:
   Ermittelt den mittleren Kantenparallelen Fluss

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   E long node: Knoten
   E long edge: Kante
   E double *koor: Koordinaten

   Ergebnis:
   double   Fluss

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
double LAGCalcMiddleEdgeParallelFlow(long knoten, long edge, double *koor)
{
  /* Liefert den Volumenstrom parallel zu einer Kante, aus der gemittelten Sicht der dranhaengenden Elemente */
  long *elemente;
  int anz_elem;
  int i;
  double q_sum;

  q_sum = 0.;

  elemente = LAGGetEdge2DElements(edge, &anz_elem);
  for (i = 0; i < anz_elem; i++)
    {
      q_sum += LAGCalcEdgeParallelFlow(knoten, edge, elemente[i], koor);
    }                                  /* endfor */
  q_sum /= (double) (anz_elem);

  Free(elemente);

  return q_sum;
}

/************************************************************************
   ROCKFLOW - Funktion:   LAGCalcEdgeParallelFlow

   Aufgabe:
   Ermittelt den kantenparallelen Fluss

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

   E long node: Knoten
   E long edge: Kante
   E long element: Element
   E double *koor: Koordinaten

   Ergebnis:
   double   Fluss

   Programmaenderungen:
   4/1998    C. Thorenz  Erste Version
   6/1998    C. Thorenz  Zweite Version
************************************************************************/
double LAGCalcEdgeParallelFlow(long knoten, long edge, long element, double *koor)
{
  /* Liefert den Volumenstrom parallel zu einer Kante, aus der Sicht eines Elements */
  double velo[3], vec[3], help;
  Kante *kante;
  Knoten *knoten0, *knoten1;
  int i;
  static double area;

  area = GetElementExchangeArea(element);

  kante = GetEdge(edge);

  knoten0 = GetNode(kante -> knoten[0]);
  knoten1 = GetNode(kante -> knoten[1]);

  if (knoten == kante -> knoten[1])
    {
      vec[0] = knoten1 -> x - knoten0 -> x;
      vec[1] = knoten1 -> y - knoten0 -> y;
      vec[2] = knoten1 -> z - knoten0 -> z;
    }
  else
    {
      vec[0] = knoten0 -> x - knoten1 -> x;
      vec[1] = knoten0 -> y - knoten1 -> y;
      vec[2] = knoten0 -> z - knoten1 -> z;
    }                                  /* endif */
  /* Vektor zur Kantenbeschreibung, zum Knoten zeigend */

  help = MBtrgVec(vec, 3);
  for (i = 0; i < 3; i++)              /* Normieren des Kantenvektors */
    vec[i] /= help;

  LAGGet2DElementFilterVelocity(element, lagrangecommon.theta, koor, velo);

  return MSkalarprodukt(vec, velo, 3) * area;   /* mat->area ist die Dicke! */
}

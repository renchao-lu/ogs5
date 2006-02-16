/**************************************************************************/
/* ROCKFLOW - Modul: gridadap.c
                                                                          */
/* Aufgabe:
   Funktionen fuer die Gitteradaption
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   01.07.1997  R.Kaiser   Korrekturen und Aenderungen aus dem aTM
                          uebertragen
   16.07.1997  R.Kaiser   Erweitert um die verallgemeinerten
                          Funktionen Interpol_q_XD
                          aus cgs_asm.c, cgs_atm.c  usw.
   25.07.1997  R.Kaiser   Erweitert um die Funktionen
                          DelIrrNodesXD und CalcIrrNodeVals
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   12/1999     C.Thorenz  Irr. Knoten korrigiert
   07/2000     RK         adaptiv.* <-> gridadap.*
   08/2000     RK         Funktion 'CriticalElements'
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"

/* Header / Andere intern benutzte Module */
#include "adaptiv.h"
#include "mathlib.h"
#include "tools.h"
#include "matrix.h"

/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructRelationships
                                                                          */
/* Aufgabe:
   Kanten- bzw. Flaechenverzeichnis aufbauen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ConstructRelationships(void)
{
  if (max_dim == 2) {
      /* Flaechenverzeichnis aufstellen */
    if ((GetRFControlGridAdapt() == 1)||(GetRFControlGridAdapt() == 2))
      ConstructPlainList();
    if (GetRFControlGridAdapt() == 3)
      ConstructPlainListFast();
  }
  else {
      /* Kantenverzeichnis aufstellen */
    if ((GetRFControlGridAdapt() == 1)||(GetRFControlGridAdapt() == 2))
      ConstructEdgeList();
    if (GetRFControlGridAdapt() == 3)
      ConstructEdgeListFast();
  }
}


/**************************************************************************
   ROCKFLOW - Funktion: ExecuteGridAdaptation

   Aufgabe:
   Schleife Gitteradaption


   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -

   Programmaenderungen:
   07/1999      RK     Erste Version
   08/2000      RK     Ueberarbeitet


**************************************************************************/
int ExecuteGridAdaptation ( int position, int *grid_change)
{

  if (!GetRFControlGridAdapt()) /* keine Gitteradaption */
    return 0;

  if (AdaptGetMethodAdaptation() == 0) /* keine Gitteradaption */
    return 0;



  if ((AdaptGetMethodAdaptation() == 1) && position == 1) {
    *grid_change = 0;
    if ((AdaptGetTimestepRefNumber() > 0) &&
        (AdaptGetRefIter() < AdaptGetTimestepRefNumber())) {

      AdaptSetGridChange(0);

      if (labs(AdaptGetMaxNodes()))
        NodeNumberLimiter(AdaptGetMaxNodes());

      DisplayMsgLn("Indikator starten");

      if (Indicator(AdaptGetRefIter())) {

        DisplayMsgLn("Verfeinern bzw. vergroebern");

        if (Refine()) {

          AdaptSetGridChange(1);
          *grid_change = 1;

          BuildActiveElementsArray();
          
          /* AdaptAssignEdgeNodesToElements(); */

          DisplayMsgLn("Umnummerierer starten");

          RenumberNodes();
        }
      }
      else
        return 0;
    }
    else
      return 0;

    AdaptSetRefIter(AdaptGetRefIter()+1);
    if (AdaptGetRefIter() <= AdaptGetTimestepRefNumber()) return 1;
    else return 0;

  }

  if ((AdaptGetMethodAdaptation() == 2) && position == 0) {
    *grid_change = 0;
    if ((AdaptGetTimestepRefNumber() > 0) &&
        (AdaptGetRefIter() < AdaptGetTimestepRefNumber())) {

      AdaptSetGridChange(0);

      if (labs(AdaptGetMaxNodes()))
        NodeNumberLimiter(AdaptGetMaxNodes());

      DisplayMsgLn("Indikator starten");

      if (Indicator(AdaptGetRefIter())) {

        DisplayMsgLn("Verfeinern bzw. vergroebern");

        if (Refine()) {

          AdaptSetGridChange(1);
          *grid_change = 1;

          BuildActiveElementsArray();
          
          /* AdaptAssignEdgeNodesToElements(); */

          DisplayMsgLn("Umnummerierer starten");

          RenumberNodes();
        }
      }
      else
        return 0;
    }
    else
      return 0;

    AdaptSetRefIter(AdaptGetRefIter()+1);
    if (AdaptGetRefIter() < AdaptGetTimestepRefNumber()) return 1;
    else return 0;

  }

  if ((AdaptGetMethodAdaptation() == 3) && position == 1) {
    *grid_change = 0;
    if ((AdaptGetTimestepRefNumber() > 0) &&
        (AdaptGetRefIter() < AdaptGetTimestepRefNumber()) &&
        (!(aktueller_zeitschritt % AdaptGetAdaptationTimestep()) ||
         aktueller_zeitschritt == 1)) {

      AdaptSetGridChange(0);

      if (labs(AdaptGetMaxNodes()))
        NodeNumberLimiter(AdaptGetMaxNodes());

      DisplayMsgLn("Indikator starten");

      if (Indicator(AdaptGetRefIter())) {

        DisplayMsgLn("Verfeinern bzw. vergroebern");

        if (Refine()) {

          AdaptSetGridChange(1);
          *grid_change = 1;

          BuildActiveElementsArray();
          
          /* AdaptAssignEdgeNodesToElements(); */

          DisplayMsgLn("Umnummerierer starten");

          RenumberNodes();
        }
      }
      else
        return 0;
    }
    else
      return 0;

    AdaptSetRefIter(AdaptGetRefIter()+1);
    if (AdaptGetRefIter() <= AdaptGetTimestepRefNumber()) return 1;
    else return 0;

  }

  if ((AdaptGetMethodAdaptation() == 4) && position == 0) {
   *grid_change = 0;
   if ((AdaptGetTimestepRefNumber() > 0) &&
        (AdaptGetRefIter() < AdaptGetTimestepRefNumber()) &&
        (!(aktueller_zeitschritt % AdaptGetAdaptationTimestep()) ||
         aktueller_zeitschritt == 1)) {


      AdaptSetGridChange(0);

      if (labs(AdaptGetMaxNodes()))
        NodeNumberLimiter(AdaptGetMaxNodes());

      DisplayMsgLn("Indikator starten");

      if (Indicator(AdaptGetRefIter())) {

        DisplayMsgLn("Verfeinern bzw. vergroebern");

        if (Refine()) {

          AdaptSetGridChange(1);
          *grid_change = 1;

          BuildActiveElementsArray();
          
          /* AdaptAssignEdgeNodesToElements(); */

          DisplayMsgLn("Umnummerierer starten");

          RenumberNodes();
        }
      }
      else
        return 0;
    }
    else
      return 0;

    AdaptSetRefIter(AdaptGetRefIter()+1);
    if (AdaptGetRefIter() < AdaptGetTimestepRefNumber()) return 1;
    else return 0;

  }

  return 0;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: DelIrrNodes2D
                                                                          */
/* Aufgabe:
   Irregulaere Knoten aus Gleichungssystem entfernen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   X long *global_node_number: Indizes der Knoten im Gesamtgleichungssystem
   X double *left_matrix: Elementmatrix (linke Seite)
   X double *right_vector: Elementvektor (rechte Seite)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1997     (cb)/R.Kaiser      erste Version
                                                                          */
/**************************************************************************/
void DelIrrNodes2D(long index, long *global_node_number,
                   double *left_matrix, double *right_vector)
{
  static long *ele_nodes;
  static long nn, *elem, regkno;
  static int i, j, anz;
  static double right_vector2[4];
  static double permutation_matrix[16];
  static double trans_permutation_matrix[16];
  static double perm_left_matrix[16];

  /* Matrizen, Vektoren initialisieren */
  MNulleMat(permutation_matrix, 4, 4);
  MNulleMat(trans_permutation_matrix, 4, 4);
  MNulleVec(right_vector2, 4);
  MNulleMat(perm_left_matrix, 4, 4);

  ele_nodes = ElGetElementNodes(index);
  nn = ElNumberOfNodes[1];


  /* irregulaere Knoten eliminieren */
  /* irr. Kno. kann nur bei i=1 oder i=3 sein */
  if ((GetNodeState(ele_nodes[1]) == -2) || \
      (GetNodeState(ele_nodes[3]) == -2)) {
    /* Permutationsmatrix */
    /* DisplayMsgLn("PERMUTATIONSMATRIX"); */
    MNulleMat(permutation_matrix, nn, nn);
    for (j = 0; j < nn; j++)
      permutation_matrix[j * nn + j] = 1.0;
    /* i=2 ist Mittelknoten, i=0 ist reg. Eckknoten */
    for (i = 1; i < nn; i += 2) {
      if (GetNodeState(ele_nodes[i]) == -2) {
                                           /* irregulaerer Knoten */
        permutation_matrix[i * nn + i] = permutation_matrix[i] = 0.5;

        /* Knotennummer des Nachbarelements finden */
        elem = GetNode2DElems(ele_nodes[i], &anz);
        if (ElGetElementNodes(elem[0])[0] == ele_nodes[0])
          regkno = ElGetElementNodes(elem[1])[0];
        else
          regkno = ElGetElementNodes(elem[0])[0];

        /* regulaeren Index in global_node_number einsetzen */
        global_node_number[i] = GetNodeIndex(regkno);
                                       /* Indizes der reg.Nachbarn */
      }
    }
    /* linke Seite */
    /* permutation_matrix x left_matrix */
    MMultMatMat(permutation_matrix, nn, nn, left_matrix, nn, nn,
                perm_left_matrix, nn, nn);
    /* perm_left_matrix x trans_permutation_matrix */
    MTranspoMat(permutation_matrix, nn, nn, trans_permutation_matrix);
    MMultMatMat(perm_left_matrix, nn, nn, trans_permutation_matrix, nn, nn,
                left_matrix, nn, nn);

    /* rechte Seite */
    for (j = 0; j < nn; j++)
      right_vector2[j] = right_vector[j];


    /* right_vector2  x trans_permutation_matrix */
    MMultVecMat(right_vector2, nn, trans_permutation_matrix, nn, nn,
                right_vector, nn);
  }                           /* Ende Irr. Knoten eliminieren */
  ele_nodes = NULL;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: DelIrrNodes3D
                                                                          */
/* Aufgabe:
   Irregulaere Knoten aus Gleichungssystem entfernen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   X long *global_node_number: Indizes der Knoten im Gesamtgleichungssystem
   X double *left_matrix: Elementmatrix (linke Seite)
   X double *right_vector: Elementvektor (rechte Seite)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   25.07.1997       (cb)/R.Kaiser       erste Version

   letzte Aenderung:      R.Kaiser     25.07.1997
                                                                          */
/**************************************************************************/
void DelIrrNodes3D(long index, long *global_node_number,
                   double *left_matrix, double *right_vector)
{
  static long k, nn, *ele_nodes, *elem, *flaeche;
  static int i, l, m, anz;
  static double right_vector2[8];
  static double permutation_matrix[64];
  static double trans_permutation_matrix[64];
  static double perm_left_matrix[64];
  static Flaeche *fl;

  nn = ElNumberOfNodes[2];
  /* Matrizen, Vektoren initialisieren */
  MNulleVec(right_vector2, nn);
  MNulleMat(permutation_matrix,nn, nn);
  MNulleMat(trans_permutation_matrix,nn,nn);
  MNulleMat(perm_left_matrix, nn, nn);

  ele_nodes = ElGetElementNodes(index);


  /* irregulaere Knoten eliminieren */
  if ((GetNodeState(ele_nodes[2]) == -4) ||
      (GetNodeState(ele_nodes[5]) == -4) ||
      (GetNodeState(ele_nodes[7]) == -4) ||
      (GetNodeState(ele_nodes[1]) == -2) ||
      (GetNodeState(ele_nodes[3]) == -2) ||
      (GetNodeState(ele_nodes[4]) == -2)) {
      /* i=6 ist Mittelknoten, i=0 ist reg. Eckknoten */

    /* Permutationsmatrix */
    MNulleMat(permutation_matrix, nn, nn);
    for (i = 0; i < nn; i++)
      permutation_matrix[i * nn + i] = 1.0;       /* Einheitsmatrix */

    for (i = 1; i < nn; i++) {
      if (GetNodeState(ele_nodes[i]) == -2) {
                                             /* irreg. Kantenknoten */
        /* irr. Kantenknoten (-2) kann nur bei i=1 oder i=3 oder i=4 liegen */
        /* Permutationsmatrix */
        permutation_matrix[i * nn + i] = permutation_matrix[i] = 0.5;
        elem = GetNode3DElems(ele_nodes[i], &anz);
        for (k = 0; k < anz; k++)
          if (NodeNumber[GetNodeIndex(ElGetElementNodes(elem[k])[0])] != ele_nodes[0]) {
            /* regulaeren Index in global_node_number einsetzen */
            global_node_number[i] = GetNodeIndex(ElGetElementNodes(elem[k])[0]);
            break;
          }
      }
      else if (GetNodeState(ele_nodes[i]) == -4) {
                                                /* irreg. Flaechenknoten */
      /* irr. Flaechenknoten (-4) kann nur bei i=5 oder i=7 oder i=2 liegen */
        /* Permutationsmatrix */
          permutation_matrix[i * nn + i] = permutation_matrix[i] = 0.25;
        if (i == 5)
          permutation_matrix[1 * nn + i] = permutation_matrix[4 * nn + i] = 0.25;
        else if (i == 7)
          permutation_matrix[3 * nn + i] = permutation_matrix[4 * nn + i] = 0.25;
        else if (i == 2)
          permutation_matrix[1 * nn + i] = permutation_matrix[3 * nn + i] = 0.25;
        elem = GetNode3DElems(ele_nodes[i], &anz);

        /* richtige Reihenfolge */
        /* Flaechen des Vorgaenger-Elements */
        flaeche = ElGetElementPlains(ElGetElementPred(index));
        for (k = 0; k < 6; k++) {       /* Anzahl Flaechen */
          fl = GetPlain(flaeche[k]);
          anz = 0;
          for (l = 0; l < 4; l++) {   /* Anzahl regulaerer Nachbarknoten */
            for (m = 0; m < 4; m++) {       /* Anzahl Knoten einer Flaeche */
              if (fl->knoten[m] == NodeNumber[GetNodeIndex(ElGetElementNodes(elem[l])[0])]) {
                anz++;
                break;
              }
            }
          }
          if (anz == 4) {     /* Flaeche gefunden */
            break;
          }
        }

        /* regulaeren Index in global_node_number einsetzen */
        for (k = 0; k < 4; k++) {
          if (NodeNumber[GetNodeIndex(fl->knoten[k])] == ele_nodes[0]) {
            /* k ist Eckknoten */
            global_node_number[i] = GetNodeIndex(fl->knoten[((k + 2) % 4)]);
          }
        }
      }
    }

    /* permutation_matrix x left_matrix */
    MTranspoMat(permutation_matrix, nn, nn, trans_permutation_matrix);
    MMultMatMat(permutation_matrix, nn, nn, left_matrix, nn, nn,
                perm_left_matrix, nn, nn);
    /* perm_left_matrix x trans_permutation_matrix */

    MMultMatMat(perm_left_matrix, nn, nn, trans_permutation_matrix, nn, nn,
                left_matrix, nn, nn);

    /* rechte Seite */
    for (i = 0; i < nn; i++)
      right_vector2[i] = right_vector[i];

    /* right_vector2 x trans_permutation_matrix */
    MMultVecMat(right_vector2, nn, trans_permutation_matrix, nn, nn,
                right_vector, nn);


  }
  ele_nodes = NULL;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcIrrNodeVals
                                                                          */
/* Aufgabe:
   Werte der Gleichungsystemeintraege der irregulaeren Knoten im
   Nachlauf berechnen (2D und 3D)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ergebnis
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1995       cb         Erste Version
   25.07.1997    R.Kaiser   Umbenannt zu CalcIrrNodeVals
                            (CalcIrrNodes_ASM) und erweitert

   letzte Aenderung:     R.Kaiser     25.07.1997

                                                                          */
/**************************************************************************/
void CalcIrrNodeVals(double *ergebnis)
{
  static long i, index, nn;
  static long *element_nodes;
  static long *elem;
  static int anz, k;


  for (index = 0; index < ElListSize(); index++) {
    if (ElGetElement(index) != NULL) {     /* wenn Element existiert */
      if (ElGetElementActiveState(index)) {    /* nur aktive Elemente */
        if (ElGetElementType(index) == 2) {
          nn = ElNumberOfNodes[1];
          element_nodes = ElGetElementNodes(index);
          /* irr. Kno. kann nur bei i=1 oder i=3 sein */
          if ((GetNodeState(element_nodes[1]) == -2) ||
              (GetNodeState(element_nodes[3]) == -2)) {
            /* i=2 ist Mittelknoten, i=0 ist reg. Eckknoten */
            for (i = 1; i < nn; i += 2) {
              if (GetNodeState(element_nodes[i]) == -2) {
                                              /* irregulaerer Knoten */

                elem = GetNode2DElems(element_nodes[i], &anz);
                ergebnis[GetNodeIndex(element_nodes[i])] = 0.5 * ergebnis[GetNodeIndex(element_nodes[0])];
                for (k = 0; k < anz; k++) {
                  if (ElGetElementNodes(elem[k])[0] != element_nodes[0]) {
                    ergebnis[GetNodeIndex(element_nodes[i])]
                               += 0.5 * ergebnis[GetNodeIndex(ElGetElementNodes(elem[k])[0])];
                    break;

                  }
                }
              }
            }
          }
          element_nodes = NULL;
        }
        if (ElGetElementType(index) == 3) {
          nn = ElNumberOfNodes[2];
          element_nodes = ElGetElementNodes(index);

          if ((GetNodeState(element_nodes[2]) == -4) ||
              (GetNodeState(element_nodes[5]) == -4) ||
              (GetNodeState(element_nodes[7]) == -4) ||
              (GetNodeState(element_nodes[1]) == -2) ||
              (GetNodeState(element_nodes[3]) == -2) ||
              (GetNodeState(element_nodes[4]) == -2)) {
            /* i=6 ist Mittelknoten, i=0 ist reg. Eckknoten */
            for (i = 1; i < nn; i++) {
              if (GetNodeState(element_nodes[i]) == -2) {
                                               /* irreg. Kantenknoten */
                /* irr. Kantenknoten (-2) kann nur bei i=1 oder i=3 oder i=4 liegen */

                elem = GetNode3DElems(element_nodes[i], &anz);
                ergebnis[GetNodeIndex(element_nodes[i])] = 0.5 * ergebnis[GetNodeIndex(element_nodes[0])];

                for (k = 0; k < anz; k++) {
                  if ((ElGetElementNodes(elem[k])[0]) != element_nodes[0]) {
                    ergebnis[GetNodeIndex(element_nodes[i])]
                               += 0.5 * ergebnis[GetNodeIndex(ElGetElementNodes(elem[k])[0])];
                    break;

                  }
                }
              }

              else if (GetNodeState(element_nodes[i]) == -4) {
                                                      /* irreg. Flaechenknoten */
                /* irr. Flaechenknoten (-4) kann nur bei i=5 oder i=7 oder i=2 liegen */

                elem = GetNode3DElems(element_nodes[i], &anz);
                ergebnis[GetNodeIndex(element_nodes[i])] = 0.25 * ergebnis[GetNodeIndex(element_nodes[0])];
                for (k = 0; k < anz; k++) {
                  if ((ElGetElementNodes(elem[k])[0]) != element_nodes[0]) {
                    ergebnis[GetNodeIndex(element_nodes[i])]
                               += 0.25 * ergebnis[GetNodeIndex(ElGetElementNodes(elem[k])[0])];

                  }
                }
              }
            }
          }
          element_nodes = NULL;
        }
      }                   /* if = nur aktive Elemente */
    }                       /* if = Element existiert */
  }                           /* for = Schleife ueber die Elemente */
}                               /* CalcIrrNodeVals */

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcIrrNodeValsOnNode
                                                                          */
/* Aufgabe:
   Werte der irregulaeren Knoten berechnen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: int ndx: Knotenindex der betrachteten Groesse
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/2001       CT         Erste Version
                                                                          */
/**************************************************************************/
void CalcIrrNodeValsOnNode(int ndx)
{
  double *help;

  help = (double *) Malloc(NodeListLength*sizeof(double));

  PresetErgebnis(help, ndx);
  CalcIrrNodeVals(help);
  TransferNodeVals(help, ndx);

  help = (double *) Free(help);
}







/**************************************************************************
   ROCKFLOW - Funktion: NodeNumberLimiter

   Aufgabe:
   Durch die Veraenderung der Indikatoren wird versucht die Knotenzahl
   zu beeinflussen.


   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long maximum_nodes  : Zugelassene Knotenzahl
                          >0: maximum_nodes wird angestrebt
                          <0: |maximum_nodes| wird nicht ueberschritten

   Programmaenderungen:
   11/1997      C.Thorenz       Erste Version
    7/2000      C.Thorenz       Neue Methode
    9/2000      C.Thorenz       Zeitkurve, neue Methode
   11/2000      C.Thorenz       Waerme mit aufgenommen

*************************************************************************/
void NodeNumberLimiter ( long maximum_nodes)
{
    static long ii, i, j, old_node_number = 1, jj;
    static double indicator_limiter = 1.;
    static double old_indicator_limiter = 1;
    static double indicator_modifier = 1., help, oscillating = 1.;
    double ref_param, new_limiter, val;
    int methode, gueltig;

    val = GetCurveValue(AdaptGetCurveNodeLimiter(), 0, aktuelle_zeit, &gueltig);

    methode = AdaptGetMethodNodeLimiter();

    switch (methode) {
    case 1:
        indicator_modifier /= min(max(2.*(fabs((double)old_node_number - (double)NodeListLength) + 1.)
           /(fabs((double)old_node_number - fabs((double)maximum_nodes*val)) + fabs((double)NodeListLength - fabs((double)maximum_nodes*val)) + 1.), 0.7), 1.4);

        if (indicator_modifier > 2.)
          indicator_modifier = 2.;
        if (indicator_modifier < .01)
          indicator_modifier = .01;

        /* help ist der Faktor fuer Veraenderungen, indicator_modifier ist die Daempfung */
        help = pow((double) NodeListLength / (double) fabs((double)maximum_nodes*val), indicator_modifier);
        /* indicator_limiter ist das "Gedaechtnis" fuer die Modifikationen */


        /* Merken der alten "Verhaeltnisse" */
        old_indicator_limiter = indicator_limiter;
        old_node_number = NodeListLength;

        if (aktueller_zeitschritt < 2)
           return;

        if (maximum_nodes*val < 0)
          if (indicator_limiter * help < 1.)
            /* Die Indikatoren sollen nur _vergroessert_ werden ... */
            help = 1. / indicator_limiter;

        /* Alle Modifikationen zuruecknehmen */
        if(!gueltig)
            help = 1. / indicator_limiter;

        /* In indicator_limiter steht die Produktsumme aller Modifikationen */
        indicator_limiter *= help;

        /* Begrenzung der Knotenzahl durch Verbiegen der Indikatoren */
        for (jj = 0; jj < AdaptGetNumAdaptQuantities(); jj++)
          for (ii = 0; ii < AdaptGetNumberInd(jj); ii++)
            for (i = 0; i < 3; i++)
              for (j = 0; j < 3; j++) {
                if (AdaptGetRefQuantity(jj) == 0) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 1) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                  ref_param = AdaptGetRefParam(jj, ii, i, 1, j) * help;
                  AdaptSetRefParam(jj, ii, i, 1, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 2) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                  ref_param = AdaptGetRefParam(jj, ii, i, 1, j) * help;
                  AdaptSetRefParam(jj, ii, i, 1, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 3) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                }
             }
        break;

    case 2:
        /* Wenn Knotenzahl oszilliert, suche die lineare Interpolation */
        if((NodeListLength!=old_node_number)&&(Signum((double)(NodeListLength-fabs((double)maximum_nodes*val))) != Signum(((double)old_node_number-fabs((double)maximum_nodes*val))))) {
          new_limiter = old_indicator_limiter + (fabs((double)maximum_nodes*val) - (double)old_node_number) * ((double)indicator_limiter - (double)old_indicator_limiter) / ((double)NodeListLength - (double)old_node_number) ;
          help = pow(new_limiter / indicator_limiter, 1./oscillating);
          oscillating += 1.;
        } else {
          /* Sonst einfach "geradeaus" */
          help = pow((double)NodeListLength / fabs((double)maximum_nodes*val), 1./oscillating);
          oscillating = MMax(oscillating - 1., 1.);
        }

        /* Merken der alten "Verhaeltnisse" */
        old_indicator_limiter = indicator_limiter;
        old_node_number = NodeListLength;

        if (aktueller_zeitschritt < 2)
          return;

        if (maximum_nodes*val < 0)
          if (indicator_limiter * help < 1.)
            /* Die Indikatoren sollen nur _vergroessert_ werden ... */
            help = 1. / indicator_limiter;

        /* Alle Modifikationen zuruecknehmen */
        if(!gueltig)
            help = 1. / indicator_limiter;

        /* In indicator_limiter steht die Produktsumme aller Modifikationen */
        indicator_limiter *= help;

        /* Begrenzung der Knotenzahl durch Verbiegen der Indikatoren */
        for (jj = 0; jj < AdaptGetNumAdaptQuantities(); jj++)
          for (ii = 0; ii < AdaptGetNumberInd(jj); ii++)
            for (i = 0; i < 3; i++)
              for (j = 0; j < 3; j++) {
                if (AdaptGetRefQuantity(jj) == 0) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 1) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                  ref_param = AdaptGetRefParam(jj, ii, i, 1, j) * help;
                  AdaptSetRefParam(jj, ii, i, 1, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 2) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                  ref_param = AdaptGetRefParam(jj, ii, i, 1, j) * help;
                  AdaptSetRefParam(jj, ii, i, 1, j, ref_param);
                }
                if (AdaptGetRefQuantity(jj) == 3) {
                  ref_param = AdaptGetRefParam(jj, ii, i, 0, j) * help;
                  AdaptSetRefParam(jj, ii, i, 0, j, ref_param);
                }
             }
        break;

    default:
        break;
    }                           /* end switch */

    return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: RefineEdgeValues
                                                                          */
/* Aufgabe:
   interpoliert Kantenmittelknoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X Knoten *kneu: neuer Mittelknoten
   E Knoten *kk1, *kk2: benachbarte Kantenknoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   16.07.1999  R.Kaiser   Erste Version
   27.11.1999  C.Thorenz  Etwas ueberarbeitet
                                                                          */
/**************************************************************************/
void RefineEdgeValues ( Knoten *kneu, Knoten *kk1, Knoten *kk2 )
{
  static int i;
  for (i=0;i<anz_nval;i++)
    if (nval_data[i].adapt_interpol)
       kneu->nval[i] = ( kk1->nval[i] + kk2->nval[i]) * 0.5;
    else
       kneu->nval[i] = nval_data[i].vorgabe;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RefinePlainValues
                                                                          */
/* Aufgabe:
   interpoliert Flaechenmittelknoten fuer alle Knotengroessen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X Knoten *knoten: neuer Mittelknoten
   E long *alte_eckknoten: Knotennummern der benachbarten Knoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   16.07.1999  R.Kaiser   Erste Version
   27.11.1999  C.Thorenz  Etwas ueberarbeitet
                                                                          */
/**************************************************************************/
void RefinePlainValues( Knoten *knoten, long *alte_eckknoten )
{
  static int i,j;
  static Knoten *eckknoten;

  for (i=0;i<anz_nval;i++) {
    if (nval_data[i].adapt_interpol) {
      knoten->nval[i] =0.;
      for (j=0;j<4;j++) {
        eckknoten = GetNode(alte_eckknoten[j]);
        knoten->nval[i] += eckknoten->nval[i];
      }
      knoten->nval[i] *= 0.25;
    } else {
      knoten->nval[i] = nval_data[i].vorgabe;
    }
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Refine3DElementValues
                                                                          */
/* Aufgabe:
   interpoliert Elementmittelknoten bei 3D
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X Knoten *knoten: neuer Mittelknoten
   E long *alte_eckknoten: Knotennummern der benachbarten Knoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   16.07.1999  R.Kaiser   Erste Version
   27.11.1999  C.Thorenz  Etwas ueberarbeitet
                                                                          */
/**************************************************************************/
void Refine3DElementValues( Knoten *knoten, long *alte_eckknoten )
{
  static int i,j;
  static Knoten *eckknoten;

  for (i=0;i<anz_nval;i++) {
    if (nval_data[i].adapt_interpol) {
      knoten->nval[i] =0.;
      for (j=0;j<8;j++) {
        eckknoten = GetNode(alte_eckknoten[j]);
        knoten->nval[i] += eckknoten->nval[i];
      }
      knoten->nval[i] *= 0.125;
    } else {
      knoten->nval[i] = nval_data[i].vorgabe;
    }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RefElementData
                                                                          */
/* Aufgabe:
   Erzeugt Elementdaten fuer verfeinerte Elemente
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Vorgaengerelements
   E long *neue_elemente: Nummern der Kindelemente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   27.06.1997  R.Kaiser   Aenderungen bzgl. der Verallgemeinerung der
                          femlib.c  (-> CalcElementJacobiMatrix)
   13.07.1999  R.Kaiser   Herausgeloest aus mod_*
                                                                          */

/**************************************************************************/
void RefElementData( long number, long *neue_elemente )
{
  static int i, j, k;


  j = ElGetElementChildsNumber(number);

  for (k=0;k<anz_eval;k++) {
    if (eval_data[k].adapt_interpol==1) {
      for (i=0;i<j;i++) {
        (ElGetElement(neue_elemente[i]))->eval[k] = (ElGetElement(number))->eval[k];
      }
    }
  }

  for (i=0;i<j;i++) {
    ADAPMakeMat(neue_elemente[i]);
  }

}

/**************************************************************************
 ROCKFLOW - Funktion: IrrNodeGetRegularNeighbors

 Aufgabe:
   Liefert regulaere Nachbarn eines irregulaeren Knotens

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
 E long knoten: irregulaerer Knoten
 R long *nachbarn: Feld mit regulaeren Nachbarn

 Ergebnis:
 Anzahl regulaerer Nachbarn

 Programmaenderungen:
   5/2000      C.Thorenz   Erste Version (aus Auftrennung von
                           EliminateIrrNodes, neuer Suchalgh. von RK)

**************************************************************************/
int IrrNodeGetRegularNeighbors (long knoten, long *nachbarn)
{
  static long k, l, m;
  static long *elemente, *flaechen;
  static int anz, anz_elem, anz_nachbarn;
  static Flaeche *fl;

  anz_nachbarn = 0;
  elemente = GetNode2DElems (knoten, &anz_elem);

  /* Irregulaerer Kantenknoten an 2D-Element */
  if ((GetNodeState (knoten) == -2) && (anz_elem))
    {
      anz_nachbarn = 2;
      nachbarn[0] = ElGetElementNodes (elemente[0])[0];
      for (k = 1; k < anz_elem; k++)
        {
          if (ElGetElementNodes (elemente[k])[0] != nachbarn[0])
            {
              nachbarn[1] = ElGetElementNodes (elemente[k])[0];
              break;
            }
        }
    }


  /* Irregulaerer Kantenknoten der an keinem 2D-Element haengt */
  if ((GetNodeState (knoten) == -2) && (!anz_elem))
    {
      elemente = GetNode3DElems (knoten, &anz_elem);
      anz_nachbarn = 2;
      nachbarn[0] = ElGetElementNodes (elemente[0])[0];
      for (k = 1; k < anz_elem; k++)
        {
          if (ElGetElementNodes (elemente[k])[0] != nachbarn[0])
            {
              nachbarn[1] = ElGetElementNodes (elemente[k])[0];
              break;
            }
        }
    }


  if (GetNodeState (knoten) == -4)
    {                           /* Irregulaerer Flaechenknoten */
      elemente = GetNode3DElems (knoten, &anz_elem);
      flaechen = ElGetElementPlains (ElGetElementPred (elemente[0]));
      for (k = 0; k < 6; k++)
        {                       /* Anzahl Flaechen */
          fl = GetPlain (flaechen[k]);
          anz = 0;
          for (l = 0; l < 4; l++)
            {                   /* Anzahl regulaerer Nachbarknoten */
              for (m = 0; m < 4; m++)
                {               /* Anzahl Knoten einer Flaeche */
                  if (fl->knoten[m] == ElGetElementNodes (elemente[l])[0])
                    {
                      anz++;
                      break;
                    }
                }
            }
          if (anz == 4)
            {                   /* Flaeche gefunden */
              break;
            }
        }
      anz_nachbarn = 4;         /* Jeder Irr. Knoten hat 4 Nachbarn auf der Flaeche */
      for (k = 0; k < 4; k++)
        nachbarn[k] = fl->knoten[k];
    }

  return anz_nachbarn;
}


/**************************************************************************
 ROCKFLOW - Funktion: EliminateIrrNodes

 Aufgabe:
   Eliminiert irregulaere Knoten aus dem fertigen Gleichungssystem

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X ergebnis

 Ergebnis:
   - void -

 Programmaenderungen:
   10/1998      C.Thorenz                     Erste Version
   22.10.1999   R.Kaiser                      Ueberarbeitet
                                              (Suchalgorithmus optimiert)
    5/2000      C.Thorenz                     Auftrennung in zwei Funktionen

**************************************************************************/
void EliminateIrrNodes (double *rechts)
{
  static long i, k;
  static long nachbarn[4], nachbarn_index[4];
  static int anz_nachbarn;

  for (i = 0; i < NodeListLength; i++)
    {
      /* Regulaere Nachbarknoten holen */
      anz_nachbarn = IrrNodeGetRegularNeighbors (NodeNumber[i], nachbarn);

      /* Index der Nachbarknoten im Glecihungssystem */
      for (k = 0; k < anz_nachbarn; k++)
        nachbarn_index[k] = GetNodeIndex (nachbarn[k]);

      /* Beginn der eigentlichen Elimination */
      if (anz_nachbarn)
        {
          /* Der irr. Knoten wird jetzt durch ausmultiplizieren der Spalte
             aus dem Gleichungssystem entfernt */
          MXEliminateIrrNode(i, anz_nachbarn, nachbarn_index, rechts);
        }                       /* endif */

    }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: BuildActiveElementsArray
                                                                          */
/* Aufgabe:
   Erstellt Feld ActiveElements mit den Nummern aller aktiven Elemente.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR         Erste Version
   11/1998     C.Thorenz   Zweite Version
   08/2000     RK          herausgeloest aus tools.c

                                                                           */
/**************************************************************************/
void BuildActiveElementsArray ( void )
{
  static long i, j, laenge;

  ActiveElements = (long *) Realloc(ActiveElements, anz_active_elements*sizeof(long));

  /* hoechsten Index+1 der Knoten holen */
  laenge = ElListSize();
  /* Verzeichnis aufstellen */
  j = 0;  /* Laufindex in ElementNumber-Feld */
  for (i=0;i<laenge;i++)  /* Schleife ueber Original-Elementnummern */
      if (ElGetElement(i)!=NULL)  /* Element existiert */
        if (ElGetElementActiveState(i))  /* aktives Element */
            ActiveElements[j++] = i;  /* in Indexfeld eintragen */
#ifdef TESTGLOBDAT
    if (j!=anz_active_elements)
        DisplayErrorMsg("RenumberElements: FEHLER IM ELEMENTVERZEICHNIS !!!!!");
#endif
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CriticalElements
                                                                          */
/* Aufgabe: Identifiziert kritische Elemente
            (Elemente mit irregulaeren Knoten)
            0: kein irregulaerer Knoten
            1: irregulaerer Knoten
            2: Geschwisterelement hat irregulaeren Knoten

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long index : Index des zu untersuchenden Elements
     R long *critele :
                          Element: index, wenn Status = 0
                                   Vorgaenger, wenn Status = 1 oder 2)
                                                                          */
/* Ergebnis:

   kritischer Status (s.o.),

                                                                          */
/* Programmaenderungen:
   08/2000      RK      Erste Version
   09/2000      CT      Zugriff umgestellt
                                                                          */
/**************************************************************************/
int CriticalElements (long index, long *critele)
{
  int typ, j, status = 0;
  long *ele_nodes;
  long *ele_nodes2;
  long vorgaenger;
  long *kinder;

  typ = ElGetElementType(index) - 1;

  switch (typ) {
    case 0:
      status = 0;
      *critele = index;
      break;
    case 1:

      ele_nodes = ElGetElementNodes(index);

      status = 0;
      *critele = index;


      if ((GetNodeState(ele_nodes[1]) == -2) || \
          (GetNodeState(ele_nodes[3]) == -2)) {
        status = 1;
        *critele = ElGetElementPred(index);
      }

      else {
        vorgaenger=ElGetElementPred(index);
        if (vorgaenger >= 0) {
          kinder = ElGetElementChilds(vorgaenger);
          for (j=0;j<4;j++) {
            if ((kinder[j] != index) && (ElGetElementActiveState(kinder[j]))) {
              ele_nodes2 = ElGetElementNodes(kinder[j]);

              if ((GetNodeState(ele_nodes2[1]) == -2) || \
                  (GetNodeState(ele_nodes2[3]) == -2)) {
                status = 2;
                *critele = vorgaenger;
                break;
              }
            }
          }
        }
      }
      break;
    case 2:
      ele_nodes = ElGetElementNodes(index);

      status = 0;
      *critele = index;

      if ((GetNodeState(ele_nodes[2]) == -4) ||
          (GetNodeState(ele_nodes[5]) == -4) ||
          (GetNodeState(ele_nodes[7]) == -4) ||
          (GetNodeState(ele_nodes[1]) == -2) ||
          (GetNodeState(ele_nodes[3]) == -2) ||
          (GetNodeState(ele_nodes[4]) == -2)) {


        status = 1;
        *critele = ElGetElementPred(index);
      }
      else {
        vorgaenger=ElGetElementPred(index);
        if (vorgaenger >= 0) {
          kinder = ElGetElementChilds(vorgaenger);
          for (j=0;j<8;j++) {
            if ((kinder[j] != index) && (ElGetElementActiveState(kinder[j]))) {
              ele_nodes2 = ElGetElementNodes(kinder[j]);

              if ((GetNodeState(ele_nodes2[2]) == -4) ||
                  (GetNodeState(ele_nodes2[5]) == -4) ||
                  (GetNodeState(ele_nodes2[7]) == -4) ||
                  (GetNodeState(ele_nodes2[1]) == -2) ||
                  (GetNodeState(ele_nodes2[3]) == -2) ||
                  (GetNodeState(ele_nodes2[4]) == -2)) {

                status = 2;
                *critele = vorgaenger;
                break;
              }
            }
          }
        }
      }
      break;
  }
  return status;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: AdaptAssignEdgeNodesToElements
                                                                          */
/* Aufgabe:
   Ermittelt die Kantenknoten (hängende Knoten) der Elemente und
   trägt diese bei den Elementen ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/2002     RK         Erste Version

                                                                          */
/**************************************************************************/
void AdaptAssignEdgeNodesToElements ( void )
{
  static long i, j, active_element; 
  static int typ;
  static int anz_kantenknoten;
  static long *knotenliste = NULL;
  static long *kanten = NULL;
 /* static long *knotenliste2 = NULL;
  static int anz_knoten; */

  for (i=0;i<anz_active_elements;i++) {  /* Schleife ueber alle aktiven Elemente */
    active_element = ActiveElements[i];
    typ = ElGetElementType(active_element) - 1; /* Elementtyp */
 
    switch (typ) {
      case 0:
        break;
      case 1:
        kanten = ElGetElementEdges(active_element);
        anz_kantenknoten = 4;    /* Default: 4 */
        knotenliste = (long *) Malloc(anz_kantenknoten*sizeof(long));
        
        for (j=0;j<4;j++) {
          knotenliste[j] = GetEdge(kanten[j])->knoten[2];
        } 
      
        ElSetEdgeNodes2D(active_element,knotenliste,anz_kantenknoten);
      
      
        knotenliste = NULL; 
        kanten = NULL; 
        
        break;
      case 2:
        break;  
    }
  }

  /* for (i=0;i<anz_active_elements;i++) { 
     active_element = ActiveElements[i];
     knotenliste2 = ElGetEdgeNodes2D(active_element,&anz_knoten);
     
     DisplayMsgLn("Element: ");
     DisplayLong(active_element);
     DisplayMsgLn("");
     DisplayMsg("Kantenknoten: ");
     for (j=0;j<anz_knoten;j++) {
       DisplayLong(knotenliste2[j]);DisplayMsg("  ");
     }
     DisplayMsgLn("");
   }*/
    
}


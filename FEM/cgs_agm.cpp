/**************************************************************************/
/* ROCKFLOW - Modul: cgs_agm.c
                                                                          */
/* Aufgabe:
   Aufstellen des Gleichungssystems fuer das Modul aGM.

                                                                          */
/* Programmaenderungen:
   04/1997     OK           Implementierung
   16.07.1997  R.Kaiser     Interpol_q_XD -> adaptiv.c
   25.07.1997  R.Kaiser     CalcIrrNodes_ASM umbenannt zu
                            CalcIrrNodeVals (erweitert) -> adaptiv.c
   25.07.1997  R.Kaiser     Adaptivitaet aus dem Modul herausgenommen
                            (DelIrrNodesXD -> adaptiv.c)
   16.09.1997  O.Kolditz    Speicheroptimierung
   11/1999     RK           EliminateIrrNodes()
   03/2003     RK           Quellcode bereinigt, Globalvariablen entfernt
   10/2004 OK PCS2    

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTGM
#define noTESTCGS_AGM

/* Header / Andere intern benutzte Module */
#include "elements.h"
#include "nodes.h"
#include "adaptiv.h"
#include "mathlib.h"
#include "matrix.h"
#include "femlib.h"
#include "rf_pcs.h" //OK_MOD"
#include "rf_tim_new.h"
#include "int_agm.h"
#include "par_ddc.h"

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
last modified:
**************************************************************************/
void GMAssembleMatrix(double *rechts,double *ergebnis,double aktuelle_zeit,\
                      CRFProcess *m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  aktuelle_zeit = aktuelle_zeit;
  ergebnis = ergebnis;
#ifdef PARALLEL
  rechts = rechts;
  int j;
  CPARDomain *m_dom = NULL;
  int no_domains =(int)dom_vector.size();
  for(j=0;j<no_domains;j++){
    m_dom = dom_vector[j];
    m_dom->AssembleMatrix(m_pcs);
    ///OK m_dom->WriteMatrix();
  }
#else
  long elanz;
  long index;
  /* Beginn der Elementschleife */
  elanz = ElListSize();
  for (index=0;index<elanz;index++) {
    if (ElGetElement(index)!=NULL) {
      if (ElGetElementActiveState(index)) {
        switch(ElGetElementType(index)){
          case 1:
            GMMakeElementEntry1D(index,rechts,ergebnis,m_pcs);
            break;
          case 2:
            GMMakeElementEntry2D(index,rechts,ergebnis,m_pcs);
            break;
        }
      }
    }
  }
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);
#endif
}

/**************************************************************************
        ROCKFLOW - Funktion: MakeGS1D_AGM

        Aufgabe:
        Beitraege der 1D-Elemente berechnen und in das Gleichungssytem
        einarbeiten

        Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
        E long index: Elementnummer
        R double *rechts: rechte Seite GS (Vektor)
        R double *ergebnis: Ergebnis des GS
        E double aktuelle_zeit: aktueller Zeitpunkt


        Ergebnis:
        - void -

        Programmaenderungen:

        letzte Aenderung: OK 16.09.97

 **************************************************************************/
void GMMakeElementEntry1D(long index,double *rechts,double *ergebnis, CRFProcess* m_pcs)
{
  /* Variablen */
  static long i, j, elanz;
  static int nn, nn2;
  static double dt_inverse, theta;
  static long node_index[2];
  static long *element_nodes;
  static double *capacitance_matrix;
  static double *conductance_matrix;
  static double *gravity_vector;
  static double p_old_time[2];
  static double p_old_iter[2];
  static double explicit_pressure[2];
  static double left_matrix[4];
  static double right_matrix[4];
  static double dummy[4];
  static double pressure_average=0.0;
  static double ddummy;
  ddummy = ergebnis[0];
  /* Funktionsinterne Vektoren und Matrizen initialisieren */
  MNulleVec(p_old_iter,2);
  MNulleVec(explicit_pressure,2);
  MNulleMat(left_matrix,2,2);
  MNulleMat(right_matrix,2,2);
  MNulleMat(dummy,2,2);
  /* Parameter fuer die Zeitdiskretisierung */
  dt_inverse = 1.0 / dt; /* dt=aktueller Zeitschritt */
  theta = 1.;            /* Wichtungsparameter fuer den Zeitkollokationspunkt */
  /* Parameter fuer die Ortsdiskretisierung */
  element_nodes = ElGetElementNodes(index); /* Original-Knotennummern des Elements */
  elanz = ElListSize(); /* Element-Anzahl */
  /* Elementmatrizen und -vektoren bereitstellen */
  capacitance_matrix = GMGetElementCapacitanceMatrix(index,m_pcs->pcs_number);
  conductance_matrix = GMGetElementConductanceMatrix(index,m_pcs->pcs_number);
  gravity_vector = GMGetElementGravityVector(index,m_pcs->pcs_number);
  nn = ElNumberOfNodes[0];   /* Knotenanzahl nn muss 2 sein ! */
  nn2 = nn * nn;
  /* Original-Knotennummern des Elements gegen Indizes im Gleichungssystem ersetzen */
  for (i=0;i<nn;i++)
    node_index[i] = GetNodeIndex(element_nodes[i]);
  /* Ergebnisse aus letztem Iterationsschritt bereitstellen
     und mittleren Druck im Element berechnen */
  for (i=0;i<nn;i++)
    p_old_iter[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
  pressure_average = 0.5*(p_old_iter[0]+p_old_iter[1]);
  /* Beitraege des betrachteten 1D-Elementes
     zur linken Systemmatrix (neuer Zeitpunkt) berechnen */
  for(i=0;i<nn2;i++)
    left_matrix[i] = capacitance_matrix[i] * dt_inverse \
                   + conductance_matrix[i] * pressure_average * theta;
#ifdef TESTCGS_AGM
    MZeigMat(left_matrix,2,2,"Linke Systemmatrix");
#endif
  /* Einspeichern in die linke Systemmatrix */
  for (i=0;i<nn;i++)
    for (j=0;j<nn;j++)
      MXInc(node_index[i],node_index[j],left_matrix[i*2+j]);
  /* Beitraege des betrachteten 1D-Elementes
     zur rechten Systemmatrix (alter Zeitpunkt) berechnen */
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix[i] * dt_inverse \
                    - conductance_matrix[i] * pressure_average * (1.0 - theta);
  /* Ergebnisse aus letztem Zeitschritt bereitstellen */
  for (i=0;i<nn;i++)
    p_old_time[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",0));
  MMultMatVec(right_matrix,2,2,p_old_time,2,explicit_pressure,2);
  /* Einspeichern in die rechte Seite */
  for (i=0;i<nn;i++) {
    rechts[node_index[i]] += explicit_pressure[i];
    rechts[node_index[i]] -= gravity_vector[i];
  }
  /* Speicherfreigaben */
  element_nodes = NULL;
}
/**************************************************************************
        ROCKFLOW - Funktion: MakeGS2D_AGM

        Aufgabe:
        Beitraege der 2D-Elemente berechnen und in das Gleichungssystem
        einarbeiten

        Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
        E long index: Elementnummer
        R double *rechts: rechte Seite GS (Vektor)
        R double *ergebnis: Ergebnis des GS
        E double aktuelle_zeit: aktueller Zeitpunkt

        Ergebnis:
        - void -

        Programmaenderungen:
        03/1997     R.Kaiser      Uebertragen aus dem aTM
                                  und angepasst an das aSM
        04/1997     R.Kaiser      Druckformulierung

        02.05.97    OK            letzte Aenderung

 **************************************************************************/
void GMMakeElementEntry2D(long index,double *rechts,double *ergebnis, CRFProcess* m_pcs)
{
 static int i,j;
 static int nn,nn2;
 static long *element_nodes;
 static long node_index[4];
 static double dt_inverse,theta;
 static double *capacitance_matrix;
 static double *conductance_matrix;
 static double *gravity_vector;
 static double p_old_iter[4];
 static double p_old_time[4];
 static double pressure_average=0.0;
 static double explicit_pressure[4];
 static double left_matrix[16];
 static double right_matrix[16];
 /* Variablen fuer die Netz-Adaption
 static long *elem;
 static long regkno;
 static double gravity_vector2[4];
 static double explicit_pressure2[4];
 static double permutation_matrix[16];
 static double trans_permutation_matrix[16];
 static double perm_left_matrix[16];
 static double dummy[16]; */
  static double ddummy;
  ddummy = ergebnis[0];

 /* Funktionsinterne Vektoren und Matrizen initialisieren */
 MNulleVec(p_old_iter,4);
 MNulleVec(p_old_time,4);
 MNulleVec(explicit_pressure,4);
 MNulleMat(left_matrix,4,4);
 MNulleMat(right_matrix,4,4);

 /* Parameter fuer die Zeitdiskretisierung */
 dt_inverse = 1.0 / dt; /* dt=aktueller Zeitschritt */
 theta = 1.;            /* Wichtungsparameter fuer den Zeitkollokationspunkt */

 /* Parameter fuer die Ortsdiskretisierung */
 element_nodes = ElGetElementNodes(index); /* Original-Knotennummern des Elements */
 nn=ElNumberOfNodes[1];
 nn2 = nn * nn;

 /* Elementmatrizen und -vektoren bereitstellen */
 capacitance_matrix = GMGetElementCapacitanceMatrix(index,m_pcs->pcs_number);
 conductance_matrix = GMGetElementConductanceMatrix(index,m_pcs->pcs_number);
 gravity_vector     = GMGetElementGravityVector(index,m_pcs->pcs_number);
#ifdef TESTCGS_AGM
   MZeigMat(capacitance_matrix,4,4,"Capacitance matrix");
   MZeigMat(conductance_matrix,4,4,"Conductance matrix");
#endif
 for(i=0;i<nn;i++)
   node_index[i]=GetNodeIndex(element_nodes[i]);
  /* Ergebnisse aus letztem Iterationsschritt bereitstellen
     und mittleren Druck im Element berechnen */
 for (i=0;i<nn;i++)
   p_old_iter[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",1));
 pressure_average = 0.25*(p_old_iter[0]+p_old_iter[1]+ \
                          p_old_iter[2]+p_old_iter[3]);
 /* Beitraege des betrachteten 2D-Elements
    zur linken Systemmatrix (neuer Zeitpunkt) berechnen */
 for(i=0;i<nn2;i++)
   left_matrix[i] = capacitance_matrix[i] * dt_inverse \
                  + conductance_matrix[i] * pressure_average * theta;
 /* Beitraege des betrachteten 2D-Elements
    zur rechten Systemmatrix (alter Zeitpunkt) berechnen */
 for (i=0;i<nn2;i++)
   right_matrix[i] = capacitance_matrix[i] * dt_inverse \
                   - conductance_matrix[i] * pressure_average * (1.0 - theta);
 /* Rechte Systemmatrix mit den Ergebnis-Vektor des alten Zeitpunkts multiplizieren
    - Ergebnisse aus letztem Zeitschritt bereitstellen */
 for (i=0;i<nn;i++)
   p_old_time[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",0));
 MMultMatVec(right_matrix,4,4,p_old_time,4,explicit_pressure,4);
 /* irregulaere Knoten eliminieren */
 if (GetRFControlGridAdapt())
   if (AdaptGetMethodIrrNodes() == 1)
     DelIrrNodes2D(index,node_index,left_matrix,explicit_pressure);
 /* Einspeichern in die linke Systemmatrix */
 for(i=0;i<nn;i++)
   for(j=0;j<nn;j++)
     MXInc(node_index[i],node_index[j],left_matrix[i*nn+j]);
 /* Einspeichern in die rechte Systemmatrix */
 for (i=0;i<nn;i++) {
   rechts[node_index[i]] += explicit_pressure[i];
   rechts[node_index[i]] -= gravity_vector[i];
 }
 /* MZeigMat(left_matrix,4,4,"left_matrix nach irr.Kno."); */
  /* Speicherfreigaben */
  element_nodes = NULL;
}


/**************************************************************************
   ROCKFLOW - Funktion: MakeGS3D_AGM

   Aufgabe:
   Beitraege der 3D-Elemente berechnen und in das Gleichungssytem
   einarbeiten

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long index: Elementnummer
   R double *rechts: rechte Seite GS (Vektor)
   R double *ergebnis: Ergebnis des GS
   E double aktuelle_zeit: aktueller Zeitpunkt

   Ergebnis:
   - void -

   Programmaenderungen:
   07/97         O.Kolditz     Anpassungen fuer Gasmodell
   25.07.1997    R.Kaiser      Adaptivitaet herausgenommen
                               DelIrrNodes3D -> adaptiv.c

   letzte Aenderung: OK 16.09.97

 **************************************************************************/
void MakeGS3D_AGM ( long index, double *rechts, double *ergebnis )
{
 static double *capacitance_matrix,*conductance_matrix,dt_inverse,theta;
 static long nn,nn2,*element_nodes;
 static int i,j;
 static long node_index[8];
 static double left_matrix[64];
 static double right_matrix[64];
 static double pressure_average=0.0;
 static double p_old_time[8];
 static double p_old_iter[8];
 static double explicit_pressure[8];
 static double *gravity_vector;
  static double ddummy;
  ddummy = ergebnis[0];


 /* Matrizen, Vektoren initialisieren */
 MNulleMat(left_matrix,nn,nn);
 MNulleMat(right_matrix,nn,nn);
 MNulleVec(p_old_iter,nn);
 MNulleVec(p_old_time,nn);
 MNulleVec(explicit_pressure,nn);
 MNulleVec(gravity_vector,nn);


 dt_inverse = 1.0 / dt; /* dt=aktueller Zeitschritt */
 theta = 1.; /* Wichtungsparameter fuer den Zeitkollokationspunkt */

 element_nodes=ElGetElementNodes(index);

 /* Elementmatrizen und -Vektoren bereitstellen */
 capacitance_matrix=AGMGetElementCapacitanceMatrix(index);
 conductance_matrix=AGMGetElementConductanceMatrix(index);

 nn = ElNumberOfNodes[2];
 nn2 = nn * nn;

 /* lokale Knotennummern des Elements gegen globale
    Knotennummern (Indizes im Gesamtgleichungssystem) ersetzen */
 for(i=0;i<nn;i++)
     node_index[i]=GetNodeIndex(element_nodes[i]);


  /* Ergebnisse aus letztem Iterationsschritt bereitstellen
     und mittleren Druck im Element berechnen */
  for (i=0;i<nn;i++)
    p_old_iter[i] = AGMGetNodePress1(element_nodes[i]);
/*    p_old_iter[i] = ergebnis[node_index[i]]; Speicheroptimierung */
  pressure_average = 0.125 * \
                    (p_old_iter[0]+p_old_iter[1]+p_old_iter[2]+p_old_iter[3]+\
                     p_old_iter[4]+p_old_iter[5]+p_old_iter[6]+p_old_iter[7]);


 /* Beitraege des betrachteten 3D-Elements
    zur linken Systemmatrix (neuer Zeitpunkt) berechnen */
 for(i=0;i<nn2;i++)
   left_matrix[i] = capacitance_matrix[i] * dt_inverse \
                  + conductance_matrix[i] * pressure_average * theta;

 /* Beitraege des betrachteten 3D-Elements
    zur rechten Systemmatrix (alter Zeitpunkt) berechnen */
 for (i=0;i<nn2;i++)
   right_matrix[i] = capacitance_matrix[i] * dt_inverse \
                   - conductance_matrix[i] * pressure_average * (1.0 - theta);

 /* Rechte Systemmatrix mit den Ergebnis-Vektor des alten Zeitpunkts multiplizieren
    - Ergebnisse aus letztem Zeitschritt bereitstellen */
 for (i=0;i<nn;i++)
   p_old_time[i] = AGMGetNodePress1(element_nodes[i]);
/*   p_old_time[i] = AGMGetNodePress1(element_nodes[i]); */

 MMultMatVec(right_matrix,nn,nn,p_old_time,nn,explicit_pressure,nn);


 /* irregulaere Knoten eliminieren */
 if (GetRFControlGridAdapt())
   if (AdaptGetMethodIrrNodes() == 1)
     DelIrrNodes3D(index,node_index,left_matrix,explicit_pressure);


 /* Einspeichern in die linke Systemmatrix */
 for(i=0;i<nn;i++)
   for(j=0;j<nn;j++)
     MXInc(node_index[i],node_index[j],left_matrix[i*nn+j]);

 /* Einspeichern in die rechte Systemmatrix */
 for (i=0;i<nn;i++) {
   rechts[node_index[i]] += explicit_pressure[i];
   rechts[node_index[i]] -= gravity_vector[i];
 }

 /* MZeigMat(left_matrix,4,4,"left_matrix nach irr.Kno."); */

  /* Speicherfreigaben */
  element_nodes = NULL;

 /* MZeigMat(left_matrix,8,8,"left_matrix nach irr.Kno.");
 MZeigVec(explicit_pressure,8,"explicit_pressure nach irr.Kno.");
 MZeigVec(rechts,NodeListLength,"rechts"); */

}


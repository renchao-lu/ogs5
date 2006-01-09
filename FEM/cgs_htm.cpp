/*************************************************************************
 ROCKFLOW - Modul: cgs_htm.c

 Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul HTM.


 Programmaenderungen:
   04/1999   OK   Implementierung
   06/1999   OK   Warnungen entfernt
   11/1999   RK   EliminateIrrNodes()
   02/2000   RK   Anpassung an Gitteradaption
   10/2000   CT   Einspeichern in GLS abstrahiert (IncorporateMatrix)
   11/2000   OK   Verallgemeinerung der Assemblierungsfkt fuer alle Elemente
                  (MakeElementEntryEQS_HTM)
   11/2000   OK   Speicheroptimierte Variante
   11/2000   CT   Kernel-Konfiguration, Wiedereinbau "altes GLS"
   03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt
   05/2003   OK   MakeElementEntryEQS_HTM: only new keywords
    

*************************************************************************/
#include "stdafx.h" 
#include "makros.h"
#include "mathlib.h"
#include "matrix.h"
#include "femlib.h"                    /* ? IncorporateMatrix -> matrix.c oder rfsolver.c */
#include "adaptiv.h"
#include "rf_pcs.h"
#include "rf_pcs.h"
#include "elements.h"
#include "nodes.h"
#include "rf_mfp_new.h"
#include "rf_tim_new.h"
#include "int_htm.h"

#define noTESTCGS_HTM

static void HTMMakeElementEntryEQS(long index,CRFProcess*m_pcs);
/* Matrizen fuer speicheroptimierte Variante */
//static double capacitance_matrix_line[4];
//static double capacitance_matrix_quad[16];
//static double dispersion_matrix_line[4];
//static double dispersion_matrix_quad[16];
//static double advection_matrix_line[4];
//static double advection_matrix_quad[16];
static double capacitance_matrix_htm[64];
static double dispersion_matrix_htm[64];
static double advection_matrix_htm[64];

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
11/2004 OK Implementation
last modified:
**************************************************************************/
void HTMAssembleMatrix(CRFProcess *m_pcs)
{
  long elanz;
  long index;
  /* Beginn der Elementschleife */
  elanz = ElListSize();
  for (index=0;index<elanz;index++) {
    if (ElGetElement(index)!=NULL) {
      if (ElGetElementActiveState(index)) {
        HTMMakeElementEntryEQS(index,m_pcs);
      }
    }
  }
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(m_pcs->eqs->b);
}
/**************************************************************************
   ROCKFLOW - Funktion: MakeElementEntryEQS_HTM

   Aufgabe:
   Elementbeitraege zum Gleichungssystem (EQS)
   (System-Matrix und RHS-Vektor einbauen)

   Formalparameter:
   long index:     Element-Nummer
   double *rechts: RHS-Vektor

   Ergebnis:
   - void -

   Programmaenderungen:
   06/1999   OK   Implementierung
   07/1997   RK   Adaptivitaet herausgenommen, DelIrrNodes3D -> adaptiv.c
   10/2000   CT   Einspeichern in GLS abstrahiert
   11/2000   OK   Speicheroptimierung
   11/2000   OK   Assemblierungsfunktion fuer alle Elementtypen verallgemeinert
   11/2000   CT   Zugriff auf Zeitkollokation ueber Kernel-Objekt
   01/2002   MK   COUNTER_CEL_HTM sollte auch hier berücksichtigt werden
   05/2003   OK   only new keywords           (TEMPERATURE0)
   06/2003   MK   GetTimeCollocationGlobal_HTM() mit TEMPERATURE0  

 **************************************************************************/
static void HTMMakeElementEntryEQS(long index,CRFProcess*m_pcs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *mass = NULL, *adv = NULL, *disp = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double rhs_vector[8];

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);


/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  if (memory_opt == 0)
    {                                  /* gespeicherte Element-Matrizen holen */
      mass = HTMGetElementHeatCapacitanceMatrix(index,m_pcs->pcs_number);
      disp = HTMGetElementHeatDiffusionMatrix(index,m_pcs->pcs_number);
      adv = HTMGetElementHeatAdvectionMatrix(index,m_pcs->pcs_number);
    }
  else if (memory_opt == 1)
    {                                  /* jede Element-Matrix berechnen */
#ifdef COUNTER_CEL_HTM
      printf("\r                  Element-Matrix: %ld", index);
#endif
      HTMCalcElementMatrix(index,NULL);
    }
  else if (memory_opt == 2)
    {                                  /* Element-Prototypen benutzen

                                          material_group = ElGetElementGroupNumber(index);
                                          mass=HTMGetElementHeatCapacitanceMatrixGroup(material_group);
                                          disp=HTMGetElementHeatDiffusionMatrixGroup(material_group);
                                          adv=HTMGetElementHeatAdvectionMatrixGroup(material_group);
                                        */
    }

#ifdef TESTCGS_HTM
  MZeigMat(mass, nn, nn, "Heat capacitance matrix");
  MZeigMat(disp, nn, nn, "Heat diffusion-dispersion matrix");
  MZeigMat(adv, nn, nn, "Heat advection matrix");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = m_pcs->m_num->ls_theta;
/*---------------------------------------------------------------------------*/
  /* Beitraege des betrachteten 2D-Elements
     zur linken Systemmatrix (neuer Zeitpunkt) berechnen */
  if (memory_opt == 0)
    {
      for (i = 0; i < nn2; i++)
        left_matrix[i] = edt * mass[i] + theta * (adv[i] + disp[i]);
    }
  else if (memory_opt == 1)
    {
      for (i = 0; i < nn2; i++)
        left_matrix[i] = edt * capacitance_matrix_htm[i] + \
            theta * (advection_matrix_htm[i] + dispersion_matrix_htm[i]);
    }


/*---------------------------------------------------------------------------*/
  /* Beitraege des betrachteten 2D-Elements
     zur rechten Systemmatrix (alter Zeitpunkt) berechnen */
  if (memory_opt == 0)
    {
      for (i = 0; i < nn2; i++)
        right_matrix[i] = edt * mass[i] - (1.0 - theta) * (adv[i] + disp[i]);
    }
  else if (memory_opt == 1)
    {
      for (i = 0; i < nn2; i++)
        {
          right_matrix[i] = edt * capacitance_matrix_htm[i] - \
              (1.0 - theta) * (advection_matrix_htm[i] + dispersion_matrix_htm[i]);
        }
    }

  /* Ergebnisse aus letztem Zeitschritt uebertragen */
  for (i = 0; i < nn; i++)
    u_old_time[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("TEMPERATURE1",0));
  /* Rechte-Seite-Vektor bilden */
  MMultMatVec(right_matrix, nn, nn, u_old_time, nn, rhs_vector, nn);
#ifdef HEAT_RADIATION
/*JDJ Abstrahlung einbauen*/
/*Q=epsilon*sigma*area*(Tsolid^4-Tair^4)*/
long group = ElGetElementGroupNumber(index);
double epsilon=0.8;
double sigma=5.67051e-8;
double area=0.01;
double rad=0.0;
double t_old_iter[4], temp_average,T_air=298.;
int heat_radiation = 0;
if (heat_radiation == 1){
if (group==1){
  /* Ergebnisse aus letztem Iterationsschritt bereitstellen
     und mittlere Temperatur im Element berechnen */
 for (i=0;i<nn;i++)
   t_old_iter[i] = HTMGetNodeTemp0(element_nodes[i]);

 temp_average = 0.25*(t_old_iter[0]+t_old_iter[1]+ \
                          t_old_iter[2]+t_old_iter[3]);

rad = epsilon * sigma * area * (pow(temp_average,4)-pow(T_air,4));

for (i = 0; i < nn; i++)
rhs_vector[i] -= rad;
}
}
//rad_total += rad;
#endif
/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix,rhs_vector,m_pcs->eqs->b);

#ifdef TESTCGS_HTM
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector, NodeListLength, "rhs_vector");
  MZeigVec(rechts, NodeListLength, "rechts");
#endif

}

/**************************************************************************
   ROCKFLOW - Funktion: CalcFlux_HTM

   Aufgabe:

   Waermefluss ermitteln.

   !!! Funktion ist noch nicht fertig, nur fuer Einphasensysteme geeignet !!!

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *rechts: Speicher fuer rechte Seite GS (Vektor) und Rueckgabe der Fluesse
   R double *ergebnis: Ergebnis des GS

   Ergebnis:
   - void -

      6/2001   CT   Erste Version
 **************************************************************************/
/*OK
void CalcFlux_HTM(double *rechts, double *ergebnis)
{
    double *residuum;
    long i;
	NUMERICALPARAMETER *htm_numerics = NumericsGetPointer("PRESSURE0");
    double theta = htm_numerics->time_collocation;
    // Initialisieren der Felder 
    MXInitMatrix();
    MNulleVec(rechts, NodeListLength);
CRFProcess* m_pcs = NULL;
    HTMAssembleMatrix(m_pcs);
    residuum = (double *) Malloc(NodeListLength * sizeof(double));
    // Das Residuum enthaelt diffusive Fluesse und Quellen und Senken 
    MXResiduum(ergebnis, rechts, residuum);
    // Auch advektive Fluesse beruecksichtigen
    for (i = 0l; i < NodeListLength; i++)
      residuum[i] -= ergebnis[i] * HTMGetNodeFlux(0, NodeNumber[i], -1, 1) * mfp_vector[0]->HeatCapacity(0,NULL,theta);  //SB: element = 0 always
    MKopierVec(residuum,rechts,NodeListLength);
    MMultVecSkalar(rechts,-1.,NodeListLength);
    residuum = (double *) Free(residuum);
}
*/


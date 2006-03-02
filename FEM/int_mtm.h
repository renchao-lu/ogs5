/**************************************************************************/
/* ROCKFLOW - Modul: int_mtm.h
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns MTM zu den diesen Rechenkern benutzenden Modulen bereit.
   
   letzte Aenderung: RK 03/2003
                                                                          */
/**************************************************************************/

#ifndef int_mtm_INC

#define int_mtm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_mtm.h"
#include "cel_mtm.h"


/* Deklarationen */

/* Elementdaten-Schnittstelle */

extern VoidFuncLD MTMSetElementRetardFac;
  /* Setzt den Retardationsfaktor des Elements number */
extern VoidFuncLD MTMSetElementDt;
  /* Setzt Courant/Neumann-Zeitschritt des Elements number */
extern VoidFuncLD MTMSetElementPecletNum;
  /* Setzt Peclet-Zahl des Elements number */
extern VoidFuncLDX MTMSetElementAdvMatrix;
  /* Setzt die Advektionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementDispMatrix;
  /* Setzt die Dispersionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementMassMatrix;
  /* Setzt die Massen-/Sorptionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementSorptionMatrix;
  /* Setzt die Sorptionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementSorptionIsothermMatrix;
  /* Setzt die Sorptionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementDecayMatrix;
  /* Setzt die Zerfallsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementDissolutionMatrix;
  /* Setzt die Loesungsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementDissolutionVector;
  /* Setzt den Loesungsvektor des Elements number */
extern VoidFuncLDX MTMSetElementSorptionVector;
  /* Setzt den Loesungsvektor des Elements number */
extern VoidFuncLDX MTMSetElementReactionVector;   /* ah neq */
  /* Setzt den Reaktionsvektor des Elements number */
extern VoidFuncLDX MTMSetElementReactionMatrix;
  /* Setzt die Reaktionsmatrix des Elements number */
extern VoidFuncLDX MTMSetElementChainReactionVector;
  /* Setzt den Kettenreaktionsvektor des Elements number */
extern VoidFuncLD MTMSetElementDDummy;
  /* Setzt Double-Wert des Elements number */

extern DoubleXFuncLong MTMGetElementAdvMatrix;
  /* Liefert die Advektive Matrix des Elements number */
extern DoubleXFuncLong MTMGetElementDispMatrix;
  /* Liefert die Dispersive Matrix des Elements number */
extern DoubleXFuncLong MTMGetElementMassMatrix;
  /* Liefert die Massen-/Sorptionsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementSorptionMatrix;
  /* Liefert die Sorptionsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementSorptionIsothermMatrix;
  /* Liefert die Sorptionsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementDecayMatrix;
  /* Liefert die Zerfallsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementDissolutionMatrix;
  /* Liefert die Loesungsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementDissolutionVector;
  /* Liefert den Loesungsvektor des Elements number */
extern DoubleXFuncLong MTMGetElementSorptionVector;
  /* Liefert den Loesungsvektor des Elements number */
extern DoubleXFuncLong MTMGetElementReactionVector;  /* ah neq */
  /* Liefert den Reaktionsvektor des Elements number */
extern DoubleXFuncLong MTMGetElementReactionMatrix;
  /* Liefert die Reaktionsmatrix des Elements number */
extern DoubleXFuncLong MTMGetElementChainReactionVector;
  /* Liefert den Kettenreaktionsvektor des Elements number */
extern DoubleFuncLong MTMGetElementRetardFac;
  /* Liefert den Retardationsfaktor des Elements number */
extern DoubleFuncLong MTMGetElementDt;
  /* Liefert Courant-Zeitschritt des Elements number */
extern DoubleFuncLong MTMGetElementPecletNum;
  /* Liefert Peclet-Zahl des Elements number */
extern DoubleFuncLong MTMGetElementDDummy;
  /* Liefert Double-Wert des Elements number */

extern VoidFuncLong MTMDeleteElementAdvMatrix;
  /* Erdet den Advektionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementDispMatrix;
  /* Erdet den Dispersionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementMassMatrix;
  /* Erdet den Massen-/Sorptionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementSorptionMatrix;
  /* Erdet den Sorptionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementSorptionIsothermMatrix;
  /* Erdet den Sorptionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementDecayMatrix;
  /* Erdet den Zerfallsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementDissolutionMatrix;
  /* Erdet den Loesungsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementDissolutionVector;
  /* Erdet den Loesungsvektor-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementSorptionVector;
  /* Erdet den Loesungsvektor-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementReactionVector;
  /* Erdet den Reaktionsvektor-Zeiger des Elements number */  /* ah neq */
extern VoidFuncLong MTMDeleteElementReactionMatrix;
  /* Erdet die Reaktionsmatrix-Zeiger des Elements number */
extern VoidFuncLong MTMDeleteElementChainReactionVector;
  /* Erdet den Kettenreaktionsvektor-Zeiger des Elements number */


extern VoidFuncLID MTMSetElementFluidDensity;
  /* Setzt die Dichte des Elements number */
extern DoubleFuncLI MTMGetElementFluidDensity;
  /* Liefert die Dichte des Elements number */
extern VoidFuncLD MTMSetElementSoilPorosity;
  /* Setzt die Dichte des Elements number */
extern DoubleFuncLong MTMGetElementSoilPorosity;
  /* Liefert die Dichte des Elements number */


/* Knotendaten-Schnittstelle */
extern DoubleFuncLong MTMGetNodePress;
  /* Liefert Druck des Knotens number */
extern VoidFuncLD MTMSetNodePress;
  /* Setzt Druck des Knotens number */
extern DoubleFuncLI MTMGetNodeConc;
  /* Liefert Konzentration der Substanz i am Knoten number */
extern VoidFuncLID MTMSetNodeConc;
  /* Setzt Konzentration der Substanz i am Knoten number */
extern DoubleFuncLong MTMGetNodeConc0;
  /* Liefert Konzentration 0 am Knoten number */
extern VoidFuncLD MTMSetNodeConc0;
  /* Setzt Konzentration 0 am Knoten number */
extern DoubleFuncLong MTMGetNodeConc1;
  /* Liefert Konzentration 1 am Knoten number */
extern VoidFuncLD MTMSetNodeConc1;
  /* Setzt Konzentration 1 am Knoten number */

extern DoubleFuncLI MTMGetNodeSorb;
  /* Liefert Konzentration der Substanz i am Knoten number */
extern VoidFuncLID MTMSetNodeSorb;
  /* Setzt Konzentration der Substanz i am Knoten number */
extern DoubleFuncLong MTMGetNodeSorb0;
  /* Liefert Konzentration 0 am Knoten number */
extern VoidFuncLD MTMSetNodeSorb0;
  /* Setzt Konzentration 0 am Knoten number */
extern DoubleFuncLong MTMGetNodeSorb1;
  /* Liefert Konzentration 1 am Knoten number */
extern VoidFuncLD MTMSetNodeSorb1;
  /* Setzt Konzentration 1 am Knoten number */

extern DoubleFuncLong MTMGetNodeTemp0;
  /* Liefert Temperatur 0 am Knoten number */
extern VoidFuncLD MTMSetNodeTemp0;
  /* Setzt Temperatur 0 am Knoten number */
extern DoubleFuncLong MTMGetNodeTemp1;
  /* Liefert Temperatur 1 am Knoten number */
extern VoidFuncLD MTMSetNodeTemp1;
  /* Setzt Temperatur 1 am Knoten number */

/* Weitere externe Objekte */


#endif

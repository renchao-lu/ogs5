/**************************************************************************
   ROCKFLOW - Modul: int_atm.h
 
   Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aTM zu den diesen Rechenkern benutzenden Modulen bereit.

   letzte Aenderung: RK 03/2003
 
**************************************************************************/

#ifndef int_atm_INC

#define int_atm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_atm.h"
#include "cel_atm.h"


/* Deklarationen */

/* Elementdaten-Schnittstelle */

/* Alte (einphasige) Zugriffe auf Matrizen */
extern VoidFuncLDX ATMSetElementAdvMatrix;
extern VoidFuncLDX ATMSetElementDispMatrix;
extern VoidFuncLDX ATMSetElementMassMatrix;

extern DoubleXFuncLong ATMGetElementAdvMatrix;
extern DoubleXFuncLong ATMGetElementFluidContentMatrix;
extern DoubleXFuncLong ATMGetElementDispMatrix;
extern DoubleXFuncLong ATMGetElementMassMatrix;

extern VoidFuncLong ATMDeleteElementAdvMatrix;
extern VoidFuncLong ATMDeleteElementFluidContentMatrix;
extern VoidFuncLong ATMDeleteElementDispMatrix;
extern VoidFuncLong ATMDeleteElementMassMatrix;

/* Neue (mehrphasige) Zugriffe auf Matrizen */
extern VoidFuncLIDX ATMSetElementAdvMatrixNew;
extern VoidFuncLIDX ATMSetElementFluidContentMatrixNew;
extern VoidFuncLIDX ATMSetElementDispMatrixNew;
extern VoidFuncLIDX ATMSetElementMassMatrixNew;

extern DoubleXFuncLI ATMGetElementAdvMatrixNew;
extern DoubleXFuncLI ATMGetElementFluidContentMatrixNew;
extern DoubleXFuncLI ATMGetElementDispMatrixNew;
extern DoubleXFuncLI ATMGetElementMassMatrixNew;

extern VoidFuncLI ATMDeleteElementAdvMatrixNew;
extern VoidFuncLI ATMDeleteElementFluidContentMatrixNew;
extern VoidFuncLI ATMDeleteElementDispMatrixNew;
extern VoidFuncLI ATMDeleteElementMassMatrixNew;



extern VoidFuncLD ATMSetElementRetardFac;
  /* Setzt den Retardationsfaktor des Elements number */
extern VoidFuncLD ATMSetElementDt;
extern VoidFuncLID ATMSetElementDtNew;
  /* Setzt Courant/Neumann-Zeitschritt des Elements number */
extern VoidFuncLD ATMSetElementPecletNum;
extern VoidFuncLID ATMSetElementPecletNumNew;
  /* Setzt Peclet-Zahl des Elements number */
extern VoidFuncLD ATMSetElementDDummy;
  /* Setzt Double-Wert des Elements number */

extern DoubleFuncLong ATMGetElementRetardFac;
  /* Liefert den Retardationsfaktor des Elements number */
extern DoubleFuncLong ATMGetElementDt;
extern DoubleFuncLI ATMGetElementDtNew;
  /* Liefert Courant-Zeitschritt des Elements number */
extern DoubleFuncLong ATMGetElementPecletNum;
extern DoubleFuncLI ATMGetElementPecletNumNew;
  /* Liefert Peclet-Zahl des Elements number */
/*extern DoubleFuncLong ATMGetElementVolume; */
  /* Liefert Volumen des Elements number */
extern DoubleFuncLong ATMGetElementDDummy;
  /* Liefert Double-Wert des Elements number */
extern DoubleFuncILDDDD ATMGetFluidContent;
  /* Liefert Fluidanteil am Volumen des Elements auf r-s-t-Koordinaten zurueck */
extern DoubleFuncILLI ATMGetNodeFluidContent;
  /* Liefert Fluidanteil am Volumen an einem Knoten zurueck */

/* Knotendaten-Schnittstelle */
extern DoubleFuncLIII ATMGetNodeConc;

extern DoubleFuncLong ATMGetNodePress;
  /* Liefert Druck des Knotens number */
extern VoidFuncLD ATMSetNodePress;
  /* Setzt Druck des Knotens number */
extern DoubleFuncLong ATMGetNodeConc0;
  /* Liefert Konzentration 0 am Knoten number */
extern VoidFuncLD ATMSetNodeConc0;
  /* Setzt Konzentration 0 am Knoten number */
extern DoubleFuncLong ATMGetNodeConc1;
  /* Liefert Konzentration 1 am Knoten number */
extern VoidFuncLD ATMSetNodeConc1;
  /* Setzt Konzentration 1 am Knoten number */
extern DoubleFuncILLI ATMGetNodeFlux;
  /* Liefert Fluss der aktellen Phase  */
extern DoubleFuncILLI ATMGetNodeSatu;
  /* Liefert Saettigung der aktellen Phase  */


/* Globaldaten-Schnittstelle */
extern IntFuncVoid ATMGetNumberOfFluidPhases;
  /* Liefert zurueck, wieviele Fluidphasen es gibt */
extern DoubleFuncVoid ATMGetGlobalTheta;
  /* Liefert theta */
extern VoidFuncDouble ATMSetGlobalTheta;
  /* Setzt theta */


/* Weitere externe Objekte */


#endif

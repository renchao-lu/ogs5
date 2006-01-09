/**************************************************************************/
/* ROCKFLOW - Modul: int_asm.h
                                                                          */
/* Aufgabe:
   Stellt die Funktionszeiger der nicht-Standard-Datenschnittstelle des
   Rechenkerns aSM zu den diesen Rechenkern benutzenden Modulen bereit.
                                                                          */
/**************************************************************************/

#ifndef int_asm_INC

#define int_asm_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "intrface.h"
#include "cgs_asm.h"
#include "cel_asm.h"
#include "cvel.h"
#include "pst_flow.h"

/* Deklarationen */

/* Elementdaten-Schnittstelle */
extern VoidFuncLD ASMSetElementDt;
  /* Setzt Courant/Neumann-Zeitschritt des Elements number */
extern VoidFuncLDX ASMSetElementConductanceMatrix;
  /* Setzt die conductance matrix des Elements number */
extern VoidFuncLDX ASMSetElementCapacitanceMatrix;
  /* Setzt die capacitance matrix des Elements number */
extern VoidFuncLDX ASMSetElementGravityVector;
  /* Setzt den gravity vector des Elements number */
extern VoidFuncLDX ASMSetElementStrainCouplingMatrixX;
extern VoidFuncLDX ASMSetElementStrainCouplingMatrixY;
extern VoidFuncLDX ASMSetElementCouplingMatrixPU;
  /* Zugriffsfunktionen fuer Kopplungsmatrizen (Konsolidierung) */
/*extern VoidFuncLD ASMSetElementVolume;*/
  /* Setzt Volumen des Elements number */
extern VoidFuncLD ASMSetElementXVelo;
  /* Setzt x-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD ASMSetElementYVelo;
  /* Setzt y-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD ASMSetElementZVelo;
  /* Setzt z-Komponente der Geschwindigkeit im Zentrum des Elements number */
extern VoidFuncLD ASMSetElementXMiddle;
  /* Setzt x-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD ASMSetElementYMiddle;
  /* Setzt y-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD ASMSetElementZMiddle;
  /* Setzt z-Komponente des Mittelpunktes des Elements number */
extern VoidFuncLD ASMSetElementXFlux;
  /* Setzt x-Komponente des Massenstroms des Elements number */
extern VoidFuncLD ASMSetElementYFlux;
  /* Setzt y-Komponente des Massenstroms des Elements number */
extern VoidFuncLD ASMSetElementZFlux;
  /* Setzt z-Komponente des Mittelstroms des Elements number */
extern VoidFuncLD ASMSetElementDDummy;
  /* Setzt Double-Wert des Elements number */
extern DoubleXFuncLong ASMGetElementConductanceMatrix;
extern DoubleXFuncLong ASMGetElementConductanceMatrixGroup;
  /* Liefert die conductance matrix des Elements number */
extern DoubleXFuncLong ASMGetElementCapacitanceMatrix;
extern DoubleXFuncLong ASMGetElementCapacitanceMatrixGroup;
  /* Liefert die capacitance matrix des Elements number */
extern DoubleXFuncLong ASMGetElementGravityVector;
extern DoubleXFuncLong ASMGetElementGravityVectorGroup;
  /* Liefert den gravity vector des Elements number */
extern DoubleXFuncLong ASMGetElementStrainCouplingMatrixX;
extern DoubleXFuncLong ASMGetElementStrainCouplingMatrixY;
extern DoubleXFuncLong ASMGetElementCouplingMatrixPU;
  /* Kopplungsmatrix - Verzerrungen */
extern DoubleFuncLong ASMGetElementDt;
  /* Liefert Courant-Zeitschritt des Elements number */
/*extern DoubleFuncLong ASMGetElementVolume;*/
  /* Liefert Volumen des Elements number */
extern DoubleFuncLong ASMGetElementDDummy;
  /* Liefert Double-Wert des Elements number */
extern VoidFuncLong ASMDeleteElementConductanceMatrix;
  /* Erdet den conductance matrix-Zeiger des Elements number */
extern VoidFuncLong ASMDeleteElementCapacitanceMatrix;
  /* Erdet den capacitance matrix-Zeiger des Elements number */
extern VoidFuncLong ASMDeleteElementGravityVector;
  /* Erdet den gravity vector-Zeiger des Elements number */

/* Knotendaten-Schnittstelle */
extern DoubleFuncLong ASMGetNodeHead0;
  /* Liefert Standrohrspiegelhoehe 0 des Knotens number */
extern VoidFuncLD ASMSetNodeHead0;
  /* Setzt Standrohrspiegelhoehe 0 des Knotens number */
extern DoubleFuncLong ASMGetNodePress0;
  /* Liefert Druck 0 am Knoten number */
extern VoidFuncLD ASMSetNodePress0;
  /* Setzt Druck 0 am Knoten number */
extern DoubleFuncLong ASMGetNodePress1;
  /* Liefert Druck 1 am Knoten number */
extern VoidFuncLD ASMSetNodePress1;
  /* Setzt Druck 1 am Knoten number */
extern DoubleFuncLong ASMGetNodeSource;
  /* Liefert Quelle des Knotens number */
extern VoidFuncLD ASMSetNodeSource;
  /* Setzt Quelle des Knotens number */
extern VoidFuncILID ASMSetNodeFlux;
  /* Setzt Flux am Knoten number */
extern DoubleFuncILLI ASMGetNodeFlux;
  /* Setzt Flux am Knoten number */

/* Globaldaten-Schnittstelle */
extern IntFuncVoid ASMGetFlowModelType;
  /* Liefert Fliessgesetz */
extern DoubleFuncVoid GetTheta_ASM;
  /* Liefert theta */
extern VoidFuncDouble SetTheta_ASM;
  /* Setzt theta */
/*extern DoubleFuncVoid GetRho_ASM;*/ /* UJ rf3217 */
  /* Liefert rho */
/*extern VoidFuncDouble SetRho_ASM;*/ /* UJ rf3217 */
  /* Setzt rho */
/*extern DoubleFuncVoid GetMu_ASM;*/ /* UJ rf3217*/
  /* Liefert mu */
/*extern VoidFuncDouble SetMu_ASM;*/ /* UJ rf3217*/
  /* Setzt mu */

/* Weitere externe Objekte */

/* Liefert Position des Druckvektors im LGS */

#endif


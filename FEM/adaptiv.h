/**************************************************************************/
/* ROCKFLOW - Modul: adaptiv.h
                                                                          */

/**************************************************************************/

#ifndef adaptiv_INC

#define adaptiv_INC
  /* Schutz gegen mehrfaches Einfuegen */

#include "gridadap.h"
#include "refine.h"
#include "masscont.h"
#include "indicatr.h"
#include "plains.h"
#include "edges.h"
#include "elements.h"

/**************************************************************************/
/* Datentypen */
extern IntFuncVoid Refine;  /* Funktionszeiger auf Verfeinerungsfunktion */


/**************************************************************************/
/* Deklarationen */
extern int FctAdaptation ( char *data, int found, FILE *f );
extern int FctAdaptationPressure ( char *data, int found, FILE *f);
extern int FctAdaptationComponent ( char *data, int found, FILE *f);
extern int FctAdaptationTemperature ( char *data, int found, FILE *f);
extern int FctAdaptationSorbedComponent ( char *data, int found, FILE *f);
extern int FctAdaptationSoluteComponent ( char *data, int found, FILE *f);
extern int FctAdaptationSaturation ( char *data, int found, FILE *f);
extern int FctAdaptationVolt ( char *data, int found, FILE *f);
extern int FctAdaptationQuantities(char *data, FILE * f, int type, char *keyword);
extern int WriteAdaptationQuantities(FILE * f, int type, char *keyword);


extern int CreateAdaptationData(void);
  /* Initialisiert Adaptive Variablen */
extern void InitGridAdaptation ( void );
extern void ConfigureGridAdaptation ( void );
  /* Zerstoert Adaptive Variablen */
extern void DestroyAdaptationData(void);

/* ah rfm */
extern void DestroyAllNewElementsGridAdaptation(void);
extern void ReinitGridAdaptation(void);

extern int AdaptGetMethodAdaptation ( void );
extern void AdaptSetMethodAdaptation (int ivalue);

extern int AdaptGetAdaptationTimestep ( void );

extern int AdaptGetMethodIrrNodes ( void );
extern void AdaptSetMethodIrrNodes (int ivalue);

extern int AdaptGetTimestepRefNumber ( void );
extern void AdaptSetTimestepRefNumber(int ivalue);

extern int AdaptGetMaxRefLevel ( void );
extern void AdaptSetMaxRefLevel (int ivalue);


extern int AdaptGetRefNeighbours2d ( void );
extern void AdaptSetRefNeighbours2d(int ivalue);
extern int AdaptGetRefNeighbours3d ( void );
extern void AdaptSetRefNeighbours3d(int ivalue);
extern long AdaptGetMaxNodes ( void );
extern int AdaptGetMethodNodeLimiter ( void );
extern int AdaptGetCurveNodeLimiter ( void );
extern int AdaptGetOutputAdaptation ( void );



extern int AdaptGetMethodInd (int number);
extern int AdaptGetRefQuantity ( int number );
extern int AdaptGetQuantityMaxRefLevel (int number, int typ);
void AdaptSetQuantityMaxRefLevel (int number, int typ, int ivalue);

extern int AdaptGetCoarseCorrect (int num_ref_quantity);
extern void AdaptSetCoarseCorrect (int num_ref_quantity, int ivalue);
extern long AdaptGetNumberInd ( int number );
extern int AdaptGetNumPhaseComponent ( int number );
extern int AdaptGetNumElectricField ( int number );


extern void AdaptSetRefNval0 (int number, int ref_nval0);
extern void AdaptSetRefNval1 (int number, int ref_nval1);
extern int AdaptGetRefNval0 (int number);
extern int AdaptGetRefNval1 (int number);

extern int AdaptGetRefInd (int num_ref_quantity, int num_ind, int typ, int AD);
extern void AdaptSetRefInd (int num_ref_quantity, int num_ind, int typ, int AD, int ivalue);

extern int AdaptGetCurveInd (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3);
extern double AdaptGetRefParam (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3);
extern void AdaptSetRefParam (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3, double param);
extern int AdaptGetRefTol (int num_ref_quantity, int num_ind);

extern long AdaptGetNumAdaptQuantities ( void );
extern long AdaptGetTotalNumInd ( void );

extern int AdaptGetRefIter ( void );
extern void AdaptSetRefIter ( int iteration);

extern int AdaptGetGridChange ( void );
extern void AdaptSetGridChange ( int grid_topo_change);

extern double AdaptGetIndValue (int typ, int AD, long element, int num_ref_quantity, int num_ind);

extern int AdaptGetActualRefLevel ( void );
extern void AdaptSetActualRefLevel ( int level );
extern long AdaptGetNumEleRefLevelX ( int level_x);
extern void AdaptSetNumEleRefLevelX ( int level_x, long num_elems );

extern void AdaptSetIndexPhasePress (int phase, int timelevel, int index);
extern int AdaptGetIndexPhasePress (int phase, int timelevel);
extern void AdaptSetIndexPhaseSatu (int phase, int timelevel, int index);
extern int AdaptGetIndexPhaseSatu (int phase, int timelevel);
extern void AdaptSetIndexTemp (int timelevel, int index);
extern int AdaptGetIndexTemp (int timelevel);
extern void AdaptSetIndexComponentConc (int component, int timelevel, int index);
extern int AdaptGetIndexComponentConc (int component, int timelevel);
extern void AdaptSetIndexComponentSorbedConc (int component, int timelevel, int index);
extern int AdaptGetIndexComponentSorbedConc (int component, int timelevel);
extern void AdaptSetIndexComponentSoluteConc (int component, int timelevel, int index);
extern int AdaptGetIndexComponentSoluteConc (int component, int timelevel);
extern int AdaptGetIndexElectricFieldVolt (int electric_field, int timelevel);
extern void AdaptSetIndexElectricFieldVolt (int electric_field, int timelevel, int index);

/* Elementdaten-Schnittstelle */

extern DoubleXFuncLong ADAPGetElementMassMatrix;
  /* Liefert den Massenmatrix-Zeiger eines Elements */
extern DoubleXFuncLong ADAPGetElementCapacitanceMatrix;
extern DoubleXFuncLong ADAPGetElementDispMatrix;
  /* Liefert den Dispersionsmatrix-Zeiger eines Elements */
extern DoubleXFuncLong ADAPGetElementConductanceMatrix;
extern DoubleFuncLong ADAPGetElementPecletNum;
  /* Liefert Peclet-Zahl eines Elements */
extern DoubleFuncLong ADAPGetElementDDummy;
  /* Liefert DDummy eines Elements */
extern VoidFuncLD ADAPSetElementDDummy;
  /* Setzt DDummy eines Elements */
extern DoubleXFuncLDX ADAPGetElementJacobi;
  /* Liefert die inverse Jakobi-Matrix und deren Determinatne
  im Zentrum des Elements number */
extern DoubleFuncLong ADAPGetElementRetardFac;
  /* Liefert den Retardationsfaktor des Elements number */


/* Rechenkern-Schnittstelle */

extern VoidFuncLong ADAPMakeMat;
  /* berechnet instationaere Matrizenanteile eines Elements */
extern VoidFuncLDXDX ADAPCalcTransMatrix;


extern void CreateAdaptationControlParameter( void );
extern void DestroyAdaptationControlParameter( void );


#endif








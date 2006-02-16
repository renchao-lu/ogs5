/* SB: help - file for rebuilding MTM2 kernel to PCS structure */


#ifndef help_MTM2_INC
#define help_MTM2_INC
/* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "nodes.h"

#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTHELP_MTM2


/* Header / Andere intern benutzte Module */
#include "cel_mtm2.h"
#include "int_mtm2.h"




//double MTM2GetFluidContent(int phase, long number, double r, double s, double t, double theta);




/*  */
//extern void *MTM2CreateELEMatricesPointer(void) ; //InitInternElementDataMTM2 ( void );
//extern void *MTM2DestroyELEMatricesPointer(void); //DestroyInternElementDataMTM2 ( void *data );




extern void MTM2SetElementDtNew ( long number, int phase, double zeitschritt );
extern void MTM2SetElementPecletNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementPecletNumNew ( long number, int phase, int component);
extern void MTM2SetElementCourantNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementCourantNumNew ( long number, int phase, int component);
extern void MTM2SetElementDamkohlerNumNew ( long number, int phase, int component, double value );
extern double MTM2GetElementDamkohlerNumNew ( long number, int phase, int component);












#endif

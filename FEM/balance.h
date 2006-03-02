/**************************************************************************/
/* ROCKFLOW - Modul: balance.h
 */
/* Aufgabe:
   Datenstrukturen und Definitionen fuer Bilanzobjekte
 */
/**************************************************************************/

#ifndef balance_INC

#define balance_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include <stdio.h>

#include "makros.h"


/*------------------------------------------------------------------------*/
/* Data types */
/*------------------------------------------------------------------------*/

typedef struct {
    int method;
    int number_coordinates;
    double *x;
    double *y;
    double *z;
    double epsilon;
} GEOMETRY_OBJECT;

typedef struct {
    int method;
    int number_geometry_objects;
    GEOMETRY_OBJECT *geometry_objects;
} BALANCESET;



extern int FctBalance(char *data, int found, FILE * f);
extern void BalanceOverAllGeometryObjects(void);
extern void BalanceInitAllObjects(char *dateiname);
extern void BalanceDestroyAllObjects(void);
extern VoidFuncIntFileX BalanceWriteModelDataHeader;
extern VoidFuncIntFileXLX BalanceWriteModelData;

#endif

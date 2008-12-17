/**************************************************************************/
/* ROCKFLOW - Modul: lag_atm.h
 */
/* Aufgabe:
 */
/**************************************************************************/

#ifndef lag_atm_INC

#define lag_atm_INC
#define LAGRANGE_USE_MATRIX_FOR_STATIONARY_FIELD

/* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */

/* Deklarationen */

/* Weitere externe Objekte */

/* Initialisiere Datenstrukturen */
extern void LAGInitialize();

/* Passe Groesse an */
extern void LAGResize();

/* Zerstoere Datenstrukturen */
extern void LAGDestroy();

/* Ermittle advektiven Transport */
extern void LAGCalcLagrangeAdvection(int phase, int c1, int c2, char *bc_name1, char *bc_name2, int modus);

#endif

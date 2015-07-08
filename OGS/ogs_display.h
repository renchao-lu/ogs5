/**************************************************************************/
/* ROCKFLOW - Modul: display.h
 */
/* Aufgabe:
   Enthaelt alle Funktionen fuer Standard Ein- und Ausgabe (Bildschirm,
   Tastatur)
 */
/**************************************************************************/

#ifndef ogs_display_INC
#define ogs_display_INC
/* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */

/* Deklarationen */
extern void DisplayStartMsg ( void );
/* Gibt Eroeffnungsbildschirm aus */
extern void DisplayEndMsg ( void );
/* Schreibt Laufzeitmeldung auf Standardausgabe */
#endif

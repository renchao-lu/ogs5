/**************************************************************************
 ROCKFLOW - Modul: timer.h

 Aufgabe:
   Funktionen zur Laufzeitermittlung im Testbetrieb.
**************************************************************************/

#ifndef timer_INC

/* Schutz gegen mehrfaches Einfuegen */
#define timer_INC

/* Andere oeffentlich benutzte Module */
#include <time.h>

/* Deklarationen */

/* Setzt Timer auf 0, aber startet noch nicht */
extern void TInitTimer(int speicher);

/* Setzt Timer auf 0 und startet ihn */
extern void TStartTimer(int speicher);

/* Liefert Laufzeit des gew. Timers seit TStartTimer als Long */
extern long TGetTimer(int speicher);

/* Liefert Laufzeit des gew. Timers seit TStartTimer als Double */
extern double TGetTimerDouble(int speicher);

/*  Haelt Timer an */
extern void TStopTimer(int speicher);

/*  Laesst Timer weiterlaufen */
extern void TRestartTimer(int speicher);

/* Liefert Laufzeit/CPU-Zeit in "Ticks" */
extern long TGetTime(void);

/* Liefert Laenge eines "Ticks" */
extern long TGetTicksPerSecond(void);

/* Zerstoert alle Zeitspeicher */
extern void TDestroyTimers(void);


/* Weitere externe Objekte */

/* Interface fuer AMG-Loeser */
extern void ctime_(float *time);


#endif

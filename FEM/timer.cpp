/**************************************************************************
   ROCKFLOW - Modul: timer.c

   Aufgabe:
   Funktionen zur Laufzeitermittlung im Testbetrieb.
   Es werden beliebig viele Zeitspeicher bereitgestellt.


   Programmaenderungen:
   07/1994     MSR        Erste Version
   6/1997      C.Thorenz  Komplett neue zweite Version
   1/1999      C.Thorenz  Dritte Version: CPU-Zeit auf POSIX-Rechner
   09/1999     AH         Funktionen: TGetTime und TGetTicksPerSecond global.
   11/1999     C.Thorenz  Beliebige Anzahl Zeitspeicher
**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "timer.h"

/* Auf POSIX-Rechern ist exaktere Zeitmessung vorhanden */
#ifdef _POSIX_SOURCE
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#endif

/* Zeitspeicher */
static int max_zeitspeicher=-1;
static long *zeit=NULL;
static int *running=NULL;

/*************************************************************************
 ROCKFLOW - Funktion: TInitTimer

 Aufgabe:
   Setzt Zeitspeicher auf 0 aber startet den Timer noch nicht. Die
   Funktion muss vor TGetTimer aufgerufen werden ! Der Timer0 ist
   bei ROCKFLOW fuer die Gesamtlaufzeit reserviert.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher: Nummer (0..9) des Zeitspeichers

 Ergebnis:
   - void -

 Programmaenderungen:
   07/1994     MSR        Erste Version
   6/1997      C.Thorenz  Komplett neue zweite Version
   1/1999      C.Thorenz  Dritte Version: CPU-Zeit auf POSIX-Rechner
**************************************************************************/
void TInitTimer(int speicher)
{
    /* Ggf. Liste der Speicher erweitern */
    if (speicher > max_zeitspeicher) {
       zeit = (long *) Realloc(zeit, sizeof(long)*(speicher+1));
       running = (int *) Realloc(running, sizeof(int)*(speicher+1));
       max_zeitspeicher = speicher;
    }

    /* Der Timer wird "genullt", aber nicht gestartet. */
    zeit[speicher] = 0;
    running[speicher] = 0;
}

/*************************************************************************
 ROCKFLOW - Funktion: TStartTimer

 Aufgabe:
   Setzt Timer auf 0 und startet den Timer neu. Der Timer0 ist bei
   ROCKFLOW fuer die Gesamtlaufzeit reserviert.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher: Nummer (0..x) des Zeitspeichers

 Ergebnis:
   - void -

 Programmaenderungen:
   07/1994     MSR        Erste Version
   6/1997      C.Thorenz  Komplett neue zweite Version
   1/1999      C.Thorenz  Dritte Version: CPU-Zeit auf POSIX-Rechner
**************************************************************************/
void TStartTimer(int speicher)
{
    /* Ggf. Liste der Speicher erweitern */
    if (speicher > max_zeitspeicher) {
       zeit = (long *) Realloc(zeit, sizeof(long)*(speicher+1));
       running = (int *) Realloc(running, sizeof(int)*(speicher+1));
       max_zeitspeicher = speicher;
    }

    /* Der Timer wird "genullt" indem die aktuelle Zeit im Speicher abgelegt
       wird. */
    zeit[speicher] = TGetTime();
    running[speicher] = 1;
}


/**************************************************************************
 ROCKFLOW - Funktion: TGetTimerDouble

 Aufgabe:
   Liefert Zeitdifferenz seit dem letzten Aufruf von TStartTimer
   als Double mit der Aufloesung, die zur Verfuegung steht.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher       : Nummer (0..9) des Zeitspeichers
   R double time_gone_by: Laufzeit

 Ergebnis:
   (CPU-)Zeitdifferenz seit dem letzten Aufruf von TStartTimer

 Programmaenderungen:
   07/1994     MSR        Erste Version
   6/1997      C.Thorenz  Komplett neue zweite Version
   1/1999      C.Thorenz  Dritte Version: CPU-Zeit auf POSIX-Rechner
**************************************************************************/
double TGetTimerDouble(int speicher)
{
    double time_gone_by;

    if (!running[speicher]) {
        /* Der Timer war angehalten */
        time_gone_by = (double) zeit[speicher] / (double) TGetTicksPerSecond();
    } else {
        /* Der Timer lief */
        time_gone_by = (double) (TGetTime() - zeit[speicher]) / (double) TGetTicksPerSecond();
    }

    return time_gone_by;
}

/**************************************************************************
 ROCKFLOW - Funktion: TGetTimer

 Aufgabe:
   Liefert Zeitdifferenz seit dem letzten Aufruf von TStartTimer
   als long mit Sekunden-Aufloesung. Wenn eine hoehere Aufloesung
   zur Verfuegung steht, wird diese intern genutzt.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher: Nummer (0..9) des Zeitspeichers
   R long time_gone_by: Laufzeit

 Ergebnis:
   (CPU-)Zeitdifferenz seit dem letzten Aufruf von TStartTimer

 Programmaenderungen:
   07/1994     MSR        Erste Version
   6/1997      C.Thorenz  Komplett neue zweite Version
   1/1999      C.Thorenz  Dritte Version: CPU-Zeit auf POSIX-Rechner
**************************************************************************/
long TGetTimer(int speicher)
{
    long time_gone_by;

    if (!running[speicher]) {
        /* Der Timer war angehalten */
        time_gone_by = (long) (zeit[speicher] / TGetTicksPerSecond());
    } else {
        /* Der Timer lief */
        time_gone_by = (long) ((TGetTime() - zeit[speicher]) / TGetTicksPerSecond());
    }

    return time_gone_by;
}


/**************************************************************************
 ROCKFLOW - Funktion: TStopTimer

 Aufgabe:
   Haelt den zug. Timer an.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher: Nummer (0..9) des Zeitspeichers

 Ergebnis:

 Programmaenderungen:
   6/1997   C.Thorenz        Erste Version
**************************************************************************/
void TStopTimer(int speicher)
{
    /* Im Speicher wird die bisher verstrichene Zeit abgelegt.
       Wird nur bei laufendem Timer ausgefuehrt. */

    if (running[speicher]) {
        zeit[speicher] = TGetTime() - zeit[speicher];
        running[speicher] = 0;
    }
}

/**************************************************************************
 ROCKFLOW - Funktion: TRestartTimer

 Aufgabe:
   Laesst den zugehoerigen Timer wieder weiterlaufen.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int speicher: Nummer (0..9) des Zeitspeichers

 Ergebnis:

 Programmaenderungen:
   6/1997   C.Thorenz        Erste Version

**************************************************************************/
void TRestartTimer(int speicher)
{
    /* Im Speicher liegt die bisher vom Timer gezaehlte Zeit. Mit der
       aktuellen Zeit wird die Startzeit errechnet.
       Wird nur bei angehaltenem Timer ausgefuehrt. */

    if (!running[speicher])

 {
        zeit[speicher] = TGetTime() - zeit[speicher];
        running[speicher] = 1;
        }
}


/**************************************************************************
 ROCKFLOW - Funktion: TGetTime

 Aufgabe:
   Probiert eine "Zeit" zu erhalten. In ANSI-C ist das die
   absolute Uhrzeit in Sekunden, bei POSIX-Systemen (UNIX) die
   echte Prozessorzeit in Ticks.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R long zeit: Zeit

 Ergebnis:

 Programmaenderungen:
   1/1999   C.Thorenz        Erste Version

**************************************************************************/
long TGetTime(void)
{

    long runtime;

#ifdef _POSIX_SOURCE
    runtime = clock();
#else
    runtime = (long)time(NULL);
#endif


    return runtime;
}

/**************************************************************************
 ROCKFLOW - Funktion: TGetTicksPerSecond

 Aufgabe:
   Liefert die Aufloesung der mit TGetTime erhaltenen
   "ticks" in 1/Sekunde.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R long ticks: Ticks pro Sekunde

 Ergebnis:

 Programmaenderungen:
   1/1999   C.Thorenz        Erste Version

**************************************************************************/
long TGetTicksPerSecond(void)
{

    long TicksPerSecond;

    TicksPerSecond = 1;

#ifdef _POSIX_SOURCE
    TicksPerSecond = CLOCKS_PER_SEC;
#endif

    return TicksPerSecond;
}

/*************************************************************************
 ROCKFLOW - Funktion: TDestroyTimers

 Aufgabe:
   Zerstoert alle Zeitspeicher

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:
   10/1999      C.Thorenz  Erste Version
**************************************************************************/
void TDestroyTimers(void) {
    /* Speicherfreigaben */
    zeit = (long *) Free(zeit);
    running = (int *) Free(running);
}

/*************************************************************************
 ROCKFLOW - Funktion: ctime_

 Aufgabe:
   Interface zu Timing des AMG-Loesers
   R double* time: CPU-Zeit

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:
   10/2001      C.Thorenz  Erste Version
**************************************************************************/
void ctime_(float *time)
{
    *time = (float)TGetTime()/(float)TGetTicksPerSecond();
} 


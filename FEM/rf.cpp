/**************************************************************************/
/* ROCKFLOW - Modul: rf.c
                                                                          */
/* Aufgabe:
   ROCKFLOW-FEM - Hauptprogramm
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   06/1998     AH         Konfigurationsdatei
   08/1999     OK         RF-FEM Applikation
   10/1999     AH         Systemzeit

   last modified: OK 14.12.1999
                                                                          */
/**************************************************************************/

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
#include <mpi.h>
#include "par_ddc.h"
#endif

#include "stdafx.h" /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
#define TEST
/* Benutzte Module */
#include "break.h"
#include "timer.h"
#include "rf_apl.h"
#include "geo_strings.h"
/* Deklarationen */
int main ( int argc, char *argv[] );
void ShowSwitches ( void );
string FileName; //WW

/* Definitionen */

/**************************************************************************/
/* ROCKFLOW - Funktion: main
                                                                          */
/* Aufgabe:
   Hauptprogramm
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int argc: Anzahl der Kommandozeilenparameter (incl. Programmname)
   E char *argv[]: Zeiger auf Feld der argc Kommandozeilenparameter
                                                                          */
/* Ergebnis:
   Fehlerfreie Bearbeitung: Exit-Code 0
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   08/1999     OK         RF-FEM Applikation
                                                                          */
/**************************************************************************/
int main ( int argc, char *argv[] )
{
  char *dateiname;
/*---------- MPI Initialization ----------------------------------*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
      MPI_Init(&argc,&argv);
      MPI_Comm_size(MPI_COMM_WORLD,&size);
      MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
      cout << "After MPI_Init myrank = " << myrank << '\n';
      time_ele_paral = 0.0;
#endif
/*---------- MPI Initialization ----------------------------------*/
/*========================================================================*/
/* Kommunikation mit Betriebssystem */
  /* Ctrl-C ausschalten */
  NoBreak();
  /* Timer fuer Gesamtzeit starten */
#ifdef TESTTIME
    TStartTimer(0);
#endif
  /* Intro ausgeben */
  DisplayStartMsg();
  /* Speicherverwaltung initialisieren */
  if (!InitMemoryTest()) {
    DisplayErrorMsg("Fehler: Speicherprotokoll kann nicht erstellt werden!");
    DisplayErrorMsg("        Programm vorzeitig beendet!");
    return 0;
  }
  /* Eingabe-Dateinamen ermitteln */
  if (argc>1) {
      dateiname = (char *) Malloc((int)strlen(argv[1])+1);
//WW      dateiname = StrDown(strcpy(dateiname,argv[1]));
      dateiname = strcpy(dateiname,argv[1]);
      DisplayMsgLn(dateiname);
  }
  else {
//WW      dateiname = StrDown(ReadString());
      dateiname = ReadString();
  }
  DisplayMsgLn("");
  DisplayMsgLn("");
  FileName = dateiname;
  /* Schalterstellungen zeigen */
#ifdef SWITCHES
    ShowSwitches();
#endif
  /* Konfiguration lesen */
#ifdef ENABLE_ADT
    ReadRFConfigFile(dateiname);
#endif
/*========================================================================*/
/* FEM-Applikation */
  /* Allgemeine FEM-Datenstrukturen anlegen */
  CreateObjectLists();
  /* Systemzeit fuer Rockflow setzen */
//OK  SetSystemTime("RF-MAIN","ROCKFLOW","ROCKFLOW: Total time",&rockflow_id_timer);
//OK  RunSystemTime("RF-MAIN");
  /* Ctrl-C abfangen und interpretieren */
  SaveBreak();
  /* FEM-Applikation: ROCKFLOW */
  RF_FEM(dateiname);
#ifdef TEST
  cout << "*********************************************" << endl;
  cout << "End of simulation" << endl;
#endif
  /* Ctrl-C erzeugt keinen Abbruch mehr */
  NoBreak();
  /* Systemzeit fuer Rockflow anhalten */
//OK  StopSystemTime("RF-MAIN");
  /* Systemzeit fuer Rockflow-Gruppe ausgeben */
//OK  StatisticsSystemTime("ROCKFLOW");
  /* Allgemeine FEM-Datenstrukturen freigeben */
  DestroyObjectLists();
#ifdef TEST
  cout << "Data destruction" << endl;
#endif
/*========================================================================*/
/* Kommunikation mit Betriebssystem */
  /* Speicher frei */
  dateiname = (char *)Free(dateiname);
  /* Speichertest beenden */
  StopMemoryTest();
  /* Laufzeit ausgeben */
#ifdef TESTTIME
  cout << "Simulation time: " << TGetTimer(0) << "s" << endl;
#endif
  /* Abspann ausgeben */
  /* Ctrl-C wieder normal */
  StandardBreak();
/*--------- MPI Finalize ------------------*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)

      MPI_Finalize();

#endif
/*--------- MPI Finalize ------------------*/
  return 0;
}


// PCH
// Temp main for executing from GUI
int mainPCH ( int argc, char *argv[] )
{
  char *dateiname;

/*========================================================================*/
/* Kommunikation mit Betriebssystem */

  /* Ctrl-C ausschalten */
  NoBreak();

  /* Timer fuer Gesamtzeit starten */
#ifdef TESTTIME
    TStartTimer(0);
#endif

  /* Intro ausgeben */
  DisplayStartMsg();

  /* Speicherverwaltung initialisieren */
  if (!InitMemoryTest()) {
    DisplayErrorMsg("Fehler: Speicherprotokoll kann nicht erstellt werden!");
    DisplayErrorMsg("        Programm vorzeitig beendet!");
    return 0;
  }

  /* Eingabe-Dateinamen ermitteln */
  if (argc>1) {
      dateiname = (char *) Malloc((int)strlen(argv[1])+1);
      dateiname = StrDown(strcpy(dateiname,argv[1]));
      DisplayMsgLn(dateiname);
  }
  else {
      dateiname = StrDown(ReadString());
  }

  DisplayMsgLn("");
  DisplayMsgLn("");
  FileName = dateiname;

  /* Schalterstellungen zeigen */
#ifdef SWITCHES
    //ShowSwitches();
#endif

  /* Konfiguration lesen */
#ifdef ENABLE_ADT
    ReadRFConfigFile(dateiname);
#endif


/*========================================================================*/
/* FEM-Applikation */

  /* Allgemeine FEM-Datenstrukturen anlegen */
  CreateObjectLists();
#ifdef TEST
    DisplayMsgLn("main: Allgemeine FEM-Datenstrukturen anlegen");
#endif

  /* Systemzeit fuer Rockflow setzen */
//OK  SetSystemTime("RF-MAIN","ROCKFLOW","ROCKFLOW: Total time",&rockflow_id_timer);
//OK  RunSystemTime("RF-MAIN");

  /* Ctrl-C abfangen und interpretieren */
  SaveBreak();

  /* FEM-Applikation: ROCKFLOW */
  RF_FEM(dateiname);
#ifdef TEST
  cout << "*********************************************" << endl;
  DisplayMsgLn("main: FEM-Applikation - ROCKFLOW- ausgefuehrt");
#endif

  /* Ctrl-C erzeugt keinen Abbruch mehr */
  NoBreak();
  /* Systemzeit fuer Rockflow anhalten */
//OK  StopSystemTime("RF-MAIN");
  /* Systemzeit fuer Rockflow-Gruppe ausgeben */
//OK  StatisticsSystemTime("ROCKFLOW");
  /* Allgemeine FEM-Datenstrukturen freigeben */
  DestroyObjectLists();
#ifdef TEST
    DisplayMsgLn("main: Allgemeine FEM-Datenstrukturen freigeben");
#endif


/*========================================================================*/
/* Kommunikation mit Betriebssystem */

  /* Speicher frei */
  dateiname = (char *)Free(dateiname);

  /* Speichertest beenden */
  StopMemoryTest();

  /* Laufzeit ausgeben */
#ifdef TESTTIME
    DisplayMsgLn("");
    DisplayMsg("Gesamtzeitaufwand: ");
    DisplayLong(TGetTimer(0));
    DisplayMsg("s");
    DisplayMsgLn("");
#endif

  /* Abspann ausgeben */

  /* Ctrl-C wieder normal */
  StandardBreak();

  return 0;
}
/**************************************************************************/
/* ROCKFLOW - Funktion: ShowSwitches
                                                                          */
/* Aufgabe:
   Zeigt globale Schalterstellungen (aus makros.h)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     hh        Erste Version
                                                                          */
/**************************************************************************/
void ShowSwitches ( void )
{
  DisplayMsgLn("");
  DisplayMsgLn("");
  DisplayMsgLn("Schalter:");
#ifdef TESTTIME
     DisplayMsgLn("  - TESTTIME");
     DisplayMsgLn("    Laufzeitausgaben in Sekunden (!)");
#endif
#ifdef EXT_RFD_MIN
    DisplayMsgLn("  - EXT_RFD_MIN");
    DisplayMsgLn("    Eingabeprotokoll f. gef. Schluesselworte dokumentieren");
#endif
#ifdef EXT_RFD
    DisplayMsgLn("  - EXT_RFD");
    DisplayMsgLn("    Eingabeprotokoll ausfuehrlich dokumentieren");
#endif
#ifdef ERROR_CONTROL
     DisplayMsgLn("  - ERROR_CONTROL");
     DisplayMsgLn("    Erweiterte Fehlerueberpruefung");
#endif
#ifdef REF_STATIC
     DisplayMsgLn("  - REF_STATIC");
     DisplayMsgLn("    Statische Variablen in Refine-Rekursionen");
#endif
#ifdef NULLE_ERGEBNIS
     DisplayMsgLn("  - NULLE_ERGEBNIS");
     DisplayMsgLn("    Startvektor fuer CG-Loeser ist Nullvektor");
#endif
#ifdef RELATIVE_EPS
     DisplayMsgLn("  - RELATIVE_EPS");
     DisplayMsgLn("    cg_eps gibt relative Schranke an (bei CG-Loesern)");
#endif
#ifdef MEMORY_TEST
     DisplayMsgLn("  - MEMORY_TEST");
     DisplayMsgLn("    Speicherprotokoll in Datei 'memtest.log'");
#endif
#ifdef MEMORY_TEST_IN_TIME
     DisplayMsgLn("  - MEMORY_TEST_IN_TIME");
     DisplayMsgLn("    Es wird waehrend der Laufzeit eine Speicherbilanz erstellt");
#endif
#ifdef MEMORY_SHOW_USAGE
     DisplayMsgLn("  - MEMORY_SHOW_USAGE");
     DisplayMsgLn("    Jedes Malloc/Realloc/Free wird aufgefuehrt");
#endif
#ifdef MEMORY_FLUSH
     DisplayMsgLn("  - MEMORY_FLUSH ");
     DisplayMsgLn("    immer fflush beim Schreiben von 'memtest.log'");
#endif
#ifdef MEMORY_REALLOC
     DisplayMsgLn("  - MEMORY_REALLOC");
     DisplayMsgLn("    realloc ersetzt durch malloc/free");
#endif
#ifdef MEMORY_STR
     DisplayMsgLn("  - MEMORY_STR");
     DisplayMsgLn("    zusaetzliche Aufrufstellenangabe bei Malloc/Realloc/Free'");
#endif
#ifdef DOSGNUC
     DisplayMsgLn("  - DOSGNUC");
     DisplayMsgLn("    wegen Realloc-Problemen bei GNU-C unter DOS");
#endif
  /* evtl. auch noch die benutzten Loeser-Normen ausgeben */
  DisplayMsgLn("----- ");
  DisplayMsgLn("");
  DisplayMsgLn("");
}


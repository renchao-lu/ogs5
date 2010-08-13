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

/**
 * the preprocessor directive RFW_FRACTURE is only useable until version 4.11 of OGS
 * */

#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL) || defined(USE_MPI_GEMS)
#include <mpi.h>
#include "par_ddc.h"
#endif
#ifdef LIS
#include <omp.h>
#include "lis.h"
#endif

#include<iostream>
/* Preprozessor-Definitionen */
#include "makros.h"
#define TEST
/* Benutzte Module */
#include "break.h"
#include "timer.h"
//16.12.2008. WW #include "rf_apl.h"
#include "files0.h"
#include "FileTools.h"
#ifdef SUPERCOMPUTER
// kg44 test for buffered outputh
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#endif
#ifdef PROBLEM_CLASS //WW
#include "problem.h"
#ifndef MFC //16.12.2008. WW
Problem *aproblem = NULL;
#endif
#endif
/* Deklarationen */
int main ( int argc, char *argv[] );
void ShowSwitches ( void );
// LB,string FileName; //WW
// LB,string FilePath; //23.02.2009. WW
// ------  12.09.2007 WW:
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL) || defined(USE_MPI_GEMS)
double elapsed_time_mpi;
// ------
#endif
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
  /* parse command line arguments */
  std::string anArg;
  std::string modelRoot;
  for( int i = 1; i < argc; i++ ) {
    anArg = std::string( argv[i] );
    if( anArg == "--help" || anArg == "-h")
      {
	std::cout << "Usage: ogs [MODEL_ROOT] [OPTIONS]\n"
		  << "Where OPTIONS are:\n"
		  << "  -h [--help]       print this message and exit\n"
		  << "  -b [--build-info] print build info and exit\n"
		  << "  --version         print ogs version and exit" << endl;
	continue;
      }
    if( anArg == "--build-info" || anArg == "-b" )
      {
	std::cout << "ogs version: " << OGS_VERSION << endl
		  << "ogs date: " << OGS_DATE << endl
		  << "cmake command line arguments: " << CMAKE_CMD_ARGS << endl
		  << "git commit info: " << GIT_COMMIT_INFO << endl
		  << "subversion info: " << SVN_REVISION << endl;
	continue;
      }
    if( anArg == "--version" )
      {
	std::cout << OGS_VERSION << endl;
	exit(0);
      }
    if( anArg == "--model-root" || anArg == "-m" )
      {
	modelRoot = std::string( argv[++i] );
	continue;
      }
    // anything left over must be the model root, unless already found
    if ( modelRoot == "" ){ modelRoot = std::string( argv[i] ); }
  } // end of parse argc loop

  if( argc > 1 and modelRoot == "" ) // non-interactive mode and no model given
    exit(0);                         // e.g. just wanted the build info

  char *dateiname;
#ifdef SUPERCOMPUTER
// *********************************************************************
// buffered output ... important for performance on cray
// (unbuffered output is limited to 10 bytes per second)
// georg.kosakowski@psi.ch 11.10.2007

  char buf[1024*1024];
  int bsize;

	 bsize = 1024*1024; // question: what happens if buffer is full?
                         // according to documentation the buffer is flushed when full.
                         // If we have a lot of output, increasing buffer is usefull.
     if(bsize > 0) {
//        bufstd = malloc(bsize);
        setvbuf(stdout, buf, _IOFBF, bsize);
     }
//**********************************************************************
#endif
/*---------- MPI Initialization ----------------------------------*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL) || defined(USE_MPI_GEMS)
      printf("Before MPI_Init\n");
      MPI_Init(&argc,&argv);
      MPI_Barrier (MPI_COMM_WORLD); // 12.09.2007 WW
      elapsed_time_mpi = -MPI_Wtime(); // 12.09.2007 WW
      MPI_Comm_size(MPI_COMM_WORLD,&mysize);
      MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
      cout << "After MPI_Init myrank = " << myrank << '\n';
      time_ele_paral = 0.0;
#endif
/*---------- MPI Initialization ----------------------------------*/
/*---------- LIS solver -----------------------------------------*/
#ifdef LIS
  //int argc=0;
  //char** argv = NULL;
  // Initialization of the lis solver.
//  lis_initialize(&argc, &argv);	PCH: Undoing NW modification for compilation OGS-5
#endif
/*========================================================================*/
/* Kommunikation mit Betriebssystem */
  /* Ctrl-C ausschalten */
  NoBreak();
  /* Timer fuer Gesamtzeit starten */
#ifdef TESTTIME
    TStartTimer(0);
#endif
  /* Intro ausgeben */
#if defined(USE_MPI) //WW
   if(myrank==0)
#endif
  DisplayStartMsg();
  /* Speicherverwaltung initialisieren */
  if (!InitMemoryTest()) {
    DisplayErrorMsg("Fehler: Speicherprotokoll kann nicht erstellt werden!");
    DisplayErrorMsg("        Programm vorzeitig beendet!");
    return 1;	// LB changed from 0 to 1 because 0 is indicating success
  }
  if( argc == 1 )                     // interactive mode
    {
      dateiname = ReadString();
    }
  else                               // non-interactive mode
    {
      if ( argc == 2 )               // a model root was supplied 
	{
	  dateiname = (char *) Malloc((int)strlen(argv[1])+1);
	  dateiname = strcpy(dateiname,argv[1]);
	}
      else                          // several args supplied
	if( modelRoot != "")
	  {
	    dateiname = (char *) Malloc( (int) modelRoot.size() + 1 );
	    dateiname = strcpy( dateiname, modelRoot.c_str() );
	  }
      DisplayMsgLn(dateiname);
  }
  //WW  DisplayMsgLn("");
  //WW  DisplayMsgLn("");
  // ----------23.02.2009. WW-----------------

  // LB Check if file exists
  std::string tmpFilename = dateiname;
  tmpFilename.append(".pcs");
  if(!IsFileExisting(tmpFilename))
  {
	  cout << " Error: Cannot find file " << dateiname << endl;
	  return 1;
  }

  FileName = dateiname;
  basic_string <char>::size_type indexChWin, indexChLinux;
  indexChWin = indexChLinux = 0;
  indexChWin = FileName.find_last_of('\\');
  indexChLinux = FileName.find_last_of('/');
  //
  if(indexChWin!=string::npos)
     FilePath = FileName.substr(0,indexChWin)+"\\";
  else if(indexChLinux!=string::npos)
     FilePath = FileName.substr(0,indexChLinux)+"/";
  // ---------------------------WW
#ifdef PROBLEM_CLASS    //12.08.2008. WW
  aproblem = new Problem(dateiname);
  aproblem->Euler_TimeDiscretize();
  delete aproblem;
  aproblem = NULL;
#else // of define PROBLEM_CLASS
  /* Schalterstellungen zeigen */
  //#ifdef SWITCHES
  //    ShowSwitches();
  //#endif
  /* Konfiguration lesen */
#ifdef ENABLE_ADT
    ReadRFConfigFile(dateiname);
#endif
/*========================================================================*/
/* FEM-Applikation */
  /* Allgemeine FEM-Datenstrukturen anlegen */
//WW  CreateObjectLists();
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
#endif //of define PROBLEM_CLASS
#ifdef TESTTIME
  cout << "Simulation time: " << TGetTimer(0) << "s" << endl;
#endif
  /* Abspann ausgeben */
  /* Ctrl-C wieder normal */
  StandardBreak();
/*--------- MPI Finalize ------------------*/
#if defined(USE_MPI) || defined(USE_MPI_PARPROC) || defined(USE_MPI_REGSOIL)
   elapsed_time_mpi += MPI_Wtime(); // 12.09.2007 WW
   cout<<"\n *** Total CPU time of parallel modeling: "<< elapsed_time_mpi<<endl; //WW
   // Count CPU time of post time loop WW
   MPI_Finalize();
#endif
/*--------- MPI Finalize ------------------*/
/*--------- LIS Finalize ------------------*/
#ifdef LIS
//  lis_finalize();	//PCH: Undoing NW modification for compilation OGS-5
#endif
/*--------- LIS Finalize ------------------*/

  free(dateiname);
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
//WW      dateiname = StrDown(strcpy(dateiname,argv[1]));
      dateiname = strcpy(dateiname,argv[1]);
      DisplayMsgLn(dateiname);
  }
  else {
//WW      dateiname = StrDown(ReadString());
      dateiname = ReadString();
  }
  //WW  DisplayMsgLn("");
  //WW  DisplayMsgLn("");
  FileName = dateiname;
#ifdef PROBLEM_CLASS    //12.08.2008. WW
  aproblem = new Problem(dateiname);
  aproblem->Euler_TimeDiscretize();
  delete aproblem;
  aproblem = NULL;
#else // of define PROBLEM_CLASS
  /* Schalterstellungen zeigen */
  //#ifdef SWITCHES
  //    ShowSwitches();
  //#endif
  /* Konfiguration lesen */
#ifdef ENABLE_ADT
    ReadRFConfigFile(dateiname);
#endif
/*========================================================================*/
/* FEM-Applikation */
  /* Allgemeine FEM-Datenstrukturen anlegen */
//WW  CreateObjectLists();
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
#endif //of define PROBLEM_CLASS
#ifdef TESTTIME
  cout << "Simulation time: " << TGetTimer(0) << "s" << endl;
#endif

  /* Abspann ausgeben */

  /* Ctrl-C wieder normal */
  StandardBreak();

  return 0;
}
#ifndef PROBLEM_CLASS    //12.08.2008. WW
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

#endif


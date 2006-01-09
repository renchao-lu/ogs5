/**************************************************************************/
/* ROCKFLOW - Modul: files.h
                                                                          */
/* Aufgabe:
   Enthaelt alle Datei- Ein- und Ausgabefunktionen, aufgeteilt in
   files0.c, files1.c und files2.c. Die Hauptfunktionen sind in
   files0.c.
                                                                          */
/**************************************************************************/

#ifndef files_INC

#define files_INC
  /* Schutz gegen mehrfaches Einfuegen */

#define noTESTFILES

#define FPD(a)    FilePrintDoubleWB(f,a)
#define FPI(a)    FilePrintIntWB(f,a)
#define FPL(a)    FilePrintLongWB(f,a)
#define FPS(a)    FilePrintString(f,a)
#define LF        LineFeed(f)


/* Andere oeffentlich benutzte Module */
#include <stdio.h>
// C++ STL
#include <string>
using namespace std;

#include "prototyp.h"

/* Deklarationen zu files0.c */
extern char *file_name;
extern char *crdat; //SB_MX
extern int ReadData ( char *dateiname );
  /* Liest alle Eingabedateien und erzeugt Protokolldatei, 0 bei Fehler */
extern void SelectData ( int lsr );
  /* zu lesende bzw. zu schreibende Knoten- und Elementdaten */
extern char *GetPathRFDFile ();
  /* Holt Pfadnamen sofern vorhanden */
extern FILE *OpenMsgFile ();
extern void CloseMsgFile (FILE *f);
  /* Schaltet auf *.msg-Datei oder stdout um */

extern void CreateFileData ( char *dateiname );  /* ah rfm */
extern void ConfigFileData ( char *dateiname );  
extern void DestroyFileData ( void );
extern void DestroyFiles(void);
  /* Vernichtet zuvor angelegte dynamische Daten */

extern IntFuncFileX CGC1_Model;
extern IntFuncFileX CGC2_Model;
  /* Modellspezifische Prueffunktionen */

extern int WriteNodeCurves ( void );
  /* Schreibt Durchbruchskurven fuer Knoten */

typedef struct {
    int idx_program;  /* Feldindex bei den Datenstrukturen */
    int idx_file;  /* Feldindex in der Datei */
    int transfer;  /* wieviele Werte sollen geschrieben werden */
    char *name;
    char *einheit;
} DatenFeld;

extern DatenFeld *datafield_n;  /* zu bearbeitende Datenfelder Knoten */
extern DatenFeld *datafield_e;  /* zu bearbeitende Datenfelder Elemente */
extern int danz_n, danz_e;  /* Feldgroessen */
extern int danz_sum_n, danz_sum_e;  /* Gesamtzahl der Daten */
extern int rfi_filetype; /* Spezifiziert die ROCKFLOW-Version, die zum Lesen
                            des RFI-Files benoetigt wird. */

/******************************************************************************/
/* Deklarationen zu files1.c */
extern int ReadTextInputFile ( char *indat, FILE *prot );
  /* Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei,
     0 bei Fehler */
extern int ReadMGTextInputFile ( char *indat, FILE *prot );
  /* Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei,
     0 bei Fehler */

  /* Inverse Modellierung */
extern int SetTextInputFile(char *db_name);  /* ah inv */
extern int SetTextInverseFile(char *db_name);  


/******************************************************************************/
/* Deklarationen zu files2.c */
extern int GeomReadRFInputFileASCII ( FILE *fin, FILE *prot );
  /* Liest Geometriedaten aus der ASCII-RF-Eingabedatei ein, 0 bei Fehler */
extern int GeomReadRFInputFileBIN ( FILE *fin, FILE *prot );
  /* Liest Geometriedaten aus der Binaer-RF-Eingabedatei ein, 0 bei Fehler */
extern int ResReadRFInputFileASCII ( FILE *fin, FILE *prot );
  /* Liest Ergebnisdaten aus der ASCII-RF-Eingabedatei ein, 0 bei Fehler */
extern int ResReadRFInputFileBIN ( FILE *fin, FILE *prot );
  /* Liest Ergebnisdaten aus der Binaer-RF-Eingabedatei ein, 0 bei Fehler */

extern int GeomWriteRFOutputFileASCII ( FILE *fout );
  /* Schreibt Geometriedaten in ASCII-RF-Ausgabedatei, 0 bei Fehler */
extern int GeomWriteRFOutputFileBIN ( FILE *fout );
  /* Schreibt Geometriedaten in Binaer-RF-Ausgabedatei, 0 bei Fehler */
extern int ResWriteRFOutputFileASCII ( FILE *fout );
  /* Schreibt Ergebnisdaten in ASCII-RF-Ausgabedatei, 0 bei Fehler */
extern int ResWriteRFOutputFileBIN ( FILE *fout );
  /* Schreibt Ergebnisdaten in Binaer-RF-Ausgabedatei, 0 bei Fehler */

/* Weitere externe Objekte */


#endif

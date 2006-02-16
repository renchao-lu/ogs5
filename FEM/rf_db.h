#ifndef rf_db_INC

#define rf_db_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include <stdio.h>


/* Deklarationen */
typedef int (*ReadFunction) ( char * , int , FILE * );
  /* Funktionsprototyp fuer eine Funktion mit Ergebnis int, einem
     char-Parameter, einem int- und einem FILE-Parameter; zum Bearbeiten
     der einzelnen Schluesselwoerter */

typedef struct {                /* Datenstruktur fuer Eingabedatenbereich */
    char *identifier;           /* Schluesselwort */
    char *data;                 /* Alle zugehoerigen Daten als String */
    ReadFunction Func;          /* Funktion, die *data interpretiert */
    int hash;                   /* 0: keine Abschnittstrennung mit # */
    int found;                  /* 0: nicht gefunden; 1: gefunden */
    int komma;                  /* 0: Kommata nicht ersetzen; 1: ersetzen */
} EingabeDatenbank;


/* Funktionsprototypen */
extern EingabeDatenbank *CreateTextInputDataBase ( int sections );
  /* Erzeugt Feld mit sections Eintraegen vom Typ EingabeDatenbank */
extern void DestroyTextInputDataBase ( EingabeDatenbank *base, int sec );
  /* Zerstoert EingabeDatenbank */
extern void SetDataBaseSection ( EingabeDatenbank *base, int index,        \
                                 char *id, ReadFunction func, int hash,    \
                                 int komma );
  /* Setzt den EingabeDatenbank-Eintrag index, beginnend ab Index 1 !!! */
extern void AddDataBaseSection ( EingabeDatenbank *base, int index,        \
                                 char *id, ReadFunction func, int hash,    \
                                 int komma );
  /* Fuegt den EingabeDatenbank-Eintrag index am Ende ein, beginnend ab Index 1 !!! */

extern int ReadDataBaseData ( EingabeDatenbank *base, char *dateiname,    \
                              int sec );
  /* Liest Datei in *data-Feld ein, 0 bei Fehler */
extern int ExecuteDataBase ( EingabeDatenbank *base, int sec, FILE *prot );
  /* Fuehrt die ReadFunctions aus, 0 bei Fehler */

extern FILE *StartProtocol ( char *kennung, char *dateiname );
extern int EndProtocol ( FILE *f );
extern int EndMGProtocol ( FILE *f );
extern FILE *StartMGProtocol ( char *kennung, char *dateiname );
extern FILE *StartExtProtocol ( char *kennung, char *dateiname );
extern int EndExtProtocol ( FILE *f );


#endif


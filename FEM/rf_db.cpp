// MFC
#include "stdafx.h" 
// C
#include <string.h>
// RF
#include "rf_db.h"
#include "memory.h"
#include "geo_strings.h"
#include "display.h"

/**************************************************************************/
/* ROCKFLOW - Funktion: CreateTextInputDataBase
                                                                          */
/* Aufgabe:
   Erzeugt Feld mit sections Eintraegen vom Typ EingabeDatenbank
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int sections: Anzahl der Schluesselwoerter
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
EingabeDatenbank *CreateTextInputDataBase ( int sections )
{
  EingabeDatenbank *base = \
    (EingabeDatenbank *) Malloc((sections+1)*sizeof(EingabeDatenbank));
  base[0].data = NULL;
  base[0].hash = 0;
  base[0].komma = 0;
  return base;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyTextInputDataBase
                                                                          */
/* Aufgabe:
   Zerstoert Eingabedatenbank;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X EingabeDatenbank *base: Zeiger auf zu loeschendes Feld, wird geerdet
   E int sec: Anzahl der Feldeintraege
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
    9/2000     CT Speicher wieder freigegeben
                                                                          */
/**************************************************************************/
void DestroyTextInputDataBase ( EingabeDatenbank *base, int sec )
{
  int i;
  for (i=0; i<sec; i++)
      base[i].data = (char *) Free(base[i].data); /* C++ */

  base = (EingabeDatenbank *) Free(base); /* C++ */
}

/**************************************************************************/
/* ROCKFLOW - Funktion: SetDataBaseSection
                                                                          */
/* Aufgabe:
   Setzt Daten fuer Feldelement
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X EingabeDatenbank *base: Zeiger auf Feld
   E int Index: Feldindex, der belegt werden soll
   E char *id: Schluesselwort
   E ReadFunction func: Funktion, die *data interpretiert
   E int hash: Wenn hash=1, wird eine Abschnittstrennung durchgefuehrt:
               Wenn das entsprechende Schluesselwort gefunden wird, wird
               in *data zuerst ein # geschrieben. So kann hinterher das
               mehrfache Auftreten eines Schluesselwortes abgeprueft
               werden. Bei hash=0 wird kein # geschrieben.
   E int komma: Wenn komma=1, werden in den gefundenen Zeichenketten alle
                Kommata gegen Blanks ersetzt. Fuer komma=0 bleiben die
                Kommata erhalten.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void SetDataBaseSection ( EingabeDatenbank *base, int index,              \
                          char *id, ReadFunction func, int hash, int komma )
{
  base[index].identifier = StrUp(id); /* in display.c */
  base[index].data = (char *) Malloc(1024);
  base[index].data[0] = '\0';
  base[index].Func = func;
  base[index].hash = hash;
  base[index].found = 0;
  base[index].komma = komma;
}
/**************************************************************************/
/* ROCKFLOW - Funktion: AddDataBaseSection
                                                                          */
/* Aufgabe:
   Erzeugt Platz und setzt Daten fuer Feldelement
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X EingabeDatenbank *base: Zeiger auf Feld
   E int Index: Feldindex, der belegt werden soll
   E char *id: Schluesselwort
   E ReadFunction func: Funktion, die *data interpretiert
   E int hash: Wenn hash=1, wird eine Abschnittstrennung durchgefuehrt:
               Wenn das entsprechende Schluesselwort gefunden wird, wird
               in *data zuerst ein # geschrieben. So kann hinterher das
               mehrfache Auftreten eines Schluesselwortes abgeprueft
               werden. Bei hash=0 wird kein # geschrieben.
   E int komma: Wenn komma=1, werden in den gefundenen Zeichenketten alle
                Kommata gegen Blanks ersetzt. Fuer komma=0 bleiben die
                Kommata erhalten.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/2000     CT        Erste Version
                                                                          */
/**************************************************************************/
void AddDataBaseSection ( EingabeDatenbank *base, int index,              \
                          char *id, ReadFunction func, int hash, int komma )
{
  base = (EingabeDatenbank *) Realloc(base, (index+1)*sizeof(EingabeDatenbank));
  SetDataBaseSection (base, index, id, func, hash, komma );
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReadDataBaseData
                                                                          */
/* Aufgabe:
   Liest Datei in *data-Feld ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X EingabeDatenbank *base: Zeiger auf Feld
   E char *dateiname: Eingabedateiname incl. Extension
   E int sec: Anzahl der Feldeintraege
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   02/2000     CT         Warnung bei unbekannten (alten) Schluesselworten
   03/2000     TR         Bugfix
                                                                          */
/**************************************************************************/
int ReadDataBaseData ( EingabeDatenbank *base, char *dateiname, int sec )
{
  char s[256];
  char h[256];
  int i, l, found = 0;
  int puffer = 0;                                         /* puffer auf 0 */
  FILE *f = fopen(dateiname,"r");
  h[0] = '\0';
  if (f==NULL)
      return 0;

  /* das feof funktioniert nicht so richtig ...
  while ((!feof(f)) && (strcmp("STOP",h)!=0)) {
    if (fgets(s,256,f)==NULL)
        return 0;
  */

  while (fgets(s,256,f)) {  /* Zeile einlesen */
    switch (s[0]) {
      case '#' : {                       /* Kommentar oder Schluesselwort */
          /* Ende der Datei */
          if (strstr(s,"#STOP")) return 1;

          puffer = 0;                             /* alten puffer beenden */
          found = 0;
          if (sscanf(&s[1]," %s",h)!=0) {                /* etwas gelesen */
              StrUp(h);
              for (i=0; i<sec; i++) {
                /* mit Schluesselwoertern vergleichen */
                if (strcmp(base[i].identifier,h) == 0) {
                    puffer = i;                    /* Puffer umlenken */
                    found = 1;                     /* gefunden */
                    base[puffer].found = 1;        /* gefunden */
                    if (base[puffer].hash) {       /* evtl. # einfuegen */
                        base[puffer].data = \
                          (char *) Realloc(base[puffer].data, \
                          ((int)strlen(base[puffer].data)+4));
                        base[puffer].data = strcat(base[puffer].data," # ");
                    }
                }
             }
          }
          /* Unbekanntes Schluesselwort ? */
          if(!found) {
            DisplayMsgLn("***********************************************");
            DisplayMsg("WARNING! Unknown keyword detected: ");
            DisplayMsgLn(s);
          }
      }
      case '*' : ;
      case 'c' : ;
      case 'C' : ;
      case ';' : ;                                           /* Kommentar */
      case '\n' : break;                                     /* Leerzeile */
      default : {                                       /* Daten gefunden */
          h[0] = '\0';
          sscanf(s,"%[^\n#*;]",h);
                               /* alles lesen bis * oder # oder ; oder \n */
/* SB: read in component names with capital and small letters */
		  if(strstr(base[puffer].identifier,"COMPONENT_PROPERTIES")){ 
			/* component properties read in */
			if(strstr(h,"$"))	StrUp(h);  //SB:name klein
		  }
		  else if(strstr(base[puffer].identifier,"OUTPUT_EX")){
//			  DisplayMsgLn(h);
		  }
		  else

          StrUp(h);
          l = (int)strlen(h);
          if (base[puffer].komma) {
              for (i=0; i<l; i++) {  /* Ersetzen von Kommata durch Blanks */
                  if (h[i]==',')
                      h[i] = ' ';
              }
          }
          base[puffer].data = \
            (char *) Realloc(base[puffer].data, \
            ((int)strlen(base[puffer].data)+(int)strlen(h)+4));
          base[puffer].data = strcat(base[puffer].data," ");
                                                  /* whitespace in Puffer */
          base[puffer].data = strcat(base[puffer].data,h);
                                            /* gelesenen String anhaengen */
          base[puffer].data = strcat(base[puffer].data," \n");
                                 /* whitespace (Zeilenvorschub) in Puffer */
      }
    }
  }
  if (fclose(f)<0)
      return 0;
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteDataBase
                                                                          */
/* Aufgabe:
   Fuehrt die ReadFunctions aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X EingabeDatenbank *base: Zeiger auf Feld
   E int sec: Anzahl der Feldeintraege
   E FILE *prot: Dateizeiger auf die Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ExecuteDataBase ( EingabeDatenbank *base, int sec, FILE *prot )
{
  int ok = 1;
  int i;
  for (i=0; i<sec; i++) {
    if (base[i].Func(base[i].data,base[i].found,prot) == 0){
          /* Fehler in base[i].Func aufgetreten */
          DisplayMsgLn("***********************************************");
          DisplayMsg("Error in keyword: #");
          DisplayMsgLn(base[i].identifier);
          ok = 0;
    }
  }
  return ok;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: StartExtProtocol
                                                                          */
/* Aufgabe:
   Eroeffnet Protokoll-Textdatei zum Anhaengen des Datenprotokolls und
   schreibt Ueberschrift
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *kennung: Name des protokollierenden Programmes
   E char *dateiname: Dateiname der zu oeffnenden Datei
                                                                          */
/* Ergebnis:
   Dateizeiger, NULL bei Fehler
                                                                          */
/* Programmaenderungen:
   06/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
FILE *StartExtProtocol ( char *kennung, char *dateiname )
{
  FILE *f = fopen(dateiname,"a");
  if (f==NULL)
      return NULL;
  if (fprintf(f,"\n\n***\n*** %s - Datenprotokoll\n***\n\n",kennung)<0)
      return  NULL;
  return f;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: EndExtProtocol
                                                                          */
/* Aufgabe:
   Schliesst erweiterte Protokoll-Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int EndExtProtocol ( FILE *f )
{
  if (fprintf(f,"\n***\n*** Ende des Datenprotokolls\n***\n")<0)
      return 0;
  if (fclose(f)<0)
      return 0;
  return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: StartMGProtocol
                                                                          */
/* Aufgabe:
   Eroeffnet Protokoll-Textdatei und schreibt Ueberschrift
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *kennung: Name des protokollierenden Programmes
   E char *dateiname: Dateiname der zu oeffnenden Datei
                                                                          */
/* Ergebnis:
   Dateizeiger, NULL bei Fehler
                                                                          */
/* Programmaenderungen:
   07/2000    AH      Erste Version
                                                                          */
/**************************************************************************/
FILE *StartMGProtocol ( char *kennung, char *dateiname )
{
  FILE *f = fopen(dateiname,"w");
  if (f==NULL)
      return NULL;
 /*  if (fprintf(f,"*\n*   (keine Kommentare uebernommen)\n*\n")<0) return NULL;*/
#ifdef EXT_RFM
    if (fprintf(f,"***\n*** %s - Protokolldatei %s \n*** Erweitertes Protokoll (EXT_RFM) \n",kennung,dateiname)<0) return NULL;
    if (fprintf(f,"\n; Allgemeine Hinweise zum Erstellen einer Text-Eingabedatei:\n")<0) return NULL;
    if (fprintf(f,"; Die maximale Zeilenlaenge betraegt 254 Zeichen.\n")<0) return NULL;
    if (fprintf(f,"; Ein Datenabschnitt wird durch ein Schluesselwort eingeleitet. Ein\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort besteht aus einem # in der 1. Spalte und 4 relevanten\n")<0) return NULL;
    if (fprintf(f,"; Zeichen zur Identifikation. Der Rest der Schluesselwortzeile wird als\n")<0) return NULL;
    if (fprintf(f,"; Kommentar behandelt.\n")<0) return NULL;
    if (fprintf(f,"; Kommentare koennen ueberall im Text vorkommen. Sie werden in der 1. Spalte\n")<0) return NULL;
    if (fprintf(f,"; einer Kommentarzeile durch *, #, c, C oder ; eingeleitet. Beginnt der\n")<0) return NULL;
    if (fprintf(f,"; Kommentar erst nach der 1. Spalte, z.B. nach einem Datum, so kann er nicht\n")<0) return NULL;
    if (fprintf(f,"; mehr mit c oder C eingeleitet werden, sondern nur noch mit den anderen\n")<0) return NULL;
    if (fprintf(f,"; o.a. Zeichen. Alles, was vor dem ersten Schluesselwort in der Eingabedatei\n")<0) return NULL;
    if (fprintf(f,"; steht, wird ebenfalls als Kommentar behandelt. Kommentare werden nicht\n")<0) return NULL;
    if (fprintf(f,"; bearbeitet und nicht mit protokolliert.\n")<0) return NULL;
    if (fprintf(f,"; Innerhalb eines Datenabschnittes muessen die einzelnen Daten durch\n")<0) return NULL;
    if (fprintf(f,"; Whitespace (Leerzeichen, Zeilenvorschuebe) oder Kommata getrennt werden.\n")<0) return NULL;
    if (fprintf(f,"; (Kommata werden in allen Datenabschnitten ausser der Projektkennung\n")<0) return NULL;
    if (fprintf(f,"; durch Leerzeichen ersetzt.)\n")<0) return NULL;
    if (fprintf(f,"; Ein Datenabschnitt wird durch ein weiteres Schluesselwort oder durch\n")<0) return NULL;
    if (fprintf(f,"; eine mit einem # eingeleitete Kommantarzeile beendet. Das\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort #STOP beendet das Lesen aus der Eingabedatei.\n")<0) return NULL;
    if (fprintf(f,"; Alle Zeilen, in denen Daten stehen, muessen mit einem Zeilenvorschub\n")<0) return NULL;
    if (fprintf(f,"; abgeschlossen werden.\n")<0) return NULL;
    if (fprintf(f,"; Die Reihenfolge der Schluesselwoerter ist beliebig. Daten eines Abschnitts\n")<0) return NULL;
    if (fprintf(f,"; koennen auch in mehreren Bloecken, die alle von dem entsprechenden\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort eingeleitet werden, eingegeben werden. Sie werden dann\n")<0) return NULL;
    if (fprintf(f,"; zu einem Datenabschnitt zusammengefuegt. Manche Datenabschnitte muessen\n")<0) return NULL;
    if (fprintf(f,"; sogar auf diese Art unterteilt werden, d.h. es werden mehrere Abschnitte\n")<0) return NULL;
    if (fprintf(f,"; mit dem gleichen Schluesselwort gefordert.\n")<0) return NULL;
    if (fprintf(f,"; Gross- und Kleinschreibung wird nicht unterschieden.\n")<0) return NULL;
    if (fprintf(f,";\n; Gleitkommazahlen sollten grundsaetzlich mit Dezimalpunkt geschrieben\n")<0) return NULL;
    if (fprintf(f,"; werden; einzeln stehende Vorzeichen sollten vermieden werden, da sie sonst\n")<0) return NULL;
    if (fprintf(f,"; evtl. als 0 interpretiert werden.\n")<0) return NULL;
    if (fprintf(f,"; \n")<0) return NULL;
    if (fprintf(f,"; Inhalt der [] bei der Beschreibung der Daten:\n")<0) return NULL;
    if (fprintf(f,"; - unendlicher Wertebereich: * (Datentyp)\n")<0) return NULL;
    if (fprintf(f,"; - endlicher Wertebereich: zulaessiger Wertebereich;\n")<0) return NULL;
    if (fprintf(f,";                           Vorgabewert bei Nichteinhalten des Wertebereichs\n")<0) return NULL;
    if (fprintf(f,";                           oder #, falls bei Nichteinhalten des Bereichs\n")<0) return NULL;
    if (fprintf(f,";                           abgebrochen wird.\n\n")<0) return NULL;
#else
    if (fprintf(f,"***\n*** %s - Protokolldatei %s \n*** Kurzprotokoll (Standard) \n",kennung,dateiname)<0) return NULL;
#endif
  return f;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: EndMGProtocol
                                                                          */
/* Aufgabe:
   Schliesst Protokoll-Textdatei und schreibt Ende-Code #STOP
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    AH      Erste Version
                                                                          */
/**************************************************************************/
int EndMGProtocol ( FILE *f )
{
  LineFeed(f);
  if (fprintf(f,"#STOP\n")<0)
      return 0;
  if (fclose(f)<0)
      return 0;
  return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: StartProtocol
                                                                          */
/* Aufgabe:
   Eroeffnet Protokoll-Textdatei und schreibt Ueberschrift
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *kennung: Name des protokollierenden Programmes
   E char *dateiname: Dateiname der zu oeffnenden Datei
                                                                          */
/* Ergebnis:
   Dateizeiger, NULL bei Fehler
                                                                          */
/* Programmaenderungen:
   03/1994   MSR  Erste Version
   11/2002   OK   error output
                                                                          */
/**************************************************************************/
FILE *StartProtocol ( char *kennung, char *dateiname )
{
  FILE *f = fopen(dateiname,"w");
  if (f==NULL) {
      DisplayErrorMsg("Fehler: Protokolldatei konnte nicht geoeffnet werden !");
      return NULL;
  }
 /*  if (fprintf(f,"*\n*   (keine Kommentare uebernommen)\n*\n")<0) return NULL;*/
#ifdef EXT_RFD
    if (fprintf(f,"***\n*** %s - Protokolldatei %s \n*** Erweitertes Protokoll (EXT_RFD) \n",kennung,dateiname)<0) return NULL;
    if (fprintf(f,"\n; Allgemeine Hinweise zum Erstellen einer Text-Eingabedatei:\n")<0) return NULL;
    if (fprintf(f,"; Die maximale Zeilenlaenge betraegt 254 Zeichen.\n")<0) return NULL;
    if (fprintf(f,"; Ein Datenabschnitt wird durch ein Schluesselwort eingeleitet. Ein\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort besteht aus einem # in der 1. Spalte und 4 relevanten\n")<0) return NULL;
    if (fprintf(f,"; Zeichen zur Identifikation. Der Rest der Schluesselwortzeile wird als\n")<0) return NULL;
    if (fprintf(f,"; Kommentar behandelt.\n")<0) return NULL;
    if (fprintf(f,"; Kommentare koennen ueberall im Text vorkommen. Sie werden in der 1. Spalte\n")<0) return NULL;
    if (fprintf(f,"; einer Kommentarzeile durch *, #, c, C oder ; eingeleitet. Beginnt der\n")<0) return NULL;
    if (fprintf(f,"; Kommentar erst nach der 1. Spalte, z.B. nach einem Datum, so kann er nicht\n")<0) return NULL;
    if (fprintf(f,"; mehr mit c oder C eingeleitet werden, sondern nur noch mit den anderen\n")<0) return NULL;
    if (fprintf(f,"; o.a. Zeichen. Alles, was vor dem ersten Schluesselwort in der Eingabedatei\n")<0) return NULL;
    if (fprintf(f,"; steht, wird ebenfalls als Kommentar behandelt. Kommentare werden nicht\n")<0) return NULL;
    if (fprintf(f,"; bearbeitet und nicht mit protokolliert.\n")<0) return NULL;
    if (fprintf(f,"; Innerhalb eines Datenabschnittes muessen die einzelnen Daten durch\n")<0) return NULL;
    if (fprintf(f,"; Whitespace (Leerzeichen, Zeilenvorschuebe) oder Kommata getrennt werden.\n")<0) return NULL;
    if (fprintf(f,"; (Kommata werden in allen Datenabschnitten ausser der Projektkennung\n")<0) return NULL;
    if (fprintf(f,"; durch Leerzeichen ersetzt.)\n")<0) return NULL;
    if (fprintf(f,"; Ein Datenabschnitt wird durch ein weiteres Schluesselwort oder durch\n")<0) return NULL;
    if (fprintf(f,"; eine mit einem # eingeleitete Kommantarzeile beendet. Das\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort #STOP beendet das Lesen aus der Eingabedatei.\n")<0) return NULL;
    if (fprintf(f,"; Alle Zeilen, in denen Daten stehen, muessen mit einem Zeilenvorschub\n")<0) return NULL;
    if (fprintf(f,"; abgeschlossen werden.\n")<0) return NULL;
    if (fprintf(f,"; Die Reihenfolge der Schluesselwoerter ist beliebig. Daten eines Abschnitts\n")<0) return NULL;
    if (fprintf(f,"; koennen auch in mehreren Bloecken, die alle von dem entsprechenden\n")<0) return NULL;
    if (fprintf(f,"; Schluesselwort eingeleitet werden, eingegeben werden. Sie werden dann\n")<0) return NULL;
    if (fprintf(f,"; zu einem Datenabschnitt zusammengefuegt. Manche Datenabschnitte muessen\n")<0) return NULL;
    if (fprintf(f,"; sogar auf diese Art unterteilt werden, d.h. es werden mehrere Abschnitte\n")<0) return NULL;
    if (fprintf(f,"; mit dem gleichen Schluesselwort gefordert.\n")<0) return NULL;
    if (fprintf(f,"; Gross- und Kleinschreibung wird nicht unterschieden.\n")<0) return NULL;
    if (fprintf(f,";\n; Gleitkommazahlen sollten grundsaetzlich mit Dezimalpunkt geschrieben\n")<0) return NULL;
    if (fprintf(f,"; werden; einzeln stehende Vorzeichen sollten vermieden werden, da sie sonst\n")<0) return NULL;
    if (fprintf(f,"; evtl. als 0 interpretiert werden.\n")<0) return NULL;
    if (fprintf(f,"; \n")<0) return NULL;
    if (fprintf(f,"; Inhalt der [] bei der Beschreibung der Daten:\n")<0) return NULL;
    if (fprintf(f,"; - unendlicher Wertebereich: * (Datentyp)\n")<0) return NULL;
    if (fprintf(f,"; - endlicher Wertebereich: zulaessiger Wertebereich;\n")<0) return NULL;
    if (fprintf(f,";                           Vorgabewert bei Nichteinhalten des Wertebereichs\n")<0) return NULL;
    if (fprintf(f,";                           oder #, falls bei Nichteinhalten des Bereichs\n")<0) return NULL;
    if (fprintf(f,";                           abgebrochen wird.\n\n")<0) return NULL;
#else
    if (fprintf(f,"***\n*** %s - Protokolldatei %s \n*** Kurzprotokoll (Standard) \n",kennung,dateiname)<0) return NULL;
#endif
  return f;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: EndProtocol
                                                                          */
/* Aufgabe:
   Schliesst Protokoll-Textdatei und schreibt Ende-Code #STOP
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int EndProtocol ( FILE *f )
{
  LineFeed(f);
  if (fprintf(f,"#STOP\n")<0)
      return 0;
  if (fclose(f)<0)
      return 0;
  return 1;
}



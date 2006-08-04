/**************************************************************************
STRLib
Task: 
Programing:
last modified:
**************************************************************************/
#include "stdafx.h"   
#include "geo_strings.h"
#include "geo_mathlib.h"
//C++
#include <sstream>
#include <string>
using namespace std;
#define KEYWORD '#'
#define SUBKEYWORD '$'

#define FORMAT_DOUBLE
#define FPD_GESAMT     4
#define FPD_NACHKOMMA  14

/**************************************************************************/
/* ROCKFLOW - Funktion: LineFeed
                                                                          */
/* Aufgabe:
   Schreibt Zeilenvorschub in Textdatei
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
int LineFeed ( FILE *f )
{
  if (fprintf(f,"\n")<0)
      return 0;
  return 1;
}
/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintString
                                                                          */
/* Aufgabe:
   Schreibt Zeichenkette ohne Zeilenvorschub in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FilePrintString ( FILE *f, char *s )
{
  if ((int)fprintf(f,"%s",s)!=(int)strlen(s))
      return 0;
  return 1;
}
/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintInt
                                                                          */
/* Aufgabe:
   Schreibt Integer-Wert ohne Zeilenvorschub in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E int x: Integer-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FilePrintInt ( FILE *f, int x )
{
  if (fprintf(f," %i ",x)<0)
      return 0;
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintLong
                                                                          */
/* Aufgabe:
   Schreibt Long-Wert ohne Zeilenvorschub in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E long x: Long-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FilePrintLong ( FILE *f, long x )
{
  if (fprintf(f," %ld ",x)<0)
      return 0;
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintDouble
                                                                          */
/* Aufgabe:
   Schreibt Double-Wert ohne Zeilenvorschub in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E double x: Double-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
   12/1995     cb         E-Format
                                                                          */
/**************************************************************************/
int FilePrintDouble ( FILE *f, double x )
{
#ifdef FORMAT_DOUBLE
    if (fprintf(f," % #*.*g ",FPD_GESAMT,FPD_NACHKOMMA,x)<0)
        return 0;
#else
    if (fprintf(f," % #g ",x)<0)
        return 0;
#endif
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintIntWB
                                                                          */
/* Aufgabe:
   Schreibt Integer-Wert ohne Zeilenvorschub und ohne fuehrende
   Leerzeichen in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E int x: Integer-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FilePrintIntWB ( FILE *f, int x )
{
  if (fprintf(f,"%-i ",x)<0)
      return 0;
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintLongWB
                                                                          */
/* Aufgabe:
   Schreibt Long-Wert ohne Zeilenvorschub und ohne fuehrende
   Leerzeichen in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E long x: Long-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FilePrintLongWB ( FILE *f, long x )
{
  if (fprintf(f,"%-ld ",x)<0)
      return 0;
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FilePrintDoubleWB
                                                                          */
/* Aufgabe:
   Schreibt Double-Wert ohne Zeilenvorschub und ohne fuehrende
   Leerzeichen in Textdatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E double x: Double-Wert
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/1994     MSR        Erste Version
   12/1995     cb         E-Format
                                                                          */
/**************************************************************************/
int FilePrintDoubleWB ( FILE *f, double x )
{
#ifdef FORMAT_DOUBLE
    if (fprintf(f,"%-#*.*g ",FPD_GESAMT,FPD_NACHKOMMA,x)<0)
        return 0;
#else
    if (fprintf(f,"%-#g ",x)<0)
        return 0;
#endif
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FileCommentDoubleMatrix
                                                                          */
/* Aufgabe:
   Schreibt Double-Matrix als Kommentar (mit fuehrendem *) in Textdatei.
   Ist der Matrixzeiger geerdet, wird NULL ausgegeben. Die Ausgabe
   erfolgt in m Zeilen und n Spalten mit Einrueckung adjust.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger
   E double *d: Zeiger auf Double-Matrix
   E int adjust: Einrueckung links in Zeichen
   E long m: Anzahl der Zeilen
   E long n: Anzahl der Spalten
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int FileCommentDoubleMatrix ( FILE *f, double *d, int adjust, long m, long n )
{
  int i;
  long j, k;
  if (d==NULL) {
      if (!FilePrintString(f,"*"))
          return 0;
      for (i=0;i<adjust;i++) {
          if (!FilePrintString(f," "))
              return 0;
      }
      if (!FilePrintString(f,"NULL"))
          return 0;
      if (!LineFeed(f))
          return 0;
  }
  else {
      for (j=0l;j<m;j++) {  /* Schleife ueber alle Zeilen */
          if (!FilePrintString(f,"*"))
              return 0;
          for (i=0;i<adjust;i++) {
              if (!FilePrintString(f," "))
                  return 0;
          }
          for (k=0l;k<n;k++) {  /* Schleife ueber alle Spalten */
              if (!FilePrintDouble(f,d[j*n+k]))
                  return 0;
              if (!FilePrintString(f," "))
                  return 0;
          }
          if (!LineFeed(f))
              return 0;
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadInt
                                                                          */
/* Aufgabe:
   Liest Integer-Wert aus String und schreibt Protokoll in Datei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R int *x: gelesener Integer-Wert
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestInt func: Funktionszeiger auf die Funktion, die den
                      eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrReadInt ( int *x, char *s, FILE *f, FctTestInt func, int *pos )
{
  int test;
  char c = ' ';
  *x = 0;
  if ((sscanf(s," %i%n%c",x,pos,&c)<=0) || (c=='.')) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %i      *** Fehler: Kein Wert eingelesen (int) !!!\n",*x);
      return 0;
  }
  else {
      test = func(x,f);
      fprintf(f," %i ",*x);
      return test;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadLong
                                                                          */
/* Aufgabe:
   Liest Long-Wert aus String und schreibt Protokoll in Datei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R long *x: gelesener Long-Wert
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestLong func: Funktionszeiger auf die Funktion, die den
                       eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrReadLong ( long *x, char *s, FILE *f, FctTestLong func, int *pos )
{
  int test;
  char c = ' ';
  *x = 0l;
  if ((sscanf(s," %ld%n%c",x,pos,&c)<=0) || (c=='.')) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %ld      *** Fehler: Kein Wert eingelesen (long) !!!\n",*x);
      return 0;
  }
  else {
      test = func(x,f);
      fprintf(f," %ld ",*x);
      return test;
  }
}

int StrReadLongNoComment ( long *x, char *s, FILE *f, FctTestLong func, int *pos )
{
  int test;
  char c = ' ';
  *x = 0l;
  if ((sscanf(s," %ld%n%c",x,pos,&c)<=0) || (c=='.')) {
      return 0;
  }
  else {
      test = func(x,f);
      fprintf(f," %ld ",*x);
      return test;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadFloat
                                                                          */
/* Aufgabe:
   Liest Float-Wert aus String und schreibt Protokoll in Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R float *x: gelesener Float-Wert
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestFloat func: Funktionszeiger auf die Funktion, die den
                         eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1998     OK        Erste Version
                                                                          */
/**************************************************************************/
int StrReadFloat ( float *x, char *s, FILE *f, FctTestFloat func, int *pos )
{
  int test;
  *x = 0.0;
  if (sscanf(s," %f%n",x,pos)<=0) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %f      *** Fehler: Kein Wert eingelesen (float) !!!\n",*x);
      return 0;
  }
  else {
      test = func(x,f);

/* CT: Protokolformat geaendert */
      if ((fabs(*x)<100000.)&&(fabs(*x)>=0.1)) {
      fprintf(f," %f ",*x);
      }
      else {
        fprintf(f," %e ",*x);
      }

      return test;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadDouble
                                                                          */
/* Aufgabe:
   Liest Double-Wert aus String und schreibt Protokoll in Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *x: gelesener Double-Wert
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestDouble func: Funktionszeiger auf die Funktion, die den
                         eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrReadDouble ( double *x, char *s, FILE *f, FctTestDouble func, int *pos )
{
  int test;
  *x = 0.0;
  if (sscanf(s," %lf%n",x,pos)<=0) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %f      *** Fehler: Kein Wert eingelesen (double) !!!\n",*x);
      return 0;
  }
  else {
      test = func(x,f);

/* CT: Protokolformat geaendert */
      if ((fabs(*x)<100000.)&&(fabs(*x)>=0.1)) {
      fprintf(f," %f ",*x);
      }
      else {
        fprintf(f," %e ",*x);
      }

      return test;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadString
                                                                          */
/* Aufgabe:
   Liest Zeichenkette aus String und schreibt Protokoll in Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char **x: Zeiger auf Adresse der gelesenen Zeichenkette; eine vorher
               vorhandene wird geloescht, es sollten nur mit malloc erzeugte
               Zeichenketten verwendet werden.
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestString func: Funktionszeiger auf die Funktion, die den
                         eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrReadString ( char **x, char *s, FILE *f, FctTestString func, int *pos )
{
  int test;
  /*  *x = Free(*x); */
  *x = NULL;
  //*x = (char *) Malloc(256);
  *x = new char[256]; //CC
  *x[0] = '\0';
  
  if (sscanf(s," %s%n",*x,pos)<=0) {
      int a = (int)strlen(*x);//CC
      //delete[] *x;//CC
      *x = new char[a +1];
      //*x = (char *) Realloc(*x,((int)strlen(*x)+1));
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %s      *** Fehler: Kein Wert eingelesen (string) !!!\n",*x);
      return 0;
  }
  else {
      test = func(*x,f);
      int a = (int)strlen(*x);//CC
     // delete[] *x;//CC
     *x = new char[a+1];//CC
      //*x = (char *) Realloc(*x,((int)strlen(*x)+1));
      fprintf(f,"%s ",*x);
      return test;
  }
}
/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadStr
                                                                          */
/* Aufgabe:
   Liest Zeichenkette aus String und schreibt Protokoll in Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char **x: Adresse der gelesenen Zeichenkett, es muss vorher Speicher
               allokiert werden
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestString func: Funktionszeiger auf die Funktion, die den
                         eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   08/2000     CT        Erste Version
                                                                          */
/**************************************************************************/
int StrReadStr ( char *x, char *s, FILE *f, FctTestString func, int *pos )
{
  int test;

  x[0] = '\0';
  if (sscanf(s," %s%n",x,pos)<=0) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      fprintf(f,"\n %s      *** Fehler: Kein Wert eingelesen (string) !!!\n",x);
      return 0;
  }
  else {
      test = func(x,f);
      fprintf(f,"%s ",x);
      return test;
  }
}

int StrReadStrNoComment ( char *x, char *s, FILE *f, FctTestString func, int *pos )
{
  int test;

  x[0] = '\0';
  if (sscanf(s," %s%n",x,pos)<=0) {
      return 0;
  }
  else {
      test = func(x,f);
      fprintf(f,"%s ",x);
      return test;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadSubKeyword
                                                                          */
/* Aufgabe:
   Liest ein Sub-Keyword (eingeleitet mit "$") aus Keyword-String. Nur bis zum
   naechsten Hash (naechstes Keyword) oder Stringende
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char *sub :  Zeichenkette, in die das Sub-Keywords geschrieben wird
   E char *s   : Zeichenkette, aus der gelesen werden soll
   E int begin : Erstes Zeichen, ab dem gesucht werden soll
   R int *found: Erstes Zeichen des Sub-Keywords im String s
   R int *ende : Letztes Zeichen des Sub-Keywords im String s, oder
                 Beginn des naechsten Keywords, oder Ende des Strings

                                                                          */
/* Ergebnis:
   1 bei gefundenem Sub-Keyword, sonst 0
                                                                          */
/* Programmaenderungen:
   08/2000 C.Thorenz  Erste Version
                                                                          */
/**************************************************************************/
int StrReadSubKeyword ( char *sub, char *s, int beginn, int *found, int *ende)
{
  int i, xi=0;

  *found=-1;
  *ende=(int)strlen(s);

  for(i=beginn;i<(int)strlen(s);i++){
     if(s[i]=='$') {
       if(*found<1) {
           /* Anfang des Sub-Keywords merken */
           *found=i;
       } else {
           /* Ende des Sub-Keywords merken (neues Sub-Keyword folgt) */
           *ende=i;
           break;
       }
     }

     if(s[i]=='#') {
        /* Ende des Sub-Keywords merken (neues Keyword folgt) */
        *ende=i;
        break;
     }

     if(*found>=0) {
       sub[xi]=s[i];
       xi++;
     }
  }

  if(*found>=0) {
     sub[xi] = '\0';
  }

  return  (*found>=0);

}
/**************************************************************************/
/* ROCKFLOW - Funktion: StrReadSubSubKeyword
                                                                          */
/* Aufgabe:
   Liest ein Sub-Sub-Keyword (eingeleitet mit "&") aus Keyword-String. Nur bis zum
   naechsten Hash, das bei mehrfach vorhandenem Keyword auftritt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char *sub :  Zeichenkette, in die das Sub-Keywords geschrieben wird
   E char *s   : Zeichenkette, aus der gelesen werden soll
   E int begin : Erstes Zeichen, ab dem gesucht werden soll
   R int *found: Erstes Zeichen des Sub-Keywords im String s
   R int *ende : Letztes Zeichen des Sub-Keywords im String s, oder
                 Beginn des naechsten Keywords, oder Ende des Strings

                                                                          */
/* Ergebnis:
   1 bei gefundenem Sub-Keyword, sonst 0
                                                                          */
/* Programmaenderungen:
   08/2000 C.Thorenz  Erste Version
                                                                          */
/**************************************************************************/
int StrReadSubSubKeyword ( char *sub, char *s, int beginn, int *found, int *ende)
{
  int i, xi=0;

  *found=-1;
  *ende=(int)strlen(s);

  for(i=beginn;i<(int)strlen(s);i++){
     if(s[i]=='&') {
       if(*found<1) {
           /* Anfang des Sub-Keywords merken */
           *found=i;
       } else {
           /* Ende des Sub-Keywords merken (neues Sub-Keyword folgt) */
           *ende=i;
           break;
       }
     }

     if((s[i]=='$')||(s[i]=='#')) {
        /* Ende des Sub-Sub-Keywords merken (neues (Sub-)Keyword folgt) */
        *ende=i;
        break;
     }

     if(*found>=0) {
       sub[xi]=s[i];
       xi++;
     }
  }

  if(*found>=0) {
     sub[xi] = '\0';
  }

  return  (*found>=0);

}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestInt
                                                                          */
/* Aufgabe:
   Testet, ob in s noch ein Integer kommt;
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrTestInt ( char *s )
{
  int i;
  char c = ' ';
  if ((sscanf(s," %d%c",&i,&c)<=0) || (c=='.'))
      return 0;
  else
      return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestLong
                                                                          */
/* Aufgabe:
   Testet, ob in s noch ein LongInt kommt;
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrTestLong ( char *s )
{
  long i;
  char c = ' ';
  if ((sscanf(s," %ld%c",&i,&c)<=0) || (c=='.'))
      return 0;
  else
      return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestFloat
                                                                          */
/* Aufgabe:
   Testet, ob in s noch ein Double kommt;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   07/1998     OK        Erste Version
                                                                          */
/**************************************************************************/
int StrTestFloat ( char *s )
{
  float i;
  if (sscanf(s," %f",&i)<=0)
      return 0;
  else
      return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestDouble
                                                                          */
/* Aufgabe:
   Testet, ob in s noch ein Double kommt;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrTestDouble ( char *s )
{
  double i;
  if (sscanf(s," %lf",&i)<=0)
      return 0;
  else
      return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestString
                                                                          */
/* Aufgabe:
   Testet, ob in s noch ein String kommt;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrTestString ( char *s )
{
  char i[256];
  if (sscanf(s," %s",i)<=0)
      return 0;
  else
      return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestHash
                                                                          */
/* Aufgabe:
   Testet, ob in s ein # folgt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
   R int *pos: gelesene Zeichen bis nach dem # (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int StrTestHash ( char *s, int *pos )
{
  int p;
  char h[256];
  if (sscanf(s," %s%n",h,&p)<=0) {
      return 0;
  }
  else {
      if (strcmp(h,"#")==0) {
          *pos = p;
          return 1;
      }
      else {
          return 0;
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestDollar
                                                                          */
/* Aufgabe:
   Testet, ob in s ein $ folgt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
   R int *pos: gelesene Zeichen bis nach dem $ (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   08/2000     CT        Erste Version
                                                                          */
/**************************************************************************/
int StrTestDollar( char *s, int *pos )
{
  int p;
  char h[256];
  if (sscanf(s," %s%n",h,&p)<=0) {
      return 0;
  }
  else {
      if (strncmp(h,"$",1)==0) {
          *pos = p;
          return 1;
      }
      else {
          return 0;
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrTestInv
                                                                          */
/* Aufgabe:
   Testet, ob in s ein # folgt
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
   R int *pos: gelesene Zeichen bis nach dem ? (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
int StrTestInv ( char *s, int *pos )
{
  int p;
  char h[256];
  if (sscanf(s," %s%n",h,&p)<=0) {
      return 0;
  }
  else {
      if (strcmp(h,"?")==0) {
          *pos = p;
          return 1;
      }
      else {
          *pos = 0;
          return 0;
      }
  }
}


/* zusaetzliche Lesefunktionen fuer RF-SHELL */

/**************************************************************************/
/* ROCKFLOW - Funktion: StringReadDouble
                                                                          */
/* Aufgabe:
   Liest aus s einen double-Wert aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s:   Zeichenkette
   R double *x: gelesener double-Wert
   R int *pos:  gelesene Zeichen bis nach dem double-Wert (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
int StringReadDouble ( double *x, char *s, int *pos )
{
        int test = 1;

    *x = 0.0;
    if (sscanf(s," %lf%n",x,pos)<=0) {
    *pos = 0;  /* nichts sinnvolles gelesen */
    /* fprintf(f,"\n %f      *** Fehler: Kein Wert eingelesen (double) !!!\n",*x); */
    return 0;
        }
        return test;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StringReadFloat
                                                                          */
/* Aufgabe:
   Liest aus s einen float-Wert aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s:  Zeichenkette
   R float *x: gelesener float-Wert
   R int *pos: gelesene Zeichen bis nach dem float-Wert (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
int StringReadFloat ( float *x, char *s, int *pos )
{
        int test = 1;

    *x = 0.0;
    if (sscanf(s," %f%n",x,pos)<=0) {
    /* if (sscanf(s," % #*.*g%n",FPD_GESAMT,FPD_NACHKOMMA,x,pos)<=0) { */
    *pos = 0;  /* nichts sinnvolles gelesen */
    /* fprintf(f,"\n %f      *** Fehler: Kein Wert eingelesen (double) !!!\n",*x); */
    return 0;
        }
        return test;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StringReadInt
                                                                          */
/* Aufgabe:
   Liest aus s einen int-Wert aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s:  Zeichenkette
   R int *x:   gelesener int-Wert
   R int *pos: gelesene Zeichen bis nach dem int-Wert (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
int StringReadInt ( int *x, char *s, int *pos )
{
  int test=1;
  char c = ' ';
  *x = 0;
  if ((sscanf(s," %i%n%c",x,pos,&c)<=0) || (c=='.')) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      /* fprintf(f,"\n %i      *** Fehler: Kein Wert eingelesen (int) !!!\n",*x); */
      return 0;
  }
  else {
      /* test = func(x,f);
      fprintf(f," %i ",*x); */
      return test;
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: StringReadLong
                                                                          */
/* Aufgabe:
   Liest aus s einen long-Wert aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s:  Zeichenkette
   R long *x:  gelesener long-Wert
   R int *pos: gelesene Zeichen bis nach dem long-Wert (wenn gefunden)
                                                                          */
/* Ergebnis:
   0: nein; 1: ja
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
int StringReadLong ( long *x, char *s, int *pos )
{
  int test=1;
  char c = ' ';
  *x = 0l;
  if ((sscanf(s," %ld%n%c",x,pos,&c)<=0) || (c=='.')) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      /* fprintf(f,"\n %ld      *** Fehler: Kein Wert eingelesen (long) !!!\n",*x); */
      return 0;
  }
  else {
      /* test = func(x,f);
      fprintf(f," %ld ",*x); */
      return test;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StringReadStr
                                                                          */
/* Aufgabe:
   Liest Zeichenkette aus String
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char **x: Zeiger auf Adresse der gelesenen Zeichenkette; eine vorher
               vorhandene wird geloescht, es sollten nur mit malloc erzeugte
               Zeichenketten verwendet werden.
   E char *s: Zeichenkette, aus der gelesen werden soll
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1999   OK   aus StrReadString
                                                                          */
/**************************************************************************/
int StringReadStr ( char **x, char *s, int *pos )
{

  *x = NULL;

//  *x = (char *) Malloc(256);
*x = new char[256];//CC
  *x[0] = '\0';
  if (sscanf(s," %s%n",*x,pos)<=0) {
      int a = (int)strlen(*x);//CC
      //delete[] *x;//CC
      *x = new char[a+1];//CC
      //*x = (char *) Realloc(*x,((int)strlen(*x)+1));
      *pos = 0;  /* nichts sinnvolles gelesen */
      return 0;
  }
  else {
      return 1;
  }
}
/* allgemeine Dummy-Testfunktionen bei uneingeschraenktem Wertebereich */

int TFInt ( int *x, FILE *f )
{
  int idummy;
  FILE fdummy;
  idummy = *x;
  fdummy = *f;

  return 1;
}


int TFLong ( long *x, FILE *f )
{
  long dummy;
  FILE fdummy;
  dummy = *x;
  fdummy = *f;
  return 1;
}

int TFFloat ( float *x, FILE *f )
{
  float dummy;
  FILE fdummy;
  dummy = *x;
  fdummy = *f;
  return 1;
}


int TFDouble ( double *x, FILE *f )
{
  double dummy;
  FILE fdummy;
  dummy = *x;
  fdummy = *f;
  return 1;
}


int TFString ( char *x, FILE *f )
{
  char dummy;
  FILE fdummy;
  dummy = *x;
  fdummy = *f;
  return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ReadString
                                                                          */
/* Aufgabe:
   Liest Zeichenkette von Standardeingabe
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   gelesene Zeichenkette
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
char *ReadString ( void )
{
  char *s = (char *) malloc(256);
  //char *s = new char[256];//CC
  scanf(" %s%*[^\n]%*c",s);
//  int a = (int)strlen(s);
//  delete[] s;
  //s = new char[a+1];//CC
  s = (char *) realloc(s,((int)strlen(s)+1));
  return s;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrUp
                                                                          */
/* Aufgabe:
   wandelt Zeichenkette in Grossbuchstaben um
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X char *s: umzuwandelnde Zeichenkette
                                                                          */
/* Ergebnis:
   umgewandelte Zeichenkette
                                                                          */
/* Programmaenderungen:
   03/1994   MSR   Erste Version
                                                                          */
/**************************************************************************/
char *StrUp ( char *s )
{
  int i;
  int l = (int)strlen(s);
  for (i=0; i<l; i++) {
      if (islower((int)s[i]))
          s[i] = (char)toupper((int)s[i]);
  }
  return s;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: StrDown
                                                                          */
/* Aufgabe:
   wandelt Zeichenkette in Kleinbuchstaben um
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   X char *s: umzuwandelnde Zeichenkette
                                                                          */
/* Ergebnis:
   umgewandelte Zeichenkette
                                                                          */
/* Programmaenderungen:
   12/1994   MSR    Erste Version
                                                                          */
/**************************************************************************/
char *StrDown ( char *s )
{
  int i;
  int l = (int)strlen(s);
  for (i=0; i<l; i++) {
      if (isupper((int)s[i]))
          s[i] = (char)tolower((int)s[i]);
  }
  return s;
}


void GetRFINodesData ()
{
}

/**************************************************************************/
/* ROCKFLOW - Funktion: StrOnlyReadStr
                                                                          */
/* Aufgabe:
   Liest Zeichenkette aus String aber schreibt Protokoll in Datei nicht
   (for Phreeqc read function)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R char **x: Adresse der gelesenen Zeichenkett, es muss vorher Speicher
               allokiert werden
   E char *s: Zeichenkette, aus der gelesen werden soll
   E FILE *f: Dateizeiger fuer Protokolldatei
   E FctTestString func: Funktionszeiger auf die Funktion, die den
                         eingelesenen Wert auf Gueltigkeit testet
   R int *pos: Anzahl der bisher gelesenen Zeichen
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/2003     MX        Erste Version
                                                                          */
/**************************************************************************/
int StrOnlyReadStr ( char *x, char *s, FILE *f, FctTestString func, int *pos ) /*MX*/
{
  int test;

  x[0] = '\0';
  if (sscanf(s," %s%n",x,pos)<=0) {
      *pos = 0;  /* nichts sinnvolles gelesen */
      return 0;
  }
  else {
      test = func(x,f);
      return test;
  }
}

/**************************************************************************
STRLib-Method: get_sub_string
Task: sub_string between pos1 and delimiter
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
string get_sub_string(string buffer,string delimiter,int pos1,int *pos2)
{
  int pos=0;
  string empty_string("");
  //string sub_string_this;
  *pos2 = (int)buffer.find(delimiter,pos1);
  if(*pos2<0)
    return empty_string;
  while(*pos2<=pos1) {
    pos1++;
    *pos2 = (int)buffer.find(delimiter,pos1);
    if(*pos2<0) {
      *pos2 = (int)buffer.size();
      break;
    }
    if(pos1>=(int)buffer.size()) 
      break;
  }
  string sub_string_this = buffer.substr(pos1,*pos2);
  while (pos>=0) {
    pos = (int)sub_string_this.find_first_of(" ");
    if (pos<0) break;
    sub_string_this.erase(pos,1);
  }
  return sub_string_this;
}

/**************************************************************************
STRLib-Method: 
Task: 
Programing:
03/2004 OK Implementation
last modification:
**************************************************************************/
void remove_white_space(string *buffer)
{
  int pos=0;
  while (pos>=0) {
    pos = (int)buffer->find_first_of(" ");
    if (pos<0) break;
    buffer->erase(pos,1);
  }
}


/**************************************************************************
STRLib-Method: get_sub_string2
Task: 
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
string get_sub_string2(string buffer,string delimiter,string *tmp)
{
  int pos2 = (int)buffer.find_first_of(delimiter);
  string sub_string = buffer.substr(0,pos2);
  *tmp = buffer.substr(pos2+delimiter.size());
  return sub_string;
}

//cut_string = line_string.substr(line_string.find_first_of(delimiter)+1); //schneidet delimiter ab
//pos2 = cut_string.find_first_of(delimiter);

/**************************************************************************
STRLib-Method: SubKeyword
Task: 
Programing:
09/2004 OK Implementation
last modification:
**************************************************************************/
bool SubKeyword(string line)
{
  if(line.find(SUBKEYWORD)!=string::npos)
    return true;
  else 
    return false;
}

/**************************************************************************
STRLib-Method: SubKeyword
Task: 
Programing:
09/2004 OK Implementation
last modification:
**************************************************************************/
bool Keyword(string line)
{
  if(line.find(KEYWORD)!=string::npos)
    return true;
  else 
    return false;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: GetLineFromFile1
                                      */
/* Aufgabe:
   Liest aus dem Eingabefile *ein die nächste Zeile
   Fängt die Zeile mit ";" an oder ist sie leer, wird sie ausgelassen
   Rückgabe ist ist ein string mit dem Zeileninhalt ab dem ersten Nicht-Leerzeichen 
   bis zum ersten Auftreten des Kommentartzeichens ";"
                                            */
/* Programmaenderungen:
    05/2004     SB  First Version
                                   */
/*  09/2005     CC move from fem to geo 
 **************************************************************************/
string GetLineFromFile1(ifstream *ein){
 
  string line, zeile = "";
  int fertig=0, i, j;
  char zeile1[MAX_ZEILEN];
  line =""; //WW
  //----------------------------------------------------------------------
  while(fertig<1){
    if(ein->getline(zeile1,MAX_ZEILEN)){				//Zeile lesen
	  line = zeile1;							//character in string umwandeln
	  i = (int) line.find_first_not_of(" ",0);		//Anfängliche Leerzeichen überlesen, i=Position des ersten Nichtleerzeichens im string
	  j = (int) line.find(";",i) ;					//Nach Kommentarzeichen ; suchen. j = Position des Kommentarzeichens, j=-1 wenn es keines gibt.
	  if(j!=i)fertig = 1;						//Wenn das erste nicht-leerzeichen ein Kommentarzeichen ist, zeile überlesen. Sonst ist das eine Datenzeile
	  if((i != -1))
		zeile = line.substr(i,j-i);   //Ab erstem nicht-Leerzeichen bis Kommentarzeichen rauskopieren in neuen substring, falls Zeile nicht leer ist
    }
    else{//end of file found
       fertig=1;
    }
  }// end while(...)
  //----------------------------------------------------------------------
  return zeile;
}

/**************************************************************************
STRLib-Method: 
Task: 
Programing:
09/2005 OK/CC Implementation
last modification:
**************************************************************************/
void is_line_empty(string *buffer)
{
  int pos=0;
  while (pos>=0) {
    pos = (int)buffer->find_first_of(" ");
    if (pos<0) break;
    buffer->erase(pos,1);
  }
}/**************************************************************************
STRLib-Method: 
Programing:
10/2005 OK Implementation
**************************************************************************/
/*string NumberToString(long i)
{

  char long_char[10];
  sprintf(long_char,"%ld",i);
  string long_str = long_char;
  return long_str;
}*/

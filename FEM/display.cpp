/**************************************************************************/
/* ROCKFLOW - Modul: display.c
                                                                          */
/* Aufgabe:
   Enthaelt alle Funktionen fuer Standard Ein- und Ausgabe (Bildschirm,
   Tastatur)
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   10/1999     AH         Warnung entfernt
   01/2002     MK         Umleitung der DisplayX-Funktionen in MSG-Datei 
                          Ausnahmen: DisplayStartMsg/DisplayEndMsg                                                  */
/**************************************************************************/

#include "stdafx.h" /* MFC */


/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "display.h"
#include "files.h"
  /* wird durch makros.h schon importiert */

/* Interne (statische) Deklarationen */


/* Definitionen */

/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayStartMsg
                                                                          */
/* Aufgabe:
   Gibt Eroeffnungsbildschirm aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void DisplayStartMsg ( void )
{
  printf("\n");
  printf("          ###################################################\n");
  printf("          ##                                               ##\n");
  printf("          ##               GEOSYS - ROCKFLOW               ##\n");
  printf("          ##                                               ##\n");
  printf("          ##  Helmholtz Center for Environmental Research  ##\n");
  printf("          ##    UFZ Leipzig - Environmental Informatics    ##\n");
  printf("          ##                  TU Dresden                   ##\n");
  printf("          ##              University of Kiel               ##\n");
  printf("          ##            University of Edinburgh            ##\n");
  printf("          ##         University of Tuebingen (ZAG)         ##\n");
  printf("          ##       Federal Institute for Geosciences       ##\n");
  printf("          ##          and Natural Resources (BGR)          ##\n");
  printf("          ##                                               ##\n");
  printf("          ##      Version %s  Date %s     ##\n",\
                                      ROCKFLOW_VERSION,ROCKFLOW_DATE);
  printf("          ##                                               ##\n");
  printf("          ###################################################\n");
  printf("\n          File name (without extension): ");
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayEndMsg
                                                                          */
/* Aufgabe:
   Gibt Programm-Abspann aus.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void DisplayEndMsg ( void )
{
  printf("\n          Programm beendet!\n\n\n");
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayMsg
                                                                          */
/* Aufgabe:
   Schreibt Zeichenkette ohne Zeilenvorschub auf Standardausgabe
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayMsg ( char *s )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"%s",s);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayMsgLn
                                                                          */
/* Aufgabe:
   Schreibt Zeichenkette mit Zeilenvorschub auf Standardausgabe,
   beginnt immer erst nach 12 Zeichen Einrueckung.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayMsgLn ( char *s )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"%s\n            ",s);
  fflush(stdout);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayMsgCR
                                                                          */
/* Aufgabe:
   Schreibt Zeichenkette mit Zeilenruecklauf auf Standardausgabe,
   beginnt immer erst nach 12 Zeichen Einrueckung.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayMsgCR ( char *s )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"%s\r            ",s);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayDouble
                                                                          */
/* Aufgabe:
   Schreibt double-Wert ohne Zeilenvorschub formatiert auf
   Standardausgabe. Wird fuer beide Formatangaben 0 angegeben,
   wird im Standardformat geschrieben.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double x: double-Wert
   E int i: Gesamtstellenzahl
   E int j: Nachkommastellen
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   12/1995     cb         E-Format
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayDouble ( double x, int i, int j )
{
  FILE *f;
  f=OpenMsgFile();
  if ((i==0) && (j==0))
      /* printf("%f",x); */
    fprintf(f,"%g",x);
  else
    fprintf(f,"% *.*g",i,j,x);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayLong
                                                                          */
/* Aufgabe:
   Schreibt long-Wert ohne Zeilenvorschub im Standardformat auf
   Standardausgabe.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long x: long-Wert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayLong ( long x )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"%ld",x);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayDoubleVector
                                                                          */
/* Aufgabe:
   Schreibt Vektor auf Standardausgabe
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *vec: Zeiger auf Vektor
   E long grad: Laenge des Vektors
   E char *text: Ueberschrift ueber Ausgabe
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1994     Hans Herrmann    Erste Version
   12/1994     MSR              Von mathlib nach display portiert
   01/2002     MK               Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayDoubleVector ( double *vec, long grad, char *text )
{
  FILE *f;
  long i;
  DisplayMsgLn("");
  DisplayMsgLn(text);
  f=OpenMsgFile();
  for (i=0;i<grad;i++) {
      fprintf(f,"| %+e |",vec[i]);
      fprintf(f,"%s\n            ","");
      fflush(stdout);
  }
  CloseMsgFile(f);
  DisplayMsgLn("");
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayDoubleMatrix
                                                                          */
/* Aufgabe:
   Schreibt Matrix auf Standardausgabe
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *mat: Zeiger auf Matrix
   E long m: 1. Dimension der Matrix
   E long n: 2. Dimension der Matrix
   E char *text: Ueberschrift ueber Ausgabe
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1994     Hans Herrmann    Erste Version
   12/1994     MSR              Von mathlib nach display portiert
   01/2002     MK         Umleitung in MSG-Datei 
                                                                         */
/**************************************************************************/
void DisplayDoubleMatrix ( double *mat, long m, long n, char *text )
{
  /* //WW
  FILE *f;
  long i, k;
  DisplayMsgLn("");
  DisplayMsgLn(text);
  if(n<=6) {
      f=OpenMsgFile();
      for (k=0;k<m;k++) {
          fprintf(f,"%s","|");
          for(i=0;i<n;i++) {
              fprintf(f,"%+10.3e",mat[i+k*n]);
              if (i!=(n-1))
                  fprintf(f,"%s"," ");
          }
          fprintf(f,"%s\n            ","|");
          fflush(stdout);
      }
      CloseMsgFile(f);
      DisplayMsgLn("");
  }
  else {
      double *zwila = NULL;
      char raus[100];
      zwila = (double *) Malloc(n*sizeof(double));
      DisplayMsg("(Darstellung erfolgt in ");
      DisplayLong(m);
      DisplayMsg(" Spaltenvektoren)");
      for (i=0;i<m;i++) {
          for(k=0;k<n;k++)
              zwila[k]=mat[i+k*m];
          sprintf(raus,"%ld.Spalte (%s)",(i+1l),text);
          raus[(int)strlen(raus)]='\0';
         // sprintf(raus,"%ld.Spalte (%s)\0",(i+1l),text);
  //  \0 gibt eine warning, ist aber sonst zu umstaendlich 
          DisplayDoubleVector(zwila,n,raus);
      }
      zwila = (double *) Free(zwila);
  }
*/
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayErrorMsg
                                                                          */
/* Aufgabe:
   Schreibt Fehlermeldung auf Standardausgabe.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette (Fehlermeldung)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayErrorMsg ( char *s )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"\n!!!!!!!!  %s\n\n            ",s);
  CloseMsgFile(f);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayTimeMsg
                                                                          */
/* Aufgabe:
   Schreibt Laufzeitmeldung auf Standardausgabe.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *s: Zeichenkette (Fehlermeldung)
   E double d: Zeitwert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1994     MSR        Erste Version
   01/2002     MK         Umleitung in MSG-Datei 
                                                                          */
/**************************************************************************/
void DisplayTimeMsg ( char *s, double d )
{
  FILE *f;
  f=OpenMsgFile();
  fprintf(f,"\n            %s%20ld s\n\n            ",s,((long) d));
  CloseMsgFile(f);
}



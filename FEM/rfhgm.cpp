/**************************************************************************/
/* ROCKFLOW - Modul: rfp3d.c
                                                                          */
/* Aufgabe: Interface RF zu HGM
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Tools */
#include "geo_strings.h"

/* Interface */
#include "rfhgm.h"


/* Definitionen */



/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMPoint
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Punkte.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMPoint ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long nr;
    double x,y,z;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {
        p+=pos;
        /* Schluesselwort schreiben */
        FilePrintString(f,"#POINT");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #POINT");
#endif

        /* Schleife ueber alle Punkt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p])) {
          ok = (StrReadLong(&nr,&data[p],f,TFLong,&pos) && ok);
          ok = (StrReadDouble(&x,&data[p+=pos],f,TFDouble,&pos) && ok);
          ok = (StrReadDouble(&y,&data[p+=pos],f,TFDouble,&pos) && ok);
          ok = (StrReadDouble(&z,&data[p+=pos],f,TFDouble,&pos) && ok);
          p+=pos;
          LineFeed(f);
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMEdge
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Kanten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMEdge ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long nr,l1,l2;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {
        p+=pos;

        /* Schluesselwort schreiben */
        FilePrintString(f,"#EDGE");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #EDGE");
#endif

        /* Schleife ueber alle Punkt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p])) {
          ok = (StrReadLong(&nr,&data[p],f,TFLong,&pos) && ok);
          ok = (StrReadLong(&l1,&data[p+=pos],f,TFLong,&pos) && ok);
          ok = (StrReadLong(&l2,&data[p+=pos],f,TFLong,&pos) && ok);
          p+=pos;
          LineFeed(f);
        }
    }
    return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMPlain
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Kanten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMPlain ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {

        /* Schluesselwort schreiben */
        FilePrintString(f,"#PLAIN");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #PLAIN");
#endif

        /* Schleife ueber alle Punkt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p+=pos])) {
          LineFeed(f);
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMPolygon
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Polygone.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMPolygon ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {

        /* Schluesselwort schreiben */
        FilePrintString(f,"#POLYGON");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #POLYGON");
#endif

        /* Schleife ueber alle Punkt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p+=pos])) {
          LineFeed(f);
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMPolyline
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Polygone.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMPolyline ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {

        /* Schluesselwort schreiben */
        FilePrintString(f,"#POLYLINE");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #POLYLINE");
#endif

        /* Schleife ueber alle Punkt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p+=pos])) {
          LineFeed(f);
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMIntersection
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Polygone.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMIntersection ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {

        /* Schluesselwort schreiben */
        FilePrintString(f,"#INTERSECTION");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort #INTERSECTION");
#endif

        /* Schleife ueber alle Schnitt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p+=pos])) {
          LineFeed(f);
        }
    }
    return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMDelaunay
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Delaunay-Netzgenerator.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMDelaunay ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    int quad,laplace;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {
        p+=pos;

        /* Schluesselwort schreiben */
        FilePrintString(f,"#DELAUNAY");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort DELAUNAY");
#endif

        /* Schleife ueber alle Schnitt-Objekte der selben Gruppe */
        while (StrTestInt(&data[p])) {
          ok = (StrReadInt(&quad,&data[p],f,TFInt,&pos) && ok);
          ok = (StrReadInt(&laplace,&data[p+=pos],f,TFInt,&pos) && ok);
          p+=pos;
          LineFeed(f);
        }
    }
    return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctHGMAdvancingFront
                                                                          */
/* Aufgabe:
   Lesefunktion fuer Advancing-Front-Netzgenerator.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found : Schluesselwort gefunden: 1, sonst 0
   E FILE *f   : Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int FctHGMAdvancingFront ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;

#ifdef TESTFILES
    char protocol[80];
#endif

    if(!found) {
      return 1;
    }

    /* Schleife ueber alle Gruppen */
    while (StrTestHash(&data[p],&pos)) {

        /* Schluesselwort schreiben */
        FilePrintString(f,"ADVANCING_FRONT");
        LineFeed(f);

#ifdef TESTFILES
        DisplayMsgLn("Eingabedatenbank: Schluesselwort ADVANCING_FRONT");
#endif

        /* Schleife ueber alle Schnitt-Objekte der selben Gruppe */
        while (StrTestLong(&data[p+=pos])) {
          LineFeed(f);
        }
    }
    return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: StartHGMMeshGenerator
                                                                          */
/* Aufgabe:
   HGM-Netzgenrator starten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *mgprot: Dateizeiger auf geoeffnete Protokoll-Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int StartHGMMeshGenerator ( FILE *mgprot )
{
  if (!mgprot) return 0;

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteHGMMeshGenerator
                                                                          */
/* Aufgabe:
   HGM-Netzgenrator ausfuehren und  RFI-Datei erzeugen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *mgdat : Dateiname der MG-Eingabedatei incl. Extension
   E FILE *mgprot: Dateizeiger auf geoeffnete Protokoll-Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int ExecuteHGMMeshGenerator ( char *mgdat, FILE *mgprot )
{
  FILE *f = fopen(mgdat,"w");
  long n=5,e=4;

  if (f==NULL || !mgprot) return 0;

/* Ausgabe in rfi-Datei */
/* Anzahl Knoten, Anzahl Elemente */
  fprintf(f,"#0#0#0#1#0.0#0#################################################################");
  fprintf(f,"\n 0  %ld  %ld",n,e);

/* Knotengeometrie */
  fprintf(f,"\n 0   0.000000  0.000000  0.000000");
  fprintf(f,"\n 1  25.000000  0.000000  0.000000");
  fprintf(f,"\n 2  50.000000  0.000000  0.000000");
  fprintf(f,"\n 3  75.000000  0.000000  0.000000");
  fprintf(f,"\n 4 100.000000  0.000000  0.000000");

/* Elementtopologie */
  fprintf(f,"\n 0  0  line  0  1");
  fprintf(f,"\n 1  0  line  1  2");
  fprintf(f,"\n 2  0  line  2  3");
  fprintf(f,"\n 3  0  line  3  4");

  fflush(f);

  return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: EndHGMMeshGenerator
                                                                          */
/* Aufgabe:
   HGM-Netzgenrator beenden.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *mgprot: Dateizeiger auf geoeffnete Protokoll-Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   07/2000    TR      Erste Version
                                                                          */
/**************************************************************************/
int EndHGMMeshGenerator ( FILE *mgprot )
{
  if (!mgprot) return 0;

  return 1;
}

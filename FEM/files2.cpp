/**************************************************************************/
/* ROCKFLOW - Modul: files2.c
                                                                          */
/* Aufgabe:
   Enthaelt die Datei- Ein- und Ausgaberoutinen,
   die RFI Geometrie-Daten (UCD) betreffen.
                                                                          */
/* Programmaenderungen:
   09/1996   RK   Erste Version
   11/1998   AH   Einfuehrung von Startnummmer im Startnetz
                  fuer Knoten und Elementen
   12/1998   AH   Trennung der Materialien von Elementeigenschaften
   02/1999   CT   Bugfix: Anpassung fuer mit 0 beginnende Elemente.
                          Kennung fuer Version im Kopf. Versionsabhaengiges lesen.
   02/1999   CT   Korrekturen fuer herausgeloeste Materialgruppen
   03/1999   AH   Bugfix: Die Elementstartnummmer muss gleich gemerkt werden.
   10/1999   AH   Systemzeit fuer Geometrie
   07/2000   OK   RFI-Lesefunktion fuer Dreiecke erweitert
   11/2000   OK   RFI-Lesefunktion fuer Dreiecksprismen und Tetraeder erweitert
                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
/* Header / Andere intern benutzte Module */
#include "files.h"
#include "geo_strings.h"
#include "nodes.h"
#include "elements.h"
#include "rfsystim.h"
#include "rf_pcs.h" //SB:namepatch
#include "rf_pcs.h" //OK_MOD"
#include <math.h>
// MSHLib
#include "msh_lib.h"

#include "msh_elements_rfi.h"
/* Interne (statische) Deklarationen */
static long offset_number;  /* Falls die Elementnummern nicht mit 0 anfangen, wird ein Offset benutzt */
/**************************************************************************/
/* ROCKFLOW - Funktion: GeomReadRFInputFileASCII
                                                                          */
/* Aufgabe:
   Liest Geometriedaten

   Es waeren noch mehr Tests moeglich, aber die rf-Datei sollte ja nicht
   von Hand erzeugt werden, so dass (vorerst) darauf verzichtet wird.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Eingabedatei
   E FILE *prot: Dateizeiger auf geoeffnete Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   09/1996   RK   Erste Version
   10/1996   MSR  Fehlerabfragen eingebaut, ueberarbeitet
   09/1997   RK   Elementnummern koennen jetzt auch bei 0 beginnen
   02/1999   CT   Anpassung fuer mit 0 beginnende Elemente. Kennung
                  fuer Version im Kopf. Versionsabhaengiges lesen.
   02/1999   CT   Korrekturen fuer herausgeloeste Materialgruppen
   10/1999   AH   Systemzeit fuer Geometrie
   07/2000   OK   RFI-Lesefunktion fuer Dreiecke erweitert
   11/2000   OK   RFI-Lesefunktion fuer Dreiecksprismen und Tetraeder erweitert
                                                                          */
/**************************************************************************/
int GeomReadRFInputFileASCII ( FILE *f, FILE *prot )
{
  char * s;  /* gelesene Zeile */
  long *knoten = NULL;  /* Feld mit Knotennummern */
  int d, anz_n, anz_e;  /* Variablen fuer Kopfzeile */
  int p = 0;
  int k;
  int pos = 0;
  int etyp = 0;  /* Elementtyp */
  char etyps[6];  /* Elementtyp als string */
  int i, j;  /* Laufvariablen */
  long nr;  /* Knoten- bzw. Elementnummer */
  long mat;  /* Materialnummer */
  long vorgaenger;  /* Vorgaenger des Elements bei Adaption */
  Knoten *kno = NULL;  /* neuer Knoten */
  long start_number;   /* Elementnummer/Knotennummer im Startnetz */
  long group_number;   /* Elementgruppennummer */
  long geo_input_id_timer;

  /* Systemzeit fuer Rockflow setzen */
  SetSystemTime("RF-GEOINP","ROCKFLOW","ROCKFLOW: Geometry read time",&geo_input_id_timer);
  RunSystemTime("RF-GEOINP");

  k = 0; /* erste Elementnummer ist groesser als die groesste Materialnummer */
  s = (char *) Malloc(MAX_ZEILE);
  /* Kopfzeile lesen */
  if (fgets(s,MAX_ZEILE,f)==NULL) {
      FilePrintString(prot,"* !!! Fehler: Geom-Kopf-Zeile nicht gelesen; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  if (sscanf(s," %d %d %d",&d,&anz_n,&anz_e)!=3) {
      FilePrintString(prot,"* !!! Fehler: Geom-Kopf-Daten unvollstaendig; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* Test, ob richtiger Abschnitt */
  if (d) {  /* d==1: Daten; d==0: Geometrie */
      FilePrintString(prot,"* !!! Fehler: Falsche Daten-Kennung (Geom); Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* Knoten lesen */
  for (i=0;i<anz_n;i++) {
      if (fgets(s,MAX_ZEILE,f)==NULL) {
          FilePrintString(prot,"* !!! Fehler: Knoten-Zeile nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      kno = (Knoten *)CreateNodeGeometry(); /* OK rf3240 */
      /* Knotennummer und -koordinaten lesen */
      if (sscanf(s," %ld %lf %lf %lf",&nr,&(kno->x),&(kno->y),&(kno->z))!=4) {
          FilePrintString(prot,"* !!! Fehler: Knotennummer/-koor nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          kno = (Knoten *)Free(kno);
          s = (char *)Free(s);
          return 0;
      }
      start_number=nr;
      /* ist Knotennummer o.k. ??? */
      if ((nr<0l) || (GetNode(nr)!=NULL)) {
          FilePrintString(prot,"* !!! Fehler: ungueltige oder doppelte Knotennummer: ");
          FilePrintLong(prot,nr);
          FilePrintString(prot,"; Einlesen beendet !");
          LineFeed(prot);
          kno = (Knoten *)Free(kno);
          s = (char *)Free(s);
          return 0;
      }
      /* Knoten setzen */
      PlaceNode(kno,nr);
      SetNodeStartNumber(nr,start_number);
  }


  /* Elemente lesen */
  for (i=0;i<anz_e;i++) {
      p = pos = 0;
      vorgaenger = -1;
      if (fgets(s,MAX_ZEILE,f)==NULL) {
          FilePrintString(prot,"* !!! Fehler: Geom-Element-Zeile nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      /* Elementnummer und -typ etc. lesen */
      

      if (rfi_filetype < 3118) {
          if (sscanf(s," %ld %ld %5s%n",&nr,&mat,etyps,&pos)!=3) {
              FilePrintString(prot,"* !!! Fehler: Elementnummer/-mat/-typ nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              s = (char *)Free(s);
              return 0;
          }
      } else {
          if (sscanf(s," %ld %ld %ld %5s%n",&nr,&mat,&vorgaenger,etyps,&pos)!=4) {
              FilePrintString(prot,"* !!! Fehler: Elementnummer/-mat/-typ nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              s = (char *)Free(s);
              return 0;
          }
      } /* endif */
      /* Gruppennummer und Statnummer */
      group_number=mat;
      start_number=nr;

      /* Falls die Elementnummern nicht mit 0 beginnen, wird ein Offset
         abgezogen */
      if (i==0) {offset_number=nr;}

      nr -= offset_number;
      vorgaenger -= offset_number;
      if (vorgaenger < 0) vorgaenger =-1;

      /* ist Elementnummer o.k. ??? */
      if (ElGetElement(nr)!=NULL) {
          FilePrintString(prot,"* !!! Fehler: Doppelte Elementnummer; Einlesen beendet !");
          fprintf(prot," %s%d ","* in Element: ",i);
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }

      /* Elementtyp auswerten, Nummern weiterhin zulaessig */
      if ((strcmp(etyps,"line")==0) || (strcmp(etyps,"1")==0))
          etyp = 1;
      else if ((strcmp(etyps,"quad")==0) || (strcmp(etyps,"2")==0))
          etyp = 2;
      else if ((strcmp(etyps,"hex")==0) || (strcmp(etyps,"3")==0))
          etyp = 3;
      else if ((strcmp(etyps,"tri")==0) || (strcmp(etyps,"4")==0)) /* OK rf3419 */
          etyp = 4;
      else if ((strcmp(etyps,"tet")==0) || (strcmp(etyps,"5")==0)) /* OK rf3424 */
          etyp = 5;
      else if ((strcmp(etyps,"pris")==0) || (strcmp(etyps,"6")==0)) /* OK rf3424 */
          etyp = 6;
      else {
          FilePrintString(prot,"* !!! Fehler: Unbekannter Elementtyp; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }

      /* Element erzeugen */
if(GetRFControlModel()==-1)
      ELECreateTopology(etyp,vorgaenger,0,nr);
else
      CreateElementTopology(etyp,vorgaenger,0,nr); /* OK rf3240 */
      ElSetElementStartNumber(nr,start_number);
      ElSetElementGroupNumber(nr,group_number);
      switch (etyp) {
          case 1: anz_1D++; msh_no_line++; break;
          case 2: anz_2D++; msh_no_quad++; break;
          case 3: anz_3D++; msh_no_hexs++; break;
          case 4: anz_2D++; msh_no_tris++; break; /* OK rf3419 */
          case 5: anz_3D++; msh_no_tets++; break; /* OK rf3424 */
          case 6: anz_3D++; msh_no_pris++; break; /* OK rf3424 */
      }
      /* Elementknoten lesen */
      etyp--;
      knoten = (long *) Malloc(sizeof(long)*ElNumberOfNodes[etyp]);
      for (j=0l;j<ElNumberOfNodes[etyp];j++) {
          if (sscanf(&s[p+=pos]," %ld%n",&(knoten[j]),&pos)!=1) {
              FilePrintString(prot,"* !!! Fehler: El-Knotennummer nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              knoten = (long *)Free(knoten);
              s = (char *)Free(s);
              return 0;
          }
          if (GetNode(knoten[j])==NULL) {
              FilePrintString(prot,"* !!! Fehler: El-Knotennummer existiert nicht; Einlesen beendet !");
              LineFeed(prot);
              knoten = (long *)Free(knoten);
              s = (char *)Free(s);
              return 0;
          }
      }
      /* Elementknoten eintragen */
      ElSetElementNodes(nr,knoten);
  }
  s = (char *)Free(s);

  StopSystemTime("RF-GEOINP");
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ResReadRFInputFileASCII
                                                                          */
/* Aufgabe:
   Liest Knoten- und Elementdaten

   Es waeren noch mehr Tests moeglich, aber die rf-Datei sollte ja nicht
   von Hand erzeugt werden, so dass (vorerst) darauf verzichtet wird.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete UCD-Eingabedatei
   E FILE *prot: Dateizeiger auf geoeffnete Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/1996     MSR         Erste Version
                                                                          */
/**************************************************************************/
int ResReadRFInputFileASCII ( FILE *f, FILE *prot )
{
  char name[31];  /* Daten-Bezeichner */
  char * s;  /* gelesene Zeile */
  double *daten = NULL;  /* Feld mit Daten */
  int d, anz_n, anz_e;  /* Variablen fuer Kopfzeile */
  int p = 0;
  int pos = 0;
  int anz;  /* Anzahl der Datenfelder */
  int *dgroessen = NULL;  /* Feld mit anz Datengroessen */
  int i, j, k;  /* Laufvariablen */
  long nr;  /* Knoten- bzw. Elementnummer */
  s = (char *) Malloc(MAX_ZEILE);
  /* muss ueberhaupt gelesen werden ??? */
  if (!(danz_n+danz_e)) {
      s = (char *)Free(s);
      return 1;
  }
  /* Kopfzeile lesen */
  if (fgets(s,MAX_ZEILE,f)==NULL) {
      FilePrintString(prot,"* !!! Fehler: Kopf-Zeile nicht gelesen; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  if (sscanf(s," %d %d %d",&d,&anz_n,&anz_e)!=3) {
      FilePrintString(prot,"* !!! Fehler: Kopf-Daten unvollstaendig; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* Test, ob richtiger Abschnitt */
  if (!d) {  /* d==1: Daten; d==0: Geometrie */
      FilePrintString(prot,"* !!! Fehler: Falsche Daten-Kennung; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* sind genuegend Knotendaten vorhanden ??? */
  if (danz_sum_n>anz_n) {
      FilePrintString(prot,"* !!! Fehler: Nicht genuegend Knotendaten vorhanden; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* Knotendaten bearbeiten */
  if (anz_n) {
      p = pos = 0;
      /* Knotendaten-Indizes bestimmen */
      if (fgets(s,MAX_ZEILE,f)==NULL) {
          FilePrintString(prot,"* !!! Fehler: Knotendaten-Zeile nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      if (sscanf(s," %d%n",&anz,&pos)!=1) {
          FilePrintString(prot,"* !!! Fehler: Knotendaten-Anzahl nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      if (danz_n>anz) {
          FilePrintString(prot,"* !!! Fehler: Nicht genuegend Knotendatenfelder vorhanden; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      /* Feldgroessen lesen */
      dgroessen = (int *) Malloc(anz*sizeof(int));
      for (i=0;i<anz;i++) {
          if (sscanf(&s[p+=pos]," %d%n",&(dgroessen[i]),&pos)!=1) {
              FilePrintString(prot,"* !!! Fehler: Feldgroesse nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              dgroessen = (int *)Free(dgroessen);
              s = (char *)Free(s);
              return 0;
          }
          /* evtl. Test: sind dgroessen[i] positiv ??? */
      }
      /* Name und Einheit lesen */
      for (i=0;i<anz;i++) {
          if (fgets(s,MAX_ZEILE,f)==NULL) {
              FilePrintString(prot,"* !!! Fehler: Einheiten-Zeile nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              dgroessen = (int *)Free(dgroessen);
              s = (char *)Free(s);
              return 0;
          }
          sscanf(s,"%[^,]",name);
          j = 0;
          while ((j<danz_n) && (strcmp(name,datafield_n[j].name)!=0))
              j++;
          if (j<danz_n) {  /* gefunden */
              datafield_n[j].idx_file = i;
              if (datafield_n[j].transfer!=dgroessen[i]) {
                  FilePrintString(prot,"* !!! Fehler: Feldgroesse von ");
                  FilePrintString(prot,name);
                  FilePrintString(prot," stimmt nicht; Einlesen beendet !");
                  LineFeed(prot);
                  dgroessen = (int *)Free(dgroessen);
                  s = (char *)Free(s);
                  return 0;
              }
          }
      }
      /* sind alle Daten vorhanden ??? */
      dgroessen = (int *)Free(dgroessen);
      for (i=0;i<danz_n;i++) {
          if (datafield_n[i].idx_file<0) {
              FilePrintString(prot,"* !!! Fehler: Datum ");
              FilePrintString(prot,datafield_n[i].name);
              FilePrintString(prot," fehlt; Einlesen beendet !");
              LineFeed(prot);
              s = (char *)Free(s);
              return 0;
          }
      }
      /* Knotendaten lesen */
      daten = (double *) Malloc(anz_n*sizeof(double));
      for (i=0;i<NodeListLength;i++) {
          p = pos = 0;
          if (fgets(s,MAX_ZEILE,f)==NULL) {
              FilePrintString(prot,"* !!! Fehler: Knotendaten-Zeile nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }
          /* Knotennummer lesen */
          if (sscanf(s," %ld%n",&nr,&pos)!=1) {
              FilePrintString(prot,"* !!! Fehler: Knotennummer nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }
          /* existiert Knotennummer ??? */
          if ((nr<0l) || (GetNode(nr)==NULL)) {
              FilePrintString(prot,"* !!! Fehler: Knoten existiert nicht; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }
          /* Knotenwerte lesen */
          for (j=0;j<anz_n;j++) {
              if (sscanf(&s[p+=pos]," %lf%n",&(daten[j]),&pos)!=1) {
                  FilePrintString(prot,"* !!! Fehler: Knotenwert nicht gelesen; Einlesen beendet !");
                  LineFeed(prot);
                  daten = (double *)Free(daten);
                  s = (char *)Free(s);
                  return 0;
              }
          }
          /* Daten uebertragen */
          for (j=0;j<danz_n;j++)
              for (k=0;k<datafield_n[j].transfer;k++)
                  SetNodeVal(nr,k+datafield_n[j].idx_program,daten[k+datafield_n[j].idx_file]);
      }
      daten = (double *)Free(daten);
  }
  /* sind genuegend Elementdaten vorhanden ??? */
  if (danz_sum_e>anz_e) {
      FilePrintString(prot,"* !!! Fehler: Nicht genuegend Elementdaten vorhanden; Einlesen beendet !");
      LineFeed(prot);
      s = (char *)Free(s);
      return 0;
  }
  /* Elementdaten bearbeiten */
  if (anz_e) {
      p = pos = 0;
      /* Elementdaten-Indizes bestimmen */
      if (fgets(s,MAX_ZEILE,f)==NULL) {
          FilePrintString(prot,"* !!! Fehler: Elementdaten-Zeile nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      if (sscanf(s," %d%n",&anz,&pos)!=1) {
          FilePrintString(prot,"* !!! Fehler: Elementdaten-Anzahl nicht gelesen; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      if (danz_e>anz) {
          FilePrintString(prot,"* !!! Fehler: Nicht genuegend Elementdatenfelder vorhanden; Einlesen beendet !");
          LineFeed(prot);
          s = (char *)Free(s);
          return 0;
      }
      /* Feldgroessen lesen */
      dgroessen = (int *) Malloc(anz*sizeof(int));
      for (i=0;i<anz;i++) {
          if (sscanf(&s[p+=pos]," %d%n",&(dgroessen[i]),&pos)!=1) {
              FilePrintString(prot,"* !!! Fehler: Feldgroesse nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              dgroessen = (int *)Free(dgroessen);
              s = (char *)Free(s);
              return 0;
          }
          /* evtl. Test: sind dgroessen[i] positiv ??? */
      }
      /* Name und Einheit lesen */
      for (i=0;i<anz;i++) {
          if (fgets(s,MAX_ZEILE,f)==NULL) {
              FilePrintString(prot,"* !!! Fehler: Einheiten-Zeile nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              dgroessen = (int *)Free(dgroessen);
              s = (char *)Free(s);
              return 0;
          }
          sscanf(s,"%[^,]",name);
          j = 0;
          while ((j<danz_e) && (strcmp(name,datafield_e[j].name)!=0))
              j++;
          if (j<danz_e) {  /* gefunden */
              datafield_e[j].idx_file = i;
              if (datafield_e[j].transfer!=dgroessen[i]) {
                  FilePrintString(prot,"* !!! Fehler: Feldgroesse von ");
                  FilePrintString(prot,name);
                  FilePrintString(prot," stimmt nicht; Einlesen beendet !");
                  LineFeed(prot);
                  dgroessen = (int *)Free(dgroessen);
                  s = (char *)Free(s);
                  return 0;
              }
          }
      }
      /* sind alle Daten vorhanden ??? */
      dgroessen = (int *)Free(dgroessen);
      for (i=0;i<danz_e;i++) {
          if (datafield_e[i].idx_file<0) {
              FilePrintString(prot,"* !!! Fehler: Datum ");
              FilePrintString(prot,datafield_e[i].name);
              FilePrintString(prot," fehlt; Einlesen beendet !");
              LineFeed(prot);
              s = (char *)Free(s);
              return 0;
          }
      }
      /* Elementdaten lesen */
      daten = (double *) Malloc(anz_e*sizeof(double));
      for (i=0;i<(ElementListLength);i++) {
          p = pos = 0;
          if (fgets(s,MAX_ZEILE,f)==NULL) {
              FilePrintString(prot,"* !!! Fehler: Elementdaten-Zeile nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }
          /* Elementnummer lesen */
          if (sscanf(s," %ld%n",&nr,&pos)!=1) {
              FilePrintString(prot,"* !!! Fehler: Elementnummer nicht gelesen; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }

          nr -= offset_number; /* Falls die Elementnummern nicht mit 0 anfingen */

          /* existiert Elementnummer ??? */
          if ((nr<0) || (ElGetElement(nr)==NULL)) {
              FilePrintString(prot,"* !!! Fehler: Element existiert nicht; Einlesen beendet !");
              LineFeed(prot);
              daten = (double *)Free(daten);
              s = (char *)Free(s);
              return 0;
          }
          /* Knotenwerte lesen */
          for (j=0;j<anz_e;j++) {
              if (sscanf(&s[p+=pos]," %lf%n",&(daten[j]),&pos)!=1) {
                  FilePrintString(prot,"* !!! Fehler: Elementwert nicht gelesen; Einlesen beendet !");
                  LineFeed(prot);
                  daten = (double *)Free(daten);
                  s = (char *)Free(s);
                  return 0;
              }
          }
          /* Daten uebertragen */
          for (j=0;j<danz_e;j++)
              for (k=0;k<datafield_e[j].transfer;k++)
                  ElSetElementVal(nr,k+datafield_e[j].idx_program,daten[k+datafield_e[j].idx_file]);
      }
      daten = (double *)Free(daten);
  }
  s = (char *)Free(s);
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GeomWriteRFOutputFileASCII
                                                                          */
/* Aufgabe: Schreibt Geometriedaten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   09/1996   R.Kaiser   Erste Version
   10/1996   MSR        korrigiert
   04/2002   OK   etyps[] für neue Elementtypen
  // 02/2004   WW   Show the deformation;
                                                                          */
/**************************************************************************/
int GeomWriteRFOutputFileASCII ( FILE *f )
{
  static long i;
  static int etyp, jj;
  static Knoten *kno;
  static char etyps[6][7] = { "line ", "quad ", "hex ", "tri ", "tet ", "prism " };
  static long *ll;
  static long anz;
  /* Kopfzeile */
  FPL(0); FPL(NodeListLength); FPL(ElementListLength); LF;
  /* Knoten */
  anz = NodeListSize();


  /*
  //Show the deformation
  double x_min = 1.0e10;
  double x_max = 0.0;
  double y_min = 1.0e10;
  double y_max = 0.0;
  // For 3D
  //double z_min = 1.0e10;
  //double z_max = 0.0;
  
  double ux_max = 0.0;
  double uy_max = 0.0;
  //For 3D double uz_max = 0.0;

  static double fact[3];
  //1. The range of the domain and displacement.
  for (i=0l;i<anz;i++)
  {
      if ((kno = GetNode(i))!=NULL)
	  {
		  if(kno->x>x_max) x_max = kno->x;
		  if(kno->x<x_min) x_min = kno->x;
		  if(kno->y>y_max) y_max = kno->y;
		  if(kno->y<y_min) y_min = kno->y;
		  if(fabs(DMGetNodeDisplacementX1(i))>ux_max) 
			   ux_max = fabs(DMGetNodeDisplacementX1(i)); 
		  if(fabs(DMGetNodeDisplacementY1(i))>uy_max) 
			   uy_max = fabs(DMGetNodeDisplacementY1(i)); 
		  //If 3D
		  //if(kno->z>z_max) z_max = kno->z;
		  //if(kno->z<z_min) z_min = kno->z;
		  //if(fabs(DMGetNodeDisplacementZ1(i))>uz_max) 
		  //	   uz_max = fabs(DMGetNodeDisplacementZ1(i)); 
      }
  }
  if(ux_max!=0.0) fact[0] = fabs(x_max-x_min)/ux_max;
  if(uy_max!=0.0) fact[1] = fabs(y_max-y_min)/uy_max;
   //if 3D  if(uz_max!=0.0) fact[2] = fabs(z_max-z_min)/uz_max;
   

  ux_max = 0.0;
  int Dim = 2;
  for (jj=0;jj<Dim;jj++)
     if(fact[jj]>ux_max) ux_max = fact[jj]; 

  ux_max *= 0.01;
  //////////
  */

  for (i=0l;i<anz;i++)
      if ((kno = GetNode(i))!=NULL) {
          FPL(i); 
		  FPD(kno->x); 
		  FPD(kno->y); 
		  FPD(kno->z); 
		  //FPD(kno->x+ux_max*DMGetNodeDisplacementX1(i)); 
		  //FPD(kno->y+ux_max*DMGetNodeDisplacementY1(i)); 
		  //If 3D.  FPD(kno->z+uz_max*DMGetNodeDisplacementZ1(i)); 
		  LF;
      }
  /* Elemente */
  anz = ElListSize();
  for (i=0;i<anz;i++) {
      if (ElGetElement(i)!=NULL) {
          FPL(i);
          FPL(ElGetElementGroupNumber(i));
          FPL(ElGetElementPred(i));
          etyp = ElGetElementType(i);
          etyp--;
          FPS(etyps[etyp]);
          ll = ElGetElementNodes(i);
          for (jj=0l;jj<ElNumberOfNodes[etyp];jj++) {
              FPL(ll[jj]);
          }
          LF;
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ResWriteRFOutputFileASCII
                                                                          */
/* Aufgabe:
   Schreibt Knoten- und Elementdaten in Ausgabedatei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   09/1996     R.Kaiser  Erste Version
   10/1996     MSR       ueberarbeitet
                                                                          */
/**************************************************************************/
int ResWriteRFOutputFileASCII ( FILE *f )
{
  static long anz;
  static long i;  /* Laufvariable */
  static int j, k;  /* Laufvariablen */
  /* Kopf schreiben */
  FPL(1); FPI(danz_sum_n); FPI(danz_sum_e); LF;
  /* Knotendaten bearbeiten */
  if (danz_sum_n) {
      /* Datenkonfiguration schreiben */
      FPI(danz_n);
      for (j=0;j<danz_n;j++)
          FPI(datafield_n[j].transfer);
      LF;
      for (j=0;j<danz_n;j++) {
//SB:namepatch          FPS(datafield_n[j].name);
		  FPS(GetCompNamehelp(datafield_n[j].name)); //SB: namepatch
          FPS(", ");
          FPS(datafield_n[j].einheit);
          LF;
      }
      /* Daten schreiben */
      anz = NodeListSize();
      for (i=0l;i<anz;i++)
          if (GetNode(i)!=NULL) {
              FPL(i);
              for (j=0;j<danz_n;j++)
                  for (k=0;k<datafield_n[j].transfer;k++)
                      FPD(GetNodeVal(i,k+datafield_n[j].idx_program));
              LF;
      }
  }
  /* Elementdaten bearbeiten */
  if (danz_sum_e) {
      /* Datenkonfiguration schreiben */
      FPI(danz_e);
      for (j=0;j<danz_e;j++)
          FPI(datafield_e[j].transfer);
      LF;
      for (j=0;j<danz_e;j++) {
          FPS(datafield_e[j].name);
          FPS(", ");
          FPS(datafield_e[j].einheit);
          LF;
      }
      /* Daten schreiben */
      anz = ElListSize();
      for (i=0;i<anz;i++)
          if (ElGetElement(i)!=NULL) {
              FPL(i);
              for (j=0;j<danz_e;j++)
                  for (k=0;k<datafield_e[j].transfer;k++)
                      FPD(ElGetElementVal(i,k+datafield_e[j].idx_program));
              LF;
      }
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GeomReadRFInputFileBIN
                                                                          */
/* Aufgabe:
   Liest Geometriedaten
   Es wird ohne zwischengeschaltete Dateischnittstelle gelesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Eingabedatei
   E FILE *prot: Dateizeiger auf geoeffnete Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   11/1996     MSR         Erste Version
                                                                          */
/**************************************************************************/
int GeomReadRFInputFileBIN ( FILE *f, FILE *prot )
{

  FilePrintString(f,"* Aufruf von GeomReadRFInputFileBIN - noch nicht implementiert ");
  LineFeed(f);
  FilePrintString(prot,"* Aufruf von GeomReadRFInputFileBIN - noch nicht implementiert ");
  LineFeed(prot);

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ResReadRFInputFileBIN
                                                                          */
/* Aufgabe:
   Liest Knoten- und Elementdaten aus der angegebenen Datei.
   Es wird ohne zwischengeschaltete Dateischnittstelle gelesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete UCD-Eingabedatei
   E FILE *prot: Dateizeiger auf geoeffnete Protokolldatei;
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   11/1996     MSR         Erste Version
                                                                          */
/**************************************************************************/
int ResReadRFInputFileBIN ( FILE *f, FILE *prot )
{

  FilePrintString(f,"* Aufruf von ResReadRFInputFileBIN - noch nicht implementiert ");
  LineFeed(f);
  FilePrintString(prot,"* Aufruf von ResReadRFInputFileBIN - noch nicht implementiert ");
  LineFeed(prot);

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GeomWriteRFOutputFileBIN
                                                                          */
/* Aufgabe:
   Schreibt Geometriedaten
   Es wird ohne zwischengeschaltete Dateischnittstelle gelesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   11/1996     MSR         Erste Version
                                                                          */
/**************************************************************************/
int GeomWriteRFOutputFileBIN ( FILE *f )
{

  FilePrintString(f,"* Aufruf von GeomWriteRFOutputFileBIN - noch nicht implementiert ");
  LineFeed(f);

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ResWriteRFOutputFileBIN
                                                                          */
/* Aufgabe:
   Schreibt Knoten- und Elementdaten
   Es wird ohne zwischengeschaltete Dateischnittstelle gelesen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger auf geoeffnete Ausgabedatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   11/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ResWriteRFOutputFileBIN ( FILE *f )
{

  FilePrintString(f,"* Aufruf von ResWriteRFOutputFileBIN - noch nicht implementiert ");
  LineFeed(f);

  return 1;
}


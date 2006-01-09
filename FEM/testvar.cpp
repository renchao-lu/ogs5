/**************************************************************************/
/* ROCKFLOW - Modul: testvar.c
                                                                          */
/* Aufgabe:
   Enthaelt alle Variablen-Testfunktionen (Definitionen) zum Ueberpruefen
   und evtl. Korrigieren von aus der Eingabedatei eingelesenen Werten.
   Wird von files1.c benutzt.
                                                                          */
/* Beschreibung der Funktionen:
   Die Funktionsnamen setzen sich aus den Buchstaben TF fuer Testfunktion
   und dem mit grossem Anfangsbuchstaben geschriebenen Variablennamen der
   Variablen, die ueberprueft werden soll, zusammen.
   Das Ergebnis ist ein Integer, der bei einem aufgetretenen Fehlerwert,
   der zum Abbruch des Programms fuehren soll, den Wert 0 annimmt;
   ansonsten wird 1 zurueckgeliefert.
   Der erste Parameter ist ein Zeiger entsprechenden Typs auf die zu
   ueberpruefende Variable. Der zweite gibt die Textdatei an, in die bei
   einer Aenderung des Wertes oder einem Fehler eine Protokollnotiz
   geschrieben wird.
                                                                          */
/* Programmaenderungen:
   06/1994     MSR        Erste Version
   07/1996     RK         Rockflow-Anpassung
   10/1996     MSR        Korrektur der Rockflow-Anpassung
   01.07.1997  R.Kaiser   Korrekturen und Aenderungen aus dem aTM
                          uebertragen
   11/1998     O. Kolditz
   02/1998     A. Habbar  Relaxationsfaktor
    3/1998     C. Thorenz Dichtemodell
   12/1998     AH         Mehrkomponentenmodel


   letzte Aenderung:    A.Habbar    11.12.1998

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "testvar.h"
#include "geo_strings.h"


/* Interne (statische) Deklarationen */


/* Definitionen */





/* Testfunktionen zum Schluesselwort ANAV */

int TFAnav ( int *x, FILE *f )
{
  /* wenn anav nicht zwischen 0 und 2, auf 0 gesetzt */
  if ((*x < 0) || (*x > 2)) {
      *x = 0;
      LineFeed(f);
      FilePrintString(f,"* ! unzulaessiges analytisches Geschwindigkeitsfeld eingegeben, Korrektur: 0");
      LineFeed(f);
  }
  return 1;
}


int TFAnav0 ( long *x, FILE *f )
{
  /* wenn anav0 nicht >=0, Abbruch */
  if (*x < 0) {
      LineFeed(f);
      FilePrintString(f,"* !!! Fehler: unzulaessiger Zentralknoten eingegeben !");
      LineFeed(f);
      return 0;
  }
  return 1;
}


int TFAnavx ( double *x, FILE *f )
{
  /* wenn anavx nicht >=0.0, Korrektur: 0.0 */
  if (*x < 0.0) {
      *x = 0.0;
      LineFeed(f);
      FilePrintString(f,"* ! unzulaessiger ANAV-Parameter, Korrektur: 0.0 !!");
      LineFeed(f);
  }
  return 1;
}


/* Testfunktionen zu Schluesselwoertern IC_FLOW, IC_SATURATION, IC_TRANSPORT */

int TFSatuvorgabe ( double *x, FILE *f )
{
  /* wenn konzvorgabe negativ, auf 0.0 gesetzt */
  if ((*x < 0.0)||(*x > 1.0)) {
    LineFeed(f);
    FilePrintString(f,"* !!! Fehler: Saettigungsvorgabe ");
    FilePrintDouble(f,*x);
    FilePrintString(f," unzulaessig ! (0 <= x <= 1 ");
    LineFeed(f);
    return 0;
  }
  return 1;
}


int TFKonzvorgabe ( double *x, FILE *f )
{
  /* wenn konzvorgabe negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! unzulaessige Konzentrationsvorgabe eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


/* Testfunktionen zu den Schluesselwoertern MATERIAL_1D, MATERIAL_2D, MATERIAL_3D */

int TFPorositaet ( double *x, FILE *f )
{
  /* wenn porositaet kleiner 0.0 auf 1.0 gesetzt */
  if (*x < 0.0) {
    *x = 1.0;
    LineFeed(f);
    FilePrintString(f,"* ! unzulaessige Porositaet eingegeben, Korrektur: 1.0");
    LineFeed(f);
  }
  return 1;
}


int TFVerteilungskoeff ( double *x, FILE *f )
{
  /* wenn verteilungskoeff negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! negativer Verteilungskoeffizient eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


int TFFelsdichte ( double *x, FILE *f )
{
  /* wenn felsdichte negativ, auf 1000.0 gesetzt */
  if (*x < 0.0) {
    *x = 1000.0;
    LineFeed(f);
    FilePrintString(f,"* ! negative Felsdichte eingegeben, Korrektur: 1000.0");
    LineFeed(f);
  }
  return 1;
}


int TFTortuositaet ( double *x, FILE *f )
{
  /* wenn tortuositaet negativ, auf 1.0 gesetzt */
  if (*x < 0.0) {
    *x = 1.0;
    LineFeed(f);
    FilePrintString(f,"* ! negative Tortuositaet eingegeben, Korrektur: 1.0");
    LineFeed(f);
  }
  return 1;
}


int TFDispersionsmodell ( int *x, FILE *f )
{
  /* dispersionsmodell = 1,2,3, sonst Abbruch */
  if ((*x >= 1) && (*x <= 3))
    return 1;
  else {
    LineFeed(f);
    FilePrintString(f,"* !!! Fehler: Dispersionsmodell unzulaessig !");
    LineFeed(f);
    return 0;
  }
}


int TFDisp1 ( double *x, FILE *f )
{
  /* wenn disp1 negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! negativer Dispersionskoeffizient eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


int TFDisp2 ( double *x, FILE *f )
{
  /* wenn disp2 negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! negatives alpha_l eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


int TFDisp3 ( double *x, FILE *f )
{
  /* wenn disp3 negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! negatives alpha_t eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


int TFArea ( double *x, FILE *f )
{
  /* wenn area nicht positiv, auf 1.0 gesetzt */
  if (*x <= 0.0) {
    *x = 1.0;
    LineFeed(f);
    FilePrintString(f,"* ! nicht positive Flaeche/Dicke eingegeben, Korrektur: 1.0");
    LineFeed(f);
  }
  return 1;
}


int TFSpeicherkoeff ( double *x, FILE *f )
{
  /* wenn speicherkoeff negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! negativer Speicherkoeffizient eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}


int TFMaterialmodell ( int *x, FILE *f )
{
  /* materialmodell = 0,1,2,3 sonst Abbruch */
  if ((*x >= 0) && (*x <= 3))
    return 1;
  else {
    LineFeed(f);
    FilePrintString(f,"* !!! Fehler: Materialmodell unzulaessig !");
    LineFeed(f);
    return 0;
  }
}


/* Testfunktionen zu Schluesselwort XD_GROUNDWATERFLOW_ELEMENT */

int TFNonlinearflowelement ( int *x, FILE *f )
{
  /* Fliessmodell entweder 0 oder 1, sonst 0 (Darcy) */
  if ((*x == 0) || (*x == 1))
      return 1;
  else {
    LineFeed(f);
    FilePrintString(f,"* ! Fehler: unzulaessiges Fliessmodell: Darcy=0, Forchheimer=1");
    LineFeed(f);
    }
    return 0;
}

int TFAlpha ( double *x, FILE *f )
{
  /* wenn alpha kleiner 0.0 oder groeser 1 dann auf 0.0 gesetzt */
  if ((*x < 0.0) || (*x > 1.0)) {
    LineFeed(f);
    FilePrintString(f,"* ! Fehler: unzulaessiger Parameter fuer nichtlineares Fliessgesetz: 0.0 <= alpha <= 1.0");
    LineFeed(f);
    return 0;
  }
  else
    return 1;
}


/* Testfunktionen zu Schluesselwort TRANSPORT_COMPONENT1 */

int TFHalbwertszeit ( double *x, FILE *f )
{
  /* wenn halbwertszeit negativ, auf 0.0 gesetzt */
  if (*x < 0.0) {
    *x = 0.0;
    LineFeed(f);
    FilePrintString(f,"* ! unzulaessige Halbwertszeit eingegeben, Korrektur: 0.0");
    LineFeed(f);
  }
  return 1;
}




int TFGravityAcceleration ( double *x, FILE *f )
{
  if (*x < 0.0) {
     *x = 9.81;
     LineFeed(f);
     FilePrintString(f,"* ! unzulaessige Gravitationskonstante, Korrektur: g = 9.81 m s^-2");
     LineFeed(f);
  }
  return 1;
}

int TFReferenceTemperature ( double *x, FILE *f )
{
  if (*x < 0.0) {
     *x = 293.0;
     LineFeed(f);
     FilePrintString(f,"* ! unzulaessige Referenztemperatur, Korrektur: T_0 = 293 K");
     LineFeed(f);
  }
  return 1;
}

int TFReferencePressure ( double *x, FILE *f )
{
  if (*x <= 0.0) {
     *x = 101325.;
     LineFeed(f);
     FilePrintString(f,"* ! unzulaessiger Referenzdruck, Korrektur: p_0 = 101325 Pa");
     LineFeed(f);
  }
  return 1;
}

/**************************************************************************
   ROCKFLOW - Modul: renumber.c
 
   Aufgabe:
   Knotenumnummerierer

   Jeder Knoten bekommt einen "Index" der seine Position im Gleichungs-
   system angibt. Im Gegenzug wird das Feld "NodeNumber" angelegt,
   in dem fuer jede Position des Gleichungssystems die zugehoerige
   Knotennummer abgelegt ist.
 
   Programmaenderungen:
   12/1994     MSR        Erste Version
   02/2000     C.Thorenz  Umnummerierer nach Gibbs et.al von Rainer eingebaut
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt   
   
 
**************************************************************************/
#include "stdafx.h" /* MFC */
/* Preprozessor-Definitionen */
#include "makros.h"
#define noTESTRENUM
/* Header / Andere intern benutzte Module */
#include "renumber.h"
#include "nodes.h"
#include "elements.h"
#include "tools.h"
#include "mathlib.h"
#include "geo_strings.h"
/* Interne (statische) Deklarationen */
/* #RENUMBER */
int umnummerierer = 0;
int umnummerier_laeufe = -1;

static long NodeNumberVectorSize = 0l, esize = 0l;     /* Groessen der Felder --> Realloc */
static short int *elementkennung = NULL;
static int anz_elkno;           /* Knotenanzahl der massgebenden Elementdimension */
static long startknoten;

/* Gibbs, Pole Stockmeyer */
typedef struct {                /* iListeN */
    long iAnz;                  /* Anzahl integer-Werte */
    long *iWerte;               /* Zeiger auf Feld mit iAnz integer-Werten */
} iListeN;


/* Zeitzaehler fuer den Umnummerierer */
#define TIMER_RENUMBER "RENUMBER"
static long renumber_id_timer = -1;

#define TIMER_RENUMBER_START "REN_START"
static long renumber_start_id_timer = -1;

/* Definitionen */
VoidFuncVoid RenumberStart;
VoidFuncVoid RenumberNodes;
VoidFuncVoid RenumberEnd;

long gpsLevel(long iStart, long NListe[], long nn,
              long Level[], long Num[], long *mLev);
long gpsNN(long k, long NListe[], long Level[], long iLevel);
long gpsNext(long k, long NListe[], long Level[], long iLevel);
long gpsLang(long NumSav[], long NumAlt[], long NListe[],
             long nn, long lSky);
long gpsKern(long NumSav[], long nn, long NListe[], long iStart, long MaxIt);
long gps(long m, iListeN * elverz[], long MaxIt, long *NN, long *maxN,
         long **altnum, long **neunum);

int TFRenumbernodes(int *x, FILE * f);

/**************************************************************************/
/* ROCKFLOW - Funktion: CompressNodes
 */
/* Aufgabe:
   einfacher Knotenumnummerierer, der lediglich die Loecher in der
   Knotenliste beseitigt
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   10/1999     AH         Systemzeit
 */
/**************************************************************************/
void CompressNodes(void)
{
    static long i, j, laenge;

    /* Systemzeit fuer den Umnummerierer ausfuehren */
//OK    RunSystemTime(TIMER_RENUMBER);

    if (NodeNumberVectorSize < NodeListLength) {
        NodeNumberVectorSize = NodeListLength;
        /* altes Indexverzeichnis loeschen */
        NodeNumber = (long *) Free(NodeNumber);
        /* neues Verzeichnis dimensionieren,
           neue Groesse des Verzeichnisses = NodeListLength */
        NodeNumber = (long *) Malloc(NodeListLength * sizeof(long));
    }
    /* hoechsten Knotennummer+1 holen */
    laenge = NodeListSize();
    /* Verzeichnis aufstellen */
    j = 0;                      /* Laufindex in NodeNumber-Feld */
    for (i = 0; i < laenge; i++) {      /* Schleife ueber Original-Knotennummern */
        if (GetNode(i) != NULL) {       /* wenn Knoten existiert */
            NodeNumber[j] = i;  /* in Indexfeld eintragen */
            SetNodeIndex(i, j); /* und Rueckwaertsverweis eintragen */
            j++;
        }
    }
#ifdef TESTRENUM
    if (j != NodeListLength)
        DisplayErrorMsg("CompressNodes: FEHLER IM KNOTENVERZEICHNIS !!!!!");
#endif
    /* Systemzeit fuer den Umnummerierer anhalten */
//OK    StopSystemTime(TIMER_RENUMBER);
}

void CompressStart(void)
{
}
void CompressEnde(void)
{
    NodeNumberVectorSize = 0l;                 /* OK rf3261 */
    esize = 0l;
    elementkennung = (short *) Free(elementkennung);
}

  /* Dummy-Funktionen, nur wg. Funktionszeiger-Zuweisung benoetigt */


/**************************************************************************/
/* ROCKFLOW - Funktion: CuthillStart
 */
/* Aufgabe:
   Knotenumnummerierer nach Cuthill-McKee; Startknoten ermitteln

   Diese Funktion muss vor dem ersten Verfeinerungsschritt aufgerufen
   werden !!!
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   11/1995     MSR        Erste Version
   10/1999     AH         Systemzeit
 */
/**************************************************************************/
void CuthillStart(void)
{
    static long i, j, bester, bestwert, breite, kmin, kmax, ind, max;
    static int st, n_anz, ii;
    static long *knoten;

    /* Systemzeit fuer den Start-Umnummerierer ausfuehren */
//OK    RunSystemTime(TIMER_RENUMBER_START);

    anz_elkno = ElNumberOfNodes[max_dim];
    /* Startknoten ermitteln */
    bester = 0l;
    bestwert = -1l;
    /* Schleife ueber alle Knoten */
    for (i = 0l; i < NodeListLength; i++) {
        st = GetNodeState(i);
        if (st == -3) {         /* Randknoten gefunden */
            startknoten = i;
            Cuthill();          /* Umnummerieren */
            /* Guete der Reihenfolge ueberpruefen --> max. Bandbreite */
            breite = 0l;
            for (j = 0; j < start_new_elems; j++) {
                knoten = ElGetElementNodes(j);
                n_anz = ElGetElementNodesNumber(j);
                /* minimale und maximale Knotennummer ermitteln */
                kmin = kmax = GetNodeIndex(knoten[0]);
                for (ii = 1; ii < n_anz; ii++) {
                    ind = GetNodeIndex(knoten[ii]);
                    if (ind < kmin)
                        kmin = knoten[ii];
                    if (ind > kmax)
                        kmax = knoten[ii];
                }

                if ((max = kmax - kmin + 1) > breite)
                    breite = max;
            }
            if ((breite < bestwert) || (bestwert < 0l)) {
                bester = i;
                bestwert = breite;
            }
        }
    }
    startknoten = bester;
    /* Systemzeit fuer den Start-Umnummerierer anhalten */
//OK    StopSystemTime(TIMER_RENUMBER_START);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Cuthill
 */
/* Aufgabe:
   Knotenumnummerierer nach Cuthill-McKee (reverse)

   Es kann nur EIN zusammenhaengendes Gebiet umnummeriert werden !!!
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   11/1995     MSR        Erste Version
   10/1999     AH         Systemzeit
 */
/**************************************************************************/
void Cuthill(void)
{
    static long ai, ei, i, j, laenge, hilf;
    static int elem_anz, ii, jj;
    static long *knoten, *elems;

    /* Systemzeit fuer den Umnummerierer ausfuehren */
//OK    RunSystemTime(TIMER_RENUMBER);

    if (NodeNumberVectorSize < NodeListLength) {
        NodeNumberVectorSize = NodeListLength;
        /* altes Indexverzeichnis loeschen */
        NodeNumber = (long *) Free(NodeNumber);
        /* neues Verzeichnis dimensionieren,
           neue Groesse des Verzeichnisses = NodeListLength */
        NodeNumber = (long *) Malloc(NodeListLength * sizeof(long));
    }
    laenge = ElListSize();
    /* Elementverzeichnis updaten */
    if (esize < laenge) {
        esize = laenge;
        elementkennung = (short *) Free(elementkennung);
        elementkennung = (short int *) Malloc(sizeof(short int) * laenge);
    }
    for (i = 0l; i < laenge; i++)
        elementkennung[i] = 1;
    /* Verzeichnis aufstellen */
    NodeNumber[0] = startknoten;        /* startknoten zuweisen */
    /* SetNodeIndex(startknoten,0);  bei Reverse Cuthill unnoetig */
    ai = 0l;                    /* Startindex der noch abzuarbeitenden Knoten */
    ei = 1l;                    /* Erster freier Index in NodeNumber */
    while (ei < NodeListLength) {
        elems = GetNodeXDElems[max_dim] (NodeNumber[ai], &elem_anz);
        for (ii = 0; ii < elem_anz; ii++) {     /* Schleife ueber alle Grenzelemente */
            if (elementkennung[elems[ii]]) {    /* El. noch nicht abgearbeitet */
                knoten = ElGetElementNodes(elems[ii]);
                for (jj = 0; jj < anz_elkno; jj++) {    /* Schleife ueber alle Elementknoten */
                    i = ai;
                    while ((i < ei) && (NodeNumber[i] != knoten[jj]))
                        i++;
                    if (i == ei) {      /* Knoten eintragen, weil noch nicht eingetragen */
                        NodeNumber[i] = knoten[jj];
                        /* SetNodeIndex(knoten[jj],i);  bei Reverse Cuthill unnoetig */
                        ei++;
                    }
                }
                /* Element austragen */
                elementkennung[elems[ii]] = 0;
            }
        }
        ai++;
    }
#ifdef TESTRENUM
    if (ei != NodeListLength)
        DisplayErrorMsg("Cuthill: FEHLER !!!!!");
#endif
    /* rueckwaerts Nummerieren */
    for (i = 0l, j = NodeListLength - 1l; i < j; i++, j--) {
        hilf = NodeNumber[i];
        NodeNumber[i] = NodeNumber[j];
        NodeNumber[j] = hilf;
        SetNodeIndex(NodeNumber[i], i);
        SetNodeIndex(NodeNumber[j], j);
    }
    if (i == j)                 /* weil oben weggelassen */
        SetNodeIndex(NodeNumber[i], i);
    /* Systemzeit fuer den Umnummerierer anhalten */
//OK    StopSystemTime(TIMER_RENUMBER);
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CuthillEnde
 */
/* Aufgabe:
   Knotenumnummerierer nach Cuthill-McKee
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   11/1995     MSR        Erste Version
 */
/**************************************************************************/
void CuthillEnde(void)
{
    NodeNumberVectorSize = 0l;                 /* OK rf3261 */
    esize = 0l;
    elementkennung = (short *) Free(elementkennung);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: Gibbs
 */
/* Aufgabe:
   Gibbs-Poole-Stockmeyer: Beste Numerierung fuer Skyline-Matrix suchen
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
 */
/**************************************************************************/
void Gibbs(void)
{
    long lsk, maxN, i, j, NN;
    long *altnum = NULL, *neunum = NULL;
    long m, ne;                 /* Anzahl Elemente, Anzahl Ecken */
    long *nodes;                /* Pointer auf Knoten */
    iListeN **elverz;

    /* Struktur fuer gps aufbauen: fuer m Elemente */
    m = anz_active_elements;
    /* ElementListLength; ElGetElementListSize(); */

    elverz = (iListeN **)Malloc(m * sizeof(iListeN *));

    for (i = 0; i < m; i++) {   /* Alle Elemente */
        elverz[i] = (iListeN *) Malloc(sizeof(iListeN));
        /* Knotenzahl pro Element */
        ne = ElNumberOfNodes[ElGetElementType(ActiveElements[i]) - 1];
        elverz[i]->iAnz = ne;
        elverz[i]->iWerte = (long *)Malloc(ne * sizeof(long));  /*Pointer auf Knotenzeile */

        nodes = ElGetElementNodes(ActiveElements[i]);
        for (j = 0; j < elverz[i]->iAnz; j++) {
            elverz[i]->iWerte[j] = nodes[j];
        }
    }

    lsk = gps(m, elverz, umnummerier_laeufe, &NN, &maxN, &NodeNumber, &neunum);

    if (NN != NodeListLength) {
        DisplayErrorMsg("GPS-Renumber encountered an error. Check your mesh!");
        exit(1);
    }

    NodeNumberVectorSize = NodeListLength;



/* TEST Vorbereitung */
    for (i = 0; i < NodeListSize(); i++) {      /* Alle Knoten */
        if (GetNode(i)) {
            /* Wenn der Knoten einen Eintrag im GLS hat diesen loeschen!!! */
            SetNodeIndex(i, -1);
        }
    }

    /* GPS-Ergebnisse eintragen */
    for (i = 0; i < NodeListLength; i++) {      /* Alle Eintraege im Gleichungssystem */
        SetNodeIndex(NodeNumber[i], i);
    }


/* TEST 1 */
    for (i = 0; i < NodeListSize(); i++) {      /* Alle Knoten */
        if (GetNode(i)) {
            /* Wenn der Knoten keinen Eintrag im GLS hat abbrechen */
            if(GetNodeIndex(i)== -1) {
                DisplayErrorMsg("GPS-Renumber encountered an internal error!");
                DisplayErrorMsg("Please submit the (?.rfd, ?.rfi, ?.rfo) files ");
                DisplayErrorMsg("to the developers.");
                exit(1);
            } 
        }
    }
/* TEST 2 */
    for (i = 0; i < NodeListLength; i++) {      /* Alle Eintraege im Gleichungssystem */
        if (GetNode(NodeNumber[i])) {
            /* Wenn der rueckindizierte Knoten keinen Eintrag im GLS hat abbrechen */
            if(GetNodeIndex(NodeNumber[i])== -1) {
                DisplayErrorMsg("GPS-Renumber encountered an internal error!");
                DisplayErrorMsg("Please submit the (?.rfd, ?.rfi, ?.rfo) files ");
                DisplayErrorMsg("to the developers.");
                exit(1);
            } 
        }
    }
/* TEST 3 */
    for (i = 0; i < NodeListLength; i++) {      /* Alle Eintraege im Gleichungssystem */
        if(i!=GetNodeIndex(NodeNumber[i])) {    /* Rueckwaertsverknuepfung pruefen */
           DisplayErrorMsg("GPS-Renumber encountered an internal error!");
           DisplayErrorMsg("Please submit the (?.rfd, ?.rfi, ?.rfo) files ");
           DisplayErrorMsg("to the developers.");
           exit(1);
        } 
    }



#ifndef ALTES_UMKOPIEREN
    for (i = 0; i < NodeListSize(); i++) {      /* Alle Knoten */
        if (GetNode(i)) {
            /* Wenn der Knoten einen Eintrag im GLS hat */
            if (neunum[i] >= 0) {
                /* Eintragen des Knotenindex */
                SetNodeIndex(i, neunum[i]);
            } else {
                DisplayErrorMsg("GPS-Renumber found node with no connection!");
            } 
        }
    }
#endif


    for (i = 0; i < m; i++) {
        Free(elverz[i]->iWerte);
        Free(elverz[i]);
    }
    Free(elverz);
    Free(altnum);
    Free(neunum);

}                               /* END GPS */

/* Dummy-Funktionen, nur wg. Funktionszeiger-Zuweisung benoetigt */
void GibbsStart(void)
{
}
void GibbsEnde(void)
{
}


/**************************************************************************
 ROCKFLOW - Funktion: gpsLevel

 Aufgabe:
   (GPS004) Levelstruktur aufbauen mit Startknoten iStart

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  istart    (Arg): Startknoten
  NListe    (Arg): Verzeichnis der Verbindungen
  nn        (Arg): Anzahl Knoten im System
  Level     (Res):  Levelnummern (>0) der Knoten
  Num       (Res):  alte Knotennummern in neuer Folge
  mLev      (Res): Breite des Pfades (=max. Anzahl Knoten in einem Level)

 Ergebnis:
  gpsLevel  (Res): Tiefe  des Pfades (=max. Levelnummer)
                 : -1: FEHLER: nicht zusammenhaengendes System

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gpsLevel(long iStart, long NListe[], long nn,
              long Level[], long Num[], long *mLev)
{
    long i, k1, k2, ia = 0, ie = 0, il = 0, nl = 0, ml = 0;

    for (i = 0; i < nn; i++)
        Level[i] = -1;          /* Vorbelegung: nicht einsortiert */
    Num[ie] = iStart;
    Level[iStart] = 0;

    while (ia <= ie) {
        k1 = Num[ia];           /* Ausgangsknoten */
        for (i = NListe[k1]; i < NListe[k1 + 1]; i++) {         /* alle Nachbarn k2 von k1 */
            k2 = NListe[i];
            if (Level[k2] < 0) {        /* noch nicht einsortiert? */
                ie++;
                Num[ie] = k2;
                Level[k2] = Level[k1] + 1;
                if (Level[k2] > nl) {   /*neuer Level */
                    nl = Level[k2];     /*Levelanzahl */
                    ml = max(ml, ie - il);      /* -Weite */
                    il = ie;    /* -Start */
                }
            }
        }                       /* for i ... */
        ia++;
    }                           /* while ia <= ie */

    mLev[0] = max(ml, ie - il + 1);
    nl++;
    if (ia < nn)
        nl = -1;                /* -1: unvollstaendig */
    return (nl);
}                               /* END GPS004 */


/*************************************************************************
 ROCKFLOW - Funktion: gpsNN

 Aufgabe:
  (GPS006) Anzahl Nachbarn von Knoten k in Level iLevel

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gpsNN(long k, long NListe[], long Level[], long iLevel)
/* vormals GPS006: Anzahl Nachbarn von Knoten k in Level iLevel
   R. Ratke, Jan. 2000;
 */
{
    long i, nk = 0;
    for (i = NListe[k]; i < NListe[k + 1]; i++) {       /* alle Nachbarn zu k */
        if (Level[NListe[i]] == iLevel)
            nk++;
    }
    return (nk);
}                               /* END GPS006 */



/*************************************************************************
 ROCKFLOW - Funktion: gpsNext

 Aufgabe:
  (GPS005) Nachbarknoten zu k mit kleinstem Grad in Level iLevel (-1: fertig)

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gpsNext(long k, long NListe[], long Level[], long iLevel)
/* vormals GPS005, R. Ratke, Jan. 2000
   Nachbarknoten zu k mit kleinstem Grad in Level iLevel (-1: fertig)
 */
{
    long i, k2, n2, k3, n3;
    for (i = NListe[k]; i < NListe[k + 1]; i++) {       /* alle Nachbarn */
        k2 = NListe[i];         /* Nachbarknoten */
        if (Level[k2] == iLevel)
            goto Gefunden;
    }
    return ((long) -1);

  Gefunden:
    n2 = gpsNN(k2, NListe, Level, iLevel);      /* Grad in Level iLevel */
    for (i++; i < NListe[k + 1]; i++) {         /* weitere Nachbarn */
        k3 = NListe[i];
        if (Level[k3] == iLevel) {
            n3 = gpsNN(k3, NListe, Level, iLevel);
            if (n3 < n2) {
                k2 = k3;
                n2 = n3;
            }                   /* geringerer Grad */
        }
    }
    return (k2);
}                               /*  END GPS005 */



/*************************************************************************
 ROCKFLOW - Funktion: gpsLang

 Aufgabe:
  (GPS007) SKYLINE: LÑnge der Obermatrix berechnen

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  NumSav  (Res): alte Nummern in neuer Folge (BESTE)
  NumAlt  (Arg): alte Nummern in neuer Folge (VERSUCH)
  NListe  (Arg): Nachbarverzeichnis
  nn      (Arg): Anzahl der Knoten im System
  lSky    (Arg): LÑnge der Skyline, nur Obermatrix (Arg -1: erster Versuch)

 Ergebnis:
   neue LÑnge der Skyline

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gpsLang(long NumSav[], long NumAlt[], long NListe[],
             long nn, long lSky)
/*vormals: GPS007, R. Ratke, Jan. 2000
   SKYLINE: LÑnge der Obermatrix berechnen
   NumSav  (Res): alte Nummern in neuer Folge (BESTE)
   NumAlt  (Arg): alte Nummern in neuer Folge (VERSUCH)
   NListe  (Arg): Nachbarverzeichnis
   nn      (Arg): Anzahl der Knoten im System
   lSky    (Arg): LÑnge der Skyline, nur Obermatrix (Arg -1: erster Versuch)
   Resultat: neue LÑnge der Skyline
 */
{
    int revers;
    long i, i1, k, k1, l, ll;
    long *Hilf;
    Hilf = (long *) Malloc(nn * sizeof(long));

    for (revers = 0; revers < 2; revers++) {
        for (i = 0; i < nn; i++) {
            Hilf[NumAlt[i]] = i;
        }                       /* neue Nrn. in alter Folge */

        ll = 0;
        for (k = 0; k < nn; k++) {      /* alte (Spalten-)Nr. */
            k1 = Hilf[k];
            l = 0;              /* neue */
            for (i = NListe[k]; i < NListe[k + 1]; i++) {       /* Nachbarn von k */
                i1 = Hilf[NListe[i]];   /* neue (Zeilen-)Nr. */
                l = max(l, k1 - i1);    /* Spaltenhoehe ohne Diagonale */
            }
            ll += l;            /*Gesamtlaenge */
        }
#ifdef TESTRENUM
        DisplayMsg("Laenge der Skyline incl. Diagonale:");
        DisplayLong(ll + nn);
        DisplayMsgLn("");
#endif
        if (ll < lSky || lSky < 0) {    /* Verbesserung! */
            for (i = 0; i < nn; i++)
                NumSav[i] = NumAlt[i];  /*Vektor retten */
            lSky = ll;
        }
        if (revers == 0) {
            k = nn;             /* umgekehrte Reihenfolge auch probieren */
            for (i = 0; i < nn / 2; i++) {
                ll = NumAlt[i];
                NumAlt[i] = NumAlt[--k];
                NumAlt[k] = ll;
            }
        }
    }                           /*revers */
    Free(Hilf);
    return (lSky);
}                               /*  END GPS007 */


/*************************************************************************
 ROCKFLOW - Funktion: gpsKern

 Aufgabe:
  (GPS003) eigentlicher Knotennummern-Optimimierer

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
  NumSav (A/R): alte Knotennummern in neuer Folge, beste Numerierung
  nn     (Arg): Anzahl Knoten im System (0 -- nn-1, ohne LÅcken!)
  NListe (Arg): Verbindungstabelle
  iStart (Arg): Startknoten mit minimalem Verbindungsgrad
  MaxIt  (Arg): Anzahl Nachiterationen (<0: unbegrenzt)

 Ergebnis:
    SkylinelÑnge (-1: Speicher reicht nicht aus / Fehler)

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gpsKern(long NumSav[], long nn, long NListe[], long iStart, long MaxIt)
{
    long *Hilf, *NumAlt;
    long frei, i, ia, ial, ie, iEnd = -1, ih1, ih2, ih3, ilev, j;
    long k, k1, k2, lev1, lev2, lsk, lsk0, lfi = 4 * nn;
    long m1, m2, ml1, ml2=0, ml2min, n2, nc, nk, nl1, nl2, nlev, nlev1,
     nlev2;
    double rk;
    double *rHilf;
/* Teil 0: Original-Numerierung ------------------------------- */
    NumAlt = (long *) Malloc(nn * sizeof(long));
    for (i = 0; i < nn; i++)
        NumAlt[i] = i;
    lsk = gpsLang(NumSav, NumAlt, NListe, nn, (long) -1);

/* Teil 1: Finde Pseudodurchmesser iStart-iEnd ---------------- */
    Hilf = (long *) Malloc(lfi * sizeof(long));
    lev1 = 0;
    lev2 = nn;                  /* Level hin und zurueck */
    ih1 = nn + nn;
    ih2 = ih1 + nn;             /* dito Knoten in neuer Folge */

    nl1 = gpsLevel(iStart, NListe, nn,
                   &Hilf[lev1], &Hilf[ih1], &ml1);      /* vorwaerts */
    if (nl1 <= 0) {
        printf("\n\n *** GPS: System hngt nicht zusammen\n\n");
        Free(Hilf);
        Free(NumAlt);
        return ((long) -1);
    }

  NeuStart:
    ml2min = nn;
    for (i = nn - 1; i >= 0; i--) {
        k = Hilf[ih1 + i];      /*Knoten des letzten Levels? */
        if (Hilf[lev1 + k] < nl1 - 1)
            goto maxTiefe;      /*fertig, vorletzter Level */
        nl2 = gpsLevel(k, NListe, nn,
                       &Hilf[lev2], &Hilf[ih2], &ml2);  /* rueckwaerts */
        if (nl2 > nl1) {        /*groessere Tiefe gefunden */
            iStart = k;
            nl1 = nl2;
            ml1 = ml2;
            lev1 = lev2;
            lev2 = nn - lev1;   /* P0100 */
            ih1 = ih2;
            ih2 = 5 * nn - ih1;
            goto NeuStart;
        }
        if (ml2 < ml2min) {
            iEnd = k;
            ml2min = ml2;
        }
    }                           /* i, letzter Level */

  maxTiefe:
    if (ml2min < ml2)           /*Levelliste ist ueberschrieben, nochmal rueckw. */
        nl2 = gpsLevel(iEnd, NListe, nn, &Hilf[lev2], &Hilf[ih2], &ml2);

    if ((NListe[iEnd + 1] - NListe[iEnd]) < (NListe[iStart + 1] - NListe[iStart])) {
        iStart = iEnd;
        lev1 = lev2;
        lev2 = nn - lev1;       /* P0100 */
        i = ml1;
        ml1 = ml2;
        ml2 = i;
    }

/* Teil 2: Level-Mengen, zusammenhaengende Teile Ci -------- */
    nlev = nn + nn;             /*Anzahl Knoten in Level (gleiche Levelnummern) */
    nlev1 = nlev + nl1;         /*dito, hin */
    nlev2 = nlev1 + nl1;        /*dito zurueck */
    ih1 = nlev2 + nl1;          /*Knoten der Komponenten Ci */
    ih2 = ih1 + nn;             /*Komponentennummer, wenn >0 */
    ih3 = ih2 + nn;             /*Knoten der Komponenten ohne "Rueckgrat" */
    frei = ih3 + nn;
    if (frei > lfi) {
        Hilf = (long *) Realloc(Hilf, frei * sizeof(long));
        lfi = frei;
    }

    for (i = 0; i < nl1; i++)
        Hilf[nlev + i] = 0;     /* P0300 */
    nc = 0;
    for (i = 0; i < nn; i++) {  /*Levelpaare bilden */
        Hilf[lev2 + i] = nl1 - Hilf[lev2 + i] - 1;      /*Levelnrn. ab 0 andersrum */
        if (Hilf[lev2 + i] == Hilf[lev1 + i]) {
            Hilf[ih2 + i] = 0;  /*"Rueckgrat" */
            Hilf[nlev + Hilf[lev1 + i]]++;      /* Anzahl in Level */
        }
        else {
            Hilf[ih2 + i] = -1;
            Hilf[ih3 + nc] = i;
            nc++;               /*Ci und Knoten(Ci) */
        }
    }

    if (nc > 0) {               /* Extra Komponenten ab ih1, Adressen ab ih2 */
        long jc = 0;            /* Gebietsnummer */
        ie = ih1 - 1;
        ia = ie;

      Ci_Start:ia++;
        if (ia > ie) {          /* keine "ungesaettigten" Nachbarschaften mehr */
            do {
                nc--;
                if (nc < 0)
                    goto Ci_End;
                k1 = Hilf[ih3 + nc];    /*Knoten suchen */
            } while (Hilf[ih2 + k1] >= 0);
            Hilf[++ie] = k1;    /*gefunden, einsortieren */
            jc++;
            Hilf[ih2 + k1] = jc;        /*neues Teilgebiet */
        }

        k1 = Hilf[ia];
        for (i = NListe[k1]; i < NListe[k1 + 1]; i++) {         /*alle Nachbarn */
            k2 = NListe[i];     /*Nachbarknoten */
            if (Hilf[ih2 + k2] < 0) {
                Hilf[++ie] = k2;
                Hilf[ih2 + k2] = jc;
            }
        }
        goto Ci_Start;

      Ci_End:                   /*Komponenten als 3*nc Matrix ab ih3 zusammenstellen */
        nc = ie - ih1 + 1;      /*nc extra Knoten ab ih1 */
        frei = ih3 + 3 * jc;
        if (frei > lfi) {
            Hilf = (long *) Realloc(Hilf, frei * sizeof(long));
            lfi = frei;
        }

        j = ih3;
        Hilf[j] = ih1;          /*Start Komponente 1 */
        for (i = ih1; i <= ie; i++) {   /*Alle Knoten */
            k1 = Hilf[i];       /*Komponentenknoten */
            k1 = ih3 + 3 * Hilf[ih2 + k1] - 3;
            if (k1 != j) {      /*neue Komponente? */
                Hilf[j + 1] = i - 1;    /*ie */
                Hilf[j + 2] = i - Hilf[j];      /*l */
                j += 3;
                Hilf[j] = i;    /*ia */
            }
        }
        Hilf[j + 1] = ie;       /*letzte Komponente */
        Hilf[j + 2] = ie - Hilf[j] + 1;
        P0260(&Hilf[ih3], (long) 3, jc, (long) 2, (long) 2);    /* sortieren (Laenge) */

        for (i = frei - 3; i >= ih3; i -= 3) {  /*check Ci, groesste Komponente zuerst */
            for (j = 0; j < (nl1 + nl1); j++)
                Hilf[nlev1 + j] = 0;    /* P0300 */
            for (j = Hilf[i]; j <= Hilf[i + 1]; j++) {  /*ia--ie */
                k1 = Hilf[j];   /*Knoten */
                Hilf[nlev1 + Hilf[lev1 + k1]]++;
                Hilf[nlev2 + Hilf[lev2 + k1]]++;
            }

            m1 = 0;
            m2 = 0;             /*neue Weiten */
            for (j = 0; j < nl1; j++) {         /*alle Level */
                if (Hilf[nlev1 + j] > 0)
                    m1 = max(Hilf[nlev + j] + Hilf[nlev1 + j], m1);
                if (Hilf[nlev2 + j] > 0)
                    m2 = max(Hilf[nlev + j] + Hilf[nlev2 + j], m2);
            }

            m1 = m1 - m2;
            if (m1 == 0) {
                m1 = ml1 - ml2;
            }                   /* kleinste Weite erzeugen */
            if (m1 > 0) {
                for (j = Hilf[i]; j <= Hilf[i + 1]; j++) {      /*lev2 ist schmaler */
                    k1 = Hilf[j];       /*Knoten */
                    Hilf[lev1 + k1] = Hilf[lev2 + k1];  /*use lev2 */
                }
                for (j = 0; j < nl1; j++) {
                    Hilf[nlev + j] += Hilf[nlev2 + j];
                    ml2 = max(ml2, Hilf[nlev + j]);
                }
            }                   /* m1>m2 */
            else {
                for (j = 0; j < nl1; j++) {     /*lev1 ist schmaler, nur Weiten updaten */
                    Hilf[nlev + j] += Hilf[nlev1 + j];
                    ml1 = max(ml1, Hilf[nlev + j]);
                }
            }                   /* else (m1>=m2) */
        }                       /* Check Ci */
    }                           /* nc > 0 */

/* Teil 3. Nummern einordnen ------------------------------- */
/* Zuerst leichteres Finden der Startknoten vorbereiten */
    Hilf[ih1] = lev2;           /* Adressen */
    Hilf[ih2] = Hilf[nlev];     /*Anzahl */
    for (i = 1; i < nl1; i++) {
        Hilf[ih1 + i] = Hilf[ih1 + i - 1] + Hilf[nlev + i - 1];
        Hilf[ih2 + i] = Hilf[nlev + i];
    }
    for (i = 0; i < nn; i++) {  /*Knoten nach Level einordnen */
        ilev = Hilf[lev1 + i];
        Hilf[ih2 + ilev]--;
        Hilf[Hilf[ih1 + ilev] + Hilf[ih2 + ilev]] = i;
    }
    for (i = 0; i < nl1; i++)
        Hilf[ih2 + i] = Hilf[nlev + i];         /* wieder Anzahl */

/* Eigentliches Einsortieren */
    ie = -1;
    ial = 0;
    ilev = 0;                   /* ilev=Level; */
    while (ilev < nl1) {
        ia = ial - 1;           /*alter Level: Start */
        ial = ie + 1;

/* Level vervollstaendigen --- */
      StartLevel:
        while (ia < ie) {
            k1 = NumAlt[++ia];
            for(;;) {         /* forever */
                k2 = gpsNext(k1, NListe, &Hilf[lev1], ilev);
                if (k2 < 0)
                    goto StartLevel;
                NumAlt[++ie] = k2;
                Hilf[lev1 + k2] = -1;   /* Kennzeichen: ist einsortiert */
                Hilf[nlev + ilev]--;
            }
        }                       /* ia<ie */

/* neuen Startknoten k2 mit min. Grad in Level ilev suchen --- */
        if (Hilf[nlev + ilev] > 0) {    /*Level noch nicht fertig */
            if (iStart < 0) {
                k2 = -1;
                n2 = nn + 1;
                for (i = 0; i < Hilf[ih2 + ilev]; i++) {
                    k = Hilf[Hilf[ih1 + ilev] + i];
                    if (Hilf[lev1 + k] == ilev) {
                        nk = gpsNN(k, NListe, &Hilf[lev1], ilev);       /* Grad in ilev */
                        if (nk < n2) {
                            k2 = k;
                            n2 = nk;
                        }
                    }
                }
            }
            else {
                k2 = iStart;
                iStart = -1;
            }

            if (k2 >= 0) {
                NumAlt[++ie] = k2;
                Hilf[lev1 + k2] = -1;
                Hilf[nlev + ilev]--;
                goto StartLevel;
            }
        }                       /* Hilf[...] > 0 */
        ilev++;
    }                           /* ilev < nl1 */

/* fertig mit Gibbs-Poole-Stockmeyer */
    lsk = gpsLang(NumSav, NumAlt, NListe, nn, lsk);

/* Iteration durch Relaxation der Knotennummern */
    rHilf = (double *) Malloc(nn * sizeof(double));
    do {
        for (i = 0; i < nn; i++) {
            Hilf[lev1 + NumSav[i]] = i;         /* neue Nrn., alte Folge */
            NumAlt[i] = NumSav[i];
        }

        for (ilev = 0; ilev < 10; ilev++) {     /* nur alle 10 Iterationen sortieren! */
            for (i = 1; i < nn - 1; i++) {      /* alle Knoten ausser Start/Ende */
                rk = 0.0;
                k = NumSav[i];  /* alter Knoten */
                for (j = NListe[k]; j < NListe[k + 1]; j++) {   /*alle Nachbarn */
                    k2 = NListe[j];
                    rk += Hilf[lev1 + k2];      /* rk +=neue Nr. Nachbar */
                }
                rHilf[i] = rk / (NListe[k + 1] - NListe[k]);    /* /Anzahl -> neue Nr. */
                Hilf[lev1 + k] = (long)(rHilf[i] + 0.5);
            }
        }

        P0265(&NumAlt[1], &rHilf[1], nn - 2, &Hilf[ih1]);
        lsk0 = lsk;
        lsk = gpsLang(NumSav, NumAlt, NListe, nn, lsk);
        if (MaxIt > 0)
            MaxIt--;
    } while (lsk < lsk0 && MaxIt != 0);         /* mindestens 1 Versuch */

    Free(rHilf);
    Free(Hilf);
    Free(NumAlt);
    return (lsk);
}                               /* END GPS003 */


/*************************************************************************
 ROCKFLOW - Funktion: gps

 Aufgabe:


 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   m      = Anzahl Elemente
   elverz[i]->iAnz: Anzahl Ecken
   elverz[i]->iWerte[j]: j-te Knotennummer von Element i
   MaxIt  = Anzahl Nachiterationen (<0: unbegrenzt)

   Resultate:
   NN     : Anzahl der Knoten, die an Elementen vorkommen
            (Die neue Numerierung wird lueckenlos: 0 bis NN-1.)
   maxN   : groesste im Elementverzeichnis vorkommende Knotennummer
            (Es wird nicht vorausgesetzt, dass alte Numerierung
            lueckenlos ist.)
   altnum[0..NN-1] : NN alte Knotennummern in neuer Reihenfolge
   neunum[0..maxN] : maxN+1 neue Knotennummern in alter Reihenfolge

   Die Zeiger  altnum und neunum muessen initialisiert sein! Bei
   Bedarf werden sie veraendert und der erforderliche Platz (re)allokiert.

 Ergebnis:
   LÑnge der Obermatrix ohne Diagonalelemente

 Programmaenderungen:
   1/2000     Rainer Ratke     Erste Version
   2/2000     C.Thorenz        Einbau in RockFlow
*************************************************************************/
long gps(long m, iListeN * elverz[], long MaxIt, long *NN, long *maxN,
         long **altnum, long **neunum)
{
    long i, ia, ie, j, k, l, gstart, maxn = 0, nn = 0;
    iListeN **ElKno;            /* Elemente an Knoten */
    long *NListe;               /* Nachbarknoten mit Adresskopf */
    long lSky, iStart;          /* Skylinelaenge, Startknoten */
    long *NumSav;               /* beste Numerierung */
    long *IntNum, *ExtNum;

/* groesste Knotennummer aus Elementverzeichnis ermitteln */
    for (i = 0; i < m; i++) {
        for (j = 0; j < elverz[i]->iAnz; j++) {
            k = elverz[i]->iWerte[j];
            if (k > maxn)
                maxn = k;
        }
    }

/* IntNum: nach evtl. (re-) allokieren: Knotenbelegung */
    IntNum = (long *) Realloc(*neunum, (1 + maxn) * sizeof(long));

    for (i = 0; i <= maxn; i++)
        IntNum[i] = -1;         /* -1: nicht vorhanden */
    for (i = 0; i < m; i++) {
        for (j = 0; j < elverz[i]->iAnz; j++)
            IntNum[elverz[i]->iWerte[j]] = 0;
    }                           /* 0: vorhanden */

/* neue interne Nummern vergeben ohne LÅcken */
    for (i = 0; i <= maxn; i++) 
        if (IntNum[i] == 0)
            IntNum[i] = nn++;
    /* interne Nummern in externer Folge, -1 bleibt */
    ExtNum = (long *) Realloc(*altnum, nn * sizeof(long));
    for (i = 0; i <= maxn; i++)
        if (IntNum[i] >= 0)
            ExtNum[IntNum[i]] = i;
    /* externe Nummern in interner Folge */

/* Nachbarliste erzeugen auf Basis interner Numerierung */
    ElKno = (iListeN **)Malloc(nn * sizeof(iListeN *));
    for (i = 0; i < nn; i++) {  /* alle Knoten */
        ElKno[i] = (iListeN *) Malloc(sizeof(iListeN));
        ElKno[i]->iAnz = 0;     /*Anzahl Elemente */
        ElKno[i]->iWerte = NULL;        /*Pointer auf Elementnummern */
    }
    for (i = 0; i < m; i++)     /* Elementnummern i in ElKno sammeln */
        for (j = 0; j < elverz[i]->iAnz; j++) {         /* alle Ecken */
            k = IntNum[elverz[i]->iWerte[j]];   /* Knoten */
            l = ElKno[k]->iAnz + 1;
            ElKno[k]->iWerte = (long *)Realloc(ElKno[k]->iWerte, l * sizeof(long));
            ElKno[k]->iWerte[l - 1] = i;        /*Elementnummer eingetragen */
            ElKno[k]->iAnz = l; /*Anzahl berichtigen */
        }

/* Nachbarknotenliste als 1-d Liste mit Inhaltsverzeichnis */
    ia = nn + 1;
    ie = 3 * nn;                /* ia: frei, ie: aktuelle Laenge */
    NListe = (long *) Malloc(ie * sizeof(long));
    for (k = 0; k < nn; k++) {  /* alle Knoten */
        long i1, k1;
        NListe[k] = ia;         /* Startadresse */
        for (l = 0; l < ElKno[k]->iAnz; l++) {  /* Elemente an k */
            i = ElKno[k]->iWerte[l];
            for (j = 0; j < elverz[i]->iAnz; j++) {     /* alle Ecken j */
                k1 = IntNum[elverz[i]->iWerte[j]];
                if (k1 == k)
                    goto end_j;
                for (i1 = NListe[k]; i1 < ia; i1++)
                    if (NListe[i1] == k1)
                        goto end_j;
                if (ia == ie) {
                    ie += nn;
                    NListe = (long *)Realloc(NListe, ie * sizeof(long));
                }
                NListe[ia++] = k1;      /*neuer Nachbar zu k */
              end_j:j = j;
            }
        }
        Free(ElKno[k]->iWerte);
        Free(ElKno[k]);
    }
    NListe[nn] = ia;
    Free(ElKno);
    NListe = (long *)Realloc(NListe, ie * sizeof(long));

/* Nachbarn nach Grad sortieren und Startknoten mit minimalem Grad */
    iStart = 0;
    gstart = nn;
    for (i = 0; i < nn; i++) {
        long ia, ie, gi, gk, j1, k1;
        ia = NListe[i];
        ie = NListe[i + 1] - 1;
        gi = ie - ia + 1;
        if (gi < gstart) {
            iStart = i;
            gstart = gi;
        }                       /*Startkn. mit min. Grad */
        for (j = ia; j < ie; j++) {     /*Bubble-Sort */
            j1 = NListe[j];
            gi = NListe[j1 + 1] - NListe[j1 + 1];
            for (k = j + 1; k <= ie; k++) {
                k1 = NListe[k];
                gk = NListe[k1 + 1] - NListe[k1 + 1];
                if (gk < gi) {
                    gi = gk;
                    NListe[j] = k1;
                    NListe[k] = j1;
                    j1 = k1;
                }
            }
        }
    }
    NumSav = (long *) Malloc(nn * sizeof(long));

    lSky = gpsKern(NumSav, nn, NListe, iStart, MaxIt);  /*GPS-ALGORITHMUS */

/* Umnumerierungslisten fuer externe Knotennummern */
    for (i = 0; i < nn; i++)
        NumSav[i] = ExtNum[NumSav[i]];  /* externe Nrn. */
    for (i = 0; i < nn; i++) {
        ExtNum[i] = NumSav[i];  /* extern in optimaler Folge */
        IntNum[ExtNum[i]] = i;  /* intern in externer Folge */
    }

    Free(NumSav);
    Free(NListe);
    *neunum = IntNum;
    *altnum = ExtNum;
    *maxN = maxn;
    *NN = nn;
    return (lSky);
}                               /*  END  gps */
/* Ende der Funktionen zu GPS */




/* Testfunktionen zum Schluesselwort RENUMBER */
int TFRenumbernodes(int *x, FILE * f)
{
    /* wenn umnummerierer kleiner 0 oder groesser 2, auf 0 gesetzt */
    if ((*x < 0) || (*x > 2)) {
        *x = 0;
        LineFeed(f);
        FilePrintString(f, "* ! unzulaessigen Knotenumnummerierer gewaehlt, Korrektur: 0");
        LineFeed(f);
    }
    return 1;
}


/*************************************************************************
 ROCKFLOW - Funktion: ConfigRenumberProperties

 Aufgabe:
   Konfiguration des Umnummerierers

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:
   12/1999   OK   Implementierung
    2/2000   CT   Erweitert um GPS

*************************************************************************/
void ConfigRenumberProperties(void)
{
    NodeNumberVectorSize = 0l; //OK3809

    switch (umnummerierer) {
    case 0:
        RenumberStart = CompressStart;
        RenumberNodes = CompressNodes;
        RenumberEnd = CompressEnde;
        /* Systemzeit fuer den Umnummerierer setzen */
//OK        SetSystemTime(TIMER_RENUMBER_START, "ROCKFLOW", "(Compress:0)", &renumber_start_id_timer);
//OK        SetSystemTime(TIMER_RENUMBER, "ROCKFLOW", "(Compress:0)", &renumber_id_timer);
        break;
    case 1:
        RenumberStart = CuthillStart;
        RenumberNodes = Cuthill;
        RenumberEnd = CuthillEnde;
        /* Systemzeit fuer den Umnummerierer setzen */
//OK        SetSystemTime(TIMER_RENUMBER_START, "ROCKFLOW", "(Cuthill:0)", &renumber_start_id_timer);
//OK        SetSystemTime(TIMER_RENUMBER, "ROCKFLOW", "(Cuthill:1)", &renumber_id_timer);
        break;
    case 2:
        RenumberStart = GibbsStart;
        RenumberNodes = Gibbs;
        RenumberEnd = GibbsEnde;
        /* Systemzeit fuer den Umnummerierer setzen */
//OK        SetSystemTime(TIMER_RENUMBER_START, "ROCKFLOW", "(Gibbs:0)", &renumber_start_id_timer);
//OK        SetSystemTime(TIMER_RENUMBER, "ROCKFLOW", "(Gibbs:1)", &renumber_id_timer);
        break;
    }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctRenumber
 */
/* Aufgabe:
   Liest die zu dem Schluesselwort RENUMBER gehoerigen Daten ein
   und erstellt den zugehoerigen Protokollabschnitt.
   RENUMBER: Knoten-Umnummerierer
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
 */
/* Ergebnis:
   0 bei Fehler, sonst 1
 */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   07/1996     RK         Rockflow-Anpassung
   10/1996     MSR        korrigiert
   02/2000     C.Thorenz  Gibbs, Pole, Stockmeyer eingebaut
 */
/**************************************************************************/
int FctRenumber(char *data, int found, FILE *f)
{
    int ok = 1;
    int pos = 0;
#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort RENUMBER");
#endif
#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {               /* RENUMBER nicht gefunden */
    }
    else {                      /* RENUMBER gefunden */
        FilePrintString(f, "#RENUMBER");
        LineFeed(f);
        if(found==1) {
            ok = (StrReadInt(&umnummerierer, data, f, TFRenumbernodes, &pos) && ok);
            LineFeed(f);
            if (umnummerierer==2) {
                ok = (StrReadInt(&umnummerier_laeufe, data, f, TFInt, &pos) && ok);
                LineFeed(f);
            }
        }
        else if (found==2) {
            fprintf(f," %i ",umnummerierer);
            if (umnummerierer==2) {
                fprintf(f," %i ",umnummerier_laeufe);
            }
        }
    }

#ifdef EXT_RFD
    FilePrintString(f, "; Schluesselwort: #RENUMBER");
    LineFeed(f);
    FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann mit den");
    LineFeed(f);
    FilePrintString(f, "; Vorgabewerten gearbeitet. Wenn es vorhanden ist, muessen auch alle");
    LineFeed(f);
    FilePrintString(f, "; Werte angegeben werden.");
    LineFeed(f);
    FilePrintString(f, "; - Knotenumnummerierer (umnummerierer) [0,1;0]");
    LineFeed(f);
    FilePrintString(f, ";   0: Nur Loecher beseitigen");
    LineFeed(f);
    FilePrintString(f, ";   1: Cuthill McKee");
    LineFeed(f);
    FilePrintString(f, ";   2: Gibbs Pole Stockmeyer (GPS)");
    LineFeed(f);
    FilePrintString(f, "; - Nur bei Gibbs Pole Stockmeyer:");
    LineFeed(f);
    FilePrintString(f, ";   -1 : Unbegrenzte Durchlaeufe im Nachlauf");
    LineFeed(f);
    FilePrintString(f, ";   >0 : Begrenzte Durchlaeufe im Nachlauf");
    LineFeed(f);
#endif

    return ok;
}

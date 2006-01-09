/**************************************************************************/
/* ROCKFLOW - Modul: refine3.c
                                                                          */
/* Aufgabe:
   Verfeinerungs- / Vergroeberungsfunktionen
   Massgebend fuer die Verfeinerung ist der Zustand des
   "verfeinern" - Schalters in den Elementdaten.
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
   04.08.1998  R.Kaiser   Adaption fuer mehrere Loesungsgroessen
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   02/2000   C.Thorenz    CompressNodes _vor_ ExecuteAllBoundaryConditions

                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* In Rekursionen sollten prinzipiell keine statischen Variablen benutzt
   werden. Wenn ein Variablenwert entweder nur vor oder nach dem
   rekursiven Aufruf benutzt wird, also nicht 'heruebergerettet' werden
   muss, kann die Variable trotzdem als statisch vereinbart werden. */
#ifdef REF_STATIC
#define STATIC static
#else
#define STATIC
#endif


/* Header / Andere intern benutzte Module */
#include "refine.h"
#include "nodes.h"
#include "elements.h"
#include "plains.h"
#include "mathlib.h"
#include "masscont.h"
#include "msh_elements_rfi.h"

/* Interne (statische) Deklarationen */

void R3D_a_1D ( long number );
void R3D_a_2D ( long number );
void R3D_a_3D ( long number );
void RR3D_a_1D ( long number );
void RR3D_a_2D ( long number );
void RR3D_a_3D ( long number );
int RRM3D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM3D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM3D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RN3D_a_1D ( long number );
int RN3D_a_2D ( long number );
int RN3D_a_3D ( long number );

int CheckNodeState3D ( long irrnode, long k1, long k2 );
int CheckNodeStateA3D ( long irrnode, Flaeche *flaeche );
long CheckNodeExist3D ( long k1, long k2 );


/* Definitionen */

static VoidFuncLong R3D_a[3] = { R3D_a_1D, R3D_a_2D, R3D_a_3D };
  /* Feld mit den Elementverfeinerungsfunktionen fuer NetRef3D_a */
static VoidFuncLong RR3D_a[3] = { RR3D_a_1D, RR3D_a_2D, RR3D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef3D_a */
static IntFuncLIDX RRM3D_a[3] = { RRM3D_a_1D, RRM3D_a_2D, RRM3D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef3D_a */
static IntFuncLong RN3D_a[3] = { RN3D_a_1D, RN3D_a_2D, RN3D_a_3D };
  /* Feld mit den Elementnachlauffunktionen fuer NetRef3D_a */

static int nachlauf;  /* 0: Verfeinerungsschleife; 1: Nachlaufschleife */
static int rr_ret; /* 0: keine Aktion; 1: Aktion (Vergroeberung) */


static int nmk_idx[6] = { 16, 18, 8, 9, 10, 11 };
  /* Indizes der Flaechenmittelknoten im Zwischenspeicher */
static int nfk_idxp[24][4] = { { 19, 4, 16, 7 },
                               { 20, 5, 16, 4 },
                               { 21, 6, 16, 5 },
                               { 22, 7, 16, 6 },
                               { 26, 14, 18, 15 },
                               { 25, 13, 18, 14 },
                               { 24, 12, 18, 13 },
                               { 23, 15, 18, 12 },
                               { 20, 4, 8, 1 },
                               { 19, 0, 8, 4 },
                               { 23, 12, 8, 0 },
                               { 24, 1, 8, 12 },
                               { 21, 5, 9, 2 },
                               { 20, 1, 9, 5 },
                               { 24, 13, 9, 1 },
                               { 25, 2, 9, 13 },
                               { 22, 6, 10, 3 },
                               { 21, 2, 10, 6 },
                               { 25, 14, 10, 2 },
                               { 26, 3, 10, 14 },
                               { 19, 7, 11, 0 },
                               { 22, 3, 11, 7 },
                               { 26, 15, 11, 3 },
                               { 23, 0, 11, 15 } };
  /* Indizes der Knoten der 6*4 neuen Aussenflaechen im Zwischenspeicher bei
     positivem Umlaufsinn */
static int kikno_idx[8][8] = { { 19, 4, 16, 7, 0, 8, 17, 11 },
                               { 20, 5, 16, 4, 1, 9, 17, 8 },
                               { 21, 6, 16, 5, 2, 10, 17, 9 },
                               { 22, 7, 16, 6, 3, 11, 17, 10 },
                               { 23, 15, 18, 12, 0, 11, 17, 8 },
                               { 24, 12, 18, 13, 1, 8, 17, 9 },
                               { 25, 13, 18, 14, 2, 9, 17, 10 },
                               { 26, 14, 18, 15, 3, 10, 17, 11 } };
  /* Indizes der Knoten der Kindelemente im Zwischenspeicher ([Ele][Kno-Nr]) */
static int kifla_idx[8][6] = { { 0, 28, 9, 24, 27, 20 },
                               { 1, 29, 13, 25, 24, 8 },
                               { 2, 30, 17, 26, 25, 12 },
                               { 3, 31, 21, 27, 26, 16 },
                               { 7, 28, 23, 35, 32, 10 },
                               { 6, 29, 11, 32, 33, 14 },
                               { 5, 30, 15, 33, 34, 18 },
                               { 4, 31, 19, 34, 35, 22 } };
  /* Indizes der Flaechen der Kindelemente im Zwischenspeicher */


/**************************************************************************/
/* ROCKFLOW - Funktion: NetRef3D_a
                                                                          */
/* Aufgabe:
   Verfeinert und Vergroebert gekoppelte 1D-, 2D- und 3D-Element-Netze
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   1: Das Netz wurde veraendert; 0: keine Veraenderung
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int NetRef3D_a ( void )
{
  /* Variablen */
  static long i, j, jj;  /* Laufvariablen */
  static long vorgaenger;  /* Vorgaenger eines evtl. zu
                              vergroebernden Elements */
  static long max_index;  /* hoechster Elementindex */
  static int level;  /* Verfeinerungslevel */
  static int ret;  /* return-Wert */
  static int num_ref_quantity; /* Laufvariablen */
  double *zwischenspeicher;


  ret = 0;
  max_index = ElListSize();  /* Hoechsten Index holen */
  /* Schleife ueber alle zu verfeinernden Elemente */
  nachlauf = 0;  /* Keine sekundaeren Kriterien ueberpruefen */
  max_index = ElListSize();  /* Hoechsten Index holen */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
    if (ElGetElement(i)!=NULL)  /* wenn Element existiert */
      if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
        if (ElGetElementRefineState(i)==1) {
          if ((level=ElGetElementLevel(i))<AdaptGetMaxRefLevel()) {
            /* wenn nicht schon max. Verfeinerungstiefe erreicht ist */
            R3D_a[ElGetElementType(i)-1](i);
            /* wenn noetig, Element(e) verfeinern */
            ret = 1;
            if (level==AdaptGetActualRefLevel())
              AdaptSetActualRefLevel(AdaptGetActualRefLevel()+1); /* evtl. ref_level erhoehen */
          }
          else {
            ElSetElementRefineState(i,0);
          }
        }
      }
  }


  /* Jetzt sind alle Elemente, die verfeinert werden sollen, verfeinert.
  Ausserdem ist die Konsistenz des Netzes sichergestellt, d.h. die max.
  Differenz des Verfeinerungsgrades zweier benachbarter Elemente betraegt 1. */

  /* Sicherstellen, dass NodeNumber, NodeListSize und NodeIndex stimmig sind */
  CompressNodes(); /* CT3405 */
  /* Randbedingungen ausfuehren */
  //OK_BC ExecuteAllBoundaryConditions();
  /* Quelle/Senken ausfuehren */
  //OK_ST ExecuteAllSourceSink();


#ifdef TESTREF
    DisplayMsgLn("Verfeinern abgeschlossen ...");
#endif

  rr_ret = 0;
  zwischenspeicher =  (double *) Malloc(2*NodeListSize()*sizeof(double));
  for (num_ref_quantity=0;num_ref_quantity<AdaptGetNumAdaptQuantities();num_ref_quantity++) {
     for (j=0;j<(2*NodeListSize());j++) zwischenspeicher[j] = 0.;


    for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
      if (ElGetElement(i)!=NULL)  /* wenn Element existiert */
        if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
          if (ElGetElementRefineState(i)==-1) {
             /* Ueberpruefen, ob Element einen Vorgaenger hat */
             if ((vorgaenger = ElGetElementPred(i))>=0) {
               /* i ist kein Ausgangselement, kann vergroebert werden */
               /* ret = (RRM1D_a[ElGetElementType(vorgaenger)-1](vorgaenger)) || ret; */
               RRM3D_a[ElGetElementType(vorgaenger)-1](vorgaenger,num_ref_quantity,zwischenspeicher);
               /* Die rekursive Vergroeberungsfunktion wird mit dem
               Vorgaengerelement aufgerufen, der Return-Wert ist
               modulglobal vereinbart */
             }
             else {
               /* i ist Ausgangselement, kann nicht vergroebert werden */
               ElSetElementRefineState(i,0);
             }
          }
        }
    }

    IncAllNodeVals(AdaptGetRefNval0(num_ref_quantity),AdaptGetRefNval1(num_ref_quantity),zwischenspeicher);

  }
  zwischenspeicher = (double *)Free(zwischenspeicher);


  ret = rr_ret || ret;
  if (!ret) {
    /* Kein Element veraendert ==> keine Massen veraendert;
       kein Nachlauf noetig etc. */

#ifdef TESTREF
      DisplayMsgLn("Refine: Keine Netzanpassung durchgefuehrt!");
#endif
    return ret;
  }


  /* Schleife ueber alle zu vergroebernden Elemente, direkt vergroebern */
  /* Der Nachlauf wird wg. sauberer Massenerhaltung benoetigt */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
    if (ElGetElement(i)!=NULL) {  /* wenn Element existiert */
      if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
        if (ElGetElementRefineState(i)==4) {
          /* Vorgaenger holen (muss existieren, da Nachlauf) */
          vorgaenger = ElGetElementPred(i);
          RR3D_a[ElGetElementType(vorgaenger)-1](vorgaenger);
          /* Die nicht rekursive Vergroeberungsfunktion wird mit dem
             Vorgaengerelement aufgerufen */
        }
      }
    }
  }
  /* Alle Elemente, die vergroebert werden sollen, sind, soweit es die
     Konsistenz des Netzes nicht beeintraechtigt, vergroebert. */

  /* irregulaere Knotenwerte interpolieren (wg. Massenerhaltung) */

  for (jj=0;jj<AdaptGetNumAdaptQuantities();jj++)
    InterpolIrregNodeVals3D(AdaptGetRefNval0(jj),AdaptGetRefNval1(jj));

#ifdef TESTREF
    DisplayMsgLn("Vergroebern abgeschlossen ...");
#endif
  nachlauf = 1;  /* Sekundaere Kriterien ueberpruefen */
  /* Nachlauf-Schleife ueber alle Elemente ( verfeinerte Gebiete abrunden ) */
  max_index = ElListSize();  /* Hoechsten Index holen */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
    if (ElGetElement(i)!=NULL) {  /* wenn Element existiert */
      if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
        /* Element(e) ueberpruefen und, wenn noetig, verfeinern */
        RN3D_a[ElGetElementType(i)-1](i);
      }
    }
  }

  /* Jetzt sind auch alle 3D-Elemente verfeinert, die vorher mindestens
     ref_neighbours verfeinerte 3D-Nachbarelemente hatten und noch nicht
     verfeinert waren. Ausserdem haben 2D- und 1D-Elemente nur Nachbarn
     von gleichem Verfeinerungsgrad. */

#ifdef TESTREF
    DisplayMsgLn("Nachlauf abgeschlossen ...");
#endif
  return ret;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R3D_a_1D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void R3D_a_1D ( long number )
{
  int i, j;  /* Laufvariablen */
  int level, aktiv;
  long *flaechen = NULL;  /* Feld mit Elementflaechen von number */
  Flaeche *flaeche = NULL;  /* Eine (Nachbar-)Flaeche */
  Flaeche *vorg_flaeche = NULL;  /* Vorgaenger einer (Nachbar-)Flaeche */
  int fanz;  /* Anzahl der Flaechen */
  STATIC Flaeche *kflaeche;  /* Eine Kindflaeche */
  STATIC long *alte_eckknoten;  /* Feld mit Knotennummern von number */
  STATIC int ii, k;  /* Laufvariablen */
  STATIC long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  STATIC long *flaechenliste0;  /* Feld mit Flaechen der neuen Elemente */
  STATIC long *flaechenliste1;  /* Feld mit Flaechen der neuen Elemente */
  STATIC long neuer_knoten;  /* Knotennummer des neuen Knotens */
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  fanz = ElGetElementPlainsNumber(number);  /* Anzahl der Flaechen */
  for (i=0;i<fanz;i++) {  /* Schleife ueber alle Flaechen */
    flaeche = GetPlain(flaechen[i]);
    if (flaeche->kinder[0]<0l) {
      /* Diese Flaeche wurde noch nicht verfeinert! */
      if (flaeche->vorgaenger>=0l) {  /* es exist. ein Vorgaenger */
        /* Vorgaenger der Flaeche holen, wird wahrscheinlich gebraucht */
        vorg_flaeche = GetPlain(flaeche->vorgaenger);
        if (flaeche->nachbarn_3D[1]<0l)  /* "benachbartes" 3D-Elem. exist. */
          if (vorg_flaeche->nachbarn_3D[1]>=0l) { /* und ist groeber als ich */
            aktiv = ElGetElementActiveState(vorg_flaeche->nachbarn_3D[1]);
            if (aktiv == 1l)    
              R3D_a_3D(vorg_flaeche->nachbarn_3D[1]); 
          } /* ==> verfeinern */
        if (flaeche->nachbarn_3D[0]<0l)  /* "benachbartes" 3D-Elem. exist. */
          if (vorg_flaeche->nachbarn_3D[0]>=0l) { /* und ist groeber als ich */
            aktiv = ElGetElementActiveState(vorg_flaeche->nachbarn_3D[0]);
            if (aktiv == 1l)               
              R3D_a_3D(vorg_flaeche->nachbarn_3D[0]);
          }    /* ==> verfeinern */
        if (flaeche->nachbar_2D<0l)   /* "benachbartes" 2D-Element exist. */
          if (vorg_flaeche->nachbar_2D>=0l) { /* und ist groeber als ich */
            aktiv = ElGetElementActiveState(vorg_flaeche->nachbar_2D);
            if (aktiv == 1l)                     
              R3D_a_2D(vorg_flaeche->nachbar_2D);  /* ==> verfeinern */
          }
        /* 1D-Nachbarn muessen nicht betrachtet werden, da hier nur
           primaere Kriterien ueberprueft werden. Zwei 1D-Straenge
           duerfen nicht aufeinander liegen. Kreuzende 1D-Straenge oder
           parallel (gegenueberliegende Flaechenseite) liegende
           erfordern primaer keine Verfeinerung, sondern erst ueber
           sekundaere Kriterien. */
      }
      /* Jetzt sind alle Nachbarn so fein wie ich
          ==> ich kann die Flaechen und mich verfeinern! */
    }
  }  /* Schleifenende schon hier, damit mehr statische Variablen moeglich */

  alte_eckknoten = ElGetElementNodes(number);
    /* Knoten von number */
  neue_elemente = (long *) Malloc(2*sizeof(long));
    /* sind spaeter die Kinder von number */
  for (i=0;i<2;i++)  /* neue Elemente erzeugen */
    neue_elemente[i] = ElCreateElement(1,number,-1);

  level = ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)+2l));

  /* ref_level_anz[ElGetElementLevel(neue_elemente[0])-1] += 2l;  */

    /* Anzahl der verfeinerten Elemente eintragen */
  /* vorhandene Kantenknoten zwischenspeichern bzw. erzeugen */
  neuer_knoten = CheckNodeExist3D(alte_eckknoten[0],alte_eckknoten[1]);
  /* Element-zu-Knoten-Liste fuer number anpassen */
  ReAssign1DElementNodes(number,alte_eckknoten,2);

  for (i=0;i<2;i++) {  /* Schleife ueber die Kindelemente */
    /* Knoten bei Kindelementen eintragen */
    knotenliste = (long *) Malloc(2*sizeof(long));
    knotenliste[0] = alte_eckknoten[i];
    knotenliste[1] = neuer_knoten;
    ElSetElementNodes(neue_elemente[i],knotenliste);
    Assign1DElementNodes(neue_elemente[i],knotenliste,2);
  }

  /* Vorgaenger */
  ElSetElementActiveState(number,0);                      /* deaktivieren */
  ElSetElementChilds(number,neue_elemente);           /* Kinder eintragen */
  ElSetElementRefineState(number,0);           /* Verfeinern-Schalter aus */
  /* Anzahl Aktive Elemente anpassen (+2 neue -1 altes) */
  anz_active_elements += 1l;
  anz_active_1D += 1l;
  anz_1D += 2l;
  msh_no_line+=2l;

  /* Knotenstatus (RB) ueberpruefen */
  CheckNodeState3D(neuer_knoten,alte_eckknoten[0],alte_eckknoten[1]);

    /* ueberprueft und setzt Status eines Kantenknotens */
  /* Speicher fuer Kindflaechen allokieren */
  flaechenliste0 = (long *) Malloc(fanz*sizeof(long));
  flaechenliste1 = (long *) Malloc(fanz*sizeof(long));
  /* angrenzende Flaechen eintragen. Fuer noch nicht erzeugte Flaechen wird
     -1 eingetragen. Die Nummer der Flaeche muss dann beim Erzeugen derselben
     nachgetragen werden. */
  for (i=0;i<fanz;i++) {  /* Schleife ueber alle angrenzenden
                             Ausgangsflaechen */
    flaeche = GetPlain(flaechen[i]);
    if (flaeche->kinder[0]>=0l) {
      /* Diese Flaeche wurde bereits verfeinert; jetzt muessen die beiden
         an die neuen 1D-Elemente angrenzenden Kindflaechen bestimmt
         werden. */
      k = 0;
      j = 0;
      while /* ((j<4) && */ (k<2) /* ) */ {  /* Schleife ueber die
                                                Flaechenkinder */
        kflaeche = GetPlain(flaeche->kinder[j]);
        if ((ii = ComparePlains1D(kflaeche->knoten,ElGetElementNodes(neue_elemente[0])))>=0) {
          /* Kindelement[0] grenzt an Kindflaeche[j] an Position ii */
          flaechenliste0[i] = flaeche->kinder[j];
          kflaeche->nachbarn_1D[ii] = neue_elemente[0];
          k++;
        }
        else if ((ii = ComparePlains1D(kflaeche->knoten,ElGetElementNodes(neue_elemente[1])))>=0) {
          /* Kindelement[1] grenzt an Kindflaeche[j] an Position ii */
          flaechenliste1[i] = flaeche->kinder[j];
          kflaeche->nachbarn_1D[ii] = neue_elemente[1];
          k++;
        }
        j++;
      }
    }
    else {
      flaechenliste0[i] = -1;
      flaechenliste1[i] = -1;
    }
  }
  /* Kindflaechen bei Kindelementen eintragen */
  ElSetElementPlains(neue_elemente[0],flaechenliste0,fanz);
  ElSetElementPlains(neue_elemente[1],flaechenliste1,fanz);
  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);
  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */
  /* evtl. Nachlauf */
  if (nachlauf) {
    /* Prueflauf ueber alle groeberen Nachbarn, ob die nach der
       Verfeinerung von number evtl. auch verfeinert werden muessen. */
    for (i=0;i<fanz;i++) {  /* Schleife ueber alle Flaechen */
      flaeche = GetPlain(flaechen[i]);  /* Flaeche holen */
      if (flaeche->nachbarn_3D[1]>=0l) {  /* Nachbar existiert */
        if (ElGetElementChilds(flaeche->nachbarn_3D[1])==NULL)
           /* und ist groeber als ich */
           R3D_a_3D(flaeche->nachbarn_3D[1]);
      }
      if (flaeche->nachbarn_3D[0]>=0l) {  /* Nachbar existiert */
        if (ElGetElementChilds(flaeche->nachbarn_3D[0])==NULL)
           /* und ist groeber als ich */
           R3D_a_3D(flaeche->nachbarn_3D[0]);
      }
      if (flaeche->nachbar_2D>=0l) {  /* Nachbar existiert */
        if (ElGetElementChilds(flaeche->nachbar_2D)==NULL)
           /* und ist groeber als ich */
           R3D_a_2D(flaeche->nachbar_2D);
      }
      for (j=0;j<4;j++) {
        if (flaeche->nachbarn_1D[j]>=0l) {  /* Nachbar existiert */
          if (ElGetElementChilds(flaeche->nachbarn_1D[j])==NULL)
           /* und ist groeber als ich */
           R3D_a_1D(flaeche->nachbarn_1D[j]);
        }
      }
    }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R3D_a_2D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void R3D_a_2D ( long number )
{
  int i, j, kk;  /* Laufvariablen */
  long *flaechen = NULL;  /* Feld mit Elementflaechen von number */
  int level, aktiv;
  Flaeche *flaeche = NULL;  /* Eine (Nachbar-)Flaeche */
  Flaeche *vorg_flaeche = NULL;  /* Vorgaenger einer (Nachbar-)Flaeche */
  STATIC Knoten *knoten;  /* Zeiger auf Knotenstruktur des Mittelknotens */
  STATIC Knoten *eckknoten;  /* Zeiger auf Struktur eines Eckknoten */
  STATIC long *alte_eckknoten;  /* Feld mit Knotennummern von number */
  STATIC int ii, k;  /* Laufvariablen */
  STATIC long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  STATIC long *flaechenliste;  /* Feld mit Flaechen der neuen Elemente */
  STATIC long neue_knoten[5];  /* Feld mit Knotennummern der neuen Randknoten */
  STATIC Flaeche *kflaeche[4];  /* Zeiger auf die 4 neuen Flaechen */
  STATIC long *kinder1D;  /* Zeiger auf die Kinder eines 1D-Nachbarn der Flaeche */
  STATIC long *kplains;  /* Hilfszeiger auf Elementflaechen */
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  flaeche = GetPlain(flaechen[0]);
  if (flaeche->kinder[0]<0l) {
      /* Diese Flaeche wurde noch nicht verfeinert! */
      if (flaeche->vorgaenger>=0l) {  /* es exist. ein Vorgaenger */
          /* Vorgaenger der Flaeche holen, wird wahrscheinlich gebraucht */
          vorg_flaeche = GetPlain(flaeche->vorgaenger);
          if (flaeche->nachbarn_3D[1]<0l)  /* "benachbartes" 3D-Elem. exist. */
              if (vorg_flaeche->nachbarn_3D[1]>=0l) { /* und ist groeber als ich */
                aktiv = ElGetElementActiveState(vorg_flaeche->nachbarn_3D[1]);
                if (aktiv == 1l)              
                  R3D_a_3D(vorg_flaeche->nachbarn_3D[1]);
              }      /* ==> verfeinern */
          if (flaeche->nachbarn_3D[0]<0l)  /* "benachbartes" 3D-Elem. exist. */
              if (vorg_flaeche->nachbarn_3D[0]>=0l) { /* und ist groeber als ich */
                aktiv = ElGetElementActiveState(vorg_flaeche->nachbarn_3D[0]);
                if (aktiv == 1l)    
                  R3D_a_3D(vorg_flaeche->nachbarn_3D[0]);
              }      /* ==> verfeinern */
          for (j=0;j<4;j++) {
              if (vorg_flaeche->nachbarn_1D[j]>=0l) {  /* "benachbartes" 1D-Element exist. */
                  if (ElGetElementChilds(vorg_flaeche->nachbarn_1D[j])==NULL) { /* und ist groeber als ich */
                    aktiv = ElGetElementActiveState(vorg_flaeche->nachbarn_1D[j]);
                    if (aktiv == 1l)        
                      R3D_a_1D(vorg_flaeche->nachbarn_1D[j]);
                  }      /* ==> verfeinern */
              }
          }
      }
      /*
         Jetzt sind alle Nachbarn so fein wie ich
         ==> ich kann die Flaeche und mich verfeinern!
      */
  }
  alte_eckknoten = ElGetElementNodes(number);
    /* Knoten von number (koennen andersrum nummeriert sein
       als die der Flaeche !!! */
  neue_elemente = (long *) Malloc(4*sizeof(long));
    /* sind spaeter die Kinder von number */
  for (i=0;i<4;i++)  /* neue Elemente erzeugen */
      neue_elemente[i] = ElCreateElement(2,number,-1);


  level = ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)+4l));


  /* ref_level_anz[ElGetElementLevel(neue_elemente[0])-1] += 4l;*/
    /* Anzahl der verfeinerten Elemente eintragen */
  /* vorhandene Kantenknoten zwischenspeichern bzw. erzeugen */
  for (i=0;i<4;i++)  /* Schleife ueber die 4 Kanten */
      neue_knoten[i] = CheckNodeExist3D(flaeche->knoten[i],flaeche->knoten[(i+1)%4]);
  if (flaeche->kinder[0]<0l) {  /* Flaeche verfeinern */
      /* Mittelknoten erzeugen */
      knoten = NewNode();
      for (j=0;j<4;j++) {
          eckknoten = GetNode(flaeche->knoten[j]);
          knoten->x += eckknoten->x;
          knoten->y += eckknoten->y;
          knoten->z += eckknoten->z;
      }
      knoten->x /= 4.0;
      knoten->y /= 4.0;
      knoten->z /= 4.0;
      neue_knoten[4] = AddNode(knoten);
      /* modellspezifische Knotendaten verfeinern */
      RefinePlainValues(knoten,flaeche->knoten);

      /* Kindflaechen erzeugen */
      for (j=0;j<4;j++) {
          kflaeche[j] = NewPlain();
          /* Knoten eintragen */
          kflaeche[j]->knoten[0] = flaeche->knoten[j];
          kflaeche[j]->knoten[1] = neue_knoten[j];
          kflaeche[j]->knoten[2] = neue_knoten[4];
          kflaeche[j]->knoten[3] = neue_knoten[(j+3)%4];
          /* Vorgaenger und Kinder eintragen */
          kflaeche[j]->vorgaenger = flaechen[0];
          flaeche->kinder[j] = AddPlain(kflaeche[j]);
      }
      /* ggf. vorhandene 1D-Elemente eintragen */
      for (i=0;i<4;i++) {  /* Schleife ueber die potentiellen 1D-Flaechennachbarn */
          if (flaeche->nachbarn_1D[i]>=0l) {  /* 1D-Nachbar existiert */
              if ((kinder1D = ElGetElementChilds(flaeche->nachbarn_1D[i]))!=NULL) {  /* und ist verfeinert */
                  /* Index der Flaechen beim 1D-Element bestimmen */
                  kplains = ElGetElementPlains(flaeche->nachbarn_1D[i]);
                  kk = 0;
                  while (
#ifdef ERROR_CONTROL
                          (kk<ElGetElementPlainsNumber(flaeche->nachbarn_1D[i])) &&
#endif
                          (flaechen[0]!=kplains[kk]) )
                      kk++;
#ifdef ERROR_CONTROL
                    if (kk==ElGetElementPlainsNumber(flaeche->nachbarn_1D[i])) {
                        DisplayErrorMsg("!!! R3D_a_2D: Fehler: 1D-Element haengt nicht an Flaeche !");
                        abort();
                    }
#endif
                  k = 0;
                  j = 0;
                  while (k<2) {  /* Schleife ueber die Flaechenkinder */
                      if ((ii = ComparePlains1D(kflaeche[j]->knoten,ElGetElementNodes(kinder1D[0])))>=0) {
                          /* Kindelement[0] grenzt an Kindflaeche[j] an Position ii */
                          kplains = ElGetElementPlains(kinder1D[0]);
                          kplains[kk] = flaeche->kinder[j];
                          kflaeche[j]->nachbarn_1D[ii] = kinder1D[0];
                          k++;
                      }
                      else if ((ii = ComparePlains1D(kflaeche[j]->knoten,ElGetElementNodes(kinder1D[1])))>=0) {
                          /* Kindelement[1] grenzt an Kindflaeche[j] an Position ii */
                          kplains = ElGetElementPlains(kinder1D[1]);
                          kplains[kk] = flaeche->kinder[j];
                          kflaeche[j]->nachbarn_1D[ii] = kinder1D[1];
                          k++;
                      }
                      j++;
                  }
              }
          }
      }
  }
  else {  /* Flaeche existiert bereits */
      for (j=0;j<4;j++) {
          /* Neue Flaechenzeiger holen */
          kflaeche[j] = GetPlain(flaeche->kinder[j]);
      }
      /* Flaechenmittelknoten in Zwischenspeicher eintragen */
      neue_knoten[4] = kflaeche[0]->knoten[2];
  }
  /*
     Es existiert eine verfeinerte Flaechen; die neuen Flaechen und die neuen
     Knoten sind bekannt und (zwischen-)gespeichert.
  */
  /* Element-zu-Knoten-Liste fuer number anpassen */
  ReAssign2DElementNodes(number,alte_eckknoten,4);
  for (i=0;i<4;i++) {  /* Schleife ueber die Kindelemente */
      knotenliste = (long *) Malloc(4*sizeof(long));
      knotenliste[0] = alte_eckknoten[i];
      knotenliste[2] = neue_knoten[4];
      flaechenliste = (long *) Malloc(sizeof(long));
      if (ComparePlains(flaeche->knoten,alte_eckknoten,-1)==1) {  /* Drehsinn positiv */
          /* Eckknoten suchen */
          k = 0;
          while (alte_eckknoten[i]!=flaeche->knoten[k])
              k++;
          /* Knoten eintragen */
          knotenliste[1] = neue_knoten[k];
          knotenliste[3] = neue_knoten[(k+3)%4];
          /* Kindflaeche bei Kindelement eintragen */
          flaechenliste[0] = flaeche->kinder[k];
          ElSetElementPlains(neue_elemente[i],flaechenliste,-1);
          /* Kindelement bei Kindflaeche eintragen */
          kflaeche[k]->nachbar_2D = neue_elemente[i];
      }
#ifdef ERROR_CONTROL
        else if (ComparePlains(flaeche->knoten,alte_eckknoten,-1)==0) {
            DisplayErrorMsg("!!! R3D_a_2D: Fehler: Flaeche passt nicht zu Element !");
            abort();
        }
#endif
      else {  /* Drehsinn negativ; ComparePlains==2 */
          /* Eckknoten suchen */
          k = 0;
          while (alte_eckknoten[i]!=flaeche->knoten[k])
              k++;
          /* Knoten eintragen */
          knotenliste[1] = neue_knoten[(k+3)%4];
          knotenliste[3] = neue_knoten[k];
          /* Flaeche eintragen */
          flaechenliste[0] = flaeche->kinder[k];
          ElSetElementPlains(neue_elemente[i],flaechenliste,-1);
          /* Kindelement bei Kindflaeche eintragen */
          kflaeche[k]->nachbar_2D = neue_elemente[i];
      }
      ElSetElementNodes(neue_elemente[i],knotenliste);
      Assign2DElementNodes(neue_elemente[i],knotenliste,4);
  }
  /* Vorgaenger */
  ElSetElementActiveState(number,0);                      /* deaktivieren */
  ElSetElementChilds(number,neue_elemente);           /* Kinder eintragen */
  ElSetElementRefineState(number,0);           /* Verfeinern-Schalter aus */
  /* Anzahl Aktive Elemente anpassen (+4 neue -1 altes) */
  anz_active_elements += 3l;
  anz_active_2D += 3l;
  anz_2D += 4l;
  msh_no_quad +=4; //SB
  /* Knotenstati (RB) ueberpruefen */
  for (j=0;j<4;j++)
    CheckNodeState3D(kflaeche[j]->knoten[1],
                      flaeche->knoten[j],
                      flaeche->knoten[(j+1)%4]);

  /* Flaechenmittelknoten: */
  CheckNodeStateA3D(kflaeche[0]->knoten[2],flaeche);

  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);
  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */
  /* evtl. Nachlauf */
  if (nachlauf) {
      /* Prueflauf ueber alle groeberen Nachbarn, ob die nach der
         Verfeinerung von number evtl. auch verfeinert werden muessen. */
      if (flaeche->nachbarn_3D[1]>=0l) {  /* Nachbar existiert */
          if (ElGetElementChilds(flaeche->nachbarn_3D[1])==NULL)
              /* und ist groeber als ich */
              R3D_a_3D(flaeche->nachbarn_3D[1]);
      }
      if (flaeche->nachbarn_3D[0]>=0l) {  /* Nachbar existiert */
          if (ElGetElementChilds(flaeche->nachbarn_3D[0])==NULL)
              /* und ist groeber als ich */
              R3D_a_3D(flaeche->nachbarn_3D[0]);
      }
      for (j=0;j<4;j++) {
          if (flaeche->nachbarn_1D[j]>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(flaeche->nachbarn_1D[j])==NULL)
                  /* und ist groeber als ich */
                  R3D_a_1D(flaeche->nachbarn_1D[j]);
          }
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R3D_a_3D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer 3D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
   21.10.1997  R.Kaiser   Fehler korrigiert (RefPlainCenterNode3D)

   letzte Aenderung:     R.Kaiser    21.10.1997
                                                                          */
/**************************************************************************/
void R3D_a_3D ( long number )
{
  int i,j, kk; /* Laufvariablen */
  long *flaechen = NULL;  /* Feld mit Elementflaechen von number */
  Flaeche *flaeche = NULL;  /* Eine (Nachbar-)Flaeche */
  /*Flaeche *vorg_flaeche = NULL;*/  /* Vorgaenger einer (Nachbar-)Flaeche */
  STATIC Flaeche *nflaeche;  /* Eine neue Flaeche */
  STATIC Knoten *knoten;  /* Zeiger auf Knotenstruktur des Mittelknotens */
  STATIC Knoten *eckknoten;  /* Zeiger auf Struktur eines Eckknoten */
  STATIC long *alte_eckknoten;  /* Feld mit Knotennummern von number */
  STATIC long *alte_flaechenknoten; /* Feld mit Knotennummern der Elementflaechen */
  STATIC int ii, k, ik;  /* Laufvariablen */
  STATIC long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  STATIC long *flaechenliste;  /* Feld mit Flaechen der neuen Elemente */
  STATIC long neue_knoten[27];  /* Feld mit Knotennummern der neuen Randknoten */
  STATIC long zw_flaechenliste[36];
    /* Feld mit den 24 neuen, verfeinerten Randflaechen im Umlaufsinn und den 12
       neuen Innenflaechen */
  STATIC Flaeche *kflaeche[24];
    /* Zeiger auf die ersten zu zw_flaechenliste korrespondierenden Flaechen
       (Aussenflaechen) */
  STATIC long *kinder1D;  /* Hilfsvariablen zum Eintragen der 1D-Elemente */
  STATIC long *kplains;  /* "" */
  STATIC Flaeche *fkind;  /* "" */
  STATIC long *elem_2d, *elem_1d, *elem_3d;
  STATIC int anzahl_2d, anzahl_1d, anzahl_3d;
  long *node_3d_elems, *node_2d_elems, *node_1d_elems;
  int anzahl_node_3d_elems, anzahl_node_2d_elems, anzahl_node_1d_elems;
  int level, aktiv;

  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */

  elem_1d = GetNode1DElems(ElGetElementNodes(number)[0],&anzahl_1d);
  elem_2d = GetNode2DElems(ElGetElementNodes(number)[0],&anzahl_2d);
  elem_3d = GetNode3DElems(ElGetElementNodes(number)[0],&anzahl_3d);

  anzahl_node_1d_elems = anzahl_1d;
  anzahl_node_2d_elems = anzahl_2d;
  anzahl_node_3d_elems = anzahl_3d;
  node_1d_elems = (long*) Malloc(anzahl_node_1d_elems*sizeof(long));
  node_2d_elems = (long*) Malloc(anzahl_node_2d_elems*sizeof(long));
  node_3d_elems = (long*) Malloc(anzahl_node_3d_elems*sizeof(long));

  for (j=0;j<anzahl_node_1d_elems;j++) {
    node_1d_elems[j] = elem_1d[j];
  }

  for (j=0;j<anzahl_node_2d_elems;j++) {
    node_2d_elems[j] = elem_2d[j];
  }

  for (j=0;j<anzahl_node_3d_elems;j++) {
    node_3d_elems[j] = elem_3d[j];
  }

  for (i=0;i<anzahl_node_1d_elems;i++) {
    if (ElGetElementLevel(node_1d_elems[i]) < ElGetElementLevel(number)) {
      aktiv = ElGetElementActiveState(node_1d_elems[i]);
      if (aktiv == 1l)           
        R3D_a_1D(node_1d_elems[i]); /* groebere 1d-Nachbarelemente verfeinern */
    }
  }

  for (i=0;i<anzahl_node_2d_elems;i++) {
    if (ElGetElementLevel(node_2d_elems[i]) < ElGetElementLevel(number)) {
      aktiv = ElGetElementActiveState(node_2d_elems[i]);
       if (aktiv == 1l)              
         R3D_a_2D(node_2d_elems[i]); /* groebere 2d-Nachbarelemente verfeinern */
    }
  }

  for (i=0;i<anzahl_node_3d_elems;i++) {
    if (ElGetElementLevel(node_3d_elems[i]) < ElGetElementLevel(number)) {
      aktiv = ElGetElementActiveState(node_3d_elems[i]);
       if (aktiv == 1l)       
        R3D_a_3D(node_3d_elems[i]);  /* groebere 3d_Nachbarelemente verfeinern */
    }
  }

  alte_eckknoten = ElGetElementNodes(number);

    /* Knoten von number */
  for (i=0;i<8;i++)
      neue_knoten[19+i] = alte_eckknoten[i];
    /* Ausgangsknoten werden auch in neue_knoten benoetigt */
  neue_elemente = (long *) Malloc(8*sizeof(long));
    /* sind spaeter die Kinder von number */
  for (i=0;i<8;i++)  /* neue Elemente erzeugen */
      neue_elemente[i] = ElCreateElement(3,number,-1);

  level = ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)+8l));

  /* ref_level_anz[ElGetElementLevel(neue_elemente[0])-1] += 8l; */

  /* Anzahl der verfeinerten Elemente eintragen */
  /* vorhandene Kantenknoten zwischenspeichern bzw. erzeugen */
  for (i=0;i<4;i++)  /* Schleife ueber die 4 oberen Kanten */
      neue_knoten[4+i] = CheckNodeExist3D(alte_eckknoten[i],alte_eckknoten[(i+1)%4]);
  for (i=0;i<4;i++)  /* Schleife ueber die 4 unteren Kanten */
      neue_knoten[12+i] = CheckNodeExist3D(alte_eckknoten[4+i],alte_eckknoten[4+((i+1)%4)]);
  for (i=0;i<4;i++)  /* Schleife ueber die 4 vertikalen Kanten */
      neue_knoten[i] = CheckNodeExist3D(alte_eckknoten[i],alte_eckknoten[i+4]);
  for (i=0;i<6;i++) {  /* Schleife ueber alle Flaechen */
      flaeche = GetPlain(flaechen[i]);
      if (flaeche->kinder[0]<0l) {  /* Flaeche verfeinern */
          /* Mittelknoten erzeugen */
          knoten = NewNode();
          alte_flaechenknoten = (long *) Malloc(4*sizeof(long));
          for (j=0;j<4;j++) {
              eckknoten = GetNode(alte_eckknoten[fk[i][j]]);
              alte_flaechenknoten[j] = alte_eckknoten[fk[i][j]];
              knoten->x += eckknoten->x;
              knoten->y += eckknoten->y;
              knoten->z += eckknoten->z;
          }
          knoten->x /= 4.0;
          knoten->y /= 4.0;
          knoten->z /= 4.0;

          neue_knoten[nmk_idx[i]] = AddNode(knoten);
          /* modellspezifische Knotendaten verfeinern */
          RefinePlainValues(knoten,alte_flaechenknoten);

          /* Kindflaechen erzeugen */
            /* Die Knoten sind in neue_knoten in Bezug auf number
               nummeriert. Die Ausgangsflaeche kann positiven oder
               negativen Umlaufsinn haben. Deshalb muessen die Knoten
               in unterschiedlichen Reihenfolgen eingetragen werden. */
          if (number==flaeche->nachbarn_3D[0]) {  /* positiv */
              for (j=0;j<4;j++) {
                  nflaeche = NewPlain();
                  /* Knoten eintragen */
                  k = 0;
                  while (
#ifdef ERROR_CONTROL
                          (k<4) &&
#endif
                          (neue_knoten[nfk_idxp[i*4+k][0]]!=flaeche->knoten[j]) )
                      k++;
#ifdef ERROR_CONTROL
                    if (k==4) {
                        DisplayErrorMsg("!!! Fehler: R3D_a_3D: Eckknoten nicht gefunden !!!");
                        abort();
                    }
#endif
                  for (ii=0;ii<4;ii++)  /* Schleife ueber die 4 Flaechenknoten */
                      nflaeche->knoten[ii] = neue_knoten[nfk_idxp[4*i+k][ii]];
                  /* Vorgaenger und Kinder eintragen */
                  nflaeche->vorgaenger = flaechen[i];
                  flaeche->kinder[j] = AddPlain(nflaeche);
              }
          }
          else { /* negativ */
              for (j=0;j<4;j++) {
                  nflaeche = NewPlain();
                  /* Knoten eintragen */
                  k = 0;
                  while (
#ifdef ERROR_CONTROL
                          (k<4) &&
#endif
                          (neue_knoten[nfk_idxp[i*4+k][0]]!=flaeche->knoten[j]) )
                      k++;
#ifdef ERROR_CONTROL
                    if (k==4) {
                        DisplayErrorMsg("!!! Fehler: R3D_a_3D: Eckknoten nicht gefunden !!!");
                        abort();
                    }
#endif
                  /* Eintragen der 4 Flaechenknoten */
                  nflaeche->knoten[0] = neue_knoten[nfk_idxp[4*i+k][0]];
                  nflaeche->knoten[1] = neue_knoten[nfk_idxp[4*i+k][3]];
                  nflaeche->knoten[2] = neue_knoten[nfk_idxp[4*i+k][2]];
                  nflaeche->knoten[3] = neue_knoten[nfk_idxp[4*i+k][1]];
                  /* Vorgaenger und Kinder eintragen */
                  nflaeche->vorgaenger = flaechen[i];
                  flaeche->kinder[j] = AddPlain(nflaeche);
              }
          }
          /* ggf. vorhandene 1D-Elemente eintragen */
          for (ik=0;ik<4;ik++) {  /* Schleife ueber die potentiellen 1D-Flaechennachbarn */
              if (flaeche->nachbarn_1D[ik]>=0l) {  /* 1D-Nachbar existiert */
                  if ((kinder1D = ElGetElementChilds(flaeche->nachbarn_1D[ik]))!=NULL) {  /* und ist verfeinert */
                      /* Index der Flaechen beim 1D-Element bestimmen */
                      kplains = ElGetElementPlains(flaeche->nachbarn_1D[ik]);
                      kk = 0;
                      while (
#ifdef ERROR_CONTROL
                              (kk<ElGetElementPlainsNumber(flaeche->nachbarn_1D[ik])) &&
#endif
                              (flaechen[i]!=kplains[kk]) )
                          kk++;
#ifdef ERROR_CONTROL
                        if (kk==ElGetElementPlainsNumber(flaeche->nachbarn_1D[ik])) {
                            DisplayErrorMsg("!!! R3D_a_3D: Fehler: 1D-Element haengt nicht an Flaeche !");
                            abort();
                        }
#endif
                      k = 0;
                      j = 0;
                      while (k<2) {  /* Schleife ueber die Flaechenkinder */
                          fkind = GetPlain(flaeche->kinder[j]);
                          if ((ii = ComparePlains1D(fkind->knoten,ElGetElementNodes(kinder1D[0])))>=0) {
                              /* Kindelement[0] grenzt an Kindflaeche[j] an Position ii */
                              kplains = ElGetElementPlains(kinder1D[0]);
                              kplains[kk] = flaeche->kinder[j];
                              fkind->nachbarn_1D[ii] = kinder1D[0];
                              k++;
                          }
                          else if ((ii = ComparePlains1D(fkind->knoten,ElGetElementNodes(kinder1D[1])))>=0) {
                              /* Kindelement[1] grenzt an Kindflaeche[j] an Position ii */
                              kplains = ElGetElementPlains(kinder1D[1]);
                              kplains[kk] = flaeche->kinder[j];
                              fkind->nachbarn_1D[ii] = kinder1D[1];
                              k++;
                          }
                          j++;
                      }
                  }
              }
          }
      }
      for (j=0;j<4;j++) {
          /* neue Aussenflaechen in Zwischenspeicher eintragen */
            /* Die Kindflaechen werden in zw_flaechenliste im positiven
               Umlaufsinn bzgl. des Elements eingetragen */
          nflaeche = GetPlain(flaeche->kinder[j]);
          k = 0;
          while (
#ifdef ERROR_CONTROL
                  (k<4) &&
#endif
                  (alte_eckknoten[fk[i][/*j*/k]]!=nflaeche->knoten[/*k*/0]) )
              k++;
#ifdef ERROR_CONTROL
            if (k==4) {
                DisplayErrorMsg("!!! Fehler: R3D_a_3D: Kindflaeche nicht Kindflaeche !!!");
                abort();
            }
#endif
          zw_flaechenliste[i*4+k] = flaeche->kinder[j];
      }
      for (j=0;j<4;j++) {
          /* Neue Aussenflaechen holen */
          kflaeche[i*4+j] = GetPlain(zw_flaechenliste[i*4+j]);
      }
      /* Flaechenmittelknoten in Zwischenspeicher eintragen */
      neue_knoten[nmk_idx[i]] = kflaeche[i*4]->knoten[2];
        /* ist fuer neue Knoten bereits eingetragen */
  }
  /*
     Es existieren ueberall verfeinerte Flaechen; die neuen Flaechen sind im
     Zwischenspeicher eingetragen; die neuen Knoten bis auf den zentralen auch.
  */
  /* Mittelknoten erzeugen (ist immer regulaerer Innenknoten) */
  knoten = NewNode();
  for (i=0;i<8;i++) {
      eckknoten = GetNode(alte_eckknoten[i]);
      knoten->x += eckknoten->x;
      knoten->y += eckknoten->y;
      knoten->z += eckknoten->z;
  }
  knoten->x /= 8.0;
  knoten->y /= 8.0;
  knoten->z /= 8.0;
  neue_knoten[17] = AddNode(knoten);
  /* modellspezifische Knotendaten verfeinern */
  Refine3DElementValues(knoten,alte_eckknoten);


  /* Innere Flaechen erzeugen (4 vertikal oben) */
  for (i=0;i<4;i++) {
      flaeche = NewPlain();
      /* Knoten eintragen */
      flaeche->knoten[0] = neue_knoten[4+i];
      flaeche->knoten[1] = neue_knoten[16];
      flaeche->knoten[2] = neue_knoten[17];
      flaeche->knoten[3] = neue_knoten[8+i];
      zw_flaechenliste[24+i] = AddPlain(flaeche);
  }
  /* Innere Flaechen erzeugen (4 horizontal mitte) */
  for (i=0;i<4;i++) {
      flaeche = NewPlain();
      /* Knoten eintragen */
      flaeche->knoten[0] = neue_knoten[i];
      flaeche->knoten[1] = neue_knoten[8+i];
      flaeche->knoten[2] = neue_knoten[17];
      flaeche->knoten[3] = neue_knoten[8+((i+3)%4)];
      zw_flaechenliste[28+i] = AddPlain(flaeche);
  }
  /* Innere Flaechen erzeugen (4 vertikal unten) */
  for (i=0;i<4;i++) {
      flaeche = NewPlain();
      /* Knoten eintragen */
      flaeche->knoten[0] = neue_knoten[12+i];
      flaeche->knoten[1] = neue_knoten[8+i];
      flaeche->knoten[2] = neue_knoten[17];
      flaeche->knoten[3] = neue_knoten[18];
      zw_flaechenliste[32+i] = AddPlain(flaeche);
  }
  /*
     Saemtliche neu zu erzeugenden Knoten, Flaechen und Elemente sind
     erzeugt. Die Nummern der Flaechen und Knoten stehen in den
     Zwischenspeichern.

     Belegungsplan zw_flaechenliste: Fuer alle 6 Flaechen hintereinander
                                     die jeweiligen Kinder => 24 Kindflaechen.
                                     Es folgen die 4 vertikalen oberen,
                                     die 4 horizontalen mittleren und die 4
                                     vertikalen unteren Innenflaechen.
     Belegungsplan neue_knoten: 4 neue Knoten auf den vertikalen Aussenkanten,
                                4 Knoten auf den oberen horizontalen Ausgangskanten,
                                4 Flaechenmittelknoten der vertikalen Aussenflaechen,
                                4 Knoten auf den unteren horizontalen Ausgangskanten,
                                der obere Flaechenmittelknoten,
                                der Gesamt-Mittelknoten (Nr. 17),
                                der untere Flaechenmittelknoten,
                                die 8 Ausgangsknoten.
  */
  /* Element-zu-Knoten-Liste fuer number anpassen */
  ReAssign3DElementNodes(number,alte_eckknoten,8);
  /* Knoten bei Kindelementen eintragen */
  for (i=0;i<8;i++) {  /* Schleife ueber die Kindelemente */
      knotenliste = (long *) Malloc(8*sizeof(long));
      for (j=0;j<8;j++) {  /* Schleife ueber die Knoten der Kinder */
          knotenliste[j] = neue_knoten[kikno_idx[i][j]];
      }
      ElSetElementNodes(neue_elemente[i],knotenliste);
      Assign3DElementNodes(neue_elemente[i],knotenliste,8);
  }
  /* Kindflaechen bei Kindelementen eintragen */
  for (i=0;i<8;i++) {  /* Schleife ueber die Kindelemente */
      flaechenliste = (long *) Malloc(6*sizeof(long));
      for (j=0;j<6;j++)  /* Schleife ueber die Flaechen der Kinder */
          flaechenliste[j] = zw_flaechenliste[kifla_idx[i][j]];
      ElSetElementPlains(neue_elemente[i],flaechenliste,-1);
  }
  /* Kindelemente bei Kindflaechen eintragen */
  for (i=0;i<8;i++) {  /* Schleife ueber alle Kindelemente */
      flaechenliste = ElGetElementPlains(neue_elemente[i]);  /* koennte man auch */
      knotenliste = ElGetElementNodes(neue_elemente[i]);  /* vorher zwischenspeichern */
      for (j=0;j<6;j++) {  /* Schleife ueber alle Flaechen */
          flaeche = GetPlain(flaechenliste[j]);
          flaeche->nachbarn_3D[PDrehsinn(flaeche->knoten,knotenliste,j)] =
            neue_elemente[i];
      }
  }
  /* Vorgaenger */
  ElSetElementActiveState(number,0);                      /* deaktivieren */
  ElSetElementChilds(number,neue_elemente);           /* Kinder eintragen */
  ElSetElementRefineState(number,0);           /* Verfeinern-Schalter aus */
  /* Anzahl Aktive Elemente anpassen (+8 neue -1 altes) */
  anz_active_elements += 7;
  anz_active_3D += 7l;
  anz_3D += 8l;
  msh_no_hexs+=8l;
  /* Knotenstati (RB) ueberpruefen */
  for (i=0;i<6;i++) {  /* Schleife ueber alle Flaechen */
      flaeche = GetPlain(flaechen[i]);
      for (j=0;j<4;j++) {
          /* kflaeche neu belegen;
             vorher: positiv bzgl. positiven Elementflaechen
             neu: positiv bzgl. real eingetragenen Elementflaechen */
          kflaeche[i*4+j] = GetPlain(flaeche->kinder[j]);
          /* Kantenmittelknoten ueberpruefen: */
             CheckNodeState3D(kflaeche[i*4+j]->knoten[1],
                               flaeche->knoten[j],
                               flaeche->knoten[(j+1)%4]);



            /* ueberprueft und setzt Status eines Kantenknotens */
      }
      /* Flaechenmittelknoten: */
         CheckNodeStateA3D(kflaeche[i*4]->knoten[2],flaeche);
  }
  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);
  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */
  /* evtl. Nachlauf */
  if (nachlauf) {
      /* Prueflauf ueber alle groeberen Nachbarn, ob die nach der
         Verfeinerung von number evtl. auch verfeinert werden muessen. */
      for (i=0;i<6;i++) {  /* Schleife ueber alle Flaechen */
          flaeche = GetPlain(flaechen[i]);  /* Flaeche holen */
          if (number==flaeche->nachbarn_3D[0]) {  /* ich bin Nachbar[0] */
              if (flaeche->nachbarn_3D[1]>=0l) {  /* Nachbar existiert */
                  if (ElGetElementChilds(flaeche->nachbarn_3D[1])==NULL)
                      /* und ist groeber als ich */
                      RN3D_a_3D(flaeche->nachbarn_3D[1]);
              }
          }
          else {  /* ich bin Nachbar[1] */
              if (flaeche->nachbarn_3D[0]>=0l) {  /* Nachbar existiert */
                  if (ElGetElementChilds(flaeche->nachbarn_3D[0])==NULL)
                      /* und ist groeber als ich */
                      RN3D_a_3D(flaeche->nachbarn_3D[0]);
              }
          }
          if (flaeche->nachbar_2D>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(flaeche->nachbar_2D)==NULL)
                  /* und ist groeber als ich */
                  R3D_a_2D(flaeche->nachbar_2D);
          }
          for (j=0;j<4;j++) {
              if (flaeche->nachbarn_1D[j]>=0l) {  /* Nachbar existiert */
                  if (ElGetElementChilds(flaeche->nachbarn_1D[j])==NULL)
                      /* und ist groeber als ich */
                      R3D_a_1D(flaeche->nachbarn_1D[j]);
              }
          }
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM3D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef3D_a. Es werden nur die Massenerhaltung durchgefuehrt und
   die zu vergroebernden Elemente markiert (Status 4) (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   1: habe meine Kinder vergroebert; 0: habe sie nicht vergroebert
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RRM3D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  STATIC int k;  /* Lauf- und Hilfsvariablen */
  int i, j, ii;  /* Lauf- und Hilfsvariablen */
  int f;  /* Anzahl der angrenzenden Flaechen */
  int r[2];  /* Refine-States */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  long *flaechen = NULL;  /* Feld mit Elementflaechen */
  Flaeche *flaeche = NULL;  /* Eine Nachbarflaeche */
  Flaeche *flaechenkind = NULL;  /* Kind einer Flaeche */

  if (num_ref_quantity == 0) {
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  for (i=0;i<2;i++)  /* Schleife ueber alle Kinder */
      r[i] = ElGetElementRefineState(kinder[i]);  /* verfeinerungsstati */
  /* Wenn alle Kinder auf status==4, sind sie bereits vergroebert und von
     einer Rekursion erfasst worden --> return 1 (wie vergroebert) */
  if ( (r[0]==4) && (r[1]==4) ) {
      rr_ret = 1;
      return 1;
  }
  /* Ueberpruefen, ob alle Kinder vergroebert werden sollen */
  if ( !( (r[0]==-1) && (r[1]==-1) ) ) {
      /* es sollen nicht alle Kinder vergroebert werden */
      /* ==> Vergroebern-Schalter loeschen und Abbruch */
      for (i=0;i<2;i++)  /* Schleife ueber alle Kinder */
          if (r[i]==-1)
              ElSetElementRefineState(kinder[i],0);
      return 0;
  }
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  f = ElGetElementPlainsNumber(number);  /* Anzahl der Flaechen */
  for (i=0;i<f;i++) {  /* Schleife ueber alle Flaechen */
      flaeche = GetPlain(flaechen[i]);
      /* ueberpruefen, ob die direkten Nachbarn meiner Kinder
         Kinder haben */
      for (j=0;j<4;j++) {  /* Schleife ueber die Flaechenkinder */
          flaechenkind = GetPlain(flaeche->kinder[j]);  /* Flaechenkind holen */
          /* Wenn das Nachbarskind Kinder hat, muss es die Kinder
             vergroebern, damit ich meine Kinder vergroebern darf */
          for (ii=0;ii<2;ii++) {  /* Schleife ueber die 3D-Nachbarn */
              if (flaechenkind->nachbarn_3D[ii]>=0l) {  /* wenn 3D-Nachbar existiert */
                  if (ElGetElementChilds(flaechenkind->nachbarn_3D[ii])!=NULL)
                      if (!RRM3D_a_3D(flaechenkind->nachbarn_3D[ii],num_ref_quantity,zwischenspeicher)) {
                          /* Es wurde nicht vergroebert ==> Abbruch */
                          for (k=0;k<2;k++)  /* Schleife ueber alle Kinder */
                              if (r[k]==-1)
                                  ElSetElementRefineState(kinder[k],0);
                          return 0;
                      }
              }
          }
          if (flaechenkind->nachbar_2D>=0l) {  /* wenn 2D-Nachbar existiert */
              if (ElGetElementChilds(flaechenkind->nachbar_2D)!=NULL)
                  if (!RRM3D_a_2D(flaechenkind->nachbar_2D,num_ref_quantity,zwischenspeicher)) {
                      /* Es wurde nicht vergroebert ==> Abbruch */
                      for (k=0;k<2;k++)  /* Schleife ueber alle Kinder */
                          if (r[k]==-1)
                              ElSetElementRefineState(kinder[k],0);
                      return 0;
                  }
          }
          /* 1D-Nachbarn muessen nicht betrachtet werden, da hier nur
             primaere Kriterien ueberprueft werden. Zwei 1D-Straenge
             duerfen nicht aufeinander liegen. Kreuzende 1D-Straenge oder
             parallel (gegenueberliegende Flaechenseite) liegende
             verhindern primaer keine Vergroeberung, sondern erst ueber
             sekundaere Kriterien. */
      }
  }
  /*
     An dieser Stelle komme ich nur an, wenn ich mich vergroebern darf!
     Keines meiner Nachbarelemente ist jetzt noch feiner als ich!
  */
  for (i=0;i<2;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
  rr_ret = 1;

  }
  /* Massenerhaltung */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);

    /* Bei der Massenerhaltung muss beruecksichtigt werden, dass schon ein
       Korrekturwert in conc[1] stehen kann */

  return 1;  /* vergroebern hat geklappt */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM3D_a_2D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef3D_a. Es werden nur die Massenerhaltung durchgefuehrt und
   die zu vergroebernden Elemente markiert (Status 4) (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   1: habe meine Kinder vergroebert; 0: habe sie nicht vergroebert
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RRM3D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  STATIC int k;  /* Lauf- und Hilfsvariablen */
  int i, j, ii, m;  /* Lauf- und Hilfsvariablen */
  int r[8];  /* Refine-States */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  long *flaechen = NULL;  /* Feld mit Elementflaechen */
  Flaeche *flaeche = NULL;  /* Eine Nachbarflaeche */
  Flaeche *flaechenkind = NULL;  /* Kind einer Flaeche */


  if (num_ref_quantity == 0) {
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  for (i=0;i<4;i++)  /* Schleife ueber alle Kinder */
      r[i] = ElGetElementRefineState(kinder[i]);  /* verfeinerungsstati */
  /* Wenn alle Kinder auf status==4, sind sie bereits vergroebert und von
     einer Rekursion erfasst worden --> return 1 (wie vergroebert) */
  if ( (r[0]==4) && (r[1]==4) && (r[2]==4) && (r[3]==4) ) {
      rr_ret = 1;
      return 1;
  }
  /* Ueberpruefen, ob alle Kinder vergroebert werden sollen */
  if ( !( (r[0]==-1) && (r[1]==-1) && (r[2]==-1) && (r[3]==-1) ) ) {
      /* es sollen nicht alle Kinder vergroebert werden */
      /* ==> Vergroebern-Schalter loeschen und Abbruch */
      for (i=0;i<4;i++)  /* Schleife ueber alle Kinder */
          if (r[i]==-1)
              ElSetElementRefineState(kinder[i],0);
      return 0;
  }
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  flaeche = GetPlain(flaechen[0]);
  /* ueberpruefen, ob die direkten Nachbarn meiner Kinder
     Kinder haben */
  for (j=0;j<4;j++) {  /* Schleife ueber die Flaechenkinder */
      flaechenkind = GetPlain(flaeche->kinder[j]);  /* Flaechenkind holen */
      /* Wenn das Nachbarskind Kinder hat, muss es die Kinder
         vergroebern, damit ich meine Kinder vergroebern darf */
      for (ii=0;ii<2;ii++) {  /* Schleife ueber die 3D-Nachbarn */
          if (flaechenkind->nachbarn_3D[ii]>=0l) {  /* wenn 3D-Nachbar existiert */
              if (ElGetElementChilds(flaechenkind->nachbarn_3D[ii])!=NULL) {
                  if (!RRM3D_a_3D(flaechenkind->nachbarn_3D[ii],num_ref_quantity,zwischenspeicher)) {
                      /* Es wurde nicht vergroebert ==> Abbruch */
                      for (k=0;k<4;k++)  /* Schleife ueber alle Kinder */
                          if (r[k]==-1)
                              ElSetElementRefineState(kinder[k],0);
                      return 0;
                  }
              }
          }
      }
      for (m=0;m<4;m++) {  /* Schleife ueber die 1D-Nachbarn */
          if (flaechenkind->nachbarn_1D[m]>=0l) {  /* wenn 1D-Nachbar existiert */
              if (ElGetElementChilds(flaechenkind->nachbarn_1D[m])!=NULL) {
                  if (!RRM3D_a_1D(flaechenkind->nachbarn_1D[m],num_ref_quantity,zwischenspeicher)) {
                      /* Es wurde nicht vergroebert ==> Abbruch */
                      for (k=0;k<4;k++)  /* Schleife ueber alle Kinder */
                          if (r[k]==-1)
                              ElSetElementRefineState(kinder[k],0);
                      return 0;
                  }
              }
          }
      }
  }
  /*
     An dieser Stelle komme ich nur an, wenn ich mich vergroebern darf!
     Keines meiner Nachbarelemente ist jetzt noch feiner als ich!
  */
  for (i=0;i<4;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
  rr_ret = 1;

  }


  /* Massenerhaltung */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);


    /* Bei der Massenerhaltung muss beruecksichtigt werden, dass schon ein
       Korrekturwert in conc[1] stehen kann */

  return 1;  /* vergroebern hat geklappt */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM3D_a_3D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 3D-Elemente ueber das Verfahren
   aus NetRef3D_a. Es werden nur die Massenerhaltung durchgefuehrt und
   die zu vergroebernden Elemente markiert (Status 4) (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   1: habe meine Kinder vergroebert; 0: habe sie nicht vergroebert
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RRM3D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  STATIC int k;  /* Lauf- und Hilfsvariablen */
  int i, j;  /* Lauf- und Hilfsvariablen */
  int r[8];  /* Refine-States */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  long *flaechen = NULL;  /* Feld mit Elementflaechen */
  long *elem_2d, *elem_1d, *elem_3d;
  int anzahl_2d, anzahl_1d, anzahl_3d;

  if (num_ref_quantity == 0) {
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */


  for (i=0;i<8;i++)  /* Schleife ueber alle Kinder */
      r[i] = ElGetElementRefineState(kinder[i]);  /* verfeinerungsstati */
  /* Wenn alle Kinder auf status==4, sind sie bereits vergroebert und von
     einer Rekursion erfasst worden --> return 1 (wie vergroebert) */
  if ( (r[0]==4) && (r[1]==4) && (r[2]==4) && (r[3]==4) &&
       (r[4]==4) && (r[5]==4) && (r[6]==4) && (r[7]==4) ) {
      rr_ret = 1;
      return 1;
  }
  /* Ueberpruefen, ob alle Kinder vergroebert werden sollen */
  if ( !( (r[0]==-1) && (r[1]==-1) && (r[2]==-1) && (r[3]==-1) &&
          (r[4]==-1) && (r[5]==-1) && (r[6]==-1) && (r[7]==-1) ) ) {
      /* es sollen nicht alle Kinder vergroebert werden */
      /* ==> Vergroebern-Schalter loeschen und Abbruch */
      for (i=0;i<8;i++)  /* Schleife ueber alle Kinder */
          if (r[i]==-1)
              ElSetElementRefineState(kinder[i],0);
      return 0;
  }

  for (i=0;i<8;i++) { /* Schleife ueber alle Knoten */
    elem_1d = GetNode1DElems(ElGetElementNodes(number)[i],&anzahl_1d);
    elem_2d = GetNode2DElems(ElGetElementNodes(number)[i],&anzahl_2d);
    elem_3d = GetNode3DElems(ElGetElementNodes(number)[i],&anzahl_3d);
    for (j=0;j<anzahl_3d;j++) {
      if (ElGetElementLevel(elem_3d[j]) == (ElGetElementLevel(number)+2)) {
        for (k=0;k<8;k++)  /* Schleife ueber alle Kinder */
          if (r[k]==-1)
            ElSetElementRefineState(kinder[k],0);
        return 0;
      }
    }
    for (j=0;j<anzahl_2d;j++) {
      if (ElGetElementLevel(elem_2d[j]) == (ElGetElementLevel(number)+2)) {
        for (k=0;k<8;k++)  /* Schleife ueber alle Kinder */
          if (r[k]==-1)
            ElSetElementRefineState(kinder[k],0);
        return 0;
      }
    }
    for (j=0;j<anzahl_1d;j++) {
      if (ElGetElementLevel(elem_1d[j]) == (ElGetElementLevel(number)+2)) {
        for (k=0;k<8;k++)  /* Schleife ueber alle Kinder */
          if (r[k]==-1)
            ElSetElementRefineState(kinder[k],0);
        return 0;
      }
    }
  }

  /*
     An dieser Stelle komme ich nur an, wenn ich mich vergroebern darf!
     Keines meiner Nachbarelemente ist jetzt noch feiner als ich!
  */

  for (i=0;i<8;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
  rr_ret = 1;


  }


  /* Massenerhaltung */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);


    /* Bei der Massenerhaltung muss beruecksichtigt werden, dass schon ein
       Korrekturwert in conc[1] stehen kann */

  return 1;  /* vergroebern hat geklappt */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RR3D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef3D_a. Das Element kann direkt vergroebert werden, es sind
   keine weiteren Abhaengigkeiten zu beruecksichtigen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void RR3D_a_1D ( long number )
{
  static int i, ii;  /* Lauf- und Hilfsvariablen */
  static long *kinder;  /* Feld mit meinen Kindern */
  static long *knoten;  /* Feld mit Elementknoten */
  static long randknoten;
  static int level;

  kinder = ElGetElementChilds(number);  /* Kinder holen */
  /* Anzahl der vergroeberten Elemente austragen */

  level = ElGetElementLevel(number);
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)-2l));
  if (AdaptGetNumEleRefLevelX(level) == 0l)
    AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);

  /*if ((ref_level_anz[ElGetElementLevel(number)]-=2l)==0l)
      ref_level--; */ /* evtl. ref_level veraendern */

  /* Vergroebern */
  for (i=0;i<2;i++) {
      /* Elementknoten holen */
      knoten = ElGetElementNodes(kinder[i]);
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign1DElementNodes(kinder[i],knoten,2);
      /* Kinder aus Flaechenverzeichnis austragen */
      ReAssign1DElementPlains(kinder[i],ElGetElementPlains(kinder[i]),ElGetElementPlainsNumber(kinder[i]));
  }
  /* Mittelknoten ermitteln */
  randknoten = knoten[1];
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign1DElementNodes(number,ElGetElementNodes(number),2);
  /* Kantenknoten-Existenz ueberpruefen */
  if ( (GetNode1DElems(randknoten,&ii)==NULL) &&
       (GetNode2DElems(randknoten,&ii)==NULL) &&
       (GetNode3DElems(randknoten,&ii)==NULL) )
      DeleteNode(randknoten);
  else {
      /* Knoten bleibt erhalten ==> Status ueberpruefen */
      /* Status >=0 oder -3: Knoten wird irregulaer (-2);
                         -1: Knoten wird irregulaer (-2)
                         -2: Kann nicht vorkommen
                         -4: Keine Veraenderung */
      SetNodeState(randknoten,-2);

  }
  /* alle Kinder entfernen */
  for (i=0;i<2;i++)
      ElDeleteElement(kinder[i]);
  /* Vorgaenger aktivieren und Kinder loeschen */
  ElSetElementActiveState(number,1);
  ElDeleteElementChilds(number);
  /* Aktive Elemente anpassen (-2 +1) */
  anz_active_elements -= 1l;
  anz_active_1D -= 1l;
  anz_1D -= 2l;
  msh_no_line-=2l;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RR3D_a_2D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef3D_a. Das Element kann direkt vergroebert werden, es sind
   keine weiteren Abhaengigkeiten zu beruecksichtigen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void RR3D_a_2D ( long number )
{
  static int i, ii;  /* Lauf- und Hilfsvariablen */
  static long *kinder;  /* Feld mit meinen Kindern */
  static long *knoten;  /* Feld mit Elementknoten */
  static long randknoten[5];
  static int level;
    /* Feld mit Knotennummern der Randknoten */
    /* Inhalt : 4 Kantenknoten
                Mittelknoten
    */
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  /* Anzahl der vergroeberten Elemente austragen */


  level = ElGetElementLevel(number);
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)-4l));
  if (AdaptGetNumEleRefLevelX(level) == 0l)
    AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);

  /* if ((ref_level_anz[ElGetElementLevel(number)]-=4l)==0l)
      ref_level--; */ /* evtl. ref_level veraendern */



  /* Vergroebern */
  for (i=0;i<4;i++) {
      /* Elementknoten holen */
      knoten = ElGetElementNodes(kinder[i]);
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign2DElementNodes(kinder[i],knoten,4);
      /* obere und untere Kantenknoten ermitteln */
      randknoten[i] = knoten[1];
      /* Kinder aus Flaechenverzeichnis austragen */
      ReAssign2DElementPlains(kinder[i],ElGetElementPlains(kinder[i]),1);
  }
  /* Mittelknoten ermitteln */
  randknoten[4] = knoten[2];
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign2DElementNodes(number,ElGetElementNodes(number),4);
  /* Mittelknoten-Existenz ueberpruefen */
  if ( (GetNode2DElems(randknoten[4],&ii)==NULL) &&
       (GetNode3DElems(randknoten[4],&ii)==NULL) )
          /* 1D-Elemente koennen nicht vorkommen */
          DeleteNode(randknoten[4]);
  else {
      /* Knoten bleibt erhalten ==> Status ueberpruefen */
      /* Status >=0 oder -3: Knoten wird irregulaer (-4)
                         -1: Knoten wird irregulaer (-4)
                         -2: bleibt erhalten
                         -4: Kann nicht vorkommen */
      SetNodeState(randknoten[4],-4);

  }
  /* Kantenknoten-Existenz ueberpruefen */
  for (i=0;i<4;i++) {
      if ( (GetNode1DElems(randknoten[i],&ii)==NULL) &&
           (GetNode2DElems(randknoten[i],&ii)==NULL) &&
           (GetNode3DElems(randknoten[i],&ii)==NULL) )
          DeleteNode(randknoten[i]);
      else {
          /* Knoten bleibt erhalten ==> Status ueberpruefen */
          /* Status >=0 oder -3: Knoten wird irregulaer (-2);
                             -1: Knoten wird irregulaer (-2)
                             -2: Kann nicht vorkommen
                             -4: Keine Veraenderung */
          SetNodeState(randknoten[i],-2);

      }
  }
  /* alle Kinder entfernen */
  for (i=0;i<4;i++)
      ElDeleteElement(kinder[i]);
  /* Vorgaenger aktivieren und Kinder loeschen */
  ElSetElementActiveState(number,1);
  ElDeleteElementChilds(number);
  /* Aktive Elemente anpassen (-4 +1) */
  anz_active_elements -= 3l;
  anz_active_2D -= 3l;
  anz_2D -= 4l;
  msh_no_quad-=4; //SB
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RR3D_a_3D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 3D-Elemente ueber das Verfahren
   aus NetRef3D_a. Das Element kann direkt vergroebert werden, es sind
   keine weiteren Abhaengigkeiten zu beruecksichtigen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void RR3D_a_3D ( long number )
{
  static int i, ii;  /* Lauf- und Hilfsvariablen */
  static long *kinder;  /* Feld mit meinen Kindern */
  static long *knoten;  /* Feld mit Elementknoten */
  static long randknoten[19];
  static int level;
    /* Feld mit Knotennummern der Randknoten */
    /* Inhalt : 6 Flaechenmittelknoten
                4 obere Kantenknoten
                4 untere Kantenknoten
                4 vertikale Kantenknoten
                Mittelknoten
    */
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  /* Anzahl der vergroeberten Elemente austragen */


  level = ElGetElementLevel(number);
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)-8l));
  if (AdaptGetNumEleRefLevelX(level) == 0l)
    AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);

  /* if ((ref_level_anz[ElGetElementLevel(number)]-=8l)==0l)
      ref_level--; */ /* evtl. ref_level veraendern */


  /* Vergroebern */
  for (i=0;i<8;i++) {
      /* Elementknoten holen */
      knoten = ElGetElementNodes(kinder[i]);
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign3DElementNodes(kinder[i],knoten,8);
      /* obere und untere Kantenknoten ermitteln */
      randknoten[6+i] = knoten[1];
      /* vertikale Kantenknoten ermitteln; werden zweimal eingetragen */
      randknoten[14+(i%4)] = knoten[4];
      /* Kinder aus Kantenverzeichnis austragen
         (Mittelflaechen werden automatisch entfernt) */
      ReAssign3DElementPlains(kinder[i],ElGetElementPlains(kinder[i]),6);
  }
  /* Mittelknoten ermitteln */
  randknoten[18] = knoten[6];
  /* Flaechenmittelknoten ermitteln */
  randknoten[0] = knoten[2];
  randknoten[1] = knoten[5];
  randknoten[2] = knoten[7];
  knoten = ElGetElementNodes(kinder[1]);
  randknoten[3] = knoten[2];
  randknoten[4] = knoten[5];
  randknoten[5] = knoten[7];
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign3DElementNodes(number,ElGetElementNodes(number),8);
  /* Mittelknoten entfernen */
  DeleteNode(randknoten[18]);
  /* Flaechenmittelknoten-Existenz ueberpruefen */
  for (i=0;i<6;i++) {
      if ( (GetNode2DElems(randknoten[i],&ii)==NULL) &&
           (GetNode3DElems(randknoten[i],&ii)==NULL) )
          /* 1D-Elemente koennen nicht vorkommen */
          DeleteNode(randknoten[i]);
      else {
          /* Knoten bleibt erhalten ==> Status ueberpruefen */
          /* Status >=0 oder -3: Knoten wird irregulaer (-4)
                             -1: Knoten wird irregulaer (-4)
                             -2: bleibt erhalten
                             -4: Kann nicht vorkommen */
          SetNodeState(randknoten[i],-4);

      }
  }
  /* Kantenknoten-Existenz ueberpruefen */
  for (i=6;i<18;i++) {
      if ( (GetNode1DElems(randknoten[i],&ii)==NULL) &&
           (GetNode2DElems(randknoten[i],&ii)==NULL) &&
           (GetNode3DElems(randknoten[i],&ii)==NULL) )
      DeleteNode(randknoten[i]);
      else {
          /* Knoten bleibt erhalten ==> Status ueberpruefen */
          /* Status >=0 oder -3: Knoten wird irregulaer (-2)
                    -1: Knoten wird irregulaer (-2)
                    -2: Kann nicht vorkommen
                    -4: Keine Veraenderung */
          SetNodeState(randknoten[i],-2);

      }
  }
  /* alle Kinder entfernen */
  for (i=0;i<8;i++)
      ElDeleteElement(kinder[i]);
  /* Vorgaenger aktivieren und Kinder loeschen */
  ElSetElementActiveState(number,1);
  ElDeleteElementChilds(number);
  /* Aktive Elemente anpassen (-8 +1) */
  anz_active_elements -= 7l;
  anz_active_3D -= 7l;
  anz_3D -= 8l;
  msh_no_hexs-=8l;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RN3D_a_1D
                                                                          */
/* Aufgabe:
   Elementnachlauffunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu ueberpruefenden Elements
                                                                          */
/* Ergebnis:
   1: es wurde verfeinert; 0: es wurde nicht verfeinert
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RN3D_a_1D ( long number )
{
  int anz;  /* Anzahl der angrenzenden Flaechen */
  int i, j, k;  /* Laufvariablen */
  int verf;  /* Return-Wert */
  long vorg;  /* Elementvorgaenger */
  long *flaechen = NULL;  /* Feld mit Elementflaechen */
  Flaeche *flaeche = NULL;  /* Flaechenzeiger */
  anz = ElGetElementPlainsNumber(number);  /* Anzahl der Flaechen */
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  /* Es kann nicht gleichzeitig feinere und groebere Nachbarn geben !!! */
  k = 0;
  while ((k<anz) && (flaechen[k]>=0l))
      k++;
  if (k<anz) {  /* Fall 1: Nicht alle Flaechen sind eingetragen, d.h. es gibt groebere Nachbarn */
      verf = 0;
      vorg = ElGetElementPred(number);  /* Vorgaenger holen */
      flaechen = ElGetElementPlains(vorg);  /* Flaechen von vorg holen */
      for (i=0;i<anz;i++) {  /* Schleife ueber alle Flaechen */
          flaeche = GetPlain(flaechen[i]);  /* Flaeche holen */
          if (flaeche->nachbarn_3D[1]>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(flaeche->nachbarn_3D[1])==NULL) {
                  /* und ist groeber als ich */
                  R3D_a_3D(flaeche->nachbarn_3D[1]);
                  verf = 1;
              }
          }
          if (flaeche->nachbarn_3D[0]>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(flaeche->nachbarn_3D[0])==NULL) {
                  /* und ist groeber als ich */
                  R3D_a_3D(flaeche->nachbarn_3D[0]);
                  verf = 1;
              }
          }
          if (flaeche->nachbar_2D>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(flaeche->nachbar_2D)==NULL) {
                  /* und ist groeber als ich */
                  R3D_a_2D(flaeche->nachbar_2D);
                  verf = 1;
              }
          }
          for (j=0;j<4;j++) {
              if (flaeche->nachbarn_1D[j]>=0l) {  /* Nachbar existiert */
                  if (ElGetElementChilds(flaeche->nachbarn_1D[j])==NULL) {
                      /* und ist groeber als ich */
                      R3D_a_1D(flaeche->nachbarn_1D[j]);
                      verf = 1;
                  }
              }
          }
      }
      return verf;
  }
  else {
      /* Fall 2: Es existieren feinere Nachbarn, d.h. alle Flaechen sind eingetragen
                 und evtl. existiert mindestens eine angreifende verfeinerte Flaeche */
      i = 0;
      while ((i<anz) && ((GetPlain(flaechen[i]))->kinder[0]<0l))
          i++;
      if (i<anz) {  /* es existiert mindestens eine verfeinerte Flaeche */
          R3D_a_1D(number);  /* number wird verfeinert */
          /* Der Prueflauf ueber die Nachbarn startet von der
             Verfeinerungsfunktion aus !!! */
          return 1;  /* Es wurde verfeinert */
      }
      return 0;  /* Es wurde nicht verfeinert */
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RN3D_a_2D
                                                                          */
/* Aufgabe:
   Elementnachlauffunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu ueberpruefenden Elements
                                                                          */
/* Ergebnis:
   1: es wurde verfeinert; 0: es wurde nicht verfeinert
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RN3D_a_2D ( long number )
{
  STATIC long *flaechen = NULL;  /* Feld mit Elementflaechen */
  STATIC Flaeche *flaeche = NULL;  /* Eine Nachbarflaeche */
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  flaeche = GetPlain(flaechen[0]);  /* Flaeche holen */
  if (flaeche->kinder[0]>=0l) {  /* Diese Flaeche wurde bereits verfeinert! */
      /* ==> 1. ich muss auch verfeinert werden! */
      R3D_a_2D(number);  /* number wird verfeinert */
      /* Der Prueflauf ueber die Nachbarn startet von der
         Verfeinerungsfunktion aus !!! */
      return 1;  /* Es wurde verfeinert */
  }
  return 0;  /* Es wurde nicht verfeinert */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RN3D_a_3D
                                                                          */
/* Aufgabe:
   Elementnachlauffunktion fuer 3D-Elemente ueber das Verfahren
   aus NetRef3D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu ueberpruefenden Elements
                                                                          */
/* Ergebnis:
   1: es wurde verfeinert; 0: es wurde nicht verfeinert
                                                                          */
/* Programmaenderungen:
   04/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RN3D_a_3D ( long number )
{
  STATIC int anz;  /* 1. Kriterium: Anzahl der Flaechen mit Kindern */
  STATIC int verf;  /* 2. Kriterium: feines 1D- oder 2D-Element benachbart */
  STATIC int i;  /* Laufvariable */
  STATIC long *flaechen = NULL;  /* Feld mit Elementflaechen */
  STATIC Flaeche *flaeche = NULL;  /* Eine Nachbarflaeche */
  anz = verf = 0;
  flaechen = ElGetElementPlains(number);  /* Flaechen von number holen */
  for (i=0;i<6;i++) {  /* Schleife ueber alle Flaechen */
      flaeche = GetPlain(flaechen[i]);  /* Flaeche holen */
      if (flaeche->kinder[0]>=0l) {  /* Diese Flaeche wurde bereits verfeinert! */
          anz++;  /* Anzahl der feineren Flaechen erhoehen */
          /* wenn die Flaeche verfeinert ist, wird / wurde das 1D/2D-Element
            irgendwann zwangslaeufig verfeinert
            ==> folgende Abfrage ist o.k. */
          if ( (flaeche->nachbar_2D>=0l) ||
               (flaeche->nachbarn_1D[0]>=0l) ||
               (flaeche->nachbarn_1D[1]>=0l) ||
               (flaeche->nachbarn_1D[2]>=0l) ||
               (flaeche->nachbarn_1D[3]>=0l)
             ) {
              /* 2. Kriterium: number ist Nachbar von feinerem 1D/2D-Element */
              verf = 1;
          }
      }
  }
  if ( (anz>=AdaptGetRefNeighbours3d()) || (verf) ) {
      /* Wenn mehr feinere Nachbarn als zugelassen existieren oder
         mindestens ein feineres 1D/2D-Element benachbart ist, wird
         number verfeinert. */
      R3D_a_3D(number);  /* number wird verfeinert */
      /* Der Prueflauf ueber die Nachbarn startet von der
         Verfeinerungsfunktion aus !!! */
      return 1;  /* Es wurde verfeinert */
  }
  return 0;  /* Es wurde nicht verfeinert */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InterpolIrregNodeVals3D
                                                                          */
/* Aufgabe:
   Interpoliert Knotenwerte der irregulaeren Knoten aus den
   Nachbarknoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int nval: Index der zu Interpolierenden Knotenwerte

                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void InterpolIrregNodeVals3D ( int nval0, int nval1 )
{
  static long i, j;  /* Laufvariablen */
  static Knoten *k;  /* Knotenzeiger zum Interpolieren der irreg. Knoten */
  j = NodeListSize();
  for (i=0l;i<j;i++) {
      if ((k = GetNode(i))!=NULL) {
          if (k->status==-2) {  /* irr. Kantenmittelknoten */
              if (k->anz1d>0) {
                  /* 2 1D-Elemente eingetragen (und evtl. weitere Elemente) */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems1d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems1d[1]))[0],nval0) ) * 0.5 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems1d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems1d[1]))[0],nval1) ) * 0.5 );
              }
              else if (k->anz2d>0) {
                  /* 2 oder mehr (paarweise eingetragene !!!) 2D-Elemente eingetragen (u.a.) */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval0) ) * 0.5 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval1) ) * 0.5 );
              }
              else {
                  /* 2 oder mehr (paarweise eingetragene !!!) 3D-Elemente eingetragen (u.a.) */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems3d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[1]))[0],nval0) ) * 0.5 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems3d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[1]))[0],nval1) ) * 0.5 );
              }
          }
          else if (k->status==-4) {  /* irr. Flaechenmittelknoten */
              if (k->anz2d>0) {
                  /* 4 2D-Elemente eingetragen (u.a.) */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[2]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[3]))[0],nval0) ) * 0.25 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[2]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[3]))[0],nval1) ) * 0.25 );
              }
              else {
                  /* 4 oder 8 (gruppenweise eingetragene !!!) 3D-Elemente eingetragen */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems3d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[1]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[2]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[3]))[0],nval0) ) * 0.25 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems3d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[1]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[2]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems3d[3]))[0],nval1) ) * 0.25 );
              }
          }
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckNodeState3D
                                                                          */
/* Aufgabe:
   Ueberprueft und setzt Status eines potentiellen irregulaeren
   Kantenknotens, der auch Randknoten sein kann.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long irrnode: potentieller irr. Knoten
   E long k1, k2: benachbarte Kantenknoten
                                                                          */
/* Ergebnis:
   Knotenstatus
                                                                          */
/* Programmaenderungen:
   01/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int CheckNodeState3D ( long irrnode, long k1, long k2 )
{
  static int i, j;
  static Knoten *kk1;
  static Knoten *kk2;
  kk1 = GetNode(k1);
  kk2 = GetNode(k2);
  for (i=0;i<kk1->anz3d;i++) {
      for (j=0;j<kk2->anz3d;j++) {
          if (kk1->elems3d[i]==kk2->elems3d[j]) {
              /* an den beiden Kantenknoten greift das gleiche aktive 3D-Element
                 an ==> irrnode ist irregulaer, da ein groesseres Element an die
                        Kante grenzt */
              SetNodeState(irrnode,-2);
              return -2;
          }
      }
  }
  for (i=0;i<kk1->anz2d;i++) {
      for (j=0;j<kk2->anz2d;j++) {
          if (kk1->elems2d[i]==kk2->elems2d[j]) {
              /* an den beiden Kantenknoten greift das gleiche aktive 2D-Element
                 an ==> irrnode ist irregulaer, da ein groesseres Element an die
                        Kante grenzt */
              SetNodeState(irrnode,-2);
              return -2;
          }
      }
  }
  for (i=0;i<kk1->anz1d;i++) {
      for (j=0;j<kk2->anz1d;j++) {
          if (kk1->elems1d[i]==kk2->elems1d[j]) {
              /* an den beiden Kantenknoten greift das gleiche aktive 1D-Element
                 an ==> irrnode ist irregulaer, da ein groesseres Element an die
                        Kante grenzt */
              SetNodeState(irrnode,-2);
              return -2;
          }
      }
  }

  if ((kk1->status==-3) && (kk2->status==-3)) {
      /* Kante ist Randkante */
      SetNodeState(irrnode,-3);
      return -3;
  }
  /* Knoten ist regulaerer Innenknoten */
  SetNodeState(irrnode,-1);
  return -1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckNodeStateA3D
                                                                          */
/* Aufgabe:
   Ueberprueft und setzt Status eines potentiellen irregulaeren
   Flaechenmittelknotens, der auch Randknoten sein kann.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long irrnode: potentieller irr. Knoten
   E Flaeche *flaeche: Flaeche, in dessen Mitte irrnode liegt
                                                                          */
/* Ergebnis:
   Knotenstatus
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
int CheckNodeStateA3D ( long irrnode, Flaeche *flaeche )
{
  static int s[4];  /* Knotenstati */
  static int j;  /* Laufvariable */
  if ( ( (flaeche->nachbarn_3D[0]>=0l) && (ElGetElementChilds(flaeche->nachbarn_3D[0])==NULL) ) ||
       ( (flaeche->nachbarn_3D[1]>=0l) && (ElGetElementChilds(flaeche->nachbarn_3D[1])==NULL) ) ||
       ( (flaeche->nachbar_2D>=0l) && (ElGetElementChilds(flaeche->nachbar_2D)==NULL) )
     ) {
      /* Knoten ist irregulaer, wenn mindestens ein kinderloser Nachbar
         existiert */
      SetNodeState(irrnode,-4);
      return -4;
  }
  /* Knoten ist entweder regulaerer Rand- oder Innenknoten */
  for (j=0;j<4;j++) {
     /* Knotenstati holen */
     s[j] = GetNodeState(flaeche->knoten[j]);
  }

  if ((s[0]==-3) && (s[1]==-3) && (s[2]==-3) && (s[3]==-3)) {
      /* Flaeche ist Randflaeche */
      SetNodeState(irrnode,-3);
      return -3;
  }
  /* Knoten ist regulaer */
  SetNodeState(irrnode,-1);
  return -1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckNodeExist3D
                                                                          */
/* Aufgabe:
   Ueberprueft die Existenz eines Kantenmittelknotens zwischen zwei
   Kantenknoten und erzeugt ggf. einen neuen Kantenmittelknoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long k1, k2: benachbarte Kantenknoten
                                                                          */
/* Ergebnis:
   Nummer des Kantenmittelknotens
                                                                          */
/* Programmaenderungen:
   02/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
long CheckNodeExist3D ( long k1, long k2 )
{
  static int i, j;
  static Knoten *kk1;
  static Knoten *kk2;
  static long *kkk1;
  static long *kkk2;
  static Knoten *kneu;
  kk1 = GetNode(k1);
  kk2 = GetNode(k2);
  /* 3D */
  for (i=0;i<kk1->anz3d;i++) {
      kkk1 = ElGetElementNodes(kk1->elems3d[i]);
      if (kkk1[0]==k1) {
          for (j=0;j<kk2->anz3d;j++) {
              kkk2 = ElGetElementNodes(kk2->elems3d[j]);
              if (kkk2[0]==k2) {
                  if (kkk1[1]==kkk2[3]) {  /* horizontale Kante */
                      /* Zwischenknoten existiert */
                      return kkk1[1];
                  }
                  else if (kkk1[3]==kkk2[1]) {
                      /* Zwischenknoten existiert */
                      return kkk1[3];
                  }
                  else if (kkk1[4]==kkk2[4]) {  /* vertikale Kante */
                      /* Zwischenknoten existiert */
                      return kkk1[4];
                  }
              }
          }
      }
  }
  /* 2D */
  for (i=0;i<kk1->anz2d;i++) {
      kkk1 = ElGetElementNodes(kk1->elems2d[i]);
      if (kkk1[0]==k1) {
          for (j=0;j<kk2->anz2d;j++) {
              kkk2 = ElGetElementNodes(kk2->elems2d[j]);
              if (kkk2[0]==k2) {
                  if (kkk1[1]==kkk2[3]) {
                      /* Zwischenknoten existiert */
                      return kkk1[1];
                  }
                  else if (kkk1[3]==kkk2[1]) {
                      /* Zwischenknoten existiert */
                      return kkk1[3];
                  }
              }
          }
      }
  }
  /* 1D - 1D-Elemente koennen nur mit Knoten[0] an Ecke angreifen (anders als bei maxdim=1 (2D)) */
  for (i=0;i<kk1->anz1d;i++) {
      kkk1 = ElGetElementNodes(kk1->elems1d[i]);
      if (kkk1[0]==k1) {
          for (j=0;j<kk2->anz1d;j++) {
              kkk2 = ElGetElementNodes(kk2->elems1d[j]);
              if (kkk2[0]==k2) {
                  if (kkk1[1]==kkk2[1]) {
                      /* Zwischenknoten existiert */
                      return kkk1[1];
                  }
              }
          }
      }
  }
  /* Knoten existiert nicht ==> erzeugen */
  kneu = NewNode();
  kneu->x = ( kk1->x + kk2->x ) / 2.0;
  kneu->y = ( kk1->y + kk2->y ) / 2.0;
  kneu->z = ( kk1->z + kk2->z ) / 2.0;

  /* modellspezifische Knotendaten verfeinern */
  RefineEdgeValues(kneu,kk1,kk2);

  return AddNode(kneu);
}

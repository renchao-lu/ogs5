/**************************************************************************/
/* ROCKFLOW - Modul: refine1.c
                                                                          */
/* Aufgabe:
   Verfeinerungs- / Vergroeberungsfunktionen
   Massgebend fuer die Verfeinerung ist der Zustand des
   "verfeinern" - Schalters in den Elementdaten.
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   04.08.1998  R.Kaiser   Adaption fuer mehrere Loesungsgroessen
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   11/1999     AH         Bugfix
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
#include "edges.h"
#include "mathlib.h"
#include "masscont.h"
#include "msh_elements_rfi.h"


/* Interne (statische) Deklarationen */

void R1D_a_1D ( long number );
void R1D_a_2D ( long number );
void R1D_a_3D ( long number );
void RR1D_a_1D ( long number );
void RR1D_a_2D ( long number );
void RR1D_a_3D ( long number );
int RRM1D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM1D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM1D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher );


/* Definitionen */

static VoidFuncLong R1D_a[3] = { R1D_a_1D, R1D_a_2D, R1D_a_3D };
  /* Feld mit den Elementverfeinerungsfunktionen fuer NetRef1D_a */
static VoidFuncLong RR1D_a[3] = { RR1D_a_1D, RR1D_a_2D, RR1D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef1D_a */
static IntFuncLIDX RRM1D_a[3] = { RRM1D_a_1D, RRM1D_a_2D, RRM1D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef2D_a */

static int rr_ret; /* Return-Wert der rekursiven Vergroeberungsfunktion
                     (nur fuer die Ueberpruefung der Nachbarn) */


/**************************************************************************/
/* ROCKFLOW - Funktion: NetRef1D_a
                                                                          */
/* Aufgabe:
   Verfeinert bzw. Vergroebert nur 1D-Elemente.
   Eine zweite Vergroeberungsschleife ist hier wegen des gewaehlten
   Massenerhaltungs-Verfahrens nicht noetig.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   1: Das Netz wurde veraendert; 0: keine Veraenderung
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   06/1995     MSR        an Prinzip der 2D-Verfeinerung angepasst und
                          zusaetzliches Sekundaerkriterium eingebaut
                                                                          */
/**************************************************************************/
int NetRef1D_a ( void )
{
  /* Variablen */
  static int ret;  /* return-Wert */
  static long i, j;  /* Laufvariable */
  static long vorgaenger;  /* Vorgaenger eines evtl. zu vergroebernden Elements */
  static long max_index;  /* hoechster Elementindex */
  static int level;  /* Verfeinerungslevel */
  static int num_ref_quantity; /* Laufvariablen */
  double *zwischenspeicher;

  ret = 0;
  /* Schleife ueber alle zu verfeinernden Elemente */
  max_index = ElListSize();  /* Hoechsten Index holen */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
      if (ElGetElement(i)!=NULL)  /* wenn Element existiert */
          if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
              if (ElGetElementRefineState(i)==1) {
                  if ((level=ElGetElementLevel(i))<AdaptGetMaxRefLevel()) {
                      /* wenn nicht schon max. Verfeinerungstiefe erreicht ist */
                      /* R1D_a[ElGetElementType(i)-1](i); */
                      R1D_a[0](i);
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
  /*
     Jetzt sind alle Elemente, die verfeinert werden sollen, verfeinert.
     Ausserdem ist die Konsistenz des Netzes sichergestellt, d.h. die max.
     Differenz des Verfeinerungsgrades zweier benachbarter Elemente betraegt 1.
  */


  /* Sicherstellen, dass NodeNumber, NodeListSize und NodeIndex stimmig sind */
  CompressNodes(); /* CT3405 */
  /* Randbedingungen ausfuehren */
  //OK_BC ExecuteAllBoundaryConditions();
  /* Quelle/Senken ausfuehren */
  //OK_ST ExecuteAllSourceSink();


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
               RRM1D_a[0](vorgaenger,num_ref_quantity,zwischenspeicher);
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


  /* Schleife ueber alle zu vergroebernden Elemente, direkt vergroebern */
  /* Der Nachlauf wird wg. sauberer Massenerhaltung benoetigt */
  for (i=0;i<ElListSize();i++) {  /* Schleife ueber alle Elemente */
      if (ElGetElement(i)!=NULL) {  /* wenn Element existiert */
          if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
              if (ElGetElementRefineState(i)==4) {
                  /* Vorgaenger holen (muss existieren, da Nachlauf) */
                  vorgaenger = ElGetElementPred(i);
                  RR1D_a[0](vorgaenger);
                    /* Die nicht rekursive Vergroeberungsfunktion wird mit dem
                       Vorgaengerelement aufgerufen */
              }
          }
      }
  }



  /*
     Alle Elemente, die vergroebert werden sollen, sind, soweit es die
     Konsistenz des Netzes nicht beeintraechtigt, vergroebert.
  */
  ret = rr_ret || ret;
  return ret;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R1D_a_1D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer die 1D-Elemente ueber das Verfahren
   aus NetRef1D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version (RSimple1D)
   06/1995     MSR        Komplett neu nach Schema NetRef2D_a
                                                                          */
/**************************************************************************/
void R1D_a_1D ( long number )
{
  STATIC int anz;  /* Lauf- und Hilfsvariablen */
  STATIC long *kanten;  /* Feld mit Elementkante */
  STATIC Kante *kante;  /* Zeiger auf Elementkante */
  STATIC Kante *kinder[2];  /* Kinder der Kante */
  STATIC long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */
  STATIC long *kantenliste;  /* Feld mit Kante der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  STATIC long *elems;  /* Zeiger auf die Knotenelemente eines Eckknotens */
  int i;  /* Lauf- und Hilfsvariablen */
  int level;  /* Verfeinerungslevel der neuen Elemente-1 == level von number */
  neue_elemente = (long *) Malloc(2*sizeof(long));
    /* sind spaeter die Kinder von number */
  for (i=0;i<2;i++)  /* neue Elemente erzeugen */
      neue_elemente[i] = ElCreateElement(1,number,-1);


  level=ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level) + 2l));

  /* ref_level_anz[(level=(ElGetElementLevel(neue_elemente[0])-1))] += 2l; */


    /* Anzahl der verfeinerten Elemente eintragen */
  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  kante = GetEdge(kanten[0]);
  /* Kante verfeinern und Mittelknoten erzeugen */
  CreateTwoEdges(kanten[0]);
  kinder[0] = GetEdge(kante->kinder[0]);  /* Kantenkind 1 */
  kinder[1] = GetEdge(kante->kinder[1]);  /* Kantenkind 2 */
  /* Kantenverzeichnis updaten */
  kinder[0]->nachbar_1D = neue_elemente[0];
  kinder[1]->nachbar_1D = neue_elemente[1];
  /* Element-zu-Knoten-Liste fuer number anpassen */
  ReAssign1DElementNodes(number,&(kante->knoten[0]),2);
  /* Kanten, Knoten und Matrizen bei den neuen Elementen eintragen */
  for (i=0;i<2;i++) {
      /* neue Knoteneintraege erzeugen */
      knotenliste = (long *) Malloc(2*sizeof(long));
      knotenliste[0] = kinder[i]->knoten[0];
      knotenliste[1] = kinder[i]->knoten[1];
      ElSetElementNodes(neue_elemente[i],knotenliste);
      /* neue Kanteneintraege erzeugen */
      kantenliste = (long *) Malloc(sizeof(long));
      kantenliste[0] = kante->kinder[i];
      ElSetElementEdges(neue_elemente[i],kantenliste);
      /* Element-zu-Knoten-Liste fuer neue Elemente anpassen */
      Assign1DElementNodes(neue_elemente[i],knotenliste,2);
  }
  /* Vorgaenger */
  ElSetElementActiveState(number,0);                      /* deaktivieren */
  ElSetElementChilds(number,neue_elemente);           /* Kinder eintragen */
  ElSetElementRefineState(number,0);           /* Verfeinern-Schalter aus */
  /* Anzahl Aktive Elemente anpassen (+2 neue -1 altes) */
  anz_active_elements++;
  anz_active_1D++;
  anz_1D += 2l;
  msh_no_line+=2l;
  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);
  /* evtl. rekursiv die Nachbarelemente verfeinern */
  for (i=0;i<2;i++) {
      elems = GetNode1DElems(kante->knoten[i],&anz);
      if (anz==2) {  /* Kein Randknoten; Nachbarelement existiert */
          if (number==elems[0]) {  /* Nachbarelement steht in elems[1] */
              if (ElGetElementLevel(elems[1])<level)  /* wenn zu grob */
                  R1D_a_1D(elems[1]);  /* ==> verfeinern */
          }
          else {  /* Nachbarelement steht in elems[0] */
              if (ElGetElementLevel(elems[0])<level)  /* wenn zu grob */
                  R1D_a_1D(elems[0]);  /* ==> verfeinern */
          }
      }
  }
  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM1D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer die 1D-Elemente ueber das Verfahren
   aus NetRef1D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   1: habe meine Kinder vergroebert; 0: habe sie nicht vergroebert
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version (RRSimple1D);
   06/1995     MSR        Komplett neu nach Schema NetRef2D_a
   06/1996     cb         Return-Wert
   11/1999     AH         Bugfix
                                                                          */
/**************************************************************************/
int RRM1D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  STATIC int j, anz;
  STATIC long *kanten;  /* Feld mit Elementkanten */
  STATIC long *elems;  /* Feld mit Knotenelementen */
  int i;  /* Lauf- und Hilfsvariablen */
  int level;  /* Verfeinerungslevel der Kinder von number */
  int r[2];  /* Refine-States */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  Kante *kante = NULL;  /* Eine Nachbarkante */

  if (num_ref_quantity == 0) {
    level = ElGetElementLevel(number) + 1;
    /* Ueberpruefen, ob alle Kinder vergroebert werden sollen */
    kinder = ElGetElementChilds(number);  /* Kinder holen */
    r[0] = ElGetElementRefineState(kinder[0]);
    r[1] = ElGetElementRefineState(kinder[1]);
    if ( !((r[0] == -1) && (r[1] == -1)) ) {
        /* es sollen nicht alle Kinder vergroebert werden */
        /* ==> Vergroebern-Schalter loeschen und Abbruch */
        for (i=0;i<2;i++) {  /* Schleife ueber alle Kinder */
            if (r[i]==-1) {
                ElSetElementRefineState(kinder[i],0);
            }
        }
        return 0;
    }
    kanten = ElGetElementEdges(number);  /* Kante von number holen */
    kante = GetEdge(kanten[0]);
    /* evtl. rekursiv die Nachbarelemente vergroebern */
    for (i=0;i<2;i++) {
        elems = GetNode1DElems(kante->knoten[i],&anz);
        if (anz==2) {  /* Kein Randknoten; Nachbarelement existiert */
            if (kinder[i]==elems[0]) {  /* Nachbarelement steht in elems[1] */
                if (ElGetElementLevel(elems[1])>level) { /* wenn zu fein */
                    if (!(RRM1D_a_1D(ElGetElementPred(elems[1]),num_ref_quantity,zwischenspeicher))) {
                        /* ==> vergroebern; falls kein Erfolg: Abbruch */
                        for (j=0;j<2;j++)  /* Schleife ueber alle Kinder */
                            if (r[j]==-1)
                                ElSetElementRefineState(kinder[j],0);
                        return 0;
                    }
                }
            }
            else {  /* Nachbarelement steht in elems[0] */
                if (ElGetElementLevel(elems[0])>level) { /* wenn zu fein */
                    if (!(RRM1D_a_1D(ElGetElementPred(elems[0]),num_ref_quantity,zwischenspeicher))) {
                        /* ==> vergroebern; falls kein Erfolg: Abbruch */
                        for (j=0;j<2;j++)  /* Schleife ueber alle Kinder */
                            if (r[j]==-1)
                                ElSetElementRefineState(kinder[j],0);
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

    for (i=0;i<2;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
    rr_ret = 1;
  }


  /* Massenausgleich */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);


  return 1;  /* vergroebern hat geklappt */

}

/**************************************************************************/
/* ROCKFLOW - Funktion: RR1D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef1D_a. Das Element kann direkt vergroebert werden, es sind
   keine weiteren Abhaengigkeiten zu beruecksichtigen.

   Verbessern: mittelknoten einfacher bestimmen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers der zu vergroebernden
                  Elemente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/1999     RK        Erste Version

                                                                          */
/**************************************************************************/
void RR1D_a_1D ( long number )
{
  STATIC Kante *kantenkind;  /* Kind einer Kante */
  STATIC long *kanten;  /* Feld mit Elementkanten */
  STATIC long mittelknoten;  /* Nummer des neuen Mittelknotens */
  int i;  /* Lauf- und Hilfsvariablen */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  Kante *kante = NULL;  /* Eine Nachbarkante */


  /* Anzahl der vergroeberten Elemente austragen */
  AdaptSetNumEleRefLevelX(ElGetElementLevel(number),
                     (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))-2l));

  if (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))==0l)
      AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);

  /* if ((ref_level_anz[ElGetElementLevel(number)]-=2l)==0l)
      ref_level--;*/


  kanten = ElGetElementEdges(number);  /* Kante von number holen */
  kante = GetEdge(kanten[0]);

  /* Vergroebern */
  /* Mittelknoten ermitteln */
  kantenkind = GetEdge(kante->kinder[1]);
  mittelknoten = kantenkind->knoten[0];

  kinder = ElGetElementChilds(number);  /* Kinder holen */

  for (i=0;i<2;i++) {
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign1DElementNodes(kinder[i],ElGetElementNodes(kinder[i]),2);
      /* Kinder aus Kantenverzeichnis austragen */
      ReAssign1DElementEdges(kinder[i],ElGetElementEdges(kinder[i]),1);
  }
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign1DElementNodes(number,ElGetElementNodes(number),2);
  /* Mittelknoten loeschen */
  DeleteNode(mittelknoten);
  /* alle Kinder entfernen */
  for (i=0;i<2;i++)
      ElDeleteElement(kinder[i]);
  /* Vorgaenger aktivieren und Kinder loeschen */
  ElSetElementActiveState(number,1);
  ElDeleteElementChilds(number);
  /* Aktive Elemente anpassen (-2 +1) */
  anz_active_elements--;
  anz_active_1D--;
  anz_1D -= 2l;
  msh_no_line-=2l;
}








void R1D_a_2D ( long number )
{
static long dummy;
dummy = number;
}

void R1D_a_3D ( long number )
{
static long dummy;
dummy = number;
}

/* Die Elementverfeinerungsfunktionen fuer 2D- und 3D-Elemente fuer
   NetRef1D_a werden nicht implementiert */


void RR1D_a_2D ( long number )
{
static long dummy;
dummy = number;
}

void RR1D_a_3D ( long number )
{
static long dummy;
dummy = number;
}



int RRM1D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
static long dummy;
dummy = number;
zwischenspeicher=zwischenspeicher;  /* Wegen Warnung */
num_ref_quantity=num_ref_quantity;
return 0;
}

int RRM1D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
static long dummy;
dummy = number;
zwischenspeicher=zwischenspeicher;  /* Wegen Warnung */
num_ref_quantity=num_ref_quantity;
return 0;
}
/* Die Elementvergroeberungsfunktionen fuer 2D- und 3D-Elemente fuer
   NetRef1D_a werden nicht implementiert */













































































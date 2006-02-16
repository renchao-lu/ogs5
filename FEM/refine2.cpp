/**************************************************************************/
/* ROCKFLOW - Modul: refine2.c
                                                                          */
/* Aufgabe:
   Verfeinerungs- / Vergroeberungsfunktionen
   Massgebend fuer die Verfeinerung ist der Zustand des
   "verfeinern" - Schalters in den Elementdaten.
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
   01/1996     MSR        Nachlauf optimiert
   31.10.1997  R.Kaiser   Je Kante bliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke 2D)
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

#define TESTREF
/* Header / Andere intern benutzte Module */
#include "refine.h"
#include "nodes.h"
#include "elements.h"
#include "edges.h"
#include "mathlib.h"
#include "masscont.h"
#include "msh_elements_rfi.h"


/* Interne (statische) Deklarationen */

void R2D_a_1D ( long number );
void R2D_a_2D ( long number );
void R2D_a_3D ( long number );
void RR2D_a_1D ( long number );
void RR2D_a_2D ( long number );
void RR2D_a_3D ( long number );
int RRM2D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM2D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RRM2D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher );
int RN2D_a_1D ( long number );
int RN2D_a_2D ( long number );
int RN2D_a_3D ( long number );

int CheckNodeState2D ( long irrnode, Kante *kante );


/* Definitionen */

static VoidFuncLong R2D_a[3] = { R2D_a_1D, R2D_a_2D, R2D_a_3D };
  /* Feld mit den Elementverfeinerungsfunktionen fuer NetRef2D_a */
static VoidFuncLong RR2D_a[3] = { RR2D_a_1D, RR2D_a_2D, RR2D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef2D_a */
static IntFuncLIDX RRM2D_a[3] = { RRM2D_a_1D, RRM2D_a_2D, RRM2D_a_3D };
  /* Feld mit den Elementvergroeberungsfunktionen fuer NetRef2D_a */
static IntFuncLong RN2D_a[3] = { RN2D_a_1D, RN2D_a_2D, RN2D_a_3D };
  /* Feld mit den Elementnachlauffunktionen fuer NetRef2D_a */


static int nachlauf;  /* 0: Verfeinerungsschleife; 1: Nachlaufschleife */
static int rr_ret; /* 0: keine Aktion; 1: Aktion (Vergroeberung) */


/**************************************************************************/
/* ROCKFLOW - Funktion: NetRef2D_a
                                                                          */
/* Aufgabe:
   Verfeinert und Vergroebert gekoppelte 1D- und 2D-Element-Netze
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   1: Das Netz wurde veraendert; 0: keine Veraenderung
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
int NetRef2D_a ( void )
{
  /* Variablen */
  static long i, j, jj;  /* Laufvariablen */
  static long vorgaenger;  /* Vorgaenger eines evtl. zu vergroebernden Elements */
  static long max_index;  /* hoechster Elementindex */
  static int level;  /* Verfeinerungslevel */
  static int ret;  /* return-Wert */
  static int num_ref_quantity; /* Laufvariablen */
  double *zwischenspeicher;


  ret = 0;
  /* Schleife ueber alle zu verfeinernden Elemente */
  nachlauf = 0;  /* Keine sekundaeren Kriterien ueberpruefen */
  max_index = ElListSize();  /* Hoechsten Index holen */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
      if (ElGetElement(i)!=NULL)  /* wenn Element existiert */
          if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
              if (ElGetElementRefineState(i)==1) {
                  if ((level=ElGetElementLevel(i))<AdaptGetMaxRefLevel()) {
                      /* wenn nicht schon max. Verfeinerungstiefe erreicht ist */
                      R2D_a[ElGetElementType(i)-1](i);
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
               RRM2D_a[ElGetElementType(vorgaenger)-1](vorgaenger,num_ref_quantity,zwischenspeicher);
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

    IncAllNodeVals(AdaptGetRefNval0(num_ref_quantity),AdaptGetRefNval0(num_ref_quantity),zwischenspeicher);

  }
  zwischenspeicher = (double *)Free(zwischenspeicher);



  ret = rr_ret || ret;
  if (!ret) {
    /* Kein Element veraendert ==> keine Massen veraendert; kein Nachlauf noetig etc. */
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
                  RR2D_a[ElGetElementType(vorgaenger)-1](vorgaenger);
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
  /* irregulaere Knotenwerte interpolieren (wg. Massenerhaltung) */


  for (jj=0;jj<AdaptGetNumAdaptQuantities();jj++)
    InterpolIrregNodeVals2D(AdaptGetRefNval0(jj),AdaptGetRefNval1(jj));

  #ifdef TESTREF
    DisplayMsgLn("Vergroebern abgeschlossen ...");
  #endif

  nachlauf = 1;  /* Sekundaere Kriterien ueberpruefen */
  /* Nachlauf-Schleife ueber alle Elemente ( verfeinerte Gebiete abrunden ) */
  max_index = ElListSize();  /* Hoechsten Index holen */
  for (i=0;i<max_index;i++) {  /* Schleife ueber alle Elemente */
      if (ElGetElement(i)!=NULL) {  /* wenn Element existiert */
          if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
              /* wenn noetig, Element(e) verfeinern */
              RN2D_a[ElGetElementType(i)-1](i);
          }
      }
  }
  /*
     Jetzt sind auch alle Elemente verfeinert, die vorher mindestens
     ref_neighbours verfeinerte Nachbarelemente hatten und noch nicht
     verfeinert waren. Ausserdem haben 1D-Elemente nur Nachbarn von
     gleichem Verfeinerungsgrad.
  */
  return ret;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R2D_a_1D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef2D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer 2D-Kluftnetzwerke)

   letzte Aenderung:      R.Kaiser     31.10.1997
                                                                          */
/**************************************************************************/
void R2D_a_1D ( long number )
{
  STATIC long *kanten;  /* Feld mit Elementkante */
  STATIC Kante *kinder[2];  /* Kinder der Kante */
  STATIC long *kantenliste;  /* Feld mit Kante der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  long *elem;
  int anz, aktiv, level;
  int i, j;  /* Laufvariable */
  Kante *kante = NULL;  /* Zeiger auf Elementkante */
  Kante *vorg_kante = NULL;  /* Vorgaenger der Elementkante */
  long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */

  neue_elemente = (long *) Malloc(2*sizeof(long));
    /* sind spaeter die Kinder von number */

  for (i=0;i<2;i++)  /* neue Elemente erzeugen */
      neue_elemente[i] = ElCreateElement(1,number,-1);


  level = ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)+2l));
     /* Anzahl der verfeinerten Elemente eintragen */



  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  kante = GetEdge(kanten[0]);
  if (kante->kinder[0]<0l) {
      /* Diese Kante wurde noch nicht verfeinert! */
      if (kante->vorgaenger>=0l) {  /* es exist. ein Vorgaenger */
          /* Vorgaenger der Kante holen, wird wahrscheinlich gebraucht */
          vorg_kante = GetEdge(kante->vorgaenger);
          elem = GetEdge2DElems(kante->vorgaenger,&anz);
          for (j=0;j<anz;j++) { /* Schleife ueber alle Nachbarn der Vorgaengerkante */
              aktiv = ElGetElementActiveState(elem[j]);
              if (aktiv == 1l)
                  R2D_a_2D(elem[j]);  /* ==> verfeinern */
          }
      }
      /*
         Jetzt sind alle Nachbarn so fein wie ich
         ==> ich kann die Kante und mich verfeinern!
      */
      /* Neue Kante und neuen Knoten erzeugen */
      CreateTwoEdges(kanten[0]);
  }

  kinder[0] = GetEdge(kante->kinder[0]);  /* Kantenkind 1 */
  kinder[1] = GetEdge(kante->kinder[1]);  /* Kantenkind 2 */
  /*
     Jetzt existiert eine verfeinerte Kante; Randbedingungen sind uebertragen!
  */
  /* Kantenverzeichnis updaten */
  kinder[0]->nachbar_1D = neue_elemente[0];
  kinder[1]->nachbar_1D = neue_elemente[1];

  /*
     Jetzt ist das bestehende Kantenverzeichnis angepasst und alle
     Nachbarelemente sind, soweit noetig (sekundaeres Kriterium),
     verfeinert. Der Neue Knoten wurde ggf. erzeugt.
  */

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

  /* Randknoten-Status etc. ueberpruefen */
  CheckNodeState2D(kinder[0]->knoten[1],kante);

  /* Anzahl Aktive Elemente anpassen (+2 neue -1 altes) */
  anz_active_elements++;
  anz_active_1D++;
  anz_1D += 2l;
  msh_no_line+=2l;

  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);

  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */

  if (nachlauf) {
      /* evtl. vorhandene groebere 2D-Nachbarn muessen verfeinert werden */
      elem = GetEdge2DElems(kanten[0],&anz);
      for (i=0;i<anz;i++) {  /* Schleife ueber alle Nachbarn */
          if (ElGetElementChilds(elem[i])==NULL)
              /* und ist groeber als ich */
              R2D_a_2D(elem[i]);
              /* der Nachbar wird DIREKT VERFEINERT! */
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: R2D_a_2D
                                                                          */
/* Aufgabe:
   Elementverfeinerungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef2D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer 2D-Kluftnetzwerk)

   letzte Aenderung:      R.Kaiser     31.10.1997
                                                                          */
/**************************************************************************/
void R2D_a_2D ( long number )
{
  STATIC Kante *kinder1;  /* Kind einer Nachbarkante */
  STATIC Kante *kinder2;  /* Kind einer Nachbarkante */
  STATIC Kante *neue_kante;  /* Kante zum Mittelpunkt des Elements */
  STATIC long mittelknoten;  /* Nummer des neuen Mittelknotens */
  STATIC Knoten *knoten;  /* Zeiger auf Knotenstruktur des Mittelknotens */
  STATIC Knoten *eckknoten;  /* Zeiger auf Struktur eines Eckknoten */
  STATIC long *alte_eckknoten;  /* Feld mit Knotennummern von number */
  STATIC long *kantenliste;  /* Feld mit Kanten der neuen Elemente */
  STATIC long *knotenliste;  /* Feld mit Knoten der neuen Elemente */
  long *elem;
  int anz, aktiv, level;
  int i, j, k;  /* Laufvariable */
  long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  Kante *vorg_kante = NULL;  /* Vorgaenger einer Nachbarkante */
  long *neue_elemente;  /* Feld mit Elementnummern der neuen Elemente */
  long neue_knoten[4];  /* Feld mit Knotennummern der neuen Randknoten */
  long zw_kantenliste[12];
    /* Feld mit den neuen, verfeinerten Randkanten im Umlaufsinn und den 4
       neuen Innenkanten */

  neue_elemente = (long *) Malloc(4*sizeof(long));
    /* sind spaeter die Kinder von number */

  alte_eckknoten = ElGetElementNodes(number);

  for (i=0;i<4;i++)  /* neue Elemente erzeugen */
      neue_elemente[i] = ElCreateElement(2,number,-1);

  level = ElGetElementLevel(neue_elemente[0])-1;
  AdaptSetNumEleRefLevelX(level,(AdaptGetNumEleRefLevelX(level)+4l));

  /*ref_level_anz[ElGetElementLevel(neue_elemente[0])-1] += 4l;*/
    /* Anzahl der verfeinerten Elemente eintragen */

  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  for (i=0;i<4;i++) {  /* Schleife ueber alle Kanten */
      kante = GetEdge(kanten[i]);
      if (kante->kinder[0]<0l) {
          /* Diese Kante wurde noch nicht verfeinert! */
          if (kante->vorgaenger>=0l) {  /* es exist. ein Vorgaenger */
              /* Vorgaenger der Kante holen, wird wahrscheinlich gebraucht */
              vorg_kante = GetEdge(kante->vorgaenger);
              elem = GetEdge2DElems(kante->vorgaenger,&anz);
              for (j=0;j<anz;j++) {
                  aktiv = ElGetElementActiveState(elem[j]);
                  if (aktiv == 1l)
                      R2D_a_2D(elem[j]);  /* ==> verfeinern */
              }

              if (kante->nachbar_1D<0l) {  /* "benachbartes" 1D-Element exist. */
                  if (vorg_kante->nachbar_1D>=0l)  /* und ist groeber als ich */
                      R2D_a_1D(vorg_kante->nachbar_1D);  /* ==> verfeinern */
              }
          }
          /*
             Jetzt sind alle Nachbarn so fein wie ich
             ==> ich kann die Kante und mich verfeinern!
          */
          /* Neue Kante und neuen Knoten erzeugen */
          CreateTwoEdges(kanten[i]);
      }
      /*
         Jetzt existiert eine verfeinerte Kante!
      */
      kinder1 = GetEdge(kante->kinder[0]);  /* Kantenkind 1 */
      kinder2 = GetEdge(kante->kinder[1]);  /* Kantenkind 2 */
      neue_knoten[i] = kinder1->knoten[1];
        /* bereits vorhandenen Randknoten merken */

      for (j=0;j<4;j++)
          if (alte_eckknoten[j] == kante->knoten[0]) {
              if (alte_eckknoten[(j+1)%4] == kante->knoten[1]) {
                  /* Kante und Element haben gleichen Drehsinn */
                  /* Neue Randkanten in Zwischenspeicher eintragen */
                  zw_kantenliste[2*i] = kante->kinder[0];
                  zw_kantenliste[2*i+1] = kante->kinder[1];
                  /* Kantenverzeichnis updaten */
                  Assign2DElementEdges(kante->kinder[0],neue_elemente[i]);
                  Assign2DElementEdges(kante->kinder[1],neue_elemente[(i+1)%4]);
              }
              if (alte_eckknoten[(j+3)%4] == kante->knoten[1]) {
                  /* Kante und Element haben entgegengesetzten Drehsinn */
                  /* Neue Randkanten in Zwischenspeicher eintragen */
                  zw_kantenliste[2*i] = kante->kinder[1];
                  zw_kantenliste[2*i+1] = kante->kinder[0];
                  /* Kantenverzeichnis updaten */
                  Assign2DElementEdges(kante->kinder[0],neue_elemente[(i+1)%4]);
                  Assign2DElementEdges(kante->kinder[1],neue_elemente[i]);
              }
         }
  }
  /*
     Jetzt sind zw_kantenliste und neue_knoten ermittelt (und neue Knoten
     erzeugt) sowie das bestehende Kantenverzeichnis angepasst und alle
     Nachbarelemente, soweit noetig (sekundaeres Kriterium), verfeinert.
     Die Randbedingungen sind angepasst.
  */
  /* Mittelknoten erzeugen (ist immer regulaerer Innenknoten) */
  knoten = NewNode();
  for (i=0;i<4;i++) {
      eckknoten = GetNode(alte_eckknoten[i]);
      knoten->x += eckknoten->x;
      knoten->y += eckknoten->y;
      knoten->z += eckknoten->z;
  }
  knoten->x /= 4.0;
  knoten->y /= 4.0;
  knoten->z /= 4.0;
  mittelknoten = AddNode(knoten);

  /* modellspezifische Knotendaten verfeinern */
  RefinePlainValues(knoten,alte_eckknoten);

  /* Innere Kanten erzeugen */
  for (i=0;i<4;i++) {
      neue_kante = NewEdge();
      /* Knoten eintragen */
      neue_kante->knoten[0] = neue_knoten[i];
      neue_kante->knoten[1] = mittelknoten;
        /* alle Kanten zeigen zur Mitte */
      /* Nachbarn (nur 2D) eintragen */
      zw_kantenliste[8+i] = AddEdge(neue_kante);
      Assign2DElementEdges(zw_kantenliste[8+i],neue_elemente[i]);
      Assign2DElementEdges(zw_kantenliste[8+i],neue_elemente[(i+1)%4]);
  }

  /* Element-zu-Knoten-Liste fuer number anpassen */
  ReAssign2DElementNodes(number,alte_eckknoten,4);
  /* Kanten, Knoten und Matrizen bei den neuen Elementen eintragen */
  for (i=0;i<4;i++) {
      /* neue Knoteneintraege erzeugen */
      knotenliste = (long *) Malloc(4*sizeof(long));
      knotenliste[0] = alte_eckknoten[i];
      knotenliste[1] = neue_knoten[i];
      knotenliste[2] = mittelknoten;
      knotenliste[3] = neue_knoten[(i+3)%4];
      ElSetElementNodes(neue_elemente[i],knotenliste);
      /* neue Kanteneintraege erzeugen */
      kantenliste = (long *) Malloc(4*sizeof(long));
      kantenliste[0] = zw_kantenliste[2*i];
      kantenliste[1] = zw_kantenliste[i+8];
      kantenliste[2] = zw_kantenliste[(i+3)%4+8];
      kantenliste[3] = zw_kantenliste[(2*i+7)%8];
      ElSetElementEdges(neue_elemente[i],kantenliste);
      /* Element-zu-Knoten-Liste fuer neue Elemente anpassen */
      Assign2DElementNodes(neue_elemente[i],knotenliste,4);
  }
  /* Vorgaenger */
  ElSetElementActiveState(number,0);                      /* deaktivieren */
  ElSetElementChilds(number,neue_elemente);           /* Kinder eintragen */
  ElSetElementRefineState(number,0);           /* Verfeinern-Schalter aus */
  /* Randknoten-Status etc. ueberpruefen */
  for (i=0;i<4;i++)
    CheckNodeState2D(neue_knoten[i],GetEdge(kanten[i]));

  /* Anzahl Aktive Elemente anpassen (+4 neue -1 altes) */
  anz_active_elements += 3;
  anz_active_2D +=3;
  anz_2D += 4l;
  msh_no_quad+=4l; //SB
  /* modellspezifische Elementdaten verfeinern */
  RefElementData(number,neue_elemente);
  /* Speicherfreigaben werden nicht benoetigt, da alle neu allokierten
     Bereiche in andere Datenstrukturen eingebunden wurden */

  if (nachlauf) {
      /* Prueflauf ueber alle groeberen Nachbarn, ob die nach der
         Verfeinerung von number evtl. auch verfeinert werden muessen. */
      for (i=0;i<4;i++) {  /* Schleife ueber alle Kanten */
          kante = GetEdge(kanten[i]);  /* Kante holen */
          elem = GetEdge2DElems(kanten[i],&anz);
          for (j=0;j<anz;j++) {
              if (number == elem[j])   /* ich bin Nachbar[j] */
                  for(k=0;k<anz;k++) {
                      if (k!=j)
                          if (ElGetElementChilds(elem[k])==NULL)
                             /* Nachbar ist groeber als ich */
                             RN2D_a_2D(elem[k]);
                  }
          }
          if (kante->nachbar_1D>=0l) {  /* Nachbar existiert */
              if (ElGetElementChilds(kante->nachbar_1D)==NULL)
                  /* und ist groeber als ich */
                  /* msr 0196, kann direkt verfeinert werden, weil 2D-Nachbarn
                     sicher verfeinert werden. RN2D_a_1D(kante->nachbar_1D); */
                  R2D_a_1D(kante->nachbar_1D);
          }
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM2D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef2D_a. Es werden nur die Massenerhaltung durchgefuehrt und
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
   06/1995     MSR        Erste Version
   09/1995     MSR        nur Massenbilanz
   11/1995     MSR        status==4 - Abfrage eingebaut
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer 2D-Kluftnetzwerke)

   letzte Aenderung:       R.Kaiser     31.10.1997
                                                                          */
/**************************************************************************/
int RRM2D_a_1D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  STATIC int i, k;  /* Lauf- und Hilfsvariablen */
  STATIC long *kanten;  /* Feld mit Elementkanten */
  long *elem;
  int anz;
  int j, ii;  /* Lauf- und Hilfsvariablen */
  int r[2];  /* Refine-States */
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  Kante *kantenkind = NULL;  /* Kind einer Kante */

  if (num_ref_quantity == 0) {
    kinder = ElGetElementChilds(number);  /* Kinder holen */
    r[0] = ElGetElementRefineState(kinder[0]);  /* verfeinerungsstati */
    r[1] = ElGetElementRefineState(kinder[1]);
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

    kanten = ElGetElementEdges(number);  /* Kante von number holen */
    kante = GetEdge(kanten[0]);

    /* ueberpruefen, ob die direkten Nachbarn meiner Kinder
       Kinder haben */
    for (j=0;j<2;j++) {  /* Schleife ueber die Kantenkinder */
        kantenkind = GetEdge(kante->kinder[j]);  /* Kantenkind holen */
        /* Wenn das Nachbarskind Kinder hat, muss es die Kinder
           vergroebern, damit ich meine Kinder vergroebern darf */
        elem = GetEdge2DElems(kante->kinder[j],&anz);
        for (ii=0;ii<anz;ii++) {  /* Schleife ueber die potentiellen 2D-Nachbarn */
            if (ElGetElementChilds(elem[ii])!=NULL)
                if (!RRM2D_a_2D(elem[ii],num_ref_quantity,zwischenspeicher)) {
                    /* Es wurde nicht vergroebert ==> Abbruch */
                    for (k=0;k<2;k++)  /* Schleife ueber alle Kinder */
                        if (r[k]==-1)
                            ElSetElementRefineState(kinder[k],0);
                        return 0;
                }
        }
    }
    /*
     An dieser Stelle komme ich nur an, wenn ich mich vergroebern darf!
     Keines meiner Nachbarelemente ist jetzt noch feiner als ich!
     Eine 1D-Vergroeberung hat keinen laengerfristigen Einfluss auf den
     Knotenstatus und die Randbedingungen!
    */

    for (i=0;i<2;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
    rr_ret = 1;
  }


  /* Massenerhaltung */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);


  return 1;  /* vergroebern hat geklappt */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RRM2D_a_2D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef2D_a. Es werden nur die Massenerhaltung durchgefuehrt und
   die zu vergroebernden Elemente markiert (Status 4) (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers des zu verfeinernden
                  Elements
                                                                          */
/* Ergebnis:
   1: habe meine Kinder vergroebert; 0: habe sie nicht vergroebert
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   09/1995     MSR        nur Massenbilanz
   11/1995     MSR        status==4 - Abfrage eingebaut
   31.10.1997  R.Kaiser   Je Kante bel. Anzahl an 2D-Nachbarelementen
                          (fuer 2D-Kluftnetzwerken)

   letzte Aenderung:       R.Kaiser      31.10.1997
                                                                          */
/**************************************************************************/
int RRM2D_a_2D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
  int i, j, m, k;  /* Lauf- und Hilfsvariablen */
  int r[4];  /* Refine-States */
  long *elem;
  int anz;
  long *kinder = NULL;  /* Feld mit meinen Kindern */
  long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  Kante *kantenkind = NULL;  /* Kind einer Kante */


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
  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  for (i=0;i<4;i++) {  /* Schleife ueber alle Kanten */
      kante = GetEdge(kanten[i]);
      /* ueberpruefen, ob die direkten Nachbarn meiner Kinder
         Kinder haben */

      for (j=0;j<2;j++) {  /* Schleife ueber die Kantenkinder */
          kantenkind = GetEdge(kante->kinder[j]);  /* Kantenkind holen */
          /* Wenn das Nachbarskind Kinder hat, muss es die Kinder
             vergroebern, damit ich meine Kinder vergroebern darf */
              elem = GetEdge2DElems(kante->kinder[j],&anz);
              for (m=0;m<anz;m++) {
                  if (ElGetElementPred(elem[m]) != number)
                      if (ElGetElementChilds(elem[m])!=NULL)
                          if (!RRM2D_a_2D(elem[m],num_ref_quantity,zwischenspeicher)) {
                              /* Es wurde nicht vergroebert ==> Abbruch */
                              for (k=0;k<4;k++)  /* Schleife ueber alle Kinder */
                                  if (r[k]==-1)
                                      ElSetElementRefineState(kinder[k],0);
                              return 0;
                          }
              }

          if (kantenkind->nachbar_1D>=0l) {  /* wenn 1D-Nachbar existiert */
              if (ElGetElementChilds(kantenkind->nachbar_1D)!=NULL)
                  if (!RRM2D_a_1D(kantenkind->nachbar_1D,num_ref_quantity,zwischenspeicher)) {
                      /* Es wurde nicht vergroebert ==> Abbruch */
                      for (k=0;k<4;k++)  /* Schleife ueber alle Kinder */
                          if (r[k]==-1)
                              ElSetElementRefineState(kinder[k],0);
                      return 0;
                  }
          }
      }
  }
  /*
     An dieser Stelle komme ich nur an, wenn ich mich vergroebern darf!
     Keines meiner Nachbarelemente ist jetzt noch feiner als ich!
  */


  /* Bei der Massenerhaltung muss beruecksichtigt werden, dass schon ein
       Korrekturwert in conc[1] stehen kann */
  for (i=0;i<4;i++)
      /* Status der Kinder auf 4 setzen */
      ElSetElementRefineState(kinder[i],4);
  rr_ret = 1;

  }

  /* Massenerhaltung */
  ExecuteMassCont(number,num_ref_quantity,zwischenspeicher);


  return 1;  /* vergroebern hat geklappt */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RR2D_a_1D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef2D_a. Das Element kann direkt vergroebert werden, es sind
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
   06/1995     MSR        Erste Version
   09/1995     MSR        Nur noch direktes Vergroebern
                                                                          */
/**************************************************************************/
void RR2D_a_1D ( long number )
{
  static int i, ii;  /* Lauf- und Hilfsvariablen */
  static long *kinder;  /* Feld mit meinen Kindern */
  static long *kanten;  /* Feld mit Elementkanten */
  static Kante *kante;  /* Eine Nachbarkante */
  static Kante *kantenkind;  /* Kind einer Kante */
  static long mittelknoten;  /* Nummer des neuen Mittelknotens */
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  kanten = ElGetElementEdges(number);  /* Kante von number holen */
  kante = GetEdge(kanten[0]);
  kantenkind = GetEdge(kante->kinder[1]);  /* Kantenkind holen */


  /* Anzahl der vergroeberten Elemente austragen */
  AdaptSetNumEleRefLevelX(ElGetElementLevel(number),
                     (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))-2l));

  if (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))==0l)
      AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);



  /*if ((ref_level_anz[ElGetElementLevel(number)]-=2l)==0l)
      ref_level--;*/  /* evtl. ref_level veraendern */


  /* Vergroebern */
  /* Mittelknoten ermitteln */
  mittelknoten = kantenkind->knoten[0];
  for (i=0;i<2;i++) {
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign1DElementNodes(kinder[i],ElGetElementNodes(kinder[i]),2);
      /* Kinder aus Kantenverzeichnis austragen */
      ReAssign1DElementEdges(kinder[i],ElGetElementEdges(kinder[i]),1);
  }
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign1DElementNodes(number,ElGetElementNodes(number),2);
  /* Mittelknoten-Existenz ueberpruefen */
  if (GetNode2DElems(mittelknoten,&ii)==NULL)
      DeleteNode(mittelknoten);
  else {
      SetNodeState(mittelknoten,-2);
  }
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


/**************************************************************************/
/* ROCKFLOW - Funktion: RR2D_a_2D
                                                                          */
/* Aufgabe:
   Elementvergroeberungsfunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef2D_a. Das Element kann direkt vergroebert werden, es sind
   keine weiteren Abhaengigkeiten zu beruecksichtigen.

   Verbessern: Variable mittelknoten ist ueberfluessig
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des Vorgaengers des zu verfeinernden
                  Elements
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   09/1995     MSR        Nur noch direktes Vergroebern
                                                                          */
/**************************************************************************/
void RR2D_a_2D ( long number )
{
  static int i, ii;  /* Lauf- und Hilfsvariablen */
  static long *kinder;  /* Feld mit meinen Kindern */
  static long *knoten;  /* Feld mit Elementknoten */
  static long randknoten[4];  /* Feld mit Knotennummern der Randknoten */
  static long mittelknoten;  /* Nummer des neuen Mittelknotens */
  kinder = ElGetElementChilds(number);  /* Kinder holen */
  /* Anzahl der vergroeberten Elemente austragen */
  AdaptSetNumEleRefLevelX(ElGetElementLevel(number),
                     (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))-4l));

  if (AdaptGetNumEleRefLevelX(ElGetElementLevel(number))==0l)
      AdaptSetActualRefLevel(AdaptGetActualRefLevel()-1);

  /* if ((ref_level_anz[ElGetElementLevel(number)]-=4l)==0l)
      ref_level--; */ /* evtl. ref_level veraendern */
  /* Vergroebern */
  for (i=0;i<4;i++) {
      /* Elementknoten holen */
      knoten = ElGetElementNodes(kinder[i]);
      /* Element-zu-Knoten-Liste anpassen (Kinder raus) */
      ReAssign2DElementNodes(kinder[i],knoten,4);
      /* Randknoten ermitteln */
      randknoten[i] = knoten[1];
      /* Kinder aus Kantenverzeichnis austragen
         (Mittelkanten werden automatisch entfernt) */
      ReAssign2DElementEdges(kinder[i],ElGetElementEdges(kinder[i]),4);
  }
  /* Mittelknoten ermitteln */
  mittelknoten = knoten[2];
  /* Element-zu-Knoten-Liste anpassen (Vorgaenger rein) */
  Assign2DElementNodes(number,ElGetElementNodes(number),4);
  /* Mittelknoten entfernen */
  DeleteNode(mittelknoten);
  /* Randknoten-Existenz ueberpruefen */
  for (i=0;i<4;i++) {
      if ( (GetNode1DElems(randknoten[i],&ii)==NULL) &&
           (GetNode2DElems(randknoten[i],&ii)==NULL) )
          DeleteNode(randknoten[i]);
      else {
          /* Knoten bleibt erhalten ==> Status ueberpruefen */
          /* Status >=0 oder -3: Knoten wird irregulaer (-2)
                    -1: Knoten wird irregulaer (-2)
                    -2: Knoten wurde geloescht bzw. einzelnes 1D- oder 2D-Element */
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
/* ROCKFLOW - Funktion: RN2D_a_1D
                                                                          */
/* Aufgabe:
   Elementnachlauffunktion fuer 1D-Elemente ueber das Verfahren
   aus NetRef2D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   1: es wurde verfeinert; 0: es wurde nicht verfeinert
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RN2D_a_1D ( long number )
{
  STATIC long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  kante = GetEdge(kanten[0]);  /* Kante holen */
  if (kante->kinder[0]>=0l) {  /* Diese Kante wurde bereits verfeinert! */
      /* ==> 1. ich muss auch verfeinert werden! */
      R2D_a_1D(number);  /* number wird verfeinert */
      /* Der Prueflauf ueber die Nachbarn startet von der
         Verfeinerungsfunktion aus !!! */
      return 1;  /* Es wurde verfeinert */
  }
  return 0;  /* Es wurde nicht verfeinert */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RN2D_a_2D
                                                                          */
/* Aufgabe:
   Elementnachlauffunktion fuer 2D-Elemente ueber das Verfahren
   aus NetRef2D_a (rekursiv).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Elementnummer des zu verfeinernden Elements
                                                                          */
/* Ergebnis:
   1: es wurde verfeinert; 0: es wurde nicht verfeinert
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
int RN2D_a_2D ( long number )
{
  STATIC int anz;  /* 1. Kriterium: Anzahl der Kanten mit Kindern */
  STATIC int verf;  /* 2. Kriterium: feines 1D-Element benachbart */
  int i;  /* Laufvariable */
  long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  anz = verf = 0;
  kanten = ElGetElementEdges(number);  /* Kanten von number holen */
  for (i=0;i<4;i++) {  /* Schleife ueber alle Kanten */
      kante = GetEdge(kanten[i]);  /* Kante holen */
      if (kante->kinder[0]>=0l) {  /* Diese Kante wurde bereits verfeinert! */
          anz++;  /* Anzahl der feineren Kanten erhoehen */
          /* wenn die Kante verfeinert ist, wird / wurde das 1D-Element
            irgendwann zwangslaeufig verfeinert
            ==> folgende Abfrage ist o.k. */
          if (kante->nachbar_1D>=0l) {
              /* 2. Kriterium: number ist Nachbar von feinerem 1D-Element */
              verf = 1;
          }
      }
  }
  if ( (anz>=AdaptGetRefNeighbours2d()) || (verf) ) {
      /* Wenn mehr feinere Nachbarn als zugelassen existieren oder
         mindestens ein feineres 1D-Element benachbart ist, wird
         number verfeinert. */
      R2D_a_2D(number);  /* number wird verfeinert */
      /* Der Prueflauf ueber die Nachbarn startet von der
         Verfeinerungsfunktion aus !!! */
      return 1;  /* Es wurde verfeinert */
  }
  return 0;  /* Es wurde nicht verfeinert */
}


void R2D_a_3D ( long number )
{
static long dummy;
dummy = number;
}

void RR2D_a_3D ( long number )
{
static long dummy;
dummy = number;
}

int RRM2D_a_3D ( long number, int num_ref_quantity, double *zwischenspeicher )
{
static long dummy;
dummy = number;
zwischenspeicher=zwischenspeicher;  /* Wegen Warnung */
num_ref_quantity=num_ref_quantity;
return 0;
}

int RN2D_a_3D ( long number )
{
static long dummy;
dummy = number;
return 0;
}
  /* Die Elementfunktionen fuer 3D-Elemente fuer NetRef2D_a werden nicht
     implementiert */


/**************************************************************************/
/* ROCKFLOW - Funktion: InterpolIrregNodeVals2D
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
   11/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void InterpolIrregNodeVals2D ( int nval0, int nval1 )
{
  static long i;  /* Laufvariablen */
  static Knoten *k;  /* Knotenzeiger zum Interpolieren der irreg. Knoten */
  /*static long *nn;*/  /* Knotenfeldzeiger */



  for (i=0l;i<NodeListSize();i++) {
      if ( (k = GetNode(i)) != NULL) {
          if (k->status == -2) {
             if (k->anz1d>0) {
                  /* 2 1D-Elemente eingetragen (und evtl. weitere Elemente) */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems1d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems1d[1]))[0],nval0) ) * 0.5 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems1d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems1d[1]))[0],nval1) ) * 0.5 );
              }

              else  {
                  /* 2 oder 4 (paarweise eingetragen !!!) 2D-Elemente eingetragen */
                  SetNodeVal(i,nval0,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval0) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval0) ) * 0.5 );
                  SetNodeVal(i,nval1,
                     ( GetNodeVal((ElGetElementNodes(k->elems2d[0]))[0],nval1) +
                       GetNodeVal((ElGetElementNodes(k->elems2d[1]))[0],nval1) ) * 0.5 );
              }
          }
      }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckNodeState2D
                                                                          */
/* Aufgabe:
   Ueberprueft und setzt Status eines potentiellen irregulaeren
   (Kanten-)Knotens, der auch Randknoten sein kann.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long irrnode: potentieller irr. Knoten
   E Kante *kante: Kante, in dessen Mitte irrnode liegt
                                                                          */
/* Ergebnis:
   Knotenstatus
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)

   letzte Aenderung:      R.Kaiser     31.10.1997
                                                                          */
/**************************************************************************/
int CheckNodeState2D ( long irrnode, Kante *kante )
{
  static int s[2];  /* Knotenstati */
  static int anzahl, i;
  anzahl = kante->anz_nachbarn_2D;
  for (i=0;i<anzahl;i++) {
      if (ElGetElementChilds(kante->nachbarn_2D[i])==NULL) {
          /* Knoten ist irregulaer, wenn mindestens ein kinderloser Nachbar
             existiert */
          SetNodeState(irrnode,-2);
          return -2;
      }
  }
  if ((kante->nachbar_1D>=0l) && (ElGetElementChilds(kante->nachbar_1D)==NULL)) {
      /* Knoten ist irregulaer, wenn mindestens ein kinderloser Nachbar
         existiert */
      SetNodeState(irrnode,-2);
      return -2;
  }

  /* Knoten ist entweder regulaerer Rand- oder Innenknoten */
  s[0] = GetNodeState(kante->knoten[0]);
  s[1] = GetNodeState(kante->knoten[1]);
     /* Knotenstati holen */

  if ((s[0]==-3) && (s[1]==-3)) {
      /* Kante ist Randkante */
      SetNodeState(irrnode,-3);
      return -3;
  }

  /* Knoten ist regulaer */
  SetNodeState(irrnode,-1);
  return -1;
}


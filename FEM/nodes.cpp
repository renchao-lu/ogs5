/**************************************************************************/
/* ROCKFLOW - Modul: nodes.c
                                                                          */
/* Aufgabe:
   Knotenverzeichnis-Management
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
   06/1998     AH         Listentypen: Listen, Baeumen etc.
   05/1999     CT         Neu: CopyNodeVals
   08/2000     RK         Datenstruktur fuer Gitteradaption erweitert
                          (nur zum Aufbau der Kanten- und
                          Flaechenverzeichnisse (Startnetz))
                          Keine Aktualisierung waehrend der Adaption !!!
                          zugehrige Funktionen: GetNodeEdges, AssignEdge
                                                 GetNodePlains, AssignPlain
   09/2000     MK         Neu: AddSolNodeValsVec (Newton Scheme) 
   08/2001     MK         Neu: TransferNodeValsDim, AddNodeValsDim        
                          fuer UpdateNodeValuesDim(ls) (Newton Scheme)
   10/2001     AH         Inverses Modellieren
                          InitModelNodeData
                          DestroyNodeList: Pruefen vor Speicherfreigabe.
                          Initialisierung von nval_intern.  ah rfm
   11/2001     AH         Warnung entfernt
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt 
   05/2003     OK         CreateModelNodeData for dynamic arrays  
   07/2003     WW         Change of the variables associated with 2D grid  
      
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#ifndef  ENABLE_ADT

/* Header / Andere intern benutzte Module */
#include "rf_pcs.h" 
#include "nodes.h"
#include "ptrarr.h"
#include "edges.h"
#include "plains.h"
#include "elements.h"
#include "loop_pcs.h"
#include "mathlib.h"
// GeoLib
#include "geo_pnt.h"

/* Interne (statische) Deklarationen */
vector<CFDMNode*>FDM_node_vector;

/* Definitionen */
long NodeListLength;
  /* Anzahl der gespeicherten Knoten, siehe Header */
long NodeListLength_L;
long NodeListLength_H;

long *NodeNumber = NULL;
  /* Zeiger auf Feld, in dem unter index nachgesehen werden kann, welche
     reale Knotennummer sich dahinter verbirgt (nach Umnummerieren);
     siehe Header */
static ZeigerFeld *nodelist = NULL;
  /* Knotenverzeichnis */
static long lowest_free_node;
  /* niedrigste freie Knotennummer */
LongXFuncLIX GetNodeXDElems[3] = { GetNode1DElems,
                                   GetNode2DElems,
                                   GetNode3DElems };
  /* Zeigerfeld auf die Funktionen zum Holen der Knotenelemente */
VoidXFuncVoid InitInternNodeData;
VoidXFuncVoidX DestroyInternNodeData;


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateNodeList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Knotenverzeichnis; der erste Knoten hat spaeter
   die Nummer 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
   07/2003     WW         A little change to 2D
                                                                          */
/**************************************************************************/
int CreateNodeList (void)
{
  NodeListLength = 0l;

  NodeListLength_L= 0l;
  NodeListLength_H = 0l;
  
  lowest_free_node = 0l;
  nodelist = CreatePtrArray(NODE_START_SIZE,NODE_INC_SIZE);
  return !(nodelist == NULL);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyNodeList
                                                                          */
/* Aufgabe:
   Entfernt komplettes Knotenverzeichnis aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
   08/2000     AH         Sicheres Handling beim Speicherfreigabe ah rfm
   11/2002   OK   execute only if nodelist exists
   02/2004   TK   Extension for execute only if nodelist exists
                                                                          */
/**************************************************************************/
void DestroyNodeList (void)
{

  if(nodelist || nodelist->elements!=NULL) {
  long i=0;
  long l = PtrArrayLength(nodelist);
  for (i = 0l; i < l; i++) {
      if (nodelist->elements[i]!=NULL) {
          ((Knoten *) nodelist->elements[i])->newnodes = (long *)Free(((Knoten *) nodelist->elements[i])->newnodes);
          ((Knoten *) nodelist->elements[i])->elems1d = (long *)Free(((Knoten *) nodelist->elements[i])->elems1d);
          ((Knoten *) nodelist->elements[i])->elems2d = (long *)Free(((Knoten *) nodelist->elements[i])->elems2d);
          ((Knoten *) nodelist->elements[i])->elems3d = (long *)Free(((Knoten *) nodelist->elements[i])->elems3d);
          ((Knoten *) nodelist->elements[i])->plains = (long *)Free(((Knoten *) nodelist->elements[i])->plains);
          ((Knoten *) nodelist->elements[i])->edges = (long *)Free(((Knoten *) nodelist->elements[i])->edges);
/*OKToDo*/
if(!(GetRFControlModel()==-1)) {
          if ( ((Knoten *) nodelist->elements[i])->nval )
            ((Knoten *) nodelist->elements[i])->nval = (double *)Free(((Knoten *) nodelist->elements[i])->nval);
}
          if ( ((Knoten *) nodelist->elements[i])->nval_intern ) 
            ((Knoten *) nodelist->elements[i])->nval_intern = DestroyInternNodeData(((Knoten *) nodelist->elements[i])->nval_intern);
          nodelist->elements[i] = Free((Knoten *) nodelist->elements[i]);
      }
  }
  l = PtrArrayLength(nodelist);
  if (l>=0) ResizePtrArray(nodelist,0l);
  NodeListLength = 0l;

  NodeListLength_L = 0l;
  NodeListLength_H = 0l;
  
  lowest_free_node = 0l;
  if (l>=0) DestroyPtrArray(nodelist);
  NodeNumber = (long *)Free(NodeNumber);
  nodelist = NULL; //OK41
 }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: NewNode
                                                                          */
/* Aufgabe:
   Erzeugt neuen initialisierten Knoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   Zeiger auf neue Knotendatenstruktur
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
                                                                          */
/**************************************************************************/
Knoten *NewNode ( void )
{
  static int i;
  static Knoten *k;
  k = (Knoten *) Malloc(sizeof(Knoten));
  k->x = k->y = k->z = k->source = 0.0;
  k->node_start_number = -1;
  k->index = -1l;
  k->newnodes = NULL;
  k->anz_new_nodes = 0;
  k->elems1d = NULL;
  k->elems2d = NULL;
  k->elems3d = NULL;
  k->plains = NULL;
  k->edges = NULL;
  k->anz_plains = 0;
  k->anz_edges = 0;
  k->anz1d = k->anz2d = k->anz3d = 0;

  k->status = -1;

/*OK*/
if(!(GetRFControlModel()==-1)&&!(GetRFControlModel()==9999)) {
  k->nval_intern = InitInternNodeData();
}
  k->nval = (double *)Malloc(anz_nval*sizeof(double));
  for (i=0;i<anz_nval;i++)
      k->nval[i] = nval_data[i].vorgabe;
  return k;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateNodeGeometry
                                                                          */
/* Aufgabe:
   Erzeugt neuen initialisierten Knoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   Zeiger auf neue Knotendatenstruktur
                                                                          */
/* Programmaenderungen:
   11/1999   RK   Kapselung Gitteradaption
   07/2000   OK   Initialisierung des Knoten-Daten-Feldes
   10/2001   AH   Initialisierung von nval_intern.  ah rfm
                                                                          */
/**************************************************************************/
Knoten *CreateNodeGeometry ( void )
{
  static Knoten *k;
  k = (Knoten *) Malloc(sizeof(Knoten));
  k->x = k->y = k->z = k->source = 0.0;
  k->node_start_number = -1;
  k->index = -1l;
  k->newnodes = NULL;
  k->anz_new_nodes = 0;
  k->elems1d = NULL;
  k->elems2d = NULL;
  k->elems3d = NULL;
  k->plains = NULL;
  k->edges = NULL;
  k->anz_plains = 0;
  k->anz_edges = 0;
  k->anz1d = k->anz2d = k->anz3d = 0;

  k->status = -1;

  k->nval = NULL; /* OK rf3419 */
#ifdef PCS_NOD
  for(i=0;i<MAX_PCS_NUMBER;i++)
#endif
  k->nval_intern = NULL;  /* ah rfm */
  k->free_surface = 0;

  return k;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: AddNode
                                                                          */
/* Aufgabe:
   Haengt Knoten an Knotenverzeichnis an Position lowest_free_node an;
   der erste Knoten bekommt die Nummer 0.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Knoten *node: Zeiger auf Knotendatenstruktur, die angehaengt
                   werden soll.
                                                                          */
/* Ergebnis:
   Knotennummer des neuen Knoten
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long AddNode ( Knoten *node )
{
  static long pos;
  pos = lowest_free_node;
  SetPtrArrayElement(nodelist, lowest_free_node, (void *) node);
  lowest_free_node = GetLowestFree(nodelist, (lowest_free_node+1));
  NodeListLength++;
  return pos;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: PlaceNode
                                                                          */
/* Aufgabe:
   Haengt Knoten an Knotenverzeichnis an Position pos an.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E Knoten *node: Zeiger auf Knotendatenstruktur, die angehaengt
                   werden soll.
   E long pos: Knotennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void PlaceNode ( Knoten *node, long pos )
{
#ifdef ERROR_CONTROL
    if (GetNode(pos)!=NULL) {
        DisplayErrorMsg("PlaceNode: Knoten existiert bereits !!!");
        abort();
    }
#endif
  SetPtrArrayElement(nodelist, pos, (void *) node);
  lowest_free_node = GetLowestFree(nodelist, lowest_free_node);
  NodeListLength++;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNode
                                                                          */
/* Aufgabe:
   Liest Knotendaten des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer, deren Daten gelesen werden sollen
                                                                          */
/* Ergebnis:
   Zeiger auf gesuchte Knotendatenstruktur, wenn nicht gefunden: NULL
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
Knoten *GetNode ( long number )
{
  return (Knoten *) GetPtrArrayElement(nodelist, number);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DeleteNode
                                                                          */
/* Aufgabe:
   Loescht Knoten aus Knotenverzeichnis an Position number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens, der geloescht werden soll.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1994     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
                                                                          */
/**************************************************************************/
void DeleteNode ( long number )
{
  if (nodelist->elements[number]!=NULL) {
      ((Knoten *) nodelist->elements[number])->newnodes = (long *)Free(((Knoten *) nodelist->elements[number])->newnodes);
      ((Knoten *) nodelist->elements[number])->elems1d = (long *)Free(((Knoten *) nodelist->elements[number])->elems1d);
      ((Knoten *) nodelist->elements[number])->elems2d = (long *)Free(((Knoten *) nodelist->elements[number])->elems2d);
      ((Knoten *) nodelist->elements[number])->elems3d = (long *)Free(((Knoten *) nodelist->elements[number])->elems3d);
      ((Knoten *) nodelist->elements[number])->edges = (long *)Free(((Knoten *) nodelist->elements[number])->edges);
      ((Knoten *) nodelist->elements[number])->plains = (long *)Free(((Knoten *) nodelist->elements[number])->plains);
      ((Knoten *) nodelist->elements[number])->nval = (double *)Free(((Knoten *) nodelist->elements[number])->nval);
/*OK*/
if(!(GetRFControlModel()==-1)&&!(GetRFControlModel()==9999)) {
      ((Knoten *) nodelist->elements[number])->nval_intern = DestroyInternNodeData(((Knoten *) nodelist->elements[number])->nval_intern);
}
      nodelist->elements[number] = Free((Knoten *) nodelist->elements[number]);
      NodeListLength--;
      if (lowest_free_node > number)
          lowest_free_node = number;
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: NODDeleteNodeGeometric
                                                                          */
/* Aufgabe:
   Delete geometric and topologic data of node number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens, der geloescht werden soll.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/2002   OK   Implementation
                                                                          */
/**************************************************************************/
void NODDeleteNodeGeometric ( long number )
{
  if (nodelist->elements[number]!=NULL) {
      ((Knoten *) nodelist->elements[number])->newnodes = (long *)Free(((Knoten *) nodelist->elements[number])->newnodes);
      ((Knoten *) nodelist->elements[number])->elems1d = (long *)Free(((Knoten *) nodelist->elements[number])->elems1d);
      ((Knoten *) nodelist->elements[number])->elems2d = (long *)Free(((Knoten *) nodelist->elements[number])->elems2d);
      ((Knoten *) nodelist->elements[number])->elems3d = (long *)Free(((Knoten *) nodelist->elements[number])->elems3d);
      ((Knoten *) nodelist->elements[number])->edges = (long *)Free(((Knoten *) nodelist->elements[number])->edges);
      ((Knoten *) nodelist->elements[number])->plains = (long *)Free(((Knoten *) nodelist->elements[number])->plains);
/*
      ((Knoten *) nodelist->elements[number])->nval = (double *)Free(((Knoten *) nodelist->elements[number])->nval);
      ((Knoten *) nodelist->elements[number])->nval_intern = DestroyInternNodeData(((Knoten *) nodelist->elements[number])->nval_intern);
*/
      nodelist->elements[number] = Free((Knoten *) nodelist->elements[number]);
      NodeListLength--;
      if (lowest_free_node > number)
          lowest_free_node = number;
  }
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ShortenNodeList
                                                                          */
/* Aufgabe:
   Entfernt alle Knoten ab number (einschl.) aus dem Knotenverzeichnis,
   bzw. laesst nur number Knoten uebrig.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer, ab der geloescht werden soll.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void ShortenNodeList ( long number )
{
  static long i;
  static long l;
  l = PtrArrayLength(nodelist);
  for (i = number; i < l; i++) DeleteNode(i);
  ShortenPtrArray(nodelist, number);
  lowest_free_node = GetLowestFree(nodelist,0);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CompressNodeList
                                                                          */
/* Aufgabe:
   Komprimiert Knotenverzeichnis (entfernt Nullzeiger)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void CompressNodeList (void)
{
  NodeListLength = CompressPtrArray(nodelist);
  lowest_free_node = NodeListLength;
}


/**************************************************************************
 ROCKFLOW - Funktion: NodeListSize
                                                                          
 Aufgabe:
   Liefert die hoechste Nummer aller Knoten + 1 (Anzahl der Eintraege im
   Pointer-Feld)
                                                                          
 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          
 Ergebnis:
   s.o.
                                                                          
 Programmaenderungen:
   02/1994    MSR     Erste Version
   02/2000    RK      Zweidim. 9-Knoten-Elemente
   03/2000    AH      Variable listsize initialisieren. Sicheres Handling
   09/2004    WW      Generalization                                                       
***************************************************************************/
long NodeListSize (void)
{
   if (!nodelist) return 0; //TK
   long listsize=PtrArrayLength(nodelist);

   
   if (HighOrder == 0) listsize = PtrArrayLength(nodelist); //  

   if (HighOrder==1)  listsize = NodeListLength_L; // Linear Elemente 
   if (HighOrder==2)  listsize = NodeListLength_H; // High order Elemente  
     
 return listsize;
}

/*{
  return PtrArrayLength(nodelist);
} */



/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeStartNumber
                                                                          */
/* Aufgabe:
   Setzt Knotennummer im Startnetz des Knotens number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E long node_start_number: Knotennummer im Startnetz
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetNodeStartNumber ( long number, long node_start_number )
{
  ((Knoten *) nodelist->elements[number])->node_start_number = node_start_number;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeStartNumber
                                                                          */
/* Aufgabe:
   Liefert Knotennummer im Startnetz des Knotens number.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long GetNodeStartNumber ( long number )
{
  return ((Knoten *) nodelist->elements[number])->node_start_number;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeSource
                                                                          */
/* Aufgabe:
   Liefert Quelle des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   08/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
double GetNodeSource ( long number )
{
  return ((Knoten *) nodelist->elements[number])->source;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeSource
                                                                          */
/* Aufgabe:
   Setzt Quelle des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E double source: Quelle
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void SetNodeSource ( long number, double source )
{
  ((Knoten *) nodelist->elements[number])->source = source;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeVal
                                                                          */
/* Aufgabe:
   Liefert Knotenwert des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E int idx: Index des Knotenwertes
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   11/2003 OK PCS Variante
                                                                          */
/**************************************************************************/
#ifdef PCS_NOD
//double NODGetPCSValue(long number,int idx,double val)
double GetNodeVal ( long number, int idx )
{
  int process = PCSGetProcessNumber();
  return ((Knoten *) nodelist->elements[number])->values[process][idx];
}
#else
double GetNodeVal ( long number, int idx )
{
  return ((Knoten *) nodelist->elements[number])->nval[idx];
}
#endif


/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeVal
                                                                          */
/* Aufgabe:
   Setzt Knotenwert des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E int idx: Index des Knotenwertes
   E double val: Wert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   11/2003 OK PCS Variante
                                                                          */
/**************************************************************************/
#ifdef PCS_NOD
//void NODSetPCSValue(long number,int idx,double val)
void SetNodeVal ( long number, int idx, double val )
{
  int process = PCSGetProcessNumber();
  ((Knoten *) nodelist->elements[number])->values[process][idx] = val;
}
#else
void SetNodeVal ( long number, int idx, double val )
{
  ((Knoten *) nodelist->elements[number])->nval[idx] = val;
}
#endif

/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeData
                                                                          */
/* Aufgabe:
   Setzt die Internen Knotendaten des Knotens number.
   Wenn schon ein Datensatz existierte, wird er zuvor geloescht.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   X void *data: Zeiger auf die Daten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void SetNodeData ( long number, void *data )
{
  ((Knoten *)nodelist->elements[number])->nval_intern =
    DestroyInternNodeData(((Knoten *)nodelist->elements[number])->nval_intern);
  ((Knoten *)nodelist->elements[number])->nval_intern = data;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeData
                                                                          */
/* Aufgabe:
   Liefert Zeiger auf die internen Knotendaten des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
void *GetNodeData ( long number )
{
  return ((Knoten *)nodelist->elements[number])->nval_intern;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeIndex
                                                                          */
/* Aufgabe:
   Liefert Index des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long GetNodeIndex ( long number )
{
  return ((Knoten *) nodelist->elements[number])->index;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeIndex
                                                                          */
/* Aufgabe:
   Setzt Index des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E long index: Index
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void SetNodeIndex ( long number, long index )
{
  ((Knoten *) nodelist->elements[number])->index = index;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeState
                                                                          */
/* Aufgabe:
   Liefert Status des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E int idx: Index des Status (0 oder 1)
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption                                                                          */

/**************************************************************************/
int GetNodeState ( long number )
{
 return ((Knoten *) nodelist->elements[number])->status;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeStatus
                                                                          */
/* Aufgabe:
   Setzt Status des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E int idx: Index des Status (0 oder 1)
   E int status: Status
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   11/1999     RK         Kapselung Gitteradaption
                                                                          */
/**************************************************************************/
void SetNodeState ( long number, int status)
{
  ((Knoten *) nodelist->elements[number])->status = status;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CopyNodeVals
                                                                          */
/* Aufgabe:
   Uebertraegt Knotenwerte von einem Knotenindex zu einem Anderen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int idx1: Index, woher die Daten kommen
   E int idx2: Index, wohin die Daten gespeichert werden sollen
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
  5/1999 C.Thorenz Erste Version

                                                                          */
/**************************************************************************/
void CopyNodeVals ( int idx1, int idx2)
{
  register long i;
  for (i=0;i<NodeListLength;i++)
      SetNodeVal(NodeNumber[i],idx2,GetNodeVal(NodeNumber[i],idx1));
}


/**************************************************************************/
/* ROCKFLOW - Funktion: TransferNodeVals
                                                                          */
/* Aufgabe:
   Uebertraegt Knotenwerte aus Vektor in Datenstruktur.
   Das Indizierungsfeld NodeNumber muss existieren und der
   Vektorindizierung entsprechen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *ergebnis: Vektor der Laenge NodeListLength mit
                       Knotenwerten, ueber den Index abgespeichert
   E int idx: Index, wohin die Ergebnisse gespeichert werden sollen
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void TransferNodeVals ( double *ergebnis, int idx )
{
  static long i;
  for (i=0;i<NodeListLength;i++)
      SetNodeVal(NodeNumber[i],idx,ergebnis[i]);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: TransferNodeValsVec
                                                                          */
/* Aufgabe:
   Kopieren von Vektorwerten in Knotendatenstruktur.
   Knotendatenfelder muessen Laenge (shift+1)*NodeListlength besitzen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *ergebnis: Vektor der Laenge NodeListLength mit
                       Knotenwerten, ueber den Index abgespeichert
   E int idx: Index, wohin die Ergebnisse gespeichert werden sollen
   E int shift: Position fuer Vektorkomponente
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/2000   OK   aus TransferNodeVals abgeleitet
                                                                          */
/**************************************************************************/
void TransferNodeValsVec(double *ergebnis,int idx, int shift)
{
  static long i;
  for (i=0;i<NodeListLength;i++)
      SetNodeVal(NodeNumber[i],idx,ergebnis[i+NodeListLength*shift]);
}


/********************************************************************************/
/* ROCKFLOW - Funktion: TransferNodeValsDim    
                                                                                */
/* Aufgabe:
   Kopieren von Vektorwerten in Knotendatenstruktur.
   Knotendatenfelder muessen Laenge (shift+1)*NodeListlength besitzen
                                                                                */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *ergebnis: Vektor der Laenge number_of_nodes mit
                       Knotenwerten, ueber den Index abgespeichert
   E int idx:              Index, wohin die Ergebnisse gespeichert werden sollen
   E long shift:           Position fuer Vektorkomponente
   E long number_of_nodes: Anzahl der Knotenwerte der Vektorkomponente
                                                                                */
/* Ergebnis:
   - void -
                                                                                */
/* Programmaenderungen:
   07/2001   MK   AddSolNodeValsVec ->  TransferNodeValsDim
                                                                                */
/********************************************************************************/
void TransferNodeValsDim(double *ergebnis,int idx, long shift, long number_of_nodes)
{
  static long i;
idx=idx;

  for (i=0;i<number_of_nodes;i++)
      SetNodeVal(NodeNumber[i],idx,ergebnis[i+shift]);
}



/********************************************************************************/
/* ROCKFLOW - Funktion: AddNodeValsDim    
                                                                                */
/* Aufgabe:
   Addieren von Vektorwerten in Knotendatenstruktur.
   Knotendatenfelder muessen Laenge (shift+1)*NodeListlength besitzen
                                                                                */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *ergebnis: Vektor der Laenge number_of_nodes mit
                       Knotenwerten, ueber den Index abgespeichert
   E int idx:              Index, wohin die Ergebnisse gespeichert werden sollen
   E long shift:           Position fuer Vektorkomponente
   E long number_of_nodes: Anzahl der Knotenwerte der Vektorkomponente
                                                                                */
/* Ergebnis:
   - void -
                                                                                */
/* Programmaenderungen:
   07/2001   MK   AddNodeValsVec ->  AddNodeValsDim
                                                                                */
/********************************************************************************/
void AddNodeValsDim(double *ergebnis,int idx, long shift, long number_of_nodes)
{
  static long i;
idx=idx;

  for (i=0;i<number_of_nodes;i++)
      SetNodeVal(NodeNumber[i],idx,GetNodeVal(NodeNumber[i],idx)+ergebnis[i+shift]);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllNodeVals
                                                                          */
/* Aufgabe:
   Initialisiert alle Knotenwerte
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int idx: Index der Knotenwerte
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void InitAllNodeVals ( int idx )
{
  static long i;
  static long l;
  l = PtrArrayLength(nodelist);
  for (i = 0l; i < l; i++) {
      if (nodelist->elements[i]!=NULL)
          (((Knoten *) nodelist->elements[i])->nval)[idx] = nval_data[idx].vorgabe;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: IncAllNodeVals
                                                                          */
/* Aufgabe:
   Inkrementiert nval[idx0] um nval[idx1] und initialisiert nval[idx1].
   NUR FUER ADAPTION !!!!!!!!!!
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int idx0, idx1: s.o.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/1995     MSR        Erste Version
                                                                          */
/**************************************************************************/
void IncAllNodeVals ( int idx0, int idx1, double *zwischenspeicher )
{
  static long i, l;
  l = PtrArrayLength(nodelist);
  for (i = 0l; i < l; i++) {
     if (nodelist->elements[i]!=NULL) {
        (((Knoten *) nodelist->elements[i])->nval)[idx0] += zwischenspeicher[i];
        (((Knoten *) nodelist->elements[i])->nval)[idx1] += zwischenspeicher[2*i];
     }
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ConstructStatusEntries
                                                                          */
/* Aufgabe:
   Traegt Randbedingungen bei Knoten ein und ermittelt Innenknoten bzw.
   Randknoten; das Kantenverzeichnis (Flaechenverzeichnis) muss bereits
   existieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   06/1995     MSR        Erste Version
   31.10.1997  R.Kaiser   Je Kante beliebige Anzahl an 2D-Nachbarelementen
                          (fuer Kluftnetzwerke)
   1.9.1998    C.Thorenz  Neue Datenstrukturen fuer Randbedingungen
   31.03.1999  AH         Randbedingungen entfernt
   08.11.1999  RK         Kapselung Gitteradaption
                                                                          */
/**************************************************************************/
void ConstructStatusEntries ( void )
{
  long *elem;
  int anz;
  int i, j;  /* Laufvariable */
  Kante *kante;  /* Zeiger auf Kantenstruktur */
  Flaeche *flaeche;  /* Zeiger auf Flaechenstruktur */

  /* Randknoten ermitteln */
  if (max_dim==2) {  /* 3D-Netz; alle Knoten auf Flaechen */
    for (i=0l;i<PlainListLength;i++) {  /* Schleife ueber alle Kanten */
      flaeche = GetPlain(i);
      if ( ((flaeche->nachbarn_3D[0]>=0l) && (flaeche->nachbarn_3D[1]<0l)) ||
           ((flaeche->nachbarn_3D[0]<0l) && (flaeche->nachbarn_3D[1]>=0l)) ) {
        /* Nur ein 2D-Nachbar an Flaeche ==> Randflaeche
           ==> Knoten sind Randknoten */
        for (j=0;j<4;j++)
          SetNodeState(flaeche->knoten[j],-3);
       }
     }
   }
   else {  /* 1D- oder 2D-Netz; alle Knoten auf Kanten */
     for (i=0l;i<EdgeListLength;i++) {  /* Schleife ueber alle Kanten */
       kante = GetEdge(i);
       elem = GetEdge2DElems(i,&anz);
       if (anz == 1) {
         /* Nur ein 2D-Nachbar an Kante ==> Randkante
            ==> Knoten sind Randknoten */
         SetNodeState(kante->knoten[0],-3);
         SetNodeState(kante->knoten[1],-3);
       }
     }
   }

}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeElements
                                                                          */
/* Aufgabe:
   Liefert zugehoerige Elemente und deren Anzahl.
   Das Ergebnisfeld muss von aufrufender Funktion wieder
   freigegeben werden !
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Elemente
                                                                          */
/* Ergebnis:
   Zeiger auf Feld mit zugehoerigen Elementen.
   Feld muss von aufrufender Funktion freigegeben werden.
                                                                          */
/* Programmaenderungen:
   6/2002  C.Thorenz  Erste Version
                                                                          */
/**************************************************************************/
long* GetNodeElements(long number, int *anzahl )
{
  long *elemente=NULL, *elems;
  int anz, i;

  *anzahl=0;

  elems = GetNode1DElems(number, &anz);
  if(anz) {
    *anzahl += anz;
    elemente = (long *) Realloc(elemente, *anzahl*sizeof(long));
    for(i=0; i<anz; i++) elemente[*anzahl-anz+i] = elems[i];
  }

  elems = GetNode2DElems(number, &anz);
  if(anz) {
    *anzahl += anz;
    elemente = (long *) Realloc(elemente, *anzahl*sizeof(long));
    for(i=0; i<anz; i++) elemente[*anzahl-anz+i] = elems[i];
  }

  elems = GetNode3DElems(number, &anz);
  if(anz) {
    *anzahl += anz;
    elemente = (long *) Realloc(elemente, *anzahl*sizeof(long));
    for(i=0; i<anz; i++) elemente[*anzahl-anz+i] = elems[i];
  }
  
  return elemente;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNode1DElems
                                                                          */
/* Aufgabe:
   Liefert zugehoerige 1D-Elemente und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Elemente
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Elemente
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long* GetNode1DElems ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz1d;
  return ((Knoten *) nodelist->elements[number])->elems1d;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign1DElement
                                                                          */
/* Aufgabe:
   Traegt 1D-Element ele bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: einzutragende Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign1DElement ( long number, long ele )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz1d;
  ((Knoten *) nodelist->elements[number])->elems1d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems1d,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->elems1d)[i-1] = ele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign1DElement
                                                                          */
/* Aufgabe:
   Traegt 1D-Element ele bei Knoten number aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: auszutragende Elementnummer
                                                                          */
/* Ergebnis:
   0 bei Fehler (ele nicht gefunden), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign1DElement ( long number, long ele )
{
  static int i, j, k;
  i = ((Knoten *) nodelist->elements[number])->anz1d;
  /* alle Eintraege ueberpruefen */
  j = 0;
  while (((((Knoten *) nodelist->elements[number])->elems1d)[j]!=ele) && (j<i))
      j++;
#ifdef ERROR_CONTROL
    if (j==i)  /* nicht gefunden */
        return 0;
#endif
  /* einer weniger */
  i = --((Knoten *) nodelist->elements[number])->anz1d;
  /* Umspeichern */
  for (k=j;k<i;k++)
      (((Knoten *) nodelist->elements[number])->elems1d)[k] =     \
        (((Knoten *) nodelist->elements[number])->elems1d)[k+1];
  /* Speicher aendern */
  ((Knoten *) nodelist->elements[number])->elems1d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems1d,(i*sizeof(long)));
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign1DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 1D-Element ele bei den Knoten nodes ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: einzutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign1DElementNodes ( long ele, long *nodes, int anzahl )
{
  static int i;
  for (i=0;i<anzahl;i++)
      Assign1DElement(nodes[i],ele);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign1DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 1D-Element ele bei den Knoten nodes aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   0 bei Fehler (Knoten waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign1DElementNodes ( long ele, long *nodes, int anzahl )
{
  static int i;
  for (i=0;i<anzahl;i++) {
#ifdef ERROR_CONTROL
        if (!ReAssign1DElement(nodes[i],ele)) {
            DisplayErrorMsg("Fehler bei ReAssign1DElementNodes !!!");
            return 0;
        }
#else
        ReAssign1DElement(nodes[i],ele);
#endif
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNode2DElems
                                                                          */
/* Aufgabe:
   Liefert zugehoerige 2D-Elemente und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Elemente
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Elemente
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long* GetNode2DElems ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz2d;
  return ((Knoten *) nodelist->elements[number])->elems2d;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign2DElement
                                                                          */
/* Aufgabe:
   Traegt 2D-Element ele bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: einzutragende Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign2DElement ( long number, long ele )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz2d;
  ((Knoten *) nodelist->elements[number])->elems2d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems2d,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->elems2d)[i-1] = ele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign2DElement
                                                                          */
/* Aufgabe:
   Traegt 2D-Element ele bei Knoten number aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: auszutragende Elementnummer
                                                                          */
/* Ergebnis:
   0 bei Fehler (ele nicht gefunden), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign2DElement ( long number, long ele )
{
  static int i, j, k;
  i = ((Knoten *) nodelist->elements[number])->anz2d;
  /* alle Eintraege ueberpruefen */
  j = 0;
  while (((((Knoten *) nodelist->elements[number])->elems2d)[j]!=ele) && (j<i))
      j++;
#ifdef ERROR_CONTROL
    if (j==i)  /* nicht gefunden */
        return 0;
#endif
  /* einer weniger */
  i = --((Knoten *) nodelist->elements[number])->anz2d;
  /* Umspeichern */
  for (k=j;k<i;k++)
      (((Knoten *) nodelist->elements[number])->elems2d)[k] =     \
        (((Knoten *) nodelist->elements[number])->elems2d)[k+1];
  /* Speicher aendern */
  ((Knoten *) nodelist->elements[number])->elems2d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems2d,(i*sizeof(long)));
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign2DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 2D-Element ele bei den Knoten nodes ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: einzutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign2DElementNodes ( long ele, long *nodes, int anzahl )
{
  static int i;
  for (i=0;i<anzahl;i++)
      Assign2DElement(nodes[i],ele);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign2DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 2D-Element ele bei den Knoten nodes aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   0 bei Fehler (Knoten waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign2DElementNodes ( long ele, long *nodes, int anzahl )
{
  static int i;
  for (i=0;i<anzahl;i++) {
#ifdef ERROR_CONTROL
        if (!ReAssign2DElement(nodes[i],ele)) {
            DisplayErrorMsg("Fehler bei ReAssign2DElementNodes !!!");
            return 0;
        }
#else
        ReAssign2DElement(nodes[i],ele);
#endif
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNode3DElems
                                                                          */
/* Aufgabe:
   Liefert zugehoerige 3D-Elemente und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Elemente
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Elemente
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
long* GetNode3DElems ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz3d;
  return ((Knoten *) nodelist->elements[number])->elems3d;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign3DElement
                                                                          */
/* Aufgabe:
   Traegt 3D-Element ele bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: einzutragende Elementnummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign3DElement ( long number, long ele )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz3d;
  ((Knoten *) nodelist->elements[number])->elems3d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems3d,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->elems3d)[i-1] = ele;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign3DElement
                                                                          */
/* Aufgabe:
   Traegt 3D-Element ele bei Knoten number aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long ele: auszutragende Elementnummer
                                                                          */
/* Ergebnis:
   0 bei Fehler (ele nicht gefunden), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign3DElement ( long number, long ele )
{
  static int i, j, k;
  i = ((Knoten *) nodelist->elements[number])->anz3d;
  /* alle Eintraege ueberpruefen */
  j = 0;
  while ((j<i) && ((((Knoten *) nodelist->elements[number])->elems3d)[j]!=ele))
      j++;
#ifdef ERROR_CONTROL
    if (j==i)  /* nicht gefunden */
        return 0;
#endif
  /* einer weniger */
  i = --((Knoten *) nodelist->elements[number])->anz3d;
  /* Umspeichern */
  for (k=j;k<i;k++)
      (((Knoten *) nodelist->elements[number])->elems3d)[k] =     \
        (((Knoten *) nodelist->elements[number])->elems3d)[k+1];
  /* Speicher aendern */
  ((Knoten *) nodelist->elements[number])->elems3d =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->elems3d,(i*sizeof(long)));
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: Assign3DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 3D-Element ele bei den Knoten nodes ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: einzutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
void Assign3DElementNodes ( long ele, long *nodes, int anzahl )
{
  static int i;
  for (i=0;i<anzahl;i++)
      Assign3DElement(nodes[i],ele);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReAssign3DElementNodes
                                                                          */
/* Aufgabe:
   Traegt 3D-Element ele bei den Knoten nodes aus
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long ele: auszutragende Elementnummer
   E long *nodes: Zeiger auf zugehoerige Knotennummern
   E int anzahl: Anzahl der Knoten
                                                                          */
/* Ergebnis:
   0 bei Fehler (Knoten waren nicht eingetragen), sonst 1
                                                                          */
/* Programmaenderungen:
   12/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int ReAssign3DElementNodes ( long ele, long *nodes, int anzahl )
{
  int i;
  for (i=0;i<anzahl;i++) {
#ifdef ERROR_CONTROL
        if (!ReAssign3DElement(nodes[i],ele)) {
            DisplayErrorMsg("Fehler bei ReAssign3DElementNodes !!!");
            return 0;
        }
#else
        ReAssign3DElement(nodes[i],ele);
#endif
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNode1DElems
                                                                          */
/* Aufgabe:
   Loescht alle 1D-/2D-/3D-Element beim Knoten number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
                                                                          */
/* Ergebnis:
   - immer Eins -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
int DeleteNodeElems ( long number)
{
  if ( ((Knoten *) nodelist->elements[number])->elems1d ) {
    ((Knoten *) nodelist->elements[number])->elems1d=(long *)Free( ((Knoten *) nodelist->elements[number])->elems1d );
    ((Knoten *) nodelist->elements[number])->anz1d=0;
  }
  if ( ((Knoten *) nodelist->elements[number])->elems2d ) {
    ((Knoten *) nodelist->elements[number])->elems2d=(long *)Free( ((Knoten *) nodelist->elements[number])->elems2d );
    ((Knoten *) nodelist->elements[number])->anz2d=0;
  }
  if ( ((Knoten *) nodelist->elements[number])->elems3d ) {
    ((Knoten *) nodelist->elements[number])->elems3d=(long *)Free( ((Knoten *) nodelist->elements[number])->elems3d );
    ((Knoten *) nodelist->elements[number])->anz3d=0;
  }
  return 1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeX
                                                                          */
/* Aufgabe:
   Liefert x Koordinate des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   03/1997     AH         Modifizierte Version (rf)
                                                                          */
/**************************************************************************/
/**************************************************************************/
/* ROCKFLOW - Funktion: SetNodeX
                                                                          */
/* Aufgabe:
   Setzt x Koordinate des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E double value: Wert
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1997     AH         Modifizierte Version (rf)
                                                                          */
/**************************************************************************/
double GetNodeX ( long number )
{
  return ((Knoten *) nodelist->elements[number])->x;
}

void SetNodeX(long number,double value)
{
  ((Knoten *) nodelist->elements[number])->x = value;
}

double GetNodeY ( long number )
{
  return ((Knoten *) nodelist->elements[number])->y;
}

void SetNodeY(long number,double value)
{
  ((Knoten *) nodelist->elements[number])->y = value;
}

double GetNodeZ ( long number )
{
  return ((Knoten *) nodelist->elements[number])->z;
}

void SetNodeZ(long number,double value)
{
  ((Knoten *) nodelist->elements[number])->z = value;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: CreateModelNodeData
                                                                          */
/* Aufgabe:
   Speicher fuer Knoten-Ergebnis-Daten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
   11/2001   AH   Initialsierung der Daten erst in InitModelNodeData
   02/2003   OK   for dynamic data
   05/2005   WW   Initialization                                                                       */
/**************************************************************************/
void CreateModelNodeData ( long number )
{
  static Knoten *k;
  k = GetNode(number);
  if(!(k->nval)) 
    k->nval = (double *)Malloc(anz_nval*sizeof(double));
  else 
    k->nval = (double *)Realloc(k->nval,anz_nval*sizeof(double));

  // Initilization. 
  for(int i=anz_nval0; i<anz_nval; i++)
    k->nval[i] = 0.0; 
  
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyModelNodeData
                                                                          */
/* Aufgabe:
   Speicher fuer Knoten-Ergebnis-Daten freigeben.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   11/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void DestroyModelNodeData ( long number )
{
  Knoten *k=NULL;
  k = GetNode(number);
  if (k->nval) k->nval = (double *)Free(k->nval);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitModelNodeData
                                                                          */
/* Aufgabe:
   Speicher fuer Knoten-Ergebnis-Daten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   05/1999   OK   Implementierung
   10/2001   AH   CreateModelNodeData in InitModelNodeData umbenannt
                  Abfrage bei Speicher-Anfrage           
                  Hier wird kein Speicher geholt.   ah rfm           
                                                                          */
/**************************************************************************/
void InitModelNodeData ( long number )
{
  static int i;
  static Knoten *k;

  k = GetNode(number);
  if(k->nval) 
    for (i=0;i<anz_nval;i++)
      k->nval[i] = nval_data[i].vorgabe;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeNodes
                                                                          */
/* Aufgabe:
   Liefert die neu generierten Nachbarknoten von Knoten number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der neugenerierten Nachbarknoten
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen neugenerierten Nachbarknoten
                                                                          */
/* Programmaenderungen:
   01/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
long* GetNodeNodes ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz_new_nodes;
  return ((Knoten *) nodelist->elements[number])->newnodes;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AssignNodeNode
                                                                          */
/* Aufgabe:
   Traegt neu generierten Nachbarknoten bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long node: einzutragende Knotennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   01/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
void AssignNodeNode ( long number, long node )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz_new_nodes;
  ((Knoten *) nodelist->elements[number])->newnodes =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->newnodes,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->newnodes)[i-1] = node;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodePlains
                                                                          */
/* Aufgabe:
   Liefert zugehoerige Flaechen und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Flaechen
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Flaechen
                                                                          */
/* Programmaenderungen:
   08/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
long* GetNodePlains ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz_plains;
  return ((Knoten *) nodelist->elements[number])->plains;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AssignPlain
                                                                          */
/* Aufgabe:
   Traegt Flaeche node_plain bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long node_plain: einzutragende Flaechennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
void AssignPlain ( long number, long node_plain )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz_plains;
  ((Knoten *) nodelist->elements[number])->plains =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->plains,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->plains)[i-1] = node_plain;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeEdges
                                                                          */
/* Aufgabe:
   Liefert zugehoerige Kanten und deren Anzahl
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   R int *anzahl: Anzahl der zugehoerigen Kanten
                                                                          */
/* Ergebnis:
   Zeiger auf die zugehoerigen Kanten
                                                                          */
/* Programmaenderungen:
   08/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
long* GetNodeEdges ( long number, int *anzahl )
{
  *anzahl = ((Knoten *) nodelist->elements[number])->anz_edges;
  return ((Knoten *) nodelist->elements[number])->edges;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AssignEdge
                                                                          */
/* Aufgabe:
   Traegt Kante node_edge bei Knoten number ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Knotennummer
   E long node_edge: einzutragende Kantennummer
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/2000     RK        Erste Version
                                                                          */
/**************************************************************************/
void AssignEdge ( long number, long node_edge )
{
  static int i;
  i = ++((Knoten *) nodelist->elements[number])->anz_edges;
  ((Knoten *) nodelist->elements[number])->edges =              \
    (long *)Realloc(((Knoten *) nodelist->elements[number])->edges,(i*sizeof(long)));
  (((Knoten *) nodelist->elements[number])->edges)[i-1] = node_edge;
}

#else

/* Quelltext bis Vesion rf 3.2.31 gespeichert, OK 16.03.1999 */

#endif


/**************************************************************************/
/* ROCKFLOW - Funktion: NODSetFreeSurfaceFlag
                                                                          */
/* Aufgabe:
   Setzt Flag = 1 des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
   E int free_surface: 1 oder 2)   
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/2002     MB         Erste Version                                   */
/**************************************************************************/
void NODSetFreeSurfaceFlag ( long number, int flag )
{
  ((Knoten *) nodelist->elements[number])->free_surface = flag;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: NODGetFreeSurfaceFlag
                                                                          */
/* Aufgabe:
   Liefert den Flag free_surface des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/2002     MB         Erste Version                                   */
/**************************************************************************/
int NODGetFreeSurfaceFlag ( long number )
{
 return ((Knoten *) nodelist->elements[number])->free_surface;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: NODSetCornerNodeFlag
                                                                          */
/* Aufgabe:
   Setzt Flag = 1 des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens   
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/2002     MB         Erste Version                                   
                                                                          */
/**************************************************************************/
void NODSetCornerNodeFlag ( long number )
{
  ((Knoten *) nodelist->elements[number])->corner_node = 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: NODGetCornerNodeFlag
                                                                          */
/* Aufgabe:
   Liefert den Flag corner_node des Knotens number
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E long number: Nummer des Knotens
                                                                          */
/* Ergebnis:
   Liefert den Flag corner_node des Knotens number
                                                                          */
/* Programmaenderungen:
   09/2002     MB         Erste Version                                   
                                                                          */
/**************************************************************************/
int NODGetCornerNodeFlag ( long number )
{
 return ((Knoten *) nodelist->elements[number])->corner_node;
}

/**************************************************************************
ROCKFLOW - Funktion: NODListExists
Task: Test if list exists
Programing:
09/2003 OK Implementation
**************************************************************************/
int NODListExists(void)
{
  if(!nodelist)
    return 0;
  else
    return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: GetNodeNumberClose
                                                                          */
/* Aufgabe:
   Ermittelt den nahliegenden existierenden Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Knotennummer, -1 im unbekannten Fall -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long GetNodeNumberClose (double xcoor, double ycoor, double zcoor)
{
 long i;
 double dist,distmin;
 double x[3];
 double y[3];
 long number;

 x[0]=xcoor; x[1]=ycoor; x[2]=zcoor;

 number=-1;
 distmin=1.e+300;
 for (i=0;i<NodeListLength;i++) {
   if (GetNode(i)!=NULL) {
         y[0]=GetNodeX(i);
         y[1]=GetNodeY(i);
         y[2]=GetNodeZ(i);
   }
   else continue;

   dist=EuklVek3dDist(x,y);
   if (dist < distmin) {
     distmin=dist;
         number=i;
   }
 }

 return number;
}

long* NODNodesClose2PointXY(CGLPoint *m_point,long *no_nodes)
{
 long i;
 double dist,distmin;
 double x[3];
 double y[3];
 long *nodes = NULL;
 long counter = 0;

 x[0] = m_point->x; 
 x[1] = m_point->y; 
 x[2] = m_point->z; 

 distmin = m_point->mesh_density / 2.;

 for (i=0;i<NodeListLength;i++) {
   if (GetNode(i)!=NULL) {
     y[0]=GetNodeX(i);
     y[1]=GetNodeY(i);
     y[2]=GetNodeZ(i);
   }
   else continue;

   dist=EuklVek3dDist(x,y);
   if (dist < distmin) {
     counter++;
     nodes = (long*)Realloc(nodes,counter*sizeof(long));
     nodes[counter-1] = i;
   }
 }
 *no_nodes = counter;
 return nodes;
}


//////////////////////////////////////////////////////////////////////////77
///////////////////////////SB/CC 06/2004 NodeElement

/*
*****************************************************************************************
*****************************************************************************************
search for: //SB:rfo
*/
//SB:rfo
/* Constructor: */
FEMNodesElements::FEMNodesElements(void){

	number_of_node_variables=0;
	number_of_nodes=0;
	number_of_times=0;
	nodevalues=NULL;
	number_of_element_variables=0;
	number_of_elements=0;
	elementvalues=NULL;
}

/* Destructor - free storage allocated in FEMReadRFOFile() */
FEMNodesElements::~FEMNodesElements(void)
{

int i, j;

/* clear names */
nodevariablenames.clear();
elementvariablenames.clear();
values_at_times.clear();

/* free array of node values */
for(i=0;i<number_of_nodes;i++){
	for(j=0;j<number_of_node_variables;j++)
		nodevalues[i][j] = (double *) Free(nodevalues[i][j]);
	nodevalues[i] = (double **) Free(nodevalues[i]);
}
nodevalues = (double ***) Free(nodevalues);

/* free array of element values */
for(i=0;i<number_of_elements;i++){
	for(j=0;j<number_of_element_variables;j++)
		elementvalues[i][j] = (double *) Free(elementvalues[i][j]);
	elementvalues[i] = (double **) Free(elementvalues[i]);
}
elementvalues = (double ***) Free(elementvalues);

number_of_times = 0;



}

/********************************************************************************
GEOSYS Function 
Task:	Reads in RFO file, produced by the FEM code, and copies to data structures 
		for display in the FEMView

From the *.rfo file specified, the number of nodes, number of elements and the number of
timesteps in the file is read in. 
If the geometry is written to the rfo file, then the geometry is read in, else the geometry 
has to come from the GEOSYS structures.
For all nodes and elements the corresponding values of the variables are read in.


***************************************************************************************/

void FEMNodesElements::FEMReadRFOFile(string rfo_file_name)
{
	
	int i, j, k, geometryflag, d1, d2, d3;
	std::stringstream in;
	char zeile[MAX_ZEILE];
	string line;
	int art, bin, nr, geom, zeitschritt, rfversionnumber;
	double time;
	int timestep; // counter for nummber of output times 


	// File handling
    rfo_file_name = rfo_file_name.append(RF_OUTPUT_EXTENSION); ///OK
	ifstream rfo_file1 (rfo_file_name.data(),ios::in);
	if (!rfo_file1.good()) return;
	rfo_file1.seekg(0L,ios::beg); // rewind

	// go through file and count lines starting with #; NUmber = Number of timesteps;
	while (!rfo_file1.eof()) {
		rfo_file1.getline(zeile, MAX_ZEILE);
		if(zeile[0] == '#') number_of_times++;
	}
		
	rfo_file1.seekg(0,ios::beg); // rewind tut nicht daher zu und wieder auf machen
	rfo_file1.close();
	ifstream rfo_file (rfo_file_name.data(),ios::in);

	timestep = 0;
	// read header line
	rfo_file.getline(zeile,MAX_ZEILE);
	for (i = 0; i < 80; i++)
        if (zeile[i] == '#') zeile[i] = ' ';
	in.str((string ) zeile);
	in >> art >> bin >> nr >> geom >> time >> zeitschritt >> rfversionnumber;
	in.clear();
	if(art != 0) DisplayErrorMsg("Error: wrong file type ");
	if(bin != 0) DisplayErrorMsg("Error: only ascii files ");
	//store timesteps
	values_at_times.push_back(time);

	
	// read steering line
	rfo_file.getline(zeile, MAX_ZEILE);
	in.str((string ) zeile);
	in >> geometryflag >> d2 >> d3;
	in.clear();

	if(geometryflag == 0){ // read in geometry
		number_of_nodes = d2;
		number_of_elements = d3;

		//read node geometry
		for(i=0;i<number_of_nodes;i++)
			rfo_file.getline(zeile, MAX_ZEILE);

		//read element geometry
		// element geometry is given by edge nodes, therefore the middle point has to be calculated
		for(i=0;i<number_of_elements;i++)
			rfo_file.getline(zeile, MAX_ZEILE);

		// read next steering line
		rfo_file.getline(zeile, MAX_ZEILE);
		in.str((string ) zeile);
		in >> d1 >> d2 >> d3;
		in.clear();
		if(d1 == 1){
			number_of_node_variables = d2;
			number_of_element_variables = d3;
		}

	} //end if(d1==0
	
	// if d1 !=0 then this is the first timestep
	else if(geometryflag == 1){
		number_of_node_variables = d2;
		number_of_element_variables = d3;
        number_of_nodes = NodeListSize();
		number_of_elements = ElListSize();
	}

	// read steering line
	rfo_file.getline(zeile, MAX_ZEILE);
	in.str((string ) zeile);
	in >> d1;
	if(d1 != number_of_node_variables) DisplayMsgLn("Error in RFO file");
	for(i=0;i<number_of_node_variables;i++){
		in >> d1;
		if(d1 > 1) DisplayMsgLn("Error in RFO file, Field size has to be one ");
	}
	in.clear();

	// read node variable names
	for(i=0;i<number_of_node_variables;i++){
		rfo_file.getline(zeile, MAX_ZEILE);
		line = zeile;
		nodevariablenames.push_back(line);
	}
	
	// read node variable values
	// allocate storage for double ***nodevalues;
	nodevalues = (double ***) Malloc(number_of_nodes * sizeof(double **));
	for(i=0;i<number_of_nodes;i++)
		nodevalues[i] = (double **) Malloc(number_of_node_variables * sizeof(double *));
	for(i=0;i<number_of_nodes;i++)
		for(j=0;j<number_of_node_variables;j++)
			nodevalues[i][j] = (double *) Malloc(number_of_times * sizeof(double));
	for(i=0;i<number_of_nodes;i++)
		for(j=0;j<number_of_node_variables;j++)
			for(k=0;k<number_of_times;k++)
				nodevalues[i][j][k] = 0.0;
	//read in number_of_node_lines
	for(i=0;i<number_of_nodes;i++){
		rfo_file.getline(zeile, MAX_ZEILE);
		in.str((string ) zeile);
		in >> d1;	if(d1 != i) DisplayMsgLn("Error reading in node values ");
		for(j=0;j<number_of_node_variables;j++)
			in >> nodevalues[i][j][timestep];
		in.clear();
	};


	// read steering line for elements
	rfo_file.getline(zeile, MAX_ZEILE);
	in.str((string ) zeile);
	in >> d1;
	if(d1 != number_of_element_variables) DisplayMsgLn("Error in RFO file");
	for(i=0;i<number_of_element_variables;i++){
		in >> d1;
		if(d1 > 1) DisplayMsgLn("Error in RFO file, Field size has to be one ");
	}
	in.clear();

	// read element variable names
	for(i=0;i<number_of_element_variables;i++){
		rfo_file.getline(zeile, MAX_ZEILE);
		line = zeile;
		elementvariablenames.push_back(line);
	}

	// read element variable values
	// allocate storage for double ***elementvalues;
	elementvalues = (double ***) Malloc(number_of_elements * sizeof(double **));
	for(i=0;i<number_of_elements;i++)
		elementvalues[i] = (double **) Malloc(number_of_element_variables * sizeof(double *));
	for(i=0;i<number_of_elements;i++)
		for(j=0;j<number_of_element_variables;j++)
			elementvalues[i][j] = (double *) Malloc(number_of_times * sizeof(double));
	for(i=0;i<number_of_elements;i++)
		for(j=0;j<number_of_element_variables;j++)
			for(k=0;k<number_of_times;k++)
				elementvalues[i][j][k] = 0.0;
	//read in number_of_element_lines
	for(i=0;i<number_of_elements;i++){
		rfo_file.getline(zeile, MAX_ZEILE);
		in.str((string ) zeile);
		in >> d1;	if(d1 != i) DisplayMsgLn("Error reading in element values ");
		for(j=0;j<number_of_element_variables;j++)
			in >> elementvalues[i][j][timestep];
		in.clear();
	}

	timestep++;
	//finished with the first timestep
	
	// read in remaining timesteps
	while(timestep < number_of_times){
		// read header line for next timestep
		// read header line, strip # and find time of timestep
		rfo_file.getline(zeile,MAX_ZEILE);
		for (i = 0; i < 80; i++)
			if (zeile[i] == '#') zeile[i] = ' ';
		in.str((string ) zeile);
		in >> art >> bin >> nr >> geom >> time >> zeitschritt >> rfversionnumber;
		in.clear();
		//store timesteps
		values_at_times.push_back(time);

		// read steering line for nodes and elements
		rfo_file.getline(zeile, MAX_ZEILE);
		// read steering line for nodes
		rfo_file.getline(zeile, MAX_ZEILE);
		// read node variable names
		for(i=0;i<number_of_node_variables;i++)
			rfo_file.getline(zeile, MAX_ZEILE);
		// read in node values for next timestep
		for(i=0;i<number_of_nodes;i++){
			rfo_file.getline(zeile, MAX_ZEILE);
			in.str((string ) zeile);
			in >> d1;	if(d1 != i) DisplayMsgLn("Error reading in node values ");
			for(j=0;j<number_of_node_variables;j++)
				in >> nodevalues[i][j][timestep];
			in.clear();
		};
		//
		// read in steering line for elements
		rfo_file.getline(zeile, MAX_ZEILE);
		// read in element variable names
		for(i=0;i<number_of_element_variables;i++)
			rfo_file.getline(zeile, MAX_ZEILE);
		//read in number_of_element_lines
		for(i=0;i<number_of_elements;i++){
			rfo_file.getline(zeile, MAX_ZEILE);
			in.str((string ) zeile);
			in >> d1;	if(d1 != i) DisplayMsgLn("Error reading in element values ");
			for(j=0;j<number_of_element_variables;j++)
				in >> elementvalues[i][j][timestep];
			in.clear();
		};
		timestep++;

	} //end while

	//test end of file
	if(rfo_file.getline(zeile, MAX_ZEILE))
		DisplayMsgLn("Error: End of *.rfo file not reached");
}

/**************************************************************************
FEMLib-Method: 
Task: master write function
Programing:
07/2004 OK Implementation
last modification:
**************************************************************************/
void WriteFDMNodes(string base_file_name)
{
  //========================================================================
  // File handling
  string fdm_file_name = base_file_name + ".fdm";
  fstream fdm_file (fdm_file_name.data(),ios::trunc|ios::out);
  fdm_file.setf(ios::scientific,ios::floatfield);
  fdm_file.precision(12);
  if (!fdm_file.good()) return;
  fdm_file.seekg(0L,ios::beg);
  //========================================================================
  fdm_file << "GeoSys-ST: Source Terms ------------------------------------------------\n";
  //========================================================================
  // FDM vector
  long i;
  long no_fdm_nodes = (long)FDM_node_vector.size();
  CFDMNode *m_fdm;
  int j;
  for(i=0;i<no_fdm_nodes;i++){
    m_fdm = FDM_node_vector[i];
    fdm_file << i << ": " \
             << m_fdm->node << endl;
    for(j=0;j<m_fdm->no_neighbors;j++){
      fdm_file << " " << m_fdm->nod_neighbors[j];
    }
    fdm_file << endl;
    for(j=0;j<m_fdm->no_neighbors;j++){
      fdm_file << " " << m_fdm->ele_neighbors[j];
    }
    fdm_file << endl;
  }
  fdm_file << "#STOP";
  fdm_file.close();
}


CFDMNode::CFDMNode()
{
  no_neighbors = 0;
}



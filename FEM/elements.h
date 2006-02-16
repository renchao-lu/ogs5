/**************************************************************************
 ROCKFLOW - Modul: elements.h

 Aufgabe:
   Datenstruktur des Elementverzeichnisses und zugehoerige Funktionen

   05/2003 OK void *element_matrices

   letzte Aenderung:  18.05.2003 OK


**************************************************************************/

#ifndef elements_INC

#define elements_INC
  /* Schutz gegen mehrfaches Einfuegen */

// C++
#include <string>
using namespace std;

/* Andere oeffentlich benutzte Module */
#include "prototyp.h"


/* Deklarationen */

extern long anz_1D; /* Gesamtanzahlen der 1D-/2D-/3D-Elemente */
extern long anz_2D;
extern long anz_3D;
extern int max_dim;  /* Maximale Elementdimension -1 */
  /* Anzahlen der momentan aktiven Elemente, sowie der aktiven 1D-/2D-/3D-Elemente */
extern long anz_active_elements;
extern long anz_active_1D;
extern long anz_active_2D;
extern long anz_active_3D;
extern long start_new_edges;  /* Startindex der neuen Kanten */
extern long start_new_plains;  /* Startindex der neuen Flaechen */
/* Deklarationen fuer Zaehler */
extern long start_new_elems;  /* Startindex der nicht-Ausgangselemente */
/* Feld in dem die Nummern der aktiven Elemente stehen */
extern long *ActiveElements;

extern long HighOrder; //WW

// Used to extropolate the Gauss values to node values. WW
// Allocates and released in class CRFProcessDeformation (pcs_dm.h/cpp)
extern int *EleNeighborsOfNode;
// CFiniteElementVec object;


typedef struct {                /* Element-Datenstruktur */
    long element_start_number;  /* Elementnummer im Startnetz */
 /* Geometry and topology */
    int elementtyp;             /* 1D-, 2D- oder 3D-Element ? */
    double volume;
    long *elementknoten;        /* Eckknoten des Elements */
    long *kanten;               /* Kanten bzw. Flaechen des Elements */
    long *Neighbors;            // Neighbors of an element //WW
    int anz_flaechen;           /* Anzahl der an ein 1D-Element grenzenden
                                   Flaechen; nur bei 3D-Netzen */
    long *ele_edge_nodes2d;     /* Kantenknoten des Elements (z.B. hängende Knoten);*/
    int anz_ele_edge_nodes2d;   /* Anzahl der Kantenknoten des Elements (default: 4) */                       
    double detjac;              /* OK: determinant of Jacobian */
    double *invjac;             /* OK: inverse Jacobian matrix */
	double *element_midpoint;	/* SB: Schwerpunkt des Elements */
	double *hetfields;			/* SB: heterogeneous field values */

 /* Grid adaptation */
    int aktiv;                  /* Ist das Element aktiv oder nicht ? */
    /* Die Information ist redundant,
       sie kann auch ueber den Kinder-Zeiger erhalten werden */
    int verfeinern;             /* Soll das Element verfeinert oder
                                   vergroebert werden ?
                                   0: keine Veraenderung
                                   1: verfeinern
                                  -1: vergroebern */
    int level;                  /* Verfeinerungs-Level des Elements */
    double art_diff;            /* kuenstliche Diffusion, Johnson-Indikator */
    int *global_verf;
    int count_verf;
    int count_vergr;
    int verf;                   /* Merker, ob im It.Schritt schon verf. */
    int dirty_bit;              /* Merker, ob das Element gerade erzeugt wurde oder
                                   ansonsten "dreckig" ist, ggf. werden die Elementmatritzen neu aufgebaut */

    long vorgaenger;            /* Vorgaengerelement */
    long *kinder;               /* Nachfolgeelemente des Elements */

 /* Numerical properties */

 /* Physical properties */
    long group_number;          /* Elementgruppennummer im Startnetz */

 /* Element data */
    void *eval_intern;          /* Element matrices  */
    double *eval;               /* modellabhaengige Elementwerte */
    double **gauss_velo;         /* Zeiger auf die Geschwindigkeiten in den Gausspunkten fuer jede Phase */
    void *element_matrices[17];  /* OK: Element matrices, todo PCS_NUMBER_PROCESSES  */

   bool doubled;

    /* Calculation of stability criteria for diffusive transport */
    /* CMCD 05 2004 */
      double Peclet_number;
      double Neumann_number;
      double Courant_number;
      double Advective_Vx;
      double Advective_Vy;
      double Advective_Vz;
      double Element_Char_Length;
	  double TimestepCourant;
} Element;


/* Groessenangaben zu abgespeicherten Feldern */
extern int ElNumberOfNodes[6];  /* Anzahl der Knoten eines Elements */
extern int ElNumberOfEdges[6];  /* Anzahl der Kanten eines Elements (1D und 2D) */
extern int ElNumberOfPlains[6]; /* Anzahl der Flaechen eines Elements (3D) */
extern int ElNumberOfChilds[6]; /* Anzahl der Kinder eines Elements */


/******************************************************************************/
/* Funktionsprototypen und externe Objekte zum Elementverzeichnis allgemein */
extern int ElCreateElementList(void);
  /* Erzeugt leeres Elementverzeichnis; das erste Element hat die Nummer 0 */
extern void ElDestroyElementList(void);
  /* Entfernt komplettes Elementverzeichnis aus dem Speicher */
extern void ElShortenElementList(long number);
  /* Entfernt alle Elemente ab number (einschl.) aus dem Elementverzeichnis */
extern void ElCompressElementList(void);
  /* Komprimiert Elementverzeichnis (entfernt Nullelemente) */
extern long ElListSize(void);
  /* Liefert die hoechste Nummer aller Elemente + 1 */
extern long ElCreateElement(int elementtyp, long vorgaenger, int level);
  /* Erzeugt leeres aktives Element der Dimension elementtyp und traegt es
     im Elementverzeichnis ein. Ergebnis ist die zugewiesene Elementnummer. */
extern void CreateElementPrototypes ( void );
  /* Erzeugt Elementprototyp mit der entsprechenden Materialgruppe  */
extern void CreateElementTopology ( int elementtyp, long vorgaenger, int level, long pos );
  /* Erzeugt leeres aktives Element der Dimension elementtyp und traegt es
     an Position pos im Elementverzeichnis ein. */
extern void ElDeleteElement(long number);
  /* Loescht Element mit der Nummer number */
extern Element *ElGetElement(long number);
  /* Liefert Zeiger auf das ganze Element; sollte moeglichst nicht benutzt
     werden */


extern long ElementListLength;
  /* ANZAHL der aktuell gespeicherten Elemente */

extern VoidXFuncVoid InitInternElementData;
extern VoidXFuncVoidX DestroyInternElementData;
extern VoidXFuncVoidX ELEDestroyElementMatrices;


/******************************************************************************/
/* Geometry and topology */
extern void ElSetElementStartNumber(long number, long element_start_number);    /* ah */
extern void ElSetElementNodes(long number, long *knoten);
extern void ElSetElementNeighbors(long number, long *Neighbor);
extern void ElSetElementEdges(long number, long *kanten);
extern void ElSetElementPlains(long number, long *flaechen, int anz);
extern void ElSetElementVolume(long number, double volume);
extern void ElSetEdgeNodes2D(long number, long *edge_nodes, int anz);

long ElGetElementStartNumber ( long number );
extern int ElementDimension(const long Index);
extern int ElGetElementType(long number);
extern long *ElGetElementNodes(long number);
extern long *ElGetElementEdges(long number);
extern long *ElGetElementPlains(long number);
extern int ElGetElementNodesNumber(long number);

///WW
extern int GetElementFaceNodes(const long Index, const int Face, 
                                     int *FacesNode, const int order=1);
extern int NumbersOfElementNode(const long Index); //Geometry
extern int NumbersOfElementNodeHQ(const long Index);
extern int NumbersOfElementFaces(const long Index);
extern int NumbersOfElementEdge(const long Index);
extern void GetElementEdge(const long EleIndex, const int Edge, int *EdgeNodes);
extern long *ElGetElementNeighbors(long number);
///

extern int ElGetElementEdgesNumber(long number);
extern int ElGetElementPlainsNumber(long number);
extern double ElGetElementVolume(long number);
extern long *ElGetEdgeNodes2D ( long number, int *anzahl );

extern void ConfigTopology(void);
extern void ComputeNeighborsOfElement(void);

/* Grid adaptation */
extern void ElSetElementActiveState(long number, int aktiv);
extern void ElSetElementRefineState(long number, int verfeinern);
extern void ElSetElementChilds(long number, long *kinder);
extern void ElSetArtDiff(long number, double wert);
extern void ElSetVerfCount ( long number , int wert );
extern void ElSetVergrCount ( long number , int wert );
extern void ElSetGlobalVerf ( long number, int verf_index, int val );
extern void ElSetVerf(long number, int wert);
extern void ElSetElementLevel(long number, int wert);
extern void ElSetElementDirtyBit(long number, int wert);
extern int ElGetElementActiveState(long number);
extern int ElGetElementRefineState(long number);
extern int ElGetElementLevel(long number);
extern long ElGetElementPred(long number);
extern long *ElGetElementChilds(long number);
extern double ElGetArtDiff(long number);
extern int ElGetVerf(long number);
extern int ElGetVerfCount ( long number );
extern int ElGetVergrCount ( long number );
extern int ElGetGlobalVerf ( long number, int verf_index );
extern int ElGetElementDirtyBit(long number);
extern int ElGetElementChildsNumber(long number);
extern void ElDeleteElementChilds(long number);

/* Numerical properties */

/* Physical properties */
extern void ElSetElementGroupNumber ( long number, long group_number );    /* ah */
long ElGetElementGroupNumber ( long number );
extern void ElDeleteElementGaussVeloPhase(long number,int phase);

/* Element matrices */
extern void ELECreateElementMatricesPointer(long number,int process);

/* Element data */
extern void ElSetElementVal(long number, int idx, double val);
extern void ElSetElementData(long number, void *data);
extern void ElSetVelocity(long number, double *gauss_velo);
extern double ElGetElementVal(long number, int idx);
extern void *ELEGetElementMatrices(long,int);
extern void *ELEGetElementMatricesPCS(long number,int process);
extern void *ElGetElementData(long number);
extern double *ElGetVelocity(long number);
extern double *ElGetVelocityNew ( long number, int phase);

extern void CreateModelElementData(long number);
extern void DestroyModelElementData(long number); /* ah inv */
extern void InitModelElementData(long number);    /* ah inv */
extern void ELECreateElementData(long number);

extern void Generate_Quadratic_Elements ();
extern void Init_Quadratic_Elements ();
extern void Init_Linear_Elements ();
extern long GetLowOrderNodeNumber(void);
extern long GetHighOrderNodeNumber(void);

/*--------------------------------------------------------------------------------*/


void ELECalcEleMidPoint(void);
void ELESetEleMidPoint(int i, double *middel_coord);
double ELEGetEleMidPoint(int i, int j);
void ELEPrintEleMidPoint(void);

/* SB het fields */
void ELESetHetFieldValues(int , int, double *);
double ELEGetHetFieldValue(int, int);


extern void ELEConfigJacobianMatrix(void);
extern int ELEListExists(void);
extern void ELEDestroyTopology (long number);
extern void ELECreateTopology ( int elementtyp, long vorgaenger, int level, long pos );
extern void ELECreateGPValues (long number);
extern void ELEDestroyPtrArray (void);
extern void ELEDeleteElementFromList(long element_start,long element_end);
extern double ELEGetValue(long index, string pcs_pv_name,int timelevel);

extern int CalculateMiddelPointElement(long index, double *coord);
extern void CalculateSimpleMiddelPointElement(long index, double *coord);//CMCD 09/2004 GeoSs4
extern void CalculateStabilityCriteria(long index, double D, double vg, double dt);//CMCD 09/2004 GeoSs4

#endif

/**************************************************************************/
/* ROCKFLOW - Modul: nodes.h
                                                                          */
/* Aufgabe:
   Datenstruktur des Knotenverzeichnisses und zugehoerige Funktionen
                                                                          */
/**************************************************************************/

#ifndef nodes_INC

#define nodes_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include "geo_pnt.h"
#include "prototyp.h"
//OK_IC #include "rfbc.h" /* Randbedingung TODO */

//06/2004 CC/SB

#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

typedef struct {        /* Knotenkoordinaten */
    double x;           /* Koordinaten */
    double y;
    double z;
    long node_start_number;     /* Knotennummer im Startnetz */
    double source;      /* Quelle */
    long index;         /* aktueller Index nach Umnummerierung, sonst -1 */
    long *elems1d;      /* zugehoerige 1D-Elemente */
    long *elems2d;      /* zugehoerige 2D-Elemente */
    long *elems3d;      /* zugehoerige 3D-Elemente */
    int anz1d;          /* Anzahl der zugehoerigen 1D-Elemente */
    int anz2d;          /* Anzahl der zugehoerigen 2D-Elemente */
    int anz3d;          /* Anzahl der zugehoerigen 3D-Elemente */
    long *plains;       /* Wird zum Aufbau des Flaechenverzeichnisses (Startnetz) gebraucht */
    int anz_plains;     /* Waehrend der Adaption werden diese Daten nicht aktualisiert !!!!  */
    long *edges;        /* Wird zum Aufbau des Kantenverzeichnisses (Startnetz) gebraucht */
    int anz_edges;      /* Waehrend der Adaption werden diese Daten nicht aktualisiert !!!!  */
    long *newnodes;     /* benachbarte neugenerierte Knoten fuer 9-Knoten-Elemente */
    int anz_new_nodes;

    int status;         /* -1: regulaerer Innenknoten
                           -2: irregulaerer Kantenknoten
                           -3: regulaerer Randknoten
                           -4: irregulaerer Flaechenknoten */
    /* MB 09.2002 */
    int corner_node;    /* corner_node flag = 1 if Eckknoten */
    int free_surface;   /* 1: bewegliche Knoten oben */
                        /* 2: bewegliche Knoten unten */
    double *nval;       /* node values */
    vector<double>time_values;//For analytical solution CMCD
    vector<double>function_values;//For analytical solution CMCD
#ifdef PCS_NOD
	double values[MAX_PCS_NUMBER][100];
#endif
    void *nval_intern;            /* interne modellabhaengige Knotendaten */
} Knoten;



/* Funktionsprototypen mit Knoten */
typedef void (*VoidFuncKXLX) ( Knoten *, long * );
typedef void (*VoidFuncKXKXKX) ( Knoten *, Knoten *, Knoten * );


extern int CreateNodeList ( void );
  /* Erzeugt leeres Knotenverzeichnis; der erste Knoten hat die Nummer 0 */
extern void DestroyNodeList ( void );
  /* Entfernt komplettes Knotenverzeichnis aus dem Speicher */
extern Knoten *NewNode ( void );
extern Knoten *CreateNodeGeometry ( void );
  /* Erzeugt neuen initialisierten Knoten */
extern long AddNode ( Knoten *node );
  /* Haengt Knoten an Knotenverzeichnis an; Ergebnis ist Knotennummer */
extern void PlaceNode ( Knoten *node, long pos );
  /* Haengt Knoten an Knotenverzeichnis an Position pos an */
extern Knoten *GetNode ( long number );
  /* Liest Knotendaten des Knotens number; nur fuer Koordinaten benutzen */
extern void DeleteNode ( long number );
  /* Loescht Knoten mit der Nummer number */
extern void NODDeleteNodeGeometric ( long number );
  /* Loescht geometrische Daten des Knotens mit der Nummer number */
extern void ShortenNodeList ( long number );
  /* Entfernt alle Knoten ab number (einschl.) aus dem Knotenverzeichnis */
extern void CompressNodeList (void);
  /* Komprimiert Knotenverzeichnis (entfernt Nullzeiger) */
extern long NodeListSize (void);
  /* Liefert die hoechste Knotennummer aller Knoten + 1 (bzw. 0 bei 0 Knoten) */

extern void SetNodeStartNumber(long number, long node_start_number);
  /* Setzt die Startnummer des Knotens number */
extern long GetNodeStartNumber ( long number );
  /* Liefert die Startnummer des Knotens number */
extern double GetNodeSource ( long number );
  /* Liefert Quelle des Knotens number */
extern void SetNodeSource ( long number, double source );
  /* Setzt Quelle des Knotens number */
extern double GetNodeVal ( long number, int idx );
  /* Liefert Knotenwert am Knoten number */
extern void SetNodeVal ( long number, int idx, double val );
  /* Setzt Knotenwert am Knoten number */
extern void SetNodeData ( long number, void *data );
  /* Setzt die internen Knotendaten des Knotens number */
extern void *GetNodeData ( long number );
  /* Liefert die internen Daten am Knoten number */
extern long GetNodeIndex ( long number );
  /* Liefert Index des Knotens number */
extern void SetNodeIndex ( long number, long index );
  /* Setzt Index des Knotens number */

extern int GetNodeState ( long number );
  /* Liefert Status des Knotens number */
extern void SetNodeState ( long number, int status );
  /* Setzt Status des Knotens number */

/* MB */ 
extern void NODSetCornerNodeFlag ( long number );
extern int NODGetCornerNodeFlag ( long number );
extern void NODSetFreeSurfaceFlag ( long number, int status );
extern int NODGetFreeSurfaceFlag ( long number );

extern void CopyNodeVals ( int idx1, int idx2);
    /* Uebertraegt Knotenwerte von einem Knotenindex zu einem anderen */
    /* Skalare Feldgroesse */
extern void TransferNodeVals ( double *ergebnis, int idx );
    /* Vektorielle Feldgroesse */
extern void TransferNodeValsVec(double *ergebnis,int idx, int shift);
extern void TransferNodeValsDim(double *ergebnis,int idx, long shift, long number_of_nodes);
extern void AddNodeValsDim(double *ergebnis,int idx, long shift, long number_of_nodes);
  /* Uebertraegt Knotenwerte aus Vektor in Datenstruktur */
extern void InitAllNodeVals ( int idx );
  /* Initialisiert alle entsprechenden Knotenwerte */
extern void IncAllNodeVals ( int idx0, int idx1, double *zwischenspeicher );
  /* Inkrementiert nval[idx0] und nval[idx1] um Werte aus zwischenspeicher */
extern void ConstructStatusEntries ( void );
  /* Traegt Randbedingungen und Status der Knoten ein */


//OK_IC extern void AddBoundaryCondition(long number, Randbedingung *neue_rb);
  /* Setzt eine Randbedingung an einem Knoten */
extern int GetBoundaryCondition(long number, int typ, double zeit, double *rb);
  /* Liefert (ggf.) Randbedingung an einem Knoten */
extern void DeleteBoundaryConditions(long number);
  /* Loescht alle Randbedingungen an einem Knoten */

extern long* GetNodeElements (long number, int *anzahl );
  /* Liefert alle zugehoerigen Elemente und deren Anzahl */
extern long* GetNode1DElems ( long number, int *anzahl );
  /* Liefert zugehoerige 1D-Elemente und deren Anzahl */
extern void Assign1DElement ( long number, long ele );
  /* Traegt 1D-Element ele bei Knoten number ein */
extern int ReAssign1DElement ( long number, long ele );
  /* Traegt 1D-Element ele bei Knoten number aus; 0 bei Fehler */
extern void Assign1DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 1D-Element ele bei den Knoten nodes ein */
extern int ReAssign1DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 1D-Element ele bei den Knoten nodes aus, 0 bei Fehler */
extern long* GetNode2DElems ( long number, int *anzahl );
  /* Liefert zugehoerige 2D-Elemente und deren Anzahl */
extern void Assign2DElement ( long number, long ele );
  /* Traegt 2D-Element ele bei Knoten number ein */
extern int ReAssign2DElement ( long number, long ele );
  /* Traegt 2D-Element ele bei Knoten number aus; 0 bei Fehler */
extern void Assign2DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 2D-Element ele bei den Knoten nodes ein */
extern int ReAssign2DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 2D-Element ele bei den Knoten nodes aus, 0 bei Fehler */
extern long* GetNode3DElems ( long number, int *anzahl );
  /* Liefert zugehoerige 3D-Elemente und deren Anzahl */
extern void Assign3DElement ( long number, long ele );
  /* Traegt 3D-Element ele bei Knoten number ein */
extern int ReAssign3DElement ( long number, long ele );
  /* Traegt 3D-Element ele bei Knoten number aus; 0 bei Fehler */
extern void Assign3DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 3D-Element ele bei den Knoten nodes ein */
extern int ReAssign3DElementNodes ( long ele, long *nodes, int anzahl );
  /* Traegt 3D-Element ele bei den Knoten nodes aus, 0 bei Fehler */
extern int DeleteNodeElems ( long number);   /* ah rfm */
  /* Loescht alle 1D-/2D-/3D-Element beim Knoten number */


extern long* GetNodeEdges ( long number, int *anzahl );
  /* Liefert zugehoerige Kanten und deren Anzahl */
extern void AssignEdge ( long number, long node_edge );
  /* Traegt Kante node_edge bei Knoten number ein */
extern long* GetNodePlains ( long number, int *anzahl );
  /* Liefert zugehoerige Flaechen und deren Anzahl */
extern void AssignPlain ( long number, long node_plain );
  /* Traegt Flaeche node_plain bei Knoten number ein */


/* Weitere externe Objekte */
extern long NodeListLength;
  /* ANZAHL der aktuell gespeicherten Knoten */
extern long *NodeNumber;
  /* Zeiger auf Feld, in dem unter index nachgesehen werden kann, welche
     reale Knotennummer sich dahinter verbirgt (nach Umnummerieren) */


/* Funktionszeiger */
extern LongXFuncLIX GetNodeXDElems[3];
  /* Zeigerfeld auf die Funktionen zum Holen der Elementknoten */
extern VoidXFuncVoid InitInternNodeData;
extern VoidXFuncVoidX DestroyInternNodeData;

/* Zugriffsfunktionen fuer Knotenkoordinaten */
extern double GetNodeX(long number);
extern void SetNodeX(long number,double value);
extern double GetNodeY(long number);
extern void SetNodeY(long number,double value);
extern double GetNodeZ(long number);
extern void SetNodeZ(long number,double value);

/*extern void SetNodeTimeHistory (long number,int max_size,double value);//CMCD
extern double GetNodeTimeHistory (long number,int time_step);//CMCD 
extern void SetNodeValueHistory (long number,int max_size,double value);//CMCD
extern double GetNodeValueHistory (long number,int time_step);//CMCD */


extern void CreateModelNodeData ( long number );
extern void DestroyModelNodeData ( long number );  /* ah inv */
extern void InitModelNodeData ( long number );     /* ah inv */


extern long* GetNodeNodes (long number, int *anzahl);
extern void AssignNodeNode (long number, long node);
extern long NodeListLength_L; //WW
extern long NodeListLength_H; //WW

extern int NODListExists(void);
/* Ermittelt den nahliegenden existierenden Knoten */
extern long GetNodeNumberClose (double xcoor, double ycoor, double zcoor);
extern long* NODNodesClose2PointXY(CGLPoint *m_point,long *no_nodes);

//////////////////////////////////////////////////////////////////////
//////NodeElements/////////////////////////////////////////////////
/////////////06/2004 CC/SB
//SB:
//OK: ToDo separation of NODe and ELEment data 
class FEMNodesElements{

	private:
	public:
	FEMNodesElements(void);
	~FEMNodesElements(void);

	//data structure for nodes
	int number_of_node_variables;
	int number_of_nodes;
	int number_of_times; //number of timesteps found in rfo file	
	vector <double> values_at_times;
	vector <string> nodevariablenames; //vector with the names of the variables with values at nodes
	double ***nodevalues;  //array of node values: number_of_nodes * number_of_node_variables * number_of_times
	
	//data structure for elements
	int number_of_element_variables;
	int number_of_elements;
	vector <string> elementvariablenames;  //vector with the names of the variables with values at element midpoints
	double ***elementvalues;  //array of element values: number_of_elements * number_of_element_variables * number_of_times
	
	void FEMReadRFOFile(string name);
};

// OK
class CFDMNode
{
  private:
  public:
    long node;
    long *nod_neighbors;
    long *ele_neighbors;
    int no_neighbors;
    CFDMNode(void);
};

/* Deklarationen */
extern vector<CFDMNode*>FDM_node_vector;
extern void WriteFDMNodes(string);

#endif

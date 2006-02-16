/**************************************************************************/
/* ROCKFLOW - Modul: gridadap.h
                                                                          */
/* Aufgabe:
   Funktionen fuer Gitteradaption
                                                                          */
/**************************************************************************/

#ifndef gridadap_INC

#define gridadap_INC
  /* Schutz gegen mehrfaches Einfuegen */

#include "adaptiv.h"
#include "nodes.h"

/* Deklarationen */

extern void ConstructRelationships ( void );
extern int ExecuteGridAdaptation ( int position, int *grid_change );
extern void DelIrrNodes2D (long index, long *global_node_number,
                        double *left_matrix, double *right_vector);
extern void DelIrrNodes3D (long index, long *global_node_number,
                        double *left_matrix, double *right_vector);
extern void CalcIrrNodeVals ( double *ergebnis );
extern void CalcIrrNodeValsOnNode(int ndx);
extern void NodeNumberLimiter ( long maximum_nodes);

extern void RefineEdgeValues( Knoten *kneu, Knoten *kk1, Knoten *kk2 );
extern void RefinePlainValues( Knoten *knoten, long *alte_eckknoten );
extern void Refine3DElementValues( Knoten *knoten, long *alte_eckknoten );

extern void RefEdgeValues( Knoten *kneu, Knoten *kk1, Knoten *kk2, int idx0, int idx1 );
extern void RefPlainValues( Knoten *knoten, long *alte_eckknoten, int idx0, int idx1 );
extern void Ref3DElementValues( Knoten *knoten, long *alte_eckknoten, int idx0, int idx1 );
extern void RefElementData( long number, long *neue_elemente );

extern int IrrNodeGetRegularNeighbors (long, long *);
  /* Regulaere Nachbarknoten eines Irr.Knoten */
extern void EliminateIrrNodes(double *);
  /* Eliminiert irr. Knoten aus dem fertig aufgebauten Gleichungssystem */
extern void BuildActiveElementsArray ( void );
  /* Erstellt Feld mit Nummern der aktiven Elemente */

extern int CriticalElements (long, long *);

extern void AdaptAssignEdgeNodesToElements( void );

#endif


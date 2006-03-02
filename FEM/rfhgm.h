/**************************************************************************/
/* ROCKFLOW - Modul: rfbc.h
                                                                          */
/* Aufgabe: Interface zu HGM

                                                                          */
/**************************************************************************/


#ifndef rfhgm_INC

#define rfhgm_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Lesefunktionen */

/* 1 Mesh elements */
/* D1.1 Mesh Points */
extern int FctHGMPoint ( char *data, int found, FILE *f );
extern int FctHGMEdge ( char *data, int found, FILE *f );
extern int FctHGMPlain ( char *data, int found, FILE *f );
extern int FctHGMPolygon ( char *data,int found, FILE *f );
extern int FctHGMPolyline ( char *data, int found, FILE *f );


/* 2 Mesh Intersection */
/* D2.1 Intersection Method */
extern int FctHGMIntersection ( char *data, int found, FILE *f );


/* 3 Mesh generator */
/* D3.1 Delaunay */
extern int FctHGMDelaunay ( char *data, int found, FILE *f );
/* D3.2 Advancing Front Method */
extern int FctHGMAdvancingFront ( char *data, int found, FILE *f );



/* Netz generieren datei.rfi */

/* HGM-Netzgenrator starten */
extern int StartHGMMeshGenerator ( FILE *prot );
  /* Erzeugt Fem-Netz in Rockflow-Format und schreibt Fehler in Protokolldatei,
     0 bei Fehler */

/* HGM-Netzgenrator ausfuehren */
extern int ExecuteHGMMeshGenerator ( char *mgdat, FILE *prot );
  /* Erzeugt Fem-Netz in Rockflow-Format und schreibt Fehler in Protokolldatei,
     0 bei Fehler */

/* HGM-Netzgenrator beenden */
extern int EndHGMMeshGenerator ( FILE *prot );


#endif


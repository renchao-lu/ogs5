/**************************************************************************/
/* ROCKFLOW - Modul: plot.h
                                                                          */
/* Aufgabe:
   Deklarationen fuer Plot-Funtionen
                                                                          */
/*
                                                                          */
/**************************************************************************/

#ifndef plot_INC

#define plot_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */
#include <stdio.h>

#include "makros.h"


/******************************************************/
/* C1.10 On-line Graphical Output                                 */
/******************************************************/

/* Deklarationen fuer Schluesselwort BCURVE */
extern int bcurve;
extern int bcurve_coor, bcurve_value;
extern double bcurve_xmin;
extern double bcurve_xmax;
extern double bcurve_ymin;
extern double bcurve_ymax;
extern int bcurve_cycle;

#ifdef __RFGRAF
/* ah b */
/* Deklarationen fuer Schluesselwort GRAF */
extern int grafic1d;
extern int graf1d_coor, graf1d_value;
extern double graf_xmin;
extern double graf_xmax;
extern double graf_ymin;
extern double graf_ymax;
extern int graf_cycle;

/* Deklarationen fuer Schluesselwort MESH2D */
extern int mesh2d;                               /* Netz Schalter */
extern int mesh2d_1c, mesh2d_2c;                 /* x-, y-index */
extern double mesh2d_xmin;                       /* Weltkoordinaten xmin */
extern double mesh2d_xmax;                       /* Weltkoordinaten xmax */
extern double mesh2d_ymin;                       /* Weltkoordinaten ymin */
extern double mesh2d_ymax;                       /* Weltkoordinaten ymax */
extern int mesh2d_cycle;

/* Deklarationen fuer Schluesselwort MESH3D */
extern int mesh3d;                               /* Netz Schalter */
extern int mesh3d_1c, mesh3d_2c;                 /* x-, y-index */
extern double mesh3d_alpha;                      /* Drehungswinkel um X-Achse */
extern double mesh3d_beta;                       /* Drehungswinkel um Y-Achse */
extern double mesh3d_gamma;                      /* Drehungswinkel um Z-Achse */
extern double mesh3d_xmin;                       /* Weltkoordinaten xmin */
extern double mesh3d_xmax;                       /* Weltkoordinaten xmax */
extern double mesh3d_ymin;                       /* Weltkoordinaten ymin */
extern double mesh3d_ymax;                       /* Weltkoordinaten ymax */
extern int mesh3d_numbering;                     /* Ausgabe Knotennummer */
extern int mesh3d_cycle;

/* Deklarationen fuer Schluesselwort GRAF_EXTD */
extern int graf_wait;
extern int graf_clear;
extern int graf_grid;
extern double graf_gdx;
extern double graf_gdy;

/* Deklarationen fuer Grafik-Fenster */
extern int root_win,draw_win,netz_win,out_win,inp_win;
extern int but[10];
extern int out_curves[20];
/* ah e */
#endif /* __RFGRAF */

  /* C1.10 Grafics */
extern int FctBcurve ( char *data, int found, FILE *f );
#ifdef __RFGRAF
extern int FctGraf ( char *data, int found, FILE *f );
extern int FctMesh2D ( char *data, int found, FILE *f );
extern int FctMesh3D ( char *data, int found, FILE *f );
extern int FctGrafExtended ( char *data, int found, FILE *f );
#endif /* __RFGRAF */

extern void ConfigGrafics(void);


#endif


/**************************************************************************
   ROCKFLOW - Modul: cvel.h
 
   Aufgabe:
   Funktionen zur Berechnung der Geschwindigkeiten in den Gausspunkten

   letzte Aenderung: RK 03/2003
 
**************************************************************************/
#ifndef cvel_INC
#define cvel_INC
#include "femlib.h"
#include "rf_pcs.h"

/* Initialisiert die Geschw.berechnung */
extern int InitializeKernel_VEL(void);
/* Berechnet und setzt die Filter-Geschwindigkeiten fuer eine Phase
   in den Gausspunkten des eines Elements mit Zeitwichtung */
extern void CECalcGaussVelo(int, long, double);
extern void CECalcGaussVelo_NEW(int,long,double,CRFProcess*);
/* Zerstoert alle Gauss-Geschwindigkeiten der Phase */
extern void DestroyAllElementGaussVelocitiesPhase(int phase);

/* Berechnet die Geschwindigkeiten an lokalen Koordinaten eines Elements  */
extern void CalcVeloXDrst(int phase, long index, double theta, double r, double s, double t, double *v);
extern void CalcVeloXDrstxyz(int phase, long index, double theta, double r, double s, double t, double *v);
extern void CalcVelo1Dr(int, long, double, double, double, double, double *);
extern void CalcVelo2Drs(int, long, double, double, double, double, double *);
extern void CalcVelo3Drst(int, long, double, double, double, double, double *);
extern void CalcVelo2DTriangleRS(int, long, double, double, double, double, double *);
extern void CalcVelo3DPrismXYZ(int, long, double, double *);
extern double* CalcVelo3DPrismRST(int phase, long index, double theta, double r, double s, double t);
extern void VELCalcElementVelocity(long i,double theta, int phase);

extern void CalcVelo3DPrismRST_NEW(int phase, long index, double theta, double r, double s, double t, CRFProcess*m_pcs);//SB
extern void CalcVelo3DTetrahedra(int phase, long index, double theta, CRFProcess*m_pcs);

/* Knotendaten-Schnittstelle */

/* Alt: Liefert Druck am Knoten number */
extern DoubleFuncLong VelocityGetNodePress0;
extern DoubleFuncLong VelocityGetNodePress1;

/* Liefert Druck in Phase alpha am Knoten number auf timelevel theta */
extern DoubleFuncILLD VelocityGetNodePress;

/* Liefert die relative Permeabilitaet im Element number auf den Koordinaten r,s,t */
extern DoubleFuncILDDDD VelocityGetRelativePermeability;

/* Liefert Temperatur am Knoten number */
extern DoubleFuncLong VELGetNodeTemperature;

/*Data access to velocities*/

extern VoidFuncLID VELSetElementVelocityX;
extern VoidFuncLID VELSetElementVelocityY;
extern VoidFuncLID VELSetElementVelocityZ;

extern DoubleFuncLI VELGetElementVelocityX;
extern DoubleFuncLI VELGetElementVelocityY;
extern DoubleFuncLI VELGetElementVelocityZ;
/* Globaldaten-Schnittstelle */
extern void VELCalculateAllGaussVelocities(CRFProcess*);
/* Weitere externe Objekte */
#endif

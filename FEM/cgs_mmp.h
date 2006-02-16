/**************************************************************************
 ROCKFLOW - Modul: cgs_mmp.h

 Aufgabe:
   Aufstellen des Gleichungssystems fuer das Modul MMP.

**************************************************************************/

#ifndef cgs_mmp_INC
#define cgs_mmp_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
extern void MakeGS_MMP(int phasen, double *, double *rechts,
                       char *bc_name,
                       char *source_mass_flow_name,
                       char *source_volume_flow_name,
                       char *sink_volume_mixture_name);

extern void MakeGS_Richards(int phasen, double *, double *rechts,
                            char *bc_name,
                            char *source_mass_flow_name,
                            char *source_volume_flow_name,
                            char *sink_volume_mixture_name);
extern void MakeGS_RichardsNew(int phase, double *ergebnis_MMP, double *rechts, char *bc_name, char *source_mass_flow_name, char *source_volume_flow_name, char *sink_volume_mixture_name);

/* OK - fuer LES und NLES Objekte  */
extern void MakeGS_MMPV1(double *rechts, double *ergebnis_MMP, double dummy);
extern void MakeGS_RichardsV1(double *rechts, double *ergebnis_MMP, double dummy);
extern void MMPCalcPhasePressure(int timelevel);
extern double MMPCalcSaturationPressureDependency(int phase, long index, double r, double s, double t, double theta);

extern void MMPAssembleMatrices(int, double *, double *);
extern void MMPIncorporateSourceBoundaryConditions
 (int phasen, double *rechts,
  char *source_mass_flow_name,
  char *source_volume_flow_name,
  char *sink_volume_mixture_name);

/* Data access to nodal data */
extern IntFuncII MMPGetPhasePressureNodeIndex;
extern void MMPSetICRichards(int); //OK4104
extern void MMPCalcSecondaryVariablesRichards(int timelevel, bool update=true);

#endif

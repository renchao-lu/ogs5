/**************************************************************************/
/* ROCKFLOW - Modul: indicatr.h
                                                                          */
/* Aufgabe:
   Fehlerindikatoren zum Verfeinern und Vergroebern.
                                                                          */
/**************************************************************************/

#ifndef indicatr_INC

#define indicatr_INC
  /* Schutz gegen mehrfaches Einfuegen */

/* Andere oeffentlich benutzte Module */


/* Deklarationen */
extern long Indicator_flow ( int fein, int verf_index, int indicator_index, int counter_indicator );
extern long Indicator_tran ( int fein, int verf_index, int indicator_index, int counter_indicator );
extern long Indicator ( int fein );
extern long Indicatr ( int fein, int verf_index, int indicator_index, int counter_indicator );


extern double Indicator0 ( long index, int typ, int idx0, int idx1 );
/* kein Indikator */

/*************************************************************************/
/*            Heuristische "Fehlerindikatoren" (-> indicat1.c)           */
/*************************************************************************/

extern double Indicator1 ( long index, int typ, int idx0, int idx1 );
/* (gemittelte) Konzentrationsdifferenzen eines Elements */
extern double Indicator2 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator: Konzentrationsgradient im Mittelpunkt des Elements
   index skaliert an einer massgebenden Elementlaenge [John,1994] */
extern double Indicator3 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator auf Basis von `Kruemmungen' der Loesungskurve */
extern double Indicator4 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator auf der Basis der charakteristischen Laenge der Elemente */

/*************************************************************************/
/*        Analytische "Fehlerindikatoren" aTM  (-> indicat2.c)           */
/*************************************************************************/

extern double Indicator_tran1 ( long index, int typ, int idx0, int idx1 );
/* Residuenindikator fuer die Poisson-Gleichung nach Bieterman & Babuska */
extern double Indicator_tran2 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator in Anlehnung an einen Residuenindikator
   fuer parabolische Differentialgleichungen */
extern double Indicator_tran3 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator fuer hyperbolische Dgln. nach Johnson [1990] */


/*************************************************************************/
/*        Analytische "Fehlerindikatoren" aSM  (-> indicat2.c)           */
/*************************************************************************/

extern double Indicator_flow1 ( long index, int typ, int idx0, int idx1 );
/* Residuenindikator fuer die Poisson-Gleichung nach Bieterman & Babuska */
extern double Indicator_flow2 ( long index, int typ, int idx0, int idx1 );
/* Fehlerindikator in Anlehnung an einen Residuenindikator
   fuer parabolische Differentialgleichungen */
extern double Indicator_flow3 ( long index, int typ, int idx0, int idx1 );
/* Verfeinerungsindikator (Sprungindikator) (zunaechst zu Testzwecken) */
extern double Indicator_flow4 ( long index, int typ, int idx0, int idx1 );

/* Weitere externe Objekte */

extern void CoarsenNeighbourElements (long index);
extern void CoarsenAllNeighbourElements (long index);
extern void InitMemoryIndicator (int iter);

#endif

#include "rf_pcs.h"

/* Aufruf der Berechnungsunktionen */
void VELCalcAll(CRFProcess*);

/* Aufruf der Berechnungsunktionen */
void VELCalcGauss(int, long, CRFProcess*);
/* Aufruf der Berechnungsunktionen und
   Setzen der Geschwindigkeiten im Mittelpunkt eines Elementes */
void VELCalcEle(int, long, CRFProcess*);

/* Setzen der Geschwindigkeiten aller Gausspunkte eines Elementes */
void VELCalcGaussLine(int phase, long index, CRFProcess*m_pcs);
void VELCalcGaussQuad(int phase, long index, CRFProcess*m_pcs);
void VELCalcGaussHex(int phase, long index, CRFProcess*m_pcs);
void VELCalcGaussTri(int phase, long index, CRFProcess*m_pcs);  //ruft VELCalcTriGlobal auf
void VELCalcGaussTet(int phase, long index, CRFProcess*m_pcs);  //ruft VELCalcTetGlobal auf
void VELCalcGaussPris(int phase, long index, CRFProcess*m_pcs); 

/* Berechnung der Geschwindigkeiten in einem Gausspunkt */
void VELCalcGaussLineLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velovec);
void VELCalcGaussQuadLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velovec);
void VELCalcGaussHexLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velovec);
void VELCalcGaussPrisLocal(int phase, long index, double* gp, CRFProcess*m_pcs, double *velovec);

/* Berechnung der Geschwindigkeiten im Element Mittelpunkt */
/* Rückgabe in globalen Koordinaten ? */
void VELCalcTriGlobal(int phase, long index, CRFProcess*m_pcs, double *velovec);
void VELCalcTetGlobal(int phase, long index, CRFProcess*m_pcs, double *velovec);
void VELCalcPrisGlobal(int phase, long index, CRFProcess*m_pcs, double *velovec);

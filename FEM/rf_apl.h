/**************************************************************************/
/* ROCKFLOW - Modul: rf_apl.h
                                                                          */
/* Aufgabe:
                                                                          */
/* Programmaenderungen:

   letzte Aenderung: OK 13.07.1999
                                                                          */
/**************************************************************************/

#ifndef rf_apl_INC

#define rf_apl_INC
  /* Schutz gegen mehrfaches Einfuegen */

#include "rf_pcs.h" //OK_MOD"

extern int RF_FEM (char *dateiname);
extern int RFPre_FEM(char* dateiname);


extern void CreateObjectLists(void);
extern void DestroyObjectLists(void);
extern void DestroyRFObjects(void);
extern void RFConfigObjects(void);
extern void ConfigRFFiles(char *dateiname);
extern void RFConfigRenumber(void);


extern IntFuncVoid PreTimeLoop;
extern IntFuncVoid PostTimeLoop;

extern void RFPre_Model();

void RFCreateObjectListsNew(void);

int RFExecuteTimeLoop(void);

#endif


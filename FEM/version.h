/**************************************************************************/
/* ROCKFLOW - Modul: version.h
                                                                          */
/* Aufgabe:
   - Definition der Rockflow-Version
   - Aufgabe der Rockflow-Version
   - Aktivierung/Desaktivierung von Rockflow-Modelle
                                                                          */
/**************************************************************************/


#ifndef rfver_INC

#define rfver_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Global benoetigte Header */
#include <stdlib.h>
  /* Speicherverwaltung */
#include <string.h>
  /* Zeichenketten */

/* Andere oeffentlich benutzte Module */
#include "rf_pcs.h" //OK_MOD"


/* ================================================================ */
/* ROCKFLOW_VERSION_CONFIGURATION */
/* ================================================================ */

/* --------------------------------- */
/* state :  0 -> inactiv    1->activ */
/* --------------------------------- */


ROCKFLOW_VERSION_CONFIGURATION rfmc[] = {
   /*  ID   Name     Group name Model Func     Data Info              state  */
 {     0,  "ASM",   "Rockflow", SetXModel00,   InitModelDataInfo00,   1 }, 
 {     2,  "AGM",   "Rockflow", SetXModel02,   InitModelDataInfo02,   1 }, 
 {     7,  "RSM",   "Rockflow", SetXModel07,   InitModelDataInfo07,   0 },
 {    10,  "DM" ,   "Rockflow", SetXModel10,   InitModelDataInfo10,   1 },

 { 10093,  "SMHTM", "Rockflow", SetXModel0093, InitModelDataInfo0093, 1 },

 { 10097,  "SMRTM", "Rockflow", SetXModel0097, InitModelDataInfo0097, 1 },
 { 10095,  "SMMTM", "Rockflow", SetXModel0095, InitModelDataInfo0095, 1 },
 { 10297,  "GMRTM", "Rockflow", SetXModel0297, InitModelDataInfo0297, 1 },
 { 10797,  "RMRTM", "Rockflow", SetXModel0797, InitModelDataInfo0797, 0 },

 { 10099,  "SMTM",  "Rockflow", SetXModel0099, InitModelDataInfo0099, 1 },
 { 10299,  "GMTM",  "Rockflow", SetXModel0299, InitModelDataInfo0299, 1 },
 { 10699,  "MMTM",  "Rockflow", SetXModel0699, InitModelDataInfo0699, 1 },


 { 10010,  "SMDM",    "Rockflow", SetXModel0010, InitModelDataInfo0010, 1 }, 
 { 100101, "THM__MK", "Rockflow", SetXModel__MK, InitModelDataInfo__MK, 1 }, 
 { 100102, "THM_JDJ", "Rockflow", MODConfigObjects_JDJ, MODConfigObjectsDummy_JDJ, 1 }, 
 { 10333,  "MB",      "Rockflow", MODConfigObjects_MB, MODConfigObjectsDummy_MB, 1 }, 
 { 100951, "MX",      "Rockflow", SetXModel_MX,  InitModelDataInfo_MX,  1 },
 // { 10011,  "WW",      "Rockflow", SetXModel_WW,  InitModelDataInfo_WW,  1 }, 
 { 18460,  "THM+",    "Rockflow", SetXModel8460, InitModelDataInfo8460, 1 }, 


 {    -1,  "",       "",        NULL,          NULL,                  0 }  /* end */
};


#endif


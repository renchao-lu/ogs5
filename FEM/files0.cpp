/**************************************************************************/
/* ROCKFLOW - Modul: files0.c
                                                                          */
/* Aufgabe:
   Enthaelt die uebergeordneten Datei- Ein- und Ausgaberoutinen, sowie
   das Speichern der Durchbruchskurven.
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   02/1999     CT         Bugfix: Anpassung fuer mit 0 beginnende Elemente.
                          Kennung fuer Version im Kopf. Versionsabhaengiges
                          lesen.
   02/1999     CT         Korrekturen fuer herausgeloeste Materialgruppen
   03/1999     AH         Korrekturen nicht noetig, da die Materialzuweisung
                          nicht mehr auf dieser Ebene erfolgt. Die Abfrage ist
                          jetzt auskommentiert. Sie war hier nur um Kompatibilitaeten
                          zum Rockflow aufzubewahren.
   03/1999     CT         anz_matxx, start_mat entfernt
   02/2000     CT         Restart wieder hergestellt
   07/2000     AH         Vorbereitungen zum HGM.
    9/2000     CT         Neu: RefreshNodeOutputData, Warnungen beseitigt
   10/2001     AH         Inverse Modellierung 
                          Trennung und Anpassung (CreateFileData)
                          In DestroyFileData Datenfeld auskommentiert.
                          Neues Konzept fuer Datenbank-Verwaltung ah inv
   01/2002     MK         DisplayMsgX-Umleitung in *.msg-Datei: OpenMsgFile
   08/2002     MK         GetPathRFDFile
                          ConfigFileData aus CreateFileData herausgeloest
   03/2003     RK         Quellcode bereinigt, Globalvariablen entfernt
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

#include <iostream>
/* Preprozessor-Definitionen */
#include "makros.h"
/* Objekte */
#include "files.h"
#include "nodes.h"
#include "elements.h"
#include "solver.h"
#include "rf_pcs.h"
#include "rf_mmp_new.h"
#include "rfmat_cp.h"
#include "gridadap.h"
#include "tools.h"
#include "rfidb.h"
#include "rf_st_new.h"
#include "rf_bc_new.h"
#include "rf_ic_new.h"
#include "rf_pcs.h"
#include "rf_out_new.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "rf_msp_new.h"
#include "rf_num_new.h"
#include "rf_random_walk.h" // PCH
#include "rf_react.h"
#include "rf_fluid_momentum.h"  // PCH
#include "rf_fct.h" //OK
/* Tools */
#include "mathlib.h"
#include "femlib.h"
#include "rfpriref.h"
#include "rfhgm.h"
#include "rf_db.h"
/* GeoLib */
#include "geo_lib.h"
#include "geo_strings.h"

#include "msh_lib.h"
#include "gs_project.h"

//#include "rf_ele_msh.h"

/* Interne (statische) Deklarationen */

/* Dateinamen */
static char *outdat = NULL;
static char *msgdat = NULL;
static char *pthdat = NULL;
static char *savedat[2] = { NULL, NULL };
static char *indat = NULL;
static char *protdat = NULL;
static char *ucddat = NULL;
static char *mgdat = NULL;
static char *mgpdat = NULL;
static char *invdat = NULL; /* ah inv */
static char *invpdat = NULL; /* ah inv */
char *crdat = NULL; /*MX*/
char *file_name = NULL; /* dateiname */

char **nodedat = NULL;
char **elemdat = NULL;
static int datlen;
//static int erste_ausgabe = 1;  /* wg. Dateimodus beim Oeffnen */
//static int erste_n = 1;
//static int erste_e = 1;
static char kopf[81];

static char s[MAX_ZEILE];

/* sonstige Auswertungs-/Berechnungsfunktionen */
int CheckGlobalConnections1 ( FILE *f );
int CheckGlobalConnections2 ( FILE *f );

int ReadRFInputFile ( char *datei, FILE *prot );


/* Definitionen */

IntFuncFileX CGC1_Model;
IntFuncFileX CGC2_Model;

DatenFeld *datafield_n = NULL;
DatenFeld *datafield_e = NULL;
int danz_n = 0;
int danz_e = 0;
int danz_sum_n = 0;
int danz_sum_e = 0;
int rfi_filetype = 0;


void DATCreateFileNames(char *dateiname)
{
  datlen = (int)strlen(dateiname) + 5;
  indat = (char *) Malloc(datlen);
  indat = strcat(strcpy(indat,dateiname),TEXT_EXTENSION);
  protdat = (char *) Malloc(datlen);
  protdat = strcat(strcpy(protdat,dateiname),PROTOCOL_EXTENSION);
  ucddat = (char *) Malloc(datlen);
  ucddat = strcat(strcpy(ucddat,dateiname),RF_INPUT_EXTENSION);
  mgdat = (char *) Malloc(datlen);
  mgdat = strcat(strcpy(mgdat,dateiname),MESH_GENERATOR_EXTENSION);
  mgpdat = (char *) Malloc(datlen);
  mgpdat = strcat(strcpy(mgpdat,dateiname),MESH_GENERATOR_PROTOCOL_EXTENSION);
  invdat = (char *) Malloc(datlen);
  invdat = strcat(strcpy(invdat,dateiname),INVERSE_EXTENSION);
  invpdat = (char *) Malloc(datlen);
  invpdat = strcat(strcpy(invpdat,dateiname),INVERSE_PROTOCOL_EXTENSION);
  crdat = (char *) Malloc(datlen);                                      /*MX*/
  crdat = strcat(strcpy(crdat,dateiname),CHEM_REACTION_EXTENSION);    /*MX*/

}

void DATDestroyFileNames(void)
{
  indat = (char *)Free(indat);
  protdat = (char *)Free(protdat);
  ucddat = (char *)Free(ucddat);
  mgdat = (char *) Free(mgdat);
  mgpdat = (char *) Free(mgpdat);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReadData
                                                                          */
/* Aufgabe:
   Liest Daten aus den Eingabedateien ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler oder Ende aufgrund Dateitest, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1996    MSR     Erste Version
   07/2000    AH      Eingabe fuer Netzgenerator
   10/2001    AH      Trennung und Anpassung (CreateFileData)
                      Inverse Modellierung
                      Neues Konzept fuer Datenbank-Verwaltung
   10/2002   OK   DATCreateFileNames(dateiname)
                  DATDestroyFileNames()

   last modified: OK 16.10.2002
                                                                          */
/**************************************************************************/
int ReadData ( char *dateiname )
{
  FILE *prot;
  int i;
  int ok = 1;
  bool MSH = false;

  cout << endl;
  cout << "---------------------------------------------" << endl;
  cout << "Data input:" << endl;
  /* Dateinamen generieren */
  DATCreateFileNames(dateiname);
  //----------------------------------------------------------------------
  // Read GEO data
  GEOLIB_Read_GeoLib(dateiname);
  //----------------------------------------------------------------------
  // Read object data
  PCSRead(dateiname);
  BCRead(dateiname);
  STRead(dateiname);
  ICRead(dateiname);
  OUTRead(dateiname);
  TIMRead(dateiname);
  MFPRead(dateiname);
  MSPRead(dateiname);
  MMPRead(dateiname);
  CPRead(dateiname); //SB:GS4
  RCRead(dateiname);
  NUMRead(dateiname);
  if(FEMRead(dateiname)) //OK4108//WW4107
     MSH = true; 
//SBOK4209 FEMWrite(dateiname);
  // PCTRead is bounded by msh
  PCTRead(dateiname);   // PCH
  FMRead(dateiname);    // PCH
  FCTRead(dateiname); //OK
  //----------------------------------------------------------------------
  // Read Excel/CVS data
  //PNTPropertiesRead(dateiname);
  /****************************************************************************/
  /* Read RFD file, write RFE protocol file */
   /* Protokolldatei oeffnen */
  if ((prot = StartProtocol("ROCKFLOW",protdat))==NULL) {
      DATDestroyFileNames();
      return 0;
  }
   /* Text-Eingabedatei lesen */
  if (!ReadTextInputFile(indat,prot)) {
      cout << "ReadTextInputFile: input of RFD data" << endl;
      DisplayErrorMsg("Fehler: Text-Eingabedatei konnte nicht gelesen werden !");
      DATDestroyFileNames();
      return 0;
  }
   /* Globale Zusammenhaenge pruefen und Variablen berechnen (1) */
  if (!CheckGlobalConnections1(prot)) {
      DisplayErrorMsg("Fehler: Text-Eingabedatei nicht stimmig !");
      ok = 0;
  }
   /* Abbruch, wenn ok==0 oder Ende, wenn modex==-1 */
  if (!ok){
      DATDestroyFileNames();
      return 0;
  }
  /* Protokolldatei schliessen */
  if (!EndProtocol(prot)) {
      DisplayErrorMsg("Fehler: Protokolldatei konnte nicht geschlossen werden !");
      ok = 0;
  }
  if(MSH) return 100;
/* erstmal ausgeknipst OK */
#ifdef TEST_MSH_GEN
  /****************************************************************************/
  /* Read mesh generator RFM file, write RFG protocol file */
  /* Protokolldatei fuer Netzgenerator oeffnen */
  if ( GetRFControlModex()==0  || GetRFControlModex()==2 )
    if ((mgprot = StartMGProtocol("ROCKFLOW-HGM",mgpdat))==NULL) {
      DisplayErrorMsg("Warnung: MGEN-Protokolldatei konnte nicht geoeffnet werden !");
      DATDestroyFileNames();
      return 0;
    }

  /* RF-MG-Eingabedatei lesen */
  if ( GetRFControlModex()==0  || GetRFControlModex()==2 ) 
    ok=ReadMGTextInputFile(mgdat,mgprot);
  if (!ok) {
      DisplayErrorMsg("Warnung: MGEN-Eingabedatei konnte nicht gelesen werden !");
      /* Protokolldatei des Netzgenerators schliessen */
      if (!EndMGProtocol(mgprot))
          DisplayErrorMsg("Warnung: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }

  /* HGM starten */
  if ( GetRFControlModex()==0  || GetRFControlModex()==2 ) ok=StartHGMMeshGenerator(mgprot);
  if (!ok) {
      DisplayErrorMsg("Fehler: HGMMeshGenerator konnte nicht gestartet werden !");
      /* Protokolldatei des Netzgenerators schliessen */
      if (!EndMGProtocol(mgprot))
          DisplayErrorMsg("Fehler: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }

  /* HGM ausfuehren */
  if ( GetRFControlModex()==0  || GetRFControlModex()==2 ) ok=ExecuteHGMMeshGenerator(ucddat,mgprot);
  if (!ok) {
      DisplayErrorMsg("Fehler: HGMMeshGenerator konnte nicht ausgefuehrt werden !");
      /* Protokolldatei des Netzgenerators schliessen */
      if (!EndMGProtocol(mgprot))
          DisplayErrorMsg("Fehler: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }

  /* HGM beenden */
  if ( GetRFControlModex()==0  || GetRFControlModex()==2 ) ok=EndHGMMeshGenerator(mgprot);
  if (!ok) {
      DisplayErrorMsg("Fehler: HGMMeshGenerator konnte nicht beendet werden !");
      /* Protokolldatei des Netzgenerators schliessen */
      if (!EndMGProtocol(mgprot))
          DisplayErrorMsg("Fehler: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }

  /* Abbruch, wenn ok==0 oder Ende, wenn modex==0 */
  if ( (!ok) || GetRFControlModex()==0 ) {
      /* Protokolldatei des Netzgenerators schliessen */
      if (!EndMGProtocol(mgprot))
          DisplayErrorMsg("Fehler: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }
  /* Protokolldatei fuer Netzgenerator schliessen */
  if ( GetRFControlModex()==2 )
    if (!EndMGProtocol(mgprot)) {
      DisplayErrorMsg("Fehler: MGEN-Protokolldatei konnte nicht geschlossen werden !");
      ok = 0;
    }
#endif
  /****************************************************************************/
  /* Read geometry RFI file */
  // MSH
  /* RF-Eingabedatei lesen */
  if (!ReadRFInputFile(ucddat,prot)) {
      cout << "ReadRFInputFile: input of RFI data" << endl;
      DisplayErrorMsg("Fehler: RF-Eingabedatei konnte nicht gelesen werden !");
      DATDestroyFileNames();
      if(fem_msh_vector.size()==0) //OK4108
        return 0;
  }
   /* Vorverfeinerer */
   AprioriRefineElements ("APRIORI_REFINE_ELEMENT");

   /*Anpassung der Gitter Z-Werte an einen *.dat file*/
   /*mappingTosurface(dateiname, NodeListSize());*/


  /* Globale Zusammenhaenge pruefen und Variablen berechnen (2) */
  if (!CheckGlobalConnections2(prot)) {
      DisplayErrorMsg("Fehler: RF-Eingabedatei nicht stimmig !");
      ok = 0;
  }

  /* Abbruch, wenn ok==0 */
  if (!ok) {
      DATDestroyFileNames();
      return 0;
  }
  /* erweitertes Protokoll bei modex==-2 */
  if (GetRFControlModex()==-2) {
      /* Protokolldatei erneut oeffnen */
      if ((prot = StartExtProtocol("ROCKFLOW",protdat))==NULL) {
          DisplayErrorMsg("Fehler: RF-Protokolldatei konnte nicht geoeffnet werden !");
          DATDestroyFileNames();
          return 0;
      }
      /* Kopf schreiben */
      for (i=0;i<80;i++)
          kopf[i] = ' ';
      kopf[80] = '\0';
      sprintf(kopf," 0 0 %d %d %#g 0",0,-1,0.0);
      for (i=0;i<80;i++)
          if ((kopf[i]==' ') || (kopf[i]=='\0'))
              kopf[i] = '#';
      if (fprintf(prot,"%s\n",kopf)!=81) {
          DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Kopf) !!");
          fclose(prot);
          return 0;
      }
      /* Geometrie Schreiben */
      if (!GeomWriteRFOutputFileASCII(prot)) {
          DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Geom) !!");
          fclose(prot);
          return 0;
      }
      /* Daten Schreiben */
      if (!ResWriteRFOutputFileASCII(prot)) {
          DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Res) !!");
          fclose(prot);
          return 0;
      }
      /* Protokolldatei erneut schliessen */
      if (!EndExtProtocol(prot))
          DisplayErrorMsg("Fehler: RF-Protokolldatei konnte nicht geschlossen werden !");
      DATDestroyFileNames();
      return 0;
  }


  /****************************************************************************/
  /* Read inverse modelling RFV file */
  /* rfv-Datei (Inverses Modellieren) ah db */
  SetDataBase("INVERSE-ROCKFLOW",invdat,invpdat);       
  if ( OpenDataBase("INVERSE-ROCKFLOW") ) {  
    ReadDataBase("INVERSE-ROCKFLOW");
    SetTextInverseFile("INVERSE-ROCKFLOW");    /* in files1.c definiert */
    InitInputFile("INVERSE-ROCKFLOW");
    ExecuteInputFile("INVERSE-ROCKFLOW");
    CloseDataBase("INVERSE-ROCKFLOW");
  }

  /* Speicher freigeben */
  invdat = (char *) Free(invdat); /* ah inv */
  invpdat = (char *) Free(invpdat); /* ah inv */
  DATDestroyFileNames();

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateFileData
                                                                          */
/* Aufgabe:
   Erstellt Datenstruktur f? Leseroutine ReadData
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *datei: Dateiname mit Extension
   E FILE *prot: Dateizeiger auf Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Trennung und Anpassung (aus ReadData)
   06/2002    MK      Pfad bestimmen (z.B. fuer Output_ex)
   07/2002    MK      Trennung in CreateFileData und ConfigFileData 
                      bzgl. nodedat und elemdat
                                                                          */
/**************************************************************************/
void CreateFileData ( char *dateiname )
{
  int i;
  char *bsdest;
  FILE *f = NULL;
   
  datlen = (int)strlen(dateiname) + 5;

  /* Datei-Daten anlegen */
  outdat = (char *) Malloc(datlen);
  msgdat = (char *) Malloc(datlen);
  pthdat = (char *) Malloc(datlen);
  savedat[0] = (char *) Malloc(datlen);
  savedat[1] = (char *) Malloc(datlen);
  outdat = strcat(strcpy(outdat,dateiname),RF_OUTPUT_EXTENSION);
  msgdat = strcat(strcpy(msgdat,dateiname),RF_MESSAGE_EXTENSION);
  if ((f = fopen(msgdat,"r")) == NULL) { /* MSG-Datei existiert nicht */
    msgdat = (char *)Free(msgdat);
  } else {
    fclose (f); 
    if ((f = fopen(msgdat,"a")) == NULL) { /* MSG-Schreibzugriff nicht moeglich */
      msgdat = (char *)Free(msgdat);
    } else fclose (f); 
  }
    /* / oder \ suchen um Pfad zu bestimmen*/
  i = 0;  
  bsdest = strrchr (dateiname,'/');
  if (bsdest == NULL) {
    bsdest = strrchr (dateiname,'\\'); /* \\ = backslash fuer DOS */
  }  
  if (bsdest != NULL) {
    i = (int)(bsdest - dateiname + 1);
  }
  strcpy(pthdat,dateiname);
  pthdat[i] = dateiname[(int)strlen(dateiname)]; /* end of string noch vorn vrschieben */

  savedat[0] = strcat(strcpy(savedat[0],dateiname),RF_SAVE_EXTENSION1);
  savedat[1] = strcat(strcpy(savedat[1],dateiname),RF_SAVE_EXTENSION2);
 
  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigFileData
                                                                          */
/* Aufgabe:
   Erstellt Namen fuer den Dateizugriff nach ReadData
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *datei: Dateiname mit Extension
   E FILE *prot: Dateizeiger auf Protokolldatei
                                                                          */
/* Ergebnis:
   -
                                                                           */
/* Programmaenderungen:
   07/2002    MK      ConfigFileData aus CreateFileData extrahiert 
                                                                          */
/**************************************************************************/
//OK_OUT

/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyFileData
                                                                          */
/* Aufgabe:
   Destruktor fuer Dateien
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1999   OK   Implementierung
   10/2001   AH   Inverse Modellierung: DatenFeld auskommentiert 
   09/2002   OK   Speicherfreigabe wieder aktiviert
                                                                          */
/**************************************************************************/
void DestroyFileData ( void )
{
  outdat = (char *)Free(outdat);
  msgdat = (char *)Free(msgdat);
  pthdat = (char *)Free(pthdat);
  savedat[0] = (char *)Free(savedat[0]);
  savedat[1] = (char *)Free(savedat[1]);
/* ah inv - auskommentiert */
/* ok_debug wieder aktiviert, wegen Speicherfehler */
  datafield_n = (DatenFeld *)Free(datafield_n);
  datafield_e = (DatenFeld *)Free(datafield_e);
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReadRFInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus den Eingabedateien ein
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *datei: Dateiname mit Extension
   E FILE *prot: Dateizeiger auf Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   25.02.1999  C.Thorenz  Bugfix: Anpassung fuer mit 0 beginnende Elemente.
                          Kennung fuer Version im Kopf. Versionsabhaengiges
                          lesen.
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen

                                                                          */
/**************************************************************************/
int ReadRFInputFile ( char *datei, FILE *prot )
{
  FILE *f;
  int art, bin, nr, geom, i;
  long zeitschritt;
  double startzeit;
  /* RF-Datei oeffnen */
  if ((f = fopen(datei,"rb"))==NULL) {
      DisplayErrorMsg("Fehler: RFI-Datei konnte nicht geoeffnet werden !");
      return 0;
  }
  /* Kopf binaer lesen */
  if (fread((void *)kopf,80,1,f)!=1) {
      DisplayErrorMsg("Fehler beim Lesen der RFI-Datei (Kopf) !!");
      fclose(f);
      return 0;
  }
  kopf[80] = '\0';
  for (i=0;i<80;i++)
      if (kopf[i]=='#')
          kopf[i] = ' ';

  sscanf(kopf,"%d %d %d %d %lg %ld %d",&art,&bin,&nr,&geom,&startzeit,&zeitschritt,&rfi_filetype);

  /* richtige Datei ? */
  if (art!=0) {
      DisplayErrorMsg("Fehler: Falsche RF-Eingabedatei !!");
      fclose(f);
      return 0;
  }
  /* zu lesende Daten bestimmen */
/* OK rf3261 - unklare Funktionsweise */
  if (startzeit>0.0)
      SelectData(2);
  else
      SelectData(1);
  /* ASCII oder binaer ? */
  if (!bin) {
#ifdef SX
	  char *buffer;        /* SX Holger's optimization */
#endif
      fclose(f);
      /* RF-Datei erneut oeffnen (ascii) */
      if ((f = fopen(datei,"r"))==NULL) {
          DisplayErrorMsg("Fehler: RFI-Datei konnte nicht geoeffnet werden !");
          return 0;
      }
#ifdef SX
      buffer=(char *)malloc(4096*4096);         /* <---- SX NEU Holger's optimization */
      setvbuf(f,buffer,_IOFBF,4096*4096); /* <---- SX NEU Holger's optimization */
#endif
      /* Kopf ueberlesen */
      if (fgets(s,MAX_ZEILE,f)==NULL) {
          DisplayErrorMsg("Fehler: RFI-Datei konnte nicht angelesen werden !");
          fclose(f);
          return 0;
      }
  }
  /* nr auswerten */
/*OK_OUT
  if (output_restart)
      output_nr = 0;
  else {
      if (output_start)
          output_nr = nr;
      else
          output_nr = nr + 1;
  }
*/
  /*  Geometrie und Daten lesen */
  if (bin) {
      if (!GeomReadRFInputFileBIN(f,prot)) {
          DisplayErrorMsg("Fehler beim Lesen der RF-Eingabedatei (Geom) !!");
          fclose(f);
          return 0;
      }
      if (!ResReadRFInputFileBIN(f,prot)) {
          DisplayErrorMsg("Fehler beim Lesen der RF-Eingabedatei (Res) !!");
          fclose(f);
          return 0;
      }
  }
  else {
      if (!GeomReadRFInputFileASCII(f,prot)) {
          DisplayErrorMsg("Fehler beim Lesen der RF-Eingabedatei (Geom) !!");
          fclose(f);
          return 0;
      }
/* OK rf3240 Lesen der RFR Restart-Datei durch ReadRFRRestartData() */
/*
      if (!ResReadRFInputFileASCII(f,prot)) {
          DisplayErrorMsg("Fehler beim Lesen der RF-Eingabedatei (Res) !!");
          fclose(f);
          return 0;
      }
*/
  }
  /* RF-Datei schliessen */
  if (fclose(f)<0) {
      DisplayErrorMsg("Fehler beim Schliessen der RF-Eingabedatei !!");
      return 0;
  }
  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckGlobalConnections1
                                                                          */
/* Aufgabe:
   Ueberprueft globale Zusammenhaenge zwischen den eingelesenen Daten
   verschiedener Schluesselwoerter und berechnet weitere Zusammenhaenge.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   06/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/
int CheckGlobalConnections1 ( FILE *f )
{
  int ok = 1;
  long i, j;
  /* sind alle Randbedingungs-Zeitkurven-Verweise zulaessig ? */
  /* sind alle Randbedingungs-Zeitkurven aufsteigend geordnet ? */
  for (i=0l;i<anz_kurven;i++) {
      for (j=1l;j<kurven[i].anz_stuetzstellen;j++) {
          if ( kurven[i].stuetzstellen[j].punkt <=    \
               kurven[i].stuetzstellen[j-1].punkt ) {
              FilePrintString(f,"* !!! Fehler in Zeitkurve !");
              LineFeed(f);
              ok = 0;
          }
      }
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CheckGlobalConnections2
                                                                          */
/* Aufgabe:
   Ueberprueft globale Zusammenhaenge zwischen den eingelesenen Daten
   der Schluesselwoerter und der UCD-Daten und berechnet weitere
   Zusammenhaenge.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   09/1996     RK         Erste Version
                                                                          */
/**************************************************************************/
int CheckGlobalConnections2 ( FILE *f )
{
  int ok = 1;
  long i, j, k;
  int ii;

  /* ist der ANAV-Zentralknoten gueltig ? */

  /* sind IC- und BC-Knoten gueltig ? */

  /* Tests fuer Knoten der Punkt- und Elementquellen */

  /* Tests fuer Knoten der freien Oberflaeche
  for (i=0l;i<anz_fsf;i++) {
      if (GetNode(nodes_fsf[i])==NULL) {
          FilePrintString(f,"* !!! Fehler: Unzulaessige Knotennummer (FS-F) !");
          LineFeed(f);
          ok = 0;
      }
  }
  for (i=0l;i<anz_fsb;i++) {
      if (GetNode(nodes_fsb[i])==NULL) {
          FilePrintString(f,"* !!! Fehler: Unzulaessige Knotennummer (FS-B) !");
          LineFeed(f);
          ok = 0;
      }
  }
  for (i=0l;i<anz_fss;i++) {
      if (GetNode(nodes_fss[i])==NULL) {
          FilePrintString(f,"* !!! Fehler: Unzulaessige Knotennummer (FS-S) !");
          LineFeed(f);
          ok = 0;
      }
  }
*/

  /* evtl. vorzeitiger Abbruch */
  if (!ok)
      return ok;
  /* Tests der Elemente */
  j = ElListSize();
  start_new_elems = 0l;
  for (i=0;i<j;i++) {
      if (ElGetElement(i)==NULL) {  /* Element existiert nicht */
          /* Sind alle Ausgangselemente fortlaufend nummeriert ? */
          if (!start_new_elems) {
              start_new_elems = i;
          }
      }
      else {
          ii = ElGetElementType(i);
          k = ElGetElementPred(i);
          /* richtige Dimension ? */
          if (k>=0) {
              if (ElGetElementType(k)!=ii) {
                 FilePrintString(f,"* !!! Fehler: Elementtyp nicht stimmig !");
                 LineFeed(f);
                 return 0;
              }
          }
          /* Sind alle Ausgangselemente fortlaufend nummeriert ? */
          if ((k>=0) && (!start_new_elems)) {
              start_new_elems = i;
          }
          else if ((k<0) && (start_new_elems)) {
              FilePrintString(f,"* !!! Fehler: Ausgangselemente nicht fortlaufend oder Materialgruppe nicht vorhanden!");
              LineFeed(f);
              return 0;
          }
      }
  }
  if (!start_new_elems)
      start_new_elems = ElListSize();
  else {  /* level, material, kinder, aktiv der verfeinerten Elemente bestimmen */
      for (i=0;i<start_new_elems;i++) {
          /* ... Rekursion aufrufen */

      }
  }
  /* modellspezifische Tests etc. */
/*  ok = ((CGC2_Model(f)) && ok); OK rf3240 */
  /* Ende */
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: *GetPathRFDFile
                                                                          */
/* Aufgabe:
   Liefert einen eventuellen Pfad der RFD-Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
   - FILE -
                                                                          */
/* Programmaenderungen:
   08/2002     MK        Erste Version
                                                                          */
/**************************************************************************/

char *GetPathRFDFile ()
{
  return pthdat;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: *OpenMsgFile *CloseMsgFile
                                                                          */
/* Aufgabe:
   Oeffnet MSG-Datei fuer Display-Umleitung
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
   - FILE -
                                                                          */
/* Programmaenderungen:
   12/2001     MK        Erste Version
                                                                          */
/**************************************************************************/

FILE *OpenMsgFile ()
{
  FILE *f = NULL;
  if (msgdat) {
    if ((f = fopen(msgdat,"a")) == NULL) {
      f=stdout;
      fprintf(f,"\n!!!!!!!!  %s\n\n            ","Fehler: Schreibzugriff auf Message-Protokolldatei nicht moeglich!!");
    }
  } else f=stdout; /* Dateiname existiert nicht */ 
  return f;
}

void CloseMsgFile (FILE *f)
{
  if (f!=stdout)
    if (fclose(f))
      DisplayErrorMsg("Fehler: Message-Protokolldatei konnte nicht geschlossen werden !!");
  
  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: WriteData
                                                                          */
/* Aufgabe:
   Schreibt Daten in RF-Ausgabedatei(en)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int kopie: andere Dateiendungen wg. Sicherheitskopie (1);
                Standard: 0
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
//OK_OUT

/**************************************************************************/
/* ROCKFLOW - Funktion: WriteNodeCurves
                                                                          */
/* Aufgabe:
   Schreibt Durchbruchskurven fuer Knoten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/1996     MSR        Erste Version
    7/2002     CT         SelectData reingenommen
                                                                          */
/**************************************************************************/
//OK_OUT

/**************************************************************************/
/* ROCKFLOW - Funktion: WriteElementCurves
                                                                          */
/* Aufgabe:
   Schreibt Durchbruchskurven fuer Elemente
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   10/1996     MSR        Erste Version
                                                                          */
/**************************************************************************/
//OK_OUT

/**************************************************************************/
/* ROCKFLOW - Funktion: SelectData
                                                                          */
/* Aufgabe:
   Stellt zu lesende bzw. zu schreibende Knoten- und Elementdaten
   zusammen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int lsr: 0:speichern, 1:laden, 2:restart
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/1996     MSR        Erste Version
    9/2000     CT         Warnung beseitigt
                                                                          */
/**************************************************************************/
void SelectData ( int lsr )
{
  int i, j;  /* Laufvariablen und Zwischenspeicher */
  /* Initialisierungen */
  datafield_n = (DatenFeld *)Free(datafield_n);
  datafield_e = (DatenFeld *)Free(datafield_e);
  danz_n = 0;
  danz_e = 0;
  danz_sum_n = 0;
  danz_sum_e = 0;
  /* Knoten */
  for (i=0;i<anz_nval;i++) {
      switch (lsr) {
          case 0: j = nval_data[i].speichern; break;
          case 1: j = nval_data[i].laden; break;
          case 2: j = nval_data[i].restart; break;
          default: DisplayErrorMsg("!!! Fehler: SelectData-Parameter falsch !");
                     abort();
      }
      if (j) {
          danz_n++;
          danz_sum_n += j;
          datafield_n = (DatenFeld *) Realloc(datafield_n,danz_n*sizeof(DatenFeld));
          datafield_n[danz_n-1].idx_program = i;
          datafield_n[danz_n-1].idx_file = -1;  /* Default */
          datafield_n[danz_n-1].transfer = j;
          datafield_n[danz_n-1].name = nval_data[i].name;
          datafield_n[danz_n-1].einheit = nval_data[i].einheit;
      }
  }
  /* Elemente */
  for (i=0;i<anz_eval;i++) {
      switch (lsr) {
          case 0: j = eval_data[i].speichern; break;
          case 1: j = eval_data[i].laden; break;
          case 2: j = eval_data[i].restart; break;
          default: DisplayErrorMsg("!!! Fehler: SelectData-Parameter falsch !");
                     abort();
      }
      if (j) {
          danz_e++;
          danz_sum_e += j;
          datafield_e = (DatenFeld *) Realloc(datafield_e,danz_e*sizeof(DatenFeld));
          datafield_e[danz_e-1].idx_program = i;
          datafield_e[danz_e-1].idx_file = -1;  /* Default */
          datafield_e[danz_e-1].transfer = j;
          datafield_e[danz_e-1].name = eval_data[i].name;
          datafield_e[danz_e-1].einheit = eval_data[i].einheit;
      }
  }
}




/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyFiles
                                                                          */
/* Aufgabe:
   Destruktion von Files:
   - Speicherfreigabe fuer File-Namen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   12/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
void DestroyFiles(void)
{
    /* sonstige Strings */
    file_name = (char *)Free(file_name);
	crdat = (char *)Free(crdat); /* MX */
}

/*************************************************************************
  ROCKFLOW - Function: RFIReadFile
  Task: Input of RFI data
  Programming: 12/2002   OK   Implementation
  last modified:
**************************************************************************/
int RFIReadFile(char *file_name_base) 
{
  /* File names */
  char m_strFileNameRFI[MAX_ZEILE];
  FILE *prot=NULL;

  strcpy(m_strFileNameRFI,file_name_base);
  strcat(m_strFileNameRFI,RF_INPUT_EXTENSION);

  /* 3 Objekt-Listen */
  DestroyNodeList();
  ElDestroyElementList();
  CreateNodeList();
  ElCreateElementList();


  /****************************************************************************/
  /* Read geometry RFI file */
  /* RF-Eingabedatei lesen */
  if (!ReadRFInputFile(m_strFileNameRFI,prot)) {
      DisplayErrorMsg("Fehler: RF-Eingabedatei konnte nicht gelesen werden !");
      return 0;
  }

   
  /*--------------------------------------------------------------------*/
  /* Jakobian und Element-Volumen berechnen */

  return 1;
}

/**************************************************************************
GeoSys-Method: FEMOpen->RFDOpen
Task: 
Programing:
11/2003 OK Implementation
08/2004 OK PCS2
01/2005 OK Boolean type
**************************************************************************/
bool RFDOpen(string file_name_base)
{
  //----------------------------------------------------------------------
  // File handling
  char *file_name_char = 0;
  file_name_char = (char*)Malloc(sizeof(char)*((long)file_name_base.length()+7));
  FILE *rfe_file = NULL;
  sprintf(file_name_char,"%s",file_name_base.data());

  const char *file_name_const_char = 0; 
  string rfe_file_name = file_name_base + "." + "rfe";
  file_name_const_char = rfe_file_name.data();
  rfe_file = fopen(file_name_const_char, "w+t");
  //----------------------------------------------------------------------
  // Old lists - ToDo
//OK  CreateIterationPropertiesList();
//OK  CreateMeshGenerationList();
  CreateRefineElementList();
  CreateFileData(file_name_char);
  //----------------------------------------------------------------------
  // Read RFD file
  string rfd_file_name = file_name_base + TEXT_EXTENSION;
  sprintf(file_name_char,"%s",rfd_file_name.data());
  if (!ReadTextInputFile(file_name_char,rfe_file)) {
    DisplayErrorMsg("Fehler: Text-Eingabedatei konnte nicht gelesen werden !");
#ifdef MFC
    AfxMessageBox("CRockflowDoc::Serialize(): Error in Reading RFD File ! (see RFE File)");
#endif
    fclose(rfe_file);
    return false;
  }
  return true;
  // Create PCS
  //LOPPreTimeLoop_PCS(); -> to process dialog
}




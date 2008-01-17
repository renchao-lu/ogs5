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
#include "tools.h"
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
#include "rf_db.h" //OK
#ifdef CHEMAPP
  #include "eqlink.h"  //MX
#endif
/* Tools */
#include "mathlib.h"
#include "femlib.h"
/* GeoLib */
#include "geo_lib.h"
#include "geo_strings.h"
// MSHLib
#include "msh_lib.h"
#include "gs_project.h"
/* Dateinamen */
char *crdat = NULL; /*MX*/
char *file_name = NULL; /* dateiname */
static char *msgdat = NULL;

#define RFD_FILE_EXTENSION ".rfd" //OK
void CURRead(string); //OK
ios::pos_type CURReadCurve(ifstream*); //OK
void CURWrite(); //OK

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
  bool MSH = false;
#if defined(USE_MPI) //WW
  if(myrank==0)
  {
#endif
  cout << endl;
  cout << "---------------------------------------------" << endl;
  cout << "Data input:" << endl;
#if defined(USE_MPI) //WW
  }
#endif
  /* Dateinamen generieren */
//OK  DATCreateFileNames(dateiname);
  static int datlen;
  datlen = (int)strlen(dateiname) + 5;
  crdat = (char *) Malloc(datlen);                                      /*MX*/
  crdat = strcat(strcpy(crdat,dateiname),CHEM_REACTION_EXTENSION);    /*MX*/
  msgdat = (char *) Malloc(datlen);
  msgdat = strcat(strcpy(msgdat,dateiname),RF_MESSAGE_EXTENSION);
  FILE *f = NULL;
  if ((f = fopen(msgdat,"r")) == NULL) { /* MSG-Datei existiert nicht */
    msgdat = (char *)Free(msgdat);
  } else {
    fclose (f); 
    if ((f = fopen(msgdat,"a")) == NULL) { /* MSG-Schreibzugriff nicht moeglich */
      msgdat = (char *)Free(msgdat);
    } else fclose (f); 
  }
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
#ifdef CHEMAPP
  CHMRead(dateiname); //MX for CHEMAPP
#endif
  NUMRead(dateiname);   
  if(FEMRead(dateiname)) //OK4108//WW4107
  {
     CompleteMesh(); //WW
     MSH = true;
  }
//SBOK4209 MSHWrite(dateiname);
#ifdef RANDOM_WALK
  // PCTRead is bounded by msh
  PCTRead(dateiname);   // PCH
#endif
  FMRead(dateiname);    // PCH
  FCTRead(dateiname); //OK
  CURRead(dateiname); //OK
  //CURWrite(); //OK
  //----------------------------------------------------------------------
  // Read Excel/CVS data
  //PNTPropertiesRead(dateiname);
  //----------------------------------------------------------------------
  /* Protokolldatei oeffnen */
  cout << "ReadTextInputFile: input of RFD data" << endl;
  static char *indat = NULL;
  indat = (char *) Malloc(datlen);
  indat = strcat(strcpy(indat,dateiname),TEXT_EXTENSION);
  FILE *prot = NULL;
  static char *protdat = NULL;
  protdat = (char *) Malloc(datlen);
  protdat = strcat(strcpy(protdat,dateiname),PROTOCOL_EXTENSION);
  if((prot = StartProtocol("ROCKFLOW",protdat))==NULL) 
  {
    return 0;
  }
//OK  ReadTextInputFile(indat,prot);
  //----------------------------------------------------------------------
  //OK crdat = (char *) Free(crdat); //OK
  msgdat = (char *)Free(msgdat);
  indat = (char *)Free(indat);
  protdat = (char *)Free(protdat);

  if(MSH) return 100;

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
  return false;
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

/**************************************************************************
FEMLib-Method:
04/2007 OK Implementation
**************************************************************************/
void PRJRead(string base_file_name)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //========================================================================
  // file handling
  string rfd_file_name;
  rfd_file_name = base_file_name + FCT_FILE_EXTENSION;
  ifstream rfd_file (rfd_file_name.data(),ios::in);
  if (!rfd_file.good()) return;
  rfd_file.seekg(0L,ios::beg);
  //========================================================================
  // keyword loop
  cout << "RFDRead" << endl;
  while (!rfd_file.eof()) 
  {
    rfd_file.getline(line,MAX_ZEILE);
    project_title = line;
  } // eof
}

/**************************************************************************
FEMLib-Method:
04/2007 OK Implementation
**************************************************************************/
void CURRead(string base_file_name)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //----------------------------------------------------------------------
  StuetzStellen *stuetz = NULL;
  anz_kurven = 1;
  stuetz = (StuetzStellen *) Malloc(sizeof(StuetzStellen));
  stuetz[0].punkt = 1.0;
  stuetz[0].wert = 1.0;
  kurven = (Kurven *) Malloc(sizeof(Kurven));
  kurven[anz_kurven - 1].anz_stuetzstellen = 1;
  kurven[anz_kurven - 1].stuetzstellen = stuetz;
  //----------------------------------------------------------------------
  // file handling
  string cur_file_name;
  cur_file_name = base_file_name + RFD_FILE_EXTENSION;
  ifstream cur_file (cur_file_name.data(),ios::in);
  if (!cur_file.good()) return;
  cur_file.seekg(0L,ios::beg);
  //========================================================================
  // keyword loop
  cout << "CURRead" << endl;
  while (!cur_file.eof()) 
  {
    cur_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return;
    //----------------------------------------------------------------------
    if(line_string.find("#CURVE")!=string::npos) { // keyword found
      position = CURReadCurve(&cur_file);
      cur_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
}

/**************************************************************************
FEMLib-Method:
04/2007 OK Implementation
**************************************************************************/
ios::pos_type CURReadCurve(ifstream *cur_file)
{
  bool new_keyword = false;
  string hash("#");
  string line_string;
  ios::pos_type position;
  std::stringstream line_stream;
  int anz = 0;
  double d1,d2;
  StuetzStellen *stuetz = NULL;
  //----------------------------------------------------------------------
  while (!new_keyword) 
  {
    position = cur_file->tellg();
//OK    cur_file->getline(buffer,MAX_ZEILE);
//OK    line_string = buffer;
    line_string = GetLineFromFile1(cur_file);
	if(line_string.size() < 1) continue;
    //....................................................................
    // Test next keyword
    if(line_string.find(hash)!=string::npos) 
    {
      new_keyword = true;
      continue;
    }
    //--------------------------------------------------------------------
    if(line_string.find(";")!=string::npos) 
    {
      continue;
    }
    //--------------------------------------------------------------------
    //DATA
//OK    cur_file->seekg(position,ios::beg);
//OK    *cur_file >> d1 >> d2;
    line_stream.str(line_string);
    line_stream >> d1 >> d2;
    anz++;
    stuetz = (StuetzStellen *) Realloc(stuetz, (anz * sizeof(StuetzStellen)));
    stuetz[anz - 1].punkt = d1;
    stuetz[anz - 1].wert = d2;
    line_stream.clear();
    //--------------------------------------------------------------------
  }
  //----------------------------------------------------------------------
  if(anz>=1l) 
  {
    anz_kurven++;
    kurven = (Kurven *) Realloc(kurven, (anz_kurven * sizeof(Kurven)));
    kurven[anz_kurven - 1].anz_stuetzstellen = anz;
    kurven[anz_kurven - 1].stuetzstellen = stuetz;
  }
  return position;
}

/**************************************************************************
FEMLib-Method:
04/2007 OK Implementation
**************************************************************************/
void CURWrite()
{
  //========================================================================
  // File handling
  string fct_file_name = "test.cur";
  fstream fct_file (fct_file_name.c_str(),ios::trunc|ios::out);
  fct_file.setf(ios::scientific,ios::floatfield);
  fct_file.precision(12);
  if (!fct_file.good()) return;
  fct_file << "GeoSys-CUR: Functions ------------------------------------------------" << endl ;
  //========================================================================
  int j;
  StuetzStellen stuetz;
  for(int i=0;i<anz_kurven;i++)
  {
    fct_file << "#CURVES" << endl;
    for(j=0;j<kurven[i].anz_stuetzstellen;j++)
    {
      stuetz = kurven[i].stuetzstellen[j];
      fct_file << stuetz.punkt << " " << stuetz.wert <<  endl;
    }
  }
  fct_file << "#STOP";
}


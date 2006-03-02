/**************************************************************************/
/* ROCKFLOW - Modul: files1.c
                                      */
/* Aufgabe:
   Enthaelt die uebergeordneten Datei- Ein- und Ausgaberoutinen, die
   die Text-Eingabedatei betreffen.
                                      */
/* Programmaenderungen:
   07/1995    MSR          Erste Version
   06/1997    C.Thorenz    Anpassung an RF-Konzept (11.6.97)
   07/1997    R.Kaiser     Korrekturen und Aenderungen aus dem aTM uebertragen
   08/1997    AH           Neue Steuerwoerter eingefuegt (Nichtlineares GLS + Grafik-Teil)
   09/1997    O.Kolditz    Schluesselwort 1D_GROUNDWATERFLOW_ELEMENT
                           Schluesselwort FRACTURE_APERTURE_DISTRIBUTION
   09/1997    C.Thorenz    Schluesselwort IC_SATURATION, BC_SATURATION
   10/1997    AH           Schluesselwort BCURVE (Grafik)
   10/1997    C.Thorenz    SOLVER_FLOW, SOLVER_TRAN (Parameterreihenfolge)
                           ITERATION_FLOW, ITERATION_TRAN (Parameter fuer rel. Genauigkeit des CG-Loesers)
   10/1997    O.Kolditz    Schluesselwort XD_HEAT_TRANSPORT_ELEMENT
   02/1998    AH           Relaxationsfaktor, Loeser
   03/1998    C. Thorenz   Dichtemodell
   09/1997    C. Thorenz   Mehrphasenmodell
   07/1998    O.Kolditz    neue Schluesselwoerter fuer Version 3.2
   08/1998    C. Thorenz   Neue Schluesselwoerter
   11/1998    AH           IC, BC, SOURCE_SINK, APRIORI_REFINE, OUTPUT (Neues Konzept: OOP)
   12/1998    OK           headvorgabe
   12/1998    AH           Mehrkomponentenmodel
                           Zuordnung der Lesefunktionen zum zugehoerigen
                           Objekt (OOP) -> Lesefunktionen objektorientiert
   02/1999    AH           Aktualisierung Quellen/Senken
   02/1999    CT           Neue Schluesselworte fuer Mehrphasen-Quellen
   03/1999    OK           Kurzprotokolle
   04/1999    AH           SOLVER (OOP) neues Konzept und zugehoerige Schluessewoerter.
   05/1999    AH           Neue Schluesselwort fuer Nichtgleichgewicht-Modell
   05/1999    AH           Schluesselwort OUTPUT_BTC in OUTPUT_EX umbennant.
   09/1999    AH           Neue Schluesselworte fuer Iterationsobjekte: ITERATION_PROPERTIES
   11/1999    RK           Neue Schluesselworte fuer Gitteradaption
   12/1999    AH           Neue Schluesselwoerte lineare Loesereigenschaften
                           bzgl. der sorbierten Konzentrationen.
                           FctLinearSolverPropertiesSorbedConcentrationNew
   02/2000    CT           Neues Schluesselwort "BC_FREE_OUTFLOW"
   03/2000    RK           Neues Schluesselwort "ADAPTATION_TEMPERATURE"
   06/2000    AH           Neue Schluesselwoerter fuer Mobil-Immobil-Konzept
                           - LINEAR_SOLVER_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION
                           - ITERATION_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION
   07/2000    AH           Vorbereitungen zum HGM.
   11/2000    CT           Neues Schluesselwort "NUMERICS_TEMPERATURE"
   10/2001    RK           Neues Schluesselwort "ADAPTATION_VOLT"
   10/2001    AH           - Neues Schluesselwort "MODEL_PROPERTIES"
                           - Neues Konzept fuer alle Eingaben (RFDB, RFIDB)
                             und Neue Schluesselwoerter fuer Inverses Modellieren.
   01/2002    MK           DM-3D-Unbekannte 
   03/2003    RK           Quellcode bereinigt, Globalvariablen entfernt   
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "files.h"
/* Objects */
#include "nodes.h"
#include "solver.h"
#include "gridadap.h"
#include "tools.h"
#include "rfiter.h"
#include "balance.h"
#include "rfdb.h"
#include "rfidb.h"
#include "rf_pcs.h"
#include "rfod.h"
#include "rfiv.h"
#include "rfii.h"
#include "rfim.h"
/* Tools */
#include "testvar.h"
#include "rfpriref.h"
#include "plot.h"
#include "rf_mg.h"
#include "rf_db.h"
/* HGM */
#include "rfhgm.h"

/* GeoLib */
#include "geo_pnt.h"
#include "geo_ply.h"
#include "geo_sfc.h"
#include "geo_vol.h"
#include "geo_strings.h"

extern int FctProject ( char *data, int found, FILE *f );

/**************************************************************************/
/* ROCKFLOW - Funktion: ReadTextInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   07/1995     MSR        Erste Version
                                      */
/**************************************************************************/
int ReadTextInputFile ( char *indat, FILE *prot )
{
/**************************************************************************/
/* 1 Datenbank anlegen */
  int ok = 1;
  int entries = 0;
  /* Zeiger auf Eingabedatenbank erzeugen (1024 Eintraege) */
  EingabeDatenbank *base = CreateTextInputDataBase(1024);
  /*--------------------------------------------------------------------------*/
  /* 1 Simulation Control */
  SetDataBaseSection(base,entries++,"PROJECT",FctProject,0,0);
  /*--------------------------------------------------------------------------*/
  /* 2 Numerical Methods */
  /*--------------------------------------------------------------------------*/
  /* 4 Solver */
  SetDataBaseSection(base,entries++,"RENUMBER",FctRenumber,0,1);
    /* 4.3 Iteration Process */
  SetDataBaseSection(base,entries++,"ITERATION_PROPERTIES_PRESSURE",FctIterationPropertiesPressureNew,1,1);
  SetDataBaseSection(base,entries++,"ITERATION_PROPERTIES_SATURATION",FctIterationPropertiesSaturationNew,1,1);
  SetDataBaseSection(base,entries++,"ITERATION_PROPERTIES_CONCENTRATION",FctIterationPropertiesConcentrationNew,1,1);
  SetDataBaseSection(base,entries++,"ITERATION_PROPERTIES_SORBED_CONCENTRATION",FctIterationPropertiesSorbedConcentrationNew,1,1);
  SetDataBaseSection(base,entries++,"ITERATION_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION",FctIterationPropertiesImmobileSolutedConcentrationNew,1,1);
  /*--------------------------------------------------------------------------*/
  /* 7 Materials */
  /*--------------------------------------------------------------------------*/
  /* 8 Adaptation */
  SetDataBaseSection(base,entries++,"ADAPTATION",FctAdaptation,0,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_PRESSURE",FctAdaptationPressure,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_TEMPERATURE",FctAdaptationTemperature,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_COMPONENT",FctAdaptationComponent,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_SATURATION",FctAdaptationSaturation,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_SORBED_COMPONENT",FctAdaptationSorbedComponent,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_SOLUTE_COMPONENT",FctAdaptationSoluteComponent,1,1);
  SetDataBaseSection(base,entries++,"ADAPTATION_VOLT",FctAdaptationVolt,1,1);  
  /*--------------------------------------------------------------------------*/
  /* 9 Functions */
  SetDataBaseSection(base,entries++,"CURVES",FctCurves,1,1); /* OK rf3246 */
  /*--------------------------------------------------------------------------*/
  /* 10 Miscellaneous */
  //OK SetDataBaseSection(base,entries++,"MESH_GENERATION",FctMeshGeneration,0,1);
  SetDataBaseSection(base,entries++,"FRACTURE_APERTURE_DISTRIBUTION",FctFractureApertureDistribution,0,1);
  SetDataBaseSection(base,entries++,"APRIORI_REFINE_ELEMENT",FctAprioriRefineElement,0,1);
  /*--------------------------------------------------------------------------*/
  /* News */
#ifdef __RFGRAF
  SetDataBaseSection(base,entries++,"GRAF",FctGraf,0,1);
  SetDataBaseSection(base,entries++,"MESH2D",FctMesh2D,0,1);
  SetDataBaseSection(base,entries++,"MESH3D",FctMesh3D,0,1);
  SetDataBaseSection(base,entries++,"GRAF_EXTD",FctGrafExtended,0,1);
#endif
/* 2 RFD-Datei einlesen */
  if (!ReadDataBaseData(base,indat,entries)) {
      DisplayErrorMsg("Fehler beim Einlesen (ReadDataBaseData) !");
      /* return 0; */ ok = 0;
  }
/**************************************************************************/
/* 3 Eingelesene Zeichenketten interpretieren und umspeichern */
  ok = ExecuteDataBase(base,entries,prot);
/**************************************************************************/
/* 4 Eingabedatenbank vernichten */
  DestroyTextInputDataBase(base,entries);
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReadMGTextInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Netz-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   07/2000    AH      Erste Version
                                      */
/**************************************************************************/
int ReadMGTextInputFile ( char *indat, FILE *prot )
{

/**************************************************************************/
/* 1 Datenbank anlegen */
  int ok = 1, entries = 0;

  /* Zeiger auf Eingabedatenbank mit 1024 Eintraegen erzeugen */
  EingabeDatenbank *base = CreateTextInputDataBase(1024);

  /*--------------------------------------------------------------------------*/
  /* 1 Mesh elements */
  /* D1.1 Mesh Points */
  SetDataBaseSection(base,entries++,"POINT",FctHGMPoint,1,1);
  /* D1.2 Mesh Edges */
  SetDataBaseSection(base,entries++,"EDGE",FctHGMEdge,1,1);
  /* D1.3 Mesh Plain */
  SetDataBaseSection(base,entries++,"PLAIN",FctHGMPlain,1,1);
  /* D1.4 Mesh Polygon */
  SetDataBaseSection(base,entries++,"POLYGON",FctHGMPolygon,1,1);
  /* D1.5 Mesh Polygon */
  /* GeoLib - Points */
  //SetDataBaseSection(base,entries++,"POINTS",GEOReadPoints,1,1);
  /* GeoLib - Polylines */
//  SetDataBaseSection(base,entries++,"POLYLINE",GEOReadPolylineNew,1,1);
  /* GeoLib - Surface */
  /* GeoLib - Volume */
  /* 2 Mesh Intersection */
  /* D2.1 Intersection Method */
  SetDataBaseSection(base,entries++,"INTERSECTION",FctHGMIntersection,1,1);

  /* 3 Mesh generator */
  /* D3.1 Delaunay */
  SetDataBaseSection(base,entries++,"DELAUNAY",FctHGMDelaunay,1,1);

  /* D3.2 Advancing Front Method */
  SetDataBaseSection(base,entries++,"ADVANCING_FRONT",FctHGMAdvancingFront,1,1);



/* 2 RFG-Datei einlesen */
  if (!ReadDataBaseData(base,indat,entries)) {
      DisplayErrorMsg("Fehler beim Einlesen (ReadDataBaseData) !");
      return 0;
  }

/**************************************************************************/
/* 3 Eingelesene Zeichenketten interpretieren und umspeichern */
  ok = ExecuteDataBase(base,entries,prot);

/**************************************************************************/
/* 4 Eingabedatenbank vernichten */
  DestroyTextInputDataBase(base,entries);

  return ok;
}



/**************************************************************************
   ROCKFLOW - Funktion: SetTextInputFile
                                                                         
   Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                                                       
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      
   Ergebnis:
   0 bei Fehler, sonst 1
                                      
   Programmaenderungen:
   10/2001    AH      Erste Version  nach neuem Datenbank-Konzept.
                      Fuer die vorigen Versionen und Einbau der
                      Steuerworte siehe bitte oben im Header der Datei.

                                      
**************************************************************************/
int SetTextInputFile(char *db_name)
{
/**************************************************************************/
/* 1 Datenbank anlegen */
  char sep_string[]="#";
  char com[]="//";  /* Kommentar-Zeichen */
  char beg_com[]="/*";  /* Blockkommentar-Beginn-Zeichen */
  char end_com[]="*/";     /* Blockkommentar-End-Zeichen */
  char beg_sec[]="{";  /* Abschnitt-Beginn-Zeichen */
  char end_sec[]="}";     /* Abschnitt-End-Zeichen */
  char sep_cap[]="#";  /* Trennzeichen fuer Kapiteln */

  /* Abschnittseigenschaften setzen */
  SetSectionPropertiesDataBase(sep_string,com,beg_com,end_com,beg_sec,end_sec,sep_cap);
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");

  /* Abschnittseigenschaften setzen */
  /* sep_string, com, beg_com, end_com, beg_sec,end_sec */
  SetSectionPropertiesDataBase(" ", ";", "/*", "*/", "{", "}", "#");
  /*--------------------------------------------------------------------------*/
  /* 1 Simulation Control */
  SetSectionInputDataBase(db_name,"PROJECT",FctProject);
  /* Rockflow-Model */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  /*--------------------------------------------------------------------------*/
  /* 2 Numerical Methods */
  /*--------------------------------------------------------------------------*/
  /* 4 Solver */
  SetSectionPropertiesDataBase(" ", ";", "/*", "/*", "{", "}", "#");
  SetSectionInputDataBase(db_name,"RENUMBER",FctRenumber);
    /* 4.3 Iteration Process */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,"ITERATION_PROPERTIES_PRESSURE",FctIterationPropertiesPressureNew);     
  SetSectionInputDataBase(db_name,"ITERATION_PROPERTIES_SATURATION",FctIterationPropertiesSaturationNew); 
  SetSectionInputDataBase(db_name,"ITERATION_PROPERTIES_CONCENTRATION",FctIterationPropertiesConcentrationNew);   
  SetSectionInputDataBase(db_name,"ITERATION_PROPERTIES_SORBED_CONCENTRATION",FctIterationPropertiesSorbedConcentrationNew);   
  SetSectionInputDataBase(db_name,"ITERATION_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION",FctIterationPropertiesImmobileSolutedConcentrationNew);   
  /*--------------------------------------------------------------------------*/
  /* 7 Materials */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  /*--------------------------------------------------------------------------*/
  /* 8 Adaptation */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,"ADAPTATION",FctAdaptation);
  SetSectionInputDataBase(db_name,"ADAPTATION_PRESSURE",FctAdaptationPressure);
  SetSectionInputDataBase(db_name,"ADAPTATION_TEMPERATURE",FctAdaptationTemperature);
  SetSectionInputDataBase(db_name,"ADAPTATION_COMPONENT",FctAdaptationComponent);
  SetSectionInputDataBase(db_name,"ADAPTATION_SATURATION",FctAdaptationSaturation);
  SetSectionInputDataBase(db_name,"ADAPTATION_SORBED_COMPONENT",FctAdaptationSorbedComponent);
  SetSectionInputDataBase(db_name,"ADAPTATION_SOLUTE_COMPONENT",FctAdaptationSoluteComponent);
  SetSectionInputDataBase(db_name,"ADAPTATION_VOLT",FctAdaptationVolt);
  /*--------------------------------------------------------------------------*/
  /* 9 Functions */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,"CURVES",FctCurves);


  /*--------------------------------------------------------------------------*/
  /* 10 Miscellaneous */
  SetSectionPropertiesDataBase("#", ";", "/*", "/*", "{", "}", "#");
  SetSectionInputDataBase(db_name,"MESH_GENERATION",FctMeshGeneration);
  SetSectionPropertiesDataBase(" ", ";", "/*", "/*", "{", "}", "#");
  SetSectionInputDataBase(db_name,"FRACTURE_APERTURE_DISTRIBUTION",FctFractureApertureDistribution);
  SetSectionPropertiesDataBase("#", ";", "/*", "/*", "{", "}", "#");
  SetSectionInputDataBase(db_name,"APRIORI_REFINE_ELEMENT",FctAprioriRefineElement);

  /*--------------------------------------------------------------------------*/
  /* 11 Grafics */
  SetSectionPropertiesDataBase(" ", ";", "/*", "/*", "{", "}", "#");
  SetSectionInputDataBase(db_name,"BCURVE",FctBcurve);


#ifdef __RFGRAF
  SetSectionPropertiesDataBase(" ", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,"GRAF",FctGraf);
  SetSectionInputDataBase(db_name,"MESH2D",FctMesh2D);
  SetSectionInputDataBase(db_name,"MESH3D",FctMesh3D);
  SetSectionInputDataBase(db_name,"GRAF_EXTD",FctGrafExtended);
#endif

  /* Ende */
  SetSectionInputDataBase(db_name,"STOP",NULL);

  return 1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetMGTextInputFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Netz-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   10/2001    AH      Neues Datenbank-Konzept
                                      */
/**************************************************************************/
int SetMGTextInputFile (char *db_name)
{

/**************************************************************************/
/* 1 Datenbank anlegen */
  char sep_string[]="#";
  char com[]="//";  /* Kommentar-Zeichen */
  char beg_com[]="/*";  /* Blockkommentar-Beginn-Zeichen */
  char end_com[]="*/";     /* Blockkommentar-End-Zeichen */
  char beg_sec[]="{";  /* Abschnitt-Beginn-Zeichen */
  char end_sec[]="}";     /* Abschnitt-End-Zeichen */
  char sep_cap[]="#";  /* Trennzeichen fuer Kapiteln */

  /* Abschnittseigenschaften setzen */
  SetSectionPropertiesDataBase(sep_string,com,beg_com,end_com,beg_sec,end_sec,sep_cap);
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");

  /* Abschnittseigenschaften setzen */
  /* sep_string, com, beg_com, end_com, beg_sec,end_sec */
  SetSectionPropertiesDataBase(" ", ";", "/*", "*/", "{", "}", "#");


  /*--------------------------------------------------------------------------*/
  /* 1 Mesh elements */
  /* D1.1 Mesh Points */
  SetSectionInputDataBase(db_name,"POINT",FctHGMPoint);
  /* D1.2 Mesh Edges */
  SetSectionInputDataBase(db_name,"EDGE",FctHGMEdge);
  /* D1.3 Mesh Plain */
  SetSectionInputDataBase(db_name,"PLAIN",FctHGMPlain);
  /* D1.4 Mesh Polygon */
  SetSectionInputDataBase(db_name,"POLYGON",FctHGMPolygon);
  /* D1.5 Mesh Polygon */
  SetSectionInputDataBase(db_name,"POLYLINE",FctHGMPolyline);
//  SetSectionInputDataBase(db_name,"POLYLINE",GEOReadPolylineNew);

  /* 2 Mesh Intersection */
  /* D2.1 Intersection Method */
  SetSectionInputDataBase(db_name,"INTERSECTION",FctHGMIntersection);

  /* 3 Mesh generator */
  /* D3.1 Delaunay */
  SetSectionInputDataBase(db_name,"DELAUNAY",FctHGMDelaunay);

  /* D3.2 Advancing Front Method */
  SetSectionInputDataBase(db_name,"ADVANCING_FRONT",FctHGMAdvancingFront);

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: SetTextInverseFile
                                                                          */
/* Aufgabe:
   Liest Daten aus der Text-Eingabedatei ein und schreibt Protokolldatei
                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *indat: Dateiname der Eingabedatei incl. Extension
   E FILE *prot: Dateizeiger auf geoeffnete Ausgabedatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   10/2001    AH      Erste Version (Inverses Modellieren)
                                      */
/**************************************************************************/
int SetTextInverseFile (char *db_name)
{
  int ok = 1;

/**************************************************************************/
/* Datenbank anlegen */

  /*--------------------------------------------------------------------------*/
  /* Inverse Methode */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,keyword_inverse_method,FctInverseMethod);

  /*--------------------------------------------------------------------------*/
  /* Inverse Variable */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,keyword_inverse_variable,FctInverseVariableNew);

  /*--------------------------------------------------------------------------*/
  /* Observation Data */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,keyword_observation_data,FctObservationData);

  /*--------------------------------------------------------------------------*/
  /* Inverse Iteration */
  SetSectionPropertiesDataBase("#", ";", "/*", "*/", "{", "}", "#");
  SetSectionInputDataBase(db_name,keyword_inverse_iteration,FctInverseIteration);


  /* Stop */
  SetSectionInputDataBase(db_name,"STOP",NULL);

  return ok;
}

/**************************************************************************
   ROCKFLOW - Funktion: FctProject
 
   Aufgabe:
   Liest die zu dem Schluesselwort PROJECT gehoerigen Daten ein und
   erstellt den zugehoerigen Protokollabschnitt.
   PROJECT: Muss nur vorhanden sein; es steht nur ein # in *data.
 
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
 
   Ergebnis:
   0 bei Fehler, sonst 1
 
   Programmaenderungen:
   07/1996   RK   Erste Version
   12/1999   OK   Protokoll (found==2)
   10/2001   AH   CreateProjectName verlegt  ah rfm
 
**************************************************************************/
int FctProject(char *data, int found, FILE * f)
{
  char project_title[MAX_ZEILE];
#ifdef TESTFILES
  DisplayMsgLn("Eingabedatenbank, Schluesselwort PROJECT");
#endif

  LineFeed(f);
  FilePrintString(f, "; 1 Simulation Control -------------------------------------------------");
  LineFeed(f);

  if (!found)
    {                                   /* PROJECT nicht gefunden */
      strcpy(project_title, "Insert your project title");
      FilePrintString(f, "* ! Projektkennung PROJECT nicht gefunden, Korrektur");
      LineFeed(f);
      FilePrintString(f, "#PROJECT");
      LineFeed(f);
      FilePrintString(f, project_title);
      LineFeed(f);
    }
  else
    {                                  /* PROJECT gefunden */
      if ((int)strlen(data) > 250)
        {
          FilePrintString(f, "* !!! Fehler: Projekttitel zu lang (mehr als 250 Zeichen) !");
          LineFeed(f);
          return 0;
        }
     
      strcpy(project_title, data); 
      FilePrintString(f, "#PROJECT");
      LineFeed(f);
      FilePrintString(f, project_title);
      LineFeed(f);
    }

#ifdef EXT_RFD
  FilePrintString(f, "; Schluesselwort: #PROJECT (Projektkennung)");
  LineFeed(f);
  FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein; falls es vorhanden ist, wird");
  LineFeed(f);
  FilePrintString(f, "; der gesamte folgende Text (ausser Kommentaren) bis zum naechsten");
  LineFeed(f);
  FilePrintString(f, "; Schluesselwort in die Protokolldatei uebernommen.");
  LineFeed(f);
#endif

  return 1;
}


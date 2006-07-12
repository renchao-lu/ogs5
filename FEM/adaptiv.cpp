/**************************************************************************/
/* ROCKFLOW - Modul: adaptiv.c
                                                                          */
/* Aufgabe:
                                                                          */
/* Programmaenderungen:
   04/1999     RK     Erste Version
   02/2000     RK     Adaption "Temperatur"
   07/2000     RK     Ueberarbeitet
   07/2000     RK     gridadap.* <-> adaptiv.*
   10/2000     CT     Warnungen beseitigt
   10/2000     CT     Mehr Platz in index_comp_conc
   01/2001     RK     Neue Methoden (5 und 6) für Gitteradaption
   10/2001     RK     Adaption "Volt" (Geoelektrik)
   03/2003     RK     Quellcode bereinigt

                                                                          */
/**************************************************************************/

#define TEST_ADAPT   

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "adaptiv.h"
#include "rf_pcs.h" //OK_MOD" 
#include "tools.h"
#include "rf_tim_new.h"

/* Used tools */
#include "geo_strings.h"

/* Interne (statische) Deklarationen */
static int method_adaptation;         /* Methode/Gitteradaption */

static int adaptation_timestep;
static int method_irr_nodes;          /* Methode Irregulaere Knoten
                                         1: Eliminierung in Elementmatrizen
                                         2: Eliminierung im Gesamtgleichungssystem */
static int timestep_ref_number;       /* Anzahl Verfeinerungen pro Zeitschritt */
static int max_ref_level;             /* Maximaler Verfeinerungsgrad */
static int ref_neighbours_2d;         /* Anzahl der feineren 2d-Nachbarn,
                                         ab der ein 2d-Element verfeinert wird */
static int ref_neighbours_3d;         /* Anzahl der feineren 3d-Nachbarn,
                                         ab der ein 3d-Element verfeinert wird */

static int method_node_limiter;       /* Methode Knoten-'Limiter' */
static int curve_node_limiter;        /* Zeitkurve fuer den Knoten-'Limiter' */
static long max_nodes;                /* Maximale Anzahl Knoten */
static int output_adaptation;         /* Ausgabe von max, min und Mittelwert fuer
                                         jeden Indikator (in jedem Zeitschritt)
                                         0: keine Ausgabe
                                         1: Ausgabe */


typedef struct {
  int ref_ind[3][2];                  /* Nummern der Verfeinerungsindikatoren
                                         fuer die 3 Elementdimensionen
                                         (Druck, Saettigung);
                                         Nummern der advektiven und diffusiven
                                         Verfeinerungsindikatoren fuer die
                                         3 Elementdimensionen (Transport) */
  int curve_ind[3][2][2];             /* Zeitkurven fuer die Indikatoren */

  DoubleFuncLIII Ind[3][2];           /* Funktionszeiger auf die
                                         Verfeinerungsindikatoren */
  double ref_param[3][2][3];          /* Parameter fuer die
                                         Verfeinerungsindikatoren */
  int ref_tol;                        /* Toleranz-Parameter:
                                         0-absolut
                                         1-Mittelwert
                                         2-Maximalwert */
} IndData;


typedef struct {
  int ref_quantity;                   /* Groesse, fuer die verfeinert werden soll:
                                         0: Druck
                                         1: Transport (component)
                                         2: Temperatur
                                         3: Saettigung
                                         4: sorbed_component
                                         5: solute_component 
                                         6: Volt */
                                         

  int method_ind;
  int quantity_max_ref_level[3];      /* Verfeinerungslevel (1d,2d,3d) */
  int coarsement_correction;          /* 'Massenerhaltung' beim Vergroebern */


  long number_ind;                    /* Anzahl Verfeinerungsindikatoren */
  IndData *ind_data;
  int number_phase_component;         /* Nummer der Phase (Druck, Saettigung) oder
                                         Komponente (Transport) */
  int ref_nval0;                      /* Knotenwert-Indizes fuer Adaption */
  int ref_nval1;
} AdaptData;

static AdaptData *adapt_data;
static long number_adapt_quantities;
static long total_number_ind;
static int ref_iter;
static int actual_ref_level;          /* tatsaechlich vorhandener maximaler Verfeinerungsgrad */
static int grid_adaptation;
static int grid_topology_change;

static int *index_phase_press;
static int *index_temp;
static int *index_comp_conc;
static int *index_phase_satu;
static int *index_comp_sorbed_conc;
static int *index_comp_solute_conc;
static int *index_electric_field_volt;


static long *num_elems_ref_level_X;   /* Anzahl der Elemente auf den verschiedenen
                                         Verfeinerungsleveln */


IntFuncVoid Refine;

int TFAdaptMethod ( int *x, FILE *f );
int TFMaxRefLevel ( int *x, FILE *f );
int TFTimestepRefNumber ( int *x, FILE *f );
int TFRefNeighbours2d ( int *x, FILE *f );
int TFRefNeighbours3d ( int *x, FILE *f );
int TFQuantityMaxRefLevel ( int *x, FILE *f );
int TFCoarsementCorrect ( int *x, FILE *f );
int TFNumberInd ( long *x, FILE *f );
int TFRefTol ( int *x, FILE *f );

char name_adaptation_pressure[80] = "ADAPTATION_PRESSURE";
char name_adaptation_component[80] = "ADAPTATION_COMPONENT";
char name_adaptation_saturation[80] = "ADAPTATION_SATURATION";
char name_adaptation_temperature[80] = "ADAPTATION_TEMPERATURE";
char name_adaptation_solute_component[80] = "ADAPTATION_SOLUTE_COMPONENT";
char name_adaptation_sorbed_component[80] = "ADAPTATION_SORBED_COMPONENT";
char name_adaptation_volt[80] = "ADAPTATION_VOLT";

/* Element- und Rechenkern-Schnittstelle */
DoubleXFuncLong ADAPGetElementMassMatrix;
DoubleXFuncLong ADAPGetElementCapacitanceMatrix;
DoubleXFuncLong ADAPGetElementConductanceMatrix;
DoubleFuncLong ADAPGetElementPecletNum;
DoubleFuncLong ADAPGetElementDDummy;
VoidFuncLD ADAPSetElementDDummy;
DoubleXFuncLDX ADAPGetElementJacobi;
VoidFuncLong ADAPMakeMat;
VoidFuncLDXDX ADAPCalcTransMatrix;
DoubleFuncLong ADAPGetElementRetardFac;
DoubleXFuncLong ADAPGetElementDispMatrix;



/**************************************************************************/
/* ROCKFLOW - Funktion: CreateAdaptationData
                                                                          */
/* Aufgabe:
   Speicherbelegungen und Initialisierungen fuer Adaptions-Daten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000   RK   Erste Version
                                                                          */
/**************************************************************************/
int CreateAdaptationData(void)
{
    adapt_data = NULL;
    index_phase_press = NULL;
    index_phase_satu = NULL;
    index_temp = NULL;
    index_comp_conc = NULL;
    index_comp_sorbed_conc = NULL;
    index_comp_solute_conc = NULL;
    index_electric_field_volt = NULL;
       
    num_elems_ref_level_X = NULL;

    method_adaptation = 0;
    adaptation_timestep = 1;
    method_irr_nodes = 1;
    timestep_ref_number = 0;
    max_ref_level = 0;
    ref_neighbours_2d = 4;
    ref_neighbours_3d = 6;
    method_node_limiter = 0;
    curve_node_limiter = 0;
    max_nodes = 0l;
    output_adaptation = 0;
    number_adapt_quantities = 0l;
    total_number_ind = 0l;
    ref_iter = 0;
    actual_ref_level = 0;
    grid_adaptation = -1;
    grid_topology_change = 0;

    return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitGridAdaptation
                                                                          */
/* Aufgabe:
   Initialisiert Adaptive Variablen
   !!! max_ref_level muss belegt und die Elemente eingelesen sein !!!
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/1996     MSR        Erste Version
   04.08.1998  R.Kaiser   Adaption fuer mehrere Loesungsgroessen
   26.02.1999  C.Thorenz  Korrekturen fuer herausgeloeste Materialgruppen
   26.07.1999  R.Kaiser   InitAdaptiv -> InitGridAdaptation
                          Angepasst an neue Datenstrukturen
   26.10.2000  C.Thorenz  Mehr Platz in index_comp_conc

                                                                          */
/**************************************************************************/
void InitGridAdaptation(void)
{
  long i, j, jj, ii;

  if (GetRFControlGridAdapt()) {
    index_temp = (int *) Malloc(2 * sizeof(int));
    index_phase_press = (int *) Malloc(2 * GetRFProcessNumPhases() * sizeof(int));
    index_phase_satu = (int *) Malloc(2 * GetRFProcessNumPhases() * sizeof(int));
    index_comp_conc = (int *) Malloc(2 * GetRFProcessNumComponents() * GetRFProcessNumPhases() * sizeof(int));
    index_comp_sorbed_conc = (int *) Malloc(2 * GetRFProcessNumComponents() * sizeof(int));
    index_comp_solute_conc = (int *) Malloc(2 * GetRFProcessNumComponents() * sizeof(int));
    index_electric_field_volt = (int *) Malloc(2 * GetRFProcessNumElectricFields() * sizeof(int));
    
    num_elems_ref_level_X = (long *) Malloc(max_ref_level * sizeof(long));

    for (i = 0l; i < max_ref_level; i++)
      num_elems_ref_level_X[i] = 0l;
    anz_active_1D = 0l;
    anz_active_2D = 0l;
    anz_active_3D = 0l;
    j = ElListSize();

    for (i = 0; i < start_new_elems; i++) {
      if (ElGetElementActiveState(i)) {
        switch (ElGetElementType(i)) {
          case 1: anz_active_1D++;
                  break;
          case 2: anz_active_2D++;
                  break;
          case 3: anz_active_3D++;
                  break;
        }
      }
    }

    for (i = start_new_elems; i < j; i++) {
      if (ElGetElement(i) != NULL) {
        (num_elems_ref_level_X[ElGetElementLevel(i) - 1])++;
        if (ElGetElementActiveState(i)) {
          switch (ElGetElementType(i)) {
            case 1: anz_active_1D++;
                    break;
            case 2: anz_active_2D++;
                    break;
            case 3: anz_active_3D++;
                    break;
          }
        }
      }
    }

    anz_active_elements = anz_active_1D + anz_active_2D + anz_active_3D;
    BuildActiveElementsArray();


    if (number_adapt_quantities > 0)
      for (jj=0;jj<number_adapt_quantities;jj++) {

        /* fluid_phase */
        if (adapt_data[jj].ref_quantity == 0)
          for (ii=0;ii<adapt_data[jj].number_ind;ii++) {
            for (i=0;i<3;i++) {
              switch (adapt_data[jj].ind_data[ii].ref_ind[i][0]) {
                case 0: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator0; break;
                case 1: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator1; break;
                case 2: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator2; break;
                case 3: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator3; break;
                case 4: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator4; break;
                case 101: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator_flow1; break;
                case 102: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator_flow2; break;
                case 103: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator_flow3; break;
                case 104: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator_flow4; break;
              }
            }
          }

        /* tracer_component, temperature */
        if ((adapt_data[jj].ref_quantity == 1) || (adapt_data[jj].ref_quantity == 2))
          for (ii=0;ii<adapt_data[jj].number_ind;ii++) {
            for (i = 0; i < 3; i++)
              for (j = 0; j < 2; j++) {
                switch (adapt_data[jj].ind_data[ii].ref_ind[i][j]) {
                  case 0: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator0; break;
                  case 1: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator1; break;
                  case 2: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator2; break;
                  case 3: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator3; break;
                  case 4: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator4; break;
                  case 101: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator_tran1; break;
                  case 102: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator_tran2; break;
                  case 103: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator_tran3; break;
                }
              }
          }

        /* saturation */
        if (adapt_data[jj].ref_quantity == 3)
          for (ii=0;ii<adapt_data[jj].number_ind;ii++) {
            for (i=0;i<3;i++) {
              switch (adapt_data[jj].ind_data[ii].ref_ind[i][0]) {
                case 0: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator0; break;
                case 1: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator1; break;
                case 2: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator2; break;
                case 3: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator3; break;
                case 4: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator4; break;
              }
            }
          }

        /* sorbed_component, solute_component */
        if ((adapt_data[jj].ref_quantity == 4) || (adapt_data[jj].ref_quantity == 5))
          for (ii=0;ii<adapt_data[jj].number_ind;ii++) {
            for (i = 0; i < 3; i++)
              for (j = 0; j < 2; j++) {
                switch (adapt_data[jj].ind_data[ii].ref_ind[i][j]) {
                  case 0: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator0; break;
                  case 1: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator1; break;
                  case 2: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator2; break;
                  case 3: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator3; break;
                  case 4: adapt_data[jj].ind_data[ii].Ind[i][j] = Indicator4; break;
                }
              }
          }
          
        /* Volt */
        if (adapt_data[jj].ref_quantity == 6)
          for (ii=0;ii<adapt_data[jj].number_ind;ii++) {
            for (i=0;i<3;i++)
                switch (adapt_data[jj].ind_data[ii].ref_ind[i][0]) {
                  case 0: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator0; break;
                  case 1: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator1; break;
                  case 2: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator2; break;
                  case 3: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator3; break;
                  case 4: adapt_data[jj].ind_data[ii].Ind[i][0] = Indicator4; break;                 
                }
          }          
               
      }
  }
  else {
    anz_active_elements = anz_1D + anz_2D + anz_3D;
    BuildActiveElementsArray();
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigureGridAdaptation
                                                                          */
/* Aufgabe:
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   09/1999     R.Kaiser   Erste Version
   10/2000     C.Thorenz  Warnungen beseitigt
                                                                          */
/**************************************************************************/
void ConfigureGridAdaptation (void)
{
  int jj;
  int index0=-1, index1=-1;


  if (GetRFControlGridAdapt()) {

    if (GetRFControlGridAdapt() == 3)
      DisplayMsg("Konfiguration Gitteradaption");



    switch (max_dim) {
      case 0: Refine = NetRef1D_a; break;
      case 1: Refine = NetRef2D_a; break;
      case 2: Refine = NetRef3D_a; break;
    }



    /* Vorbereitungen fuer das adaptive Modell */
    /* Kanten- bzw. Flaechenverzeichnis aufbauen */
    ConstructRelationships();
    ConstructStatusEntries();




    if (number_adapt_quantities > 0) {
      for (jj=0;jj<number_adapt_quantities;jj++) {
        switch (adapt_data[jj].ref_quantity) {
          case 0: index0 = AdaptGetIndexPhasePress(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexPhasePress(adapt_data[jj].number_phase_component,1);
                  break;
          case 1: index0 = AdaptGetIndexComponentConc(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexComponentConc(adapt_data[jj].number_phase_component,1);
                  break;
          case 2: index0 = AdaptGetIndexTemp(0);
                  index1 = AdaptGetIndexTemp(1);
                  break;
          case 3: index0 = AdaptGetIndexPhaseSatu(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexPhaseSatu(adapt_data[jj].number_phase_component,1);
                  break;
          case 4: index0 = AdaptGetIndexComponentSorbedConc(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexComponentSorbedConc(adapt_data[jj].number_phase_component,1);
                  break;
          case 5: index0 = AdaptGetIndexComponentSoluteConc(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexComponentSoluteConc(adapt_data[jj].number_phase_component,1);
                  break;
          case 6: index0 = AdaptGetIndexElectricFieldVolt(adapt_data[jj].number_phase_component,0);
                  index1 = AdaptGetIndexElectricFieldVolt(adapt_data[jj].number_phase_component,1);
                  break;                                 
                 
        }
        adapt_data[jj].ref_nval0 = index0;
        adapt_data[jj].ref_nval1 = index1;
      }
    }

    if (GetRFControlGridAdapt() == 3) {
      DisplayMsg("Gitteradaption konfiguriert !!");
      DisplayMsgLn("");
    }
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyAdaptationData
                                                                          */
/* Aufgabe:
   Speicherfreigaben fuer Adaptionsdaten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2000   RK   Erste Version
   11/2001   AH   Speicherfreigabe ergaenzt
                                                                          */
/**************************************************************************/
void DestroyAdaptationData(void)
{
  int i;

  ActiveElements = (long *)Free(ActiveElements);
  num_elems_ref_level_X = (long *)Free(num_elems_ref_level_X);

  index_phase_press = (int *)Free(index_phase_press);
  index_temp = (int *)Free(index_temp);
  index_phase_satu = (int *)Free(index_phase_satu);
  index_comp_conc = (int *)Free(index_comp_conc);
  index_comp_sorbed_conc = (int *)Free(index_comp_sorbed_conc);
  index_comp_solute_conc = (int *)Free(index_comp_solute_conc);
  index_electric_field_volt = (int *)Free(index_electric_field_volt);

  for(i=0;i<number_adapt_quantities;i++) {
    if (adapt_data[i].ind_data)
      adapt_data[i].ind_data = (IndData *) Free(adapt_data[i].ind_data);
  }
  adapt_data = (AdaptData *) Free(adapt_data);


  anz_active_elements = 0l;
  anz_active_1D = 0l;
  anz_active_2D = 0l;
  anz_active_3D = 0l;

}




/**************************************************************************/
/* ROCKFLOW - Funktion: AdaptGet...
                                                                          */
/* Aufgabe:
   Zugriffsfunktionen auf die adaptiven Variablen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

                                                                          */
/* Ergebnis:

                                                                          */
/* Programmaenderungen:
   09/1999  R.Kaiser  Erste Version
                                                                          */
/**************************************************************************/

int AdaptGetMethodAdaptation ( void )
{
  if (method_adaptation == 5) {
    if (aktueller_zeitschritt == 1) 
      return 1;
    else
      return 2;  
   
  }  
  if (method_adaptation == 6) {
    if (aktueller_zeitschritt == 1) 
      return 3;
    else
      return 4;  
    
  }  

  return method_adaptation;
}


int AdaptGetAdaptationTimestep (void )
{
  return adaptation_timestep;
}


int AdaptGetTimestepRefNumber ( void )
{
  if (aktueller_zeitschritt == 1)
    return max_ref_level;

  return timestep_ref_number;
}

int AdaptGetMethodIrrNodes ( void )
{
  return method_irr_nodes;
}

int AdaptGetMaxRefLevel ( void )
{
  return max_ref_level;
}

int AdaptGetRefNeighbours2d ( void )
{
  return ref_neighbours_2d;
}

int AdaptGetRefNeighbours3d ( void )
{
  return ref_neighbours_3d;
}


int AdaptGetMethodNodeLimiter ( void )
{
  return method_node_limiter;
}


int AdaptGetCurveNodeLimiter ( void )
{
  return curve_node_limiter;
}

int AdaptGetOutputAdaptation ( void )
{
  return output_adaptation;
}

long AdaptGetMaxNodes ( void )
{
  return max_nodes;
}

long AdaptGetNumAdaptQuantities ( void )
{
  return number_adapt_quantities;
}


int AdaptGetRefQuantity ( int number )
{
  return adapt_data[number].ref_quantity;
}


int AdaptGetMethodInd ( int number )
{
  return adapt_data[number].method_ind;
}

int AdaptGetQuantityMaxRefLevel ( int number, int typ )
{
  return adapt_data[number].quantity_max_ref_level[typ];
}

int AdaptGetNumPhaseComponent ( int number )
{
  return adapt_data[number].number_phase_component;
}

long AdaptGetNumberInd ( int number )
{
  return adapt_data[number].number_ind;
}

int AdaptGetRefNval0 (int number)
{
  return adapt_data[number].ref_nval0;
}

int AdaptGetRefNval1 (int number)
{
  return adapt_data[number].ref_nval1;
}

int AdaptGetCoarseCorrect (int num_ref_quantity)
{
  return adapt_data[num_ref_quantity].coarsement_correction;
}


double AdaptGetIndValue (int typ, int AD, long element, int num_ref_quantity, int num_ind)
{
  return adapt_data[num_ref_quantity].ind_data[num_ind].Ind[typ][AD](element,typ,
                                            adapt_data[num_ref_quantity].ref_nval0,
                                            adapt_data[num_ref_quantity].ref_nval1);
}

int AdaptGetRefTol (int num_ref_quantity, int num_ind)
{
  return adapt_data[num_ref_quantity].ind_data[num_ind].ref_tol;
}


int AdaptGetRefInd (int num_ref_quantity, int num_ind, int typ, int AD)
{
  return adapt_data[num_ref_quantity].ind_data[num_ind].ref_ind[typ][AD];
}


int AdaptGetCurveInd (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3)
{
  return adapt_data[num_ref_quantity].ind_data[num_ind].curve_ind[idx1][idx2][idx3];
}

double AdaptGetRefParam (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3)
{
  return adapt_data[num_ref_quantity].ind_data[num_ind].ref_param[idx1][idx2][idx3];
}

int AdaptGetRefIter ( void )
{
  return ref_iter;
}

int AdaptGetGridChange ( void )
{
  return grid_topology_change;
}

long AdaptGetTotalNumInd ( void )
{
  return total_number_ind;
}


long AdaptGetNumEleRefLevelX ( int level_x)
{
  return num_elems_ref_level_X[level_x];
}


int AdaptGetActualRefLevel ( void )
{
  return actual_ref_level;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: AdaptSet...
                                                                          */
/* Aufgabe:
   Setzt adaptive Variablen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

                                                                          */
/* Ergebnis:

                                                                          */
/* Programmaenderungen:
   09/1999  R.Kaiser  Erste Version
                                                                          */
/**************************************************************************/

void AdaptSetRefNval0 (int number, int ref_nval0)
{
  adapt_data[number].ref_nval0 = ref_nval0;
}

void AdaptSetRefNval1 (int number, int ref_nval1)
{
  adapt_data[number].ref_nval1 = ref_nval1;
}

void AdaptSetRefParam (int num_ref_quantity, int num_ind, int idx1, int idx2, int idx3, double param)
{
  adapt_data[num_ref_quantity].ind_data[num_ind].ref_param[idx1][idx2][idx3] = param;
}

void AdaptSetRefIter ( int iteration)
{
  ref_iter = iteration;
}

void AdaptSetGridChange ( int grid_topo_change)
{
  grid_topology_change = grid_topo_change;
}


void AdaptSetActualRefLevel ( int level )
{
  actual_ref_level = level;
}

void AdaptSetNumEleRefLevelX ( int level_x, long num_elems )
{
  num_elems_ref_level_X[level_x] = num_elems;
}

void AdaptSetMethodAdaptation(int ivalue)
{
    method_adaptation = ivalue;
}

void AdaptSetMethodIrrNodes (int ivalue)
{
  method_irr_nodes = ivalue;
}

void AdaptSetMaxRefLevel(int ivalue)
{
    max_ref_level = ivalue;
}

void AdaptSetTimestepRefNumber(int ivalue)
{
    timestep_ref_number = ivalue;
}

void AdaptSetRefNeighbours2d(int ivalue)
{
    ref_neighbours_2d = ivalue;
}

void AdaptSetRefNeighbours3d(int ivalue)
{
    ref_neighbours_3d = ivalue;
}

void AdaptSetRefInd (int num_ref_quantity, int num_ind, int typ, int AD, int ivalue)
{
  adapt_data[num_ref_quantity].ind_data[num_ind].ref_ind[typ][AD] = ivalue;
}

void AdaptSetCoarseCorrect (int num_ref_quantity, int ivalue)
{
  adapt_data[num_ref_quantity].coarsement_correction = ivalue;
}

void AdaptSetQuantityMaxRefLevel (int number, int typ, int ivalue)
{
  adapt_data[number].quantity_max_ref_level[typ] = ivalue;
}


/**************************************************************************/
/* ROCKFLOW - Funktion:
                                                                          */
/* Aufgabe:

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

                                                                          */
/* Ergebnis:

                                                                          */
/* Programmaenderungen:
   09/1999  R.Kaiser  Erste Version
                                                                          */
/**************************************************************************/

int AdaptGetIndexPhasePress (int phase, int timelevel)
{
 if (timelevel == 0)
   return index_phase_press[phase];
 if (timelevel == 1)
   return index_phase_press[GetRFProcessNumPhases() + phase];
 return -1;
}

void AdaptSetIndexPhasePress (int phase, int timelevel, int index)
{
  if (timelevel == 0)
    index_phase_press[phase] = index;
  if (timelevel == 1)
    index_phase_press[GetRFProcessNumPhases() + phase] = index;
}

int AdaptGetIndexComponentConc (int component, int timelevel)
{
 if (timelevel == 0)
   return index_comp_conc[component];
 if (timelevel == 1)
   return index_comp_conc[GetRFProcessNumComponents() + component];
 return -1;
}

void AdaptSetIndexComponentConc (int component, int timelevel, int index)
{
  if (timelevel == 0)
    index_comp_conc[component] = index;
  if (timelevel == 1)
    index_comp_conc[GetRFProcessNumComponents() + component] = index;
}

int AdaptGetIndexTemp (int timelevel)
{
 if (timelevel == 0)
   return index_temp[0];
 if (timelevel == 1)
   return index_temp[1];
 return -1;
}

void AdaptSetIndexTemp (int timelevel, int index)
{
  if (timelevel == 0)
    index_temp[0] = index;
  if (timelevel == 1)
    index_temp[1] = index;
}


int AdaptGetIndexPhaseSatu (int phase, int timelevel)
{
 if (timelevel == 0)
   return index_phase_satu[phase];
 if (timelevel == 1)
   return index_phase_satu[GetRFProcessNumPhases() + phase];
 return -1;
}

void AdaptSetIndexPhaseSatu (int phase, int timelevel, int index)
{
  if (timelevel == 0)
    index_phase_satu[phase] = index;
  if (timelevel == 1)
    index_phase_satu[GetRFProcessNumPhases() + phase] = index;
}

int AdaptGetIndexComponentSorbedConc (int component, int timelevel)
{
 if (timelevel == 0)
   return index_comp_sorbed_conc[component];
 if (timelevel == 1)
   return index_comp_sorbed_conc[GetRFProcessNumComponents() + component];
 return -1;
}

void AdaptSetIndexComponentSorbedConc (int component, int timelevel, int index)
{
  if (timelevel == 0)
    index_comp_sorbed_conc[component] = index;
  if (timelevel == 1)
    index_comp_sorbed_conc[GetRFProcessNumComponents() + component] = index;
}

int AdaptGetIndexComponentSoluteConc (int component, int timelevel)
{
 if (timelevel == 0)
   return index_comp_solute_conc[component];
 if (timelevel == 1)
   return index_comp_solute_conc[GetRFProcessNumComponents() + component];
 return -1;
}

void AdaptSetIndexComponentSoluteConc (int component, int timelevel, int index)
{
  if (timelevel == 0)
    index_comp_solute_conc[component] = index;
  if (timelevel == 1)
    index_comp_solute_conc[GetRFProcessNumComponents() + component] = index;
}

int AdaptGetIndexElectricFieldVolt (int electric_field, int timelevel)
{
 if (timelevel == 0)
   return index_electric_field_volt[electric_field];
 if (timelevel == 1)
   return index_electric_field_volt[GetRFProcessNumElectricFields() + electric_field];
 return -1;
}

void AdaptSetIndexElectricFieldVolt (int electric_field, int timelevel, int index)
{
  if (timelevel == 0)
    index_electric_field_volt[electric_field] = index;
  if (timelevel == 1)
    index_electric_field_volt[GetRFProcessNumElectricFields() + electric_field] = index;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptation
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION gehoerigen Daten ein und
   erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version

                                                                          */
/**************************************************************************/
int FctAdaptation(char *data, int found, FILE * f)
{
  int ok = 1;
  int pos = 0;
  int p = 0;


  LineFeed(f);
  FilePrintString(f, "; 8 Adaptation ---------------------------------------------------------");
  LineFeed(f);

#ifdef TESTFILES
  DisplayMsgLn("Eingabedatenbank, Schluesselwort #ADAPTATION");
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    grid_adaptation = 0; /* keine Gitteradaption !!! */
#ifdef EXT_RFD
    FilePrintString(f, "; Schluesselwort: #ADAPTATION");
    LineFeed(f);
#endif
    }
  else {
    /* ADAPTATION gefunden */
    grid_adaptation = 1;  /* Gitteradaption */
    FilePrintString(f, "#ADAPTATION");
    LineFeed(f);
#ifdef EXT_RFD
    FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein.");
    LineFeed(f);
    FilePrintString(f, "; Es wird dann ohne Netzadaption gerechnet.");
    LineFeed(f);
#endif
    if (found == 1) {
      ok = (StrReadInt(&method_adaptation,&data[p],f,TFInt,&pos) && ok);
      if ((method_adaptation == 3) || (method_adaptation == 4) || (method_adaptation == 6))
        ok = (StrReadInt(&adaptation_timestep,&data[p+=pos],f,TFInt,&pos) && ok);    
      ok = (StrReadInt(&max_ref_level,&data[p+=pos],f,TFMaxRefLevel,&pos) && ok);
      ok = (StrReadInt(&timestep_ref_number,&data[p+=pos],f,TFTimestepRefNumber,&pos) && ok);
      ok = (StrReadInt(&method_irr_nodes,&data[p+=pos],f,TFInt,&pos) && ok);
      ok = (StrReadInt(&ref_neighbours_2d,&data[p+=pos],f,TFRefNeighbours2d,&pos) && ok);
      ok = (StrReadInt(&ref_neighbours_3d,&data[p+=pos],f,TFRefNeighbours3d,&pos) && ok);
      ok = (StrReadInt(&method_node_limiter,&data[p+=pos],f,TFInt,&pos) && ok);
      if (method_node_limiter == 1) {
        ok = (StrReadInt(&curve_node_limiter,&data[p+=pos],f,TFInt,&pos) && ok);
        ok = (StrReadLong(&max_nodes,&data[p+=pos],f,TFLong,&pos) && ok);
      }
      if (method_node_limiter == 2) {
        ok = (StrReadInt(&curve_node_limiter,&data[p+=pos],f,TFInt,&pos) && ok);
        ok = (StrReadLong(&max_nodes,&data[p+=pos],f,TFLong,&pos) && ok);
      }
      ok = (StrReadInt(&output_adaptation,&data[p+=pos],f,TFInt,&pos) && ok);
    }
    else if (found == 2) {
      fprintf(f, " %d ", method_adaptation);
      if ((method_adaptation == 3) || (method_adaptation == 4) || (method_adaptation == 6))
        fprintf(f, " %d ", adaptation_timestep);      
      fprintf(f, " %d ", max_ref_level);
      fprintf(f, " %d ", timestep_ref_number);
      fprintf(f, " %d ", method_irr_nodes);
      fprintf(f, " %d ", ref_neighbours_2d);
      fprintf(f, " %d ", ref_neighbours_3d);
      fprintf(f, " %d ", method_node_limiter);  
      fprintf(f, " %d ", curve_node_limiter); 
      fprintf(f, " %ld ",max_nodes);
      fprintf(f, " %d ", output_adaptation);  
    } 
     
     
    LineFeed(f);
#ifdef EXT_RFD
    FilePrintString(f,"; - Methode/Gitteradaption (method_adaptation)"); LineFeed(f);
    FilePrintString(f,";   0: keine Gitteradaption"); LineFeed(f);
    FilePrintString(f,";   1: rechnen, adaptieren, rechnen,...,adaptieren, rechnen, naechster Zeitschritt"); LineFeed(f);
    FilePrintString(f,";   2: adaptieren, rechnen,...,adaptieren, rechnen, naechster Zeitschritt"); LineFeed(f);
    FilePrintString(f,";   3: rechnen, adaptieren, rechnen,...,adaptieren, rechnen, naechster Zeitschritt"); LineFeed(f);
    FilePrintString(f,";      - Zeitschritte (adaptation_timestep), in denen adaptiert werden soll"); LineFeed(f);
    FilePrintString(f,";        Adaption alle n (adaptation_timestep) Zeitschritte"); LineFeed(f);
    FilePrintString(f,";   4: adaptieren, rechnen,...,adaptieren, rechnen, naechster Zeitschritt"); LineFeed(f);
    FilePrintString(f,";      - Zeitschritte (adaptation_timestep), in denen adaptiert werden soll"); LineFeed(f);
    FilePrintString(f,";        Adaption alle n (adaptation_timestep) Zeitschritte"); LineFeed(f);
    FilePrintString(f,";   5: im ersten Zeitschritt Methode 1, danach Methode 2"); LineFeed(f);
    FilePrintString(f,";   6: im ersten Zeitschritt Methode 3, danach Methode 4"); LineFeed(f);
    FilePrintString(f,";      - Zeitschritte (adaptation_timestep), in denen adaptiert werden soll"); LineFeed(f);
    FilePrintString(f,";        Adaption alle n (adaptation_timestep) Zeitschritte"); LineFeed(f);    
    FilePrintString(f,"; - Maximale Verfeinerungs-Tiefe insgesamt (max_ref_level) [>=0;0]"); LineFeed(f);
    FilePrintString(f,";   Wenn ein Element schon max_ref_level-mal verfeinert wurde, wird"); LineFeed(f);
    FilePrintString(f,";   es auf keinen Fall weiter verfeinert."); LineFeed(f);
    FilePrintString(f,"; - Verfeinerungs-Tiefe pro Zeitschritt (timestep_ref_number)"); LineFeed(f);
    FilePrintString(f,"; - Methode irregulaere Knoten (method_irr_nodes)"); LineFeed(f);
    FilePrintString(f,";   1: Eliminierung in Elementmatrizen"); LineFeed(f);
    FilePrintString(f,";   2: Eliminierung im Gesamtgleichungssystem"); LineFeed(f);
    FilePrintString(f,"; - Anzahl der verfeinerten Nachbarn eines 2d-Elements, ab der es verfeinert wird - "); LineFeed(f);
    FilePrintString(f,";   sekundaeres Kriterium (ref_neighbours_2d) [1..4;4]"); LineFeed(f);
    FilePrintString(f,";   Der Wert hat bei reiner 1D-Verfeinerung keinen Einfluss."); LineFeed(f);
    FilePrintString(f,"; - Anzahl der verfeinerten Nachbarn eines 3d-Elements, ab der es verfeinert wird - "); LineFeed(f);
    FilePrintString(f,";   sekundaeres Kriterium (ref_neighbours_3d) [1..6;6]"); LineFeed(f);
    FilePrintString(f,";   Der Wert hat bei reiner 1D-Verfeinerung keinen Einfluss."); LineFeed(f);
    FilePrintString(f,"; - Methode Knoten-'Limiter' (method_node_limiter)"); LineFeed(f);
    FilePrintString(f,";   method_node_limiter = 1:"); LineFeed(f);
    FilePrintString(f,";   - Zeitkurve (curve_node_limiter)"); LineFeed(f);
    FilePrintString(f,";   - Maximale Knotenanzahl");
    LineFeed(f);
    FilePrintString(f, ";    < 0  Indikatoren werden variiert, um |x| Knoten nicht zu ueberschreiten");
    LineFeed(f);
    FilePrintString(f, ";      0  Keine Begrenzung der Knotenzahl");
    LineFeed(f);
    FilePrintString(f, ";    > 0  Indikatoren werden variiert, um x Knoten zu erhalten");
    LineFeed(f);
    FilePrintString(f,";   method_node_limiter = 2:"); LineFeed(f);
    FilePrintString(f,";   - Zeitkurve (curve_node_limiter)"); LineFeed(f);
    FilePrintString(f,";   - Maximale Knotenanzahl");
    LineFeed(f);
    LineFeed(f);
    FilePrintString(f,"; Es folgen die Schluesselworte");
    LineFeed(f);
    FilePrintString(f,"; (#ADAPTATION_PRESSURE");
    LineFeed(f);
    FilePrintString(f,";  #ADAPTATION_COMPONENT ");
    LineFeed(f);
    FilePrintString(f,";  #ADAPTATION_TEMPERATURE ");
    LineFeed(f);
    FilePrintString(f,";  #ADAPTATION_SATURATION ...) ");
    LineFeed(f);
    FilePrintString(f,"; fuer die Feldgroessen:");
    LineFeed(f);
    LineFeed(f);
    FilePrintString(f,"; - Methode/Indikator (method_ind)"); LineFeed(f);
    FilePrintString(f,";   0: Defaultwerte"); LineFeed(f);
    FilePrintString(f,";   1: Standard 1"); LineFeed(f);
    FilePrintString(f,";   2: Standard 2"); LineFeed(f);
    FilePrintString(f,";   3: Professional"); LineFeed(f);LineFeed(f);
    FilePrintString(f,";   method_ind = 0: "); LineFeed(f);
    FilePrintString(f,";   Es muessen keine weiteren Werte eingelesen werden !!"); LineFeed(f);
    LineFeed(f);
    FilePrintString(f,";   method_ind = 1: "); LineFeed(f);
    FilePrintString(f,";   Unterscheidung zwischen Diffusion und Advektion bei Transportprozessen"); LineFeed(f);
    FilePrintString(f,";   (Verwendung analytischer oder heuristischer Verfeinerungsindikatoren moeglich)"); LineFeed(f);
    FilePrintString(f,";   - Korrektur bei Vergroeberung (coarsement_correction) [0,1]"); LineFeed(f);
    FilePrintString(f,";   - Anzahl Indikatoren (number_ind)"); LineFeed(f);
    FilePrintString(f,";   Es folgen 'number_ind' Bloecke der Form: "); LineFeed(f);
    FilePrintString(f,";   - Toleranz-Parameter [0,1,2]"); LineFeed(f);
    FilePrintString(f,";     0-absolut"); LineFeed(f);
    FilePrintString(f,";     1-Mittelwert"); LineFeed(f);
    FilePrintString(f,";     2-Maximalwert"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator diffusive 1d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator advektive 1d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator diffusive 2d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator advektive 2d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator diffusive 3d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator advektive 3d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    LineFeed(f);
    FilePrintString(f,";   method_ind = 2: "); LineFeed(f);
    FilePrintString(f,";   Keine Unterscheidung zwischen Diffusion und Advektion bei Transportprozessen"); LineFeed(f);
    FilePrintString(f,";   (heuristische Verfeinerungsindikatoren bei Transportprozessen)"); LineFeed(f);
    FilePrintString(f,";   (heuristische und analytische Verfeinerungsindikatoren bei Stroemung)"); LineFeed(f);
    FilePrintString(f,";   - Korrektur bei Vergroeberung (coarsement_correction) [0,1]"); LineFeed(f);
    FilePrintString(f,";   - Anzahl Indikatoren (number_ind)"); LineFeed(f);
    FilePrintString(f,";   Es folgen 'number_ind' Bloecke der Form: "); LineFeed(f);
    FilePrintString(f,";   - Toleranz-Parameter [0,1,2]"); LineFeed(f);
    FilePrintString(f,";     0-absolut"); LineFeed(f);
    FilePrintString(f,";     1-Mittelwert"); LineFeed(f);
    FilePrintString(f,";     2-Maximalwert"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 1d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 2d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 3d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    LineFeed(f);
    FilePrintString(f,";   method_ind = 3: "); LineFeed(f);
    FilePrintString(f,";   - Maximale Verfeinerungstiefe 1d-Elemente"); LineFeed(f);
    FilePrintString(f,";   - Maximale Verfeinerungstiefe 2d-Elemente"); LineFeed(f);
    FilePrintString(f,";   - Maximale Verfeinerungstiefe 3d-Elemente"); LineFeed(f);
    FilePrintString(f,";   - Korrektur bei Vergroeberung (coarsement_correction) [0,1]"); LineFeed(f);
    FilePrintString(f,";   - Anzahl Indikatoren (number_ind)"); LineFeed(f);
    FilePrintString(f,";   Es folgen 'number_ind' Bloecke der Form: "); LineFeed(f);
    FilePrintString(f,";   - Toleranz-Parameter [0,1,2]"); LineFeed(f);
    FilePrintString(f,";     0-absolut"); LineFeed(f);
    FilePrintString(f,";     1-Mittelwert"); LineFeed(f);
    FilePrintString(f,";     2-Maximalwert"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 1d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Verfeinerung"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Vergroeberung"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 2d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Verfeinerung"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Vergroeberung"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    FilePrintString(f,";   - Verfeinerungsindikator 3d-Elemente"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Verfeinerung"); LineFeed(f);
    FilePrintString(f,";     - Zeitkurve Vergroeberung"); LineFeed(f);
    FilePrintString(f,";     - Verfeinerungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Vergroeberungsschranke"); LineFeed(f);
    FilePrintString(f,";     - Dummy"); LineFeed(f);
    LineFeed(f);
#endif
   LineFeed(f);
   }

    return ok;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationPressure
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_PRESSURE gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert

                                                                          */
/**************************************************************************/
int FctAdaptationPressure(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 0;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_pressure);

#ifdef TESTFILES
  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    if ((GetRFProcessNumPhases() > 0) && (grid_adaptation == 1)) {
      /* Schluesselwort #ADAPTATION vorhanden */
      sprintf(string,"; Schluesselwort nicht gefunden !! %s", keyword);
      FilePrintString(f,string); LineFeed(f);
      return 0;
    }
  }
  else {
    /* ADAPTATION_PRESSURE gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);  
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationComponent
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_COMPONENT gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert
                                                                          */
/**************************************************************************/
int FctAdaptationComponent(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 1;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_component);

#ifdef TESTFILES
  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    if ((GetRFProcessNumComponents() > 0) && (grid_adaptation == 1)) {
      /* Schluesselwort #ADAPTATION vorhanden */
      sprintf(string,"; Schluesselwort nicht gefunden !! %s", keyword);
      FilePrintString(f,string); LineFeed(f);
      return 0;
    }
  }
  else {
    /* ADAPTATION_COMPONENT gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);   
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationTemperature
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_TEMPERATURE gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert
                                                                          */
/**************************************************************************/
int FctAdaptationTemperature(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 2;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_temperature);

#ifdef TESTFILES
  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    if ((GetRFProcessHeatReactModel()) && (grid_adaptation == 1)) {
      /* Schluesselwort #ADAPTATION vorhanden */
      sprintf(string,"; Schluesselwort nicht gefunden !! %s", keyword);
      FilePrintString(f,string); LineFeed(f);
      return 0;
    }
  }
  else {
    /* ADAPTATION_TEMPERATURE gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);       
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationSaturation
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_SATURATION gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert
                                                                          */
/**************************************************************************/
int FctAdaptationSaturation(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 3;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_saturation);

#ifdef TESTFILES
  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    if ((GetRFProcessNumPhases() > 1) && (grid_adaptation == 1)) {
      /* Schluesselwort #ADAPTATION vorhanden */
      sprintf(string,"; Schluesselwort nicht gefunden !! %s", keyword);
      FilePrintString(f,string); LineFeed(f);
      return 0;
    }
  }
  else {
    /* ADAPTATION_SATURATION gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);          
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationSorbedComponent
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_SORBED_COMPONENT gehoerigen
   Daten ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert
                                                                          */
/**************************************************************************/
int FctAdaptationSorbedComponent(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 4;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_sorbed_component);

  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
#ifdef TESTFILES
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {

  }
  else {
    /* ADAPTATION_SORBED_COMPONENT gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);              
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationSoluteComponent
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_SOLUTE_COMPONENT gehoerigen
   Daten ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/1999     R.Kaiser   Erste Version
   07/2000     R.Kaiser   Lesefunktionen verallgemeinert
                                                                          */
/**************************************************************************/
int FctAdaptationSoluteComponent(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 5;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_solute_component);

  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
#ifdef TESTFILES
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {

  }
  else {
    /* ADAPTATION_SOLUTE_COMPONENT gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);              
  }    
  return ok;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationVolt
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort ADAPTATION_VOLT (Geo-Elektrik) 
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   05/2001     R.Kaiser   Erste Version
   
                                                                          */
/**************************************************************************/
int FctAdaptationVolt(char *data, int found, FILE * f)
{
  int ok = 1;
  int type = 6;
  char keyword[80] = "\0";
  char string[80] = "\0";

  /* Schluesselwort */
  sprintf(keyword,"#%s",name_adaptation_volt);

#ifdef TESTFILES
  sprintf(string,"Eingabedatenbank, Schluesselwort %s",keyword);
  DisplayMsgLn(string);
#endif

  if ((!found) || (!GetRFControlGridAdapt()) ||
      (GetRFControlGridAdapt() == 2)) {
    if ((GetRFProcessNumElectricFields() > 0) && (grid_adaptation == 1)) {
      /* Schluesselwort #ADAPTATION vorhanden */
      sprintf(string,"; Schluesselwort nicht gefunden !! %s", keyword);
      FilePrintString(f,string); LineFeed(f);
      return 0;
    }
  }
  else {
    /* ADAPTATION_VOLT gefunden */
    if (found == 1)
      ok = FctAdaptationQuantities(data,f,type,keyword);
    else if (found == 2) 
      ok = WriteAdaptationQuantities(f,type,keyword);      
  }
  return ok;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: FctAdaptationQuantities
                                                                          */
/* Aufgabe:

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

                                                                          */
/* Ergebnis:

                                                                          */
/* Programmaenderungen:
   07/2000     R.Kaiser   Erste Version
   10/2000     C.Thorenz  Warnungen beseitigt
   10/2001     R.Kaiser   Adaption für Geoelektrik
   
                                                                          */
/**************************************************************************/
int FctAdaptationQuantities(char *data, FILE * f, int type, char *keyword)
{
  int ok = 1;
  int pos = 0;
  int p = 0;
  int l1;
  int i, j, k, l;
  int counter = 0, counter_stop = 0;
  int num_phase_compo=-1;
  char string[80] = "\0";

  while (StrTestHash(&data[p += pos],&pos)) {
    if (grid_adaptation == 0) {
      /* Schluesselwort #ADAPTATION nicht vorhanden */
#ifdef EXT_RFD
      sprintf(string,"; Schluesselwort: %s",keyword);
      FilePrintString(f,string); LineFeed(f);
#endif
      return ok;
    }
    if (grid_adaptation == 1) {
      /* Schluesselwort #ADAPTATION vorhanden */

      if (((type == 1) && (GetRFProcessNumComponents() == 0)) ||
          ((type == 0) && (GetRFProcessNumPhases() == 0)) ||
          ((type == 2) && (!GetRFProcessHeatReactModel())) ||
          ((type == 3) && (GetRFProcessNumPhases() < 1)) ||
          ((type == 4) && (GetRFProcessNumComponents() == 0)) ||
          ((type == 5) && (GetRFProcessNumComponents() == 0)) ||
          ((type == 6) && (GetRFProcessNumElectricFields() == 0)) 
          ) {
        return ok;
      }


      if (((type == 1) && (counter == (GetRFProcessNumPhases() * GetRFProcessNumComponents()))) ||
          ((type == 0) && (counter == GetRFProcessNumPhases())) ||
          ((type == 2) && (counter == 1)) ||
          ((type == 3) && (counter == GetRFProcessNumPhases())) ||
          ((type == 4) && (counter == GetRFProcessNumComponents())) ||
          ((type == 5) && (counter == GetRFProcessNumComponents())) ||
          ((type == 6) && (counter == GetRFProcessNumElectricFields()))     
        ) {
        return ok;
      }



      sprintf(string,keyword);
      FilePrintString(f,string);
      LineFeed(f);

      number_adapt_quantities++;
      adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
      adapt_data[number_adapt_quantities - 1].ind_data = NULL;
      adapt_data[number_adapt_quantities - 1].ref_quantity = type;
      adapt_data[number_adapt_quantities - 1].number_phase_component = counter;
      counter++;

      ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].method_ind,&data[p+=pos],f,TFInt,&pos) && ok);

      switch (adapt_data[number_adapt_quantities - 1].method_ind) {
        case 0:
          adapt_data[number_adapt_quantities - 1].coarsement_correction = 1;
          adapt_data[number_adapt_quantities - 1].number_ind  = 1;
          total_number_ind += 1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (1 * sizeof(IndData)));
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_tol = 1;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[0][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][2] = 0.0;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[1][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][2] = 0.0;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[2][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][2] = 0.0;
          break;

        case 1:
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].coarsement_correction,&data[p+=pos],f,TFCoarsementCorrect,&pos) && ok);
          ok = (StrReadLong(&adapt_data[number_adapt_quantities - 1].number_ind,&data[p+=pos],f,TFNumberInd,&pos) && ok);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol,&data[p+=pos],f,TFRefTol,&pos) && ok);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              for (j = 0; j < 2; j++) {
                ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][j],&data[p+=pos],f,TFInt,&pos) && ok);
                for (k=0;k<3;k++)
                  ok = (StrReadDouble(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][j][k],&data[p+=pos],f,TFDouble,&pos) && ok);
                LineFeed(f);
              }
            }
          }
          break;

        case 2:
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].coarsement_correction,&data[p+=pos],f,TFCoarsementCorrect,&pos) && ok);
          ok = (StrReadLong(&adapt_data[number_adapt_quantities - 1].number_ind,&data[p+=pos],f,TFNumberInd,&pos) && ok);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol,&data[p+=pos],f,TFRefTol,&pos) && ok);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][0],&data[p+=pos],f,TFInt,&pos) && ok);
              for (k=0;k<3;k++)
                ok = (StrReadDouble(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][0][k],&data[p+=pos],f,TFDouble,&pos) && ok);
              LineFeed(f);

            }
          }
          break;

        case 3:
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[0],&data[p+=pos],f,TFQuantityMaxRefLevel,&pos) && ok);
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[1],&data[p+=pos],f,TFQuantityMaxRefLevel,&pos) && ok);
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[2],&data[p+=pos],f,TFQuantityMaxRefLevel,&pos) && ok);
          ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].coarsement_correction,&data[p+=pos],f,TFCoarsementCorrect,&pos) && ok);
          ok = (StrReadLong(&adapt_data[number_adapt_quantities - 1].number_ind,&data[p+=pos],f,TFNumberInd,&pos) && ok);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol,&data[p+=pos],f,TFRefTol,&pos) && ok);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][0],&data[p+=pos],f,TFInt,&pos) && ok);
              for (k=0;k<2;k++)
                ok = (StrReadInt(&adapt_data[number_adapt_quantities - 1].ind_data[l].curve_ind[i][0][k],&data[p+=pos],f,TFInt,&pos) && ok);
              for (k=0;k<3;k++)
                ok = (StrReadDouble(&adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][0][k],&data[p+=pos],f,TFDouble,&pos) && ok);
              LineFeed(f);

            }
          }
          break;
      }
    LineFeed(f);
    }
  }

  if ((type == 0) || (type == 3)) num_phase_compo = GetRFProcessNumPhases();
  if (type == 1) num_phase_compo = GetRFProcessNumPhases() * GetRFProcessNumComponents();
  if (type == 2) num_phase_compo = 1;
  if ((type == 4) || (type == 5)) num_phase_compo = GetRFProcessNumComponents();
  if (type == 6) num_phase_compo = GetRFProcessNumElectricFields();

  if (type != 6) {
  while (counter < num_phase_compo) {
    if (grid_adaptation == 1) {

      sprintf(string,keyword);
      FilePrintString(f,string);
      LineFeed(f);

      number_adapt_quantities++;
      adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
      adapt_data[number_adapt_quantities - 1].ind_data = NULL;
      adapt_data[number_adapt_quantities - 1].ref_quantity = type;
      adapt_data[number_adapt_quantities - 1].number_phase_component = counter;
      counter++;
      FilePrintInt(f,adapt_data[number_adapt_quantities - 1].method_ind = 2);
      FilePrintInt(f,adapt_data[number_adapt_quantities - 1].coarsement_correction = 1);
      FilePrintInt(f,adapt_data[number_adapt_quantities - 1].number_ind = 0);
      LineFeed(f);
      LineFeed(f);
    }
  }
  }
  if (type == 6) {
    counter_stop = number_adapt_quantities;
    while (counter < num_phase_compo) {
      if (grid_adaptation == 1) {

      sprintf(string,keyword);
      FilePrintString(f,string);
      LineFeed(f);

      number_adapt_quantities++;
      adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
      adapt_data[number_adapt_quantities - 1].ind_data = NULL;
      adapt_data[number_adapt_quantities - 1].ref_quantity = type;
      adapt_data[number_adapt_quantities - 1].number_phase_component = counter;
      counter++;
      FilePrintInt(f,adapt_data[number_adapt_quantities - 1].method_ind = adapt_data[counter_stop - 1].method_ind);
      
      switch (adapt_data[number_adapt_quantities - 1].method_ind) {
        case 0:
          adapt_data[number_adapt_quantities - 1].coarsement_correction = 1;
          adapt_data[number_adapt_quantities - 1].number_ind  = 1;
          total_number_ind += 1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (1 * sizeof(IndData)));
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_tol = 1;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[0][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[0][0][2] = 0.0;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[1][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[1][0][2] = 0.0;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_ind[2][0] = 3;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][0] = 1.2;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][1] = 0.8;
          adapt_data[number_adapt_quantities - 1].ind_data[0].ref_param[2][0][2] = 0.0;
          break;

        case 1:
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].coarsement_correction = adapt_data[counter_stop - 1].coarsement_correction);
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].number_ind = adapt_data[counter_stop - 1].number_ind);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol = adapt_data[counter_stop - 1].ind_data[l].ref_tol);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              for (j = 0; j < 2; j++) {
                FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][j] = adapt_data[counter_stop - 1].ind_data[l].ref_ind[i][j]);
                for (k=0;k<3;k++)
                  FilePrintDouble(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][j][k] = adapt_data[counter_stop - 1].ind_data[l].ref_param[i][j][k]);
                LineFeed(f);
              }
            }
          }
          break;

        case 2:
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].coarsement_correction = adapt_data[counter_stop - 1].coarsement_correction );
          FilePrintLong(f,adapt_data[number_adapt_quantities - 1].number_ind = adapt_data[counter_stop - 1].number_ind);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol = adapt_data[counter_stop - 1].ind_data[l].ref_tol);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][0] = adapt_data[counter_stop - 1].ind_data[l].ref_ind[i][0]);
              for (k=0;k<3;k++)
                FilePrintDouble(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][0][k] = adapt_data[counter_stop - 1].ind_data[l].ref_param[i][0][k]);
              LineFeed(f);

            }
          }
          break;

        case 3:
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[0] = adapt_data[counter_stop - 1].quantity_max_ref_level[0]);
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[1] = adapt_data[counter_stop - 1].quantity_max_ref_level[1]);
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].quantity_max_ref_level[2] = adapt_data[counter_stop - 1].quantity_max_ref_level[2]);
          FilePrintInt(f,adapt_data[number_adapt_quantities - 1].coarsement_correction = adapt_data[counter_stop - 1].coarsement_correction);
          FilePrintLong(f,adapt_data[number_adapt_quantities - 1].number_ind = adapt_data[counter_stop - 1].number_ind);
          LineFeed(f);
          l1 = adapt_data[number_adapt_quantities - 1].number_ind;
          total_number_ind += l1;
          adapt_data[number_adapt_quantities - 1].ind_data = (IndData *) Realloc(adapt_data[number_adapt_quantities - 1].ind_data, (l1 * sizeof(IndData)));
          for (l=0;l<l1;l++) {
            FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_tol = adapt_data[counter_stop - 1].ind_data[l].ref_tol);
            LineFeed(f);
            for (i = 0; i < 3; i++) {
              FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_ind[i][0] = adapt_data[counter_stop - 1].ind_data[l].ref_ind[i][0]);
              for (k=0;k<2;k++)
                FilePrintInt(f,adapt_data[number_adapt_quantities - 1].ind_data[l].curve_ind[i][0][k] = adapt_data[counter_stop - 1].ind_data[l].curve_ind[i][0][k]);
              for (k=0;k<3;k++)
                FilePrintDouble(f,adapt_data[number_adapt_quantities - 1].ind_data[l].ref_param[i][0][k] = adapt_data[counter_stop - 1].ind_data[l].ref_param[i][0][k]);
              LineFeed(f);

            }
          }
          break;
      }

      LineFeed(f);
      LineFeed(f);
      }
    }
 
 
  }
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: WriteAdaptationQuantities
                                                                          */
/* Aufgabe:
   Daten-Protokoll im RFD-File (found==2)
   Schreibt alle ADP-Objekte des Typs 'type' in das RFD-File
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E FILE *f:       RFD-File-Zeiger
   E int type:      Objekt-Typ
   E char *keyword: Schluesselwort
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   07/2001     RK    Erste Version
                                                                          */
/**************************************************************************/
int WriteAdaptationQuantities(FILE * f, int type, char *keyword)
{  
  int ok = 1;
  int l, l1;
  int i, j, k;
  int index; 
  
  FilePrintString(f,keyword); LineFeed(f);
  for (index=0;index<number_adapt_quantities;index++) {
    if(adapt_data[index].ref_quantity == type) {
      fprintf(f," %d ",adapt_data[index].method_ind);
      switch (adapt_data[index].method_ind) {
      case 0:
        fprintf(f," %d ",adapt_data[index].coarsement_correction);
        fprintf(f," %ld ",adapt_data[index].number_ind);
        LineFeed(f);
        fprintf(f," %d ",adapt_data[index].ind_data[0].ref_tol);
        LineFeed(f);
        fprintf(f," %d ",adapt_data[index].ind_data[0].ref_ind[0][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[0][0][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[0][0][1]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[0][0][2]);
        LineFeed(f);
        fprintf(f," %d ",adapt_data[index].ind_data[0].ref_ind[1][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[1][0][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[1][0][1]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[1][0][2]);
        LineFeed(f); 
        fprintf(f," %d ",adapt_data[index].ind_data[0].ref_ind[2][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[2][0][0]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[2][0][1]);
        fprintf(f," %e ",adapt_data[index].ind_data[0].ref_param[2][0][2]); 
        LineFeed(f); 
        break;
      case 1:
        fprintf(f," %d ",adapt_data[index].coarsement_correction);
        fprintf(f," %ld ",adapt_data[index].number_ind);
        LineFeed(f);
        l1 = adapt_data[index].number_ind;
        for (l=0;l<l1;l++) {
          fprintf(f," %d ",adapt_data[index].ind_data[l].ref_tol);                                          
          LineFeed(f);
          for (i = 0; i < 3; i++) {
            for (j = 0; j < 2; j++) {
              fprintf(f," %d ",adapt_data[index].ind_data[l].ref_ind[i][j]);
              for (k=0;k<3;k++)
                fprintf(f," %e ",adapt_data[index].ind_data[l].ref_param[i][j][k]);
              LineFeed(f);  
            }
          }
        }
        break;
      case 2:
        fprintf(f," %d ",adapt_data[index].coarsement_correction);
        fprintf(f," %ld ",adapt_data[index].number_ind);
        LineFeed(f);
        l1 = adapt_data[index].number_ind;
        for (l=0;l<l1;l++) {
          fprintf(f," %d ",adapt_data[index].ind_data[l].ref_tol);                                          
          LineFeed(f);      
          for (i = 0; i < 3; i++) {
            fprintf(f," %d ",adapt_data[index].ind_data[l].ref_ind[i][0]);  
            for (k=0;k<3;k++)
              fprintf(f," %e ",adapt_data[index].ind_data[l].ref_param[i][0][k]);
            LineFeed(f);
          }
        }
        break;
      case 3:
        fprintf(f," %d ",adapt_data[index].quantity_max_ref_level[0]);
        fprintf(f," %d ",adapt_data[index].quantity_max_ref_level[1]);
        fprintf(f," %d ",adapt_data[index].quantity_max_ref_level[2]);
        fprintf(f," %d ",adapt_data[index].coarsement_correction);
        fprintf(f," %ld ",adapt_data[index].number_ind);
        LineFeed(f);
        l1 = adapt_data[index].number_ind;
        for (l=0;l<l1;l++) {
          fprintf(f," %d ",adapt_data[index].ind_data[l].ref_tol);                                          
          LineFeed(f);      
          for (i = 0; i < 3; i++) {
            fprintf(f," %d ",adapt_data[index].ind_data[l].ref_ind[i][0]);  
            for (k=0;k<2;k++)
              fprintf(f," %d ",adapt_data[index].ind_data[l].curve_ind[i][0][k]);  
            for (k=0;k<3;k++)
              fprintf(f," %e ",adapt_data[index].ind_data[l].ref_param[i][0][k]);
            LineFeed(f);
          }
        }
        break;
      }
    }
  }
  return ok;
}
 




/**************************************************************************/
/*                           Testfunktionen                               */
/**************************************************************************/

int TFAdaptMethod ( int *x, FILE *f )
{
  /* wenn method < 0 oder method > 3, auf 1 setzen */
  if ((*x < 0) || (*x > 3)) {
    *x = 1;
    LineFeed(f);
    FilePrintString(f,"* ! method unzulaessig, Korrektur: 1");
    LineFeed(f);
  }
  return 1;
}

int TFMaxRefLevel ( int *x, FILE *f )
{
  /* wenn max_ref_level < 0, auf 0 setzen */
  if (*x < 0) {
    *x = 0;
    LineFeed(f);
    FilePrintString(f,"* ! max_ref_level unzulaessig, Korrektur: 0");
    LineFeed(f);
  }
  return 1;
}

int TFTimestepRefNumber ( int *x, FILE *f )
{
  if (*x > max_ref_level) {
      *x = max_ref_level;
      LineFeed(f);
      FilePrintString(f,"* ! timestep_ref_number unzulaessig, Korrektur: max_ref_level");
      LineFeed(f);
  }
  return 1;
}

int TFRefNeighbours2d ( int *x, FILE *f )
{
 if ((*x <= 0) || (*x > 4)) {
   *x = 4;
   LineFeed(f);
   FilePrintString(f,"* ! ref_neighbours_2d unzulaessig, Korrektur: 4");
   LineFeed(f);
 }
 return 1;
}


int TFRefNeighbours3d ( int *x, FILE *f )
{
 if ((*x <= 0) || (*x > 6)) {
   *x = 6;
   LineFeed(f);
   FilePrintString(f,"* ! ref_neighbours_3d unzulaessig, Korrektur: 6");
   LineFeed(f);
 }
 return 1;
}

int TFQuantityMaxRefLevel ( int *x, FILE *f )
{
 if (*x > max_ref_level) {
   *x = max_ref_level;
   LineFeed(f);
   FilePrintString(f,"* ! quantity_max_ref_level unzulaessig, Korrektur: max_ref_level");
   LineFeed(f);
 }
 return 1;
}


int TFCoarsementCorrect ( int *x, FILE *f )
{
 if ((*x < 0) || (*x > 1)) {
   *x = 1;
   LineFeed(f);
   FilePrintString(f,"* ! coarsement_correction unzulaessig, Korrektur: 1");
   LineFeed(f);
 }
 return 1;
}

int TFNumberInd ( long *x, FILE *f )
{
 if (*x < 0)  {
   *x = 0;
   LineFeed(f);
   FilePrintString(f,"* ! number_ind unzulaessig, Korrektur: 0");
   LineFeed(f);
 }
 return 1;
}

int TFRefTol ( int *x, FILE *f )
{
 if ((*x < 0) || (*x > 2)) {
   *x = 1;
   LineFeed(f);
   FilePrintString(f,"* ! ref_tol unzulaessig, Korrektur: 1");
   LineFeed(f);
 }
 return 1;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CreateAdaptationControlParameter
                                                                          */
/* Aufgabe:
   Konstruktor fuer Adaptions-Objekte 
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int found: Schluesselwort gefunden: 1, sonst 0
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/2001   RK   Erste Version
   
                                                                          */
/**************************************************************************/
void CreateAdaptationControlParameter(void)
{
  int ii,i,j,k;
  int l,l1;
  int number_of_phase_component;
  int index;
  
  method_adaptation = 0;
  adaptation_timestep = 1;
  max_ref_level = 0; 
  timestep_ref_number = 0; 
  method_irr_nodes = 1;
  ref_neighbours_2d = 4;
  ref_neighbours_3d = 6;
  method_node_limiter = 0;
  curve_node_limiter = 0;
  max_nodes = 0l;
  output_adaptation = 0;  
  
  number_adapt_quantities = 0;
  number_of_phase_component = 0;
  
  if(!adapt_data) {
    /* ADAPTATION_PRESSURE */
    number_adapt_quantities = GetRFProcessNumPhases();
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<GetRFProcessNumPhases();ii++) {
      index = ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 0; /* Druck */
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }
    number_of_phase_component += GetRFProcessNumPhases();
    
    
    /* ADAPTATION_SATURATION */
    number_adapt_quantities += GetRFProcessNumPhases();
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<GetRFProcessNumPhases();ii++) {
      index = number_of_phase_component + ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 3; /* Sättigung */
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }    
    number_of_phase_component += GetRFProcessNumPhases();
    
    
    /* ADAPTATION_SORBED_COMPONENT */
    number_adapt_quantities += GetRFProcessNumComponents();
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<GetRFProcessNumComponents();ii++) {
      index = number_of_phase_component + ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 4; 
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }    
    number_of_phase_component += GetRFProcessNumComponents();
    
    /* ADAPTATION_SOLUTE_COMPONENT */
    number_adapt_quantities += GetRFProcessNumComponents();
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<GetRFProcessNumComponents();ii++) {
      index = number_of_phase_component + ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 5; 
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }    
    number_of_phase_component += GetRFProcessNumComponents();
    
    /* ADAPTATION_COMPONENT */
    number_adapt_quantities += GetRFProcessNumComponents();
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<GetRFProcessNumComponents();ii++) {
      index = number_of_phase_component + ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 1; 
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }    
    number_of_phase_component += GetRFProcessNumComponents();
    
    
    /* ADAPTATION_TEMPERATURE */
    number_adapt_quantities += 1;
    adapt_data = (AdaptData *) Realloc(adapt_data, (number_adapt_quantities * sizeof(AdaptData)));
    for(ii=0;ii<1;ii++) {
      index = number_of_phase_component + ii;
      adapt_data[index].ind_data = NULL;
      adapt_data[index].ref_quantity = 2; 
      adapt_data[index].number_phase_component = ii;
      adapt_data[index].method_ind = 0; 
      
      adapt_data[index].quantity_max_ref_level[0] = 1;
      adapt_data[index].quantity_max_ref_level[1] = 1;
      adapt_data[index].quantity_max_ref_level[2] = 1;
      adapt_data[index].coarsement_correction = 0;
      adapt_data[index].number_ind = 3;
      l1 = adapt_data[index].number_ind;
      total_number_ind += l1;
      adapt_data[index].ind_data = (IndData *) Realloc(adapt_data[index].ind_data, (l1 * sizeof(IndData)));
      
      for (l=0;l<l1;l++) {
        adapt_data[index].ind_data[l].ref_tol = 0;
        for (i=0;i<3;i++) {
          for (j=0;j<2;j++) {
            adapt_data[index].ind_data[l].ref_ind[i][j] = 3;
            for (k=0;k<2;k++)
              adapt_data[index].ind_data[l].curve_ind[i][j][k] = 0;
            for (k=0;k<3;k++)
              adapt_data[index].ind_data[l].ref_param[i][j][k] = 0.0;
          }

        }
      }   
      
    }    
    number_of_phase_component += 1;
    
    
  
  }
  else {
      DisplayMsgLn("CreateAdaptationControlParameter: data construction failed");
  }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyAdaptationControlParameter
                                                                          */
/* Aufgabe:
   Speicherfreigaben fuer Adaptionsparameter,
   die vorher via Lesefunktionen bzw. CreateAdaptationControlParameter 
   allokiert wurden
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   07/2001   RK   Erste Version
                                                                          */
/**************************************************************************/
void DestroyAdaptationControlParameter(void)
{
  int ii;
  int number_of_phase_component;
  
  
  number_of_phase_component = 0;

  if(adapt_data) {
    /* ADAPTATION_PRESSURE */
    number_of_phase_component += GetRFProcessNumPhases();
    /* ADAPTATION_SATURATION */
    number_of_phase_component += GetRFProcessNumPhases();
    /* ADAPTATION_SORBED_COMPONENT */
    number_of_phase_component += GetRFProcessNumComponents();
    /* ADAPTATION_SOLUTE_COMPONENT */
    number_of_phase_component += GetRFProcessNumComponents();
    /* ADAPTATION_COMPONENT */
    number_of_phase_component += GetRFProcessNumComponents();
    /* ADAPTATION_TEMPERATURE */
    number_of_phase_component += 1;
    
    for(ii=0;ii<number_of_phase_component;ii++) {
      adapt_data[ii].ind_data = (IndData *) Free(adapt_data[ii].ind_data);
    }
    
    adapt_data = (AdaptData *) Free(adapt_data);
  
  } else {
      DisplayMsgLn("DestroyAdaptationControlParameter: data destruction failed");
  }

}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyAllNewElementsGridAdaptation
                                                                          */
/* Aufgabe:
   Zerstoert alle durch die Adaption neu erzeugte Elemente.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version
                                                                          */
/**************************************************************************/
void DestroyAllNewElementsGridAdaptation(void)
{
  long i, j;
  
  if (GetRFControlGridAdapt()) { 
    j = ElListSize();
    for (i = start_new_elems; i < j; i++) {
      if (ElGetElement(i) != NULL) {
        ElDeleteElement(i);
      }
    }
    for (i = 0; i < NodeListSize(); i++) {
        if (GetNode(i)!=NULL)  /* wenn Knoten existiert */
          if (GetNodeStartNumber(i)<0) {  /* wenn Knoten nicht zum Startnetz gehoert */
            DeleteNode(i);
          }
    }
    CompressNodeList();
    for (i = 0; i < NodeListSize(); i++) {
        if (GetNode(i)!=NULL) {  /* wenn Knoten existiert */
            DeleteNodeElems(i);
        }
    }
    for (i = 0l; i < max_ref_level; i++)
      num_elems_ref_level_X[i] = 0l;
    for (i = 0; i < start_new_elems; i++)
      ElSetElementActiveState(i,1);
    DestroyEdgeList();
    DestroyPlainList();
    ElCompressElementList();
    CompressNodes();

  }   
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ReinitGridAdaptation
                                                                          */
/* Aufgabe:
   Neuinitialisierung adaptiver Variablen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   10/2001    AH      Erste Version (aus InitGridAdaptation)
                                                                          */
/**************************************************************************/
void ReinitGridAdaptation(void)
{
  long i, j;
  
  if (GetRFControlGridAdapt()) { 
    anz_active_1D = 0l;
    anz_active_2D = 0l;
    anz_active_3D = 0l;
    j = ElListSize();
    for (i = 0; i < start_new_elems; i++) {
      if (ElGetElementActiveState(i)) {
        switch (ElGetElementType(i)) {
        case 1: anz_active_1D++;
                break;
        case 2: anz_active_2D++;
                break;
        case 3: anz_active_3D++;
                break;
        }
      }
    }
    for (i = start_new_elems; i < j; i++) {
      if (ElGetElement(i) != NULL) {
        (num_elems_ref_level_X[ElGetElementLevel(i) - 1])++;
        if (ElGetElementActiveState(i)) {
          switch (ElGetElementType(i)) {
          case 1: anz_active_1D++;
                  break;
          case 2: anz_active_2D++;
                  break;
          case 3: anz_active_3D++;
                  break;
          }
        }
      }
    }

    anz_active_elements = anz_active_1D + anz_active_2D + anz_active_3D;
    BuildActiveElementsArray();
    CreateEdgeList();
    CreatePlainList();
    ConfigTopology();
    /* Vorbereitungen fuer das adaptive Modell */
    /* Kanten- bzw. Flaechenverzeichnis aufbauen */   
    ConstructRelationships(); 
    ConstructStatusEntries(); 
  }
  else {
    anz_active_elements = anz_1D + anz_2D + anz_3D;
    BuildActiveElementsArray();  
  }   
     
}

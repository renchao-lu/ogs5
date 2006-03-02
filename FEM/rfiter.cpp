/**************************************************************************/
/* ROCKFLOW - Modul: rfiter.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp ITERATION_PROPERTIES
   - Verwaltung von ITERATION_PROPERTIES mit dem Datentyp LIST_ITERATION_PROPERTIES
                                                                          */
/* Programmaenderungen:
   09/1999    AH      Erste Version
   12/1999    AH      Bugfix bei Mehrfachlesen (fuer z.B. Mehrkomponentenmodell)
                      Neue Funktionen: GetIterationNumberIterationProperties
                                       GetConvergenceRateIterationProperties
   06/2000    AH      Mobil-Immobil-Konzept (Defaults)    
   03/2003    RK      Quellcode bereinigt, Globalvariablen entfernt   
      

   last modified: RK 12.03.2003
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Used tools */
#include "mathlib.h"
#include "geo_strings.h"

/* Used objects */
#include "rf_pcs.h" //OK_MOD"

/* Interface */
#include "rfiter.h"

//OK_IC #include "rfbc.h" /* DMATRIX ToDo OK 3261 */
#include "tools.h"
#include "nodes.h"

  /* Liste von Randbedingungen */
static LIST_ITERATION_PROPERTIES *list_of_iteration_properties = NULL;

/* Nur fuer den Interface RF-BC */
static DMATRIX *ip_matrix_values = NULL;
static LMATRIX *ip_matrix_iter = NULL;
static char **ip_names = NULL;
static long ip_names_number = 0;
/*static long ip_cur_col = -1;*/



/* ================================================================ */
/* ITERATION_PROPERTIES */
/* ================================================================ */

  /* Schluesselwoerter */
char *keyword_iteration_properties_fluid_phase      = "ITERATION_PROPERTIES_PRESSURE";
char *keyword_iteration_properties_water_content    = "ITERATION_PROPERTIES_WATER_CONTENT";
char *keyword_iteration_properties_saturation       = "ITERATION_PROPERTIES_SATURATION";
char *keyword_iteration_properties_tracer_component = "ITERATION_PROPERTIES_CONCENTRATION";
char *keyword_iteration_properties_sorbed_component = "ITERATION_PROPERTIES_SORBED_CONCENTRATION";
char *keyword_iteration_properties_solute_component = "ITERATION_PROPERTIES_SOLUTE_CONCENTRATION";
char *keyword_iteration_properties_temperature_phase      = "ITERATION_PROPERTIES_TEMPERATURE_PHASE";
char *keyword_iteration_properties_immobile_solute_component = "ITERATION_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION";

  /* Namen der IP-Gruppen  */
char *name_iteration_properties_fluid_phase      = "ITERATION_PROPERTIES_PRESSURE";
char *name_iteration_properties_water_content    = "ITERATION_PROPERTIES_WATER_CONTENT";
char *name_iteration_properties_saturation       = "ITERATION_PROPERTIES_SATURATION";
char *name_iteration_properties_tracer_component = "ITERATION_PROPERTIES_CONCENTRATION";
char *name_iteration_properties_sorbed_component = "ITERATION_PROPERTIES_SORBED_CONCENTRATION";
char *name_iteration_properties_solute_component = "ITERATION_PROPERTIES_SOLUTE_CONCENTRATION";
char *name_iteration_properties_temperature_phase      = "ITERATION_PROPERTIES_TEMPERATURE_PHASE";
char *name_iteration_properties_immobile_solute_component = "ITERATION_PROPERTIES_IMMOBILE_SOLUTE_CONCENTRATION";

/* Liste der vordefinierten IP-Gruppen */
static char **names_iteration_properties_fluid_phase=NULL;
static char **names_iteration_properties_water_content=NULL;
static char **names_iteration_properties_saturation=NULL;
static char **names_iteration_properties_tracer_component=NULL;
static char **names_iteration_properties_sorbed_component=NULL;
static char **names_iteration_properties_solute_component=NULL;
static char **names_iteration_properties_temperature_phase=NULL;
static char **names_iteration_properties_immobile_solute_component=NULL;

/* Systemnamen */
static char *sys_name_iteration_properties=NULL;


/* Interne Deklarationen - Funktionen */
void InitIterationSolverPropertiesOld(void);


/*************************************************************************
 ROCKFLOW - Funktion: ConfigIterationProperties

 Aufgabe:
   Konfiguration von Iterations-Parametern

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)

 Ergebnis:
   - void -

 Programmaenderungen:
   12/1999   OK   Implementierung

*************************************************************************/
void ConfigIterationProperties(void)
{
  /*--------------------------------------------------------------------*/
  /* O/4   Solver */
  /* O/4.3 Iteration Properties */
    InitDefaultIterationPropertiesNames("ROCKFLOW");

  /* ToDo - nur zum Testen - Initialisieren der alten Loeser-Parameter - OK 3261*/
    InitIterationSolverPropertiesOld();
}

void InitIterationSolverPropertiesOld(void)
{
  /* #ITERATION_TIME_CONTROL */
  iteration_min_iter = 5;
  iteration_max_iter = 100;
  iteration_weight_plus = 10.;
  iteration_weight_up = 1.4;
  iteration_weight_down = 0.7;
  iteration_min_dt = 1.e-3;
  iteration_max_dt = 1.e+6;
}



/*************************************************************************
 ROCKFLOW - Funktion: InitDefaultIterationPropertiesNames

 Aufgabe:
   Konstruktor fuer DefaultIterationPropertiesNames

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Systemnamen.
  
 Ergebnis:
   - void -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
void InitDefaultIterationPropertiesNames(char *sys_name)
{
    int i;
    char string[81];
    char *new_name;

    /* Name nur fuer die Zukunft */
    sys_name_iteration_properties = (char *) Malloc((int)strlen(sys_name) + 1);
    if (sys_name_iteration_properties == NULL) {
        return;
    }
    strcpy(sys_name_iteration_properties, sys_name);


    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    names_iteration_properties_fluid_phase=(char **) Realloc(names_iteration_properties_fluid_phase,(GetRFProcessNumPhases())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_fluid_phase,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_fluid_phase[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    names_iteration_properties_water_content=(char **) Realloc(names_iteration_properties_water_content,(GetRFProcessNumPhases())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_water_content,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_water_content[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    names_iteration_properties_saturation=(char **) Realloc(names_iteration_properties_saturation,(GetRFProcessNumPhases())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_saturation,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_saturation[i]=new_name;
    }


    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    names_iteration_properties_temperature_phase=(char **) Realloc(names_iteration_properties_temperature_phase,(GetRFProcessNumPhases())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_temperature_phase,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_temperature_phase[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_mass_tracer_component */
    names_iteration_properties_tracer_component=(char **) Realloc(names_iteration_properties_tracer_component,(GetRFProcessNumComponents())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_tracer_component,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_tracer_component[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    names_iteration_properties_solute_component=(char **) Realloc(names_iteration_properties_solute_component,(GetRFProcessNumComponents())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_solute_component,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_solute_component[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    names_iteration_properties_sorbed_component=(char **) Realloc(names_iteration_properties_sorbed_component,(GetRFProcessNumComponents())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_sorbed_component,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_sorbed_component[i]=new_name;
    }

    /* Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    names_iteration_properties_immobile_solute_component=(char **) Realloc(names_iteration_properties_immobile_solute_component,(GetRFProcessNumComponents())*sizeof(char **) );
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
          sprintf(string,"%s%i",name_iteration_properties_immobile_solute_component,i+1);
      new_name=(char *)Malloc((int)strlen(string)+1);
      strcpy(new_name,string);
      names_iteration_properties_immobile_solute_component[i]=new_name;
    }

    return;
}


/*************************************************************************
 ROCKFLOW - Funktion: FreeDefaultIterationPropertiesNames

 Aufgabe:
   Destruktor fuer DefaultIterationPropertiesNames

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -

 Ergebnis:
   - void -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
void FreeDefaultIterationPropertiesNames(void)
{
    long i;

    if (!sys_name_iteration_properties) return;

    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
      names_iteration_properties_fluid_phase[i]=(char *)Free(names_iteration_properties_fluid_phase[i]);
    }
    names_iteration_properties_fluid_phase=(char **)Free(names_iteration_properties_fluid_phase);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
      names_iteration_properties_water_content[i]=(char *)Free(names_iteration_properties_water_content[i]);
    }
    names_iteration_properties_water_content=(char **)Free(names_iteration_properties_water_content);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
      names_iteration_properties_saturation[i]=(char *)Free(names_iteration_properties_saturation[i]);
    }
    names_iteration_properties_saturation=(char **)Free(names_iteration_properties_saturation);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_volume_fluid_phase */
    for (i = 0; i < GetRFProcessNumPhases(); i++) {
      names_iteration_properties_temperature_phase[i]=(char *)Free(names_iteration_properties_temperature_phase[i]);
    }
    names_iteration_properties_temperature_phase=(char **)Free(names_iteration_properties_temperature_phase);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_source_mass_tracer_component */
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
      names_iteration_properties_tracer_component[i]=(char *)Free(names_iteration_properties_tracer_component[i]);
    }
    names_iteration_properties_tracer_component=(char **)Free(names_iteration_properties_tracer_component);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
      names_iteration_properties_solute_component[i]=(char *)Free(names_iteration_properties_solute_component[i]);
    }
    names_iteration_properties_solute_component=(char **)Free(names_iteration_properties_solute_component);


    /* Freigabe der Namen fuer vordefinierte Quell/Senk-Objekte */
        /* fuer names_iteration_properties_solute_component */
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
      names_iteration_properties_sorbed_component[i]=(char *)Free(names_iteration_properties_sorbed_component[i]);
    }
    names_iteration_properties_sorbed_component=(char **)Free(names_iteration_properties_sorbed_component);

    /* Freigabe der Namen fuer vordefinierte Iterationsobjekte */
        /* fuer names_iteration_properties_immobile_solute_component */
    for (i = 0; i < GetRFProcessNumComponents(); i++) {
      names_iteration_properties_immobile_solute_component[i]=(char *)Free(names_iteration_properties_immobile_solute_component[i]);
    }
    names_iteration_properties_immobile_solute_component=(char **)Free(names_iteration_properties_immobile_solute_component);


    sys_name_iteration_properties=(char *)Free(sys_name_iteration_properties);

    return;
}


/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesFluidPhase

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften der Phase mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer der Phase.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesFluidPhase(int number)
{
  if (number == 0 )
    return name_iteration_properties_fluid_phase;
  else if (number>0 && number <= GetRFProcessNumPhases())
    return names_iteration_properties_fluid_phase[number-1];
  else
    return NULL;
}



/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesSaturation

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften der Saettigung mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer der Saettigung.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesSaturation(int number)
{
  if (number == 0 )
    return name_iteration_properties_saturation;
  else if (number>0 && number <= GetRFProcessNumPhases())
    return names_iteration_properties_saturation[number-1];
  else
    return NULL;
}


/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesTracerComponent

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften des Traecers mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer des Tracers.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesTracerComponent(int number)
{
  if (number == 0 )
    return name_iteration_properties_tracer_component;
  else if (number>0 && number <= GetRFProcessNumComponents())
    return names_iteration_properties_tracer_component[number-1];
  else
    return NULL;
}


/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesTracerComponent

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften des Traecers mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer des Tracers.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesSorbedComponent(int number)
{
  if (number == 0 )
    return name_iteration_properties_sorbed_component;
  else if (number>0 && number <= GetRFProcessNumComponents())
    return names_iteration_properties_sorbed_component[number-1];
  else
    return NULL;
}



/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesTracerComponent

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften des Traecers mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer des Tracers.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesSoluteComponent(int number)
{
  if (number == 0 )
    return name_iteration_properties_solute_component;
  else if (number>0 && number <= GetRFProcessNumComponents())
    return names_iteration_properties_solute_component[number-1];
  else
    return NULL;
}



/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesWaterContent

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften des Wassergehalts mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer des Wassergehalts.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesWaterContent(int number)
{
  if (number == 0 )
    return name_iteration_properties_water_content;
  else if (number>0 && number <= GetRFProcessNumPhases())
    return names_iteration_properties_water_content[number-1];
  else
    return NULL;
}



/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesTemperaturePhase

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften der Temperaturphase mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer der Temperaturphase.

 Ergebnis:
   - s.o -

 Programmaenderungen:
   04/1999     AH         Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesTemperaturePhase(int number)
{
  if (number == 0 )
    return name_iteration_properties_temperature_phase;
  else if (number>0 && number <= GetRFProcessNumPhases())
    return names_iteration_properties_temperature_phase[number-1];
  else
    return NULL;
}


/*************************************************************************
 ROCKFLOW - Funktion: GetDefaultNameIterationPropertiesImmobileSoluteComponent

 Aufgabe:
   Liefert den Default-Namen fuer die linearen Loesereigenschaften des Traecers mit der
   Nummer number.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int number: Nummer des Tracers.

 Ergebnis:
   - s.o -

 Programmaenderungen:
 06/2000    AH      Erste Version

*************************************************************************/
char *GetDefaultNameIterationPropertiesImmobileSoluteComponent(int number)
{
  if (number == 0 )
    return name_iteration_properties_immobile_solute_component;
  else if (number>0 && number <= GetRFProcessNumComponents())
    return names_iteration_properties_immobile_solute_component[number-1];
  else
    return NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: create_iteration_properties
                                                                          */
/* Aufgabe:
   Konstruktor for ITERATION_PROPERTIES
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - Adresse des IP's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
ITERATION_PROPERTIES *create_iteration_properties(char *name)
{
  ITERATION_PROPERTIES *ip;

  ip = (ITERATION_PROPERTIES *) Malloc(sizeof(ITERATION_PROPERTIES));
  if ( ip == NULL ) return NULL;


  ip->name = (char *) Malloc((int)strlen(name)+1);
  if ( ip->name == NULL ) {
         Free(ip);
         return NULL;
  }
  strcpy(ip->name,name);

  ip->type=0;

  ip->count_of_values=0;
  ip->values=NULL;
  ip->distribution_type=0;

  return ip;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_iteration_properties
                                                                          */
/* Aufgabe:
   Destructor for ITERATION_PROPERTIES
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des IP's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_iteration_properties(void *member)
{
  ITERATION_PROPERTIES *ip=(ITERATION_PROPERTIES *)member;

  if ( ip->name ) ip->name=(char *)Free(ip->name);
  if ( ip->values ) ip->values=(double *)Free(ip->values);
  if ( ip ) ip=(ITERATION_PROPERTIES *)Free(ip);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: init_iteration_properties
                                                                          */
/* Aufgabe:
   Initialisator for ITERATION_PROPERTIES
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - Adresse des IP's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
ITERATION_PROPERTIES *init_iteration_properties(ITERATION_PROPERTIES *ip, \
                                                                                        long count_of_values)
{
  if (count_of_values <= 0) return NULL;

  if ( ip->values ) ip->values=(double *)Free(ip->values);
  ip->values=(double *)Malloc(count_of_values*sizeof(double));
  if ( ip->values == NULL ) return NULL;

  ip->count_of_values=count_of_values;
  return ip;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: delete_iteration_properties
                                                                          */
/* Aufgabe:
   Randbedingung loeschen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Adresse des IP's.
                                                                          */
/* Ergebnis:
   - Adresse des IP's -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
ITERATION_PROPERTIES *delete_iteration_properties(ITERATION_PROPERTIES *ip)
{
  if ( ip->values ) ip->values=(double *)Free(ip->values);
  ip->count_of_values=0;

  return ip;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_name
                                                                          */
/* Aufgabe:
   Liefert Name des IP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Name des IP's -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
char *get_ip_name(ITERATION_PROPERTIES *ip)
{
  if (ip->name)
    return ip->name;
  else
        return NULL;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_name
                                                                          */
/* Aufgabe:
   Setzt Name des IP's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_name(ITERATION_PROPERTIES *ip, char *name)
{
  if (!name) return;
  if(ip) {
    ip->name=(char *)Free(ip->name);
    ip->name = (char *) Malloc((int)strlen(name)+1);
    if ( ip->name == NULL ) return;
    strcpy(ip->name,name);
    return;
  }
  else return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_type
                                                                          */
/* Aufgabe:
   Setzt Typ des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
   E long begin_element: Anfangsknoten                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_type(ITERATION_PROPERTIES *ip, long type)
{
  if (ip) ip->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_type
                                                                          */
/* Aufgabe:
   Liefert Typ des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_type(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->type:-1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_maxiter
                                                                          */
/* Aufgabe:
   Setzt maximale Iterationszahl des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E maxiter: maximale Iteration des nichtlinearen iterativen Loesers.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_maxiter(ITERATION_PROPERTIES *ip, long maxiter)
{
  if (ip) ip->maxiter=maxiter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_maxiter
                                                                          */
/* Aufgabe:
   Liefert maximale Iterationszahl des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - maximale Iterationszahl -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_maxiter(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->maxiter:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_criterium
                                                                          */
/* Aufgabe:
   Setzt Kriterium des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E long criterium: Kriterium (Konvergenztyp)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_criterium(ITERATION_PROPERTIES *ip, long criterium)
{
  if (ip) ip->criterium=criterium;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_criterium
                                                                          */
/* Aufgabe:
   Liefert Kriterium des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_criterium(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->criterium:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_norm
                                                                          */
/* Aufgabe:
   Setzt Norm (Konvergenznorm) des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E int norm : Norm
                0: unendliche Norm
                1: Eins-Norm
                2: Euklidische Norm
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_norm(ITERATION_PROPERTIES *ip, int norm)
{
  if (ip) ip->norm=norm;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_norm
                                                                          */
/* Aufgabe:
   Liefert Norm (Konvergenznorm) des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int get_ip_norm(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->norm:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_abs_eps
                                                                          */
/* Aufgabe:
   Setzt absolute Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double abs_eps: absolute Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_abs_eps(ITERATION_PROPERTIES *ip, double abs_eps)
{
  if (ip) ip->abs_eps=abs_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_abs_eps
                                                                          */
/* Aufgabe:
   Liefert absolute Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_abs_eps(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->abs_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_rel_eps
                                                                          */
/* Aufgabe:
   Setzt relative Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double rel_eps: relative Genauigkeit oder Abbruchschranke der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_rel_eps(ITERATION_PROPERTIES *ip, double rel_eps)
{
  if (ip) ip->rel_eps=rel_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_rel_eps
                                                                          */
/* Aufgabe:
   Liefert relative Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_rel_eps(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->rel_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_rel_cg_eps
                                                                          */
/* Aufgabe:
   Setzt die Anfangsgenauigkeit des IP's (fuer variable Genauigkeit).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double rel_cg_eps: Anfangsgenauigkeit der Iteration (fuer variable Genauigkeit)
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_rel_cg_eps(ITERATION_PROPERTIES *ip, double rel_cg_eps)
{
  if (ip) ip->rel_cg_eps=rel_cg_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_rel_cg_eps
                                                                          */
/* Aufgabe:
   Liefert die Anfangsgenauigkeit des IP's (fuer variable Genauigkeit).
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_rel_cg_eps(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->rel_cg_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_theta
                                                                          */
/* Aufgabe:
   Setzt Gewichtsfaktor des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double theta: Gewichtsfaktor fuer nichtlineare iterative Loeser.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_theta(ITERATION_PROPERTIES *ip, double theta)
{
  if (ip) ip->theta=theta;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_theta
                                                                          */
/* Aufgabe:
   Liefert Gewichtsfaktor des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_theta(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->theta:-1.;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_time
                                                                          */
/* Aufgabe:
   Setzt die Zeit der Gueltigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double time: Zeit der Gueltigkeit des Loesers
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_time(ITERATION_PROPERTIES *ip, double time)
{
  if (ip) ip->time=time;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_time
                                                                          */
/* Aufgabe:
   Liefert die Zeit der Gueltigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_time(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->time:-1.;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_kind
                                                                          */
/* Aufgabe:
   Setzt Art der Zeitgueltigkeit des LSP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E long kind: Art der Zeitgueltigkeit
                0  : immer gueltig
                1  : gueltig in einem Zeitintervall
                2  : gueltig fuer einen bestimmten Zeitpunkt
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_kind(ITERATION_PROPERTIES *ip, long kind)
{
  if (ip) ip->kind=kind;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_kind
                                                                          */
/* Aufgabe:
   Liefert Art der Zeitgueltigkeit des LSP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   02/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_kind(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->kind:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_count_of_values
                                                                          */
/* Aufgabe:
   Liefert Anzahl der zugehoerigen Werte des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - Anzahl der zugehoerigen Werte des IP's -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_count_of_values(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->count_of_values:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_count_of_values
                                                                          */
/* Aufgabe:
   Setzt Anzahl der zugehoerigen Werte des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_count_of_values(ITERATION_PROPERTIES *ip, long count_of_values)
{
  if (ip) ip->count_of_values=count_of_values;
  return;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_value
                                                                          */
/* Aufgabe:
   Setzt Anfangswert mit dem Index (index) fuer alle Knoten des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E long step_elements: Knotenschrittweite                                                                           */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_value(ITERATION_PROPERTIES *ip, long index, double value)
{
  if (ip && ip->values) {
    if ( (index<ip->count_of_values) && (index>=0) )
          ip->values[index]=value;
        else
      printf("* !!! Fehler: Unzulaessiger Feldindex (BC) !");
        return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_value
                                                                          */
/* Aufgabe:
   Liefert Anfangswert mit dem Index (index) fuer alle Knoten des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - Anfangswert -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_value(ITERATION_PROPERTIES *ip, long index)
{
  if (ip && ip->values) {
    if ( (index<ip->count_of_values) && (index>=0) )
          return ip->values[index];
        else {
      printf("* !!! Fehler: Unzulaessiger Feldindex (BC) (Returnwert -1)!");
          return -1.;
        }
  }
  else {
    printf("* !!! Fehler: Instanz existiert nicht (BC) (Returnwert -1)!");
        return -1.;
  }
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_distribution_type
                                                                          */
/* Aufgabe:
   Setzt Verteilungstyp des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E long refine_type: Verteilungstyp
                                                                          */
   /* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_distribution_type(ITERATION_PROPERTIES *ip, long distribution_type)
{
  if (ip) ip->distribution_type=distribution_type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_distribution_type
                                                                          */
/* Aufgabe:
   Liefert Verteilungstyp des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - Verteilungstyp -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_distribution_type(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->distribution_type:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_level
                                                                          */
/* Aufgabe:
   Setzt Modus des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E long level: Level
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_level(ITERATION_PROPERTIES *ip, long level)
{
  if (ip) ip->level=level;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_level
                                                                          */
/* Aufgabe:
   Liefert Modus des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - Modus -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long get_ip_level(ITERATION_PROPERTIES *ip)
{
        return (ip)?ip->level:-1;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_var_eps
                                                                          */
/* Aufgabe:
   Setzt die variable Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double var_eps: Variable Genauigkeit oder variable Abbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_var_eps(ITERATION_PROPERTIES *ip, double var_eps)
{
  if (ip) ip->var_eps=var_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_var_eps
                                                                          */
/* Aufgabe:
   Liefert die variable Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_var_eps(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->var_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_begin_eps
                                                                          */
/* Aufgabe:
   Setzt Anfangsgenauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double begin_eps: Anfangsgenauigkeit oder Anfangsabbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_begin_eps(ITERATION_PROPERTIES *ip, double begin_eps)
{
  if (ip) ip->begin_eps=begin_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_begin_eps
                                                                          */
/* Aufgabe:
   Liefert Anfangsgenauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_begin_eps(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->begin_eps:0.0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_end_eps
                                                                          */
/* Aufgabe:
   Setzt Endgenauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double end_eps: Endgenauigkeit oder Endabbruchschranke
                       der Iteration
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_end_eps(ITERATION_PROPERTIES *ip, double end_eps)
{
  if (ip) ip->end_eps=end_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_end_eps
                                                                          */
/* Aufgabe:
   Liefert Endgenauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_end_eps(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->end_eps:0.0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_fac_eps
                                                                          */
/* Aufgabe:
   Setzt Faktor der variablen Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double fac_eps: Faktor der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_fac_eps(ITERATION_PROPERTIES *ip, double fac_eps)
{
  if (ip) ip->fac_eps=fac_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_fac_eps
                                                                          */
/* Aufgabe:
   Liefert Faktor der variablen Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_fac_eps(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->fac_eps:0.0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_ip_exp_eps
                                                                          */
/* Aufgabe:
   Setzt Exponenten der variablen Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
   E double exp_eps: Exponent der variablen Genauigkeit
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void set_ip_exp_eps(ITERATION_PROPERTIES *ip, double exp_eps)
{
  if (ip) ip->exp_eps=exp_eps;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_ip_exp_eps
                                                                          */
/* Aufgabe:
   Liefert Exponenten der variablen Genauigkeit des IP's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf eine Instanz vom Typ
                                        ITERATION_PROPERTIES.
                                                                          */
/* Ergebnis:
   - s.o. -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
double get_ip_exp_eps(ITERATION_PROPERTIES *ip)
{
  return (ip)?ip->exp_eps:0.0;
}


/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/



/**************************************************************************/
/* ROCKFLOW - Funktion: create_iteration_properties_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Randbedingungen;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Listenname
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
LIST_ITERATION_PROPERTIES *create_iteration_properties_list (char *name)
{
  LIST_ITERATION_PROPERTIES *list_of_iteration_properties;

  list_of_iteration_properties = (LIST_ITERATION_PROPERTIES *) Malloc(sizeof(LIST_ITERATION_PROPERTIES));
  if ( list_of_iteration_properties == NULL ) return NULL;


  list_of_iteration_properties->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_iteration_properties->name == NULL ) {
         Free(list_of_iteration_properties);
         return NULL;
  }
  strcpy(list_of_iteration_properties->name,name);

  list_of_iteration_properties->ip_list=create_list();
  if ( list_of_iteration_properties->ip_list == NULL ) {
    Free(list_of_iteration_properties->name);
        Free(list_of_iteration_properties);
        return NULL;
  }

  list_of_iteration_properties->names_of_iteration_properties=NULL;
  list_of_iteration_properties->count_of_iteration_properties_name=0;

  return list_of_iteration_properties;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_iteration_properties_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der Randbedingungen aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_iteration_properties_list(void)
{
  if (list_of_iteration_properties->name) list_of_iteration_properties->name=(char *)Free(list_of_iteration_properties->name);
  if(list_of_iteration_properties->names_of_iteration_properties) \
    list_of_iteration_properties->names_of_iteration_properties = \
      (char **)Free(list_of_iteration_properties->names_of_iteration_properties);

  if (list_of_iteration_properties->ip_list) {
    delete_list(list_of_iteration_properties->ip_list,destroy_iteration_properties);
    list_of_iteration_properties->ip_list=destroy_list(list_of_iteration_properties->ip_list);
  }

  if (list_of_iteration_properties) list_of_iteration_properties=(LIST_ITERATION_PROPERTIES *)Free(list_of_iteration_properties);

  return;
}

/**************************************************************************/
/* ROCKFLOW - Funktion: iteration_properties_list_empty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 Liste --> leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int iteration_properties_list_empty(void)
{
  if (!list_of_iteration_properties) return 0;
  else if (!list_of_iteration_properties->ip_list) return 0;
  else return list_empty(list_of_iteration_properties->ip_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_ip_init
                                                                          */
/* Aufgabe:
   Liste der IP's initialisieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void list_ip_init(void)
{
  if (list_of_iteration_properties) list_current_init(list_of_iteration_properties->ip_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_ip_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_ip_next (void)
{
  return list_of_iteration_properties->ip_list ?
             get_list_next(list_of_iteration_properties->ip_list): NULL;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: declare_iteration_properties_name
                                                                          */
/* Aufgabe:
   Bekanntmachung der Randbedingung mit dem Name name.
   Fuegt einen Typnamen in die Namensliste ein.
   Es wird keine Instanz vom Typ ITERATION_PROPERTIES erzeugt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IP's_Namen -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long declare_iteration_properties_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  if (!list_of_iteration_properties) return 0;
  for ( i=0; i<list_of_iteration_properties->count_of_iteration_properties_name; i++)
    if(strcmp(list_of_iteration_properties->names_of_iteration_properties[i],name) == 0) break;
  if (i < list_of_iteration_properties->count_of_iteration_properties_name) return i;

  list_of_iteration_properties->names_of_iteration_properties= \
          (char **) Realloc(list_of_iteration_properties->names_of_iteration_properties, \
          (list_of_iteration_properties->count_of_iteration_properties_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_iteration_properties->names_of_iteration_properties[i]=new_name;

  return ++(list_of_iteration_properties->count_of_iteration_properties_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_iteration_properties_names
                                                                          */
/* Aufgabe:
   Alle Randbedingungen von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen IP's_Namen -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_iteration_properties_names (void)
{
  long i;

  for (i=0; i<list_of_iteration_properties->count_of_iteration_properties_name; i++)
    if(list_of_iteration_properties->names_of_iteration_properties[i]) \
      list_of_iteration_properties->names_of_iteration_properties[i] = \
        (char *)Free(list_of_iteration_properties->names_of_iteration_properties[i]);

  return list_of_iteration_properties->count_of_iteration_properties_name;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: insert_iteration_properties_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ ITERATION_PROPERTIES in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long insert_iteration_properties_list (ITERATION_PROPERTIES *ip)
{
  if(!list_of_iteration_properties) return 0;
  list_of_iteration_properties->count_of_iteration_properties=append_list(list_of_iteration_properties->ip_list,(void *) ip);
  return list_of_iteration_properties->count_of_iteration_properties;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: remove_iteration_properties_list
                                                                          */
/* Aufgabe:
   Loescht und zerstoert eine Instanz vom Typ ITERATION_PROPERTIES der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur,
                              die geloescht werden soll.
                                                                          */
/* Ergebnis:
   Anzahl verbleibender ITERATION_PROPERTIES-Objekte
                                                                          */
/* Programmaenderungen:
   03/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
long remove_iteration_properties_list (ITERATION_PROPERTIES *ip)
{
  list_of_iteration_properties->count_of_iteration_properties = \
      remove_list_member(list_of_iteration_properties->ip_list,(void *) ip, destroy_iteration_properties);
  return list_of_iteration_properties->count_of_iteration_properties;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: iteration_properties_name_exist
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long iteration_properties_name_exist (char *name)
{
  long i;

  if (!name) return 0;

  for ( i=0; i<list_of_iteration_properties->count_of_iteration_properties_name; i++)
    if(strcmp(list_of_iteration_properties->names_of_iteration_properties[i],name) == 0) break;
  if (i < list_of_iteration_properties->count_of_iteration_properties_name) return ++i;
  else return 0;
}




/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateIterationPropertiesList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Randbedingungen (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void CreateIterationPropertiesList(void)
{
  list_of_iteration_properties = create_iteration_properties_list("ITERATION_PROPERTIES");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyIterationPropertiesList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von Randbedingungen (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void DestroyIterationPropertiesList(void)
{
  long i;

  //OK_BC if (ip_matrix_values) DestroyDoubleMatrix(ip_matrix_values);
  //OK_BC if (ip_matrix_iter) DestroyLongMatrix(ip_matrix_iter);
  FreeDefaultIterationPropertiesNames();
  /* ah zv */
  for(i=0; i<ip_names_number; i++)
    if (ip_names[i]) Free(ip_names[i]);
  if (ip_names) Free(ip_names);

  undeclare_iteration_properties_names();
  destroy_iteration_properties_list();
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: IterationPropertiesListEmpty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 --> Liste leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int IterationPropertiesListEmpty(void)
{
  return iteration_properties_list_empty();
}



/**************************************************************************/
/* ROCKFLOW - Funktion: SetIterationProperties
                                                                          */
/* Aufgabe:
   Setzt den Randbedingung in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
void SetIterationProperties (char *name)
{
  char *new_name;

  ip_names=(char **) Realloc(ip_names,(ip_names_number+2)*sizeof(char **) );
  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  ip_names[ip_names_number]=new_name;
  ip_names[++ip_names_number]=NULL;

  return;
}






/**************************************************************************/
/* ROCKFLOW - Funktion: IndexIterationProperties
                                                                          */
/* Aufgabe:
   Prueft auf Existenz eines BC-Namens.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des IP's.
                                                                          */
/* Ergebnis:
   - Plazierung in der Namenliste, ansonsten 0 -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
long IndexIterationProperties(char *name)
{
  long i;

  if (!name) return -1;

  for ( i=0; i<ip_names_number; i++)
    if(strcmp(ip_names[i],name) == 0) break;
  if (i < ip_names_number) return i;
  else return -1;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: GetIterationPropertiesGroup
                                                                          */
/* Aufgabe:
   Liefert den Zeiger auf Datentyp ITERATION_PROPERTIES
   mit dem Gruppennamen NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name              - Gruppenname
   E: ITERATION_PROPERTIES *ip -
                                                                          */
/* Ergebnis:
   Zeiger auf Datentyp ITERATION_PROPERTIES
                                                                          */
/* Programmaenderungen:
   03/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
ITERATION_PROPERTIES *GetIterationPropertiesGroup(char *name,ITERATION_PROPERTIES *ip)
{
  if (!name) return NULL;
  if (!iteration_properties_name_exist(name) ) return NULL;

  if(ip==NULL) list_ip_init();

  while ( (ip=(ITERATION_PROPERTIES *)get_list_ip_next()) != NULL ) {
    if ( strcmp(StrUp(get_ip_name(ip)),StrUp(name))==0 ) break;
  }

  return ip;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyIterationPropertiesListGroup
                                                                          */
/* Aufgabe:
   Zerstoert saemtliche ITERATION_PROPERTIES-Objekte der Gruppe NAME
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *name - Gruppenname
                                                                          */
/* Ergebnis:
   Kontrollflag
                                                                          */
/* Programmaenderungen:
   03/1999     OK         Erste Version
                                                                          */
/**************************************************************************/
int DestroyIterationPropertiesListGroup(char *name)
{
  ITERATION_PROPERTIES *ip=NULL;

  if (!name) return -1;
  if (!iteration_properties_name_exist(name) ) return -1;

  list_ip_init();

  while ( (ip=(ITERATION_PROPERTIES *)get_list_ip_next()) != NULL ) {
    if ( strcmp(StrUp(get_ip_name(ip)),StrUp(name))==0 )
        remove_iteration_properties_list(ip);
  }

  return 0;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteIterationProperties
                                                                          */
/* Aufgabe:
   Liefert Wert der Randbedingung am Knoten mit dem index i.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   08/1999     AH        Erste Version
                                                                          */
/**************************************************************************/
int ExecuteIterationProperties (char *name, double *current, double *last,
                                double *ref, long dim)
{
  ITERATION_PROPERTIES *ip=NULL;
  int ret=0,found=0;
  double cur_eps=0.0,last_eps=0.0;
  long col;
  long cur_iter,max_iter;
  double base=10.0;

  if (!name) { return 0; }
  if (IterationPropertiesListEmpty()) { return 0; }
  if ( !iteration_properties_name_exist(name) ) { return 0; }

  col=IndexIterationProperties(name);
  if (col < 0) return ret;

  list_ip_init();
  while ( (ip=(ITERATION_PROPERTIES *)get_list_ip_next()) != NULL ) {
    if ( strcmp(StrUp(get_ip_name(ip)),StrUp(name)) ) continue;
    found=0;
    switch (get_ip_type(ip)) {
          case 0:
            found=1;
            ret=!ExecuteIterationPropertiesMethod0(ip,current,last,dim,&cur_eps);
            break;
          case 1:
            found=1;
            ret=!ExecuteIterationPropertiesMethod1(ip,current,last,ref,dim,&cur_eps);
            break;
          case 2:
            found=1;
            if(ip_matrix_iter->m[0][col] == 0) set_ip_var_eps(ip,get_ip_begin_eps(ip));
            else {
              set_ip_var_eps(ip,get_ip_var_eps(ip)*get_ip_fac_eps(ip)*pow(base,get_ip_exp_eps(ip)));
            }
            ret=!ExecuteIterationPropertiesMethod2(ip,current,last,ref,dim,&cur_eps);
            break;
        } /* switch */
        if (found) {
          ip_matrix_iter->m[0][col]++;
          cur_iter=ip_matrix_iter->m[0][col];
          last_eps=ip_matrix_values->m[0][col];
          ip_matrix_values->m[0][col]=cur_eps;
          if (fabs(last_eps)< MKleinsteZahl) ip_matrix_values->m[1][col]=cur_eps;
          else ip_matrix_values->m[1][col]=cur_eps/last_eps;
          max_iter=get_ip_maxiter(ip);
          if (cur_iter>max_iter && get_ip_kind(ip) == 0) {
            DisplayErrorMsg("Maximale Anzahl an Modell-Iterationen erreicht.");
            ret=0;
          }
          return ret;
        }
  } /* while */
  return ret;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetIterationNumberIterationProperties
                                                                          */
/* Aufgabe:
   Liefert Anzahl der ausgefuehrten Iterationen.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Identifikatorname des Prozesses.
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   12/1999     AH        Erste Version
                         Funktion noch nicht abgeschlossen (max_num_iter)
                                                                          */
/**************************************************************************/
long GetIterationNumberIterationProperties (char *name)
{
  ITERATION_PROPERTIES *ip=NULL;
  int found=0;
  long col;
  long max_num_iter=0,num_iter=0;

  if (!name) { return 0; }
  if (IterationPropertiesListEmpty()) { return 0; }
  if ( !iteration_properties_name_exist(name) ) { return 0; }

  col=IndexIterationProperties(name);
  if (col < 0) return found;

  list_ip_init();
  while ( (ip=(ITERATION_PROPERTIES *)get_list_ip_next()) != NULL ) {
    if ( strcmp(StrUp(get_ip_name(ip)),StrUp(name)) ) continue;
    found=0;
    switch (get_ip_type(ip)) {
          case 0:
          case 1:
          case 2:
            found=1;
            break;
        } /* switch */
        if (found) {
          num_iter=ip_matrix_iter->m[0][col];
          max_num_iter=max(num_iter,max_num_iter);
        }
  } /* while */
  return max_num_iter;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: GetConvergenceRateIterationProperties
                                                                          */
/* Aufgabe:
   Liefert die Konvergenzrate der ausgefuehrten Iteration.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Identifikatorname des Prozesses.
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   12/1999     AH        Erste Version
                         Funktion noch nicht abgeschlossen (max_cur_eps)
                                                                          */
/**************************************************************************/
double GetConvergenceRateIterationProperties (char *name)
{
  ITERATION_PROPERTIES *ip=NULL;
  int found=0;
  double max_cur_eps=0.0,cur_eps=0.0;
  long col;

  if (!name) { return 0.; }
  if (IterationPropertiesListEmpty()) { return 0.; }
  if ( !iteration_properties_name_exist(name) ) { return 0.; }

  col=IndexIterationProperties(name);
  if (col < 0) return 0.0;

  list_ip_init();
  while ( (ip=(ITERATION_PROPERTIES *)get_list_ip_next()) != NULL ) {
    if ( strcmp(StrUp(get_ip_name(ip)),StrUp(name)) ) continue;
    switch (get_ip_type(ip)) {
          case 0:
          case 1:
          case 2:
             cur_eps=ip_matrix_values->m[0][col];
             max_cur_eps=max(cur_eps,max_cur_eps);
             found=1;
             break;
    } /* switch */
  } /* while */
  if (found) return max_cur_eps;
  else return -1.;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitIterationProperties
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   08/1999     AH        Erste Version
                                                                          */
/**************************************************************************/
void InitIterationProperties (char *name)
{
  long i,n=2;
  long col=IndexIterationProperties(name);

  if (col<0) return;

  for (i=0; i<n; i++) {
    ip_matrix_values->m[i][col]=0.0;
    ip_matrix_iter->m[i][col]=0;
  }

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: InitAllIterationProperties
                                                                          */
/* Aufgabe:
   Setzt alle Randbedingungsknoten auf Null.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int zeitkurve: Zeitkurve, >= 0
   E double zeitpunkt: Zeitpunkt, >= 0.0
                                                                          */
/* Ergebnis:
   s.o.
                                                                          */
/* Programmaenderungen:
   08/1999     AH        Erste Version
                                                                          */
/**************************************************************************/
void InitAllIterationProperties (void)
{
  long i,j,n;
  long ncol=ip_names_number;

  if (ncol<=0) return;

  n=2;
  //OK_BC if (ip_matrix_values) DestroyDoubleMatrix(ip_matrix_values);
  //OK_BC ip_matrix_values=CreateDoubleMatrix(n,ncol);
  //OK_BC if (ip_matrix_iter) DestroyLongMatrix(ip_matrix_iter);
  //OK_BC ip_matrix_iter=CreateLongMatrix(n,ncol);
  for (i=0; i<n; i++)
    for (j=0; j<ncol; j++) {
      ip_matrix_values->m[i][j]=0.0;
      ip_matrix_iter->m[i][j]=0;
    }

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteIterationPropertiesMethod0
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int ExecuteIterationPropertiesMethod0 (ITERATION_PROPERTIES *ip, double *current, double *last,
                                       long dim, double *cur_eps)
{
  double eps=get_ip_abs_eps(ip);
  double error=MVekDist(current,last,dim);

  *cur_eps=error;
  if (*cur_eps <= eps) return 1;
  else return 0;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteIterationPropertiesMethod1
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version
                                                                          */
/**************************************************************************/
int ExecuteIterationPropertiesMethod1 (ITERATION_PROPERTIES *ip,
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps)
{
  double eps=get_ip_rel_eps(ip);
  double error=MVekDist(current,last,dim);
  double d=MBtrgVec(ref,dim);

  if (fabs(d)< MKleinsteZahl) *cur_eps=error;
  else *cur_eps=error/d;
  if (*cur_eps <= eps) return 1;
  else return 0;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteIterationPropertiesMethod2
                                                                          */
/* Aufgabe:
   Setzt einen Knoten-Randbedingung (Eingabe in Knotennummer)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ITERATION_PROPERTIES *ip: Zeiger auf die Datenstruktur bc.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999     AH         Erste Version  Noch zu ergaenzen !!!
                                                                          */
/**************************************************************************/
int ExecuteIterationPropertiesMethod2 (ITERATION_PROPERTIES *ip,
                                       double *current, double *last, double *ref,
                                       long dim, double *cur_eps)
{
  double eps=get_ip_var_eps(ip);
  double error=MVekDist(current,last,dim);
  double d=MBtrgVec(ref,dim);

  if (fabs(d)< MKleinsteZahl) *cur_eps=error;
  else *cur_eps=error/d;
  if (*cur_eps <= eps) return 1;
  else return 0;
}


/*------------------------------------------------------------------------*/
/* Lesefunktionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationProperties
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES gehoerigen Daten
   ein und erstellt den zugehoerigen Protokollabschnitt.
   #ITERATION_PROPERTIES: Iterations-Parameter fuer alle Modelle.
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
   08/1999     AH      erste Version
                                                                          */
/**************************************************************************/
int FctIterationProperties(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    char ip_name[255]="ROCKFLOW";
    double d;
    long l;
    int i=0;
    ITERATION_PROPERTIES *ip;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif
    if (!found) {           /* ITERATION_PRESSURE nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f, "; Schluesselwort: #ITERATION_PROPERTIES (ITERATION)");
      LineFeed(f);
#endif
      if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
        ok = 0;
      }
      else {
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        set_ip_type(ip,1);      /* Picard */
        set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
        set_ip_criterium(ip,2); /* Kriterium */
        set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
        set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
        set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
        set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
        set_ip_kind(ip,0);      /* Immer gueltig */
        /* IP-Objekt in der Liste einfuegen */
        insert_iteration_properties_list(ip);
      }
    } /* found */
    else {                /* ITERATION_PRESSURE gefunden */
      FilePrintString(f, "#ITERATION_PROPERTIES");
      LineFeed(f);
#ifdef EXT_RFD
      FilePrintString(f, "; Schluesselwort: #ITERATION_PROPERTIES (ITERATION)");
      LineFeed(f);
      FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein");
      LineFeed(f);
      LineFeed(f);
#endif
        while (StrTestHash(&data[p], &pos)) {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          i++;
          LineFeed(f);
          while (StrTestLong(&data[p+=pos])) {
            if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
              ok = 0;
              break;
            }
            else {
              ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
              set_ip_type(ip,0);
            }
            if ( get_ip_type(ip) == 0 ) {
              ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
              set_ip_maxiter(ip,l);/* Max. Anzahl an Iterationen */
              ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
              set_ip_criterium(ip,l); /* Kriterium */
              ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
              set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
              ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
              set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
              ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
              set_ip_rel_cg_eps(ip,d);   /* var. Fehlerschranke */
              LineFeed(f);
            }
            if (ip) {
              insert_iteration_properties_list(ip);
            }
            LineFeed(f);
          } /* while long */
        }  /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Nichtlinearer Gleichungsloeser (nonlinear_method_flow) [1..2;1]");
            LineFeed(f);
            FilePrintString(f, ";   1: PICARD; 2: MOD. NEWTON-RAPHSON");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer nichtlinearen Gleichungsloeser");
            LineFeed(f);
            FilePrintString(f, ";   (nonlinear_maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, "; - Konvergenztyp : (nonlinear_convergence_type_flow) [1,2;1] ");
            LineFeed(f);
            FilePrintString(f, ";   1: Differenz in der Loesung");
            LineFeed(f);
            FilePrintString(f, ";   2: Differenz im Residuum");
            LineFeed(f);
            FilePrintString(f, "; - Absolute Fehlerschranke fuer nichtlinearen Gleichungsloeser");
            LineFeed(f);
            FilePrintString(f, ";   (nonlinear_abs_eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, "; - Relative Fehlerschranke fuer nichtlinearen Gleichungsloeser");
            LineFeed(f);
            FilePrintString(f, ";   (nonlinear_rel_eps_flow) [>0.0;1.]");
            LineFeed(f);
            FilePrintString(f, "; - Anzahl der Iteration fuer den Wiederaufbau der globalen Matrix");
            LineFeed(f);
            FilePrintString(f, ";   (nonlinear_assemble_flow)");
            LineFeed(f);
            FilePrintString(f, "; - Relative Fehlerschranke fuer CG-Gleichungsloeser");
            LineFeed(f);
            FilePrintString(f, ";   (nonlinear_rel_cg_eps_flow) [>0.0;1.e-3]");
            LineFeed(f);
            FilePrintString(f, ";   Nur fuer AbsoluteFehlerschranke=0. !!!! ");
            LineFeed(f);
#endif
   }
    return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesPressure
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES_PRESSURE
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   ITERATION_PROPERTIES_PRESSURE: Iterations-Parameter fuer die Druecke.
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
   08/1999    AH         Erste Version
                                                                          */
/**************************************************************************/
int FctIterationPropertiesPressure ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],string[255];
    ITERATION_PROPERTIES *ip;
    int i=0,k;


#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_PRESSURE");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.? Iterationen  ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"*** C?.?.? Iterationsparameter ***");
      LineFeed(f);
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_PRESSURE (ITERATION_PROPERTIES)");
      LineFeed(f);
      FilePrintString(f,"#ITERATION_PROPERTIES_PRESSURE");
      LineFeed(f);
#endif
      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
    } /* if !found */
    else {   /* Schluesselwort gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_PRESSURE (ITERATION_PROPERTIES)"); LineFeed(f);
      FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann"); LineFeed(f);
      FilePrintString(f,"; mit Default-Werte gearbeitet."); LineFeed(f);
#endif
      while (StrTestHash(&data[p], &pos)) {
        sprintf(ip_name,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0),i+1);
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        i++;
        while (StrTestLong(&data[p+=pos])) {
          if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
            ok = 0;
            break;
          }
          else {
            ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
          }

          if ( get_ip_type(ip) == 0 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 1 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 2 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_begin_eps(ip,d);    /* Anfangsfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_end_eps(ip,d);      /* Endfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_fac_eps(ip,d);      /* Faktor fuer die var. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_exp_eps(ip,d);      /* Exponenet fuer die var. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if (ip) {
            insert_iteration_properties_list(ip);
          }
          LineFeed(f);
        } /* while */
      } /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Gleichungsloeser (loeser_flow) [1..6;6]");
            LineFeed(f);
            FilePrintString(f, ";   1: SpGAUSS; 2:SpBICGSTAB; 3:SpBICG;     4:SpQMRCGSTAB; 5: SpCG");
            LineFeed(f);
            FilePrintString(f, ";   6: SpCGNR;  7:CGS,        8: SpRichard; 9:SpJOR;      10:SpSOR");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer Gleichungsloeser (maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlerschranke fuer Gleichungsloeser (eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Vorkonditionierer fuer Gleichungsloeser (vorkond_flow) [0,1;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: keine Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   1: Diagonalen-Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Speichertechnik fuer Gesamtgleichungssystem (speichertechnik_flow) [1,2;2]");
            LineFeed(f);
            FilePrintString(f, ";   1: vollbesetzte Matrix");
            LineFeed(f);
            FilePrintString(f, ";   2: Sparse (nur A[i,j]!=0.0 werden gespeichert)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlertyp (linear_error_type_flow) [0..4;2]");
            LineFeed(f);
            FilePrintString(f, ";   0: abs. ; 1:rel. zu B0; 2:rel. zu R0; 3:abs. und/oder rel.; 4:R/x");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 3 : Kombination zwischen 0 und 2");
            LineFeed(f);
            FilePrintString(f, ";                    ist |R0| < 1 dann wird Fehlertyp 0 betrachtet,");
            LineFeed(f);
            FilePrintString(f, ";                    ansonsten ist Fehlertyp 2 massgebend.");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 4 : Vektornorm des aktuellen Residuums");
            LineFeed(f);
            FilePrintString(f, ";                    wird mit der aktuellen Loesung normiert");
            LineFeed(f);
            FilePrintString(f, "; - Daempfungs-/Relaxationsfaktor fuer iterative Gleichungsloeser (gls_iter_theta) [>0.0;1.0]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern: Richardson,JOR,SOR)");
            LineFeed(f);
            FilePrintString(f, ";     JOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> gedaempfte Jacobi-Verfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Jacobi oder Gesamtschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> Jacobi Ueberrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     SOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> sukzessive Unterrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Gauss-Seidel oder Einzelschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> sukzessive Ueberrelaxation");
            LineFeed(f);
#endif
    }
  return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesPressureV0
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES_PRESSURE
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   ITERATION_PROPERTIES_PRESSURE: Iterations-Parameter fuer die Druecke.
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
   08/1999    AH         Erste Version
                                                                          */
/**************************************************************************/
int FctIterationPropertiesPressureV0 ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],string[255];
    ITERATION_PROPERTIES *ip;
    int i=0;


#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_PRESSURE");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.? Iterationen  ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"*** C?.?.? Iterationsparameter ***");
      LineFeed(f);
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_PRESSURE (ITERATION_PROPERTIES)");
      LineFeed(f);
      FilePrintString(f,"#ITERATION_PROPERTIES_PRESSURE");
      LineFeed(f);
#endif
      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
    } /* if !found */
    else {   /* Schluesselwort gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_PRESSURE (ITERATION_PROPERTIES)"); LineFeed(f);
      FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann"); LineFeed(f);
      FilePrintString(f,"; mit Default-Werte gearbeitet."); LineFeed(f);
#endif
      while (StrTestHash(&data[p], &pos)) {
        sprintf(ip_name,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0),i+1);
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        i++;
        while (StrTestLong(&data[p+=pos])) {
          if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
            ok = 0;
            break;
          }
          else {
            ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
          }

          if ( get_ip_type(ip) == 0 ) {
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);/* Max. Anzahl an Iterationen */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_criterium(ip,l); /* Kriterium */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_cg_eps(ip,d);   /* var. Fehlerschranke */
            LineFeed(f);
          }

          if (ip) {
            insert_iteration_properties_list(ip);
          }
          LineFeed(f);
        } /* while */
      } /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Gleichungsloeser (loeser_flow) [1..6;6]");
            LineFeed(f);
            FilePrintString(f, ";   1: SpGAUSS; 2:SpBICGSTAB; 3:SpBICG;     4:SpQMRCGSTAB; 5: SpCG");
            LineFeed(f);
            FilePrintString(f, ";   6: SpCGNR;  7:CGS,        8: SpRichard; 9:SpJOR;      10:SpSOR");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer Gleichungsloeser (maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlerschranke fuer Gleichungsloeser (eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Vorkonditionierer fuer Gleichungsloeser (vorkond_flow) [0,1;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: keine Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   1: Diagonalen-Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Speichertechnik fuer Gesamtgleichungssystem (speichertechnik_flow) [1,2;2]");
            LineFeed(f);
            FilePrintString(f, ";   1: vollbesetzte Matrix");
            LineFeed(f);
            FilePrintString(f, ";   2: Sparse (nur A[i,j]!=0.0 werden gespeichert)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlertyp (linear_error_type_flow) [0..4;2]");
            LineFeed(f);
            FilePrintString(f, ";   0: abs. ; 1:rel. zu B0; 2:rel. zu R0; 3:abs. und/oder rel.; 4:R/x");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 3 : Kombination zwischen 0 und 2");
            LineFeed(f);
            FilePrintString(f, ";                    ist |R0| < 1 dann wird Fehlertyp 0 betrachtet,");
            LineFeed(f);
            FilePrintString(f, ";                    ansonsten ist Fehlertyp 2 massgebend.");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 4 : Vektornorm des aktuellen Residuums");
            LineFeed(f);
            FilePrintString(f, ";                    wird mit der aktuellen Loesung normiert");
            LineFeed(f);
            FilePrintString(f, "; - Daempfungs-/Relaxationsfaktor fuer iterative Gleichungsloeser (gls_iter_theta) [>0.0;1.0]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern: Richardson,JOR,SOR)");
            LineFeed(f);
            FilePrintString(f, ";     JOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> gedaempfte Jacobi-Verfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Jacobi oder Gesamtschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> Jacobi Ueberrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     SOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> sukzessive Unterrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Gauss-Seidel oder Einzelschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> sukzessive Ueberrelaxation");
            LineFeed(f);
#endif
    }
  return ok;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesConcentration
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES_CONCENTRATION
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   ITERATION_PROPERTIES_CONCENTRATION: Iterations-Parameter fuer die
                                       Konzentrationen.
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
   08/1999    AH         Erste Version
                                                                          */
/**************************************************************************/
int FctIterationPropertiesConcentration ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],string[255];
    ITERATION_PROPERTIES *ip;
    int i=0,k;


#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_CONCENTRATION");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.? Iterationen  ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"*** C?.?.? Iterationsparameter ***");
      LineFeed(f);
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_CONCENTRATION (ITERATION_PROPERTIES)");
      LineFeed(f);
      FilePrintString(f,"#ITERATION_PROPERTIES_CONCENTRATION");
      LineFeed(f);
#endif
      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
    } /* if !found */
    else {   /* Schluesselwort gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_CONCENTRATION (ITERATION_PROPERTIES)"); LineFeed(f);
      FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann"); LineFeed(f);
      FilePrintString(f,"; mit Default-Werte gearbeitet."); LineFeed(f);
#endif
      while (StrTestHash(&data[p], &pos)) {
        sprintf(ip_name,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        i++;
        while (StrTestLong(&data[p+=pos])) {
          if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
            ok = 0;
            break;
          }
          else {
            ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
          }

          if ( get_ip_type(ip) == 0 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 1 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 2 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_begin_eps(ip,d);    /* Anfangsfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_end_eps(ip,d);      /* Endfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_fac_eps(ip,d);      /* Faktor fuer die var. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_exp_eps(ip,d);      /* Exponenet fuer die var. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if (ip) {
            insert_iteration_properties_list(ip);
          }
          LineFeed(f);
        } /* while */
      } /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Gleichungsloeser (loeser_flow) [1..6;6]");
            LineFeed(f);
            FilePrintString(f, ";   1: SpGAUSS; 2:SpBICGSTAB; 3:SpBICG;     4:SpQMRCGSTAB; 5: SpCG");
            LineFeed(f);
            FilePrintString(f, ";   6: SpCGNR;  7:CGS,        8: SpRichard; 9:SpJOR;      10:SpSOR");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer Gleichungsloeser (maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlerschranke fuer Gleichungsloeser (eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Vorkonditionierer fuer Gleichungsloeser (vorkond_flow) [0,1;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: keine Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   1: Diagonalen-Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Speichertechnik fuer Gesamtgleichungssystem (speichertechnik_flow) [1,2;2]");
            LineFeed(f);
            FilePrintString(f, ";   1: vollbesetzte Matrix");
            LineFeed(f);
            FilePrintString(f, ";   2: Sparse (nur A[i,j]!=0.0 werden gespeichert)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlertyp (linear_error_type_flow) [0..4;2]");
            LineFeed(f);
            FilePrintString(f, ";   0: abs. ; 1:rel. zu B0; 2:rel. zu R0; 3:abs. und/oder rel.; 4:R/x");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 3 : Kombination zwischen 0 und 2");
            LineFeed(f);
            FilePrintString(f, ";                    ist |R0| < 1 dann wird Fehlertyp 0 betrachtet,");
            LineFeed(f);
            FilePrintString(f, ";                    ansonsten ist Fehlertyp 2 massgebend.");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 4 : Vektornorm des aktuellen Residuums");
            LineFeed(f);
            FilePrintString(f, ";                    wird mit der aktuellen Loesung normiert");
            LineFeed(f);
            FilePrintString(f, "; - Daempfungs-/Relaxationsfaktor fuer iterative Gleichungsloeser (gls_iter_theta) [>0.0;1.0]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern: Richardson,JOR,SOR)");
            LineFeed(f);
            FilePrintString(f, ";     JOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> gedaempfte Jacobi-Verfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Jacobi oder Gesamtschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> Jacobi Ueberrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     SOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> sukzessive Unterrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Gauss-Seidel oder Einzelschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> sukzessive Ueberrelaxation");
            LineFeed(f);
#endif
    }
  return ok;
}





/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesSorbedConcentration
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES_CONCENTRATION
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   ITERATION_PROPERTIES_CONCENTRATION: Iterations-Parameter fuer die
                                       Konzentrationen.
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
   08/1999    AH         Erste Version
                                                                          */
/**************************************************************************/
int FctIterationPropertiesSorbedConcentration ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],string[255];
    ITERATION_PROPERTIES *ip;
    int i=0,k;


#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_SORBED_CONCENTRATION");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.? Iterationen  ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"*** C?.?.? Iterationsparameter ***");
      LineFeed(f);
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_SORBED_CONCENTRATION (ITERATION_PROPERTIES)");
      LineFeed(f);
      FilePrintString(f,"#ITERATION_PROPERTIES_SORBED_CONCENTRATION");
      LineFeed(f);
#endif
      for (i=0; i<GetRFProcessNumComponents(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_SORBED_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
    } /* if !found */
    else {   /* Schluesselwort gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_SORBED_CONCENTRATION (ITERATION_PROPERTIES)"); LineFeed(f);
      FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann"); LineFeed(f);
      FilePrintString(f,"; mit Default-Werte gearbeitet."); LineFeed(f);
#endif
      while (StrTestHash(&data[p], &pos)) {
        sprintf(ip_name,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_SORBED_COMPONENT(0),i+1);
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        i++;
        while (StrTestLong(&data[p+=pos])) {
          if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
            ok = 0;
            break;
          }
          else {
            ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
          }

          if ( get_ip_type(ip) == 0 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 1 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 2 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_begin_eps(ip,d);    /* Anfangsfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_end_eps(ip,d);      /* Endfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_fac_eps(ip,d);      /* Faktor fuer die var. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_exp_eps(ip,d);      /* Exponenet fuer die var. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if (ip) {
            insert_iteration_properties_list(ip);
          }
          LineFeed(f);
        } /* while */
      } /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Gleichungsloeser (loeser_flow) [1..6;6]");
            LineFeed(f);
            FilePrintString(f, ";   1: SpGAUSS; 2:SpBICGSTAB; 3:SpBICG;     4:SpQMRCGSTAB; 5: SpCG");
            LineFeed(f);
            FilePrintString(f, ";   6: SpCGNR;  7:CGS,        8: SpRichard; 9:SpJOR;      10:SpSOR");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer Gleichungsloeser (maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlerschranke fuer Gleichungsloeser (eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Vorkonditionierer fuer Gleichungsloeser (vorkond_flow) [0,1;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: keine Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   1: Diagonalen-Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Speichertechnik fuer Gesamtgleichungssystem (speichertechnik_flow) [1,2;2]");
            LineFeed(f);
            FilePrintString(f, ";   1: vollbesetzte Matrix");
            LineFeed(f);
            FilePrintString(f, ";   2: Sparse (nur A[i,j]!=0.0 werden gespeichert)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlertyp (linear_error_type_flow) [0..4;2]");
            LineFeed(f);
            FilePrintString(f, ";   0: abs. ; 1:rel. zu B0; 2:rel. zu R0; 3:abs. und/oder rel.; 4:R/x");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 3 : Kombination zwischen 0 und 2");
            LineFeed(f);
            FilePrintString(f, ";                    ist |R0| < 1 dann wird Fehlertyp 0 betrachtet,");
            LineFeed(f);
            FilePrintString(f, ";                    ansonsten ist Fehlertyp 2 massgebend.");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 4 : Vektornorm des aktuellen Residuums");
            LineFeed(f);
            FilePrintString(f, ";                    wird mit der aktuellen Loesung normiert");
            LineFeed(f);
            FilePrintString(f, "; - Daempfungs-/Relaxationsfaktor fuer iterative Gleichungsloeser (gls_iter_theta) [>0.0;1.0]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern: Richardson,JOR,SOR)");
            LineFeed(f);
            FilePrintString(f, ";     JOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> gedaempfte Jacobi-Verfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Jacobi oder Gesamtschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> Jacobi Ueberrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     SOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> sukzessive Unterrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Gauss-Seidel oder Einzelschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> sukzessive Ueberrelaxation");
            LineFeed(f);
#endif
    }
  return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesSaturation
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort #ITERATION_PROPERTIES_SATURATION
   gehoerigen Daten ein und erstellt den zugehoerigen Protokollabschnitt.
   ITERATION_PROPERTIES_SATURATION: Iterations-Parameter fuer die
                                    Saettigung der einzelnen Phasen.
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
   08/1999    AH         Erste Version
                                                                          */
/**************************************************************************/
int FctIterationPropertiesSaturation ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],string[255];
    ITERATION_PROPERTIES *ip;
    int i=0,k;


#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_SATURATION");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.? Iterationen  ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"*** C?.?.? Iterationsparameter ***");
      LineFeed(f);
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_SATURATION (ITERATION_PROPERTIES)");
      LineFeed(f);
      FilePrintString(f,"#ITERATION_PROPERTIES_SATURATION");
      LineFeed(f);
#endif
      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
    } /* if !found */
    else {   /* Schluesselwort gefunden */
#ifdef EXT_RFD
      FilePrintString(f,"; Schluesselwort: #ITERATION_PROPERTIES_SATURATION (ITERATION_PROPERTIES)"); LineFeed(f);
      FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann"); LineFeed(f);
      FilePrintString(f,"; mit Default-Werte gearbeitet."); LineFeed(f);
#endif
      while (StrTestHash(&data[p], &pos)) {
        sprintf(ip_name,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        /* IP's-Name in der Liste einfuegen */
        declare_iteration_properties_name(ip_name);
        i++;
        while (StrTestLong(&data[p+=pos])) {
          if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
            ok = 0;
            break;
          }
          else {
            ok = (StrReadLong(&l,&data[p],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
          }

          if ( get_ip_type(ip) == 0 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 1 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if ( get_ip_type(ip) == 2 ) {
            ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
            set_ip_norm(ip,k);
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_begin_eps(ip,d);    /* Anfangsfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_end_eps(ip,d);      /* Endfehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_fac_eps(ip,d);      /* Faktor fuer die var. Fehlerschranke */
            ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
            set_ip_exp_eps(ip,d);      /* Exponenet fuer die var. Fehlerschranke */
            ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
            set_ip_kind(ip,l);         /* Immer gueltig */
            LineFeed(f);
          }

          if (ip) {
            insert_iteration_properties_list(ip);
          }
          LineFeed(f);
        } /* while */
      } /* while hash */
#ifdef EXT_RFD
            FilePrintString(f, "; - Gleichungsloeser (loeser_flow) [1..6;6]");
            LineFeed(f);
            FilePrintString(f, ";   1: SpGAUSS; 2:SpBICGSTAB; 3:SpBICG;     4:SpQMRCGSTAB; 5: SpCG");
            LineFeed(f);
            FilePrintString(f, ";   6: SpCGNR;  7:CGS,        8: SpRichard; 9:SpJOR;      10:SpSOR");
            LineFeed(f);
            FilePrintString(f, "; - Max. Anzahl der Iterationen fuer Gleichungsloeser (maxiter_flow) [>=0;1000]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlerschranke fuer Gleichungsloeser (eps_flow) [>0.0;1.E-9]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Vorkonditionierer fuer Gleichungsloeser (vorkond_flow) [0,1;0]");
            LineFeed(f);
            FilePrintString(f, ";   0: keine Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   1: Diagonalen-Vorkonditionierung");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern)");
            LineFeed(f);
            FilePrintString(f, "; - Speichertechnik fuer Gesamtgleichungssystem (speichertechnik_flow) [1,2;2]");
            LineFeed(f);
            FilePrintString(f, ";   1: vollbesetzte Matrix");
            LineFeed(f);
            FilePrintString(f, ";   2: Sparse (nur A[i,j]!=0.0 werden gespeichert)");
            LineFeed(f);
            FilePrintString(f, "; - Fehlertyp (linear_error_type_flow) [0..4;2]");
            LineFeed(f);
            FilePrintString(f, ";   0: abs. ; 1:rel. zu B0; 2:rel. zu R0; 3:abs. und/oder rel.; 4:R/x");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 3 : Kombination zwischen 0 und 2");
            LineFeed(f);
            FilePrintString(f, ";                    ist |R0| < 1 dann wird Fehlertyp 0 betrachtet,");
            LineFeed(f);
            FilePrintString(f, ";                    ansonsten ist Fehlertyp 2 massgebend.");
            LineFeed(f);
            FilePrintString(f, ";   Zu Fehlertyp 4 : Vektornorm des aktuellen Residuums");
            LineFeed(f);
            FilePrintString(f, ";                    wird mit der aktuellen Loesung normiert");
            LineFeed(f);
            FilePrintString(f, "; - Daempfungs-/Relaxationsfaktor fuer iterative Gleichungsloeser (gls_iter_theta) [>0.0;1.0]");
            LineFeed(f);
            FilePrintString(f, ";   (Einfluss nur bei iterativen Loesern: Richardson,JOR,SOR)");
            LineFeed(f);
            FilePrintString(f, ";     JOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> gedaempfte Jacobi-Verfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Jacobi oder Gesamtschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> Jacobi Ueberrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     SOR-Verfahren mit:");
            LineFeed(f);
            FilePrintString(f, ";     Faktor 0 < theta < 1 --> sukzessive Unterrelaxation");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta = 1     --> Gauss-Seidel oder Einzelschrittverfahren");
            LineFeed(f);
            FilePrintString(f, ";     Faktor theta > 1     --> sukzessive Ueberrelaxation");
            LineFeed(f);
#endif
    }
  return ok;
}




/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* Test Site - OK 11.12.1999 - rf3261 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


int CreateIterationPropertiesPressure(void)
{
  int i,ok=1;
  char ip_name[255],string[255];
  ITERATION_PROPERTIES *ip = NULL;

      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
  return ok;
}

int CreateIterationPropertiesSaturation(void)
{
  int i,ok=1;
  char ip_name[255],string[255];
  ITERATION_PROPERTIES *ip;

      for (i=0; i<GetRFProcessNumPhases(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
  return ok;
}

int CreateIterationPropertiesComponent(void)
{
  int i,ok=1;
  char ip_name[255],string[255];
  ITERATION_PROPERTIES *ip = NULL;

      for (i=0; i<GetRFProcessNumComponents(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
  return ok;
}

int CreateIterationPropertiesSorptedComponent(void)
{
  int i,ok=1;
  char ip_name[255],string[255];
  ITERATION_PROPERTIES *ip = NULL;

      for (i=0; i<GetRFProcessNumComponents(); i++) {
        sprintf(string,"%s%d",DEFAULT_NAME_ITERATION_PROPERTIES_SORBED_COMPONENT(0),i+1);
        sprintf(ip_name,"DEFAULT_");
        strcat(ip_name,string);

        if ( (ip=create_iteration_properties(ip_name)) == NULL ) {
          ok = 0;
        }
        else {
          /* IP's-Name in der Liste einfuegen */
          declare_iteration_properties_name(ip_name);
          set_ip_type(ip,1);      /* Picard */
          set_ip_maxiter(ip,1000);/* Max. Anzahl an Iterationen */
          set_ip_criterium(ip,2); /* Kriterium */
          set_ip_abs_eps(ip,1.e-9);      /* abs. Fehlerschranke */
          set_ip_rel_eps(ip,1.e-3);      /* rel. Fehlerschranke */
          set_ip_rel_cg_eps(ip,1.e-3);   /* var. Fehlerschranke */
          set_ip_time(ip,-1.);           /* Gueltigkeitszeit */
          set_ip_kind(ip,0);      /* Immer gueltig */
          /* IP-Objekt in der Liste einfuegen */
          insert_iteration_properties_list(ip);
        } /* else */
      } /* for */
  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationProperties
                                                                          */
/* Aufgabe:
   Lese- und Protokollfunktion von Parametern zur Steuerung
   des Iterationsprozesses (IP Objekte) fuer nichtlineare Modelle
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
   08/1999   AH   Erste Version
   11/1999   OK   Protokoll existierender Objekte (found=2)
                  Verallgemeinerung fuer alle IP Objekte
   12/1999   AH   Bugfix bei Mehrfachlesen (fuer z.B. Mehrkomponentenmodell)
                                                                          */
/**************************************************************************/
int FctIterationPropertiesNew ( char *data,int found,FILE *f,char *name )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    long l;
    double d;
    char ip_name[255],keyword[255];
    ITERATION_PROPERTIES *ip = NULL;
    int i=0,k;


    /* Schleife ueber alle Phasen bzw. Komponenten */
    while (StrTestHash(&data[p], &pos) || (found==2) ) {

        /* 1 Tests ---------------------------------------------------------- */

        /* 2 IP Objekt Namen ------------------------------------------------ */
            /* Namen generieren */
        sprintf(ip_name,"%s%d",name,i+1);
        if(found==1) {
            /* Namen in die Liste einfuegen */
            declare_iteration_properties_name(ip_name);
        }
        else if (found==2) {
            /* Pruefen, ob Objekt existiert */
            if(!iteration_properties_name_exist(ip_name)) break;
        }

        /* 3 Schluesselwort schreiben --------------------------------------- */
        sprintf(keyword,"%s%s","#",name);
        FilePrintString(f,keyword);
        LineFeed(f);

        i++; /* Objekt-Zaehler */

        /* 4 Objekte erzeugen / bereitstellen ------------------------------- */
        if(found==1) {
            /* Neues Objekt erzeugen */
            ip=create_iteration_properties(ip_name);
        }
        else if (found==2) {
            /* Objekt aus der Liste holen */
            ip=GetIterationPropertiesGroup(ip_name,NULL);
        }
        /* Test - Abbruch, wenn IP Objekt nicht existiert */
        if (ip==NULL ) {
            ok = 0;
            break;
        }


        /* 5 Objekt-Daten einlesen / schreiben ------------------------------- */
        if(found==1) {
            ok = (StrReadLong(&l,&data[p+=pos],f,TFLong,&pos) && ok);
            set_ip_type(ip,l);
        }
        else if (found==2) {
            fprintf(f," %ld ",get_ip_type(ip));
        }
#ifdef EXT_RFD
#else
        FilePrintString(f, "; iteration method");
        LineFeed(f);
#endif
        if ( get_ip_type(ip) == 0 ) {
            if(found==1) {
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ip_norm(ip,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_abs_eps(ip,d);      /* abs. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_kind(ip,l);         /* Immer gueltig */
                LineFeed(f);
            }
            else if (found==2) {
                fprintf(f," %d ",get_ip_norm(ip));
                fprintf(f," %ld ",get_ip_maxiter(ip));
                fprintf(f," %e ",get_ip_abs_eps(ip));
                fprintf(f," %ld ",get_ip_kind(ip));
            }
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, abs error tolerance, time control");
        LineFeed(f);
#endif
        }
        if ( get_ip_type(ip) == 1 ) {
            if(found==1) {
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ip_norm(ip,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_rel_eps(ip,d);      /* rel. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_kind(ip,l);         /* Immer gueltig */
                LineFeed(f);
            }
            else if (found==2) {
                fprintf(f," %d ",get_ip_norm(ip));
                fprintf(f," %ld ",get_ip_maxiter(ip));
                fprintf(f," %e ",get_ip_rel_eps(ip));
                fprintf(f," %ld ",get_ip_kind(ip));
            }
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, rel error tolerance, time control");
        LineFeed(f);
#endif
        }
        if ( get_ip_type(ip) == 2 ) {
            if(found==1) {
                ok = (StrReadInt(&k,&data[p += pos],f,TFInt,&pos) && ok);
                set_ip_norm(ip,k);
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_maxiter(ip,l);      /* Max. Anzahl an Iterationen */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_begin_eps(ip,d);    /* Anfangsfehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_end_eps(ip,d);      /* Endfehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_fac_eps(ip,d);      /* Faktor fuer die var. Fehlerschranke */
                ok = (StrReadDouble(&d, &data[p += pos], f, TFDouble, &pos) && ok);
                set_ip_exp_eps(ip,d);      /* Exponenet fuer die var. Fehlerschranke */
                ok = (StrReadLong(&l, &data[p += pos], f, TFLong, &pos) && ok);
                set_ip_kind(ip,l);         /* Immer gueltig */
                LineFeed(f);
            }
            else if (found==2) {
                fprintf(f," %d ",get_ip_norm(ip));
                fprintf(f," %ld ",get_ip_maxiter(ip));
                fprintf(f," %e ",get_ip_begin_eps(ip));
                fprintf(f," %e ",get_ip_end_eps(ip));
                fprintf(f," %e ",get_ip_fac_eps(ip));
                fprintf(f," %e ",get_ip_exp_eps(ip));
                fprintf(f," %ld ",get_ip_kind(ip));
            }
#ifdef EXT_RFD
#else
        FilePrintString(f, "; norm, max iterations, eps start, eps final, eps fac, eps exp, time control");
        LineFeed(f);
#endif
        }

        /* 6 Objekt in Liste einfuegen  ------------------------------------- */
        if(found==1) {
            insert_iteration_properties_list(ip);
        }
        else if (found==2) {
        }
        p += pos;
    } /* while hash */

  return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesConcentrationNew
                                      */
/* Aufgabe:
   Iterations-Prozess fuer geloeste Komponenten-Konzentrationen (Fluidphase)
   Lese- und Protokollfunktion fuer Schluesselworte #ITERATION_PROPERTIES_CONCENTRATION
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
   11/1999   OK   Verallgemeinerung fuer alle IP Objekte
                                      */
/**************************************************************************/
int FctIterationPropertiesConcentrationNew ( char *data,int found, FILE *f )
{
    int ok = 1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_CONCENTRATION");
#endif

    if (!found) {   /* Schluesselwort nicht gefunden */
      if (GetRFProcessChemicalModel()>0) {
        FilePrintString(f,"; Keyword: #ITERATION_PROPERTIES_CONCENTRATION was not found");
        LineFeed(f);
        FilePrintString(f,"; Default values are used");
        LineFeed(f);
        CreateIterationPropertiesComponent();
      }
    }
    else {          /* Schluesselwort gefunden */
        /* Schluesselwort schreiben */
        ok = FctIterationPropertiesNew(data,found,f, \
                                    name_iteration_properties_tracer_component);
                                    /* DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0)); */
    }

  return ok;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesSorbedConcentrationNew
                                      */
/* Aufgabe:
   Iterations-Prozess fuer sorbierte Komponenten-Konzentrationen (Feststoffphase)
   Lese- und Protokollfunktion fuer Schluesselworte
   #ITERATION_PROPERTIES_SORBED_CONCENTRATION
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
   11/1999   OK   Verallgemeinerung fuer alle IP Objekte
                                      */
/**************************************************************************/
int FctIterationPropertiesSorbedConcentrationNew ( char *data,int found, FILE *f )
{
    int ok = 1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_SORBED_CONCENTRATION");
#endif

    if (!found) {   /* Schluesselwort nicht gefunden */
      if (GetRFProcessChemicalModel()>1) {
        FilePrintString(f,"; Keyword: #ITERATION_PROPERTIES_SORBED_CONCENTRATION was not found");
        LineFeed(f);
        FilePrintString(f,"; Default values are used");
        LineFeed(f);
        CreateIterationPropertiesSorptedComponent();
      }
    }
    else {          /* Schluesselwort gefunden */
        /* Schluesselwort schreiben */
        ok = FctIterationPropertiesNew(data,found,f, \
                                    name_iteration_properties_sorbed_component);
                                    /* DEFAULT_NAME_ITERATION_PROPERTIES_TRACER_COMPONENT(0)); */
    }

  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesImmobileSolutedConcentrationNew
                                                                          */
/* Aufgabe:
   Iterations-Prozess fuer sorbierte Komponenten-Konzentrationen (Feststoffphase)
   Lese- und Protokollfunktion fuer Schluesselworte
   #ITERATION_PROPERTIES_SORBED_CONCENTRATION
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
   05/2000    AH      Fuer Mobil-Immobil-Konzept
                                                                          */
/**************************************************************************/
int FctIterationPropertiesImmobileSolutedConcentrationNew ( char *data,int found, FILE *f )
{
    int ok = 1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_IMMOBILE_SOLUTED_CONCENTRATION");
#endif

    if (!found) {   /* Schluesselwort nicht gefunden */
      if (GetRFProcessChemicalModel()>1) {
        FilePrintString(f,"; Keyword: #ITERATION_PROPERTIES_IMMOBILE_SOLUTED_CONCENTRATION was not found");
        LineFeed(f);
        FilePrintString(f,"; Default values are used");
        LineFeed(f);
        CreateIterationPropertiesSorptedComponent();
      }
    }
    else {          /* Schluesselwort gefunden */
        /* Schluesselwort schreiben */
        ok = FctIterationPropertiesNew(data,found,f, \
                                    name_iteration_properties_immobile_solute_component);
                                    /* DEFAULT_NAME_ITERATION_PROPERTIES_IMMOBILE_SOLUTE_COMPONENT(0)); */
    }

  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesPressureNew
                                      */
/* Aufgabe:
   Iterations-Prozess fuer nichtlineare Stroemungsmodelle
   Lese- und Protokollfunktion fuer Schluesselworte
   #ITERATION_PROPERTIES_PRESSURE
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
   11/1999   OK   Verallgemeinerung fuer alle IP Objekte
                                      */
/**************************************************************************/
int FctIterationPropertiesPressureNew ( char *data,int found, FILE *f )
{
    int ok = 1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_PRESSURE");
#endif

    FilePrintString(f, "; 4.3 Iteration Process Properties -------------------------------------");
    LineFeed(f);

    if (!found) {   /* Schluesselwort nicht gefunden */
      if (GetRFProcessFlowModel()>0 || GetRFProcessNumPhases()>1) {
        FilePrintString(f,"; Keyword: #ITERATION_PROPERTIES_PRESSURE was not found");
        LineFeed(f);
        FilePrintString(f,"; Default values are used");
        LineFeed(f);
        CreateIterationPropertiesPressure();
      }
    }
    else {          /* Schluesselwort gefunden */
        /* Schluesselwort schreiben */
        ok = FctIterationPropertiesNew(data,found,f, \
                                    name_iteration_properties_fluid_phase);
                                    /* DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0)); */
    }

  return ok;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: FctIterationPropertiesSaturationNew
                                      */
/* Aufgabe:
   Iterations-Prozess fuer Mehrphasen-Stroemungsmodelle
   Lese- und Protokollfunktion fuer Schluesselworte
   #ITERATION_PROPERTIES_SATURATION
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
   11/1999   OK   Verallgemeinerung fuer alle IP Objekte
                                      */
/**************************************************************************/
int FctIterationPropertiesSaturationNew ( char *data,int found, FILE *f )
{
    int ok = 1;

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort #ITERATION_PROPERTIES_SATURATION");
#endif

    if (!found) {   /* Schluesselwort nicht gefunden */
      if (GetRFProcessNumPhases()>1) {
        FilePrintString(f,"; Keyword: #ITERATION_PROPERTIES_SATURATION was not found");
        LineFeed(f);
        FilePrintString(f,"; Default values are used");
        LineFeed(f);
        CreateIterationPropertiesSaturation();
      }
    }
    else {          /* Schluesselwort gefunden */
        /* Schluesselwort schreiben */
        ok = FctIterationPropertiesNew(data,found,f, \
                                    name_iteration_properties_saturation);
                                    /* DEFAULT_NAME_ITERATION_PROPERTIES_FLUID_PHASE(0)); */
    }

  return ok;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcIterationError
                                                                          */
/* Aufgabe:
   Ermittelt den Fehler bei Iterationen

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *new_iteration : Vektor des neuen Iterationsschritts
   E double *old_iteration : Vektor des alten Iterationsschritts
   E double *reference     : Vektor des alten Zeitschritts (als Referenz)
   E longlength            : Laenge der Vektoren
   E int method            : Methode der Fehlerermittlung
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   1/1999     C.Thorenz  Zweite Version                                                                          */
/**************************************************************************/
double CalcIterationError(double *new_iteration, double *old_iteration, double *reference, long length, int method)
{
    static long i;
    static double error, change, max_c, min_c;

    error = 0.;
    change = 0.;

    max_c = 0.;
    min_c = 1.e99;

    switch (method) {
    default:
    case 0:
        return 0.;
    /* Max. Unterschied zwischen altem und neuem Iterationsschritt */
    case 1:
        for (i = 0l; i < length; i++)
            error = max(error, fabs(new_iteration[i] - old_iteration[i]));
        return error;

    /* Max. Unterschied zwischen altem und neuem Iterationsschritt,
       jeweils normiert mit dem Mittelwert der Groesse des Wertes  */
    case 2:
        for (i = 0l; i < length; i++)
            error = max(error, 2. * fabs(new_iteration[i] - old_iteration[i]) / (fabs(new_iteration[i]) + fabs(old_iteration[i]) + MKleinsteZahl));
        return error;

    /* Max. Unterschied zwischen altem und neuem Iterationsschritt,
       normiert mit dem groessten Wert */
    case 3:
        for (i = 0l; i < length; i++) {
            error = max(error, fabs(new_iteration[i] - old_iteration[i]));
            max_c = max(max(max_c, fabs(new_iteration[i])),fabs(old_iteration[i]));
        }
        return error / (max_c + MKleinsteZahl);

    /* Max. Unterschied zwischen altem und neuem Iterationsschritt,
       normiert mit der Spanne der Werte */
    case 4:
        for (i = 0l; i < length; i++) {
            error = max(error, fabs(new_iteration[i] - old_iteration[i]));
            min_c = min(min_c, fabs(new_iteration[i]));
            max_c = max(max_c, fabs(new_iteration[i]));
        }
        return error / (max_c - min_c + MKleinsteZahl);

    /* Max. Unterschied zwischen altem und neuem Iterationsschritt,
       normiert mit dem Unterschied zum alten Zeitschritt. Die
       genaueste Methode, da die Fehlerberechnung dann Zeitschritt-
       unabhaengig wird! */
    case 5:
        for (i = 0l; i < length; i++)
            error = max(error, fabs(new_iteration[i] - old_iteration[i]) / (fabs(new_iteration[i] - reference[i]) + MKleinsteZahl));
        return error;

    /* Max. Unterschied zwischen altem und neuem Iterationsschritt,
       normiert mit dem maximalen Unterschied zum alten Zeitschritt */
    case 6:
        for (i = 0l; i < length; i++) {
            error = max(error, fabs(new_iteration[i] - old_iteration[i]));
            change = max(change, fabs(new_iteration[i] - reference[i]));
        }
        return error / (change + MKleinsteZahl);

    /* Der Vektorabstand */
    case 7:
        return MVekDist(old_iteration, new_iteration, length);

    /* Der Vektorabstand der Iteration, normiert mit dem Vektorabstand
       zur alten Zeitebene */
    case 8:
        return MVekDist(old_iteration, new_iteration, length) / (MVekDist(reference, new_iteration, length) + MKleinsteZahl);
    }
}

/**************************************************************************/
/* ROCKFLOW - Funktion: CalcIterationErrorVectorAgainstNodes
                                                                          */
/* Aufgabe:
   Ermittelt den Fehler bei Iterationen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E double *new_iteration : Vektor des neuen Iterationsschritts
   E int old_iteration_ndx : Knotenindex fuer Werte des alten Iterationsschritts
   E int reference_ndx     : Knotenindex fuer Werte des alten Zeitschritts (als Referenz)
   E int method            : Methode der Fehlerermittlung
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   1/1999     C.Thorenz  Zweite Version                                                                          */
/**************************************************************************/
double CalcIterationErrorVectorAgainstNodes(double *new_iteration, int old_iteration_ndx, int reference_ndx, int method) {

   double *old_iteration, *reference, error;

   old_iteration = (double *) Malloc(sizeof(double) * NodeListLength);
   reference = (double *) Malloc(sizeof(double) * NodeListLength);

   PresetErgebnis (old_iteration, old_iteration_ndx );
   PresetErgebnis (reference, reference_ndx );

   error = CalcIterationError(new_iteration, old_iteration, reference, NodeListLength, method);

   Free(old_iteration);
   Free(reference);

   return error;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CalcIterationErrorVectorAgainstNodesV2
                                                                          */
/* Aufgabe:
   Ermittelt den Fehler bei Iterationen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E int new_iteration_ndx : Knotenindex fuer Werte des neuen Iterationsschritts
   E int old_iteration_ndx : Knotenindex fuer Werte des alten Iterationsschritts
   E int reference_ndx     : Knotenindex fuer Werte des alten Zeitschritts (als Referenz)
   E int method            : Methode der Fehlerermittlung
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   1/1999     C.Thorenz  Zweite Version    
   07/2002   OK   für Lösungsvektoren                                                                        */
/**************************************************************************/
double CalcIterationErrorVectorAgainstNodesV2(int new_iteration_ndx, int old_iteration_ndx, \
                                                                                          int reference_ndx, int method) {

   double *old_iteration, *reference, error, *new_iteration;;

   new_iteration = (double *) Malloc(sizeof(double) * NodeListLength);
   old_iteration = (double *) Malloc(sizeof(double) * NodeListLength);
   reference = (double *) Malloc(sizeof(double) * NodeListLength);

   PresetErgebnis (old_iteration, old_iteration_ndx );
   PresetErgebnis (reference, reference_ndx );
   PresetErgebnis (new_iteration, new_iteration_ndx );

   error = CalcIterationError(new_iteration, old_iteration, reference, NodeListLength, method);

   Free(old_iteration);
   Free(reference);
   Free(new_iteration);

   return error;
}


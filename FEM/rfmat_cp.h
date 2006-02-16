/**************************************************************************/
/* ROCKFLOW - Modul: rfmat_cp.h
                                                                          */
/* Task:
   Methods for ComponentProperties
                                                                          */
/* Programming:
   10/2004   SB  First Implemented
                                                                          */
/**************************************************************************/

/* Schutz gegen mehrfaches Einfuegen */
#ifndef rfmat_cp_INC
#define rfmat_cp_INC


#define CP_FILE_EXTENSION ".mcp"  /* File extension for component properties input file */
#include<fstream>

/*************************************************************************

Class ComponentProperties

**************************************************************************/

class CompProperties
{
	private:
	public:

	/* constructor */
	CompProperties(long);
	/* destructor */
	~CompProperties(void);

	string name;								/* Gruppenname */
	string compname;							/* component name */
	long mobil;									/* flag mobil */
	long transport_phase;						/* number of phase, in which component is transported */
	int valence;								// valence of ionic elements /*MX*/

    /* Diffusionsmodelle und zugehoerige Beschreibungswerte */
    int diffusion_model;						/* Zerfallsmodell in geloester Phase */
    int count_of_diffusion_model_values;        /* Anzahl der Parameter zur Spezifikation des Diffusionsmodells */
    double diffusion_model_values[10];				/* Parameter fuer das Diffusionsmodell */
	int diffusion_function_name;
	int GetNumberDiffusionValuesCompProperties(int ); /* Zugriff auf Number of Parameters */

    /* Zerfallsmodelle und zugehoerige Beschreibungswerte in der geloesten Phase */
    int decay_model;							/* Zerfallsmodell in geloester Phase */
    int count_of_decay_model_values;			/* Anzahl und Werte zur Spezifikation der */
    double decay_model_values[10];					/* Parameter fuer Zerfallsprozess wie z.B. Zerfallsrate */
	int decay_function_name;
	int GetNumberDecayValuesCompProperties(int);	/* Zugriff auf Number of Parameters */

    /* Sorption */
    int isotherm_model;							/* Isothermen-Typ */
    int count_of_isotherm_model_values;         /* Anzahl der Isothermen-Koeffizienten */
    double isotherm_model_values[10];				/* Isothermen-Koeffizienten */
	int isotherm_function_name;
	int GetNumberIsothermValuesCompProperties(int); /* Zugriff auf Number of Parameters */

    ios::pos_type Read(ifstream*);						/* Lesefunktion für eine Instanz von CompProperties */
    void Write(ofstream*);						/* Schreibfunktion für eine Instanz von CompProperties */

	/* Member - Functions */
	double CalcDiffusionCoefficientCP(long index);
	double CalcDiffusionCoefficientCP_Method1(long index, double T, double P, double eta);
	double CalcElementRetardationFactor( long index, double*gp, double theta );
	double CalcElementRetardationFactorNew( long index, double*gp, CRFProcess* m_pcs );
	double CalcElementMeanConc(long index);
	double CalcElementMeanConcNew(long index, CRFProcess* m_pcs);
	double CalcElementDecayRate( long index);
    double CalcElementDecayRateNew( long index, CRFProcess* m_pcs);
	// IO
    string file_base_name;
    
};

/* Vector auf CompProperties , globale Zugriffe */
extern vector <CompProperties*> cp_vec;

/* ----------------------------------------------------------------------- */
/* Read all Component Properties instance by instance from input file *.cp */
extern bool CPRead(string);
/* Write all Component Properties instance by instance to output file */
extern void CPWrite(string,int);
extern int CPGetMobil(long comp);
extern void MCPDelete();

#endif

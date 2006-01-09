/* reaction package header file */

#ifndef reaction_INC

#define reactions_INC

#include <vector>
using namespace std;

/* Structure for exchange of reaction rates */
class REACT{
	private:
	public:
		REACT(void);
		~REACT(void);
// Data
	char **name;       /* names of reactants */
	double **val_in ;  /* input concentration values to reaction module */
	double **val_out ; /* output concentration values of reaction module */
	double **rate ;    /* reaction rates for MTM2 */
	int *rateflag;     /* flag used for determining if reaction are calculated with PHREEQC */
	int countsteps;    /* number of timesteps, after which reactions are recalculated in the whole model domain */
	int number_of_comp;/* Number of components in above data structures: size = number_of_comp * NodeListLength */
	int heatflag;      /* if 1, heat transport is active and passed to phreeqc */
	long nodenumber;   /* number of nodes, on which reactions are calculated */
	/* hier später arrays of reactions reinhängen ?*/

	// rcml moved here
	int rcml_number_of_master_species; /* number of master species (inorgan. equilibrium) */
	int rcml_number_of_equi_phases; /* number of phases (in equilibrium) */
	int rcml_number_of_kinetics; /* number of kinetic reactions  */
	int rcml_number_of_ion_exchanges; /* number of phases (in equilibrium) */
	/* hier später reaction models reinhängen ?*/
    int rcml_pH_flag;   /* =0, pH constant; =1 (default), pH will change  */
    int rcml_pe_flag;   /* =0, pe constant; =1 (default), pe will change  */
	int rcml_heat_flag; /* =0, temp constant (default); =1 , temp will change  */
	int rcml_number_of_pqcsteps; /* Anzahl der Reaktionsschritte in PHREEQC aus dem Befehl: -steps "time" in "pqcsteps" steps */
	char * outfile; /* Ausgabefile von PHREEQC */

// Member functions
	REACT* GetREACT(void);
	void CreateREACT(void);
	void InitREACT(void);
	void ExecuteReactions(void);
	void ExecuteReactionsPHREEQC(void);
	int  TestPHREEQC(void);
	int  Call_Phreeqc(void);
	void GetTransportResults(void);
	int  ReadReactionModel(FILE *File);
	int  ReadInputPhreeqc( long index, FILE *fpqc, FILE *Fphinp); //fsout removed 3912
	int  ReadOutputPhreeqc(char* fout);
	void ResetpHpe(void);
	void CalculateReactionRates(void);
	void SetConcentrationResults(void);
	void CalculateReactionRateFlag(void);
	void SetNeighborNodesActive(long startnode, long level, int* help);

};
extern vector <REACT*> REACT_vec;

extern void DestroyREACT(void);
extern double MATCalcIonicStrengthNew(long index);

#endif

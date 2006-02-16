#ifndef loop_pcs_INC

#define loop_pcs_INC
  /* Schutz gegen mehrfaches Einfuegen */

extern void LOPConfig_PCS(void);
extern void PCSCalcSecondaryVariables(void);

//void LOPCalcNodeFluxes_PCS(void);   //CMCD 09/2004
double Checkcourant_PCS(void);    //CMCD 09/2004
/*
extern int pcs_fluid_transport_process;
extern int pcs_isothermal_flow;
extern int pcs_non_isothermal_flow;
extern int pcs_number_of_fluid_components;
extern int pcs_heat_transport_process;
extern int pcs_comp_transport_process;
extern int pcs_deformation_process;
extern int pcs_dimension;
*/
extern int pcs_number_of_fluid_phases;
extern int LOPPreTimeLoop_PCS();
extern int LOPTimeLoop_PCS(double*);
extern double dt_sum;
#endif

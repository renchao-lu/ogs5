#include <math.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;


double pressure (double, double, int);
// calculates the pressure depending on density and temperature
double density (double, double,  double,  double, int);
// calculates the density iteratively depending on pressure and temperature
double enthalpy (double, double, int);
// calculates the density iteratively depending on pressure and temperature
double isochoric_heat_capacity (double,  double,  int);
// calculates the isochoric heat capacity depending on density and temperature
double isobaric_heat_capacity (double,  double,  int);
// calculates the isobaric heat capacity depending on density and temperature
double co2_viscosity (double, double);
// calculates the viscosity depending on density and temperature !ONLY for CO2!!!
double co2_heat_conductivity (double, double);
// calculates the heat conductivity of co2 depending on density and temperature
double ch4_viscosity_295K (double);
// calculates the viscosity of CH4 at 25 °C depending on pressure
double Fluid_Viscosity (double,double ,double ,string);
//Viscosity for several fluids
double Fluid_Heat_Conductivity (double, double, string);
//Heat conductivity for several fluids

#include <math.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;


double pressure (double rho, double T, int fluid);
// calculates the pressure depending on density and temperature
double density (double P, double rho0,  double T,  double prec, int fluid);
// calculates the density iteratively depending on pressure and temperature
double enthalpy (double P, double T, int fluid);
// calculates the density iteratively depending on pressure and temperature
double isochoric_heat_capacity (double rho,  double T,  int fluid);
// calculates the isochoric heat capacity depending on density and temperature
double isobaric_heat_capacity (double rho,  double T,  int fluid);
// calculates the isobaric heat capacity depending on density and temperature
double co2_viscosity (double rho, double T);
// calculates the viscosity depending on density and temperature !ONLY for CO2!!!



/*!
 *  \File Constants.cpp
 *  Define constants used in the program.
 *  Created by wenqing on 7/22/15.
*/

#include "PhysicalConstants.h"

namespace constant_group
{

// Thermal constant
double ThermalConstant :: _celsius_zero_in_kelvin = 273.15;

// Fluid
double FluidConstant::_comp_mol_mass_air = 28.96;
double FluidConstant::_comp_mol_mass_water = 18.016;
double FluidConstant::_comp_mol_mass_n2 = 28.014;
double FluidConstant::_gas_constant = 8314.41;
double FluidConstant::_specific_gas_constant = 461.5;

}


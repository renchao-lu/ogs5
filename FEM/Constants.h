/*!
 *  \File Constants.h
 *  Declear constants used in the program.
 *  Created by wenqing on 7/22/15.
*/

#ifndef OGS_CONSTANTS_H
#define OGS_CONSTANTS_H

namespace constant_group
{
class ThermalConstant
{
    public:
        static double CelsiusZeroInKelvin()
        {
            return _celsius_zero_in_kelvin;
        }

    private:
        static double _celsius_zero_in_kelvin;
};

class FluidConstant
{
    public:
        static double ComponentMolarMassAir()
        {
            return _comp_mol_mass_air;
        }

        static double ComponentMolarMassWater()
        {
            return _comp_mol_mass_water;
        }

        static double ComponentMolarMassWaterN2()
        {
            return _comp_mol_mass_n2;
        }

        static double GasConstant()
        {
            return _gas_constant;
        }

        static double SpecificGasConstant()
        {
            return _specific_gas_constant;
        }

    private:
        static double _comp_mol_mass_air;
        static double _comp_mol_mass_water;
        static double _comp_mol_mass_n2;
        static double _gas_constant;
        static double _specific_gas_constant;
};

}
#endif //OGS_CONSTANTS_H

/*
FluidConstant::ComponentMolarMassAir()
FluidConstant::ComponentMolarMassWater()
FluidConstant::ComponentMolarMassWaterN2()
FluidConstant::GasConstant()
*/
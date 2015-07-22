/*!
 *  \File PhysicalConstants.h
 *  Declear constants used in the program.
 *  Created by wenqing on 7/22/15.
*/

#ifndef OGS_PHYSICAL_CONSTANTS_H
#define OGS_PHYSICAL_CONSTANTS_H

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

/**
 * Namespace containing physical constants
 *  by Christoph Lehmann
 * All members of this namespace should be given in SI standard units,
 * i.e. in terms of kg, m, s, K, mol, A, cd.
 */
namespace Phys
{

/**
  Ideal gas constant in SI standard units (J mol^-1 K^-1)

  Source:               http://physics.nist.gov/cgi-bin/cuu/Value?r
  Date visited:         2015-04-17
  Standard uncertainty: 0.000 0075 J mol^-1 K^-1
*/
    const double R = 8.3144621;


/**
 * Molar masses of certain elements and chemical compounds
 */
    namespace MolMass
    {
        /**
         * Water
         *
         * Source:       http://www.ciaaw.org/pubs/TSAW2013_xls.xls
         * Date visited: 2015-05-28
         *
         * According to the IUPAC report the molar mass of O is in the range [15.999 03, 15.999 77] g/mol
         * and the molar mass of H is in the range [1.007 84, 1.008 11] g/mol
         */
        const double Water = 0.018016; ///< kg mol^-1

        /**
         * N_2
         *
         * Source:       http://www.ciaaw.org/pubs/TSAW2013_xls.xls
         * Date visited: 2015-05-28
         *
         * According to the IUPAC report the molar mass of N is in the range [14.006 43, 14.007 28] g/mol
         */
        const double N2    = 0.028013; ///< kg mol^-1

        /**
         * O_2
         *
         * Source:       http://www.ciaaw.org/pubs/TSAW2013_xls.xls
         * Date visited: 2015-05-28
         *
         * According to the IUPAC report the molar mass of O is in the range [15.999 03, 15.999 77] g/mol
         */
        const double O2    = 0.032;    ///< kg mol^-1
    }

}
#endif //OGS_CONSTANTS_H

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

}
#endif //OGS_CONSTANTS_H

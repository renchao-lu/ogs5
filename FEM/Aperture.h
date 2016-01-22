
#ifndef APERTURE_H_
#define APERTURE_H_

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>

class ApertureRcYasuhara2006
{
public:
    ApertureRcYasuhara2006(double a1, double a2, double a3, double Rc0)
    : _a1(a1), _a2(a2), _a3(a3), _Rc0(Rc0)
    {
        assert(a2>.0 && a3>.0);
    }

    double Aperture(double Rc) const
    {
        const double b = _a1 + _a2*std::exp(-(Rc*100-_Rc0)/_a3);
        return b*1.e-6;
    }

    double ResidualAperture() const
    {
    	return _a1*1e-6;
    }

    double Rc(double b) const
    {
        const double bn = b * 1.e+6;
        double new_rc = 0.99;
        if (bn-_a1>.0)
            new_rc = (_Rc0 - _a3 * std::log((bn-_a1)/_a2))/100.0;
        new_rc = std::min(new_rc, 0.99);
        return new_rc;
    }

private:
    double _a1;
    double _a2;
    double _a3;
    double _Rc0;
};

class ApertureRcNew
{
public:
	ApertureRcNew(double a1, double a2)
		: _a1(a1), _a2(a2)
	{
		assert(a2>.0);
	}

	double Rc(double old_rc, double db_ps) const
	{	
		double new_rc = 0.99;
		new_rc = pow( std::sqrt(old_rc) + 0.5 * _a1 * db_ps / std::sqrt(_a2 / M_PI), 2);
		new_rc = std::min(new_rc, 0.99);
		return new_rc;
	}

private:
	double _a1; // tilt angle: the ratio of radius of cross section to protrubing height;
	double _a2; // total area of a single fracture surface;
};
#endif //APERTURE_H_

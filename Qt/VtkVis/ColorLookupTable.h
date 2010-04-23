/**
 * \file ColorLookupTable.h
 * 23/04/2010 KR Initial implementation
 *
 */


#ifndef COLORLOOKUPTABLE_H
#define COLORLOOKUPTABLE_H

// ** INCLUDES **
#include <vector>
#include "Color.h"

/**
 * \brief Adapter class to convert FEM Mesh to a representation more suited for visualisation purposes
 */
class ColorLookupTable
{
public:
	enum LUTType {
		LINEAR = 0,
		EXPONENTIAL = 1,
		SIGMOID = 2
	};

	ColorLookupTable(double rangeBegin, double rangeEnd, size_t numberOfEntries = 256);
	~ColorLookupTable();

	void build();
	GEOLIB::Color* getColor(size_t pos);
	void getColor(size_t pos, unsigned char* color);
	void setColor(GEOLIB::Color* val, double pos);
	void setStartColor(GEOLIB::Color* begin);
	void setEndColor(GEOLIB::Color* end);
	void setRange(double start, double end);


private:
	
	unsigned char linInterpolation(unsigned char a, unsigned char b, double p);
	unsigned char expInterpolation(unsigned char a, unsigned char b, double gamma, double p);

	std::vector<GEOLIB::Color*> _lookuptable;
	std::map<size_t, GEOLIB::Color*> _dict;
	std::pair<double, double> _range;
	LUTType _type;
};

#endif // COLORLOOKUPTABLE_H

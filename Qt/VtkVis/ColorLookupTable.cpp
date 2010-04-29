/**
 * \file ColorLookupTable.cpp
 * 23/04/2010 KR Initial implementation
 *
 */


#include "ColorLookupTable.h"

#include <cmath>

ColorLookupTable::ColorLookupTable(double rangeBegin, double rangeEnd, size_t numberOfEntries) : _range(rangeBegin, rangeEnd)
{
	_lookuptable.reserve(numberOfEntries);
	for (size_t i=0; i<numberOfEntries; i++)
		_lookuptable.push_back(NULL);

	// default values ( can be changed via setStartColor() / startEndColor ()
	GEOLIB::Color* start = new GEOLIB::Color(0,0,255);
	GEOLIB::Color* end   = new GEOLIB::Color(255,0,0);
	_dict.insert( std::pair<size_t, GEOLIB::Color*>(0, start) );
	_dict.insert( std::pair<size_t, GEOLIB::Color*>(numberOfEntries-1, end) );

	_type = ColorLookupTable::LINEAR;
}

ColorLookupTable::~ColorLookupTable()
{
}


unsigned char ColorLookupTable::linInterpolation(unsigned char a, unsigned char b, double p)
{
    return static_cast<unsigned char>(a * (1 - p) + b * p);
}

// doesn't work quite correct yet
unsigned char ColorLookupTable::expInterpolation(unsigned char a, unsigned char b, double gamma, double p)
{
	assert (gamma>0 && gamma<2);
	double temp = (b-a) * pow(p/(b-a), gamma);
	return static_cast<unsigned char>(a* (1 - temp) + b * temp);
}


void ColorLookupTable::build()
{
	std::pair<size_t, GEOLIB::Color*> lastValue(0, new GEOLIB::Color(0,0,0));

	for (std::map<size_t, GEOLIB::Color*>::const_iterator it = _dict.begin(); it != _dict.end(); ++it)
	{
		_lookuptable[it->first] = it->second;

		if ( it->first-lastValue.first > 1 ) 
		{
			for (size_t i = lastValue.first+1; i < it->first; i++)
			{
				GEOLIB::Color* c = new GEOLIB::Color();
				double pos = (i - lastValue.first) / (static_cast<double>(it->first - lastValue.first));
				for (size_t j=0; j<3; j++)
					(*c)[j] = linInterpolation((*(lastValue.second))[j], (*(it->second))[j], pos);
				_lookuptable[i] = c;

			}
		}
		
		lastValue.first = it->first;
		lastValue.second = it->second;
	}
}

GEOLIB::Color* ColorLookupTable::getColor(size_t pos)
{
	if (pos>=_range.first && pos<=_range.second)
	{
		GEOLIB::Color* c = NULL;

		size_t idx = static_cast<size_t>( floor( (pos-_range.first) * (_lookuptable.size()/(_range.second-_range.first)) ) );
		return _lookuptable[idx];
	}
	else 
		std::cout << "ColorLookupTable::getColor() - Error: value out of range." << std::endl;
	return _lookuptable[0];
}

void ColorLookupTable::getColor(size_t pos, unsigned char* color)
{
	GEOLIB::Color* c = getColor(pos);
	for (size_t j=0; j<3; j++)
		color[j] = (*c)[j];
}


void ColorLookupTable::setStartColor(GEOLIB::Color* begin)
{
	_dict.insert( std::pair<size_t, GEOLIB::Color*>(0, begin) );
}

void ColorLookupTable::setEndColor(GEOLIB::Color* end)
{
	_dict.insert( std::pair<size_t, GEOLIB::Color*>(_lookuptable.capacity()-1, end) );
}

void ColorLookupTable::setColor(GEOLIB::Color* val, double pos)
{
	if (pos>0 && pos<1)
	{
		size_t idx = static_cast<size_t>( floor(_lookuptable.capacity()*pos) );
		_dict.insert( std::pair<size_t, GEOLIB::Color*>(idx, val) );
	}
	else 
		std::cout << "ColorLookupTable::setValue() - Error: pos should be in (0,1)" << std::endl;
}

void ColorLookupTable::setRange(double start, double end)
{
	if (start < end)
	{
		_range.first  = start;
		_range.second = end;
	}
	else 
		std::cout << "ColorLookupTable::setRange() - Error: start > end" << std::endl;
}
/**
 * \file Color.cpp
 * 04/02/2010 KR Initial implementation
 *
 */


#ifndef COLOR_H_
#define COLOR_H_

#include "TemplatePoint.h"

#include <fstream>
#include <cstdlib>
#include <map>
#include <list>

#include "StringTools.h"


namespace GEOLIB {


typedef TemplatePoint<unsigned char> Color;

/// Returns a random RGB colour.
static Color* getRandomColor()
{ 
	return new Color((rand()%5)*50, (rand()%5)*50, (rand()%5)*50); 
}

static int readColorLookupTable(std::map<std::string, GEOLIB::Color> &colors, const std::string &filename)
{
	std::string id = "", line = "";

	std::ifstream in( filename.c_str() );

	if (!in.is_open())
	{
		std::cout << "Color::readLookupTable() - Could not open file..."  << std::endl;
		return 0;
	}

	while ( getline(in, line) )
	{
		std::list<std::string> fields = splitString(line, '\t');
		GEOLIB::Color c;

		if (fields.size()>=4)
		{
			id = fields.front();
			fields.pop_front();
			c[0] = atoi(fields.front().c_str());
			fields.pop_front();
			c[1] = atoi(fields.front().c_str());
			fields.pop_front();
			c[2] = atoi(fields.front().c_str());
			colors.insert(std::pair<std::string, GEOLIB::Color>(id, c));
		}
	}

	return 1;
}


static Color getColor(const std::string &id, const std::map<std::string, GEOLIB::Color> &colors)
{
	size_t nColors = colors.size();
	for (std::map<std::string, GEOLIB::Color>::const_iterator it=colors.begin(); it !=colors.end(); ++it)
	{
		if (id.compare(it->first) == 0)
			return it->second;
	}
	std::cout << "Key not found in color lookup table..." << std::endl;
	GEOLIB::Color* c = getRandomColor();
	return *c;
}


} // namespace

#endif /* COLOR_H_ */

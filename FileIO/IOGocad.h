/*
 * IOGocad.h
 *
 *  Created on: Feb 10, 2010
 *      Author: fischeth
 */

#ifndef IOGOCAD_H_
#define IOGOCAD_H_

#include <string>
#include <vector>
#include <iostream>

namespace GEOLIB {
class GEOObjects;
}

namespace FileIO {

class IOGocad {
public:
	IOGocad(const std::string &fname, GEOLIB::GEOObjects *geo_obj);
	~IOGocad();

private:
	void readObjects ( std::istream &in );
	void readHeader ( std::istream &in );
	void readCoordinateSystem ( std::istream &in );
	bool isProperty ( const std::string &line, std::istream &in );
	bool isGeologicInformation ( const std::string &line );
	void readTSurfData ( std::istream &in );
	void readPLineData ( std::istream &in );

	std::string _fname;
	GEOLIB::GEOObjects *_geo_obj;
	static const size_t MAX_COLS_PER_ROW = 256;
};

} // end namespace

#endif /* IOGOCAD_H_ */

/*
 * PetrelIO.h
 *
 *  Created on: Feb 16, 2010
 *      Author: fischeth
 */

#ifndef PETRELIO_H_
#define PETRELIO_H_

#include <list>
#include <string>
#include <iostream>
#include <vector>
#include "GEOObjects.h"

namespace FileIO {

class PetrelIO {
public:
	PetrelIO(std::list<std::string> &fnames, std::string &unique_model_name, GEOLIB::GEOObjects* obj);
	virtual ~PetrelIO();

private:
	void readPetrelSurface (std::istream &in);
	std::string _unique_name;
	std::vector<GEOLIB::Point*>* pnt_vec;
//	GEOLIB::GEOObjects *_geo_obj;
	static const size_t MAX_COLS_PER_ROW = 256;
};

} // end namespace FileIO

#endif /* PETRELIO_H_ */

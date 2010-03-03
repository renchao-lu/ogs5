/*
 * PetrelIO.cpp
 *
 *  Created on: Feb 16, 2010
 *      Author: fischeth
 */

#include "PetrelIO.h"
#include <fstream>

namespace FileIO {

PetrelIO::PetrelIO(std::list<std::string> &fnames,
		std::string &unique_model_name, GEOLIB::GEOObjects* geo_obj) :
	_unique_name(unique_model_name), pnt_vec(new std::vector<GEOLIB::Point*>)
{

	for (std::list<std::string>::const_iterator it(fnames.begin()); it
			!= fnames.end(); it++) {
		std::cout << "PetrelIO::PetrelIO open stream from file " << *it << " ... " << std::flush;
		std::ifstream in((*it).c_str());
		if (in) {
			std::cout << "done" << std::endl;
			readPetrelSurface (in);
			in.close();
		} else {
			std::cerr << "error opening stream " << std::endl;
		}
	}

	// store data in GEOObject
	geo_obj->addPointVec(pnt_vec, _unique_name);
}

PetrelIO::~PetrelIO()
{
	delete pnt_vec;
}

void PetrelIO::readPetrelSurface (std::istream &in)
{
	char buffer[MAX_COLS_PER_ROW];
	in.getline (buffer, MAX_COLS_PER_ROW);
	std::string line (buffer);

	// read header
	if (line.find("# Petrel Points with attributes") != std::string::npos) {
		std::cout << "read points from petrel file" << std::endl;
		// read Version string
		in.getline (buffer, MAX_COLS_PER_ROW);
		line = buffer;
//		if (line.find ("VERSION 1") != std::string::npos) std::cout << "petrel version 1" << std::endl;
		// read header
		in.getline (buffer, MAX_COLS_PER_ROW);
		line = buffer;
//		if (line.find ("BEGIN HEADER") != std::string::npos) std::cout << "read petrel header " << std::endl;
		in.getline (buffer, MAX_COLS_PER_ROW);
		line = buffer;
		while (line.find("END HEADER") == std::string::npos) {
//			std::cout << line << std::endl;
			in.getline (buffer, MAX_COLS_PER_ROW);
			line = buffer;
		}

		std::cout << "read points ... " << std::flush;
		// read points
		size_t idx (pnt_vec->size());
		while (in) {
			pnt_vec->push_back (new GEOLIB::Point);
			in >> *((*pnt_vec)[idx]);
			if (!in) {
				delete (*pnt_vec)[idx];
				pnt_vec->pop_back ();
			} else idx++;
		}
		std::cout << "done" << std::endl;
	} else std::cout << "error: " << line << std::endl;
}


} // end namespace FileIO

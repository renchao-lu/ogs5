/*
 * GMSHInterface.h
 *
 *  Created on: Apr 29, 2010
 *      Author: TF
 */

#ifndef GMSHINTERFACE_H_
#define GMSHINTERFACE_H_

#include <string>

// GEOLIB
#include "GEOObjects.h"

namespace FileIO {

class GMSHInterface {
public:
	/**
	 * writes the geometric data (Points, Polylines, Surfaces) into a file of the GMSH format
	 * @param fname
	 * @param proj_name
	 * @param geo
	 * @return
	 */
	bool writeGMSHInputFile(const std::string &fname,
			const std::string &proj_name, const GEOLIB::GEOObjects& geo);
};

}

#endif /* GMSHINTERFACE_H_ */

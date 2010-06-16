/*
 * GMSHInterface.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: TF
 */

#include <fstream>

#include <GMSHInterface.h>

// GEOLIB
#include "Point.h"
#include "Polyline.h"
#include "Surface.h"

namespace FileIO {

bool GMSHInterface::writeGMSHInputFile(const std::string &fname,
		const std::string &proj_name, const GEOLIB::GEOObjects& geo)
{
	std::cerr << "GMSHInterface::writeGMSHInputFile *** " << std::endl;
	std::cerr << "get data from geo ... " << std::flush;
	// get data from geo
	const std::vector<GEOLIB::Point*> *pnts (geo.getPointVec (proj_name));
	const std::vector<GEOLIB::Polyline*> *plys (geo.getPolylineVec (proj_name));
	std::cerr << "ok" << std::endl;

	// open file
	std::ofstream out (fname.c_str());
	if (!out) return false;

	out << "// GMSH input file created by OpenGeoSys" << std::endl;
	out << std::endl;

	std::cerr << "write points ... " << std::flush;
	// write points
	float characteristic_len (10.0);
	size_t n (pnts->size());
	for (size_t k(0); k<n; k++) {
		out << "Point(" << k << ") = {" << *(*pnts)[k] << ","
				<< characteristic_len << "};" << std::endl;
	}
	std::cerr << "ok" << std::endl;

	std::cerr << "write Polylines ... " << std::flush;
	// write Polylines
	n = plys->size();
	size_t n_lines (0);
	for (size_t k(0); k<n; k++) {
		// write k-th Polyline
		GEOLIB::Polyline* ply ((*plys)[k]);
		size_t s (ply->getSize());
		for (size_t j(0); j<s-1; j++) {
			out << "Line(" << n_lines+j << ") = {" << ply->getPointID(j) << ","
					<< ply->getPointID(j+1) << "};" << std::endl;
		}
		out << "Line Loop (" << n_lines+s-1 << ") = {";
		for (size_t j(0); j<s-2; j++) {
			out << n_lines + j << ",";
		}
		out << n_lines + s-2 << "};" << std::endl << std::endl;
		if (k==0) {
			out << "Plane Surface (" << n_lines+s << ") = {" << n_lines+s-1 << "};" << std::endl;
			n_lines++;
		}
		n_lines += s;
	}
	out << std::endl;
	std::cerr << "ok" << std::endl;

	return true;
}

}

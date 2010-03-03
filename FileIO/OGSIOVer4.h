/*
 * OGSIOVer4.h
 *
 *  Created on: Jan 14, 2010
 *      Author: fischeth
 */

#ifndef OGSIOVER4_H_
#define OGSIOVER4_H_

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "Point.h"
#include "Polyline.h"
#include "Surface.h"
#include "StringTools.h"

// forward declaration
namespace GEOLIB {
class GEOObjects;
}

namespace FileIO {
/** I/O - routines for the OGS-4 gli file format */

/** reads the points from in_stream using the OGS-4 file format */
std::string readPoints(std::istream &in, std::vector<GEOLIB::Point*>* pnt_vec,
		bool &zero_based_indexing);

/** read a single Polyline from stream in into the ply_vec-vector */
std::string readPolyline(std::istream &in,
		std::vector<GEOLIB::Polyline*>* ply_vec,
		std::vector<std::string>& ply_vec_names,
		const std::vector<GEOLIB::Point*> &pnt_vec,
		bool zero_based_indexing);

/** reads polylines */
std::string readPolylines(std::istream &in,
		std::vector<GEOLIB::Polyline*>* ply_vec,
		std::vector<std::string>& ply_vec_names,
		const std::vector<GEOLIB::Point*> &pnt_vec,
		bool zero_based_indexing);

/** read a single Surface */
std::string readSurface(std::istream &in, std::vector<GEOLIB::Surface*> &sfc_vec,
		const std::vector<std::string>& ply_vec_names,
		const std::vector<GEOLIB::Polyline* > &ply_vec);

/** read a single Surface */
std::string readSurfaces(std::istream &in, std::vector<GEOLIB::Surface*> &sfc_vec,
		const std::vector<std::string>& ply_vec_names,
		const std::vector<GEOLIB::Polyline* > &ply_vec);

/** method reads geometric objects from file in gli format */
void readGLIFileV4 (const std::string& fname, GEOLIB::GEOObjects* geo);

} // end namespace

#endif /* OGSIOVER4_H_ */

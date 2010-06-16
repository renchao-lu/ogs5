/*
 * OGSIOVer4.h
 *
 *  Created on: Jan 14, 2010
 *      Author: TF / KR
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

///**
// * readPolylines reads Polylines
// * @param in input stream
// * @param ply_vec polyline vector
// * @param ply_vec_names the associated names with the polylines
// * @param pnt_vec the point vector
// * @param zero_based_indexing zero based indexing or one based indexing of points
// * @param path the path to external files
// * @return the next tag
// */
//std::string readPolylines(std::istream &in,
//		std::vector<GEOLIB::Polyline*>* ply_vec, std::vector<std::string>& ply_names,
//		std::vector<GEOLIB::Point*>& pnt_vec, bool zero_based_indexing,
//		const size_t *pnt_id_map, const std::string &path);
///**
// * readSurfaces reads Surfaces
// * @param in input stream
// * @param sfc_vec vector of GEOLIB::Surfaces
// * @param sfc_names vector of names of the GEOLIB::Surfaces
// * @param ply_vec vector of GEOLIB::Polylines
// * @param ply_vec_names vector of names of GEOLIB::Polylines
// * @param pnt_vec vector of GEOLIB::Points
// * @param path the path to external files
// * @return
// */
//std::string readSurfaces(std::istream &in,
//		std::vector<GEOLIB::Surface*> &sfc_vec, std::vector<std::string>& sfc_names,
//		const std::vector<GEOLIB::Polyline*> &ply_vec, const std::vector<std::string>& ply_vec_names,
//		std::vector<GEOLIB::Point*> &pnt_vec,
//		const std::string &path);

/** method reads geometric objects from file in gli format */
void readGLIFileV4 (const std::string& fname, GEOLIB::GEOObjects* geo);

} // end namespace

#endif /* OGSIOVER4_H_ */

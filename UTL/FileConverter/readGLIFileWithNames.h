/*
 * readGLIFileWithNames.h
 *
 *  Created on: May 17, 2010
 *      Author: TF
 */

// FileIO
#include "OGSIOVer4.h"

#ifndef READGLIFILEWITHNAME_H
#define READGLIFILEWITHNAME_H

/**
 * readPoints reads the point coordinates and point names in gli-format from stream in
 * @param in input stream
 * @param pnt_vec vector of pointers to GEOLIB::Point objects (storing mainly the coordinates)
 * @param zero_based_indexing are the indices read from gli-file zero-based?
 * @param pnt_names the names of the points
 * @return the next tag, for example #POLYLINE or #SURFACE
 */
std::string readPoints(std::istream &in, std::vector<GEOLIB::Point*>& pnt_vec,
		bool &zero_based_indexing, std::vector<std::string>& pnt_names);

/**
 * reads a gli-file from a given stream inclusive the names of the geometric objects
 * @param in the input stream
 * @param pnt_vec the point coordinates
 * @param pnt_names the names of the points
 * @param ply_vec the polylines
 * @param ply_names the names of the polylines
 * @param sfc_vec the surfaces
 * @param sfc_names the names of the surfaces
 * @param path the path for additional external files
 */
void readGLIFileWithNames (std::istream &in,
	std::vector<GEOLIB::Point*>& pnt_vec, std::vector<std::string>& pnt_names,
	std::vector<GEOLIB::Polyline*>& ply_vec, std::vector<std::string>& ply_names,
	std::vector<GEOLIB::Surface*>& sfc_vec, std::vector<std::string>& sfc_names,
	const std::string path
	);

#endif

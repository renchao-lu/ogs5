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

GMSHInterface::GMSHInterface (const std::string &fname) :
	_n_lines (0), // _n_line_loops (0),
	_n_plane_sfc(0)
{
	// open file
	_out.open (fname.c_str());
	// check file stream
	if (!_out) return;

	_out << "// GMSH input file created by OpenGeoSys" << std::endl;
	_out << std::endl;
	_out.precision (20);
}

GMSHInterface::~GMSHInterface ()
{
	_out.close ();
}

void GMSHInterface::writeGMSHPoints(const std::vector<GEOLIB::Point*>& pnt_vec)
{
	// write points
//	float characteristic_len (10.0);
	size_t n (pnt_vec.size());
	for (size_t k(0); k<n; k++) {
		_out << "Point(" << k << ") = {" << (*(pnt_vec[k]))[0] << ","
			<< (*(pnt_vec[k]))[1] << "," << (*(pnt_vec[k]))[2]
//			<< "," << characteristic_len
			<< "};" << std::endl;
	}
}

void GMSHInterface::writeGMSHPolyline (const GEOLIB::Polyline* ply)
{
	size_t s (ply->getSize());
	// write line segments (= Line) of the polyline
	for (size_t j(0); j<s-1; j++) {
		_out << "Line(" << _n_lines+j << ") = {" << ply->getPointID(j) << ","
				<< ply->getPointID(j+1) << "};" << std::endl;
	}
	// write the line segments contained in the polyline (=Line Loop)
	_out << "Line Loop (" << _n_lines + s - 1 << ") = {";
	for (size_t j(0); j<s-2; j++) {
		_out << _n_lines + j << ",";
	}
	_out << _n_lines + s-2 << "};" << std::endl;
	_n_lines += s;
}

void GMSHInterface::writeGMSHPolylines(const std::vector<GEOLIB::Polyline*>& ply_vec)
{
	size_t n (ply_vec.size());
	for (size_t k(0); k<n; k++) {
		// write k-th polyline
		writeGMSHPolyline (ply_vec[k]);
	}
	_out << std::endl;
}

void GMSHInterface::writeGMSHPolygon(const GEOLIB::Polygon& polygon)
{
	writeGMSHPolyline (&polygon);
	_polygon_list.push_back (_n_lines-1);
}

bool GMSHInterface::writeGMSHInputFile(const std::string &proj_name, const GEOLIB::GEOObjects& geo)
{
	std::cerr << "GMSHInterface::writeGMSHInputFile " << std::endl;
	std::cerr << "get data from geo ... " << std::flush;
	// get data from geo
	const std::vector<GEOLIB::Point*> *pnts (geo.getPointVec (proj_name));
	const std::vector<GEOLIB::Polyline*> *plys (geo.getPolylineVec (proj_name));
	std::cerr << "ok" << std::endl;

	// check file stream
	if (!_out) return false;

	// write points
	writeGMSHPoints (*pnts);

	// write Polylines
	writeGMSHPolylines (*plys);
	std::cerr << "ok" << std::endl;

	return true;
}

void GMSHInterface::writePlaneSurface ()
{
	_out << "Plane Surface (" << _n_plane_sfc << ") = {" << std::flush;
	std::list<size_t>::const_iterator it (_polygon_list.begin());
	_out << *it << std::flush;
	for (it++; it != _polygon_list.end(); it++)
		_out << ", " << *it << std::flush;
	_out << "};" << std::endl;
	_n_plane_sfc++;
	_polygon_list.clear();
}

} // end namespace FileIO

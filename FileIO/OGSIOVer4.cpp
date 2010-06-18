/*
 * OGSIOVer4.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: fischeth
 */

#include <sstream>
#include <iomanip>
// FileIO
#include "OGSIOVer4.h"
// Base
#include "StringTools.h"
#include "quicksort.h"
// GEO
#include "GEOObjects.h"
#include "Point.h"
#include "Polyline.h"
#include "Triangle.h"
#include "Surface.h"
// for tests only
#include "PointVec.h"

// MATHLIB
#include "AnalyticalGeometry.h"

using namespace GEOLIB;

namespace FileIO {
/**************************************************************************
 GeoLib- Funktion: readPoints
 Aufgabe: Lesen der GLI Points und schreiben in einen Vector
 08/2005 CC Implementation
 01/2010 TF big modifications
 **************************************************************************/
/** reads the points inclusive their names from input stream in
 * using the OGS-4 file format */
std::string readPoints(std::istream &in, std::vector<Point*>* pnt_vec,
		bool &zero_based_indexing, std::vector<std::string>* pnt_names)
{
	std::string line;
	size_t cnt(0);

	getline(in, line);
	// geometric key words start with the hash #
	// while not found a new key word do ...
	while (line.find("#") == std::string::npos && !in.eof() && !in.fail()) {
		// read id and point coordinates
		std::stringstream inss(line);
		size_t id;
		double x, y, z;
		inss >> id >> x >> y >> z;
		if (!inss.fail ()) {
			if (cnt == 0) {
				if (id == 0)
					zero_based_indexing = true;
				else
					zero_based_indexing = false;
			}
			pnt_vec->push_back(new Point(x, y, z));

			// read mesh density
			if (line.find("$MD") != std::string::npos) {
				double mesh_density;
				size_t pos1(line.find_first_of("M"));
				inss.str(line.substr(pos1 + 2, std::string::npos));
				inss >> mesh_density;
			}

			// read name of point
			std::string name;
			if (line.find("$ID") != std::string::npos) { //OK
				size_t pos1(line.find_first_of("I"));
				inss.str(line.substr(pos1 + 2, std::string::npos));
				inss >> name;
			} else {
				name = "POINT" + number2str (id);
			}
			pnt_names->push_back (name);

			cnt++;
		}
		getline(in, line);
	}

	return line;
}

/** reads points from a vector */
void readPolylinePointVector(const std::string &fname,
		std::vector<Point*>& pnt_vec, Polyline *ply, const std::string &path)
{
	// open file
	std::ifstream in((path+fname).c_str());
	if (!in)
		std::cerr << "error opening stream from " << fname << std::endl;

	double x, y, z;
	while (in) {
		in >> x >> y >> z;
		size_t pnt_id(pnt_vec.size());
		pnt_vec.push_back(new Point(x, y, z));
		ply->addPoint(pnt_id);
	}
}

/**************************************************************************
 GeoLib-Method: Read
 Task: Read polyline data from file
 Programing:
 03/2004 CC Implementation
 09/2004 OK file path for PLY files
 07/2005 CC PLY id
 08/2005 CC parameter
 09/2005 CC itoa - convert integer to string
 01/2010 TF cleaned method from unused variables
 **************************************************************************/
/** read a single Polyline from stream in into the ply_vec-vector */
std::string readPolyline(std::istream &in,
		std::vector<Polyline*>* ply_vec, std::vector<std::string>& ply_vec_names,
		std::vector<Point*>& pnt_vec, bool zero_based_indexing, const std::vector<size_t>& pnt_id_map,
		const std::string &path)
{
	std::string line;
	Polyline *ply(new Polyline(pnt_vec));
	size_t type;

	// Schleife ueber alle Phasen bzw. Komponenten
	do {
		in >> line;
		if (line.find("$ID") != std::string::npos) { // subkeyword found CC
			in >> line; // read value
			//			id = strtol(line_string.data(), NULL, 0);
		}
		//....................................................................
		if (line.find("$NAME") != std::string::npos) { // subkeyword found
			in >> line; // read value
			ply_vec_names.push_back(line);
		}
		//....................................................................
		if (line.find("$TYPE") != std::string::npos) { // subkeyword found
			in >> line; // read value
			type = static_cast<size_t> (strtol(line.c_str(), NULL, 0));
		}
		//....................................................................
		if (line.find("$EPSILON") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$MAT_GROUP") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$POINTS") != std::string::npos) { // subkeyword found
			// read the point ids
			in >> line;
			if (type != 100) {
				while (!in.eof() && line.size() != 0 && (line.find("#")
						== std::string::npos) && (line.find("$")
						== std::string::npos)) {
					size_t pnt_id(str2number<size_t> (line));
					if (!zero_based_indexing)
						pnt_id--; // one based indexing
					size_t ply_size (ply->getSize());
					if (ply_size > 0) {
						if (ply->getPointID (ply_size-1) != pnt_id_map[pnt_id])
							ply->addPoint(pnt_id_map[pnt_id]);
					} else {
						ply->addPoint(pnt_id_map[pnt_id]);
					}
					in >> line;
				}
			} else {
				std::cerr << "*** polyline is an arc *** reading not implemented" << std::endl;
				type = 2;
			}
			// empty line or the keyword or subkeyword or end of file
		}
		//....................................................................
		if (line.find("$POINT_VECTOR") != std::string::npos) { // subkeyword found
			in >> line; // read file name
			line = path + line;
			readPolylinePointVector(line, pnt_vec, ply, path);
		} // subkeyword found
	} while (line.find("#") == std::string::npos && line.size() != 0 && in);

	ply_vec->push_back(ply);

	return line;
}

/**************************************************************************
 GEOLib-Function:
 Task: polyline read function
 Programming:
 03/2004 CC Implementation
 05/2004 CC Modification
 04/2005 CC Modification calculate the minimal distance between points reference for
 mesh density of line element calculation
 07/2005 CC read ID of polyline
 08/2005 CC parameter
 01/2010 TF changed signature of function
 **************************************************************************/
/** reads polylines */
std::string readPolylines(std::istream &in, std::vector<Polyline*>* ply_vec,
		std::vector<std::string>& ply_vec_names, std::vector<Point*>& pnt_vec,
		bool zero_based_indexing, const std::vector<size_t>& pnt_id_map,
		const std::string &path) {
	if (!in) {
		std::cerr << "*** readPolylines input stream error " << std::endl;
		return std::string("");
	}
	std::string tag("#POLYLINE");

	while (!in.eof() && tag.find("#POLYLINE") != std::string::npos) {
		tag = readPolyline(in, ply_vec, ply_vec_names, pnt_vec,
				zero_based_indexing, pnt_id_map, path);
	}
	return tag;
}

void readTINFile(const std::string &fname, Surface* sfc,
		std::vector<Point*> &pnt_vec)
{
	// open file
	std::ifstream in(fname.c_str());
	if (!in)
		std::cerr << "readTINFile error opening stream from " << fname
				<< std::endl;

	size_t id;
	double x, y, z;
	while (in) {
		// read id
		in >> id;
		// determine size
		size_t pnt_pos(pnt_vec.size());
		// read first point
		in >> x >> y >> z;
		pnt_vec.push_back(new Point(x, y, z));
		// read second point
		in >> x >> y >> z;
		pnt_vec.push_back(new Point(x, y, z));
		// read third point
		in >> x >> y >> z;
		pnt_vec.push_back(new Point(x, y, z));
		// create new Triangle
		sfc->addTriangle(pnt_pos, pnt_pos+1, pnt_pos+2);
	}
}

/**************************************************************************
 GeoLib-Method: readSurface
 Task: Read surface data from input stream
 Programing:
 03/2004 OK Implementation
 05/2005 OK EPSILON
 09/2005 CC file_path_base
 01/2010 TF signatur modification, reimplementation
 **************************************************************************/
/** read a single Surface */
std::string readSurface(std::istream &in,
		std::vector<Surface*> &sfc_vec, std::vector<std::string>& sfc_names,
		const std::vector<Polyline*> &ply_vec, const std::vector<std::string>& ply_vec_names,
		std::vector<Point*> &pnt_vec,
		const std::string &path)
{
	std::string line;
	Surface *sfc(new Surface(pnt_vec));

	int type (-1);
	std::string name;

	do {
		in >> line;
		if (line.find("$ID") != std::string::npos) { // subkeyword found CC
			in >> line; // read value
			//			id = strtol(line_string.data(), NULL, 0);
		}
		//....................................................................
		if (line.find("$NAME") != std::string::npos) { // subkeyword found
			in >> line; // read value
			name = line.substr(0);
		}
		//....................................................................
		if (line.find("$TYPE") != std::string::npos) { // subkeyword found
			in >> line; // read value
			type = strtol(line.c_str(), NULL, 0);
//			if (type == 3)
//				std::cerr
//						<< "surface type 3: flat surface with any normal direction - - reading not implemented"
//						<< std::endl;
//			if (type == 100)
//				std::cerr << "cylindrical surface - reading not implemented"
//						<< std::endl;
//			if (type == 2)
//				std::cerr << "vertical surface - reading not implemented"
//						<< std::endl;
		}
		//....................................................................
		if (line.find("$EPSILON") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$TIN") != std::string::npos) { // subkeyword found
			in >> line; // read value (file name)
			line = path + line;
//			if (type == 1) std::cerr << "reading tin file " << line << " ... " << std::flush;
			readTINFile(line, sfc, pnt_vec);
//			std::cout << "ok" << std::endl;
		}
		//....................................................................
		if (line.find("$MAT_GROUP") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$POLYLINES") != std::string::npos) { // subkeyword found
			// read the polylines (as std::string)

//			std::cout << "read polyline for surface of type " << type << std::endl;

			in >> line;
			while (!in.eof() && line.size() != 0 && (line.find("#")
					== std::string::npos) && (line.find("$")
					== std::string::npos)) {
				// search polyline
				size_t ply_id(0);
				while (ply_id < ply_vec.size()
						&& ply_vec_names[ply_id].compare(line) != 0) {
					ply_id++;
				}
				if (ply_id == ply_vec.size())
					std::cerr << "polyline for surface not found!" << std::endl;
				else {
					if (type == 3) {
						std::cerr << "surface type 3: flat surface with any normal direction - - reading not implemented"
								<< std::endl;
					}

					if (type == 2) {
						std::cerr << "vertical surface - reading not implemented"
												<< std::endl;
					}

					if (type == -1 || type == 0) {

						if (ply_vec[ply_id]->isClosed()) {
							// compute triangulation of closed polyline (polygon)
							std::list<GEOLIB::Polyline*> ply_list;
							ply_list.push_back (ply_vec[ply_id]);
							MATHLIB::getListOfSimplePolygons(ply_list, pnt_vec);

							for (std::list<GEOLIB::Polyline*>::const_iterator ply_it (ply_list.begin());
								ply_it != ply_list.end(); ply_it++) {
								std::list<GEOLIB::Triangle> triangles;
								MATHLIB::earClippingTriangulationOfPolygon(*ply_it, triangles);
//								std::cout << "done - " << triangles.size () << " triangles " << std::endl;

								// add Triangles to Surface
								std::list<GEOLIB::Triangle>::const_iterator it (triangles.begin());
								while (it != triangles.end()) {
									sfc->addTriangle ((*it)[0], (*it)[1], (*it)[2]);
									it++;
								}
							}
						}
					}
				}
				in >> line;
			}
			// empty line or a keyword is found
		}
	} while (line.find("#") == std::string::npos && line.size() != 0 && in);

	sfc_names.push_back (name);
	sfc_vec.push_back(sfc);

	return line;
}

/**************************************************************************
 GEOLib-Method:
 Task: Surface read function
 Programming:
 03/2004 OK Implementation
 05/2004 CC Modification
 01/2010 TF changed signature of function, big modifications
 **************************************************************************/
std::string readSurfaces(std::istream &in,
		std::vector<Surface*> &sfc_vec, std::vector<std::string>& sfc_names,
		const std::vector<Polyline*> &ply_vec, const std::vector<std::string>& ply_vec_names,
		std::vector<Point*> &pnt_vec,
		const std::string &path)
{
	if (!in.good()) {
		std::cerr << "*** readSurfaces input stream error " << std::endl;
		return std::string("");
	}
	std::string tag("#SURFACE");

	while (!in.eof() && tag.find("#SURFACE") != std::string::npos) {
		tag = readSurface(in, sfc_vec, sfc_names, ply_vec, ply_vec_names, pnt_vec, path);
	}

	return tag;
}

void readGLIFileV4(const std::string& fname, GEOObjects* geo)
{
	std::cout << "GEOLIB::readGLIFile open stream from file " << fname
			<< " ... " << std::flush;
	std::ifstream in(fname.c_str());
	if (!in)
		std::cerr << "error opening stream from " << fname << std::endl;
	std::cout << "done" << std::endl;

	std::string tag;
	while (tag.find("#POINTS") == std::string::npos && !in.eof()) {
		getline (in, tag);
	}

	// read names of points into vector of strings
	std::vector<std::string>* pnt_names (new std::vector<std::string>);
	bool zero_based_idx(true);
	std::vector<Point*> *pnt_vec(new std::vector<Point*>);
	std::cout << "read points from stream ... " << std::flush;
	tag = readPoints(in, pnt_vec, zero_based_idx, pnt_names);
	std::cout << " ok, " << pnt_vec->size() << " points read" << std::endl;

	std::string unique_name(fname);
	if (!pnt_vec->empty())
		geo->addPointVec(pnt_vec, unique_name, pnt_names); // KR: insert into GEOObjects if not empty

	// extract path for reading external files
	size_t pos(fname.rfind("/"));
	std::string path(fname.substr(0, pos + 1));

	std::vector<std::string> ply_names; // read names of plys into temporary string-vec
	std::vector<Polyline*> *ply_vec(new std::vector<Polyline*>);
	if (tag.find("#POLYLINE") != std::string::npos && in) {
		std::cout << "read polylines from stream ... " << std::flush;
		tag = readPolylines(in, ply_vec, ply_names, *pnt_vec,
				zero_based_idx, geo->getPointVecObj(unique_name)->getIDMap(), path);
		std::cout << " ok, " << ply_vec->size() << " polylines read"
				<< std::endl;
	} else
		std::cerr
				<< "tag #POLYLINE not found or input stream error in GEOObjects"
				<< std::endl;

	std::vector<Surface*> *sfc_vec(new std::vector<Surface*>);
	std::vector<std::string> sfc_names;
	if (tag.find("#SURFACE") != std::string::npos && in) {
		std::cout << "read surfaces from stream ... " << std::flush;
		tag = readSurfaces(in, *sfc_vec, sfc_names, *ply_vec, ply_names, *pnt_vec, path);
		std::cout << " ok, " << sfc_vec->size() << " surfaces read"
				<< std::endl;
	} else
		std::cerr
				<< "tag #SURFACE not found or input stream error in GEOObjects"
				<< std::endl;
	in.close();

//	std::ofstream out ("test.gli", std::ios::out);
//	if (out) {
//		out << "#POINTS" << std::endl;
//		for (size_t k(0); k<(*ply_vec)[1]->getSize(); k++) {
//			out << k << " " << (*(*ply_vec)[1])[k] << std::endl;
//		}
//		out << "#POLYLINE" << std::endl;
//		out << "$NAME" << std::endl;
//		out << " PLY0" << std::endl;
//		out << "$POINTS" << std::endl;
//		for (size_t k(0); k<(*ply_vec)[1]->getSize(); k++) {
//			out << k << std::endl;
//		}
//		out << 0 << std::endl;
//		out << "#SURFACE" << std::endl;
//		out << "$NAME" << std::endl;
//		out << " SFC0" << std::endl;
//		out << "$POLYLINES" << std::endl;
//		out << " PLY0" << std::endl;
//		out.close ();
//	}

	if (!ply_vec->empty())
		geo->addPolylineVec(ply_vec, unique_name); // KR: insert into GEOObjects if not empty
	if (!sfc_vec->empty())
		geo->addSurfaceVec(sfc_vec, unique_name); // KR: insert into GEOObjects if not empty
}

} // end namespace

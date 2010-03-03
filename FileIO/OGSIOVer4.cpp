/*
 * OGSIOVer4.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: fischeth
 */

#include "OGSIOVer4.h"
#include <sstream>
#include "StringTools.h"
#include "GEOObjects.h"
#include "Point.h"
#include "Polyline.h"
#include "Surface.h"


using namespace GEOLIB;

namespace FileIO {
/**************************************************************************
GeoLib- Funktion: readPoints
Aufgabe: Lesen der GLI Points und schreiben in einen Vector
08/2005 CC Implementation
01/2010 TF big modifications
**************************************************************************/
std::string readPoints(std::istream &in, std::vector<Point*>* pnt_vec,
		bool &zero_based_indexing)
{
	const size_t MAX_ROW_LENGTH = 256;
	char buffer[MAX_ROW_LENGTH];
	in.getline (buffer, MAX_ROW_LENGTH);
	std::string line (buffer);

	if (line.find("#POINTS") == std::string::npos) return line;

	size_t cnt (0);

	in.getline (buffer, MAX_ROW_LENGTH);
	line = buffer;
	// geometric key words start with the hash #
	// while not found a new key word do ...
	while (line.find("#") == std::string::npos && !in.eof()) {
		// read id and point coordinates
		std::stringstream inss(line);
		size_t id;
		double x, y, z;
		inss >> id >> x >> y >> z;
		if (cnt == 0) {
			if (id == 0) zero_based_indexing = true;
			else zero_based_indexing = false;
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
		}
		in.getline (buffer, MAX_ROW_LENGTH);
		line = buffer;
		cnt++;
	}

	return line;
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
std::string readPolyline(std::istream &in, std::vector<Polyline*>* ply_vec,
		std::vector<std::string>& ply_vec_names,
		const std::vector<Point*>& pnt_vec, bool zero_based_indexing)
{
	std::string line;
	Polyline *ply (new Polyline (pnt_vec));

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
			ply_vec_names.push_back (line);
		}
		//....................................................................
		if (line.find("$TYPE") != std::string::npos) { // subkeyword found
			in >> line; // read value
//			type = strtol(line_string.data(), NULL, 0);
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
			while (!in.eof() && line.size() != 0 &&
					(line.find("#") == std::string::npos) &&
					(line.find("$") == std::string::npos)) {
				size_t pnt_id (str2number<size_t> (line));
				if (!zero_based_indexing) pnt_id--; // one based indexing
				ply->addPoint (pnt_id);
				in >> line;
			}
			// empty line or the keyword or subkeyword or end of file
		}
		//....................................................................
		if (line.find("$POINT_VECTOR") != std::string::npos) { // subkeyword found
			in >> line; // read file name
//			ReadPointVector(line_string);//CC
		} // subkeyword found
	} while (line.find("#") == std::string::npos && line.size() != 0 && in);

	ply_vec->push_back (ply);

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
std::string readPolylines (std::istream &in, std::vector<Polyline*>* ply_vec,
		std::vector<std::string>& ply_vec_names,
		const std::vector<Point*>& pnt_vec, bool zero_based_indexing)
{
	if (!in) {
		std::cerr << "*** readPolylines input stream error " << std::endl;
		return std::string ("");
	}
	std::string tag ("#POLYLINE");

	while (!in.eof() && tag.find("#POLYLINE") != std::string::npos) {
		tag = readPolyline (in, ply_vec, ply_vec_names, pnt_vec, zero_based_indexing);
	}
	return tag;
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
std::string readSurface (std::istream &in, std::vector<Surface*> &sfc_vec,
		const std::vector<std::string>& ply_vec_names,
		const std::vector<Polyline*> &ply_vec)
{
	std::string line;
	Surface *sfc (new Surface (ply_vec));

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
//			name = line.substr(0);
		}
		//....................................................................
		if (line.find("$TYPE") != std::string::npos) { // subkeyword found
			in >> line; // read value
//			type = strtol(line_string.data(), NULL, 0);
		}
		//....................................................................
		if (line.find("$EPSILON") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$TIN") != std::string::npos) { // subkeyword found
			in >> line; // read value (file name)
		}
		//....................................................................
		if (line.find("$MAT_GROUP") != std::string::npos) { // subkeyword found
			in >> line; // read value
		}
		//....................................................................
		if (line.find("$POLYLINES") != std::string::npos) { // subkeyword found
			// read the polylines (as std::string)
			in >> line;
			while (!in.eof() && line.size() != 0 &&
					(line.find("#") == std::string::npos) &&
					(line.find("$") == std::string::npos)) {
				// search polyline
				size_t ply_id (0);
				while (ply_id < ply_vec.size() && ply_vec_names[ply_id].compare(line) != 0) {
					ply_id++;
				}
				if (ply_id == ply_vec.size()) std::cerr << "polyline for surface not found!" << std::endl;
				else {
					sfc->addPolyline (ply_id);
//					std::cout << "Surface " << sfc->getName() << " is bounded by polyline " << ply_vec_names[ply_id] << std::endl;
				}
				in >> line;
			}
			// empty line or a keyword is found
		}
	} while (line.find("#") == std::string::npos && line.size() != 0 && in);

	sfc_vec.push_back (sfc);

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
std::string readSurfaces(std::istream &in, std::vector<Surface*> &sfc_vec,
		const std::vector<std::string>& ply_vec_names,
		const std::vector<Polyline*> &ply_vec)
{
	if (!in.good()) {
		std::cerr << "*** readSurfaces input stream error " << std::endl;
		return std::string ("");
	}
	std::string tag ("#SURFACE");

	while (!in.eof() && tag.find("#SURFACE") != std::string::npos) {
		tag = readSurface (in, sfc_vec, ply_vec_names, ply_vec);
	}
	return tag;
}

void readGLIFileV4 (const std::string& fname, GEOObjects* geo)
{
	std::cout << "GEOLIB::readGLIFile open stream from file " << fname
			<< " ... " << std::flush;
	std::ifstream in(fname.c_str());
	if (!in)
		std::cerr << "error opening stream from " << fname << std::endl;
	std::cout << "done" << std::endl;

	bool zero_based_idx(true);
	std::vector<Point*> *pnt_vec (new std::vector<Point*>);
	std::cout << "read points from stream ... " << std::flush;
	std::string tag(readPoints(in, pnt_vec, zero_based_idx));
	std::cout << " ok: " << pnt_vec->size() << " points read" << std::endl;

	std::vector<std::string> ply_vec_names; // read names of plys into temporary string-vec
	std::vector<Polyline*> *ply_vec (new std::vector<Polyline*>);
	if (tag.find("#POLYLINE") != std::string::npos && in) {
		std::cout << "read polylines from stream ... " << std::flush;
		tag = readPolylines(in, ply_vec, ply_vec_names, *pnt_vec, zero_based_idx);
		std::cout << " ok, " << ply_vec->size() << " polylines read" << std::endl;
	} else
		std::cerr
				<< "tag #POLYLINE not found or input stream error in GEOObjects"
				<< std::endl;

	std::vector<Surface*> *sfc_vec (new std::vector<Surface*>);
	if (tag.find("#SURFACE") != std::string::npos && in) {
		std::cout << "read surfaces from stream ... " << std::flush;
		tag = readSurfaces(in, *sfc_vec, ply_vec_names, *ply_vec);
		std::cout << " ok, " << sfc_vec->size() << " surfaces read" << std::endl;
	} else
		std::cerr
				<< "tag #SURFACE not found or input stream error in GEOObjects"
				<< std::endl;
	in.close();

	std::string unique_name (fname);
	if (!pnt_vec->empty()) geo->addPointVec(pnt_vec, unique_name);		// KR: insert into GEOObjects if not empty
	if (!ply_vec->empty()) geo->addPolylineVec(ply_vec, unique_name);	// KR: insert into GEOObjects if not empty
	if (!sfc_vec->empty()) geo->addSurfaceVec(sfc_vec, unique_name);	// KR: insert into GEOObjects if not empty
}

} // end namespace

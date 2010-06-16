/*
 * readGLIFileWithNames.cpp
 *
 *  Created on: May 17, 2010
 *      Author: TF
 */

#include "readGLIFileWithNames.h"

/** reads the points inclusive their names from input stream in
 * using the OGS-4 file format */
std::string readPoints(std::istream &in, std::vector<GEOLIB::Point*>& pnt_vec,
		bool &zero_based_indexing, std::vector<std::string>& pnt_names)
{
	const size_t MAX_ROW_LENGTH = 256;
	char buffer[MAX_ROW_LENGTH];
	in.getline(buffer, MAX_ROW_LENGTH);
	std::string line(buffer);

	size_t cnt(0);
	in.getline(buffer, MAX_ROW_LENGTH);
	line = buffer;
	// geometric key words start with the hash #
	// while not found a new key word do ...
	while (line.find("#") == std::string::npos && !in.eof() && !in.fail()) {
		// read id and point coordinates
		if (line.size() > 0) {
			std::stringstream inss(line);
			size_t id;
			double x, y, z;
			inss >> id >> x >> y >> z;
			if (! inss.fail()) {
				if (cnt == 0) {
					if (id == 0)
						zero_based_indexing = true;
					else
						zero_based_indexing = false;
				}
				pnt_vec.push_back(new GEOLIB::Point(x, y, z));

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
					name = number2str (id);
				}
				pnt_names.push_back (name);
				cnt++;
			}
			in.getline(buffer, MAX_ROW_LENGTH);
			line = buffer;
		}
	}

	return line;
}

void readGLIFileWithNames (std::istream &in,
	std::vector<GEOLIB::Point*>& pnt_vec, std::vector<std::string>& pnt_names,
	std::vector<GEOLIB::Polyline*>& ply_vec, std::vector<std::string>& ply_names,
	std::vector<GEOLIB::Surface*>& sfc_vec, std::vector<std::string>& sfc_names,
	const std::string path
	)
{
	std::string tag;
	while (tag.find("#POINTS") == std::string::npos) {
		getline (in, tag);
	}

	bool zero_based_idx(true);

	std::cout << "read points from stream ... " << std::flush;
	tag = readPoints(in, pnt_vec, zero_based_idx, pnt_names);
	std::cout << " ok: " << pnt_vec.size() << " points read" << std::endl;

	if (tag.find("#POLYLINE") != std::string::npos && in) {
		std::cout << "read polylines from stream ... " << std::flush;

		size_t n_pnts_in_file (pnt_vec.size());
		size_t *pnt_id_map (new size_t[n_pnts_in_file]);
		for (size_t k(0); k<n_pnts_in_file; k++) pnt_id_map[k] = k;

		tag = FileIO::readPolylines(in, &ply_vec, ply_names, pnt_vec, zero_based_idx, pnt_id_map, path);
		std::cout << " ok, " << ply_vec.size() << " polylines read"
				<< std::endl;
	} else
		std::cerr
				<< "tag #POLYLINE not found or input stream error in GEOObjects"
				<< std::endl;

	if (tag.find("#SURFACE") != std::string::npos && in) {
		std::cout << "read surfaces from stream ... " << std::flush;
		tag = FileIO::readSurfaces(in, sfc_vec, sfc_names, ply_vec, ply_names, pnt_vec, path);
		std::cout << " ok, " << sfc_vec.size() << " surfaces read"
				<< std::endl;
	} else
		std::cerr
				<< "tag #SURFACE not found or input stream error in GEOObjects"
				<< std::endl;
}

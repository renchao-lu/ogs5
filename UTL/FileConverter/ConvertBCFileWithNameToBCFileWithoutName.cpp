/*
 * ConvertBCFileWithNameToBCFileWithoutName.cpp
 *
 *  Created on: May 17, 2010
 *      Author: TF
 */

#include <vector>
#include <string>
#include <iomanip>

// Base
#include "StringTools.h"

// GEOLIB
#include "Point.h"
#include "Polyline.h"
#include "Surface.h"

// FEM
#include "rf_st_new.h"
#include "files0.h"
#include "problem.h"
Problem *aproblem = NULL;

//
#include "readGLIFileWithNames.h"

ios::pos_type ReadSingleBC(std::ifstream& bc_file, CBoundaryCondition *bc)
{
	string line_string;
	bool new_keyword = false;
	bool new_subkeyword = false;
	string hash("#");
	ios::pos_type position;
	string sub_string, strbuff;
	int ibuff; //pos,
	double dbuff; //WW
	ios::pos_type position_line;
	std::stringstream in;
	int nLBC;

	size_t pos;

	// loop over all components
	while (!new_keyword) {
		new_subkeyword = false;
		position = bc_file.tellg();
//		line_string = GetLineFromFile1(&bc_file);
		getline (bc_file, line_string);
		if (line_string.size() < 1)
			break;
		if (line_string.find(hash) != string::npos) {
			new_keyword = true;
			break;
		}
		//....................................................................
		if (line_string.find("$PCS_TYPE") != string::npos) { // subkeyword found
			getline (bc_file, bc->pcs_type_name);
			in.clear();
		}
		//....................................................................
		if (line_string.find("$PRIMARY_VARIABLE") != string::npos) { // subkeyword found
			getline (bc_file, bc->pcs_pv_name);
			in.clear();
		}
		//....................................................................
		if (line_string.find("$GEO_TYPE") != string::npos) { //subkeyword found
			getline (bc_file, sub_string);
			in.str(sub_string);
			in >> sub_string;
			if (sub_string.compare("POINT") == 0) { //OK
				bc->geo_type_name = "POINT";
				bc->geo_type = 0;
			}
			if (sub_string.find("POLYLINE") != string::npos) {
				bc->geo_type_name = "POLYLINE";
				CGLPolyline* m_ply = NULL;
				m_ply = GEOGetPLYByName(bc->getGeoName()); //CC 10/05
				if (!m_ply)
					cout << "Warning in BCRead: no PLY data" << endl;
				bc->geo_type = 1;
			}

			if (sub_string.find("SURFACE") != string::npos) {
				bc->geo_type_name = "SURFACE";
				bc->geo_type = 2;
			}
			if (sub_string.find("VOLUME") != string::npos) {
				bc->geo_type_name = "VOLUME";
				bc->geo_type = 3;
			}
			std::string t;
			in >> t;
			bc->setGeoName (t);
		}
		//....................................................................
		if (line_string.find("$DIS_TYPE") != string::npos) {
			getline (bc_file, line_string);
			in.str (line_string);
			in >> line_string;

			bc->periodic = false; // JOD

			if (line_string.find("DIRECT") != string::npos) {
				bc->dis_type_name = "DIRECT";
				std::string t;
				in >> t;
				bc->setFileName (FilePath + t);
				in.clear();
			}
			// Patch-wise constant. 19.03.2009. WW
			if (line_string.find("PATCH_C") != string::npos) {
				bc->dis_type_name = "PATCH_C";
				std::string t;
				in >> t;
				bc->setFileName (FilePath + t);
				in >> bc->geo_node_value;
				in.clear();
			}

			if ((pos = line_string.find("CONSTANT")) != std::string::npos) {
				bc->dis_type_name = "CONSTANT";
				bc->dis_type = 0;
				pos += 9;
				bc->geo_node_value = str2number<double>(line_string.substr (pos, line_string.size()-pos));
			}
			if (line_string.find("POINTS") != string::npos) {
				bc->dis_type_name = "POINTS";
				bc->dis_type = 1;

				in >> bc->db_file_name; //sub_line
				in.clear();
			}
			if (line_string.find("LINEAR") != string::npos) {
				bc->dis_type_name = "LINEAR";
				bc->dis_type = 2;
				// Distribued. WW
				in >> nLBC; //sub_line
				in.clear();

				for (int i = 0; i<nLBC; i++) {
					in.str(GetLineFromFile1(&bc_file));
					in >> ibuff >> dbuff >> strbuff;
					in.clear();

					bc->PointsHaveDistribedBC.push_back(ibuff);
					bc->DistribedBC.push_back(dbuff);
					if (strbuff.size() > 0) {
						bc->PointsFCTNames.push_back(strbuff);
						bc->time_dep_interpol = true;
					}
				}
			}
			if (line_string.find("DATA_BASE") != string::npos) {
				bc->dis_type_name = "DATA_BASE";
				bc->dis_type = 3;
				in >> bc->db_file_name; //sub_line
				in.clear();
			}
//			// TF 18/05/2010 not used anymore ?!
//			if (line_string.find("SUBSTITUTE") != string::npos) {
//				dis_type_name = "VARIABLE";
//				in >> geo_node_substitute; //sub_line
//				in.clear();
//			}
			if (line_string.find("PERIODIC") != string::npos) { // JOD
				bc->dis_type_name = "PERIODIC";
				bc->periodic = true;
				bc->dis_type = 0;
				in >> bc->geo_node_value;
				in.clear();
				std::string t;
				getline (bc_file, t);
				in.str(t);
				in >> bc->periode_time_length >> bc->periode_phase_shift; //sub_line
				in.clear();
			} // subkeyword found
		}
		// Time dependent function
		//..Time dependent curve ............................................
		if (line_string.find("$TIM_TYPE") != string::npos) { // subkeyword found
			getline (bc_file, line_string);
			in.str(line_string);
			in >> line_string;

			if (line_string.find("CURVE") != string::npos) {
				bc->setTimTypeName ("CURVE");
				bc->dis_type = 0;
				int t;
				in >> t;
				bc->setCurveIndex (t);
				in.clear();
			}
			continue;
		}
		//....................................................................
		if (line_string.find("$FCT_TYPE") != string::npos) { // subkeyword found
			getline (bc_file, bc->fct_name);
			in.str(bc->fct_name);
			in >> bc->fct_name;
			in.clear();
		}

		if (line_string.find("$MSH_TYPE") != string::npos) { //subkeyword found
			getline (bc_file, sub_string);
			in.str(sub_string);
			in >> sub_string;
			bc->msh_type_name = "NODE";
			if (sub_string.find("NODE") != string::npos) {
				in >> bc->msh_node_number;
				in.clear();
			}
		}
		//....................................................................
		//OK4108
		if (line_string.find("$DIS_TYPE_CONDITION") != string::npos) { // subkeyword found
			getline (bc_file, line_string);
			in.str(line_string);
			in >> line_string;
			if (line_string.find("CONSTANT") != string::npos) {
				bc->dis_type_name = "CONSTANT";
				bc->dis_type = 0;
				in >> bc->geo_node_value;
				in.clear();
			}
			in.str(GetLineFromFile1(&bc_file)); // 0.0 IF HEAD > 0.04
			in >> bc->node_value_cond >> line_string >> bc->pcs_pv_name_cond
					>> line_string >> bc->condition;
			in.clear();
			in.str(GetLineFromFile1(&bc_file)); // PCS OVERLAND_FLOW
			in >> line_string >> bc->pcs_type_name_cond;
			in.clear();
			bc->conditional = true;
		}
		//....................................................................
	}
	return position;
}

/**************************************************************************
 FEMLib-Method:
 Task: BC read function
 Programing:
 01/2004 OK Implementation
 01/2005 OK Boolean type
 01/2005 OK Destruct before read
 **************************************************************************/
bool BCRead(std::ifstream& in_bc_stream, std::vector<CBoundaryCondition*>& bc_vec)
{
	char line[MAX_ZEILE];
	std::string line_string;
	ios::pos_type position;

	if (!in_bc_stream.good()) {
		std::cout << "! Error in BCRead: No boundary conditions !" << std::endl;
		return false;
	}

	// Keyword loop
	std::cout << "BCRead" << std::endl;
	while (!in_bc_stream.eof()) {
		in_bc_stream.getline(line, MAX_ZEILE);
		line_string = line;
		if (line_string.find("#STOP") != string::npos)
			return true;
		//----------------------------------------------------------------------
		if (line_string.find("#BOUNDARY_CONDITION") != string::npos) { // keyword found
			CBoundaryCondition *bc (new CBoundaryCondition());
			position = ReadSingleBC (in_bc_stream, bc);
			bc_vec.push_back(bc);
			in_bc_stream.seekg(position, ios::beg);
		} // keyword found
	} // eof
	return true;
}

/**************************************************************************
 FEMLib-Method: BCWrite
 Task: master write function
 Programing:
 02/2004 OK Implementation
 last modification:
 **************************************************************************/
void BCWrite (std::ofstream& out_bc_stream, const std::vector<GEOLIB::Point*>& pnt_vec,
		const std::vector<std::string>& pnt_names,
		const std::vector<GEOLIB::Polyline*>& ply_vec,
		const std::vector<std::string>& ply_names,
		const std::vector<GEOLIB::Surface*>& sfc_vec,
		const std::vector<std::string>& sfc_names,
		const std::vector<CBoundaryCondition*>& bc_vec)
{
	if (!out_bc_stream.good()) return;

	out_bc_stream.setf(ios::scientific, ios::floatfield);
	out_bc_stream.precision(12);

	out_bc_stream << "GeoSys-BC: Boundary Conditions ------------------------------------------------\n";

	std::vector<CBoundaryCondition*>::const_iterator bc_it (bc_vec.begin());
	while (bc_it != bc_vec.end()) {
		//KEYWORD
		out_bc_stream << "#BOUNDARY_CONDITION" << std::endl;
		//NAME+NUMBER
		out_bc_stream << " $PCS_TYPE" << std::endl;
		out_bc_stream << "  " << (*bc_it)->pcs_type_name << std::endl;
		out_bc_stream << " $PRIMARY_VARIABLE" << std::endl;
		out_bc_stream << "  " << (*bc_it)->pcs_pv_name << std::endl;
		//GEO_TYPE
		out_bc_stream << " $GEO_TYPE" << std::endl;
		out_bc_stream << "  ";
		bool nfound (true);
		if ((*bc_it)->geo_type_name.compare ("POINT") == 0) {
			// search in vector pnt_names
			for (size_t j(0); j<pnt_vec.size() && nfound; j++) {
				if ((*bc_it)->getGeoName ().compare (pnt_names[j]) == 0) {
					nfound = false;
					out_bc_stream << (*bc_it)->geo_type_name << " " << j << std::endl;
				} else {
					if ((*bc_it)->getGeoName ().compare ("POINT"+pnt_names[j]) == 0) {
						nfound = false;
						out_bc_stream << (*bc_it)->geo_type_name << " " << j << std::endl;
					}
				}
			}
		} else {
			out_bc_stream << (*bc_it)->geo_type_name << " " << (*bc_it)->getGeoName () << std::endl;
		}
//			if ((*bc_it)->geo_type_name.compare("POLYLINE") == 0) {
//				// search in vector ply_names
//				for (size_t j(0); j<ply_vec.size() && nfound; j++) {
//					if ((*bc_it)->getGeoName ().compare (ply_names[j]) == 0) {
//						nfound = false;
//						out_bc_stream << (*bc_it)->geo_type_name << " " << j << std::endl;
//					}
//				}
//			} else {
//				if ((*bc_it)->geo_type_name.compare("SURFACE") == 0) {
//					// search in vector sfc_names
//					for (size_t j(0); j<sfc_vec.size() && nfound; j++) {
//						if ((*bc_it)->getGeoName ().compare (sfc_names[j]) == 0) {
//							nfound = false;
//							out_bc_stream << (*bc_it)->geo_type_name << " " << j << std::endl;
//						}
//					}
//				}  else {
//					out_bc_stream << (*bc_it)->geo_type_name << " " << (*bc_it)->getGeoName () << std::endl;
//				}
//			}
//		}
		//DIS_TYPE
		out_bc_stream << " $DIS_TYPE" << std::endl;
		out_bc_stream << "  ";
		out_bc_stream << (*bc_it)->dis_type_name;
		switch ((*bc_it)->dis_type_name[0]) {
		case 'C': // Constant
			out_bc_stream << " " << (*bc_it)->geo_node_value << std::endl;
			break;
		case 'L': // Linear
			out_bc_stream << " " << (*bc_it)->PointsHaveDistribedBC.size() << std::endl;
			for (size_t i=0; i<(*bc_it)->PointsHaveDistribedBC.size(); i++) {
				out_bc_stream << "  " << (*bc_it)->PointsHaveDistribedBC[i] << " ";
				out_bc_stream << "  " << (*bc_it)->DistribedBC[i] << std::endl;
			}
			break;
		case 'N': //OK4801 NOD
			out_bc_stream << " " << (*bc_it)->geo_node_value << std::endl;
			break;
		}
		//FCT
		if ((*bc_it)->fct_name.length() > 0) { //OK4108
			out_bc_stream << " $FCT_TYPE" << std::endl;
			out_bc_stream << "  " << (*bc_it)->fct_name << std::endl;
		}

		++bc_it;
	}
	out_bc_stream << "#STOP";
}


int main (int argc, char *argv[])
{
	if (argc != 2) {
		std::cout << "Usage: " << std::endl;
		std::cout << argv[0] << " Path/ExampleWithoutEnding" << std::endl;
		return -1;
	}
	std::string base_name (argv[1]);
	std::string path(base_name.substr(0, base_name.rfind("/") + 1));
	std::string fname (base_name + ".gli");

	// read gli file
	std::ifstream in_gli (fname.c_str(), std::ios::in);
	if (!in_gli) {
		std::cerr << "could not open gli-file " << fname << std::endl;
		return -1;
	}
	std::vector <GEOLIB::Point*> pnt_vec;
	std::vector <std::string> pnt_names;
	std::vector<GEOLIB::Polyline*> ply_vec;
	std::vector <std::string> ply_names;
	std::vector<GEOLIB::Surface*> sfc_vec;
	std::vector <std::string> sfc_names;

	readGLIFileWithNames (in_gli, pnt_vec, pnt_names, ply_vec, ply_names, sfc_vec, sfc_names, path);
	in_gli.close();

	// read boundary conditions
	fname = base_name + ".bc";
	std::ifstream in_bc_stream (fname.c_str(), std::ios::in);
	if (!in_bc_stream) {
		std::cerr << "could not open source term file " << fname << std::endl;
		return -1;
	}
	std::vector<CBoundaryCondition*> bc_vec;
	std::cout << "read boundary conditions ... " << std::flush;
	BCRead(in_bc_stream, bc_vec);
	std::cout << "ok, " << bc_vec.size() << " boundary conditions read" << std::endl;

	// write boundary conditions file
	fname = base_name + ".new.bc";
	std::ofstream out_bc_stream(fname.c_str(), std::ios::out);
	if (!out_bc_stream) {
		std::cerr << "could not open source term file " << fname
				<< std::endl;
		return -1;
	}
	std::cout << "write boundary conditions to " << fname << " ... " << std::flush;
	out_bc_stream << std::scientific << std::setprecision(12);
	BCWrite(out_bc_stream, pnt_vec, pnt_names, ply_vec, ply_names, sfc_vec,
			sfc_names, bc_vec);
	std::cout << "ok, " << bc_vec.size() << " boundary conditions written"
			<< std::endl;
	out_bc_stream.close();

	for (size_t k(0); k<bc_vec.size(); k++)
		delete bc_vec[k];
	for (size_t k(0); k<pnt_vec.size(); k++)
		delete pnt_vec[k];
}

/*
 * ConvertSourceTermFileWithNameToSourceTermFileWithoutName.cpp
 *
 *  Created on: May 10, 2010
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
#include "tools.h"
#include "problem.h"
Problem *aproblem = NULL;
//
#include "readGLIFileWithNames.h"

/**************************************************************************
FEMLib-Method:
Task: for CSourceTerm::Read
Programing:
11/2007 JOD Implementation
**************************************************************************/
ios::pos_type ReadGeoType(std::ifstream& st_file, CSourceTerm *st)
{
	ios::pos_type position;
	string sub_string;
	std::stringstream in;

	in.str(GetLineFromFile1(&st_file));
	in >> st->geo_type_name;

	if (st->geo_type_name.find("POINT") != std::string::npos) {
		st->geo_type = 0;
		st->geo_type_name = "POINT";
		std::string t;
		in >> t;
		st->setGeoName (t);
		in.clear();
	}
	if (st->geo_type_name.compare("LINE") == 0) {
		st->geo_type = 1;
		in >> st->geo_id;
		st->geo_type_name = "LINE";
		in.clear();
	}
	if (st->geo_type_name.find("POLYLINE") != string::npos) {
		st->geo_type = 1;
		st->geo_type_name = "POLYLINE";
		std::string t;
		in >> t;
		st->setGeoName (t);
		in.clear();
	}
	if (st->geo_type_name.find("SURFACE") != string::npos) {
		st->geo_type = 2;
		st->geo_type_name = "SURFACE";
		std::string t;
		in >> t;
		st->setGeoName (t);
		in.clear();
	}
	if (st->geo_type_name.find("VOLUME") != string::npos) {
		st->geo_type = 3;
		st->geo_type_name = "VOLUME";
		std::string t;
		in >> t;
		st->setGeoName (t);
		in.clear();
	}
	if (st->geo_type_name.find("DOMAIN") != string::npos) {//CMCD for analytical function
		st->geo_type = 3;
		st->geo_type_name = "DOMAIN";
		in.clear();
	}
	if (st->geo_type_name.find("COLUMN") != string::npos) {
		st->geo_type = 0;
		st->geo_type_name = "COLUMN";
		std::string t;
		in >> t;
		st->setGeoName (t);
		in.clear();
	}
	if (st->pcs_pv_name.find("EXCAVATION") != string::npos) //WW
	{
		in.str(GetLineFromFile1(&st_file));
		in >> sub_string >> st->geo_type;
		in.clear();
	}
	return position;
}


ios::pos_type ReadSingleST(std::ifstream &st_file, CSourceTerm* st)
{
	char line[MAX_ZEILE];
	std::string line_string, sub_string;
	bool new_keyword = false;
	std::stringstream in;

	st->channel = st->node_averaging = st->no_surface_water_pressure = 0;
	ios::pos_type position;

	// loop over all components
	while (!new_keyword) {
		position = st_file.tellg();
		if (!GetLineFromFile(line, &st_file))
			break;
		line_string = line;
		if (line_string.find("#") != string::npos) {
			new_keyword = true;
			break;
		}
		remove_white_space(&line_string); //OK
		/* Keywords nacheinander durchsuchen */
		//....................................................................
		if (line_string.find("$PCS_TYPE") != string::npos) { // subkeyword found
			in.str(GetLineFromFile1(&st_file));
			in >> st->pcs_type_name;
			in.clear();
			continue;
		}
		//....................................................................
		if (line_string.find("$PRIMARY_VARIABLE") != string::npos) { // subkeyword found
			in.str(GetLineFromFile1(&st_file));
			in >> st->pcs_pv_name;
			in.clear();
			continue;
		}
		//....................................................................
		if (line_string.find("$GEO_TYPE") != string::npos) {
			position = ReadGeoType(st_file, st);
			continue;
		}
		//....................................................................
		if (line_string.find("$DIS_TYPE") != string::npos) //05.09.2008 WW
		{
			//10.04.2008. WW  if(line_string.compare("$DIS_TYPE")==0) {
			if (line_string.find("CONDITION") != string::npos) //05.09.2008 WW
			{
				st->conditional = true;
				position = st->ReadDistributionType(&st_file);
				in.str(GetLineFromFile1(&st_file)); //PCS type
				in >> line_string >> st->pcs_type_name_cond;
				in.clear();
				in.str(GetLineFromFile1(&st_file)); //
				in >> st->pcs_pv_name_cond;
				in.clear();
				in.str(GetLineFromFile1(&st_file));
				in >> st->coup_leakance >> st->rill_height;
				in.clear();
			} else //05.09.2008 WW
			{
				position = st->ReadDistributionType(&st_file);
				continue;
			}
		}
		//..............................................
		//..............................................
		if (line_string.find("$COUPLING_SWITCH") != string::npos) {
			st->COUPLING_SWITCH = true; // switch case
			in.str(GetLineFromFile1(&st_file));
			in >> st->rainfall >> st->rainfall_duration;
			in.clear();
		}
		//..............................................
		if (line_string.find("$NODE_AVERAGING") != string::npos) {
			in.clear();
			st->node_averaging = 1;
			continue;
		}
		//..............................................
		if (line_string.find("$NEGLECT_SURFACE_WATER_PRESSURE") != string::npos) { // JOD 4.10.01
			in.clear();
			st->no_surface_water_pressure = 1;
			continue;
		}
		//..............................................
		if (line_string.find("$CHANNEL") != string::npos) {
			in.clear();
			in.str(GetLineFromFile1(&st_file));
			in >> st->channel_width;
			st->channel = 1;
			continue;
		}
		//..............................................
		if (line_string.find("$TIM_TYPE") != string::npos) {
			in.str(GetLineFromFile1(&st_file));
			in >> st->tim_type_name;
			if (st->tim_type_name.find("CURVE") != string::npos) {
				st->dis_type = 0;
				in >> st->CurveIndex;
			}
			in.clear();
			continue;
		}
		//..............................................
		if (line_string.find("$FCT_TYPE") != string::npos) {
			in.str(GetLineFromFile1(&st_file));
			in >> st->fct_name; //sub_line
			if (st->fct_name.find("METHOD") != string::npos) //WW
				in >> st->fct_method;
			in.clear();
		}
		//....................................................................
		if (line_string.find("$MSH_TYPE") != string::npos) {
			in.str(GetLineFromFile1(&st_file));
			in >> sub_string; //sub_line
			st->msh_type_name = "NODE";
			if (sub_string.find("NODE") != string::npos) {
				in >> st->msh_node_number;
				in.clear();
			}
			continue;
		}
		//....................................................................
	} // end !new_keyword
	return position;
}


/**************************************************************************
FEMLib-Method:
Task: ST read function
Programing:
01/2004 OK Implementation
**************************************************************************/
bool STRead(std::ifstream& in_st_stream, std::vector<CSourceTerm*>& st_vector)
{
	char line[MAX_ZEILE];
	std::string line_string;
	std::ios::pos_type position;

	// Rewind the file
	in_st_stream.clear();
	in_st_stream.seekg(0L, std::ios::beg);

	// Keyword loop
	while (!in_st_stream.eof()) {
		in_st_stream.getline(line, MAX_ZEILE);
		line_string = line;
		if (line_string.find("#STOP") != string::npos) {
			size_t no_source_terms (st_vector.size());
			int no_an_sol = 0;
			long number_of_terms = 0;
			for (size_t i=0; i<no_source_terms; i++) {
				if (st_vector[i]->analytical) {
					no_an_sol++;
					number_of_terms = max(st_vector[i]->number_of_terms,
							number_of_terms);
				}
			}
			if (no_an_sol > 0) {
				for (size_t i = 0; i < no_source_terms; i++) {
					st_vector[i]->no_an_sol = no_an_sol;
					st_vector[i]->max_no_terms = number_of_terms;
				}
			}
			return true;
		}

		if (line_string.find("#SOURCE_TERM") != string::npos) { // keyword found
			CSourceTerm *st (new CSourceTerm());
			position = ReadSingleST(in_st_stream, st);
			st_vector.push_back(st);
			in_st_stream.seekg(position, ios::beg);
		} // keyword found
	} // eof
	return true;
}

void STWrite(std::ofstream& out_st_stream, const std::vector<GEOLIB::Point*>& pnt_vec,
		const std::vector<std::string>& pnt_names,
		const std::vector<GEOLIB::Polyline*>& ply_vec,
		const std::vector<std::string>& ply_names,
		const std::vector<GEOLIB::Surface*>& sfc_vec,
		const std::vector<std::string>& sfc_names,
		const std::vector<CSourceTerm*>& st_vec)
{
	out_st_stream << "GeoSys-ST: Source Terms ------------------------------------------------" << std::endl;;
	for (size_t k(0); k<st_vec.size(); k++) {
		//KEYWORD
		out_st_stream << "#SOURCE_TERM" << std::endl;

		//NAME+NUMBER
		out_st_stream << " $PCS_TYPE" << std::endl;
		out_st_stream << "  ";
		out_st_stream << st_vec[k]->pcs_type_name << std::endl;
		out_st_stream << " $PRIMARY_VARIABLE" << std::endl;
		out_st_stream << "  ";
		out_st_stream << st_vec[k]->pcs_pv_name << std::endl;

		//GEO_TYPE
		out_st_stream << " $GEO_TYPE" << std::endl;
		out_st_stream << "  ";
		if (st_vec[k]->geo_type_name.compare("LINE") == 0) {
			out_st_stream << st_vec[k]->geo_type_name << " " << st_vec[k]->geo_id << std::endl;
		} else {
			if (st_vec[k]->geo_type_name.compare("POINT") == 0) {
				// search in vector pnt_names
				bool nfound (true);
				for (size_t j(0); j<pnt_vec.size() && nfound; j++) {

					if (st_vec[k]->getGeoName().compare (pnt_names[j]) == 0) {
						nfound = false;
						out_st_stream << st_vec[k]->geo_type_name << " " << j << std::endl;
					}

					if (st_vec[k]->getGeoName().compare ("POINT"+pnt_names[j]) == 0) {
						nfound = false;
						out_st_stream << st_vec[k]->geo_type_name << " " << j << std::endl;
					}
				}
			} else {
				if (st_vec[k]->geo_type_name.compare("POLYLINE") == 0) {
					// search in vector ply_names
					bool nfound (true);
					for (size_t j(0); j<ply_vec.size() && nfound; j++) {
						if (st_vec[k]->getGeoName().compare (ply_names[j]) == 0) {
							nfound = false;
							out_st_stream << st_vec[k]->geo_type_name << " " << j << std::endl;
						}
					}
				} else {
					if (st_vec[k]->geo_type_name.compare("SURFACE") == 0) {
						// search in vector sfc_names
						bool nfound (true);
						for (size_t j(0); j<sfc_vec.size() && nfound; j++) {
							if (st_vec[k]->getGeoName().compare (sfc_names[j]) == 0) {
								nfound = false;
								out_st_stream << st_vec[k]->geo_type_name << " " << j << std::endl;
							}
						}
					} else
						out_st_stream << st_vec[k]->geo_type_name << " " << st_vec[k]->getGeoName() << std::endl;
				}
			}
		}

		// TIM_TYPE
		if (st_vec[k]->tim_type_name.size() > 0) { //OK
			out_st_stream << " $TIM_TYPE" << std::endl;
			out_st_stream << "  ";
			out_st_stream << st_vec[k]->tim_type_name << std::endl;
		}

		//DIS_TYPE
		out_st_stream << " $DIS_TYPE" << std::endl;
		out_st_stream << "  ";
		out_st_stream << st_vec[k]->dis_type_name;
		switch (st_vec[k]->dis_type_name[0]) {
		case 'C': // Constant
			out_st_stream << " " << st_vec[k]->geo_node_value;
			out_st_stream << std::endl;
			break;
		case 'L': // Linear
			out_st_stream << " " << (int) st_vec[k]->PointsHaveDistribedBC.size() << std::endl;
			for (long i = 0; i < (long) st_vec[k]->PointsHaveDistribedBC.size(); i++) {
				out_st_stream << "  " << st_vec[k]->PointsHaveDistribedBC[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistribedBC[i] <<std::endl;
			}
			break;
		case 'R': // RIVER
			out_st_stream << " " << (int) st_vec[k]->PointsHaveDistribedBC.size() << std::endl;
			for (long i = 0; i < (long) st_vec[k]->PointsHaveDistribedBC.size(); i++) {
				out_st_stream << "  " << st_vec[k]->PointsHaveDistribedBC[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistribedBC[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistBC_KRiverBed[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistBC_WRiverBed[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistBC_TRiverBed[i] << " ";
				out_st_stream << "  " << st_vec[k]->DistBC_BRiverBed[i] << std::endl;
			}
			break;
		}
	}
	out_st_stream << "#STOP";
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

	// read points from gli file
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

	// read source terms
	fname = base_name + ".st";
	std::ifstream in_st_stream (fname.c_str(), std::ios::in);
	if (!in_st_stream) {
		std::cerr << "could not open source term file " << fname << std::endl;
		return -1;
	}
	std::vector<CSourceTerm*> st_vec;
	std::cout << "read source terms ... " << std::flush;
	STRead(in_st_stream, st_vec);
	std::cout << "ok, " << st_vec.size() << " source terms read" << std::endl;

	// write source term file
	fname = base_name + ".new.st";
	std::ofstream out_st_stream (fname.c_str(), std::ios::out);
	if (!out_st_stream) {
		std::cerr << "could not open source term file " << argv[3] << std::endl;
		return -1;
	}
	std::cout << "write source terms ... " << std::flush;
	out_st_stream << std::scientific << std::setprecision (12);
	STWrite(out_st_stream, pnt_vec, pnt_names, ply_vec, ply_names, sfc_vec, sfc_names, st_vec);
	std::cout << "ok, " << st_vec.size() << " source terms written" << std::endl;
	out_st_stream.close();

	for (size_t k(0); k<st_vec.size(); k++)
		delete st_vec[k];
	for (size_t k(0); k<pnt_vec.size(); k++)
		delete pnt_vec[k];
}



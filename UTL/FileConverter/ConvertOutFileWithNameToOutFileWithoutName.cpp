/*
 * ConvertOutFileWithNameToOutFileWithoutName.cpp
 *
 *  Created on: May 20, 2010
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
#include "rf_out_new.h"
#include "files0.h"

#include "problem.h"
Problem *aproblem = NULL;

//
#include "readGLIFileWithNames.h"

ios::pos_type ReadCOutput(std::ifstream &out_file, COutput* out_obj)
{
	std::string line_string;
	std::string delimiter(" ");
	bool new_keyword = false;
	std::string hash("#");
	ios::pos_type position;
	bool new_subkeyword = false;
	std::string dollar("$");
	ios::pos_type position_line;
	bool ok = true;
	std::stringstream line;
	std::stringstream in;
	std::string name;
	ios::pos_type position_subkeyword;

	// Schleife ueber alle Phasen bzw. Komponenten
	while (!new_keyword) {
		position = out_file.tellg();
		if (new_subkeyword)
			out_file.seekg(position_subkeyword, ios::beg);
		new_subkeyword = false;
		line_string.clear();
		line_string = GetLineFromFile1(&out_file);
		if (line_string.size() < 1)
			break;

		if (Keyword(line_string))
			return position;

		if (line_string.find("$NOD_VALUES") != string::npos) { // subkeyword found
			while ((!new_keyword) && (!new_subkeyword)) {
				position_subkeyword = out_file.tellg();
				//SB input with comments  out_file >> line_string>>ws;
				line_string = GetLineFromFile1(&out_file);
				if (line_string.find(hash) != string::npos) {
					return position;
				}
				if (line_string.find(dollar) != string::npos) {
					new_subkeyword = true;
					break;
				}
				if (line_string.size() == 0)
					break; //SB: empty line
				in.str(line_string);
				in >> name;
				out_obj->nod_value_vector.push_back(name);
				in.clear();
			}
			continue;
		}

		if (line_string.find("$PCON_VALUES") != string::npos) { // subkeyword found //MX
			while ((!new_keyword) && (!new_subkeyword)) {
				position_subkeyword = out_file.tellg();
				out_file >> line_string >> ws;
				if (line_string.find(hash) != string::npos) {
					return position;
				}
				if (line_string.find(dollar) != string::npos) {
					new_subkeyword = true;
					break;
				}
				if (line_string.size() == 0)
					break;
				out_obj->pcon_value_vector.push_back(line_string);
			}
			continue;
		}


		if (line_string.find("$ELE_VALUES") != string::npos) { // subkeyword found
			ok = true;
			while (ok) {
				position_line = out_file.tellg();
				out_file >> line_string;
				if (SubKeyword(line_string)) {
					out_file.seekg(position_line, ios::beg);
					ok = false;
					continue;
				}
				if (Keyword(line_string))
					return position;
				out_obj->ele_value_vector.push_back(line_string);
				out_file.ignore(MAX_ZEILE, '\n');
			}

			continue;
		}
		//-------------------------------------------------------------------- // Added 03.2010 JTARON
		if (line_string.find("$RWPT_VALUES") != string::npos) { // subkeyword found
			while ((!new_keyword) && (!new_subkeyword)) {
				position_subkeyword = out_file.tellg();
				line_string = GetLineFromFile1(&out_file);
				if (line_string.find(hash) != string::npos) {
					return position;
				}
				if (line_string.find(dollar) != string::npos) {
					new_subkeyword = true;
					break;
				}
				if (line_string.size() == 0)
					break; //SB: empty line
				in.str(line_string);
				in >> name;
				out_obj->rwpt_value_vector.push_back(name);
				in.clear();
			}
			continue;
		}

		if (line_string.find("$GEO_TYPE") != string::npos) { //subkeyword found
			line.str(GetLineFromFile1(&out_file));
			line >> out_obj->geo_type_name;
			if (out_obj->geo_type_name.find("POINT") != string::npos) {
				out_obj->geo_type = 0;
			}
			if (out_obj->geo_type_name.find("POLYLINE") != string::npos) {
				out_obj->geo_type = 1;
			}
			if (out_obj->geo_type_name.find("SURFACE") != string::npos) {
				out_obj->geo_type = 2;
			}
			if (out_obj->geo_type_name.find("VOLUME") != string::npos) {
				out_obj->geo_type = 3;
			}
			if (out_obj->geo_type_name.find("DOMAIN") != string::npos) {
				out_obj->geo_type = 4;
			}
			if (out_obj->geo_type < 4) {
				std::string tname;
				line >> tname;
				out_obj->setGeoName (tname);
			}

			line.clear();
			continue;
		}

		if (line_string.find("$TIM_TYPE") != string::npos) { // subkeyword found
			while ((!new_keyword) && (!new_subkeyword)) {
				position_subkeyword = out_file.tellg();
				out_file >> line_string;
				if (line_string.size() == 0) //SB
					break;
				if (line_string.find(hash) != string::npos) {
					new_keyword = true;
					break;
				}
				if (line_string.find(dollar) != string::npos) {
					new_subkeyword = true;
					break;
				}
				if (line_string.find("STEPS") != string::npos) {
					out_file >> out_obj->nSteps;
					out_obj->tim_type_name = "STEPS"; //OK
				}
				if (line_string.find("STEPPING") != string::npos) { // JTARON 2010, reconfigured... didn't work
					double stepping_length, stepping_end, stepping_current;
					out_file >> stepping_length >> stepping_end;
					stepping_current = stepping_length;
					while (stepping_current <= stepping_end) {
						out_obj->time_vector.push_back(stepping_current);
						out_obj->rwpt_time_vector.push_back(stepping_current);
						stepping_current += stepping_length;
					}
				} else {
					out_obj->time_vector.push_back(strtod(line_string.data(), NULL));
					out_obj->rwpt_time_vector.push_back(strtod(line_string.data(), NULL));
				}
				out_file.ignore(MAX_ZEILE, '\n');
			}
			continue;
		}

		if (line_string.find("$DAT_TYPE") != string::npos) { // subkeyword found
			out_file >> out_obj->dat_type_name;
			out_file.ignore(MAX_ZEILE, '\n');
			continue;
		}

		if (line_string.find("$AMPLIFIER") != string::npos) { // subkeyword found
			double t (0.0);
			out_file >> t;
			out_obj->setAmplifier (t);
			out_file.ignore(MAX_ZEILE, '\n');
			continue;
		}

		if (line_string.find("$PCS_TYPE") != string::npos) { // subkeyword found
			out_file >> out_obj->pcs_type_name;
			out_file.ignore(MAX_ZEILE, '\n');
			continue;
		}

		if (line_string.find("$DIS_TYPE") != string::npos) { // subkeyword found
			out_file >> out_obj->dis_type_name;
			out_file.ignore(MAX_ZEILE, '\n');
			continue;
		}

		if (line_string.find("$MSH_TYPE") != string::npos) { // subkeyword found
			out_file >> out_obj->msh_type_name;
			out_file.ignore(MAX_ZEILE, '\n');
			continue;
		}

		if (line_string.find("$MMP_VALUES") != string::npos) { //OK
			ok = true;
			while (ok) {
				position_line = out_file.tellg();
				out_file >> line_string;
				if (SubKeyword(line_string)) {
					out_file.seekg(position_line, ios::beg);
					ok = false;
					continue;
				}
				if (Keyword(line_string))
					return position;
				out_obj->mmp_value_vector.push_back(line_string);
				out_file.ignore(MAX_ZEILE, '\n');
			}
			continue;
		}

		if (line_string.find("$MFP_VALUES") != string::npos) { //OK
			ok = true;
			while (ok) {
				position_line = out_file.tellg();
				out_file >> line_string;
				if (SubKeyword(line_string)) {
					out_file.seekg(position_line, ios::beg);
					ok = false;
					continue;
				}
				if (Keyword(line_string))
					return position;
				out_obj->mfp_value_vector.push_back(line_string);
				out_file.ignore(MAX_ZEILE, '\n');
			}

			continue;
		}

	}
	return position;
}


/**************************************************************************
FEMLib-Method:
Task: OUT read function
Programing:
06/2004 OK Implementation
08/2004 WW Remove the old files
01/2005 OK Boolean type
01/2005 OK Destruct before read
06/2006 WW Remove the old files by new way
**************************************************************************/
bool OUTRead(std::ifstream& in_stream, std::vector<COutput*>& out_vec, std::string& file_base_name)
{
	char line[MAX_ZEILE];
	string line_string;
	ios::pos_type position;

	// Give version in file name
	if (line_string.find("#VERSION") != string::npos) { //15.01.2008. WW
		file_base_name.append("(V");
		file_base_name.append(OGS_VERSION);
		file_base_name.append(")");
	}

	// Keyword loop
	std::cout << "OUTRead" << std::endl;
	while (!in_stream.eof()) {
		in_stream.getline(line, MAX_ZEILE);
		line_string = line;
		if (line_string.find("#STOP") != string::npos)
			return true;

		if (line_string.find("#OUTPUT") != string::npos) { // keyword found
			COutput *out = new COutput();
			out->file_base_name = file_base_name;
			position = ReadCOutput (in_stream, out);
			out_vec.push_back(out);
			out->ID =  number2str (out_vector.size()-1);
			in_stream.seekg(position, ios::beg);
		} // keyword found
	} // eof
	return true;
}

/**************************************************************************
FEMLib-Method: OUTWrite
Task: master write function
Programing:
06/2004 OK Implementation
last modification:
**************************************************************************/
void OUTWrite(std::ofstream& out_file,
		const std::vector <GEOLIB::Point*> pnt_vec, const std::vector <std::string> pnt_names,
		const std::vector<GEOLIB::Polyline*> ply_vec, const std::vector <std::string> ply_names,
		const std::vector<GEOLIB::Surface*> sfc_vec, const std::vector <std::string> sfc_names,
		const std::vector<COutput*>& out_vec)
{
	out_file.setf(ios::scientific, ios::floatfield);
	out_file.precision(12);

	out_file
			<< "GeoSys-OUT: Output ------------------------------------------------\n";

	size_t n (out_vec.size());
	for (size_t k = 0; k<n; k++) {
		// KEYWORD
		out_file << "#OUTPUT" << endl;
		// PCS_TYPE
		out_file << " $PCS_TYPE" << endl;
		out_file << "  ";
		out_file << out_vec[k]->pcs_type_name << std::endl;
		// NOD_VALUES
		out_file << " $NOD_VALUES" << std::endl;
		for (size_t i = 0; i < out_vec[k]->nod_value_vector.size(); i++) {
			out_file << "  ";
			out_file << out_vec[k]->nod_value_vector[i] << std::endl;
		}
		// ELE_VALUES
		out_file << " $ELE_VALUES" << std::endl;
		for (size_t i = 0; i < out_vec[k]->ele_value_vector.size(); i++) {
			out_file << "  ";
			out_file << out_vec[k]->ele_value_vector[i] << std::endl;
		}
		// GEO_TYPE
		out_file << " $GEO_TYPE" << endl;
		out_file << "  " << out_vec[k]->geo_type_name << " ";
		bool nfound (true);
		if (out_vec[k]->geo_type_name.compare("POINT") == 0) {
			// search in vector pnt_names
			for (size_t j(0); j<pnt_vec.size() && nfound; j++) {
				if (out_vec[k]->getGeoName().compare (pnt_names[j]) == 0) {
					nfound = false;
					std::cout << "searched " << out_vec[k]->getGeoName() << ", found " << pnt_names[j] << std::endl;
					out_file << j << std::endl;
				} else {
					if (out_vec[k]->getGeoName().compare ("POINT"+pnt_names[j]) == 0) {
						nfound = false;
						std::cout << "searched " << out_vec[k]->getGeoName() << ", found " << pnt_names[j] << std::endl;
						out_file << j << std::endl;
					}
				}
			}
		} else
			out_file << out_vec[k]->getGeoName() << std::endl;
		// TIM_TYPE
		out_file << " $TIM_TYPE" << std::endl;
		if (out_vec[k]->tim_type_name == "STEPS") {
			out_file << "  " << out_vec[k]->tim_type_name << " " << out_vec[k]->nSteps << endl;
		} else {
			for (size_t i = 0; i < out_vec[k]->time_vector.size(); i++) {
				out_file << "  ";
				out_file << out_vec[k]->time_vector[i] << endl;
			}
		}
		// DIS_TYPE
		if (out_vec[k]->dis_type_name.size() > 0) {
			out_file << " $DIS_TYPE" << endl;
			out_file << "  ";
			out_file << out_vec[k]->dis_type_name << endl;
		}
		// MSH_TYPE
		if (out_vec[k]->msh_type_name.size() > 0) {
			out_file << " $MSH_TYPE" << endl;
			out_file << "  ";
			out_file << out_vec[k]->msh_type_name << endl;
		}
		// DAT_TYPE
		out_file << " $DAT_TYPE" << endl;
		out_file << "  ";
		out_file << out_vec[k]->dat_type_name << endl;
	}
	out_file << "#STOP";
	out_file.close();
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

	// read outfile content
	fname = base_name + ".out";
	std::ifstream in_stream (fname.c_str(), std::ios::in);
	if (!in_stream) {
		std::cerr << "could not open out file " << fname << std::endl;
		return -1;
	}
	std::vector<COutput*> out_vec;
	std::cout << "read out entities ... " << std::flush;
	OUTRead(in_stream, out_vec, base_name);
	std::cout << "ok, " << out_vec.size() << " source terms read" << std::endl;

	// write file
	fname = base_name + ".new.out";
	std::ofstream out_stream (fname.c_str(), ios::trunc|ios::out);
	if (!out_stream) {
		std::cerr << "could not open output file " << fname << std::endl;
		return -1;
	}
	std::cout << "write output ... " << std::flush;
	out_stream << std::scientific << std::setprecision (12);
	OUTWrite(out_stream, pnt_vec, pnt_names, ply_vec, ply_names, sfc_vec, sfc_names, out_vec);
	std::cout << "ok, " << out_vec.size() << " output entities written" << std::endl;
	out_stream.close();

	for (size_t k(0); k<out_vec.size(); k++)
		delete out_vec[k];
	for (size_t k(0); k<pnt_vec.size(); k++)
		delete pnt_vec[k];
}

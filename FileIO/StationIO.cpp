/**
 * \file StationIO.cpp
 * 23/03/2010 KR Initial implementation
 *
 */

#include <fstream>
#include "StationIO.h"
#include "StringTools.h"

int StationIO::readStationFile(const std::string &path, std::string &name, std::vector<GEOLIB::Point*> *stations, GEOLIB::Station::StationType type )
{
	int returnValue = 1;
	std::string line;

	std::ifstream in( path.c_str() );

	if (!in.is_open())
    {
		std::cout << "Station::readFromFile() - Could not open file...\n";
		return 0;
	}

	/* try to find a name for the list in the first line of the file */
	getline(in, line);
	if ((line.substr(0,1)).compare("!")==0)
		name = line.substr( 1, line.length()-1 );
	else in.seekg(0);	// sets stream to the beginning of the file

	/* read all stations */
	while ( getline(in, line) )
	{
		GEOLIB::Station* newStation;
		if (type == GEOLIB::Station::STATION)
			newStation = GEOLIB::Station::createStation(line);
		if (type == GEOLIB::Station::BOREHOLE)
			newStation = GEOLIB::StationBorehole::createStation(line);

		if (newStation)
		{
			//newStation->setList(name);
			stations->push_back(newStation);
		}
		else
			returnValue = -1;
	}

	in.close();

    return returnValue;
}

/*
// all boreholes to GMS which each borehole in a single file
void StationIO::writeBoreholesToGMS(const std::vector<GEOLIB::Point*> *stations)
{
	//std::vector<std::string> soilID(1);
	std::vector<std::string> soilID = readSoilIDfromFile("d:/BodeTimeline.txt");
	for (size_t i=0; i<stations->size(); i++)
		StationIO::writeBoreholeToGMS(static_cast<GEOLIB::StationBorehole*>((*stations)[i]), std::string("Borehole-" + static_cast<GEOLIB::StationBorehole*>((*stations)[i])->getName() + ".txt"), soilID);
	StationIO::writeSoilIDTable(soilID, "SoilIDReference.txt");
}
*/
void StationIO::writeBoreholesToGMS(const std::vector<GEOLIB::Point*> *stations, const std::string &filename)
{
	std::ofstream out( filename.c_str(), std::ios::out );
	size_t idx = 0;
	std::vector<std::string> soilID = readSoilIDfromFile("d:/BodeTimeline.txt");

	// write header
	out	<< "name" << "\t" << std::fixed << "X" << "\t" << "Y"  << "\t" << "Z" <<  "\t" << "soilID" << std::endl;

	for (size_t j=0; j<stations->size(); j++)
	{
		GEOLIB::StationBorehole* station = static_cast<GEOLIB::StationBorehole*>((*stations)[j]);
		std::vector<GEOLIB::Point*> profile = station->getProfile();
		std::vector<std::string> soilNames  = station->getSoilNames();

		size_t nLayers = profile.size();
		for (size_t i=1; i<nLayers; i++) {

			if ( (i>1) && (soilNames[i].compare(soilNames[i-1]) == 0) ) continue;
			idx = getSoilID(soilID, soilNames[i]);

			out	<< station->getName() << "\t" << std::fixed << (*(profile[i-1]))[0] << "\t"
				<< (*(profile[i-1]))[1]  << "\t" << (*(profile[i-1]))[2] <<  "\t"
				<< idx << std::endl;
		}
	}

	out.close();
	StationIO::writeSoilIDTable(soilID, "d:/SoilIDReference.txt");
}


int StationIO::writeBoreholeToGMS(const GEOLIB::StationBorehole* station, const std::string &filename, std::vector<std::string> &soilID)
{
	std::ofstream out( filename.c_str(), std::ios::out );
	size_t idx = 0;

	// write header
	out	<< "name" << "\t" << std::fixed << "X" << "\t" << "Y"  << "\t" << "Z" <<  "\t" << "soilID" << std::endl;

	std::vector<GEOLIB::Point*> profile = station->getProfile();
	std::vector<std::string> soilNames  = station->getSoilNames();

	// write table
	size_t nLayers = profile.size();
	for (size_t i=1; i<nLayers; i++) {

		if ( (i>1) && (soilNames[i].compare(soilNames[i-1]) == 0) ) continue;
		idx = getSoilID(soilID, soilNames[i]);

		out	<< station->getName() << "\t" << std::fixed << (*(profile[i-1]))[0] << "\t"
			<< (*(profile[i-1]))[1]  << "\t" << (*(profile[i-1]))[2] <<  "\t"
			<< idx << std::endl;
	}
	out.close();

    return 1;
}

size_t StationIO::getSoilID(std::vector<std::string> &soilID, std::string &soilName)
{
	for (size_t j=0; j<soilID.size(); j++)
	{
		if (soilID[j].compare(soilName) == 0) return j;
	}
	soilID.push_back(soilName);
	return (soilID.size() - 1);
}

int StationIO::writeSoilIDTable(const std::vector<std::string> &soilID, const std::string &filename)
{
	std::ofstream out( filename.c_str(), std::ios::out );
	size_t idx = 0;

	// write header
	out	<< "ID" << "\t" << std::fixed << "Soil name"<< std::endl;

	// write table
	size_t nIDs = soilID.size();
	for (size_t i=0; i<nIDs; i++)
		out	<< i << "\t" << std::fixed << soilID[i] << "\t" << std::endl;
	out.close();

    return 1;
}


void StationIO::writeStratigraphyTable(const std::vector<GEOLIB::Point*> *boreholes, const std::string &filename)
{
	size_t maxIterations = 1;
	std::ofstream out( filename.c_str(), std::ios::out );

	for (size_t j=0; j<boreholes->size(); j++) {
		out << static_cast<GEOLIB::StationBorehole*>((*boreholes)[j])->getName() << "\t";
	}
	out << std::endl;

	for (size_t i=0; i<maxIterations; i++) {
		for (size_t j=0; j<boreholes->size(); j++) {
			std::vector<std::string> soilNames  = static_cast<GEOLIB::StationBorehole*>((*boreholes)[j])->getSoilNames();
			if (!soilNames.empty())
			{
				if (i==0 && maxIterations < soilNames.size()) maxIterations = soilNames.size();
				if (i<soilNames.size()) out << soilNames[i];
			}
			out << "\t";
		}
		out << std::endl;
	}

	out.close();	
}

std::vector<std::string> StationIO::readSoilIDfromFile(const std::string &filename)
{
	std::vector<std::string> soilID;
	std::string line;

	std::ifstream in( filename.c_str() );

	if (in.is_open())
	{
		while ( getline(in, line) )
		{
			trim(line);
			soilID.push_back(line);
		}
	}
	in.close();

	return soilID;
}
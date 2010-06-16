/**
 * \file GMSInterface.cpp
 * 08/06/2010 KR Initial implementation
 *
 */

#include <fstream>

// Base
#include "StringTools.h"

#include "GMSInterface.h"



int GMSInterface::readBoreholesFromGMS(std::vector<GEOLIB::Point*> *boreholes, const std::string &filename)
{
	double depth = 0;
	std::string line="", cName="", sName="";
	std::list<std::string>::const_iterator it;
	GEOLIB::Point* pnt = new GEOLIB::Point();
	GEOLIB::StationBorehole* newBorehole = NULL;

	std::ifstream in( filename.c_str() );

	if (!in.is_open())
    {
		std::cout << "GMSInterface::readBoreholeFromGMS() - Could not open file...\n";
		return 0;
	}

	/* skipping first line because it contains field names */
	getline(in, line);

	/* read all stations */
	while ( getline(in, line) )
	{
		std::list<std::string> fields = splitString(line, '\t');

		if (fields.size() >= 5)
		{
			if (fields.begin()->compare(cName) == 0) // add new layer
			{
				it = fields.begin();
				(*pnt)[0] = strtod((++it)->c_str(), 0);
				(*pnt)[1] = strtod((++it)->c_str(), 0);
				(*pnt)[2] = strtod((++it)->c_str(), 0);
				newBorehole->addSoilLayer((*pnt)[0], (*pnt)[1], (*pnt)[2], sName);
				sName = (*(++it));
				depth=(*pnt)[2];
			}
			else // add new borehole
			{
				if (newBorehole != NULL)
				{
					newBorehole->setDepth((*newBorehole)[2]-depth);
					boreholes->push_back(newBorehole);
				}
				cName = *fields.begin();
				it = fields.begin();
				(*pnt)[0] = strtod((++it)->c_str(), 0);
				(*pnt)[1] = strtod((++it)->c_str(), 0);
				(*pnt)[2] = strtod((++it)->c_str(), 0);
				sName = (*(++it));
				newBorehole = GEOLIB::StationBorehole::createStation(cName, (*pnt)[0], (*pnt)[1], (*pnt)[2], 0);
			}
		}
		else
			std::cout << "GMSInterface::readBoreholeFromGMS() - Error reading format..." << std::endl;
	}

	in.close();
	return 1;
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
void GMSInterface::writeBoreholesToGMS(const std::vector<GEOLIB::Point*> *stations, const std::string &filename)
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
		out	<< station->getName() << "\t" << std::fixed << (*(profile[nLayers-1]))[0] << "\t"
				<< (*(profile[nLayers-1]))[1]  << "\t" << (*(profile[nLayers-1]))[2] <<  "\t"
				<< idx << std::endl;	// this line marks the end of the borehole
	}

	out.close();
	GMSInterface::writeSoilIDTable(soilID, "d:/SoilIDReference.txt");
}


int GMSInterface::writeBoreholeToGMS(const GEOLIB::StationBorehole* station, const std::string &filename, std::vector<std::string> &soilID)
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
	out	<< station->getName() << "\t" << std::fixed << (*(profile[nLayers-1]))[0] << "\t"
				<< (*(profile[nLayers-1]))[1]  << "\t" << (*(profile[nLayers-1]))[2] <<  "\t"
				<< idx << std::endl;	// this line marks the end of the borehole
	out.close();

    return 1;
}


size_t GMSInterface::getSoilID(std::vector<std::string> &soilID, std::string &soilName)
{
	for (size_t j=0; j<soilID.size(); j++)
	{
		if (soilID[j].compare(soilName) == 0) return j;
	}
	soilID.push_back(soilName);
	return (soilID.size() - 1);
}


int GMSInterface::writeSoilIDTable(const std::vector<std::string> &soilID, const std::string &filename)
{
	std::ofstream out( filename.c_str(), std::ios::out );

	// write header
	out	<< "ID" << "\t" << std::fixed << "Soil name"<< std::endl;

	// write table
	size_t nIDs = soilID.size();
	for (size_t i=0; i<nIDs; i++)
		out	<< i << "\t" << std::fixed << soilID[i] << "\t" << std::endl;
	out.close();

    return 1;
}

std::vector<std::string> GMSInterface::readSoilIDfromFile(const std::string &filename)
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


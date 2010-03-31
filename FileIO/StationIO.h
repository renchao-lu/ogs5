/**
 * \file StationIO.h
 * 23/03/2010 KR Initial implementation
 *
 */

#ifndef STATIONIO_H
#define STATIONIO_H

#include "Station.h"


/**
 * \brief A number of methods for data input and output for station data.
 *
 * A number of methods for data input and output for station data.
 */
class StationIO {

public:
	/// Imports a file with station data.
	static int readStationFile(const std::string &path, std::string &name, std::vector<GEOLIB::Point*> *stations, GEOLIB::Station::StationType type);

	/// Exports all boreholes in a vector to GMS file format (this is a convenience-function with given filenames, etc.)
	static void writeBoreholesToGMS(const std::vector<GEOLIB::Point*> *stations);

	/// Exports borehole data to a file in GMS-format.
	static int writeBoreholeToGMS(const GEOLIB::StationBorehole* station, const std::string &filename, std::vector<std::string> &soilID);

	/// Writes a file that assigns each soilID-index in the GMS export file a name.
	static int writeSoilIDTable(const std::vector<std::string> &soilID, const std::string &filename);

	/// Writes a file that contains all stratigraphies for the boreholes in the given vector.
	static void writeStratigraphyTable(const std::vector<GEOLIB::Point*> *boreholes, const std::string &filename);

private:
	/// Finds the ID assigned to soilName or creates a new one ( this method is called from writeBoreholeToGMS() )
	static size_t getSoilID(std::vector<std::string> &soilID, std::string &soilName);

};

#endif // STATIONIO_H

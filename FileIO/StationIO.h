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

	/// Exports borehole data from all boreholes in a list to a file in GMS-format. (Note: there are some hardcoded tmp-files in the method that you might need to change!)
	static void writeBoreholesToGMS(const std::vector<GEOLIB::Point*> *stations, const std::string &filename);

	/// Exports borehole data from one borehole to a file in GMS-format.
	static int writeBoreholeToGMS(const GEOLIB::StationBorehole* station, const std::string &filename, std::vector<std::string> &soilID);

	/// Writes a file that assigns each soilID-index in the GMS export file a name.
	static int writeSoilIDTable(const std::vector<std::string> &soilID, const std::string &filename);

	/// Writes a file that contains all stratigraphies for the boreholes in the given vector.
	static void writeStratigraphyTable(const std::vector<GEOLIB::Point*> *boreholes, const std::string &filename);

private:
	/// Finds the ID assigned to soilName or creates a new one ( this method is called from writeBoreholeToGMS() )
	static size_t getSoilID(std::vector<std::string> &soilID, std::string &soilName);

	/**
	 * \brief Reads SoilIDs for Borehole export from an external file
	 *
	 * The method expects a file with the name of one stratigraphic layer at each line. These layers are assigned 
	 * ascending IDs, i.e. the first name gets index 0, the second line gets index 1, etc.
	 * \return An array with the names of the stratigraphic layers in which the index for each string equals its ID.
	 */
	static std::vector<std::string> readSoilIDfromFile(const std::string &filename);

};

#endif // STATIONIO_H

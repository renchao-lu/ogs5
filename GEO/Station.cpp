/**
 * \file Station.cpp
 * KR Initial implementation
 */

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <cmath>
#include "StringTools.h"
#include "Station.h"

using namespace std;

namespace GEOLIB {

Station::Station(double x, double y, double z) :
	Point (x,y,z), _type(Station::STATION), _color(new Color(0,128,0))
{
	addProperty("x", &getX, &Station::setX);
	addProperty("y", &getY, &Station::setY);
	addProperty("z", &getZ, &Station::setZ);
}

Station::Station(Color* const color) : _type(Station::STATION), _color(color)
{
	addProperty("x", &getX, &Station::setX);
	addProperty("y", &getY, &Station::setY);
	addProperty("z", &getZ, &Station::setZ);
}

void Station::addProperty(std::string pname, double (*getFct)(void*), void (*set)(void*, double))
{
	STNProperty p;
	p.name = pname;
	p.get = getFct;
	p.set = set;
	_properties.push_back(p);
}


Station::~Station()
{
}

void Station::setColor(unsigned char r, unsigned char g, unsigned char b)
{
	(*_color)[0]=r;
	(*_color)[1]=g;
	(*_color)[2]=b;
}

void Station::setColor(const Color* const color)
{
	(*_color)[0]=(*color)[0];
	(*_color)[1]=(*color)[1];
	(*_color)[2]=(*color)[2];
}

Station* Station::createStation(const std::string & line)
{
	Station* station = new Station();
	list<string> fields = splitString(line, '\t');

	if (fields.size() >= 3) {
		station->_name  = fields.front();
		fields.pop_front();
		(*station)[0]     = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		(*station)[1]     = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		if (fields.size() > 0)
		{
			(*station)[2] = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
			fields.pop_front();
		}
	}
	else
	{
		cout << "Station::createStation() - Unexpected file format...\n";
		return NULL;
	}
	return station;

}

Station* Station::createStation(const string &name, double x, double y, double z)
{
	Station* station = new Station();
	station->_name = name;
	(*station)[0] = x;
	(*station)[1] = y;
	(*station)[2] = z;
	return station;
}

const std::map<std::string, double> Station::getProperties()
{
	std::map<std::string, double> propertyMap;

	for (int i=0; i<static_cast<int>(_properties.size()); i++)
	{
		double (*getFct)(void*) = _properties[i].get;
		//setFct set = _properties[i].set;
		propertyMap[_properties[i].name] = (*getFct)((void*)this);
	}

	return propertyMap;
}

bool Station::inSelection(const std::vector<PropertyBounds> &bounds)
{
	double value;
	for (size_t i=0; i<bounds.size(); i++)
	{
		for (size_t j=0; j<_properties.size(); j++)
		{
			if (_properties[j].name.compare(bounds[i].getName())==0)
			{
				double (*get)(void*) = _properties[j].get;
				value = (*get)((void*)this);
				if (!(value >= bounds[i].getMin() && value <= bounds[i].getMax())) return false;
			}
		}
	}
	return true;
}




// The Borehole class

#include "DateTools.h"

StationBorehole::StationBorehole(double x, double y, double z) :
	Station (x,y,z)
{
	_type = Station::BOREHOLE;
	addProperty("date", &StationBorehole::getDate, &StationBorehole::setDate);
	addProperty("depth", &StationBorehole::getDepth, &StationBorehole::setDepth);

	// add first point of borehole
	_profilePntVec.push_back(this);
	_soilName.push_back("");
}

StationBorehole::~StationBorehole(void)
{
	// deletes profile vector of borehole, starting at layer 1
	// the first point is NOT deleted as it points to the station object itself
	for (size_t k(1); k<_profilePntVec.size(); k++) delete _profilePntVec[k];
}

int StationBorehole::find(const string &str)
{
	size_t size = _soilName.size();
	for (size_t i=0; i<size; i++)
	{
		if (_soilName[i].find(str)==0) return 1;
	}
	return 0;
}

int StationBorehole::readStratigraphyFile(const string &path, vector<list<string> > &data)
{
    string line;
	ifstream in( path.c_str() );

	if (!in.is_open())
    {
		cout << "StationBorehole::readStratigraphyFile() - Could not open file...\n";
		return 0;
	}

	while ( getline(in, line) )
	{
		list<string> fields = splitString(line, '\t');
		data.push_back(fields);
	}

	in.close();

	return 1;
}

int StationBorehole::addStratigraphy(const string &path, StationBorehole* borehole)
{
	vector<list<string> > data;
	if (readStratigraphyFile(path, data))
	{
		size_t size = data.size();
		for (size_t i=0; i<size; i++)
			addLayer(data[i], borehole);

		// check if a layer is missing
		size = borehole->_soilName.size();
		std::cout << "StationBorehole::addStratigraphy ToDo" << std::endl;
	//	for (size_t i=0; i<size; i++)
	//	{
	//		if ((borehole->_soilLayerThickness[i] == -1) ||(borehole->_soilName[i].compare("") == 0))
	//		{
	//			borehole->_soilLayerThickness.clear();
	//			borehole->_soilName.clear();
	//
	//			cout << "StationBorehole::addStratigraphy() - Profile incomplete (Borehole " << borehole->_name << ", Layer " << (i+1) << " missing).\n";
	//
	//			return 0;
	//		}
	//	}
	}
	else
	{
		borehole->addSoilLayer(borehole->getDepth(), "depth");
	}

	return 1;
}

int StationBorehole::addLayer(list<string> fields, StationBorehole* borehole)
{
	if (fields.size() >= 4) /* check if there are enough fields to create a borehole object */
	{
		if (fields.front().compare(borehole->_name) == 0) /* check if the name of the borehole matches the name in the data */
		{
			fields.pop_front();

			int layer = atoi(fields.front().c_str());
			fields.pop_front();

			std::cerr << "StationBorehole::addLayer - assuming correct order"
					<< std::endl;
			double thickness(strtod(
					replaceString(",", ".", fields.front()).c_str(), 0));
			fields.pop_front();
			borehole->addSoilLayer(thickness, fields.front());
			/* layers are not always in the correct order */
/*			if (layer < static_cast<int> (borehole->_soilName.size() + 1)) // if the current stratigraphy-array is larger than the layer-ID
			{
				borehole->_soilLayerThickness[layer - 1] = strtod(
						replaceString(",", ".", fields.front()).c_str(), 0);
				fields.pop_front();
				borehole->_soilName[layer - 1] = fields.front();
				fields.pop_front();
			} else {
				if (layer > static_cast<int> (borehole->_soilName.size() + 1)) // if the current stratigraphy-array is smaller than the layer-ID
				{
					while (static_cast<int> (borehole->_soilName.size())
							< layer - 1) // expand the array until it has the right size
					{
						borehole->_soilLayerThickness.push_back(-1);
						borehole->_soilName.push_back("");
					}
				}

				borehole->_soilLayerThickness.push_back(strtod(replaceString(
						",", ".", fields.front()).c_str(), 0));
				fields.pop_front();
				borehole->_soilName.push_back(fields.front());
				fields.pop_front();
			}
			*/

		}
	} else {
		cout
				<< "StationBorehole::addLayer() - Unexpected file format (Borehole "
				<< borehole->_name << ")...\n";
		return 0;
	}
	return 1;
}

int StationBorehole::addStratigraphies(const string &path, vector<Point*> *boreholes)
{
	vector<list<string> > data;

	if (readStratigraphyFile(path, data))
	{
		std::string name;

		size_t it=0;
		size_t nBoreholes = data.size();
		for (size_t i=0; i<nBoreholes; i++) {
			list<string> fields = data[i];

			if (fields.size() >= 4) {
	//			StationBorehole* bh (static_cast<StationBorehole*>((*boreholes)[it]));
				name = static_cast<StationBorehole*>((*boreholes)[it])->_name;
				if ( fields.front().compare(name) != 0 ) {
						if (it < boreholes->size()-1) it++;
				}

				fields.pop_front();
				//the method just assumes that layers are read in correct order
				fields.pop_front();
				double thickness (strtod(replaceString(",", ".", fields.front()).c_str(), 0));
				fields.pop_front();
				std::string soil_name (fields.front());
				fields.pop_front();
				static_cast<StationBorehole*>((*boreholes)[it])->addSoilLayer(thickness, soil_name);
			} else
			{
				cout << "StationBorehole::addStratigraphies() - Unexpected file format...\n";
				//return 0;
			}
		}
	}
	else 
	{
		createSurrogateStratigraphies(boreholes);
	}

	return 1;
}


StationBorehole* StationBorehole::createStation(const string &line)
{
	StationBorehole* borehole = new StationBorehole();
	list<string> fields = splitString(line, '\t');

	if (fields.size()      >= 5) {
		borehole->_name     = fields.front();
		fields.pop_front();
		(*borehole)[0]      = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		(*borehole)[1]      = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		(*borehole)[2]      = strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		borehole->_depth	= strtod((replaceString(",", ".", fields.front())).c_str(), NULL);
		fields.pop_front();
		if (fields.empty())
			borehole->_date = 0;
		else
		{
			borehole->_date = strDate2Double(fields.front());
			fields.pop_front();
		}
	}
	else
	{
		cout << "Station::createStation() - Unexpected file format...\n";
		return NULL;
	}
	return borehole;
}

StationBorehole* StationBorehole::createStation(const string &name, double x, double y, double z, double depth, std::string date)
{
	StationBorehole* station = new StationBorehole();
	station->_name  = name;
	(*station)[0]   = x;
	(*station)[1]   = y;
	(*station)[2]   = z;
	station->_depth = depth;
	station->_date  = strDate2Double(date);
	return station;
}

int StationBorehole::writeAsGMS(const string &filename)
{
	ofstream out( filename.c_str(), ios::out );

	for (size_t i=0; i<_profilePntVec.size(); i++) {
		out	<< _name << "\t" << fixed << (*(_profilePntVec[i]))[0] << "\t"
			<< (*(_profilePntVec[i]))[1]  << "\t" << (*(_profilePntVec[i]))[2] <<  "\t"
			<< _soilName[i] << endl;
	}
	out.close();

    return 1;
}

void StationBorehole::createSurrogateStratigraphies(std::vector<Point*> *boreholes)
{
	size_t nBoreholes = boreholes->size();
	for (size_t i=0; i<nBoreholes; i++)
	{
		StationBorehole* bore = static_cast<StationBorehole*>((*boreholes)[i]);
		bore->addSoilLayer(bore->getDepth(), "depth");
	}
}

void StationBorehole::addSoilLayer ( double thickness, const std::string &soil_name)
{
	if (_profilePntVec.size () == 0)
		addSoilLayer ((*this)[0], (*this)[1], (*this)[2]-thickness, soil_name);
	else {
		size_t idx (_profilePntVec.size());
		// read coordinates from last above
		double x((*_profilePntVec[idx-1])[0]);
		double y((*_profilePntVec[idx-1])[1]);
		double z((*_profilePntVec[idx-1])[2]-thickness);
		addSoilLayer (x, y, z, soil_name);
	}
}

void StationBorehole::addSoilLayer ( double x, double y, double z, const std::string &soil_name)
{
	_profilePntVec.push_back (new Point (x, y, z));
	_soilName.push_back(soil_name);
}

} // namespace

/**
 * \file GEOModels.cpp
 * 9/2/2010 LB Initial implementation
 *
 * Implementation of GEOModels
 */

// ** INCLUDES **
#include "GEOModels.h"

#include "Model.h"
#include "PntsModel.h"
#include "StationTreeModel.h"
#include "LinesModel.h"

GEOModels::GEOModels( QObject* parent /*= 0*/ )
{
	_stationModel = new StationTreeModel();
}

GEOModels::~GEOModels()
{
	delete _stationModel;
}

void GEOModels::addPointVec( std::vector<GEOLIB::Point*> *points, std::string &name )
{
	GEOObjects::addPointVec(points, name);

	PntsModel* model = new PntsModel(QString::fromStdString(name), points, this);
	_pntModels.push_back(model);
	emit pointModelAdded(model);
}

bool GEOModels::appendPointVec(const std::vector<GEOLIB::Point*> &points, std::string &name)
{
	bool ret (GEOLIB::GEOObjects::appendPointVec (points, name));

	// search model
	QString qname (name.c_str());
	bool nfound (true);
	std::vector<PntsModel*>::iterator it(_pntModels.begin());
	while (nfound && it != _pntModels.end()) {
		if (((*it)->name()).contains (qname)) nfound = false;
	}
	if (nfound) std::cerr << "Model not found" << std::endl;
	else (*it)->updateData ();

	// ToDo : update graphic scene

	return ret;
}

bool GEOModels::removePointVec( const std::string &name )
{
	if (! isPntVecUsed(name)) {
		for (std::vector<PntsModel*>::iterator it = _pntModels.begin(); it
				!= _pntModels.end(); ++it) {
			if ((*it)->name().toStdString() == name) {
				emit pointModelRemoved(*it);
				delete *it;
				_pntModels.erase(it);
				break;
			}
		}
		return GEOObjects::removePointVec(name);
	}
	std::cout << "PointVec " << name << " is used" << std::endl;
	return false;
}

void GEOModels::addStationVec( std::vector<GEOLIB::Point*> *stations, std::string name, const GEOLIB::Color* const color )
{
	GEOObjects::addStationVec(stations, name, color);
	_stationModel->addStationList(QString::fromStdString(name), stations);
	emit stationVectorAdded(_stationModel, name);
}

void GEOModels::filterStationVec(const std::string &name, const std::vector<PropertyBounds> &bounds)
{
	std::vector<GEOLIB::Point*> *stations = GEOObjects::getStationVec(name);
	_stationModel->filterStations(name, stations, bounds);
	emit stationVectorAdded(_stationModel, name);
}

bool GEOModels::removeStationVec( const std::string &name )
{
	emit stationVectorRemoved(_stationModel, name);
	_stationModel->removeStationList(name);
	bool ret (GEOObjects::removeStationVec(name));
	return ret;
}

void GEOModels::addPolylineVec( std::vector<GEOLIB::Polyline*> *lines, const std::string &name )
{
	GEOObjects::addPolylineVec(lines, name);

	PolylinesModel* model = new PolylinesModel(QString::fromStdString(name), lines, this);
	_lineModels.push_back(model);
	emit polylineModelAdded(model);
}

bool GEOModels::removePolylineVec( const std::string &name )
{
	if (isPlyVecUsed(name)) {
		std::cout
				<< "GEOModels::removePolylineVec() - there are Surfaces depending on these polylines"
				<< std::endl;
		return false;
	} else {
		for (std::vector<PolylinesModel*>::iterator it = _lineModels.begin();
				it != _lineModels.end(); ++it) {
			if ((*it)->name().toStdString() == name) {
				emit polylineModelRemoved(*it);
				delete *it;
				_lineModels.erase(it);
				return GEOObjects::removePolylineVec (name);
			}
		}
	}
	return false;
}

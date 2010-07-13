/*
 * GeoInfo.cpp
 *
 *  Created on: Jun 18, 2010
 *      Author: TF
 */

// STL
#include <limits>

// FEM
#include <GeoInfo.h>


GeoInfo::GeoInfo() :
	_geo_type (GEOLIB::POINT), _geo_obj_idx (std::numeric_limits<size_t>::max())
{}

GeoInfo::GeoInfo(GEOLIB::GEOTYPE geo_type, size_t geo_obj_idx) :
	_geo_type (geo_type), _geo_obj_idx (geo_obj_idx)
{}

GeoInfo::~GeoInfo()
{}

GEOLIB::GEOTYPE GeoInfo::getGeoType () const
{
	return _geo_type;
}

std::string GeoInfo::getGeoTypeAsString () const
{
	switch (_geo_type) {
	case GEOLIB::POINT:
		return "POINT";
	case GEOLIB::POLYLINE:
		return "POLYLINE";
	case GEOLIB::SURFACE:
		return "SURFACE";
	case GEOLIB::VOLUME:
		return "VOLUME";
	case GEOLIB::GEODOMAIN:
		return "DOMAIN";
	default:
		return "";
	}
}

void GeoInfo::setGeoType (GEOLIB::GEOTYPE geo_type)
{
	_geo_type = geo_type;
}

size_t GeoInfo::getGeoObjIdx () const
{
	return _geo_obj_idx;
}

void GeoInfo::setGeoObjIdx (size_t geo_obj_idx)
{
	_geo_obj_idx = geo_obj_idx;
}

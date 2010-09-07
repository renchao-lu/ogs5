/*
 * GeoInfo.h
 *
 *  Created on: Jun 18, 2010
 *      Author: TF
 */

#ifndef GEOINFO_H_
#define GEOINFO_H_

// STL
#include <cstddef>
#include <string>
#include <limits>

// GEO
#include "GeoType.h"

/**
 * GeoInfo stores the type of the geometric entity and
 * the index within the vector the geometric entity is
 * managed
 */
class GeoInfo {
public:
	/**
	 * standard constructor. You need to set the attributes via
	 * setGeoType() and setGeoObjIdx()!
	 */
	GeoInfo ();
	/**
	 * The constructor of a GeoInfo object initializes the
	 * attributes of the object.
	 * @param geo_type the type of the geometric entity
	 * @param geo_obj_idx the index in the vector
	 */
	GeoInfo(GEOLIB::GEOTYPE geo_type, size_t geo_obj_idx = std::numeric_limits<size_t>::max());
	/**
	 * virtual destructor - destroys the object
	 */
	virtual ~GeoInfo();

	/**
	 * getter method for the geo type
	 * @return the geo type
	 */
	GEOLIB::GEOTYPE getGeoType () const;

	/**
	 * get the type as a string for log output
	 * @return
	 */
	std::string getGeoTypeAsString () const;

	/**
	 * getter for the obj index
	 * @return
	 */
	size_t getGeoObjIdx () const;

	/**
	 * setter for the geo type
	 * @param geo_type type of the geometric entity
	 */
	void setGeoType (GEOLIB::GEOTYPE geo_type);
	/**
	 * setter for the object index
	 * @param geo_obj_idx the object index within the vector
	 */
	void setGeoObjIdx (size_t geo_obj_idx);

protected:
	/**
	 * type of the geometric entity
	 */
	GEOLIB::GEOTYPE _geo_type;
	/**
	 * index of geometric object (GEOLIB::Point, GEOLIB::Polyline, ...)
	 */
	size_t _geo_obj_idx;
};

#endif /* GEOINFO_H_ */

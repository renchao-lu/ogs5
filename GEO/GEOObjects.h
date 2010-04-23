/*
 * GEOObjects.h
 *
 *  Created on: Jan 21, 2010
 *      Author: fischeth / KR
 */

#ifndef GEOOBJECTS_H_
#define GEOOBJECTS_H_

#include <vector>
#include <string>

#include "PointVec.h"
#include "Point.h"
#include "PolylineVec.h"
#include "Polyline.h"
#include "SurfaceVec.h"
#include "Surface.h"

#include "Color.h"
#include "Station.h"

namespace GEOLIB {

/// \brief Container class for geometric objects.
/**
 * This class contains all the methods necessary for the I/O of geometric objects.
 * Said objects are Points, Polylines, Surfaces and Stations and they are stored in
 * vectors (arrays) which are identified by a unique name.
 * For a hierarchical definition, surfaces are bounded by polylines and polylines
 * are defined by points. Therefore, a vector of surfaces references a vector polylines
 * and a vector of polylines references a vector of points, respectively. For
 * identification purposes, all of these vectors have the same name, i.e. the polyline-
 * vector named "aaa" references a point vector "aaa". However, this name ("aaa") is
 * unique among all the vectors of the same class, i.e. there exists only one point-
 * vector with this name, etc.
 * Note: The fact that vectors are uniquely named and the same name is assigned to
 * related objects is automatically handled by this class.
 *
 * For each of these object-classes exists an "add", "remove" and "get"-method which
 * allows for loading/unloading as well as accessing the data, respectively.
 * E.g. for points these methods are "addPointVec(name)", "getPointVec(name)" and
 * "removePointVec(name)". For some objects, additional methods might exist if
 * necessary.
 */
class GEOObjects {
public:
	/// Adds a vector of points with the given name to GEOObjects.
	virtual void addPointVec(std::vector<Point*> *points, std::string &name);
	/** copies the pointers to the points in the vector to the PointVec with provided name.
	 * the pointers are managed by the GEOObjects, i.e. GEOObjects will delete the Points at the
	 * end of its scope
	 * \param points the vector with points
	 * \param name the name of the internal PointVec
	 * \return true if the points are appended, false if the a PointVec with the
	 * corresponding name does not exist
	 * */
	virtual bool appendPointVec(const std::vector<Point*> &points,
			std::string &name);
	/// Returns the point vector with the given name.
	std::vector<Point*> *getPointVec(const std::string &name);
	/** If there exists no dependencies the point vector with the given
	 * name from GEOObjects will be removed and the method returns true,
	 * else the return value is false.
	 */
	virtual bool removePointVec(const std::string &name);

	/// Adds a vector of stations with the given name and colour to GEOObjects.
	virtual void addStationVec(std::vector<Point*> *stations, std::string &name,
			const Color* const color);
	/// Filters a list of stations with the given name based on the criteria in PropertyBounds.
	/// (See property system in Station class for more information.)
	std::vector<Point*> *filterStationVec(const std::string &name,
			const std::vector<PropertyBounds> &bounds);
	/// Returns the station vector with the given name.
	std::vector<Point*> *getStationVec(const std::string &name);

	/// Removes the station vector with the given name from GEOObjects
	virtual bool removeStationVec(const std::string &name) {
		return removePointVec(name);
	}

	/// Adds a vector of polylines with the given name to GEOObjects.
	virtual void addPolylineVec(std::vector<Polyline*> *lines,
			const std::string &name);
	/// Returns the polyline vector with the given name.
	std::vector<Polyline*> *getPolylineVec(const std::string &name);
	/**
	 * If no Surfaces depends on the vector of Polylines with the given
	 * name it will be removed and the method returns true,
	 * else the return value is false.
	 */
	virtual bool removePolylineVec(const std::string &name);

	/** Adds a vector of surfaces with the given name to GEOObjects. */
	virtual void addSurfaceVec(std::vector<Surface*> *surfaces,
			const std::string &name);
	/// Returns the surface vector with the given name.
	std::vector<Surface*> *getSurfaceVec(const std::string &name);
	/** removes the vector of Surfaces with the given name */
	virtual bool removeSurfaceVec(const std::string &name);

	/** constructor */
	GEOObjects();
	/** destructor */
	virtual ~GEOObjects();

protected:
	/// Returns the total number of point vectors
//	size_t nPointVecs() { return _pnt_vecs.size(); }

	/// Returns the total number of polyline vectors
//	size_t nPolylineVecs() { return _ply_vecs.size(); }

	/// Returns the total number of surface vectors
//	size_t nSurfaceVecs() { return _sfc_vecs.size(); }

	/**
	 * Determines if the given name is unique among all the names in point vectors and creates a
	 * new name if this is not the case. The new name is then simply "name + x", where x>1 is
	 * the smallest number that creates a unique name (i.e. "name-2", "name-3", etc.)
	 * \param name Original name of the list, this name might be changed within this method if necessary.
	 * \return true if the name was unique, false if a new name has been generated
	 */
	bool isUniquePointVecName(std::string &name);

	/** Method search into the std::vector<PolylineVec*> for the given name */
	bool isPntVecUsed (const std::string &name) const;
	/** Method search into the std::vector<SurfaceVec*> for the given name */
	bool isPlyVecUsed (const std::string &name) const;
	/** vector manages pointers to PointVec objects */
	std::vector<PointVec*> _pnt_vecs;
	/** vector manages pointers to PolylineVec objects */
	std::vector<PolylineVec*> _ply_vecs;
	/** vector manages pointers to SurfaceVec objects */
	std::vector<SurfaceVec*> _sfc_vecs;
};

} // end namespace

#endif /* GEOOBJECTS_H_ */

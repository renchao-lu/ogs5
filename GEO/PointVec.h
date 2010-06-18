/*
 * \file PointVec.h
 *
 *  Created on: Feb 2, 2010
 *      Author: TF / KR
 */


// GEOLIB
#include "Point.h"
#include "Station.h"

// Base
#include "NameMapper.h"
#include "quicksort.h"
#include "binarySearch.h"

#include <vector>
#include <string>

#ifndef POINTVEC_H_
#define POINTVEC_H_

void makePntsUnique (std::vector<GEOLIB::Point*>* pnt_vec, std::vector<size_t> &pnt_id_map);

namespace GEOLIB {

/**
 * \brief This class manages pointers to Points in a std::vector along with a name.
 * It also handles the deleting of points. Additionally, each vector of points is identified by
 * a unique name from class GEOObject. For this reason PointVec should have
 * a name.
 * */
class PointVec : public NameMapper
{
public:
	/// Signals if the vector contains object of type Point or Station
	enum PointType
	{
		POINT    = 0,
		STATION  = 1
	};

	/**
	 * Constructor initializes the name of the PointVec object,
	 * the internal pointer _pnt_vec to the raw points and the internal
	 * pointer the vector of names of the points
	 * and sets the type of PointVec.
	 * @param name the name of the point group
	 * @param points pointer to a vector of GEOLIB::Pointers -
	 * PointVec will take the ownership of the vector,
	 * i.e. delete the points and the vector itself
	 * @param names the names to the points -
	 * PointVec will take the ownership of the vector, i.e. it
	 * deletes the names
	 * @param type the type of the point, \sa enum PointType
	 * @return an object of type PointVec
	 */
	PointVec (const std::string& name, std::vector<Point*>* points, std::vector<std::string>* names,
			PointType type = PointVec::POINT) :
		NameMapper (names), _pnt_vec(points), _type(type), _name (name)
	{
		assert (_pnt_vec);

		if (type == PointVec::POINT) {
			makePntsUnique (_pnt_vec, _pnt_id_map);
//			std::cout << "INFO: " << _pnt_vec->size() << " unique points" << std::endl;
		}
	}

	/** Destructor deletes all Points of this PointVec. */
	~PointVec ()
	{
		for (size_t k(0); k<size(); k++) {
			delete (*_pnt_vec)[k];
			(*_pnt_vec)[k] = NULL;
		}
		delete _pnt_vec;
	}

	/**
	 * Method adds a Point to the vector and takes care to delete it.
	 * */
	void push_back (Point *pnt) {
		push_back (pnt, "");
	}

	/**
	 * push_back adds new elements at the end of the vectors _pnt_vec
	 * and _pnt_vec_names.
	 * @param pnt a pointer to the point, PointVec takes ownership
	 * @param name the name of the point
	 */
	void push_back (Point *pnt, const std::string& name)
	{
		_perm.push_back (_pnt_vec->size());
		_pnt_vec->push_back (pnt);
		if (_ele_vec_names) {
			_ele_vec_names->push_back (name);
			// sort names of points
			quicksort (*_ele_vec_names, 0, _ele_vec_names->size(), _perm);
		}
	}

	/**
	 * get the actual number of Points
	 */
	size_t size () const { return _pnt_vec->size(); }
	/**
	 * get the type of Point, this can be either POINT or STATION
	 * */
	int getType() const { return _type; }

	/**
	 * getVector returns the internal vector of Points,
	 * you are not able to change the Points or the address of the vector.
	 */
	const std::vector<Point*>* getVector () const { return _pnt_vec; }

	std::vector<Point*> *filterStations(const std::vector<PropertyBounds> &bounds) const
	{
		std::vector<Point*> *tmpStations (new std::vector<Point*>);
		size_t size (_pnt_vec->size());
		for (size_t i=0; i<size; i++)
		{
			if (static_cast<Station*>((*_pnt_vec)[i])->inSelection(bounds)) tmpStations->push_back((*_pnt_vec)[i]);
		}
		return tmpStations;
	}

	/** sets the name of the object
	 * \param n the name as standard string */
	void setName(const std::string & n) { _name = n; }
	/** returns the name of the object */
	std::string getName () const { return _name; }

	/**
	 * search the vector of names for the ID of the point with the given name
	 * @param name the name of the point
	 * @param id the id of the point
	 * @return the id of the point
	 */
	bool getPointIDByName (const std::string& name, size_t &id) const
	{
		bool ret;
		if ((ret = getElementIDByName (name, id)))
			id = _pnt_id_map[id];
		return ret;
	}

	const std::vector<size_t>& getIDMap () const { return _pnt_id_map; }

private:
	/** copy constructor doesn't have an implementation */
	// compiler does not create a (possible unwanted) copy constructor
	PointVec (const PointVec &);
	/** standard constructor doesn't have an implementation */
	// compiler does not create a (possible unwanted) standard constructor
	PointVec ();

	/** assignment operator doesn't have an implementation */
	// this way the compiler does not create a (possible unwanted) assignment operator
	PointVec& operator= (const PointVec& rhs);

	/** pointer to a vector of pointers to Points */
	std::vector <Point*> *_pnt_vec;
	/** the type of the point (\sa enum PointType) */
	PointType _type;
	/** the name of the object */
	std::string _name;

	std::vector<size_t> _pnt_id_map;
};

} // end namespace

#endif /* POINTVEC_H_ */

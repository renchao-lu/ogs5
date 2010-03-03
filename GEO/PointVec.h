/*
 * \file PointVec.h
 *
 *  Created on: Feb 2, 2010
 *      Author: fischeth / KR
 */

#include "Point.h"
#include "Station.h"
#include <vector>
#include <string>

#ifndef POINTVEC_H_
#define POINTVEC_H_

namespace GEOLIB {

/**
 * \brief This class manages pointers to Points in a std::vector along with a name.
 * It also handles the deleting of points. Additionally, each vector of points is identified by
 * a unique name from class GEOObject. For this reason PointVec should have
 * a name.
 * */
class PointVec
{
public:
	/// Signals if the vector contains object of type Point or Station
	enum PointType
	{
		POINT    = 0,
		STATION  = 1,
	};

	/**
	 * Constructor constructs a new std::vector-pointer to manage the
	 * pointers to Points.
	 * */
	PointVec () : _pnt_vec(new std::vector<Point*>), _type(PointVec::POINT)
	{};
	/**
	 * Constructor initialises the internal pointer to the std::vector<POINT>
	 * and sets the type of PointVec.
	 * */
	PointVec (std::vector<Point*>* points, PointType type = PointVec::POINT) :
		_pnt_vec(points), _type(type) {}
	/**
	 * Constructor constructs a new std::vector-pointer to manage the
	 * pointers to Points, sets the name to the given name and the type
	 * of PointVec to the given type.
	 * */
	PointVec(const std::string &name, PointType type = PointVec::POINT) :
		_pnt_vec(new std::vector<Point*>), _type(type), _name (name)
	{}

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
		_pnt_vec->push_back (pnt);
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
	/**
	 * getVector returns the internal vector of Points, the vector and points
	 * could be changed
	 */
	std::vector<Point*> *getVector () { return _pnt_vec; }

	std::vector<Point*> *filterStations(const std::vector<PropertyBounds> &bounds)
	{
		std::vector<Point*> *tmpStations = new std::vector<Point*>();
		int size = static_cast<int>(_pnt_vec->size());
		for (int i=0; i<size; i++)
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

private:
	/** copy constructor doesn't have an implementation */
	// compiler does not create a (possible unwanted) copy constructor
	PointVec (const PointVec &);
	/** assignment operator doesn't have an implementation */
	// this way the compiler does not create a (possible unwanted) assignment operator
	PointVec& operator= (const PointVec& rhs);
	/** pointer to a vector of pointers to Points */
	std::vector <Point*> *_pnt_vec;
	PointType _type;
	/** the name of the object */
	std::string _name;
};

} // end namespace


#endif /* POINTVEC_H_ */

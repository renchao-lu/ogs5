/*
 * PolylineVec.h
 *
 *  Created on: Feb 9, 2010
 *      Author: fischeth
 */

#ifndef POLYLINEVEC_H_
#define POLYLINEVEC_H_

#include "TemplateVec.h"
#include "Polyline.h"

namespace GEOLIB {


/**
 * \brief class PolylineVec encapsulate a std::vector of Polylines
 * additional one can give the vector of polylines a name
 * */

typedef TemplateVec<Polyline> PolylineVec;

//class PolylineVec
//{
//public:
//	PolylineVec () : _ply_vec (new std::vector<Polyline*>) {};
//	PolylineVec (std::vector<Polyline*> *lines) : _ply_vec(lines) {};
//	PolylineVec (const std::string &name) :
//		_ply_vec (new std::vector<Polyline*>), _name (name)
//	{}
//
//	virtual ~PolylineVec ()
//	{
//		for (size_t k(0); k<size(); k++) delete (*_ply_vec)[k];
//		delete _ply_vec;
//	}
//
//	void push_back (Polyline* ply) {
//		_ply_vec->push_back (ply);
//	}
//	virtual size_t size () const { return _ply_vec->size(); }
//
//	const std::vector<Polyline*>* const getVector () const { return _ply_vec; }
//	std::vector<Polyline*> *getVector () { return _ply_vec; }
//
//	/** sets the name of the object
//	 * \param n the name as standard string */
//	void setName (const std::string & n) { _name = n; }
//	/** returns the name of the object */
//	std::string getName () const { return _name; }
//
//private:
//	std::vector <Polyline*> *_ply_vec;
//	/** the name of the object */
//	std::string _name;
//};

} // end namespace

#endif /* POLYLINEVEC_H_ */

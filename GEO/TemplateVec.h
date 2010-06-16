/*
 * TemplateVec.h
 *
 *  Created on: Feb 26, 2010
 *      Author: TF
 */

#ifndef TEMPLATEVEC_H_
#define TEMPLATEVEC_H_

// Base
#include "NameMapper.h"

namespace GEOLIB {

/** \brief class TemplateVec encapsulate a std::vector of pointers to T
 * additional one can give the vector of T a name
 *
 * instances are PolylineVec and SurfaceVec
 * */
template <class T> class TemplateVec : public NameMapper
{
public:
	/**
	 * Constructor.
	 * @param name the name of the project
	 * @param data_vec vector of data elements
	 * @param names vector of names  of the data elements
	 * @return an object of the appropriate class
	 */
	TemplateVec (const std::string &name, std::vector<T*> *data_vec, std::vector<std::string>* names) :
		NameMapper (names), _name(name), _data_vec(data_vec)
	{}

	/**
	 * destructor, deletes all data elements
	 * @return
	 */
	virtual ~TemplateVec ()
	{
		for (size_t k(0); k<size(); k++) delete (*_data_vec)[k];
		delete _data_vec;
	}

	/**
	 * @return the number of data elements
	 */
	size_t size () const { return _data_vec->size(); }

	/**
	 * get a pointer to a standard vector containing the data elements
	 * @return the data elements
	 */
	const std::vector<T*>* getVector () const { return _data_vec; }

	/** sets the name of the object/project,
	 * the data elements belonging to
	 * \param n the name as standard string */
	void setName (const std::string & n) { _name = n; }
	/**
	 * the (project) name, the data element belonging to
	 * @return the name of the object
	 */
	std::string getName () const { return _name; }

private:
	/** copy constructor doesn't have an implementation */
	// compiler does not create a (possible unwanted) copy constructor
	TemplateVec (const TemplateVec &);
	/** assignment operator doesn't have an implementation */
	// this way the compiler does not create a (possible unwanted) assignment operator
	TemplateVec& operator= (const TemplateVec& rhs);

	/** the name of the object */
	std::string _name;

	/**
	 * pointer to a vector of data elements
	 */
	std::vector <T*> *_data_vec;
};

} // end namespace GEOLIB

#endif /* TEMPLATEVEC_H_ */

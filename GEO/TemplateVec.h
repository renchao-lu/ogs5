/*
 * TemplateVec.h
 *
 *  Created on: Feb 26, 2010
 *      Author: fischeth
 */

#ifndef TEMPLATEVEC_H_
#define TEMPLATEVEC_H_

namespace GEOLIB {

/** \brief class TemplateVec encapsulate a std::vector of pointers to T
 * additional one can give the vector of T a name
 *
 * instances are PolylineVec and SurfaceVec
 * */
template <class T> class TemplateVec
{
public:
	/**
	 * Constructor constructs a new std::vector to manage the
	 * pointers to T.
	 * */
	TemplateVec () : _data_vec (new std::vector<T*>) {};
	/**
	 * Constructor initialises the internal pointer to the std::vector<T*>
	 * */
	TemplateVec (std::vector<T*> *data_items) : _data_vec(data_items) {};
	TemplateVec (const std::string &name) :
		_data_vec (new std::vector<T*>), _name (name)
	{}
	TemplateVec (std::vector<T*> *data_vec, const std::string &name) :
		_data_vec(data_vec), _name(name)
	{}

	virtual ~TemplateVec ()
	{
		for (size_t k(0); k<size(); k++) delete (*_data_vec)[k];
		delete _data_vec;
	}
	/**
	 * Method adds a data item to the vector. The class
	 * takes care over the deletion of data item, do not delete
	 * the data!
	 * */
	void push_back (T* data_item) {
		_data_vec->push_back (data_item);
	}

	size_t size () const { return _data_vec->size(); }

	const std::vector<T*>* getVector () const { return _data_vec; }
	std::vector<T*>* getVector () { return _data_vec; }

	/** sets the name of the object
	 * \param n the name as standard string */
	void setName (const std::string & n) { _name = n; }
	/** returns the name of the object */
	std::string getName () const { return _name; }

private:
	/** copy constructor doesn't have an implementation */
	// compiler does not create a (possible unwanted) copy constructor
	TemplateVec (const TemplateVec &);
	/** assignment operator doesn't have an implementation */
	// this way the compiler does not create a (possible unwanted) assignment operator
	TemplateVec& operator= (const TemplateVec& rhs);
	std::vector <T*> *_data_vec;
	/** the name of the object */
	std::string _name;
};

} // end namespace GEOLIB

#endif /* TEMPLATEVEC_H_ */

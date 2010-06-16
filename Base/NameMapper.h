/*
 * NameMapper.h
 *
 *  Created on: Jun 9, 2010
 *      Author: TF
 */

#ifndef NAMEMAPPER_H_
#define NAMEMAPPER_H_

// STL
#include <vector>
#include <string>

// Base
#include "quicksort.h"

/**
 * Class NameMapper allows access by names in logarithmic time.
 * To ensure access in logarithmic time the names are sorted
 * lexicografically.
 */
class NameMapper
{
public:
	NameMapper(std::vector<std::string>* names);
	virtual ~NameMapper();

protected:
	/**
	 * search the vector of names for the ID of the point with the given name
	 * @param name the name of the point
	 * @param id the id of the point
	 * @return the id of the point
	 */
	bool getElementIDByName (const std::string& name, size_t &id) const;
	/**
	 * This is vector of names.
	 * To allow an efficient searching this vector will be sorted.
	 */
	std::vector<std::string>* _ele_vec_names;
	/**
	 * The permutation of points according their names are stored in _perm.
	 * \sa _pnt_vec_names and getPointByName()
	 */
	std::vector<size_t> _perm;
};

#endif /* NAMEMAPPER_H_ */

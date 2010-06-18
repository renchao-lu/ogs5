/*
 * NameMapper.cpp
 *
 *  Created on: Jun 9, 2010
 *      Author: TF
 */

#include "NameMapper.h"
#include "binarySearch.h"

NameMapper::NameMapper(std::vector<std::string>* names) :
	_ele_vec_names (names)
{
	if (_ele_vec_names) {
		size_t n_pnts (_ele_vec_names->size());
		_perm.reserve (n_pnts);
		for (size_t k(0); k<n_pnts; k++) _perm.push_back (k);
		// sort names of points
		quicksort (*_ele_vec_names, 0, _ele_vec_names->size(), _perm);
	} 
//	else {
//		std::cerr << "INFO: no names for geometric entities given" << std::endl;
//	}
}

NameMapper::~NameMapper()
{
	delete _ele_vec_names;
}

bool NameMapper::getElementIDByName (const std::string& name, size_t &id) const
{
	if (_ele_vec_names) {
		// binary search points
		size_t beg (0), end (_ele_vec_names->size());
		id = searchElement (name, beg, end, (*_ele_vec_names));
		if (id != std::numeric_limits<size_t>::max()) {
			id = _perm[id];
			return true;
		}
	}

	return false;
}

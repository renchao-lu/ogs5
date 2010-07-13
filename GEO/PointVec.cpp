/*
 * PointVec.cpp
 *
 *  Created on: Jun 11, 2010
 *      Author: TF
 */

#include "PointVec.h"

namespace GEOLIB {

PointVec::PointVec (const std::string& name, std::vector<Point*>* points, std::vector<std::string>* names,
			PointType type)
: NameMapper (names), _pnt_vec(points), _type(type), _name (name)
{
	assert (_pnt_vec);
	std::cout << "INFO: " << _pnt_vec->size() << " points" << std::endl;
	makePntsUnique (_pnt_vec, _pnt_id_map);
	std::cout << "INFO: " << _pnt_vec->size() << " unique points" << std::endl;
}

PointVec::~PointVec ()
{
	for (size_t k(0); k<size(); k++) {
		delete (*_pnt_vec)[k];
		(*_pnt_vec)[k] = NULL;
	}
	delete _pnt_vec;
}

std::vector<Point*> * PointVec::filterStations(const std::vector<PropertyBounds> &bounds) const
{
	std::vector<Point*> *tmpStations (new std::vector<Point*>);
	size_t size (_pnt_vec->size());
	for (size_t i=0; i<size; i++)
	{
		if (static_cast<Station*>((*_pnt_vec)[i])->inSelection(bounds)) tmpStations->push_back((*_pnt_vec)[i]);
	}
	return tmpStations;
}

bool PointVec::getPointIDByName (const std::string& name, size_t &id) const
{
	bool ret;
	if ((ret = getElementIDByName (name, id)))
		id = _pnt_id_map[id];
	return ret;
}

}

void makePntsUnique (std::vector<GEOLIB::Point*>* pnt_vec, std::vector<size_t> &pnt_id_map)
{
	size_t n_pnts_in_file (pnt_vec->size());
	std::vector<size_t> perm;
	pnt_id_map.reserve (n_pnts_in_file);
	for (size_t k(0); k<n_pnts_in_file; k++) {
		perm.push_back (k);
		pnt_id_map.push_back(k);
	}

	// sort the points, unfortunately quicksort is not stable
	Quicksort<GEOLIB::Point*> (*pnt_vec, 0, n_pnts_in_file, perm);

	// sort identical points by id - to make sorting stable
	double eps (sqrt(std::numeric_limits<double>::min()));
	// determine intervals with identical points to resort for stability of sorting
	std::vector<size_t> identical_pnts_interval;
	bool identical (false);
	for (size_t k=0; k<n_pnts_in_file-1; k++) {
		if ( fabs((*((*pnt_vec)[k+1]))[0]-(*((*pnt_vec)[k]))[0]) < eps
			&&  fabs( (*((*pnt_vec)[k+1]))[1]-(*((*pnt_vec)[k]))[1]) < eps
			&&  fabs( (*((*pnt_vec)[k+1]))[2]-(*((*pnt_vec)[k]))[2]) < eps) {
			// points are identical, sort by id
			if (!identical) identical_pnts_interval.push_back (k);
			identical = true;
		} else {
			if (identical) identical_pnts_interval.push_back (k+1);
			identical = false;
		}
	}
	if (identical) identical_pnts_interval.push_back (n_pnts_in_file);

	for (size_t i(0); i<identical_pnts_interval.size()/2; i++) {
		// bubble sort by id
		size_t beg (identical_pnts_interval[2*i]);
		size_t end (identical_pnts_interval[2*i+1]);
		for (size_t j (beg); j<end; j++) {
			for (size_t k (beg); k<end-1; k++) {
				if (perm[k] > perm[k+1]) std::swap (perm[k], perm[k+1]);
			}
		}
	}

//	std::cout << "sorted points: " << std::endl;
//	for (size_t k(0); k<pnt_vec->size(); k++) std::cout << k << ": " << *((*pnt_vec)[k]) << "  | " << pnt_id_map[perm[k]] << std::endl;

	// check if there are identical points
	for (size_t k=0; k<n_pnts_in_file-1; k++) {
		if ( fabs((*((*pnt_vec)[k+1]))[0]-(*((*pnt_vec)[k]))[0]) < eps
				&&  fabs( (*((*pnt_vec)[k+1]))[1]-(*((*pnt_vec)[k]))[1]) < eps
				&&  fabs( (*((*pnt_vec)[k+1]))[2]-(*((*pnt_vec)[k]))[2]) < eps) {
			pnt_id_map[perm[k+1]] = pnt_id_map[perm[k]];
		}
	}
//	std::cout << "*** id mapping: " << std::endl;
//	for (size_t k(0); k<n_pnts_in_file; k++) std::cout << k << ": " << pnt_id_map[k] << std::endl;

	// reverse permutation
	Quicksort<GEOLIB::Point*> (perm, 0, n_pnts_in_file, *pnt_vec);

	// remove the second, third, ... occurrence from vector
	for (size_t k(0); k<n_pnts_in_file; k++) {
		if (pnt_id_map[k] < k) {
			delete (*pnt_vec)[k];
			(*pnt_vec)[k] = NULL;
		}
	}
	// remove NULL-ptr from vector
	for (std::vector<GEOLIB::Point*>::iterator it(pnt_vec->begin());
		it != pnt_vec->end(); ) {
		if (*it == NULL) {
			it = pnt_vec->erase (it);
		}
		else it++;
	}

	size_t cnt (0);
	for (size_t k(0); k<n_pnts_in_file; k++) {
		if (pnt_id_map[k] == k) {
			pnt_id_map[k] = cnt;
			cnt++;
		}
	}

	// renumber id-mapping
	size_t n_unique_pnts (pnt_vec->size());
	for (size_t k(1); k<n_pnts_in_file; k++) {
		size_t j (pnt_id_map[k]);
		while (j != pnt_id_map[j] && j > n_unique_pnts) j = pnt_id_map[j];
		pnt_id_map[k] = j;
	}

//	std::cout << "id mapping: " << std::endl;
//	for (size_t k(0); k<n_pnts_in_file; k++) std::cout << k << ": " << pnt_id_map[k] << std::endl;
//
//	std::cout << pnt_vec->size() << " unique points: " << std::endl;
//	for (size_t k(0); k<pnt_vec->size(); k++) std::cout << k << ": " << *((*pnt_vec)[k]) << std::endl;

}

/*
 * PointVec.cpp
 *
 *  Created on: Jun 11, 2010
 *      Author: TF
 */

#include "PointVec.h"

void makePntsUnique (std::vector<GEOLIB::Point*>* pnt_vec, std::vector<size_t> &pnt_id_map)
{
	size_t n_pnts_in_file (pnt_vec->size());
	size_t *perm (new size_t[n_pnts_in_file]);
	pnt_id_map.reserve (n_pnts_in_file);
	for (size_t k(0); k<n_pnts_in_file; k++) {
		perm[k] = k;
		pnt_id_map.push_back(k);
	}

	quicksortPnts (*pnt_vec, 0, n_pnts_in_file, perm);

	double eps (sqrt(std::numeric_limits<double>::min()));
	// check if there are point with same coordinates
	for (size_t k=0; k<n_pnts_in_file-1; k++) {
		if ( fabs((*((*pnt_vec)[k+1]))[0]-(*((*pnt_vec)[k]))[0]) < eps
				&&  fabs( (*((*pnt_vec)[k+1]))[1]-(*((*pnt_vec)[k]))[1]) < eps
				&&  fabs( (*((*pnt_vec)[k+1]))[2]-(*((*pnt_vec)[k]))[2]) < eps) {
			pnt_id_map[perm[k+1]] = pnt_id_map[perm[k]];
		}
	}
	// reverse permutation
	quicksortPnts (perm, 0, n_pnts_in_file, *pnt_vec);

	std::vector<GEOLIB::Point*> *unique_pnt_vec(new std::vector<GEOLIB::Point*>);
	for (size_t k(0); k<n_pnts_in_file; k++) {
		if (pnt_id_map[k] == k)
			unique_pnt_vec->push_back (new GEOLIB::Point ((*(*pnt_vec)[k])[0], (*(*pnt_vec)[k])[1], (*(*pnt_vec)[k])[2]));
	}

	size_t cnt (0);
	for (size_t k(0); k<n_pnts_in_file; k++) {
		if (pnt_id_map[k] == k) {
			pnt_id_map[k] = cnt;
			cnt++;
		}
	}

	// renumber id-mapping
	size_t n_unique_pnts (unique_pnt_vec->size());
	for (size_t k(1); k<n_pnts_in_file; k++) {
		size_t j (pnt_id_map[k]);
		while (j != pnt_id_map[j] && j > n_unique_pnts) j = pnt_id_map[j];
		pnt_id_map[k] = j;
	}

//	std::cout << "id mapping: " << std::endl;
//	for (size_t k(0); k<n_pnts_in_file; k++) std::cout << k << ": " << pnt_id_map[k] << std::endl;

	for (size_t k(0); k<pnt_vec->size(); k++) {
		delete (*pnt_vec)[k];
	}
	pnt_vec->erase (pnt_vec->begin(), pnt_vec->end());
	for (size_t k(0); k<unique_pnt_vec->size(); k++) {
		pnt_vec->push_back ((*unique_pnt_vec)[k]);
	}

//	std::cout << "unique points: " << std::endl;
//	for (size_t k(0); k<pnt_vec->size(); k++) std::cout << k << ": " << *((*pnt_vec)[k]) << std::endl;

	delete unique_pnt_vec;
	delete [] perm;
}


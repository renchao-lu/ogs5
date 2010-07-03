/*
 * quicksort.cpp
 *
 *  Created on: May 26, 2010
 *      Author: TF
 */

#include "quicksort.h"

// MathLib
#include "MathTools.h"

size_t partitionPnts_(std::vector<GEOLIB::Point*>& array, size_t beg, size_t end, size_t *perm)
{
	size_t i = beg + 1;
	size_t j = end - 1;
	GEOLIB::Point* m = array[beg];

	for (;;) {
		while ((i < end) && (isLowerEqual (*array[i], *m)))
			i++;
		while ((j > beg) && !(isLowerEqual (*array[j], *m)))
			j--;

		if (i >= j)
			break;
		BASELIB::swap(array[i], array[j]);
		BASELIB::swap(perm[i], perm[j]);
	}

	BASELIB::swap(array[beg], array[j]);
	BASELIB::swap(perm[beg], perm[j]);
	return j;
}

void quicksortPnts(std::vector<GEOLIB::Point*>& array, size_t beg, size_t end, size_t* perm)
{
	if (beg < end) {
		size_t p = partitionPnts_(array, beg, end, perm);
		quicksortPnts(array, beg, p, perm);
		quicksortPnts(array, p+1, end, perm);
	}
}

size_t partitionPnts_(size_t *perm, size_t beg, size_t end, std::vector<GEOLIB::Point*>& array)
{
	size_t i = beg + 1;
	size_t j = end - 1;
	size_t m = perm[beg];

	for (;;) {
		while ((i < end) && (perm[i] <= m))
			i++;
		while ((j > beg) && !(perm[j] <= m))
			j--;

		if (i >= j)
			break;
		BASELIB::swap(perm[i], perm[j]);
		BASELIB::swap(array[i], array[j]);
	}

	BASELIB::swap(perm[beg], perm[j]);
	BASELIB::swap(array[beg], array[j]);
	return j;
}

void quicksortPnts(size_t *perm, size_t beg, size_t end, std::vector<GEOLIB::Point*>& array)
{
	if (beg < end) {
		size_t p = partitionPnts_(perm, beg, end, array);
		quicksortPnts(perm, beg, p, array);
		quicksortPnts(perm, p+1, end, array);
	}
}

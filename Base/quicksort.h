/*
 * quicksort.h
 *
 *  Created on: May 26, 2010
 *      Author: TF
 */

#ifndef QUICKSORT_H_
#define QUICKSORT_H_

// STL
#include <cstddef>

// Base
#include "swap.h"

/**
 * version of partition_ that additional updates the permutation vector
 * */
template <class T>
size_t partition_(T* array, size_t beg, size_t end, size_t *perm)
{
	size_t i = beg + 1;
	size_t j = end - 1;
	T m = array[beg];

	for (;;) {
		while ((i < end) && (array[i] <= m))
			i++;
		while ((j > beg) && !(array[j] <= m))
			j--;

		if (i >= j)
			break;
		::swap(array[i], array[j]);
		::swap(perm[i], perm[j]);
	}

	::swap(array[beg], array[j]);
	::swap(perm[beg], perm[j]);
	return j;
}

/**
 * version of quickSort that stores the permutation
 * */
template <class T>
void quicksort(T* array, size_t beg, size_t end, size_t* perm)
{
	if (beg < end) {
		size_t p = partition_(array, beg, end, perm);
		quicksort(array, beg, p, perm);
		quicksort(array, p+1, end, perm);
	}
}

// specializations
// STL
#include <vector>
/**
 * version of partition_ that additional updates the permutation vector
 * */
template <class T>
size_t partition_(std::vector<T>& array, size_t beg, size_t end, std::vector<size_t>& perm)
{
	size_t i = beg + 1;
	size_t j = end - 1;
	T m = array[beg];

	for (;;) {
		while ((i < end) && (array[i] <= m))
			i++;
		while ((j > beg) && !(array[j] <= m))
			j--;

		if (i >= j)
			break;
		::swap(array[i], array[j]);
		::swap(perm[i], perm[j]);
	}

	::swap(array[beg], array[j]);
	::swap(perm[beg], perm[j]);
	return j;
}

/**
 * version of quickSort that stores the permutation
 * */
template <class T>
void quicksort(std::vector<T>& array, size_t beg, size_t end, std::vector<size_t>& perm)
{
	if (beg < end) {
		size_t p = partition_(array, beg, end, perm);
		quicksort(array, beg, p, perm);
		quicksort(array, p+1, end, perm);
	}
}


// GEOLIB
#include "Point.h"
/**
 * version of partition_ that additional updates the permutation vector
 * */
size_t partitionPnts_(std::vector<GEOLIB::Point*>& array, size_t beg, size_t end, size_t *perm);

/**
 * version of quickSort that stores the permutation
 * */
void quicksortPnts(std::vector<GEOLIB::Point*>& array, size_t beg, size_t end, size_t* perm);

/**
 * sorts the points according the given permutation
 * @param perm the permutation
 * @param beg the beginning index
 * @param end the ending index
 * @param array the field of GEOLIB::Point
 * @return the splitting index
 */
size_t partitionPnts_(size_t *perm, size_t beg, size_t end, std::vector<GEOLIB::Point*>& array);

/**
 * sorts the points according the given permutation using function partitionPnts_
 * @param perm the permutation
 * @param beg the beginning index
 * @param end the ending index
 * @param array the field of GEOLIB::Point
 */
void quicksortPnts(size_t *perm, size_t beg, size_t end, std::vector<GEOLIB::Point*>& array);



#endif /* QUICKSORT_H_ */

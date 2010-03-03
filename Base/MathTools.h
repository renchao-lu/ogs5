/*
 * \file MathTools.h
 *
 *  Created on: Jan 13, 2010
 *      Author: fischeth
 */

#ifndef MATHTOOLS_H_
#define MATHTOOLS_H_

#include <vector>
#include "Point.h"

// forward declaration
class CGLPoint;

/**
 * standard inner product in R^3
 * \param v0 array of type T representing the vector
 * \param v1 array of type T representing the vector
 * \param n the size of the array
 * */
template <class T>
double scpr (const T* const v0, const T* const v1, size_t n)
{
	long double res (0.0);
	for (size_t k(0); k<n; k++) res += v0[k]*v1[k];
	return (double)res;
}

/** squared euklid norm of the vector p0 */
double sqrNrm2 (const GEOLIB::Point* const p0);

/** squared dist between p0 and p1 */
double sqrDist (const GEOLIB::Point* const p0, const GEOLIB::Point* const p1);

/** squared euklid norm of the vector p0
 * using old CGLPoint
 */
double sqrNrm2(const CGLPoint* const p0);

/** squared dist between p0 and p1 */
double sqrDist (const CGLPoint* const p0, const CGLPoint* const p1);

#endif /* MATHTOOLS_H_ */

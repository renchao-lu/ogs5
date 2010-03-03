/*
 * GEOHelpers.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: fischeth
 */

#include "MathTools.h"
#include "geo_pnt.h"
//#include "Point.h"

double sqrNrm2 (const GEOLIB::Point* const p0)
{
	return scpr (p0->getData(), p0->getData(), 3);
}

double sqrDist (const GEOLIB::Point* const p0, const GEOLIB::Point* const p1)
{
	double v[3] = {(*p1)[0] - (*p0)[0], (*p1)[1] - (*p0)[1], (*p1)[2] - (*p0)[2] };
	return scpr (v, v, 3);
}

double sqrNrm2 (const CGLPoint* const p0)
{
	double v[3] = { p0->getX(), p0->getY(), p0->getZ() };
	return scpr (v, v, 3);
}

double sqrDist(const CGLPoint* const p0, const CGLPoint* const p1)
{
	double v[3] = { p1->getX() - p0->getX(), p1->getY() - p0->getY(), p1->getZ() - p0->getZ() };
	return scpr (v, v, 3);
}


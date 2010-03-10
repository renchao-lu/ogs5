/*
 * GEOHelpers.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: TF
 */

#include "MathTools.h"
#include "geo_pnt.h"

double calcProjPntToLineAndDists(const double p[3], const double a[3],
		const double b[3], double proj_pnt[3],
		double &d, double &d0, double &d1)
{
	// g (lambda) = a + lambda v, v = b-a
	double v[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
	// (g(lambda)-p) * v = 0 => in order to compute lambda we define a help vector u
	double u[3] = {p[0] - a[0], p[1] - a[1], p[2] - a[2]};
	double lambda (scpr (u, v, 3) / scpr (v, v, 3));

	// compute projected point
	for (size_t k(0); k<3; k++) proj_pnt[k] = a[k] + lambda * v[k];

	d = sqrt (sqrDist (p, proj_pnt));
	d0 = sqrt (sqrDist (proj_pnt, a));
	d1 = sqrt (sqrDist (proj_pnt, b));

	return lambda;
}

double sqrNrm2 (const GEOLIB::Point* p0)
{
	return scpr (p0->getData(), p0->getData(), 3);
}

double sqrDist (const GEOLIB::Point* p0, const GEOLIB::Point* p1)
{
	double v[3] = {(*p1)[0] - (*p0)[0], (*p1)[1] - (*p0)[1], (*p1)[2] - (*p0)[2]};
	return scpr (v, v, 3);
}

double sqrDist(const double* p0, const double* p1)
{
	double v[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
	return scpr (v, v, 3);
}

double sqrNrm2 (const CGLPoint* p0)
{
	double v[3] = { p0->getX(), p0->getY(), p0->getZ() };
	return scpr (v, v, 3);
}

double sqrDist(const CGLPoint* p0, const CGLPoint* p1)
{
	double v[3] = { p1->getX() - p0->getX(), p1->getY() - p0->getY(), p1->getZ() - p0->getZ() };
	return scpr (v, v, 3);
}


/*
 * \file AnalyticalGeometry.h
 *
 *  Created on: Mar 17, 2010
 *      Author: TF
 */

#ifndef ANALYTICAL_GEOMETRY_H_
#define ANALYTICAL_GEOMETRY_H_

// MathLib
#include "Vector3.h"

namespace GEOLIB {
	class Polyline;
}

namespace MATHLIB {

enum orientation {
	CW = 1,
	CCW = 2,
	COLLINEAR = 3
};

/**
 * computes the circumscribed circle of a triangle given by the 3d-points a, b, c
 */
//void getCircumscribedSphereOfTriangle(const double a[3], const double b[3],
//		const double c[3], double middle_pnt[3], double& radius);

/**
 * compute a supporting plane (represented by plane_normal and d) for the polygon
 */
void getNewellPlane (GEOLIB::Polyline* ply, Vector &plane_normal, double& d);

//void delaunayTriangulatePolygon(const GEOLIB::Polyline* ply, std::list<GEOLIB::Triangle> &triangles);

/**
 * simple triangulation of simple polygons - ear clipping algorithm
 */
void earClippingTriangulationOfPolygon (const GEOLIB::Polyline* ply, std::list<GEOLIB::Triangle> &triangles);

} // end namespace MATHLIB

#endif /* MATHTOOLS_H_ */

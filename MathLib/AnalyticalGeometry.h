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
 * computes the orientation of the three 2D-Points given by their coordinates
 * p0_x, p0_y, p1_x, p1_y, p2_x and p2_y
 * \returns CW (clockwise), CCW (counterclockwise) or COLLINEAR (points are on a line)
 */
size_t getOrientation (const double& p0_x, const double& p0_y,
	const double& p1_x, const double& p1_y,
	const double& p2_x, const double& p2_y);

/**
 * wrapper for getOrientation ()
 */
size_t getOrientation (const GEOLIB::Point* p0, const GEOLIB::Point* p1, const GEOLIB::Point* p2);

/**
 * computes the circumscribed circle of a triangle given by the 3d-points a, b, c
 */
//void getCircumscribedSphereOfTriangle(const double a[3], const double b[3],
//		const double c[3], double middle_pnt[3], double& radius);

/**
 * compute a supporting plane (represented by plane_normal and the value d) for the polygon
 * Let \f$n\f$ be the plane normal and \f$d\f$ a parameter. Then for all points \f$p \in \R^3\f$ of the plane
 * it holds \f$ n \cdot p + d = 0\f$
 * @param ply a closed polyline describing a polygon
 * @param plane_normal the normal of the plane the polygon is located in
 * @param d parameter from the plane equation
 */
void getNewellPlane (GEOLIB::Polyline* ply, Vector &plane_normal, double& d);

//void delaunayTriangulatePolygon(const GEOLIB::Polyline* ply, std::list<GEOLIB::Triangle> &triangles);

/**
 * simple triangulation of simple polygons - ear clipping algorithm
 */
void earClippingTriangulationOfPolygon (const GEOLIB::Polyline* ply, std::list<GEOLIB::Triangle> &triangles);

/**
 * Function getSimplePolygons checks if the Polylines of the list
 * are simple Polygons. If a Polygon is not simple it is decomposed into
 * simple Polygons. The original Polygon in the list is substituted by these.
 * @param ply_list list of Polylines describing Polygons
 * @param pnt_vec In case we have to add some points we need the vector of points.
 */
void getListOfSimplePolygons (std::list<GEOLIB::Polyline*>& ply_list, std::vector<GEOLIB::Point*>& pnt_vec);

} // end namespace MATHLIB

#endif /* MATHTOOLS_H_ */

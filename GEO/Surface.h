/*
 * Surface.h
 *
 *  Created on: Jan 22, 2010
 *      Author: fischeth
 */

#ifndef SURFACE_H_
#define SURFACE_H_

#include <vector>
#include "Point.h"
#include "Triangle.h"

namespace GEOLIB {

/** \brief A Surface is represented by Triangles. It consists of a reference
 * to a vector of (pointers to) points (m_sfc_pnts) and a vector that stores
 * the Triangles consisting of points from m_sfc_pnts.
 * */
class Surface
{
public:
	Surface	(const std::vector<Point*> &pnt_vec) : m_sfc_pnts(pnt_vec) {};
	virtual ~Surface ()
	{
		for (size_t k(0); k<m_sfc_triangles.size(); k++)
			delete m_sfc_triangles[k];
	};

	/**
	 * adds three indices describing a triangle
	 * */
	void addTriangle (size_t pnt_a, size_t pnt_b, size_t pnt_c)
	{
		assert (pnt_a < m_sfc_pnts.size() && pnt_b < m_sfc_pnts.size() && pnt_c < m_sfc_pnts.size());
		m_sfc_triangles.push_back (new Triangle(m_sfc_pnts, pnt_a, pnt_b, pnt_c));
	}

	/**
	 * returns the number of triangles describing the Surface
	 * */
	size_t getNTriangles () const { return m_sfc_triangles.size(); }

	/** \brief const access operator for the access to the i-th Triangle of the surface.
	*/
	const Triangle* operator[] (size_t i) const {
		assert (i < m_sfc_triangles.size());
		return m_sfc_triangles[i];
	}

protected:
	/** a vector of pointers to Points */
	const std::vector<Point*> &m_sfc_pnts;
	/** position of pointers to the geometric points */
	std::vector<Triangle*> m_sfc_triangles;
};

}

#endif /* SURFACE_H_ */

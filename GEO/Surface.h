/*
 * Surface.h
 *
 *  Created on: Jan 22, 2010
 *      Author: fischeth
 */

#ifndef SURFACE_H_
#define SURFACE_H_

#include <vector>

#include "Polyline.h"

namespace GEOLIB {

/** \brief Class Surface consists of a reference to a polyline vector and
 * a vector that stores the indices in the polyline vector.
 * A surface is composed by the enclosing polylines. The class Surface stores the position
 * of pointers to the polylines in the m_sfc_ply_ids vector.
 * */
class Surface
{
public:
	Surface	(const std::vector<Polyline*> &ply_vec) : m_sfc_plys(ply_vec) {};
	virtual ~Surface () {};

	/** adds a new pointer to a polyline to the surface */
	void addPolyline (size_t pos) {
		assert (pos < m_sfc_plys.size());
		m_sfc_ply_ids.push_back (pos);
	}

	/** returns the number of polylines */
	size_t getSize () const { return m_sfc_ply_ids.size(); }

	/// returns the index of the i-th Polyline in the vector
	size_t getPolylineID(size_t i) const	{
		assert (i < m_sfc_ply_ids.size());
		return m_sfc_ply_ids[i];
	}

	/** \brief const access operator for the access to the i-th polyline of the surface.
	*/
	const Polyline* operator[] (size_t i) const {
		assert (i < m_sfc_ply_ids.size());
		return m_sfc_plys[m_sfc_ply_ids[i]];
	}
	/** \brief access operator (see book Effektiv C++ programmieren - subsection 1.3.2 ).
	* \sa const Point& operator[] (size_t idx) const
	*/
	Polyline* operator[] (size_t i) {
		assert (i < m_sfc_ply_ids.size());
		return m_sfc_plys[m_sfc_ply_ids[i]];
	}

protected:
	/** a vector of pointers to polylines */
	const std::vector<Polyline*> &m_sfc_plys;
	/** position of pointers to the geometric points */
	std::vector<size_t> m_sfc_ply_ids;
};

}

#endif /* SURFACE_H_ */

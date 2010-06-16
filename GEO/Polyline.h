/*
 * PolyLine.h
 *
 *  Created on: Jan 14, 2010
 *      Author: TF
 */

#ifndef POLYLINE_H_
#define POLYLINE_H_

// Math
#include "MathTools.h"

#include "Point.h"

#include <vector>
#include <cmath>

namespace GEOLIB {

/** \brief Class Polyline consists mainly of a reference to a point vector and
 * a vector that stores the indices in the point vector.
 * A polyline consists of at least one line segment. The polyline is specified by the endpoints
 * of the line segments. The class Polyline stores the position of pointers to the endpoints in the
 * m_ply_pnt_ids vector.
 * */
class Polyline {
public:
	/** constructor
	 * \param pnt_vec a reference to the point vector
	 */
	Polyline(const std::vector<Point*>& pnt_vec) :
		m_ply_pnts(pnt_vec)
	{}
	~Polyline()
	{}

	/** write the points to the stream */
	void write(std::ostream &os) const {
		size_t size(m_ply_pnt_ids.size());
		for (size_t k(0); k < size; k++) {
			os << *(m_ply_pnts[m_ply_pnt_ids[k]]) << std::endl;
		}
	}

	/** adds a new pointer to a point to the polyline */
	void addPoint(size_t pos) {
		assert(pos < m_ply_pnts.size());
		m_ply_pnt_ids.push_back(pos);
	}

	/**
	 * returns the number of points,
	 * the number of segments is about one smaller
	 * */
	size_t getSize() const {
		return m_ply_pnt_ids.size();
	}

	/** returns true if the polyline is closed */
	bool isClosed() const {
		if (m_ply_pnt_ids.front() == m_ply_pnt_ids.back())
			return true;
		else
			return false;
	}

	/**
	 * returns the index of the i-th polyline point
	 * in the point vector
	 */
	size_t getPointID(size_t i) const {
		assert(i < m_ply_pnt_ids.size());
		return m_ply_pnt_ids[i];
	}

	/** \brief const access operator for the access to the i-th point of the polyline.
	 */
	const Point* operator[](size_t i) const {
		assert(i < m_ply_pnt_ids.size());
		return m_ply_pnts[m_ply_pnt_ids[i]];
	}
	/** \brief access operator (see book Effektiv C++ programmieren - subsection 1.3.2 ).
	 * \sa const Point& operator[] (size_t idx) const
	 */
	Point* operator[](size_t i) {
		//return const_cast<Point*> (static_cast<const Polyline> (*this)[i]);	// LB: caused endless recursive function call on Windows
		assert(i < m_ply_pnt_ids.size());
		return m_ply_pnts[m_ply_pnt_ids[i]];
	}

	/**
	 * \brief returns the coordinates of the i-th point contained in the polyline
	 * */
	const Point* getPoint(size_t i) const {
		assert(i < m_ply_pnt_ids.size());
		return m_ply_pnts[m_ply_pnt_ids[i]];
	}

	/**
	 * \brief returns the coordinates of the i-th point contained in the polyline
	 * */
	Point* getPoint(size_t i) {
		assert(i < m_ply_pnt_ids.size());
		return m_ply_pnts[m_ply_pnt_ids[i]];
	}

	const std::vector<Point*> & getPointsVec () const { return m_ply_pnts; }

protected:
	/** a reference to the vector of pointers to the geometric points */
	const std::vector<Point*> &m_ply_pnts;
	/** position of pointers to the geometric points */
	std::vector<size_t> m_ply_pnt_ids;
};

/** overload the output operator for class Polyline */
//std::ostream& operator<< (std::ostream &os, const Polyline &pl)
//{
//	pl.write (os);
//	return os;
//}

} // end namespace

#endif /* POLYLINE_H_ */

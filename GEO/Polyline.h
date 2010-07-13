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
 * A polyline consists of at least one line segment. The polyline is specified by the points
 * of the line segments. The class Polyline stores the position of pointers to the points in the
 * m_ply_pnt_ids vector.
 * */
class Polyline {
public:
	/** constructor
	 * \param pnt_vec a reference to the point vector
	 */
	Polyline(const std::vector<Point*>& pnt_vec);
	/**
	 * Copy constructor
	 * @param ply Polyline
	 */
	Polyline (const Polyline& ply);

	virtual ~Polyline() {}

	/** write the points to the stream */
	void write(std::ostream &os) const;

	/** adds a new pointer to a point to the polyline */
	void addPoint(size_t pos);

	/**
	 * returns the number of points,
	 * the number of segments is about one smaller
	 * */
	size_t getSize() const;

	/** returns true if the polyline is closed */
	bool isClosed() const;

	/**
	 * returns the index of the i-th polyline point
	 * in the point vector
	 */
	size_t getPointID(size_t i) const;

	/** \brief const access operator for the access to the i-th point of the polyline.
	 */
	const Point* operator[](size_t i) const;

	/** \brief access operator (see book Effektiv C++ programmieren - subsection 1.3.2 ).
	 * \sa const Point& operator[] (size_t idx) const
	 */
//	Point* operator[](size_t i);

	/**
	 * \brief returns the coordinates of the i-th point contained in the polyline
	 * */
	const Point* getPoint(size_t i) const;

	const std::vector<Point*> & getPointsVec () const;

protected:
	/** a reference to the vector of pointers to the geometric points */
	const std::vector<Point*> &_ply_pnts;
	/** position of pointers to the geometric points */
	std::vector<size_t> _ply_pnt_ids;
};

/** overload the output operator for class Polyline */
std::ostream& operator<< (std::ostream &os, const Polyline &pl);

} // end namespace

#endif /* POLYLINE_H_ */

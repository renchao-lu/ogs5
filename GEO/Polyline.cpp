/*
 * Polyline.cpp
 *
 *  Created on: Jun 21, 2010
 *      Author: TF
 */

#include "Polyline.h"

namespace GEOLIB {

Polyline::Polyline(const std::vector<Point*>& pnt_vec) :
	_ply_pnts(pnt_vec)
{}

Polyline::Polyline(const Polyline& ply) :
	_ply_pnts (ply._ply_pnts)
{
	for (size_t k(0); k<ply.getSize(); ++k) {
		_ply_pnt_ids.push_back (ply.getPointID (k));
	}
}

void Polyline::write(std::ostream &os) const
{
	size_t size(_ply_pnt_ids.size());
	for (size_t k(0); k < size; k++) {
		os << *(_ply_pnts[_ply_pnt_ids[k]]) << std::endl;
	}
}

void Polyline::addPoint(size_t pos)
{
	assert(pos < _ply_pnts.size());
	_ply_pnt_ids.push_back(pos);
}

size_t Polyline::getSize() const
{
	return _ply_pnt_ids.size();
}

bool Polyline::isClosed() const
{
	if (_ply_pnt_ids.front() == _ply_pnt_ids.back())
		return true;
	else
		return false;
}

size_t Polyline::getPointID(size_t i) const
{
	assert(i < _ply_pnt_ids.size());
	return _ply_pnt_ids[i];
}

const Point* Polyline::operator[](size_t i) const
{
	assert(i < _ply_pnt_ids.size());
	return _ply_pnts[_ply_pnt_ids[i]];
}

const Point* Polyline::getPoint(size_t i) const
{
	assert(i < _ply_pnt_ids.size());
 	return _ply_pnts[_ply_pnt_ids[i]];
}

const std::vector<Point*> & Polyline::getPointsVec () const
{
	return _ply_pnts;
}

std::ostream& operator<< (std::ostream &os, const Polyline &pl)
{
	pl.write (os);
	return os;
}

} // end namespace GEOLIB

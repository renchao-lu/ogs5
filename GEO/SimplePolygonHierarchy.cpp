/*
 * SimplePolygonHierarchy.cpp
 *
 *  Created on: Jun 22, 2010
 *      Author: TF
 */

#include <SimplePolygonHierarchy.h>

namespace GEOLIB {

SimplePolygonHierarchy::SimplePolygonHierarchy(const Polygon* polygon) :
	_node (polygon)
{}

SimplePolygonHierarchy::~SimplePolygonHierarchy()
{}

const Polygon* SimplePolygonHierarchy::getPolygon () const
{
	return _node;
}

bool SimplePolygonHierarchy::isPolygonInside (const SimplePolygonHierarchy* polygon_hierarchy) const
{
	const Polygon* polygon (polygon_hierarchy->getPolygon());
	// check all points of polygon
	size_t n_pnts_polygon (polygon->getSize() - 1), cnt(0);
	for (size_t k(0); k<n_pnts_polygon && cnt == k; k++) {
		if (_node->isPntInPolygon (*(polygon->getPoint(k)))) {
//			std::cout << "point " << *(polygon->getPoint(k)) << " is in polygon " << std::endl;
			cnt++;
//		} else {
//			std::cout << "point " << *(polygon->getPoint(k)) << " is not in polygon " << std::endl;
		}
	}
	// all points of the given polygon are contained in the
	if (cnt == n_pnts_polygon) return true;
	else {
//		std::cerr << cnt << " from " << n_pnts_polygon << " points are in polygon" << std::endl;
		return false;
	}
}

void SimplePolygonHierarchy::insertSimplePolygonHierarchy (SimplePolygonHierarchy* polygon_hierarchy)
{
	const Polygon* polygon (polygon_hierarchy->getPolygon());
	std::cout << "inserting polygon " << *polygon << " in polygon " << *_node << std::endl;
	bool nfound (true);
	for (std::list<SimplePolygonHierarchy*>::const_iterator it (_childs.begin());
		it != _childs.end() && nfound; it++)
	{
		// check all points of polygon
		size_t n_pnts_polygon (polygon->getSize()), cnt(0);
		for (size_t k(0); k<n_pnts_polygon && cnt == k; k++) {
			if (((*it)->getPolygon())->isPntInPolygon (*(polygon->getPoint(k))))
				cnt++;
		}
		// all points of the given polygon are contained in the
		if (cnt == n_pnts_polygon) {
			(*it)->insertSimplePolygonHierarchy (polygon_hierarchy);
			nfound = false;
		}
	}
	if (nfound)
		_childs.push_back (polygon_hierarchy);
}

void createPolygonHierarchy (std::list<SimplePolygonHierarchy*>& list_of_simple_polygon_hierarchies)
{
	std::list<SimplePolygonHierarchy*>::iterator it0 (list_of_simple_polygon_hierarchies.begin()), it1;
	while (it0 != list_of_simple_polygon_hierarchies.end()) {
		it1 = it0;
		it1++;
		while (it1 != list_of_simple_polygon_hierarchies.end()) {
//			std::cout << "******* testing Polygon " << *((*it1)->getPolygon()) << " and Polygon " << *((*it0)->getPolygon()) << " ... " << std::endl;
//			if (0.19 <= (*((*it1)->getPolygon()->getPoint(0)))[0] && (*((*it1)->getPolygon()->getPoint(0)))[0] <= 0.21 &&
//					0.24 <= (*((*it1)->getPolygon()->getPoint(0)))[1] && (*((*it1)->getPolygon()->getPoint(0)))[1] <= 0.26)
//				std::cout << "relevant polygon" << std::endl;

			if ((*it0)->isPolygonInside (*it1)) {
				(*it0)->insertSimplePolygonHierarchy (*it1);
//				std::cout << "Polygon " << *((*it0)->getPolygon()) << " contains Polygon " << *((*it1)->getPolygon()) << std::endl;
				it1 = list_of_simple_polygon_hierarchies.erase (it1);
			} else {
				if ((*it1)->isPolygonInside (*it0)) {
					(*it1)->insertSimplePolygonHierarchy (*it0);
//					std::cout << "polygons contain each other" << std::endl;
					(*it1)->insertSimplePolygonHierarchy (*it0);
					it0 = list_of_simple_polygon_hierarchies.erase (it0);
//				} else {
//					std::cout << "polygons do not contain each other" << std::endl;
				}

				it1++;
			}
		}
		it0++;
	}
}

} // end namespace GEOLIB

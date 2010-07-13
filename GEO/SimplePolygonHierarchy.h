/*
 * SimplePolygonHierarchy.h
 *
 *  Created on: Jun 22, 2010
 *      Author: TF
 */

#ifndef SIMPLEPOLYGONHIERARCHY_H_
#define SIMPLEPOLYGONHIERARCHY_H_

#include "Polygon.h"

namespace GEOLIB {

class SimplePolygonHierarchy {
public:
	SimplePolygonHierarchy(const Polygon* polygon);
	virtual ~SimplePolygonHierarchy();

	const Polygon* getPolygon () const;

	bool isPolygonInside (const SimplePolygonHierarchy* polygon_hierarchy) const;
	void insertSimplePolygonHierarchy (SimplePolygonHierarchy* polygon_hierarchy);

private:
	const Polygon* _node;
	std::list<SimplePolygonHierarchy*> _childs;
};

void createPolygonHierarchy (std::list<SimplePolygonHierarchy*>& list_of_simple_polygon_hierarchies);

}

#endif /* SIMPLEPOLYGONHIERARCHY_H_ */

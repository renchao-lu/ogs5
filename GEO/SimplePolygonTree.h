/*
 * SimplePolygonTree.h
 *
 *  Created on: Jun 22, 2010
 *      Author: TF
 */

#ifndef SIMPLEPOLYGONTREE_H_
#define SIMPLEPOLYGONTREE_H_

#include "Polygon.h"
// FileIO
#include "GMSHInterface.h"

namespace GEOLIB {

class SimplePolygonTree {
public:
	SimplePolygonTree(const Polygon* polygon, SimplePolygonTree* parent = NULL);
	virtual ~SimplePolygonTree();

	const Polygon* getPolygon () const;

	bool isPolygonInside (const SimplePolygonTree* polygon_tree) const;
	void insertSimplePolygonTree (SimplePolygonTree* polygon_tree);

	void visitAndProcessNodes (FileIO::GMSHInterface& gmsh_io);

private:
	size_t getLevel () const;
	void _visitAndProcessNodes (FileIO::GMSHInterface& gmsh_io);
	const Polygon* _node;
	SimplePolygonTree* _parent;
	std::list<SimplePolygonTree*> _childs;
};

void createPolygonTree (std::list<SimplePolygonTree*>& list_of_simple_polygon_trees);

}

#endif /* SIMPLEPOLYGONTREE_H_ */

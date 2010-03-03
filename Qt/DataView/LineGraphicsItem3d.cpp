/**
 * \file LineGraphicsItem3d.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of LineGraphicsItem3d
 */

// ** INCLUDES **
#include "LineGraphicsItem3d.h"

#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGLineChunk.h>

OSG_USING_NAMESPACE

LineGraphicsItem3d::LineGraphicsItem3d( GEOLIB::Polyline* const line, QObject* parent /*= 0*/ )
: GraphicsItem3d(parent), _line(line)
{
	_itemType = LINE_ITEM;

	computePositions();

	GeoPTypesPtr type = GeoPTypesUI8::create();
	beginEditCP(type, GeoPTypesUI8::GeoPropDataFieldMask);
	type->addValue(GL_LINE_STRIP);
	endEditCP(type, GeoPTypesUI8::GeoPropDataFieldMask);

	GeoPLengthsPtr length = GeoPLengthsUI32::create();
	beginEditCP(length, GeoPLengthsUI32::GeoPropDataFieldMask);
	length->addValue(_toPntVecs.size());
	endEditCP(length, GeoPLengthsUI32::GeoPropDataFieldMask);

	_pos = GeoPositions3f::create();
	beginEditCP(_pos, GeoPositions3f::GeoPropDataFieldMask);
	foreach (Vec3f vec, _toPntVecs)
		_pos->addValue(vec);
	endEditCP(_pos, GeoPositions3f::GeoPropDataFieldMask);

	// 	GeoColors3fPtr colors = GeoColors3f::create();
	// 	beginEditCP(colors, GeoColors3f::GeoPropDataFieldMask);
	// 	colors->addValue(Color3f(1,0,0));
	// 	endEditCP(colors, GeoColors3f::GeoPropDataFieldMask);

	LineChunkPtr lineChunk = LineChunk::create();
	beginEditCP(lineChunk);
	lineChunk->setWidth(1.0);
	endEditCP(lineChunk);

	_mat = SimpleMaterial::create();
	beginEditCP(_mat);
	_mat->setDiffuse(colorNormal());
	_mat->setLit(false);
	_mat->addChunk(lineChunk);
	endEditCP(_mat);

	// No indices on GL_LINE_STRIP

	GeometryPtr geo = Geometry::create();
	beginEditCP(geo,
		Geometry::TypesFieldMask    |
		Geometry::LengthsFieldMask  |
		Geometry::PositionsFieldMask    |
		//Geometry::NormalsFieldMask      |
		//Geometry::ColorsFieldMask   |
		Geometry::MaterialFieldMask
		);

	geo->setTypes(type);
	geo->setLengths(length);
	geo->setPositions(_pos);
	//geo->setColors(colors);
	geo->setMaterial(_mat);

	endEditCP(geo,
		Geometry::TypesFieldMask    |
		Geometry::LengthsFieldMask  |
		Geometry::PositionsFieldMask    |
		//Geometry::NormalsFieldMask  |
		//Geometry::ColorsFieldMask   |
		Geometry::MaterialFieldMask
		);

	_osgNode = Node::create();
	_transform = Transform::create();
	setPosition(_centerPnt);
	NodePtr geoNode = Node::create();
	beginEditCP(geoNode);
	geoNode->setCore(geo);
	endEditCP(geoNode);
	beginEditCP(_osgNode);
	_osgNode->setCore(_transform);
	_osgNode->addChild(geoNode);
	endEditCP(_osgNode);

	show();
}

void LineGraphicsItem3d::updatePosition()
{
	computePositions();
	beginEditCP(_pos, GeoPositions3f::GeoPropDataFieldMask);
	int index = 0;
	foreach (Vec3f vec, _toPntVecs)
	{
		_pos->setValue(vec, index);
		++index;
	}
	endEditCP(_pos, GeoPositions3f::GeoPropDataFieldMask);
	setPosition(_centerPnt);
}


void LineGraphicsItem3d::computePositions()
{
	Vec3f sumUpVec(0, 0, 0);

	size_t numPoints = _line->getSize();
	for (int i = 0; i < numPoints; i++)
	{
		const double * coords = (*_line)[i]->getData();
		sumUpVec += Vec3f(coords[0], coords[1], coords[2]);
	}

	// ignore last point for closed polylines
	const double * coordsFirst = (*_line)[0]->getData();
	const double * coordsLast = (*_line)[numPoints-1]->getData();
	if (coordsFirst[0] == coordsLast[0] &&
		coordsFirst[1] == coordsLast[1] &&
		coordsFirst[2] == coordsLast[2])
		_centerPnt = Pnt3f(sumUpVec * (1.0 / (numPoints - 1)));
	else
		_centerPnt = Pnt3f(sumUpVec * (1.0 / (numPoints)));

	_toPntVecs.clear();
	for (int i = 0; i < numPoints; i++)
	{
		const double * coords = (*_line)[i]->getData();
		Vec3f vec = Vec3f(coords[0], coords[1], coords[2]) - Vec3f(_centerPnt);
		_toPntVecs.push_back(vec);
	}
}

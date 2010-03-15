/**
 * \file LineGraphicsItem2d.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of LineGraphicsItem2d
 */

// ** INCLUDES **
#include "LineGraphicsItem2d.h"
#include "Polyline.h"

#include <QPainter>
#include <QStyleOption>
#include <QRectF>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

LineGraphicsItem2d::LineGraphicsItem2d( Model* model, GEOLIB::Polyline* line,
									   EViewPlane viewplane, QGraphicsItem* parent)
: GraphicsItem2d(model, viewplane, 1, LINE_ITEM, parent), _line(line)
{
	updatePosition();

	_pen = colorNormal();
	//setViewItemScaling(VIEWITEMSCALING_NORMAL);
	//setFlag(ItemIgnoresTransformations);
}

LineGraphicsItem2d::~LineGraphicsItem2d()
{
	//delete _line;
}
QRectF LineGraphicsItem2d::boundingRect() const
{
	switch (_viewplane)
	{
	case VIEWPLANE_XY: return QRectF(QPointF(_minimumPntRelative.X(), _maximumPntRelative.Y()), QPointF(_maximumPntRelative.X(), _minimumPntRelative.Y()));
	case VIEWPLANE_XZ: return QRectF(QPointF(_minimumPntRelative.X(), _minimumPntRelative.Z()), QPointF(_maximumPntRelative.X(), _maximumPntRelative.Z()));
	case VIEWPLANE_YZ: return QRectF(QPointF(_minimumPntRelative.Y(), _minimumPntRelative.Z()), QPointF(_maximumPntRelative.Y(), _maximumPntRelative.Z()));
	default: return QRectF(0, 0, 0, 0);
	}
	return QRectF();
}


void LineGraphicsItem2d::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{

}
void LineGraphicsItem2d::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(widget)

	QPen pen = QPen(_pen);
	//pen.setWidth(0.1);
	painter->setPen(pen);

	if (option->state & QStyle::State_Selected)
	{
		painter->setPen(QPen(colorSelected()));
		painter->setBrush(QBrush(colorSelected()));
	}
	else
		painter->setBrush(QBrush(_pen));

	for (std::vector<GEOLIB::Point>::const_iterator it = _pntsRelative.begin();
		it != _pntsRelative.begin() + _pntsRelative.size() - 1; ++it)
	{
		const double * coords1 = it->getData();
		const double * coords2 = (it+1)->getData();
		switch (_viewplane)
		{
		case VIEWPLANE_XY:
			painter->drawLine(QPointF(coords1[0], coords1[1]), QPointF(coords2[0], coords2[1]));
			break;
		case VIEWPLANE_XZ:
			painter->drawLine(QPointF(coords1[0], coords1[2]), QPointF(coords2[0], coords2[2]));
			break;
		case VIEWPLANE_YZ:
			painter->drawLine(QPointF(coords1[1], coords1[2]), QPointF(coords2[1], coords2[2]));
			break;
		default:
			break;
		}
	}

	GraphicsItem2d::paint(painter, option, widget);
}

GEOLIB::Polyline* LineGraphicsItem2d::line() const
{
	return _line;
}

void LineGraphicsItem2d::updatePosition()
{
	computeRelativePositions();
	switch (_viewplane)
	{
	case VIEWPLANE_XY: setPos(_centerPnt.X(), _centerPnt.Y()); break;
	case VIEWPLANE_XZ: setPos(_centerPnt.X(), _centerPnt.Z()); break;
	case VIEWPLANE_YZ: setPos(_centerPnt.Y(), _centerPnt.Z()); break;
	default:
		break;
	}

}

QVariant LineGraphicsItem2d::itemChange(GraphicsItemChange change, const QVariant& value)
{
	//updatePosition();

	// TODO
// 	switch (change)
// 	{
// 	case ItemPositionHasChanged:
// 		QPointF newPosPnt = value.toPointF();
// 		GEOLIB::Vector newPos;
// 		GEOLIB::Vector oldPos = _line->centerPoint();
// 		switch (_viewplane)
// 		{
// 		case VIEWPLANE_XY:
// 			newPos = GEOLIB::Vector(newPosPnt.x(), newPosPnt.y(), 0.0);
// 			oldPos.z = 0.0; break;
// 		case VIEWPLANE_XZ:
// 			newPos = GEOLIB::Vector(newPosPnt.x(), 0.0, newPosPnt.y());
// 			oldPos.y = 0.0; break;
// 		case VIEWPLANE_YZ:
// 			newPos = GEOLIB::Vector(0.0, newPosPnt.x(), newPosPnt.y());
// 			oldPos.x = 0.0; break;
// 		default:
// 			break;
// 		}
// 		GEOLIB::Vector offset = newPos - oldPos;
// 		_line->translate(offset);
// 		break;
// 	}

	emit itemPositionChanged(_line);
	return  QGraphicsItem::itemChange(change, value);
}

void LineGraphicsItem2d::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
 	Q_UNUSED(event)
 	_pen = colorHover();
 	update(QRectF());
}

void LineGraphicsItem2d::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
 	Q_UNUSED(event)
 		_pen = colorNormal();
 	update(QRectF());
}

const QColor LineGraphicsItem2d::colorNormal()
{
	return _colorNormal;
}

const QColor LineGraphicsItem2d::colorHover()
{
	return _colorHover;
}
const QColor LineGraphicsItem2d::colorSelected()
{
	return _colorSelected;
}

void LineGraphicsItem2d::computeRelativePositions()
{
	_minimumPnt = MATHLIB::Vector(*(*_line)[0]);
	_maximumPnt = MATHLIB::Vector(*(*_line)[0]);

	MATHLIB::Vector sumUpVec;

	size_t numPoints = _line->getSize();
	if (numPoints > 100)
		std::cerr << "numPoints > 100" << std::endl;

	for (size_t i = 0; i < numPoints; i++)
	{
		const double * coords = (*_line)[i]->getData();
		sumUpVec += MATHLIB::Vector(coords[0], coords[1], coords[2]);

		if (coords[0] < _minimumPnt.X()) _minimumPnt.setX(coords[0]);
		if (coords[0] > _maximumPnt.X()) _maximumPnt.setX(coords[0]);
		if (coords[1] < _minimumPnt.Y()) _minimumPnt.setY(coords[1]);
		if (coords[1] > _maximumPnt.Y()) _maximumPnt.setY(coords[1]);
		if (coords[2] < _minimumPnt.Z()) _minimumPnt.setZ(coords[2]);
		if (coords[2] > _maximumPnt.Z()) _maximumPnt.setZ(coords[2]);
	}

	// ignore last point for closed polylines
	if ((*_line)[0] == (*_line)[numPoints-1])
		_centerPnt = sumUpVec * (1.0 / (numPoints - 1));
	else
		_centerPnt = sumUpVec * (1.0 / (numPoints));

	_minimumPntRelative = -(_centerPnt - _minimumPnt);
	_maximumPntRelative = _maximumPnt - _centerPnt;

	_pntsRelative.clear();
	for (size_t i = 0; i < numPoints; i++)
	{
		MATHLIB::Vector p = MATHLIB::Vector(*(*_line)[i]) - _centerPnt;;
		_pntsRelative.push_back(p);
	}
}
void LineGraphicsItem2d::setEditable( bool enable )
{
	setAcceptHoverEvents(enable);
	setFlag(ItemIsMovable, enable);
	setFlag(ItemIsSelectable, enable);
}

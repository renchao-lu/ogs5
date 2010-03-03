/**
 * \file PntGraphicsItem2d.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of PntGraphicsItem2d
 */

// ** INCLUDES **
#include "PntGraphicsItem2d.h"
#include "Color.h"
#include "DetailWindow.h"

#include <QPainter>
#include <QStyleOption>
#include <QRectF>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

#include <iostream>

using namespace std;

PntGraphicsItem2d::PntGraphicsItem2d(
	Model* model, GEOLIB::Point *point, GEOLIB::Color* color,
	double renderSize /*= 1.0*/, EItemType type /*= PNT_ITEM*/,
	EViewPlane viewplane /*= VIEWPLANE_XY*/, QGraphicsItem* parent /*= 0*/)
	: GraphicsItem2d(model, viewplane, renderSize, type, parent),
	_point(point), _color(color)
{
	if (!_color) {
		if (dynamic_cast<GEOLIB::Station*> (point)) {
			_color = (dynamic_cast<GEOLIB::Station*>(point))->getColor();
		} else {
			_color = new GEOLIB::Color(0,0,0);
		}
	}
	updatePosition();
	setFlag(ItemIgnoresTransformations);
	_pen = colorNormal();
}

PntGraphicsItem2d::~PntGraphicsItem2d()
{
}

QRectF PntGraphicsItem2d::boundingRect() const
{
	return QRectF(-_renderSize*2, -_renderSize*2,
		_renderSize * 2*2, _renderSize * 2*2);
}

QVariant PntGraphicsItem2d::itemChange( GraphicsItemChange change, const QVariant & value )
{
	switch (change)
	{
	case ItemPositionHasChanged:
		QPointF pos = QGraphicsItem::pos();
		switch (_viewplane)
		{
			// TODO
		case VIEWPLANE_XY:
			//_point->setX(pos.x());
			//_point->setY(pos.y());
			break;
		case VIEWPLANE_XZ:
			//_point->setX(pos.x());
			//_point->setZ(pos.y());
			break;
		case VIEWPLANE_YZ:
			//_point->setY(pos.x());
			//_point->setZ(pos.y());
			break;
		}
		break;
	}

	emit itemPositionChanged(_point);
	return QGraphicsItem::itemChange(change, value);
}


void PntGraphicsItem2d::paint( QPainter *painter,
								 const QStyleOptionGraphicsItem *option,
								 QWidget *widget /* = 0 */ )
{
	Q_UNUSED(widget)

	painter->setPen(QPen(_pen));

	if (option->state & QStyle::State_Selected)
	{
		painter->setPen(QPen(colorSelected()));
		painter->setBrush(QBrush(colorSelected()));
	}
	else
		painter->setBrush(QBrush(_pen));

	painter->drawRect(QRectF(-_renderSize, -_renderSize,
		_renderSize * 2, _renderSize * 2));

	GraphicsItem2d::paint(painter, option, widget);
}


GEOLIB::Point* PntGraphicsItem2d::point() const
{
	return _point;
}

void PntGraphicsItem2d::updatePosition()
{
	const double* coords = _point->getData();
	switch (_viewplane)
	{
	case VIEWPLANE_XY:
		setPos(coords[0], coords[1]); break;
	case VIEWPLANE_XZ:
		setPos(coords[0], coords[2]); break;
	case VIEWPLANE_YZ:
		setPos(coords[1], coords[2]); break;
	}
}

void PntGraphicsItem2d::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event)
	_pen = colorHover();
	update(boundingRect());
}

void PntGraphicsItem2d::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
	Q_UNUSED(event)
	_pen = colorNormal();
	update(boundingRect());
}

void PntGraphicsItem2d::contextMenuEvent( QGraphicsSceneContextMenuEvent *event )
{
	QMenu menu;
	QAction* showInfoAction    = menu.addAction("View Information...");
	QAction* showDiagramAction = menu.addAction("View Diagram...");
	connect(showDiagramAction, SIGNAL(triggered()), this, SLOT(viewDiagram()));
	QAction* selectedAction    = menu.exec(event->screenPos());
}

void PntGraphicsItem2d::updatePositionFromOutside()
{
	blockSignals(true);
	updatePosition();
	blockSignals(false);
}

const QColor PntGraphicsItem2d::colorNormal()
{
	if ((*_color)[0] == 0 && (*_color)[1] == 0 && (*_color)[2] == 0)
		return _colorNormal;
	else
		return QColor((*_color)[0], (*_color)[1], (*_color)[2]);
}

const QColor PntGraphicsItem2d::colorHover()
{
	return _colorHover;
}

const QColor PntGraphicsItem2d::colorSelected()
{
	return _colorSelected;
}

void PntGraphicsItem2d::setEditable( bool enable )
{
	setAcceptHoverEvents(enable);
	setFlag(ItemIsSelectable, enable);
	if (_type == PNT_ITEM && _type == MSH_NODE_ITEM)
		setFlag(ItemIsMovable, enable);
}

void PntGraphicsItem2d::viewDiagram()
{
	emit diagramRequested(_modelIndex);
}

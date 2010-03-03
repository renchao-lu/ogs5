/**
 * \file GraphicsItem2d.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of GraphicsItem2d
 */

// ** INCLUDES **
#include "GraphicsItem2d.h"
#include <qpainter.h>

const QColor GraphicsItem2d::_colorNormal = QColor(180, 180, 180);
const QColor GraphicsItem2d::_colorHover = QColor(255, 128, 0);
const QColor GraphicsItem2d::_colorSelected= QColor(255, 0, 0);


GraphicsItem2d::GraphicsItem2d( Model* model, EViewPlane viewplane,
	double renderSize /*= 1.0*/, EItemType type /*= EMPTY_ITEM*/,
	QGraphicsItem* parent /*= 0*/, QObject* parentObject /*= 0*/ )
: QObject(parentObject), QGraphicsItem(parent),
 _model(_model),
 _renderSize(renderSize),
 _type(type),
 _viewplane(viewplane),
 _viewItemScaling(VIEWITEMSCALING_NONE),
 _drawBoundingBox(false)
{
}

EViewPlane GraphicsItem2d::viewplane() const
{
	return _viewplane;
}

void GraphicsItem2d::setViewplane( EViewPlane viewplane )
{
	_viewplane = viewplane;
	updatePosition();
}

double GraphicsItem2d::renderSize() const
{
	return _renderSize;
}

void GraphicsItem2d::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	update();
	QGraphicsItem::mousePressEvent(event);
}

void GraphicsItem2d::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	update();
	QGraphicsItem::mouseReleaseEvent(event);
}

QVariant GraphicsItem2d::itemChange( GraphicsItemChange change, const QVariant & value )
{
	return QGraphicsItem::itemChange(change, value);
}

EViewItemScaling GraphicsItem2d::viewItemScaling() const
{
	return _viewItemScaling;
}

void GraphicsItem2d::setViewItemScaling( EViewItemScaling scaling )
{
	_viewItemScaling = scaling;
}

void GraphicsItem2d::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= 0*/ )
{
	if (_drawBoundingBox)
	{
		painter->setPen(QColor("red"));
		painter->setBrush(Qt::NoBrush);
		painter->drawRect(boundingRect());
	}
}

QModelIndex GraphicsItem2d::modelIndex() const
{
	return _modelIndex;
}

Model* GraphicsItem2d::model() const
{
	return _model;
}

void GraphicsItem2d::setModelIndex( QModelIndex index )
{
	_modelIndex = index;
}

void GraphicsItem2d::setRenderSize( double size )
{
	_renderSize = size;
}

EItemType GraphicsItem2d::itemType() const
{
	return _type;
}

/**
 * \file CoordinateOriginGraphicsItem2d.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of CoordinateOriginGraphicsItem2d
 */

// ** INCLUDES **
#include "CoordinateOriginGraphicsItem2d.h"

#include <QPainter>

CoordinateOriginGraphicsItem2d::CoordinateOriginGraphicsItem2d( EViewPlane viewplane /*= VIEWPLANE_XY*/, QGraphicsItem* parent /*= 0*/ )
	: GraphicsItem2d(NULL, viewplane, 1.0, INVALID_ITEM, parent)
{
	updatePosition();
	setFlag(ItemIgnoresTransformations);
}

QRectF CoordinateOriginGraphicsItem2d::boundingRect() const
{
	return QRectF(0, 0, _renderSize, _renderSize);
}

void CoordinateOriginGraphicsItem2d::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */ )
{
	Q_UNUSED(widget)
	Q_UNUSED(option)

	//double lineScaling = _renderSize / _viewScaling;
	//if (lineScaling < 1.0)
	//	lineScaling = 1.0;
	QLineF xLine(0.0, 0.0, _renderSize * 50, 0.0);
	QLineF yLine(0.0, 0.0, 0.0, _renderSize * 50);

	QMatrix negateY(1, 0, 0, -1, 0, 0);
	QTransform transform(negateY);

	if (_viewplane == VIEWPLANE_XY)
	{
		painter->setPen(QPen(Qt::red));
		painter->setBrush(QBrush(Qt::red));
		painter->drawLine(xLine);
		painter->setFont(QFont("Arial", 12));
		
		//painter->setTransform(transform, true);
		painter->drawText(_renderSize * 55.5, -1.0, "X");
		
		painter->setTransform(transform, true);
		painter->setPen(QPen(Qt::green));
		painter->setBrush(QBrush(Qt::green));
		painter->drawLine(yLine);
		painter->setFont(QFont("Arial", 12));
		painter->setTransform(transform, true);
		
		painter->drawText(0.0, -_renderSize * 55.5, "Y");
		//painter->setTransform(transform, true);
	}
	else if (_viewplane == VIEWPLANE_XZ)
	{
		painter->setPen(QPen(Qt::red));
		painter->setBrush(QBrush(Qt::red));
		painter->drawLine(xLine);
		painter->setFont(QFont("Arial", 12));

		//painter->setTransform(transform, true);
		painter->drawText(_renderSize * 55.5, -1.0, "X");
		
		
		painter->setTransform(transform, true);
		painter->setPen(QPen(Qt::blue));
		painter->setBrush(QBrush(Qt::blue));
		painter->drawLine(yLine);
		painter->setFont(QFont("Arial", 12));
		painter->setTransform(transform, true);

		painter->drawText(0.0, -_renderSize * 55.5, "Z");
		//painter->setTransform(transform, true);
	}
	else if (_viewplane == VIEWPLANE_YZ)
	{
		painter->setPen(QPen(Qt::green));
		painter->setBrush(QBrush(Qt::green));
		painter->drawLine(xLine);
		painter->setFont(QFont("Arial", 12));

		//painter->setTransform(transform, true);
		painter->drawText(_renderSize * 55.5, -1.0, "Y");
		
		painter->setTransform(transform, true);
		painter->setPen(QPen(Qt::blue));
		painter->setBrush(QBrush(Qt::blue));
		painter->drawLine(yLine);
		painter->setFont(QFont("Arial", 12));
		painter->setTransform(transform, true);

		painter->drawText(0.0, -_renderSize * 55.5, "Z");
		//painter->setTransform(transform, true);
	}
}

void CoordinateOriginGraphicsItem2d::updatePosition()
{
	setPos(0.0, 0.0);
}

/**
 * \file CoordinateOriginGraphicsItem2d.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef COORDINATEORIGINGRAPHICSITEM2D_H
#define COORDINATEORIGINGRAPHICSITEM2D_H

// ** INCLUDES **
#include "GraphicsItem2d.h"
#include "Enums.h"

/**
 * The CoordinateOriginGraphicsItem2d is a 2d graphics item that renders
 * the coordinate system.
 */
class CoordinateOriginGraphicsItem2d : public GraphicsItem2d
{
	Q_OBJECT

public:
	
	/**
	 * Constructor.
	 * \param viewplane The initial viewplane
	 * \param parent The parent graphics item
	 */
	CoordinateOriginGraphicsItem2d(EViewPlane viewplane = VIEWPLANE_XY, QGraphicsItem* parent = 0);
	
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget /* = 0 */);

protected:
	//QVariant itemChange( GraphicsItemChange change, const QVariant & value);

private:
	void updatePosition();

};

#endif // COORDINATEORIGINGRAPHICSITEM2D_H

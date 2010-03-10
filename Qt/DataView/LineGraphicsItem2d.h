/**
 * \file LineGraphicsItem2d.h
 * 24/9/2009 LB Initial implementation
 */

#ifndef LINEGRAPHICSITEM2D_H
#define LINEGRAPHICSITEM2D_H

#include "Enums.h"
#include "GraphicsItem2d.h"
#include "Polyline.h"
#include "Vector3.h"

/**
 *	The LineGraphicsItem2d renders a CGLPolyline on a graphics view.
 */
class LineGraphicsItem2d : public GraphicsItem2d
{
	Q_OBJECT

public:
	/**
	 * Constructor.
	 * \param model The Qt model
	 * \param line The  polyline
	 * \param viewplane The initial viewplane
	 * \param parent The parent graphics item
	 * \return
	 */
	LineGraphicsItem2d(Model* model, GEOLIB::Polyline* line,
		EViewPlane viewplane = VIEWPLANE_XY, QGraphicsItem* parent = 0);
	~LineGraphicsItem2d();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	GEOLIB::Polyline* line() const;

	void setEditable(bool enable);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value);
	void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
	static const QColor colorNormal();
	static const QColor colorHover();
	static const QColor colorSelected();

	void computeRelativePositions();

private:
	GEOLIB::Polyline* _line;
	QColor _pen;
	std::vector<GEOLIB::Point> _pntsRelative;
	MATHLIB::Vector _centerPnt, _minimumPnt, _maximumPnt, _minimumPntRelative, _maximumPntRelative;

public slots:
	void updatePosition();

signals:
	void itemPositionChanged(GEOLIB::Polyline* line);

};
#endif // LINEGRAPHICSITEM2D_H

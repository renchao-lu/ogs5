/**
 * \file PntGraphicsItem2d.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef PNTGRAPHICSITEM2D_H
#define PNTGRAPHICSITEM2D_H

// ** INCLUDES **
#include "Enums.h"
#include "GraphicsItem2d.h"
#include "Station.h"
#include "Color.h"
#include "Point.h"

#include <QObject>

/**
 * The PntGraphicsItem2d renders a Point or a Mesh_Group::CNode
 * in a 2d graphics view.
 */
class PntGraphicsItem2d : public GraphicsItem2d
{
	Q_OBJECT

public:

	//PntGraphicsItem2d(QGraphicsItem* parent = 0);
	/**
	 * Constructor. Specify a point OR a node object!
	 * \param model The Qt model
	 * \param point The  point
	 * \param node The Mesh_Group::CNode
	 * \param renderSize The rendered size in pixel
	 * \param viewplane The initial viewplane
	 * \param parent The parent graphics item
	 */
	PntGraphicsItem2d(Model* model, GEOLIB::Point *point, GEOLIB::Color* color = NULL,
		double renderSize = 1.0, EItemType type = PNT_ITEM,
		EViewPlane viewplane = VIEWPLANE_XY, QGraphicsItem* parent = 0);

	~PntGraphicsItem2d();

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	GEOLIB::Point* point() const;

	void setEditable(bool enable);

public slots:
	void viewDiagram();
	/// Same as updatePosition() except that all signals are blocked.
	void updatePositionFromOutside();

	/// Updates the item position to the referenced CGLPoint position.
	void updatePosition();
protected:
	/// Updates the referenced CGLPoint position to the items position.
	QVariant itemChange( GraphicsItemChange change, const QVariant & value );
	void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	/// Creates a context menu for the item and connects the menu items with the respective functions
	void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
	const QColor colorNormal();
	static const QColor colorHover();
	static const QColor colorSelected();

private:
	GEOLIB::Point* _point;
//	boost::shared_ptr<GEOLIB::Color> _color;
	GEOLIB::Color* _color;
	QColor _pen;

signals:
	void itemPositionChanged(GEOLIB::Point* point);
	void diagramRequested(QModelIndex&);

};

#endif // PNTGRAPHICSITEM2D_H

/**
 * \file GraphicsItem2d.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef GRAPHICSITEM2D_H
#define GRAPHICSITEM2D_H

// ** INCLUDES **
#include "Enums.h"
#include <QGraphicsItem>
#include <QModelIndex>

class Model;

/**
 * The GraphicsItem2d is the base graphics item for the 2d view. An item
 * can be part of Qts Model/View-Framework and then holds references through
 * the model (_model) and the model-item-index (_modelIndex).
 */
class GraphicsItem2d : public QObject, public QGraphicsItem
{
	Q_OBJECT

public:

	/**
	 * Constructor.
	 * \param model The Qt model
	 * \param viewplane The initial viewplane
	 * \param renderSize Subclasses may use this in their paint() method
	 * \param parent The parent graphics item
	 */
	GraphicsItem2d(Model* model, EViewPlane viewplane = VIEWPLANE_XY,
		double renderSize = 1.0, EItemType type = INVALID_ITEM,
		QGraphicsItem* parent = 0, QObject* parentObject = 0);

	/// Virtual empty destructor because this is a polymorphic base class
	virtual ~GraphicsItem2d() {}

	/// Returns the bounding rectangle
	virtual QRectF boundingRect() const = 0;

	/// Returns the viewplane, see EViewPlane
	EViewPlane viewplane() const;

	/// Sets the viewplane, see EViewPlane
	void setViewplane(EViewPlane viewplane);

	/// Returns the render size, this may be used when painting the object
	double renderSize() const;

	/// Sets the render size
	void setRenderSize(double size);

	/// Returns the view scaling mode, see EViewItemScaling
	EViewItemScaling viewItemScaling() const;

	/// Sets the view scaling mode, see EViewItemScaling
	void setViewItemScaling(EViewItemScaling scaling);

	/// Returns the associated QModelIndex which belongs to a Qt model
	QModelIndex modelIndex() const;

	/// Sets the model index
	void setModelIndex(QModelIndex index);

	/// Returns the associated model
	Model* model() const;

	/// Paints the object
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget = 0);

	/// Returns the item type
	EItemType itemType() const;

	virtual void setEditable(bool enable) = 0;

protected:
	EViewPlane	_viewplane;
	double		_renderSize;
	EViewItemScaling _viewItemScaling;
	QModelIndex _modelIndex;
	Model* _model;
	bool _drawBoundingBox;
	EItemType _type;

	/// Should be called when the items underlying data object was modified
	virtual void updatePosition() = 0;

	/// Is called when the QGraphicsItem was modified
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

	static const QColor _colorNormal;// = QColor(0, 0, 0);
	static const QColor _colorHover;//(255, 0, 0);
	static const QColor _colorSelected;//(255, 128, 0);

private:

};

#endif // GRAPHICSITEM2D_H

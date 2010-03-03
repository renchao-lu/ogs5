/**
 * \file GraphicsView2d.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of GraphicsView2d
 */

// ** INCLUDES **
#include "GraphicsView2d.h"
#include "PntGraphicsItem2d.h"
#include "GraphicsScene.h"

#include <QGLWidget>
#include <QWheelEvent>
#include <math.h>
#include <iostream>

GraphicsView2d::GraphicsView2d(QWidget *parent)
	: QGraphicsView(parent), _graphicsScene(NULL)
{
    setSceneRect(QRectF());
	//_graphicsScene->setSceneRect(-5, -5, 10, 10);
	//setScene(_graphicsScene);
	//setViewportUpdateMode(FullViewportUpdate);
	setViewportUpdateMode(BoundingRectViewportUpdate);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);
	setResizeAnchor(AnchorViewCenter);

	this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers), this));

	QTransform viewTransform = viewportTransform();
        QMatrix m = viewTransform.toAffine();
	QMatrix negateY;
	negateY.setMatrix(1, 0, 0, -1, 0, 0);
        setMatrix(m * negateY);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
}

void GraphicsView2d::wheelEvent( QWheelEvent *event )
{
	scaleView(pow((double)2, -event->delta() / 240.0));
}
void GraphicsView2d::scaleView( qreal scaleFactor )
{
	qreal factor = matrix().scale(scaleFactor, scaleFactor)
		       .mapRect(QRectF(0, 0, 1, 1)).width();
	//if (factor < 0.001)
	//	return;

	scale(scaleFactor, scaleFactor);

	updateView();
}

GraphicsScene* GraphicsView2d::scene() const
{
	return _graphicsScene;
}

void GraphicsView2d::updateView()
{
	// graphic items have always the same size in the view
	// inverse the 
	double factor = transform().mapRect(QRectF(0, 0, 1, 1)).width();
	QList<QGraphicsItem*> items = _graphicsScene->items();
	QMatrix invertedScaling;
	invertedScaling.scale(1.0 / factor, 1.0 / factor);
// 	for (QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it)
// 	{
// 		2DGraphicsItem* ogsItem = dynamic_cast<2DGraphicsItem *>(*it);
// 		if (ogsItem)
// 			if (ogsItem->viewItemScaling() == VIEWITEMSCALING_NONE)
// 				ogsItem->setTransform(QTransform(invertedScaling));
// 	}

	// scale scenes background
	if (factor <= 16.0)
		_graphicsScene->setBackgroundGridSize(5);
	if (factor <= 4.0)
		_graphicsScene->setBackgroundGridSize(25);
	if (factor <= 1.0)
		_graphicsScene->setBackgroundGridSize(125);
	if (factor <= 0.25)
		_graphicsScene->setBackgroundGridSize(500);
	if (factor > 16.0)
		_graphicsScene->setBackgroundGridSize(1);
}

void GraphicsView2d::resizeEvent( QResizeEvent *event )
{
	Q_UNUSED(event)

	fitInView(_graphicsScene->sceneRect(), Qt::KeepAspectRatio);
	updateView();
}

void GraphicsView2d::keyPressEvent( QKeyEvent *event )
{
	switch (event->key())
	{
	case Qt::Key_Shift:
		event->accept();
		setDragMode(ScrollHandDrag);
		_graphicsScene->clearFocus();
		break;
	case Qt::Key_Alt:
		event->accept();
		setDragMode(RubberBandDrag);
		break;
	}
}

void GraphicsView2d::keyReleaseEvent( QKeyEvent *event )
{
	switch (event->key())
	{
	case Qt::Key_Shift:
		event->accept();
		setDragMode(NoDrag);
		_graphicsScene->setFocus();
		break;
	case Qt::Key_Alt:
		event->accept();
		setDragMode(NoDrag);
		break;
	}
}

void GraphicsView2d::setScene( GraphicsScene* scene )
{
	_graphicsScene = scene;
	//connect(_graphicsScene, SIGNAL(changed(QList<QRectF>)),
	//		 this, SLOT(updateView()));
	//connect(_graphicsScene, SIGNAL(selectionChanged()),
	//	this, SLOT(updateView()));

	QGraphicsView::setScene(scene);
	setSceneRect(QRectF());
}

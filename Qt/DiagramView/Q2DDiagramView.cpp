/**
 * \file Q2DDiagramView.cpp
 * KR Initial implementation
 */

#include <math.h>
#include "Q2DDiagramView.h"
#include <QGraphicsTextItem>


Q2DDiagramView::Q2DDiagramView(QWidget* parent) : QGraphicsView(parent)
{
	_scene = new Q2DDiagramScene();
	setScene(_scene);
	initialize();
}

Q2DDiagramView::Q2DDiagramView(QDiagramList* list, QWidget* parent) : QGraphicsView(parent)
{
	_scene = new Q2DDiagramScene(list);
	setScene(_scene);
	initialize();
}

Q2DDiagramView::~Q2DDiagramView()
{
}

void Q2DDiagramView::addGraph(QDiagramList* list)
{
	_scene->addGraph(list);
	update();
}

int Q2DDiagramView::getHeight()
{
	return (_scene->itemsBoundingRect()).height();
}

int Q2DDiagramView::getWidth()
{
	return (_scene->itemsBoundingRect()).width();
}

/**
 * Initialises the view.
 */
void Q2DDiagramView::initialize()
{	
	//QMatrix currentMatrix = matrix();
	//setMatrix(currentMatrix * scene->getTransformationMatrix());
	
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	update();
}

/*
 * Keeps the aspect ration of the labels when the view is resized.
 * It is only necessary to call this if 
 *		Qt::AspectRatioMode == Qt::IgnoreAspectRatio.
 * Also, this method is kind of annoying because you have to set the
 * appropriate transform for every single QGraphicsTextItem seperately.
 */
/*
void Q2DDiagramView::keepItemAspectRatio()
{
	double xFactor = transform().mapRect(QRectF(0, 0, 1, 1)).width();
	double yFactor = transform().mapRect(QRectF(0, 0, 1, 1)).height();
    QMatrix invertedScaling;
    invertedScaling.scale(1.0 , xFactor / yFactor);

	scene->xLabel->setTransform(QTransform(invertedScaling));
	scene->yLabel->setTransform(QTransform(invertedScaling));
	scene->yLabel->rotate(-90);
}
*/

QSize Q2DDiagramView::minimumSizeHint() const
{
    return QSize(3*_scene->MARGIN,2*_scene->MARGIN);
}

QSize Q2DDiagramView::sizeHint() const
{
    return QSize(6*_scene->MARGIN, 4*_scene->MARGIN);
}

void Q2DDiagramView::resizeEvent(QResizeEvent* event)
{
	update();
	//keepItemAspectRatio();
}

/**
 * Updates the view automatically when a new list is added or when 
 * the window containing the view is resized or changes its state. 
 * Basically, the methods makes sure that everything keeps looking 
 * as it is supposed to.
 */
void Q2DDiagramView::update()
{
	//setResizeAnchor(QGraphicsView::AnchorViewCenter);
	QRectF viewRect = _scene->itemsBoundingRect();
	_scene->setSceneRect(viewRect);
	QRectF sceneInView(_scene->MARGIN/2,_scene->MARGIN/2,viewRect.width()+_scene->MARGIN,viewRect.height()+_scene->MARGIN);
	fitInView(sceneInView, Qt::IgnoreAspectRatio);
}

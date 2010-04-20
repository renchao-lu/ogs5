/**
 * \file StratView.cpp
 * 2010/03/16 - KR Initial implementation
 */

#include <math.h>
#include "StratView.h"
#include "Station.h"


StratView::~StratView()
{
	delete _scene;
}

void StratView::setStation(GEOLIB::StationBorehole* station)
{
	_scene = new StratScene(station);
	setScene(_scene);
	initialize();
}

void StratView::initialize()
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	update();
}

void StratView::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED (event)
	update();
}

void StratView::update()
{
	QRectF viewRect = _scene->itemsBoundingRect();
	_scene->setSceneRect(viewRect);
	QRectF sceneInView(_scene->MARGIN,_scene->MARGIN,viewRect.width()+2*_scene->MARGIN,viewRect.height()+2*_scene->MARGIN);
	fitInView(sceneInView, Qt::IgnoreAspectRatio);
}

/**
 * \file ViewWidget2d.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of ViewWidget2d
 */

// ** INCLUDES **
#include "ViewWidget2d.h"
#include "GraphicsScene.h"
#include "ui_ViewWidget2dBase.h"
#include "GraphicsItem2d.h"
#include <QComboBox>

ViewWidget2d::ViewWidget2d( QWidget* parent /*= 0*/ )
: QMainWindow(parent)
{
	setupUi(this);

	QComboBox* viewplaneComboBox = new QComboBox(this);
	viewplaneComboBox->addItem("X/Y");
	viewplaneComboBox->addItem("X/Z");
	viewplaneComboBox->addItem("Y/Z");
	connect(viewplaneComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setViewplane(int)));
	toolBar->addWidget(viewplaneComboBox);

	itemTypeComboBox = new QComboBox(this);
	itemTypeComboBox->addItem("Point");
	itemTypeComboBox->addItem("Line");
	itemTypeComboBox->addItem("Station");
	itemTypeComboBox->addItem("Mesh");
	connect(itemTypeComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(setEditableItemType(int)));
	toolBar->addWidget(itemTypeComboBox);

}

GraphicsScene* ViewWidget2d::scene() const
{
	return graphicsView2d->scene();
}

void ViewWidget2d::setScene( GraphicsScene* scene )
{
	if (graphicsView2d->scene())
		disconnect(graphicsView2d->scene(), SIGNAL(itemsLoaded()),
			this, SLOT(setEditableItemType())); 
	graphicsView2d->setScene(scene);
	connect(scene, SIGNAL(itemsLoaded()), this, SLOT(setEditableItemType())); 
}

QSize ViewWidget2d::minimumSizeHint() const
{
	return QSize(150, 150);
}
void ViewWidget2d::on_actionShow_All_triggered( bool checked /*= false */ )
{
	graphicsView2d->fitInView(graphicsView2d->scene()->sceneRect(),
		Qt::KeepAspectRatio);
	graphicsView2d->updateView();
}

void ViewWidget2d::setViewplane ( int index )
{
	graphicsView2d->scene()->setViewPlane((EViewPlane)index);
}


void ViewWidget2d::setEditableItemType( )
{
	setEditableItemType(itemTypeComboBox->currentIndex());
}

void ViewWidget2d::setEditableItemType( int index )
{
	EItemType editableItemType;
	switch (index)
	{
	case 0: editableItemType = PNT_ITEM; break;
	case 1: editableItemType = LINE_ITEM; break;
	case 2: editableItemType = STATION_ITEM; break;
	case 3: editableItemType = MSH_NODE_ITEM; break;
	}

	foreach (QGraphicsItem* item, scene()->items())
	{
		//if possible cast item to GraphicsItem2D
		//if not possible (e.g. for QGraphicsItem from standard-QT, the following condition will be skipped //KR
		GraphicsItem2d* item2d = dynamic_cast<GraphicsItem2d*>(item);
		if (item2d)
		{
			if (item2d->itemType() == editableItemType)
				item2d->setEditable(true);
			else
				item2d->setEditable(false);
		}
	}
}

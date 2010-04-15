/**
 * \file GraphicsScene.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of GraphicsScene
 */

// ** INCLUDES **
#include "GraphicsScene.h"
#include "GraphicsItem2d.h"
#include "geo_pnt.h"
#include "CoordinateOriginGraphicsItem2d.h"
#include "PntsModel.h"
#include "LinesModel.h"
#include "TreeModel.h"
#include "StationTreeModel.h"
#include "ModelItem.h"
#include "PntGraphicsItem2d.h"

#include <QPainter>
#include <QPalette>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelection>
#include <QModelIndex>
#include <QTime>

GraphicsScene::GraphicsScene(QObject *parent)
: QGraphicsScene(parent), _backgroundGridSize(1), _viewplane(VIEWPLANE_XY)
{
	connect(this, SIGNAL(selectionChanged()), this, SLOT(emitSelectionChanged()));

	setItemIndexMethod(BspTreeIndex);
}

GraphicsScene::~GraphicsScene()
{
}

void GraphicsScene::drawPixmap(const QPixmap &img, const QPointF &origin, const double &scalingFactor)
{
	QGraphicsPixmapItem* raster (addPixmap(img));
	raster->scale(scalingFactor, scalingFactor);
	raster->setPos(origin);
}


void GraphicsScene::loadItemsFromTreeModel(StationTreeModel* model, std::string name)
{
	std::vector<ModelTreeItem*> lists = model->getLists();
	size_t nLists = lists.size();

	for (size_t i=0; i<nLists; i++)
	{
		if ( name.compare( lists[i]->data(0).toString().toStdString() ) == 0 )
		{
			size_t nChildren = lists[i]->childCount();
			for (size_t j=0; j<nChildren; j++)
				addItem( static_cast<ModelTreeItem*>(lists[i]->child(j))->getItem()->item2d() );
		}
	}
	emit sceneChanged();
}

void GraphicsScene::loadItemsFromTableModel( Model* model )
{
// 	connect(model, SIGNAL(itemsSelectedFromOutside(QVector<QGraphicsItem*>)),
// 		this, SLOT(selectItems(QVector<QGraphicsItem*>)));
// 	connect(model, SIGNAL(itemsDeselectedFromOutside(QVector<QGraphicsItem*>)),
// 		this, SLOT(deselectItems(QVector<QGraphicsItem*>)));
//
// 	connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateItems(QModelIndex,QModelIndex)));
// 	_modelsByType[model->modelContentType()] = model;

	for (int i = 0; i < model->rowCount(); ++i)
	{
		ModelItem* item = model->itemFromIndex(model->index(i, 0));
		GraphicsItem2d* item2d = item->item2d();
		if (item2d)
		{
			// do not try to load objects into the scene that have been loaded previously
			if (item2d->scene() == this)
				continue;
			else
				addItem(item2d);
		}

		// Load items ModelItem submodels
		foreach (Model* itemSubModel, item->models())
			loadItemsFromTableModel(itemSubModel);
	}

	// Load items in submodels
	foreach (Model* subModel, model->subModels())
		loadItemsFromTableModel(subModel);

	emit sceneChanged();
}

void GraphicsScene::setViewPlane( EViewPlane viewplane )
{
	// set viewplane on all items
	QList<QGraphicsItem*> sceneItems = items();
	QMutableListIterator<QGraphicsItem*> it(sceneItems);
	while (it.hasNext())
	{
		PntGraphicsItem2d* item = (PntGraphicsItem2d*)it.next();
		item->setViewplane(viewplane);
	}
}

void GraphicsScene::setBackgroundGridSize( int size )
{
	_backgroundGridSize = size;
}

int GraphicsScene::backgroundGridSize() const
{
	return _backgroundGridSize;
}

void GraphicsScene::emitSelectionChanged()
{
	QList<QGraphicsItem*> selectedItems = this->selectedItems();
	QItemSelection itemSelection;

	// Iterate over selected 2d items
	foreach (QGraphicsItem* item, selectedItems)
	{
		// Get the model index of that item
		GraphicsItem2d* item2d = static_cast<GraphicsItem2d*>(item);
		QModelIndex index = item2d->modelIndex();
		if (index.isValid())
		{
			// Select the complete row in the model
			itemSelection.select(index.sibling(index.row(),0), index.sibling(index.row(), index.model()->columnCount()-1));
		}
	}

	// Get the items to deselect
	_oldSelection.merge(itemSelection, QItemSelectionModel::Deselect);

	emit itemSelectionChanged(itemSelection, _oldSelection);

	// Store the actual selection for the next function call
	_oldSelection = itemSelection;
}

void GraphicsScene::selectionChangedFromOutside()
{
	QList<QGraphicsItem*> selectedItems = this->selectedItems();
	QItemSelection itemSelection;

	// Iterate over selected 2d items
	foreach (QGraphicsItem* item, selectedItems)
	{
		// Get the model index of that item
		GraphicsItem2d* item2d = static_cast<GraphicsItem2d*>(item);
		QModelIndex index = item2d->modelIndex();
		if (index.isValid())
		{
			// Select the complete row in the model
			itemSelection.select(index.sibling(index.row(),0), index.sibling(index.row(), index.model()->columnCount()-1));
		}
	}

	// Get the items to deselect
	_oldSelection.merge(itemSelection, QItemSelectionModel::Deselect);

	// Store the actual selection for the next function call
	_oldSelection = itemSelection;
}

void GraphicsScene::updateItems( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
	const Model* model = static_cast<const Model*>(topLeft.model());
	ModelItem* item = model->itemFromIndex(topLeft);
	switch (model->modelContentType())
	{
	case Model::PNTS_MODEL:
		((PntGraphicsItem2d*)(item->item2d()))->updatePositionFromOutside();
		break;
	case Model::MSH_NODE_MODEL:
		((PntGraphicsItem2d*)(item->item2d()))->updatePositionFromOutside();
		break;
	default:
		break;
	}
}

void GraphicsScene::toggleMshVisibility( bool visible )
{
	toggleModelItemVisibility(Model::MSH_MODEL, visible);
	toggleModelItemVisibility(Model::MSH_NODE_MODEL, visible);
}

void GraphicsScene::toggleMshNodeVisibility( bool visible )
{
	toggleModelItemVisibility(Model::MSH_NODE_MODEL, visible);

}

void GraphicsScene::toggleMshElemVisibility( bool visible )
{
	toggleModelItemVisibility(Model::MSH_ELEM_MODEL, visible);
}

void GraphicsScene::togglePntsVisibility( bool visible )
{
	toggleModelItemVisibility(Model::PNTS_MODEL, visible);
}

void GraphicsScene::toggleLinesVisibility( bool visible )
{
	toggleModelItemVisibility(Model::LINES_MODEL, visible);
}

void GraphicsScene::toggleStationsVisibility( bool visible )
{
	toggleModelItemVisibility(Model::STATION_MODEL, visible);

}

void GraphicsScene::toggleModelItemVisibility( Model::ModelContentType modelType, bool visible )
{
	Model* model;
	if (modelType == Model::MSH_NODE_MODEL)
		model = _modelsByType[Model::MSH_MODEL]->subModels()[0];
	else if (modelType == Model::MSH_ELEM_MODEL)
		model = _modelsByType[Model::MSH_MODEL]->subModels()[1];
	else
		model = _modelsByType[modelType];
	if (model)
	{
		for (int i = 0; i < model->rowCount(); i++)
		{
			if (model->modelType() == Model::TREE_MODEL)
				toggleModelItemVisibilityRecursive(model->index(i, 0), model, visible);
			else
			{
				ModelItem* item = model->itemFromIndex(model->index(i, 0));
				GraphicsItem2d* item2d = item->item2d();
				if (item2d)
				{
					if (visible)
						item2d->show();
					else
						item2d->hide();
				}
			}
		}
	}
	this->update();
}

void GraphicsScene::toggleModelItemVisibilityRecursive( QModelIndex parent, Model* model, bool visible )
{
	ModelItem* parentItem = model->itemFromIndex(parent);
	GraphicsItem2d* parentItem2d = parentItem->item2d();
	if (parentItem2d)
	{
		if (visible)
			parentItem2d->show();
		else
			parentItem2d->hide();
	}

	for (int i = 0; i < parentItem->childCount(); ++i)
	{
		QModelIndex childIndex = model->index(i, 0, parent);
		toggleModelItemVisibilityRecursive(childIndex, model, visible);
	}
}

void GraphicsScene::selectItems( QVector<QGraphicsItem*> items )
{
	foreach(QGraphicsItem* item, items)
		item->setSelected(true);
	selectionChangedFromOutside();
}

void GraphicsScene::deselectItems( QVector<QGraphicsItem*> items )
{
	foreach(QGraphicsItem* item, items)
		item->setSelected(false);
	selectionChangedFromOutside();
}

/**
 * \file ModelItem.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of ModelItem
 */

// ** INCLUDES **
#include "ModelItem.h"
#include "Model.h"
#include "GraphicsItem2d.h"

#include <QGraphicsScene>

ModelItem::ModelItem(GraphicsItem2d* item2d, QObject* parent)
: QObject(parent), _item2d(item2d)
{
	_numberOfChildren = 0;
}

ModelItem::~ModelItem()
{
	if (_item2d)
	{
		if (_item2d->scene())
			_item2d->scene()->removeItem(_item2d);
		_item2d->deleteLater();
	}

	while (_models.size() > 0)
	{
		_models.back()->clearData();
		delete _models.back();
		_models.pop_back();
	}
}
GraphicsItem2d* ModelItem::item2d()
{
	return _item2d;
}

int ModelItem::childCount() const
{
	return _numberOfChildren;
}

void ModelItem::setNumberOfChildren(int numberOfChildren)
{
	_numberOfChildren = numberOfChildren;
}

vector<Model*> ModelItem::models() const
{
	return _models;
}

void ModelItem::addModel( Model* model )
{
	_models.push_back(model);
}

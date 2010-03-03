/**
 * \file ModelTreeItem.cpp
 * KR Initial implementation
 */

#include "ModelTreeItem.h"
#include "ModelItem.h"

using namespace GEOLIB;


/**
 * Constructor.
 * \param data The data associated with each column
 * \param parent The parent item in the tree
 * \param item The ModelItem-object
 */
ModelTreeItem::ModelTreeItem(const QList<QVariant> &data, TreeItem *parent, ModelItem* item)
:TreeItem(data, parent), _item(item)
{
}

ModelTreeItem::~ModelTreeItem()
{
	if (_item) delete _item;
}

/// Returns the station object from which this item has been constructed
Station* ModelTreeItem::getStation()
{
	return _stn;
}

/// Returns the ModelItem associated with this item
ModelItem* ModelTreeItem::getItem() const
{
	_item->setNumberOfChildren(TreeItem::childCount());
	return _item;
}

/// Associates a station object with this item
void ModelTreeItem::setStation(Station* stn)
{
	_stn = stn;
}

/// Associates a ModelItem with this item
void ModelTreeItem::setItem( ModelItem* item )
{
	_item = item;
}


/**
 * \file StationTreeModel.cpp
 * KR Initial implementation
 */

#include <QDebug>
#include <QItemSelection>

#include "StationTreeModel.h"
#include "Station.h"
#include "ModelItem.h"
#include "PntGraphicsItem2d.h"
#include "OGSError.h"
#include "BaseItem.h"



/**
 * Constructor.
 */
StationTreeModel::StationTreeModel( QObject *parent )
: TreeModel(parent)
{
	//_modelContentType = STATION_MODEL;
	QList<QVariant> rootData;
	rootData << "Station Name" << "x" << "y";
	_rootItem = new ModelTreeItem(rootData, NULL, NULL);
}

StationTreeModel::~StationTreeModel()
{
	delete _rootItem;
}

/**
 * Returns the model index of an item in the tree.
 * \param row The row where the item is located
 * \param column The column where the item is located
 * \param parent The parent of the item
 * \return The model index of the item
 */
QModelIndex StationTreeModel::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	ModelTreeItem *parentItem;

	if (!parent.isValid())
		parentItem = (ModelTreeItem*)(_rootItem);
	else
		parentItem = static_cast<ModelTreeItem*>(parent.internalPointer());

	ModelTreeItem *childItem = (ModelTreeItem*)(parentItem->child(row));
	if (childItem)
	{
		QModelIndex newIndex = createIndex(row, column, childItem);
		// assign ModelIndex to GraphicsItem so it can communicate with the model even after visualisation
		childItem->getItem()->item2d()->setModelIndex(newIndex);
		return newIndex;
	}
	else
		return QModelIndex();
}

/**
 * Returns the model item associated with a tree item. This is a re-implementation of
 * QOgsModelBase::itemFromIndex to deal with TreeItems with attached ModelItem
 * \param index Index of the requested item
 * \return The model item associated with the tree item
 */
ModelItem* StationTreeModel::itemFromIndex( const QModelIndex& index ) const
{
	if (index.isValid())
	{
		ModelTreeItem* treeItem = static_cast<ModelTreeItem*>(index.internalPointer());
		ModelItem* modelItem = treeItem->getItem();
		modelItem->setNumberOfChildren(treeItem->childCount());
		return modelItem;
	}
	else
		return NULL;
}

/**
 * Returns the Station-Object of the ModelTreeItem with the given index and the name of the list this station belongs to.
 * \param index Index of the requested item
 * \param listName Here, the method will put the name of the list this station belongs to.
 * \return The station object associated with the tree item
 */
GEOLIB::Station* StationTreeModel::stationFromIndex( const QModelIndex& index, QString &listName ) const
{
	if (index.isValid())
	{
		ModelTreeItem* treeItem = static_cast<ModelTreeItem*>(index.internalPointer());
		TreeItem* parentItem = treeItem->parent();
		listName = parentItem->data(0).toString();
		return treeItem->getStation();
	}
	else
		return NULL;
}

/**
 * Selects GraphicsItems based on the selected Index in the View
 */
void StationTreeModel::setSelection( const QItemSelection & selected, const QItemSelection & deselected )
{
	this->blockSignals(true);
	foreach (QModelIndex index, deselected.indexes())
	{
		GraphicsItem2d* item = itemFromIndex(index)->item2d();
		item->setSelected(false);
	}

	foreach (QModelIndex index, selected.indexes())
	{
		GraphicsItem2d* item = itemFromIndex(index)->item2d();
		item->setSelected(true);
	}
	this->blockSignals(false);
}


vtkPolyDataAlgorithm* StationTreeModel::vtkSource(const std::string &name)
{
	size_t nLists = _lists.size();
	for (size_t i=0; i<nLists; i++)
	{
		if ( name.compare( _lists[i]->data(0).toString().toStdString() ) == 0 )
			return dynamic_cast<BaseItem*>(_lists[i]->getItem()->item2d())->vtkSource();
	}
	return NULL;
}

/**
 * Inserts a subtree under _rootItem.
 * \param listName Name of the new subtree. If no name is given a default name is assigned.
 * \param stations The list with stations to be added as children of that subtree
 */
void StationTreeModel::addStationList(QString listName, const std::vector<GEOLIB::Point*> *stations)
{
	QList<QVariant> grpName;
	if (listName.compare("")==0) // if no name is given a default name is assigned
	{
		listName = "List";
		listName.append(QString::number(rowCount()+1));
	}
	grpName.push_back(QVariant(listName));
	grpName.push_back(QVariant(""));
	grpName.push_back(QVariant(""));
	BaseItem* grpItem = new BaseItem(stations);
	ModelItem* item = new ModelItem(static_cast<GraphicsItem2d*>(grpItem), NULL);
	ModelTreeItem* group = new ModelTreeItem(grpName, _rootItem, item);
	_lists.push_back(group);
	_rootItem->appendChild(group);
	int vectorSize = stations->size();

	for (int i=0; i<vectorSize; i++)
	{
		QList<QVariant> stn;
		stn.push_back(QVariant(QString::fromStdString(static_cast<GEOLIB::Station*>((*stations)[i])->getName())));
		stn.push_back(QVariant(QString::number((*(*stations)[i])[0],'f')));
		stn.push_back(QVariant(QString::number((*(*stations)[i])[1],'f')));

		PntGraphicsItem2d* item2D = new PntGraphicsItem2d(NULL, (*stations)[i], NULL, 1.0, STATION_ITEM);

		ModelItem* item = new ModelItem(item2D);
		ModelTreeItem* child = new ModelTreeItem(stn, group, item);
		child->setStation(static_cast<GEOLIB::Station*>((*stations)[i]));
		group->appendChild(child);
	}

	qDebug() << "List" << listName << "loaded, " << stations->size() << "items added.";

	reset();
}

/**
 * Removes the TreeItem with the given Index including all its children
 */
void StationTreeModel::removeStationList(QModelIndex index)
{
	if (index.isValid()) //
	{
		ModelTreeItem* item = static_cast<ModelTreeItem*>(getItem(index));

		// also delete the lists entry in the list directory of the model
		for (size_t i=0; i<_lists.size(); i++)
			if (item == _lists[i]) _lists.erase(_lists.begin() + i);

		removeRows(0, item->childCount(), index);
		removeRows(item->row(), 1, parent(index));

		emit updateScene();
	}
}

/**
 * Removes the TreeItem with the given name including all its children
 */
void StationTreeModel::removeStationList(const std::string &name)
{
	//removeStationList( getItemByName(QModelIndex(), name) );
	for (size_t i=0; i<_lists.size(); i++)
	{
		if ( name.compare( _lists[i]->data(0).toString().toStdString() ) == 0 )
			removeStationList(createIndex(_lists[i]->row(), 0, _lists[i]));
	}
}

/*
 * Traverses the (sub-)tree of the TreeItem with the given index and looks for an item in that subtree
 * with the given name. Using this method is probably rather slow and not recommended.
 * If this method is called with an empty QModelIndex, the TreeModel will assign the rootItem to that
 * index.
 * \param index The TreeItem whose subtree will be searched.
 * \param name The name of the item that should be found
 * \return The QModelIndex of the desired item (or an empty index if no such item exists).
 *
QModelIndex StationTreeModel::getItemByName(const QModelIndex &idx, const std::string &name) const
{
	QModelIndex didx;
	TreeItem* item = getItem(idx);

	int nChildren = item->childCount();
	for (int i=0; i<nChildren; i++)
	{
		TreeItem* child = item->child(i);
		QString test = child->data(0).toString();

		if ( name.compare(child->data(0).toString().toStdString()) != 0 )
			didx = getItemByName( index(i, 0, idx), name );
		else didx = index(i, 0, idx);
		if (didx.isValid())
			return didx;

	}

	return QModelIndex(); // this is no valid QModelIndex and signifies that no item by the given name could be found.
}
*/

/**
 * Filters the station list based on the property boundaries given in bounds.
 * Technically, the complete station list is removed from the model and only those items are re-loaded that fit the description.
 * If no station in the list fulfills the given description an error msg is given.
 */
void StationTreeModel::filterStations(const std::string &listName, const std::vector<GEOLIB::Point*> *stations, const std::vector<PropertyBounds> &bounds)
{

	//QString listName = static_cast<ModelTreeItem*>(index.internalPointer())->data(0).toString();
	//removeStationList(index);

	int itemCount=0;
	removeStationList(listName);

	QList<QVariant> grpName;
	grpName.push_back(QVariant(QString::fromStdString(listName)));
	grpName.push_back(QVariant(""));
	grpName.push_back(QVariant(""));
	BaseItem* grpItem = new BaseItem(stations);
	ModelItem* item = new ModelItem(static_cast<GraphicsItem2d*>(grpItem), NULL);
	ModelTreeItem* group = new ModelTreeItem(grpName, _rootItem, item);
	_rootItem->appendChild(group);

	size_t vectorSize = stations->size();
	for (size_t i=0; i<vectorSize; i++)
	{
		if (static_cast<GEOLIB::Station*>((*stations)[i])->inSelection(bounds))
		{
			itemCount++;
			QList<QVariant> stn;
			stn.push_back(QVariant(QString::fromStdString(static_cast<GEOLIB::Station*>((*stations)[i])->getName())));
			stn.push_back(QVariant(QString::number((*(*stations)[i])[0],'f')));
			stn.push_back(QVariant(QString::number((*(*stations)[i])[1],'f')));

			PntGraphicsItem2d* item2D = new PntGraphicsItem2d(NULL, (*stations)[i], NULL, 1.0, STATION_ITEM);

			ModelItem* item = new ModelItem(item2D);
			ModelTreeItem* child = new ModelTreeItem(stn, group, item);
			child->setStation(static_cast<GEOLIB::Station*>((*stations)[i]));
			group->appendChild(child);
		}
	}

	std::cout << "Filter applied to List \"" << listName << "\", " << itemCount << " items added.";

	if (itemCount==0) OGSError::box("No object is within the given boundaries."); //The filtered list is empty.

	reset();

	//emit updateScene();
}

void StationTreeModel::setSelectionFromOutside( const QItemSelection & selected, const QItemSelection & deselected )
{
	QVector<QGraphicsItem*> selectedItems;
	QVector<QGraphicsItem*> deselectedItems;

	updateSelection(selected, selectedItems, deselected, deselectedItems);

	emit itemsSelectedFromOutside(selectedItems);
	emit itemsDeselectedFromOutside(deselectedItems);
}

void StationTreeModel::updateSelection( const QItemSelection &selected, QVector<QGraphicsItem*> &selectedItems, const QItemSelection &deselected, QVector<QGraphicsItem*> &deselectedItems )
{
	foreach (QModelIndex index, deselected.indexes())
	{
		if (index.column() == 0)
		{
			GraphicsItem2d* item = itemFromIndex(index)->item2d();
			deselectedItems.push_back(item);

			//qDebug() << "Model: Deselecting row" << index.row();
		}
	}

	foreach (QModelIndex index, selected.indexes())
	{
		if (index.column() == 0)
		{
			GraphicsItem2d* item = itemFromIndex(index)->item2d();
			selectedItems.push_back(item);

			//qDebug() << "Model: Selecting row" << index.row(); //KR
		}
	}

	_selectedItems.merge(selected, QItemSelectionModel::Select);
	_selectedItems.merge(deselected, QItemSelectionModel::Deselect);
}

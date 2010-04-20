/**
 * \file Model.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of Model
 */

// ** INCLUDES **
#include "Model.h"
#include "ModelItem.h"
#include "GraphicsItem2d.h"

#include <vtkPolyDataAlgorithm.h>

#include <QItemSelection>
#include <QDebug>

#include <iostream>


Model::Model( QString name, QObject* parent /*= 0*/ )
: QAbstractTableModel(parent), _modelType(TABLE_MODEL),
 _modelContentType(EMPTY_MODEL), _name(name), _vtkSource(NULL)
{
}

Model::~Model()
{
}

QModelIndex Model::index( int row, int column, const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

		if (_data.size() == 0 || row < 0 || column < 0 || _data.size() < row+1)
			return QModelIndex();

	ModelItem* item = _data.at(row);
	QModelIndex newIndex = createIndex(row, column, (void*)item);
	if (item->item2d())
		item->item2d()->setModelIndex(newIndex);
	return newIndex;
}

ModelItem* Model::itemFromIndex( const QModelIndex& index ) const
{
	if (index.isValid())
		return (ModelItem*)(index.internalPointer());
	else
		return NULL;
}

QModelIndex Model::indexFromItem( const ModelItem* item ) const
{
	if (item != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex modelIndex = index(i, 0);
			ModelItem* itemFromModel = itemFromIndex(modelIndex);

			if (item == itemFromModel)
				return modelIndex;
		}
	}
	return QModelIndex();
}

Model::ModelType Model::modelType() const
{
	return _modelType;
}

Model::ModelContentType Model::modelContentType() const
{
	return _modelContentType;
}
QVector<Model*> Model::subModels() const
{
	return _subModels;
}
int Model::rowCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return _data.size();
}

Qt::ItemFlags Model::flags( const QModelIndex& index ) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool Model::removeRows( int row, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
	QItemSelection deletedRowsSelection;
	beginInsertColumns(parent, row, row + count - 1);
	for (int i = 0; i < count; i++)
	{
		QVector<ModelItem*>::iterator it = _data.begin() + row;
		ModelItem* item = *it;
		QModelIndex index = indexFromItem(item);
		QModelIndex lastIndex = index.sibling(index.row(), columnCount()-1);
		deletedRowsSelection.select(index, lastIndex);
		delete item;
		_data.erase(it);
	}
 	_selectedItems.merge(deletedRowsSelection, QItemSelectionModel::Deselect);
	endInsertColumns();

	return true;
}
void Model::setSelection( const QItemSelection & selected, const QItemSelection & deselected )
{
	QVector<QGraphicsItem*> selectedItems;
	QVector<QGraphicsItem*> deselectedItems;

	updateSelection(selected, selectedItems, deselected, deselectedItems);

	emit itemsSelected(selectedItems);
	emit itemsDeselected(deselectedItems);

}

void Model::setSelection( const QItemSelection & selected )
{
	return setSelection(selected, QItemSelection());
}

void Model::setSelectionFromOutside( const QItemSelection & selected, const QItemSelection & deselected )
{
	QVector<QGraphicsItem*> selectedItems;
	QVector<QGraphicsItem*> deselectedItems;

	updateSelection(selected, selectedItems, deselected, deselectedItems);

	emit itemsSelectedFromOutside(selectedItems);
	emit itemsDeselectedFromOutside(deselectedItems);

}
void Model::updateData()
{
	foreach( Model* subModel, _subModels )
		subModel->updateData();
	reset();

	//_vtkSource->Update();
}

void Model::clearData()
{
	if (rowCount() > 0)
		removeRows(0, rowCount());

	foreach (Model* model, _subModels)
	{
		model->clearData();
		delete model;
	}

	_data.clear();
	_subModels.clear();

	//_vtkSource->Update();

	emit selectionCleared();
}

void Model::clearSelectedData()
{
	while (_selectedItems.indexes().size() > 0)
	{
		QModelIndexList indices = _selectedItems.indexes();
		int lastRow = 0;
		for (int i = 0; i < indices.size(); i++)
			if (indices.at(i).row() > lastRow)
				lastRow = indices.at(i).row();

		qDebug() << "Model: Removing row" << lastRow;
		removeRow(lastRow);
	}

	//_vtkSource->Update();

	emit selectionCleared();
}

void Model::updateSelection( const QItemSelection &selected, QVector<QGraphicsItem*> &selectedItems, const QItemSelection &deselected, QVector<QGraphicsItem*> &deselectedItems )
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

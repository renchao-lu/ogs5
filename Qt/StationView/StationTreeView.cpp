/**
 * \file StationTreeView.cpp
 * KR Initial implementation
 */

#include <iostream>
#include <QFileDialog>
#include <QMenu>
#include "StationTreeView.h"
#include "StationTreeModel.h"
#include "Station.h"
#include "ModelTreeItem.h"

using namespace GEOLIB;


StationTreeView::StationTreeView(QWidget* parent) : QTreeView(parent)
{
//	setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
}

StationTreeView::~StationTreeView()
{
}

void StationTreeView::updateView()
{
	setAlternatingRowColors(true);
	resizeColumnToContents(0);
	setColumnWidth(1,50);
	setColumnWidth(2,50);
}

void StationTreeView::on_Clicked(QModelIndex idx)
{
	qDebug("%d, %d",idx.parent().row(), idx.row());
}

void StationTreeView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
	emit itemSelectionChanged(selected, deselected);
	return QTreeView::selectionChanged(selected, deselected);
}

void StationTreeView::selectionChangedFromOutside( const QItemSelection &selected, const QItemSelection &deselected )
{
	QItemSelectionModel* selModel = this->selectionModel();

	selModel->blockSignals(true);
	selModel->select(deselected, QItemSelectionModel::Deselect);
	selModel->select(selected, QItemSelectionModel::Select);
	selModel->blockSignals(false);

	return QTreeView::selectionChanged(selected, deselected);
}

void StationTreeView::contextMenuEvent( QContextMenuEvent* event )
{
	QModelIndex index = this->selectionModel()->currentIndex();
	ModelTreeItem* item = static_cast<ModelTreeItem*>(index.internalPointer());

	// The current index refers to a parent item (e.g. a listname)
	if (item->childCount()>0)
	{
		QMenu menu;
		QAction* propertyAction = menu.addAction("Display list properties...");
		menu.addSeparator();
		QAction* removeAction = menu.addAction("Remove station list");

		connect(propertyAction, SIGNAL(triggered()), this, SLOT(showPropertiesDialog()));
		connect(removeAction, SIGNAL(triggered()), this, SLOT(removeStationList()));
		menu.exec(event->globalPos());
	}
	// The current index refers to a station object
	else
	{
		QString temp_name;
		if (static_cast<StationTreeModel*>(model())->stationFromIndex(index, temp_name)->type() == Station::BOREHOLE)
		{
			QMenu menu;
			QAction* exportAction = menu.addAction("Export to GMS...");
			connect(exportAction, SIGNAL(triggered()), this, SLOT(exportStation()));
			menu.exec(event->globalPos());
		}
	}
}

void StationTreeView::exportStation()
{
	QModelIndex index = this->selectionModel()->currentIndex();
	QString fileName = QFileDialog::getSaveFileName(this, "Export Borehole to GMS-Format", "","*.txt");
    if (!fileName.isEmpty()) {
    	QString temp_name;
		static_cast<StationBorehole*>(static_cast<StationTreeModel*>(model())->stationFromIndex(index, temp_name))->writeAsGMS(fileName.toStdString());
    }
}

void StationTreeView::removeStationList()
{
	TreeItem* item = static_cast<StationTreeModel*>(model())->getItem(this->selectionModel()->currentIndex());
	emit stationListRemoved((item->data(0).toString()).toStdString());
}

void StationTreeView::showPropertiesDialog()
{
	QModelIndex index = this->selectionModel()->currentIndex();
	QString name = (static_cast<ModelTreeItem*>(index.internalPointer())->data(0)).toString();
	emit propertiesDialogRequested(name.toStdString());
}

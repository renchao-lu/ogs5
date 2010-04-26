/**
 * \file StationTreeView.cpp
 * KR Initial implementation
 */

#include <iostream>
#include <QFileDialog>
#include <QMenu>

#include "Station.h"
#include "StationIO.h"

#include "StationTreeView.h"
#include "StationTreeModel.h"
#include "ModelTreeItem.h"
#include "StratWindow.h"

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
		QAction* exportAction   = menu.addAction("Export to GMS...");
		menu.addSeparator();
		QAction* removeAction   = menu.addAction("Remove station list");

		connect(propertyAction, SIGNAL(triggered()), this, SLOT(showPropertiesDialog()));
		connect(exportAction,   SIGNAL(triggered()), this, SLOT(exportList()));
		connect(removeAction,   SIGNAL(triggered()), this, SLOT(removeStationList()));
		menu.exec(event->globalPos());
	}
	// The current index refers to a station object
	else
	{
		QString temp_name;
		if (static_cast<StationTreeModel*>(model())->stationFromIndex(index, temp_name)->type() == Station::BOREHOLE)
		{
			QMenu menu;
			QAction* stratAction = menu.addAction("Display Stratigraphy...");
			QAction* exportAction = menu.addAction("Export to GMS...");
			connect(stratAction, SIGNAL(triggered()), this, SLOT(displayStratigraphy()));
			connect(exportAction, SIGNAL(triggered()), this, SLOT(exportStation()));
			menu.exec(event->globalPos());
		}
	}
}

void StationTreeView::displayStratigraphy()
{
	QModelIndex index = this->selectionModel()->currentIndex();
	QString temp_name;
	StratWindow* stationView = new StratWindow(static_cast<GEOLIB::StationBorehole*>(static_cast<StationTreeModel*>(model())->stationFromIndex(index, temp_name)));
	stationView->show();
}


void StationTreeView::exportList()
{
	TreeItem* item = static_cast<StationTreeModel*>(model())->getItem(this->selectionModel()->currentIndex());
	std::string listName = item->data(0).toString().toStdString();
	QString fileName = QFileDialog::getSaveFileName(this, "Export Boreholes to GMS-Format", "","*.txt");
    if (!fileName.isEmpty()) {
		emit stationListExportRequested(listName, fileName.toStdString());
	}
}

void StationTreeView::exportStation()
{
	QModelIndex index = this->selectionModel()->currentIndex();
	QString fileName = QFileDialog::getSaveFileName(this, "Export Borehole to GMS-Format", "","*.txt");
    if (!fileName.isEmpty()) {
    	QString temp_name;
		std::vector<std::string> temp_soil_names; 
		temp_soil_names.push_back(""); // soil name vector needs to be initialised
		StationIO::writeBoreholeToGMS(static_cast<StationBorehole*>(static_cast<StationTreeModel*>(model())->stationFromIndex(index, temp_name)), fileName.toStdString(), temp_soil_names);
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

/**
 * \file DataView.cpp
 * 24/9/2009 LB Initial implementation
 * 
 * Implementation of DataView
 */

#include "DataView.h"
#include "Model.h"

#include <QHeaderView>

DataView::DataView( QWidget* parent /*= 0*/ )
: QTableView(parent)
{
	verticalHeader()->hide();
	resizeColumnsToContents();
	resizeRowsToContents();
}

QModelIndexList DataView::selectedIndexes() const
{
	return QTableView::selectedIndexes();
}


void DataView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
	emit itemSelectionChanged(selected, deselected);
	return QTableView::selectionChanged(selected, deselected);
}

void DataView::selectionChangedFromOutside( const QItemSelection &selected, const QItemSelection &deselected )
{
	QItemSelectionModel* selModel = this->selectionModel();

	Q_ASSERT(selModel);

	selModel->blockSignals(true);
	selModel->select(deselected, QItemSelectionModel::Deselect);
	selModel->select(selected, QItemSelectionModel::Select);
	selModel->blockSignals(false);

	Model* model = (Model*)this->model();
	model->setSelectionFromOutside(selected, deselected);

	return QTableView::selectionChanged(selected, deselected);
}

void DataView::clearSelection()
{
	selectionModel()->clearSelection();
}

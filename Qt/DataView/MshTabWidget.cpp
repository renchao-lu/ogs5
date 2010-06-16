/**
 * \file MshTabWidget.cpp
 * 3/11/2009 LB Initial implementation
 * 18/05/2010 KR Re-Implementation
 *
 * Implementation of MshTabWidget
 */

// ** INCLUDES **
#include "MshTabWidget.h"
#include "MshModel.h"
#include "TreeItem.h"

#include <QObject>

MshTabWidget::MshTabWidget( QWidget* parent /*= 0*/ )
: QWidget(parent)
{
	setupUi(this);

	connect(this->clearSelectedPushButton, SIGNAL(clicked()), this, SLOT(removeMesh()));
	connect(this->clearAllPushButton, SIGNAL(clicked()), this, SLOT(removeAllMeshes()));
	

/*
	mshTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mshTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	mshNodeTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mshNodeTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(mshTableView, SIGNAL(itemSelectionChanged(QItemSelection, QItemSelection)),
		this, SLOT(changeMshSubmodelViews(QItemSelection, QItemSelection)));
*/
}


void MshTabWidget::removeMesh()
{
	emit requestMeshRemoval(this->treeView->selectionModel()->currentIndex());
}

void MshTabWidget::removeAllMeshes()
{
	TreeItem* root = static_cast<MshModel*>(this->treeView->model())->getItem(QModelIndex());
	int nChildren = root->childCount()-1;
	for (int i=nChildren; i>=0; i--)
		emit requestMeshRemoval(this->treeView->model()->index(i, 0, QModelIndex()));
}

/*
void MshTabWidget::changeMshSubmodelViews( QItemSelection selected, QItemSelection deselected )
{

	if (selected.size() > 0)
	{
		QModelIndex index = *(selected.begin()->indexes().begin());
		if (!index.isValid())
			return;

		MshModel* mshModel = static_cast<MshModel*>(mshTableView->model());

		ModelItem* item = mshModel->itemFromIndex(index);

		mshNodeTableView->setModel(item->models()[0]);
		mshNodeTableView->resizeColumnsToContents();
		mshNodeTableView->resizeRowsToContents();

		connect(mshNodeTableView, SIGNAL(itemSelectionChanged(const QItemSelection&,const QItemSelection&)),
			item->models()[0], SLOT(setSelection(const QItemSelection&, const QItemSelection&)));

//		connect(item->models()[0], SIGNAL(dataChanged(QModelIndex,QModelIndex)), _scene, SLOT(updateItems(QModelIndex,QModelIndex)));
//		connect(item->models()[1], SIGNAL(dataChanged(QModelIndex,QModelIndex)), _scene, SLOT(updateItems(QModelIndex,QModelIndex)));

		mshElemTableView->setModel(item->models()[1]);
		mshElemTableView->resizeColumnsToContents();
		mshElemTableView->resizeRowsToContents();

	}
	else
	{
		mshNodeTableView->setModel(NULL);
		mshElemTableView->setModel(NULL);
	}

}
*/

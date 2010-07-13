/**
 * \file VtkVisPipelineView.cpp
 * 18/2/2010 LB Initial implementation
 *
 * Implementation of VtkVisPipelineView
 */

// ** INCLUDES **
#include "VtkVisPipelineView.h"

#include "VtkVisPipelineItem.h"
#include "VtkVisPipeline.h"

#include <vtkActor.h>

#include <QMenu>
#include <QContextMenuEvent>
#include <QFileDialog>

VtkVisPipelineView::VtkVisPipelineView( QWidget* parent /*= 0*/ )
: QTreeView(parent)
{
	setItemsExpandable(false);
}

void VtkVisPipelineView::contextMenuEvent( QContextMenuEvent* event )
{
	QModelIndex index = selectionModel()->currentIndex();
	if (index.isValid())
	{
		// VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(index.internalPointer());
		QMenu menu;
		QAction* addFilterAction = menu.addAction("Add filter...");
		menu.addSeparator();
		QAction* exportAction = menu.addAction("Export...");
		QAction* removeAction = menu.addAction("Remove");

		connect(addFilterAction, SIGNAL(triggered()), this, SLOT(addPipelineFilterItem()));
		connect(exportAction, SIGNAL(triggered()), this, SLOT(exportSelectedPipelineItem()));
		connect(removeAction, SIGNAL(triggered()), this, SLOT(removeSelectedPipelineItem()));

		menu.exec(event->globalPos());
	}
}

void VtkVisPipelineView::exportSelectedPipelineItem()
{
	QModelIndex idx = this->selectionModel()->currentIndex();
	QString filename = QFileDialog::getSaveFileName(this, "Export object to vtk-file", "","VTK file (*.vtp *.vtu)");
	static_cast<VtkVisPipelineItem*>(static_cast<VtkVisPipeline*>(this->model())->getItem(idx))->writeToFile(filename.toStdString());
}

void VtkVisPipelineView::removeSelectedPipelineItem()
{
	emit requestRemovePipelineItem(selectionModel()->currentIndex());
}

void VtkVisPipelineView::addPipelineFilterItem()
{
	emit requestAddPipelineFilterItem(selectionModel()->currentIndex());
}

void VtkVisPipelineView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
	QTreeView::selectionChanged(selected, deselected);

	QModelIndex index = this->selectionModel()->currentIndex();
	if (index.isValid())
	{
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(index.internalPointer());
		emit actorSelected(item->actor());
		emit itemSelected(item);
	}
	else
	{
		emit actorSelected((vtkActor*)NULL);
		emit itemSelected(NULL);
	}
		
}

void VtkVisPipelineView::selectItem( vtkActor* actor )
{
	QModelIndex index = ((VtkVisPipeline*)(this->model()))->getIndex(actor);
	if (!index.isValid())
		return;

	blockSignals(true);
	QItemSelectionModel* selectionModel = this->selectionModel();
	selectionModel->clearSelection();
	selectionModel->select(index, QItemSelectionModel::Select);
	blockSignals(false);
}

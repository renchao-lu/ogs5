/**
 * \file VtkVisPipelineView.cpp
 * 18/2/2010 LB Initial implementation
 *
 * Implementation of VtkVisPipelineView
 */

// ** INCLUDES **
#include "VtkVisPipelineView.h"

#include "VtkVisPipelineItem.h"

#include <QMenu>
#include <QContextMenuEvent>

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
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(index.internalPointer());
		QMenu menu;
		QAction* removeAction = menu.addAction("Remove");
		menu.addSeparator();
		QAction* addFilterAction = menu.addAction("Add filter");

		connect(removeAction, SIGNAL(triggered()), this, SLOT(removeSelectedPipelineItem()));
		connect(addFilterAction, SIGNAL(triggered()), this, SLOT(addPipelineFilterItem()));

		menu.exec(event->globalPos());
	}
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
		emit itemSelected(item);
	}
	else
		emit itemSelected(NULL);
}
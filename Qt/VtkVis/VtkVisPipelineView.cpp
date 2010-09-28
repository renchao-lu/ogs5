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
#include <vtkDataSetMapper.h>

#include <QMenu>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QSettings>

// OpenSG
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGGroup.h>
#include "vtkOsgActor.h"

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
		QAction* exportVtkAction = menu.addAction("Export as VTK");
		QAction* exportOsgAction = menu.addAction("Export as OpenSG");
		QAction* removeAction = menu.addAction("Remove");

		connect(addFilterAction, SIGNAL(triggered()), this, SLOT(addPipelineFilterItem()));
		connect(exportVtkAction, SIGNAL(triggered()), this, SLOT(exportSelectedPipelineItemAsVtk()));
		connect(exportOsgAction, SIGNAL(triggered()), this, SLOT(exportSelectedPipelineItemAsOsg()));
		connect(removeAction, SIGNAL(triggered()), this, SLOT(removeSelectedPipelineItem()));

		menu.exec(event->globalPos());
	}
}

void VtkVisPipelineView::exportSelectedPipelineItemAsVtk()
{
	QSettings settings("UFZ", "OpenGeoSys-5");
	QModelIndex idx = this->selectionModel()->currentIndex();
	QString filename = QFileDialog::getSaveFileName(this, "Export object to vtk-file",
		settings.value("lastExportedFileDirectory").toString(),"VTK file (*.vtk)");
	if (!filename.isEmpty())
	{
		static_cast<VtkVisPipelineItem*>(static_cast<VtkVisPipeline*>(this->model())->getItem(idx))->writeToFile(filename.toStdString());
		QDir dir = QDir(filename);
		settings.setValue("lastExportedFileDirectory", dir.absolutePath());
	}
}

void VtkVisPipelineView::exportSelectedPipelineItemAsOsg()
{
	QSettings settings("UFZ", "OpenGeoSys-5");
	QModelIndex idx = this->selectionModel()->currentIndex();
	QString filename = QFileDialog::getSaveFileName(this, "Export object to OpenSG file",
		settings.value("lastExportedFileDirectory").toString(), "OpenSG file (*.osg *.osb)");
	if (!filename.isEmpty())
	{
		OSG::NodePtr root = OSG::makeCoredNode<OSG::Group>();
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(static_cast<VtkVisPipeline*>(this->model())->getItem(idx));
		vtkOsgActor* actor = static_cast<vtkOsgActor*>(item->actor());
		item->mapper()->SetScalarVisibility(true);
		actor->SetVerbose(true);
		actor->UpdateOsg();
		beginEditCP(root);
		root->addChild(actor->GetOsgRoot());
		endEditCP(root);
		//actor->ClearOsg();

		OSG::SceneFileHandler::the().write(root, filename.toStdString().c_str());

		QDir dir = QDir(filename);
		settings.setValue("lastExportedFileDirectory", dir.absolutePath());
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

/**
 * \file VtkVisPipeline.cpp
 * 17/2/2010 LB Initial implementation
 * 
 * Implementation of VtkVisPipeline
 */

// ** INCLUDES **
#include "VtkVisPipeline.h"

//#include "VtkVisPipelineItem.h"
#include "Model.h"
#include "StationTreeModel.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkAlgorithm.h>
#include <vtkPointSet.h>
#include <vtkActor.h>

#include <vtkOutlineFilter.h>

#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>

VtkVisPipeline::VtkVisPipeline( vtkRenderer* renderer, QObject* parent /*= 0*/ )
: TreeModel(parent), _renderer(renderer)
{
	QList<QVariant> rootData;
	rootData << "Object name" << "Visible";
	delete _rootItem;
	_rootItem = new TreeItem(rootData, NULL);
}

Qt::ItemFlags VtkVisPipeline::flags( const QModelIndex &index ) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	if (index.column() == 1)
		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	else
		return QAbstractItemModel::flags(index);
}

void VtkVisPipeline::addPipelineItem( Model* model )
{
	addPipelineItem(model->vtkSource());
}

void VtkVisPipeline::addPipelineItem(StationTreeModel* model, const std::string &name)
{
	addPipelineItem(model->vtkSource(name));
/*
	vtkSphereSource* sphere = vtkSphereSource::New();
	sphere->SetRadius(50);
	sphere->SetPhiResolution(2);
	sphere->SetThetaResolution(2);
	sphere->SetReleaseDataFlag(1);

	vtkGlyph3D* glyphs = vtkGlyph3D::New();
    glyphs->ScalingOn();
    glyphs->SetScaleModeToScaleByScalar();

    glyphs->SetSource(sphere->GetOutput());
    glyphs->SetInput(model->vtkSource(name)->GetOutput());

	addPipelineItem(glyphs);
*/
}


void VtkVisPipeline::addPipelineItem( vtkAlgorithm* source,
									  QModelIndex parent /* = QModelindex() */)
{
	TreeItem* parentItem = getItem(parent);
	vtkPointSet* input = NULL;

	if (parent.isValid())
	{
		VtkVisPipelineItem* visParentItem = static_cast<VtkVisPipelineItem*>(parentItem);
		input = static_cast<vtkPointSet*>(visParentItem->algorithm()->GetOutputDataObject(0));
	}

	QList<QVariant> itemData;
	itemData << source->GetClassName() << true;
	VtkVisPipelineItem* item;
	item = new VtkVisPipelineItem(_renderer, source, parentItem, input, itemData);
	parentItem->appendChild(item);

	QModelIndex newIndex = index(parentItem->childCount(), 0, parent);

	_renderer->ResetCamera(_renderer->ComputeVisiblePropBounds());

	reset();
	emit vtkVisPipelineChanged();
}

void VtkVisPipeline::removeSourceItem( Model* model )
{
	for (int i = 0; i < rowCount(); i++)
	{
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(getItem(index(i, 0)));
		if (item->algorithm() == model->vtkSource())
			removePipelineItem(index(i, 0));
	}
	//removePipelineItem(model->vtkSource());
}

void VtkVisPipeline::removeSourceItem(StationTreeModel* model, const std::string &name)
{
	for (int i = 0; i < _rootItem->childCount(); i++)
	{
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(getItem(index(i, 0)));
		if (item->algorithm() == model->vtkSource(name))
			removePipelineItem(index(i, 0));
	}
}

void VtkVisPipeline::removePipelineItem( QModelIndex index )
{
	if (!index.isValid())
		return;

	//TreeItem* item = getItem(index);
	removeRows(index.row(), 1, index.parent());
	
	_renderer->ResetCamera(_renderer->ComputeVisiblePropBounds());
	emit vtkVisPipelineChanged();
}

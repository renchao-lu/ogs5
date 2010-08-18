/**
 * \file VtkVisPipeline.cpp
 * 17/2/2010 LB Initial implementation
 *
 * Implementation of VtkVisPipeline
 */

// ** INCLUDES **
#include "VtkVisPipeline.h"

#include "Model.h"
#include "TreeModel.h"
#include "MshModel.h"
#include "MshItem.h"
#include "StationTreeModel.h"
#include "VtkVisPipelineItem.h"
#include "VtkMeshSource.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkAlgorithm.h>
#include <vtkPointSet.h>
#include <vtkActor.h>
#include <vtkLight.h>

VtkVisPipeline::VtkVisPipeline( vtkRenderer* renderer, QObject* parent /*= 0*/ )
: TreeModel(parent), _renderer(renderer)
{
	QList<QVariant> rootData;
	rootData << "Object name" << "Visible";
	delete _rootItem;
	_rootItem = new TreeItem(rootData, NULL);
	//_renderer->SetBackground(1,1,1);
}

void VtkVisPipeline::addLight(const GEOLIB::Point &pos)
{
	double lightPos[3];
	for (std::list<vtkLight*>::iterator it = _lights.begin(); it != _lights.end(); ++it)
	{
		(*it)->GetPosition(lightPos);
		if (pos[0] == lightPos[0] && pos[1] == lightPos[1] && pos[2] == lightPos[2]) return;
	}
	vtkLight* l = vtkLight::New();
	l->SetPosition(pos[0], pos[1], pos[2]);
	_renderer->AddLight(l);
	_lights.push_back(l);
}

vtkLight* VtkVisPipeline::getLight(const GEOLIB::Point &pos) const
{
	double lightPos[3];
	for (std::list<vtkLight*>::const_iterator it = _lights.begin(); it != _lights.end(); ++it)
	{
		(*it)->GetPosition(lightPos);
		if (pos[0] == lightPos[0] && pos[1] == lightPos[1] && pos[2] == lightPos[2]) return (*it);
	}
	return NULL;
}

void VtkVisPipeline::removeLight(const GEOLIB::Point &pos)
{
	double lightPos[3];
	for (std::list<vtkLight*>::iterator it = _lights.begin(); it != _lights.end(); ++it)
	{
		(*it)->GetPosition(lightPos);
		if (pos[0] == lightPos[0] && pos[1] == lightPos[1] && pos[2] == lightPos[2])
		{
			_renderer->RemoveLight(*it);
			(*it)->Delete();
			_lights.erase(it);
			return;
		}
	}
}

void VtkVisPipeline::setBGColor(const GEOLIB::Color &color)
{ 
	_renderer->SetBackground(color[0]/255.0, color[1]/255.0, color[2]/255.0); 
}

QModelIndex VtkVisPipeline::getIndex( vtkActor* actor )
{
	return _actorMap.value(actor, QModelIndex());
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
}

void VtkVisPipeline::addPipelineItem(MshModel* model, const QModelIndex &idx)
{
	addPipelineItem(static_cast<MshItem*>(model->getItem(idx))->vtkSource());
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
	VtkVisPipelineItem* item = new VtkVisPipelineItem(_renderer, source, parentItem, input, itemData);
	parentItem->appendChild(item);

	int parentChildCount = parentItem->childCount();
	QModelIndex newIndex = index(parentChildCount - 1, 0, parent);

	_renderer->ResetCamera(_renderer->ComputeVisiblePropBounds());

	_actorMap.insert(item->actor(), newIndex);

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
}

void VtkVisPipeline::removeSourceItem(StationTreeModel* model, const std::string &name)
{
	for (int i = 0; i < _rootItem->childCount(); i++)
	{
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(getItem(index(i, 0)));
		if (item->algorithm() == model->vtkSource(name))
		{
			removePipelineItem(index(i, 0));
			return;
		}
	}
}

void VtkVisPipeline::removeSourceItem(MshModel* model, const QModelIndex &idx)
{
	MshItem* sItem = static_cast<MshItem*>(model->getItem(idx));
	
	for (int i = 0; i < _rootItem->childCount(); i++)
	{
		VtkVisPipelineItem* item = static_cast<VtkVisPipelineItem*>(getItem(index(i, 0)));
		if (item->algorithm() == sItem->vtkSource())
		{
			removePipelineItem(index(i, 0));
			return;
		}
	}
}

void VtkVisPipeline::removePipelineItem( QModelIndex index )
{
	if (!index.isValid())
		return;

	QMap<vtkActor*, QModelIndex>::iterator it = _actorMap.begin();
	while (it != _actorMap.end())
	{
		QModelIndex itIndex = it.value();
		if (itIndex == index)
		{
			_actorMap.erase(it);
			break;
		}
		++it;
	}

	//TreeItem* item = getItem(index);
	removeRows(index.row(), 1, index.parent());

	_renderer->ResetCamera(_renderer->ComputeVisiblePropBounds());
	emit vtkVisPipelineChanged();
}

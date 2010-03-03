/**
 * \file VtkVisPipelineItem.cpp
 * 17/2/2010 LB Initial implementation
 * 
 * Implementation of VtkVisPipelineItem
 */

// ** INCLUDES **
#include "VtkVisPipelineItem.h"

#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>

VtkVisPipelineItem::VtkVisPipelineItem(
	vtkRenderer* renderer,
	vtkPolyDataAlgorithm* algorithm,
	TreeItem* parentItem,
	vtkPolyData* input,
	const QList<QVariant> data /*= QList<QVariant>()*/)
: TreeItem(data, parentItem), _renderer(renderer), _algorithm(algorithm), _input(input)
{
	if (_input != NULL)
		_algorithm->SetInput(_input);
	Initialize();
}

VtkVisPipelineItem::~VtkVisPipelineItem()
{
	_renderer->RemoveActor(_actor);
	_actor->Delete();
	_mapper->Delete();
}

QVariant VtkVisPipelineItem::data( int column ) const
{
	if (column == 1)
	{
		return isVisible();
	}
	else
		return TreeItem::data(column);
}

bool VtkVisPipelineItem::setData( int column, const QVariant &value )
{
	if (column == 1)
	{
		setVisible(value.toBool());
		return true;
	}
	else
		return TreeItem::setData(column, value);

}
bool VtkVisPipelineItem::isVisible() const
{
	return (bool)_actor->GetVisibility();
}

void VtkVisPipelineItem::setVisible( bool visible )
{
	_actor->SetVisibility((int)visible);
	_actor->Modified();
	_renderer->Render();
}

void VtkVisPipelineItem::Initialize()
{
	_mapper = vtkPolyDataMapper::New();
	_mapper->SetInput(_algorithm->GetOutput());
	_actor = vtkActor::New();
	_actor->SetMapper(_mapper);
	_renderer->AddActor(_actor);
}

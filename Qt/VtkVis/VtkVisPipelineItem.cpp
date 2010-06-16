/**
 * \file VtkVisPipelineItem.cpp
 * 17/2/2010 LB Initial implementation
 * 
 * Implementation of VtkVisPipelineItem
 */

// ** INCLUDES **
#include "VtkVisPipelineItem.h"
#include "VtkAlgorithmProperties.h"

#include <vtkAlgorithm.h>
#include <vtkPointSet.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>


VtkVisPipelineItem::VtkVisPipelineItem(
	vtkRenderer* renderer,
	vtkAlgorithm* algorithm,
	TreeItem* parentItem,
	vtkPointSet* input,
	const QList<QVariant> data /*= QList<QVariant>()*/)
: TreeItem(data, parentItem), _renderer(renderer), _algorithm(algorithm), _input(input)
{
	//if (_input != NULL)
		//_algorithm->SetInput(_input);
		//static_cast<vtkPolyDataAlgorithm*>(_algorithm)->SetInput(_input);

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
	_mapper = vtkDataSetMapper::New();
	_mapper->SetInputConnection(0, _algorithm->GetOutputPort(0));
	_actor = vtkActor::New();
	_actor->SetMapper(_mapper);
	_renderer->AddActor(_actor);
	
	// Set pre-set properties
	VtkAlgorithmProperties* vtkProps = dynamic_cast<VtkAlgorithmProperties*>(_algorithm);
	if (vtkProps)
		setVtkProperties(vtkProps);
	
	// Copy properties from parent
	else
	{
		VtkVisPipelineItem* parentItem = dynamic_cast<VtkVisPipelineItem*>(parent());
		while (parentItem)
		{
			VtkAlgorithmProperties* parentProps = dynamic_cast<VtkAlgorithmProperties*>(parentItem->algorithm());
			if (parentProps)
			{
				setVtkProperties(parentProps);
				parentItem = NULL;
			}
			else
				parentItem = dynamic_cast<VtkVisPipelineItem*>(parentItem->parent());
		}
	}

}

void VtkVisPipelineItem::setVtkProperties(VtkAlgorithmProperties* vtkProps)
{
	if (vtkProps->GetTexture() != NULL)
	{
		_mapper->ScalarVisibilityOff();
		_actor->GetProperty()->SetColor(1,1,1); // don't colorise textures
		_actor->SetTexture(vtkProps->GetTexture());
	}
	else 
	{
		vtkSmartPointer<vtkProperty> itemProperty = vtkProps->GetProperties();
		_actor->SetProperty(itemProperty);
	}

	if (!vtkProps->GetScalarVisibility())
		_mapper->ScalarVisibilityOff();
}
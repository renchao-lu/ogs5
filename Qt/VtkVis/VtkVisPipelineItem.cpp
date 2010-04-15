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
#include <vtkPolyDataAlgorithm.h>


#include <vtkTIFFReader.h>
#include <vtkTexture.h>
#include <vtkImageData.h>
#include <vtkPlaneSource.h>
#include <vtkSmartPointer.h>
#include <vtkTextureMapToPlane.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkCylinderSource.h>
#include <vtkSphereSource.h>


VtkVisPipelineItem::VtkVisPipelineItem(
	vtkRenderer* renderer,
	vtkAlgorithm* algorithm,
	TreeItem* parentItem,
	vtkPointSet* input,
	const QList<QVariant> data /*= QList<QVariant>()*/)
: TreeItem(data, parentItem), _renderer(renderer), _algorithm(algorithm), _input(input)
{
	if (_input != NULL)
		//_algorithm->SetInput(_input);
		static_cast<vtkPolyDataAlgorithm*>(_algorithm)->SetInput(_input);

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

/// Initalises vtkMapper and vtkActor necessary for visualisation of the item and sets the item's properties
void VtkVisPipelineItem::Initialize()
{
	_mapper = vtkDataSetMapper::New();
	_mapper->SetInputConnection(0, _algorithm->GetOutputPort(0));
	_actor = vtkActor::New();
	_actor->SetMapper(_mapper);
	_renderer->AddActor(_actor);

	vtkProperty* itemProperty;
	
	if (itemProperty = dynamic_cast<VtkAlgorithmProperties*>(_algorithm)->GetProperties())
	{
		itemProperty->SetColor(0,1,0);
		_actor->SetProperty(itemProperty);
	}
	
}

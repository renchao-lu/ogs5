/**
 * \file VisualizationWidget.cpp
 * 3/11/2009 LB Initial implementation
 *
 * Implementation of VisualizationWidget
 */

// ** INCLUDES **
#include "VisualizationWidget.h"
#include "ViewWidget2d.h"
#include "GraphicsScene.h"
#include "Model.h"

#include "Point.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

VisualizationWidget::VisualizationWidget( QWidget* parent /*= 0*/ )
: QWidget(parent)
{
	setupUi(this);

	viewWidget2d = new ViewWidget2d(this);
	tab2DLayout->addWidget(viewWidget2d);

	createScene();
	
	// Vtk
	_vtkRender = vtkRenderer::New();
	vtkWidget->GetRenderWindow()->AddRenderer(_vtkRender);
	_vtkRender->SetBackground(0.0,0.0,0.0);
}

void VisualizationWidget::createScene()
{
	_graphicsScene = new GraphicsScene(this);
	viewWidget2d->graphicsView2d->setScene(_graphicsScene);
}

GraphicsScene* VisualizationWidget::scene() const
{
	return _graphicsScene;
}

void VisualizationWidget::updateViews()
{
	_vtkRender->ResetCamera();

	viewWidget2d->graphicsView2d->updateView();
	
	// TODO update vtk
}

void VisualizationWidget::showAll()
{
	viewWidget2d->on_actionShow_All_triggered();
	// TODO update vtk
}

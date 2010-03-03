	/**
 * \file VisualizationWidget.h
 * 3/11/2009 LB Initial implementation
 *
 */


#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H

// ** INCLUDES **
#include "ui_VisualizationWidgetBase.h"

class GraphicsScene;
class Model;
class ViewWidget2d;

class vtkRenderer;

/**
 * VisualizationWidget is tab widget which provides a combined 2d and/or a 3d view
 * to a GraphicsScene.
 */
class VisualizationWidget : public QWidget, public Ui_VisualizationWidgetBase
{
	Q_OBJECT

public:
	ViewWidget2d* viewWidget2d;

	/// Constructor.
	VisualizationWidget(QWidget* parent = 0);

	/// Returns the graphics scene.
	GraphicsScene* scene() const;

	public slots:
	/// Updates the 2d and the 3d view.
	void updateViews();

	/// Shows the entire scene on the views.
	void showAll();

	/// Returns the vtk renderer
	vtkRenderer* renderer() const { return _vtkRender; }

private:
	GraphicsScene* _graphicsScene;

	vtkRenderer* _vtkRender;
	
	/// Creates the scene which represents the models.
	void createScene();

};

#endif // VISUALIZATIONWIDGET_H

	/**
 * \file VisualizationWidget.h
 * 3/11/2009 LB Initial implementation
 *
 */


#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H

// ** INCLUDES **
#include "ui_VisualizationWidgetBase.h"

class vtkRenderer;

/**
 * VisualizationWidget is a widget which provides the 3d vtk scene view.
 */
class VisualizationWidget : public QWidget, public Ui_VisualizationWidgetBase
{
	Q_OBJECT

public:

	/// Constructor.
	VisualizationWidget(QWidget* parent = 0);
	~VisualizationWidget();

public slots:
	/// Updates the the 3d view.
	void updateView();

	/// Shows the entire scene on the views.
	void showAll();

	/// Returns the vtk renderer
	vtkRenderer* renderer() const { return _vtkRender; }

protected slots:
	/// Toggles stereo rendering on / off
	void on_stereoToolButton_toggled(bool checked);

	/// TODO Toggles full screen mode (actually not working)
	void on_fullscreenToolButton_clicked(bool checked);

	/// Adjusts the eye angle (separation) for stereo viewing
	void on_eyeAngleSlider_valueChanged(int value);

private:
	vtkRenderer* _vtkRender;
	
};

#endif // VISUALIZATIONWIDGET_H

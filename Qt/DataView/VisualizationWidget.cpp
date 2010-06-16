/**
 * \file VisualizationWidget.cpp
 * 3/11/2009 LB Initial implementation
 *
 * Implementation of VisualizationWidget
 */

// ** INCLUDES **
#include "VisualizationWidget.h"
#include "Configure.h"
#include "Model.h"
#include "Point.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

#include <QSettings>

VisualizationWidget::VisualizationWidget( QWidget* parent /*= 0*/ )
: QWidget(parent)
{
	setupUi(this);

	// Create renderer
	_vtkRender = vtkRenderer::New();
	_vtkRender->SetBackground(0.0,0.0,0.0);

	// BUG Render Window conflicts with VREDs render window
#ifndef OGS_VRED_PLUGIN
	vtkRenderWindow* renderWindow = vtkWidget->GetRenderWindow();
	renderWindow->StereoCapableWindowOn();
	renderWindow->SetStereoTypeToCrystalEyes();
	renderWindow->AddRenderer(_vtkRender);
#endif // OGS_VRED_PLUGIN

	// Restore settings
	QSettings settings("UFZ", "OpenGeoSys-5");
	stereoToolButton->setChecked(settings.value("stereoEnabled").toBool());
	if (settings.contains("stereoEyeAngle"))
		_vtkRender->GetActiveCamera()->SetEyeAngle(settings.value("stereoEyeAngle").toDouble());
	else
		_vtkRender->GetActiveCamera()->SetEyeAngle(2.0);

	if (!stereoToolButton->isChecked())
	{
		eyeAngleLabel->setEnabled(false);
		eyeAngleSlider->setEnabled(false);
	}

	eyeAngleSlider->setValue(_vtkRender->GetActiveCamera()->GetEyeAngle() * 10);
}

VisualizationWidget::~VisualizationWidget()
{
	// Write settings
	QSettings settings("UFZ", "OpenGeoSys-5");
	settings.setValue("stereoEnabled", stereoToolButton->isChecked());
	settings.setValue("stereoEyeAngle", _vtkRender->GetActiveCamera()->GetEyeAngle());
}
void VisualizationWidget::updateView()
{
	vtkWidget->GetRenderWindow()->Render();
}

void VisualizationWidget::showAll()
{
	_vtkRender->ResetCamera();
}

void VisualizationWidget::on_stereoToolButton_toggled( bool checked )
{
	if (checked)
	{
		vtkWidget->GetRenderWindow()->StereoRenderOn();
		eyeAngleLabel->setEnabled(true);
		eyeAngleSlider->setEnabled(true);
	}
	else
	{
		vtkWidget->GetRenderWindow()->StereoRenderOff();
		eyeAngleLabel->setEnabled(false);
		eyeAngleSlider->setEnabled(false);
	}

	updateView();
}

void VisualizationWidget::on_fullscreenToolButton_clicked( bool checked )
{
	vtkWidget->GetRenderWindow()->FullScreenOn();
}

void VisualizationWidget::on_eyeAngleSlider_valueChanged( int value )
{
	_vtkRender->GetActiveCamera()->SetEyeAngle(value / 10.0);
	updateView();
}

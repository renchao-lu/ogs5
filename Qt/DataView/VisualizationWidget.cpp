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
#include "VtkPickCallback.h"
#include "VtkCustomInteractorStyle.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkCellPicker.h>

#include <vtkInteractorStyleSwitch.h>
#include <vtkInteractorStyleRubberBandZoom.h>

#include <QSettings>

VisualizationWidget::VisualizationWidget( QWidget* parent /*= 0*/ )
: QWidget(parent)
{
	setupUi(this);

	// Create renderer
	_vtkRender = vtkRenderer::New();
	_vtkRender->SetBackground(0.0,0.0,0.0);

	_interactorStyle = VtkCustomInteractorStyle::New();
	vtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(_interactorStyle);

	_vtkPickCallback = VtkPickCallback::New();
	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->AddObserver(vtkCommand::EndPickEvent, _vtkPickCallback);
	vtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(picker);

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

	eyeAngleSlider->setValue((int)(_vtkRender->GetActiveCamera()->GetEyeAngle() * 10));
}

VisualizationWidget::~VisualizationWidget()
{
	// Write settings
	QSettings settings("UFZ", "OpenGeoSys-5");
	settings.setValue("stereoEnabled", stereoToolButton->isChecked());
	settings.setValue("stereoEyeAngle", _vtkRender->GetActiveCamera()->GetEyeAngle());

	_interactorStyle->deleteLater();
	_vtkPickCallback->deleteLater();
}
VtkCustomInteractorStyle* VisualizationWidget::interactorStyle() const
{
	return _interactorStyle;
}
VtkPickCallback* VisualizationWidget::vtkPickCallback() const
{
	return _vtkPickCallback;
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
	Q_UNUSED(checked)
	vtkWidget->GetRenderWindow()->FullScreenOn();
}

void VisualizationWidget::on_eyeAngleSlider_valueChanged( int value )
{
	_vtkRender->GetActiveCamera()->SetEyeAngle(value / 10.0);
	updateView();
}

void VisualizationWidget::on_zoomToolButton_toggled( bool checked )
{
	if (checked)
	{
		vtkSmartPointer<vtkInteractorStyleRubberBandZoom> interactorStyle =
			vtkSmartPointer<vtkInteractorStyleRubberBandZoom>::New();
		vtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorStyle);
		QCursor cursor;
		cursor.setShape(Qt::CrossCursor);
		vtkWidget->setCursor(cursor);
	}
	else
	{
		vtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(_interactorStyle);
		QCursor cursor;
		cursor.setShape(Qt::ArrowCursor);
		vtkWidget->setCursor(cursor);
	}
}

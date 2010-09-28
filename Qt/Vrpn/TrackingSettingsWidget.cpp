/**
 * \file TrackingSettingsWidget.cpp
 * 06/09/2010 LB Initial implementation
 * 
 * Implementation of TrackingSettingsWidget class
 */

// ** INCLUDES **
#include "TrackingSettingsWidget.h"
#include "VtkTrackedCamera.h"
#include "QSpaceNavigatorClient.h"
#include "QVrpnArtTrackingClient.h"

#include <QCompleter>
#include <QStringList>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSettings>

TrackingSettingsWidget::TrackingSettingsWidget(VtkTrackedCamera* cam, QWidget* parent /*= 0*/)
: QWidget(parent), _cam(cam)
{
	setupUi(this);
	
	QStringList deviceNameAtSuggestionList;
	deviceNameAtSuggestionList << "141.65.34.36" << "visserv3.intern.ufz.de" << "localhost";
	QCompleter* deviceNameAtCompleter = new QCompleter(deviceNameAtSuggestionList, this);
	deviceNameAtCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	deviceNameAtLineEdit->setCompleter(deviceNameAtCompleter);
	
	offsetXLineEdit->setValidator(new QDoubleValidator(offsetXLineEdit));
	offsetYLineEdit->setValidator(new QDoubleValidator(offsetYLineEdit));
	offsetZLineEdit->setValidator(new QDoubleValidator(offsetZLineEdit));
	realToVirtualScaleLineEdit->setValidator(new QDoubleValidator(realToVirtualScaleLineEdit));
	aspectRatioLineEdit->setValidator(new QDoubleValidator(aspectRatioLineEdit));
	screenHeightLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2, screenHeightLineEdit));
}

TrackingSettingsWidget::~TrackingSettingsWidget()
{
	
}

void TrackingSettingsWidget::on_offsetXLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setTrackingOffsetX(value);
}

void TrackingSettingsWidget::on_offsetYLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setTrackingOffsetY(value);
}
void TrackingSettingsWidget::on_offsetZLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setTrackingOffsetZ(value);
}

void TrackingSettingsWidget::on_realToVirtualScaleLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setRealToVirtualScale(value);
}

void TrackingSettingsWidget::on_aspectRatioLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setScreenAspectRatio(value);
}

void TrackingSettingsWidget::on_screenHeightLineEdit_textChanged(QString text)
{
	double value = text.toDouble();
	_cam->setScreenHeight(value);
}

void TrackingSettingsWidget::on_applyPushButton_pressed()
{
	QVrpnArtTrackingClient* art = QVrpnArtTrackingClient::Instance();
	art->StopTracking();
	QString deviceName = deviceNameLineEdit->text() + "@" + deviceNameAtLineEdit->text();
	art->StartTracking(deviceName.toStdString().c_str(), updateIntervalSpinBox->value());
	applyPushButton->setEnabled(false);
}
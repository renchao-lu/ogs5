/**
 * \file TrackingSettingsWidget.h
 * 06/09/2010 LB Initial implementation
 */

#ifndef TRACKINGSETTINGSWIDGET_H
#define TRACKINGSETTINGSWIDGET_H

#include "ui_TrackingSettingsWidgetBase.h"

class TrackingSettingsWidget : public QWidget, public Ui_TrackingSettingsWidgetBase
{
	Q_OBJECT
	
public:
	TrackingSettingsWidget(QWidget* parent = 0);
	virtual ~TrackingSettingsWidget();

protected slots:
	void on_deviceNameLineEdit_textChanged() { applyPushButton->setEnabled(true); }
	void on_deviceNameAtLineEdit_textChanged() { applyPushButton->setEnabled(true); }

private:
};

#endif // TRACKINGSETTINGSWIDGET_H

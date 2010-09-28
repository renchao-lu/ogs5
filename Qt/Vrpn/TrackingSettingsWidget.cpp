/**
 * \file TrackingSettingsWidget.cpp
 * 06/09/2010 LB Initial implementation
 * 
 * Implementation of TrackingSettingsWidget class
 */

// ** INCLUDES **
#include "TrackingSettingsWidget.h"

#include <QCompleter>
#include <QStringList>
#include <QLineEdit>

TrackingSettingsWidget::TrackingSettingsWidget(QWidget* parent /*= 0*/)
: QWidget(parent)
{
	setupUi(this);
	
	QStringList deviceNameAtSuggestionList;
	deviceNameAtSuggestionList << "141.65.34.36" << "visserv3.intern.ufz.de" << "localhost";
	QCompleter* deviceNameAtCompleter = new QCompleter(deviceNameAtSuggestionList, this);
	deviceNameAtCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	deviceNameAtLineEdit->setCompleter(deviceNameAtCompleter);
}

TrackingSettingsWidget::~TrackingSettingsWidget()
{
	
}

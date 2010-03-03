/**
 * \file ListPropertiesDialog.cpp
 * KR Initial implementation
 */

#include <QGridLayout>
#include "ListPropertiesDialog.h"
#include "DateTools.h"
#include "PropertyBounds.h"
#include "StringTools.h"

using namespace GEOLIB;


/**
 * Creates a new dialog.
 * \param db The database connection
 */
ListPropertiesDialog::ListPropertiesDialog(std::string listName, GEOModels* geoModels, QDialog* parent) : 
	QDialog(parent), _geoModels(geoModels), _listName(listName)
{
	setupDialog();
	show();
}


ListPropertiesDialog::~ListPropertiesDialog()
{
}


/// Constructs a dialog window based on the properties retrieved from the station objects
void ListPropertiesDialog::setupDialog()
{
	int i=0;
	double minVal=0, maxVal=0;

	std::vector<Point*> *stations = _geoModels->getStationVec(_listName);

	std::map<std::string, double> properties = static_cast<Station*>((*stations)[0])->getProperties();
	QGridLayout* layout = new QGridLayout;

	setWindowTitle("List Properties");

	for(std::map<std::string, double>::const_iterator it = properties.begin(); it != properties.end(); ++it)
    {
		QLabel* prop = new QLabel();
		QLineEdit* min = new QLineEdit();
		QLineEdit* max = new QLineEdit();
		prop->setText(QString::fromStdString(it->first));

		if (getPropertyBounds(stations, it->first, minVal, maxVal))
		{
			min->setText(QString::number(minVal, 'f'));
			if (prop->text().compare("date")==0) min->setText(QString::fromStdString(date2String(minVal)));

			max->setText(QString::number(maxVal, 'f'));
			if (prop->text().compare("date")==0) max->setText(QString::fromStdString(date2String(maxVal)));
		}

		propLabel.push_back(prop);
		minValue.push_back(min);
		maxValue.push_back(max);

		layout->addWidget( propLabel[i] , i, 0 );
		layout->addWidget( minValue[i]  , i, 1 );
		layout->addWidget( maxValue[i]  , i, 2 );
		i++;
	}

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(buttonBox, i+1, 1, 1, 2 );

	setLayout(layout);
}

int ListPropertiesDialog::getPropertyBounds(const std::vector<Point*> *stations, const std::string &prop, double &minVal, double &maxVal)
{
	if (stations->size()>0)
	{
		std::map<std::string, double> properties (static_cast<Station*>((*stations)[0])->getProperties());
		minVal = properties[prop];
		maxVal = properties[prop];

		size_t size = stations->size();
		for (size_t i=1; i<size; i++)
		{
			properties = static_cast<Station*>((*stations)[i])->getProperties();
			if (minVal > properties[prop]) minVal = properties[prop];
			if (maxVal < properties[prop]) maxVal = properties[prop];
		}
		return 1;
	}
	return 0;
}

/// Instructions if the OK-Button has been pressed.
void ListPropertiesDialog::accept()
{
	std::vector<PropertyBounds> bounds;
	int noProp = propLabel.size();
	double minVal, maxVal;

	for (int i=0; i<noProp; i++)
	{
		if (propLabel[i]->text().compare("date")==0)
		{
			minVal = strDate2Double(minValue[i]->text().toStdString());
			maxVal = strDate2Double(maxValue[i]->text().toStdString());
		}
		else
		{
			minVal = strtod(replaceString(",", ".", minValue[i]->text().toStdString()).c_str() ,0);
			maxVal = strtod(replaceString(",", ".", maxValue[i]->text().toStdString()).c_str(), 0);
		}
		PropertyBounds b(propLabel[i]->text().toStdString(), minVal, maxVal);
		bounds.push_back(b);
	}

	emit propertyBoundariesChanged(_listName, bounds);

	this->done(QDialog::Accepted);
}

/// Instructions if the Cancel-Button has been pressed.
void ListPropertiesDialog::reject()
{
	this->done(QDialog::Rejected);
}

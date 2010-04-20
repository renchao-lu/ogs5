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
	QDialog(parent), _listName(listName), _geoModels(geoModels)
{
	setupDialog();
	show();
}


ListPropertiesDialog::~ListPropertiesDialog()
{
	delete _buttonBox;

	size_t nProps = _propLabel.size();

	for (size_t i=0; i<nProps; i++)
	{
		delete _propLabel[i];
		delete _minValue[i];
		delete _maxValue[i];
	}
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
		QLabel* prop = new QLabel(this);
		QLineEdit* min = new QLineEdit(this);
		QLineEdit* max = new QLineEdit(this);
		prop->setText(QString::fromStdString(it->first));

		if (getPropertyBounds(stations, it->first, minVal, maxVal))
		{
			min->setText(QString::number(minVal, 'f'));
			if (prop->text().compare("date")==0) min->setText(QString::fromStdString(date2String(minVal)));

			max->setText(QString::number(maxVal, 'f'));
			if (prop->text().compare("date")==0) max->setText(QString::fromStdString(date2String(maxVal)));
		}

		_propLabel.push_back(prop);
		_minValue.push_back(min);
		_maxValue.push_back(max);

		layout->addWidget( _propLabel[i] , i, 0 );
		layout->addWidget( _minValue[i]  , i, 1 );
		layout->addWidget( _maxValue[i]  , i, 2 );
		i++;
	}

	_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	layout->addWidget(_buttonBox, i+1, 1, 1, 2 );

	setLayout(layout);
}

int ListPropertiesDialog::getPropertyBounds(const std::vector<Point*> *stations, const std::string &prop, double &minVal, double &maxVal)
{
	if (!stations->empty())
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
	int noProp = _propLabel.size();
	double minVal, maxVal;

	for (int i=0; i<noProp; i++)
	{
		if (_propLabel[i]->text().compare("date")==0)
		{
			minVal = strDate2Double(_minValue[i]->text().toStdString());
			maxVal = strDate2Double(_maxValue[i]->text().toStdString());
		}
		else
		{
			minVal = strtod(replaceString(",", ".", _minValue[i]->text().toStdString()).c_str() ,0);
			maxVal = strtod(replaceString(",", ".", _maxValue[i]->text().toStdString()).c_str(), 0);
		}
		PropertyBounds b(_propLabel[i]->text().toStdString(), minVal, maxVal);
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

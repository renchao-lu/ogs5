/**
 * \file ListPropertiesDialog.h
 * KR Initial implementation
 */

#ifndef LISTPROPERTIESDIALOG_H
#define LISTPROPERTIESDIALOG_H

#include <vector>
#include <QtGui/QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "GEOModels.h"
#include "Station.h"


/**
 * \brief A dialog for selecting a subset of a station list based on the properties of that list.
 *
 * A dialog for selecting a subset of a station list based on the properties of that list.
 * Note: Currently, this dialog only works if the list is loaded from a database.
 */
class ListPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	ListPropertiesDialog(std::string listName, GEOModels* geoModels, QDialog* parent = 0);
	~ListPropertiesDialog();

	QDialogButtonBox* buttonBox;	/// The buttons used in this dialog.
	std::vector<QLabel*> propLabel;		/// The names of the properties.
	std::vector<QLineEdit*> minValue;	/// The minimum values of each property.
	std::vector<QLineEdit*> maxValue;	/// The maximum values of each property.


private:
	int getPropertyBounds(const std::vector<GEOLIB::Point*> *stations, const std::string &prop, double &minVal, double &maxVal);
	void setupDialog();

	std::string _listName;
	GEOModels* _geoModels;

private slots:
	void accept();
	void reject();

signals:
	void propertyBoundariesChanged(std::string name, std::vector<PropertyBounds> bounds);
};

#endif //LISTPROPERTIESDIALOG_H

/**
 * \file VisPrefsDialog.cpp
 * 14/06/2010  KR Initial implementation
 */

#include <QSettings>
#include "VisPrefsDialog.h"

#include "VtkVisPipeline.h"

/// Constructor
VisPrefsDialog::VisPrefsDialog(VtkVisPipeline* pipeline, QDialog* parent) : 
	QDialog(parent), _vtkVisPipeline(pipeline), _above(0,0,2000000), _below(0,0,-2000000)
{
	setupUi(this);
	if (_vtkVisPipeline->getLight(_above))
		lightAboveBox->toggle();
	if (_vtkVisPipeline->getLight(_below))
		lightBelowBox->toggle();
}

VisPrefsDialog::~VisPrefsDialog()
{
}

void VisPrefsDialog::on_bgBlackButton_toggled()
{
	GEOLIB::Color black(0,0,0);
	_vtkVisPipeline->setBGColor(black);
}

void VisPrefsDialog::on_bgWhiteButton_toggled()
{
	GEOLIB::Color white(255, 255, 255);
	_vtkVisPipeline->setBGColor(white);
}

void VisPrefsDialog::on_lightAboveBox_clicked()
{
	if (lightAboveBox->isChecked())
		_vtkVisPipeline->addLight(_above);
	else
		_vtkVisPipeline->removeLight(_above);
}

void VisPrefsDialog::on_lightBelowBox_clicked()
{
	if (lightBelowBox->isChecked())
		_vtkVisPipeline->addLight(_below);
	else
		_vtkVisPipeline->removeLight(_below);
}

void VisPrefsDialog::accept()
{
	this->done(QDialog::Accepted);
}


void VisPrefsDialog::reject()
{
	this->done(QDialog::Rejected);
}

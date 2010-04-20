/**
 * \file VtkAddFilterDialog.cpp
 * 23/2/2010 LB Initial implementation
 *
 * Implementation of VtkAddFilterDialog
 */

// ** INCLUDES **
#include "VtkAddFilterDialog.h"
#include "VtkVisPipeline.h"
#include "VtkVisPipelineItem.h"

#include <vtkPolyDataAlgorithm.h>
#include <vtkContourFilter.h>
#include <vtkOutlineFilter.h>

#include <QModelIndex>
#include <QRadioButton>

VtkAddFilterDialog::VtkAddFilterDialog( VtkVisPipeline* pipeline, QModelIndex parentIndex, QDialog* parent /*= 0*/ )
: QDialog(parent), _pipeline(pipeline), _parentIndex(parentIndex)
{
	setupUi(this);

	VtkVisPipelineItem* parentItem = static_cast<VtkVisPipelineItem*>(_pipeline->getItem(parentIndex));
	parentTypeLineEdit->setText(parentItem->data(0).toString());
	QString outputType = QString::fromStdString(parentItem->algorithm()->GetOutputDataObject(0)->GetClassName());
	parentOutputLineEdit->setText(outputType);

	QWidget* _filterSelectWidget = new QWidget(this);

	QVBoxLayout* layout = new QVBoxLayout(_filterSelectWidget);

	QGroupBox* groupBox = new QGroupBox("Select Filter", this);
	QRadioButton* radio1 = new QRadioButton("Contour", this);
	_radioButtons.push_back(radio1);
	//radio1->setIcon()
	QRadioButton* radio2 = new QRadioButton("Outline", this);
	_radioButtons.push_back(radio2);

	radio1->setChecked(true);

	QVBoxLayout* vbox = new QVBoxLayout(groupBox);
	vbox->addWidget(radio1);
	vbox->addWidget(radio2);
	vbox->addStretch(1);

	layout->addWidget(groupBox);

	filterScrollArea->setWidget(_filterSelectWidget);
}

void VtkAddFilterDialog::on_buttonBox_accepted()
{
	foreach (QRadioButton* radioButton, _radioButtons)
	{
		if (radioButton->isChecked())
		{
			vtkContourFilter* vtkContour = vtkContourFilter::New();
			vtkContour->GenerateValues(5, 0.0, 1.0);
			vtkAlgorithm* vtkFilter = vtkOutlineFilter::New();
			_pipeline->addPipelineItem(vtkContour, _parentIndex);
		}
	}
	qDebug("clicked");
}
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
	
	QVBoxLayout* layout = new QVBoxLayout;
	
	QGroupBox* groupBox = new QGroupBox("Select Filter");
	QRadioButton* radio1 = new QRadioButton("Contour");
	//radio1->setIcon()
	QRadioButton* radio2 = new QRadioButton("Outline");
	
	radio1->setChecked(true);

	QVBoxLayout* vbox = new QVBoxLayout;
	vbox->addWidget(radio1);
	vbox->addWidget(radio2);
	vbox->addStretch(1);
	groupBox->setLayout(vbox);

	layout->addWidget(groupBox);
	_filterSelectWidget->setLayout(layout);

	filterScrollArea->setWidget(_filterSelectWidget);
	
}
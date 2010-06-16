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
#include "VtkOGSFilter.h"

#include <vtkContourFilter.h>
#include <vtkOutlineFilter.h>

#include <QModelIndex>


VtkAddFilterDialog::VtkAddFilterDialog( VtkVisPipeline* pipeline, QModelIndex parentIndex, QDialog* parent /*= 0*/ )
: QDialog(parent), _pipeline(pipeline), _parentIndex(parentIndex)
{
	setupUi(this);
	filterListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	VtkVisPipelineItem* parentItem = static_cast<VtkVisPipelineItem*>(_pipeline->getItem(parentIndex));
	parentTypeLineEdit->setText(parentItem->data(0).toString());
	QString outputType = QString::fromStdString(parentItem->algorithm()->GetOutputDataObject(0)->GetClassName());
	parentOutputLineEdit->setText(outputType);

	VtkFilterItem::VtkTargetObject source = (outputType.compare("vtkUnstructuredGrid") == 0) ? VtkFilterItem::UNSTRUCTUREDGRID : VtkFilterItem::POLYDATA;

	VtkOGSFilter filter;
	std::vector<VtkFilterItem> filters = filter.getAvailableFilters();

	size_t nFilters = filters.size();
	for (size_t i=0; i<nFilters; i++)
	{
		if (source == filters[i].target())
		{
			new QListWidgetItem(QString::fromStdString(filters[i].name()), filterListWidget);
			_applicableFilters.push_back(filters[i]);
		}
	}

	if (_applicableFilters.empty())
		this->buttonBox->setDisabled(true);
}

void VtkAddFilterDialog::on_buttonBox_accepted()
{
	VtkOGSFilter filter;
	size_t idx = filterListWidget->currentRow();
	vtkAlgorithm* item = filter.apply(static_cast<VtkVisPipelineItem*>(_pipeline->getItem(_parentIndex))->algorithm(), _applicableFilters[idx].filter());
	if (item)
		_pipeline->addPipelineItem(item, _parentIndex);
}




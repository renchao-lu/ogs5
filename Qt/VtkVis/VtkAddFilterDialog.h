/**
 * \file VtkAddFilterDialog.h
 * 23/2/2010 LB Initial implementation
 *
 */


#ifndef VTKADDFILTERDIALOG_H
#define VTKADDFILTERDIALOG_H

// ** INCLUDES **
#include "ui_VtkAddFilterDialogBase.h"

#include "VtkOGSFilter.h"

class VtkVisPipeline;
class VtkFilterItem;
class QModelIndex;
class QRadioButton;


/**
 * \brief Dialog for selecting a filter to be applied to a VtkPipelineItem.
 * The dialog lets you select filters defined in VtkOGSFilter that have been registered as VtkFilterItem - objects.
 */
class VtkAddFilterDialog : public QDialog, public Ui_VtkAddFilterDialogBase
{
	Q_OBJECT

public:
	/// Constructor
	VtkAddFilterDialog(VtkVisPipeline* pipeline, QModelIndex parentIndex, QDialog* parent = 0);

public slots:
	void on_buttonBox_accepted();
private:
	VtkVisPipeline* _pipeline;
	QModelIndex _parentIndex;
	std::vector<VtkFilterItem> _applicableFilters;
};


#endif // VTKADDFILTERDIALOG_H

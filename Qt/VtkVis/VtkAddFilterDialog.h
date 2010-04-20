/**
 * \file VtkAddFilterDialog.h
 * 23/2/2010 LB Initial implementation
 *
 */


#ifndef VTKADDFILTERDIALOG_H
#define VTKADDFILTERDIALOG_H

// ** INCLUDES **
#include "ui_VtkAddFilterDialogBase.h"

class VtkVisPipeline;
class QModelIndex;
class QRadioButton;

/**
 * VtkAddFilterDialog
 */
class VtkAddFilterDialog : public QDialog, public Ui_VtkAddFilterDialogBase
{
	Q_OBJECT

public:
	VtkAddFilterDialog(VtkVisPipeline* pipeline, QModelIndex parentIndex, QDialog* parent = 0);

public slots:
	void on_buttonBox_accepted();
private:
	VtkVisPipeline* _pipeline;
	QModelIndex _parentIndex;
	QVector<QRadioButton*> _radioButtons;

};

#endif // VTKADDFILTERDIALOG_H

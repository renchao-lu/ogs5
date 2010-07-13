/**
 * \file VtkVisTabWidget.cpp
 * 18/2/2010 LB Initial implementation
 *
 * Implementation of VtkVisTabWidget
 */

// ** INCLUDES **
#include "VtkVisTabWidget.h"
#include "VtkVisPipelineItem.h"

#include <vtkActor.h>
#include <vtkProperty.h>

#include "ColorTableModel.h"
#include "ColorTableView.h"

/* test includes */
#include "VtkMeshSource.h"
#include "VtkStationSource.h"

VtkVisTabWidget::VtkVisTabWidget( QWidget* parent /*= 0*/ )
: QWidget(parent)
{
	setupUi(this);

	connect(this->vtkVisPipelineView, SIGNAL(itemSelected(VtkVisPipelineItem*)),
		this, SLOT(setActiveItem(VtkVisPipelineItem*)));


}

void VtkVisTabWidget::setActiveItem( VtkVisPipelineItem* item )
{
	if (item)
	{
		actorPropertiesGroupBox->setEnabled(true);
		_item = item;
		vtkProperty* vtkProps = _item->actor()->GetProperty();
		diffuseColorPickerButton->setColor(vtkProps->GetDiffuseColor());
		visibleEdgesCheckBox->setChecked(vtkProps->GetEdgeVisibility());
		edgeColorPickerButton->setColor(vtkProps->GetEdgeColor());
		opacitySlider->setValue((int)(vtkProps->GetOpacity() * 100.0));
		double* scale = item->actor()->GetScale();
		scaleZ->setText(QString::number(scale[2]));

	/* test - integrating colour tables into property-window */
	VtkStationSource* test = dynamic_cast<VtkStationSource*>(_item->algorithm());
	if (test)
	{
		std::map<std::string, GEOLIB::Color> colors = test->getColorLookupTable();
		if (!colors.empty())
		{
			QVBoxLayout *vbox = new QVBoxLayout;
			this->filterPropertiesGroupBox->setLayout(vbox);
			//readColorLookupTable(colors, "d:/BoreholeColourReference.txt");
			ColorTableModel* ctm = new ColorTableModel(colors);
			ColorTableView* ctv = new ColorTableView();
			ctv->setModel(ctm);
			ctv->setItemDelegate(new ColorTableViewDelegate);
			vbox->addWidget(ctv);
			ctv->resizeRowsToContents();
		}
	}
	/* ------------------------------------------------------- */


		emit requestViewUpdate();
	}
	else
		actorPropertiesGroupBox->setEnabled(false);

}

void VtkVisTabWidget::on_diffuseColorPickerButton_colorPicked( QColor color )
{
	_item->actor()->GetProperty()->SetDiffuseColor(
		color.redF(), color.greenF(), color.blueF());

	emit requestViewUpdate();
}

void VtkVisTabWidget::on_visibleEdgesCheckBox_stateChanged( int state )
{
	if (state == Qt::Checked)
	{
		_item->actor()->GetProperty()->SetEdgeVisibility(1);
		edgeColorPickerButton->setEnabled(true);
	}
	else
	{
		_item->actor()->GetProperty()->SetEdgeVisibility(0);
		edgeColorPickerButton->setEnabled(false);
	}
	
	emit requestViewUpdate();
}

void VtkVisTabWidget::on_edgeColorPickerButton_colorPicked( QColor color )
{
	_item->actor()->GetProperty()->SetEdgeColor(
		color.redF(), color.greenF(), color.blueF());
	emit requestViewUpdate();
}

void VtkVisTabWidget::on_opacitySlider_sliderMoved( int value )
{
	_item->actor()->GetProperty()->SetOpacity(value / 100.0);
	emit requestViewUpdate();
}

void VtkVisTabWidget::on_scaleZ_textChanged(const QString &text)
{
	bool ok=true;
	double scale = text.toDouble(&ok);

	if (ok)
	{
		_item->actor()->SetScale(1, 1, scale);
		emit requestViewUpdate();
	}
}

void VtkVisTabWidget::addColorTable()
{
	
}

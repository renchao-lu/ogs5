/**
 * \file VtkVisTabWidget.h
 * 18/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISTABWIDGET_H
#define VTKVISTABWIDGET_H

// ** INCLUDES **
#include "ui_VtkVisTabWidgetBase.h"

/**
 * VtkVisTabWidget
 */
class VtkVisTabWidget : public QWidget, public Ui_VtkVisTabWidgetBase
{
	Q_OBJECT

public:
	VtkVisTabWidget(QWidget* parent = 0);



private:

};

#endif // VTKVISTABWIDGET_H

/**
 * \file MshTabWidget.h
 * 3/11/2009 LB Initial implementation
 *
 */


#ifndef MSHTABWIDGET_H
#define MSHTABWIDGET_H

// ** INCLUDES **
#include "ui_MshTabWidgetBase.h"

class GraphicsScene;

/**
 * MshTabWidget
 */
class MshTabWidget : public QWidget, public Ui_MshTabWidgetBase
{
	Q_OBJECT

public:
	MshTabWidget(QWidget* parent = 0);

	void setScene(const GraphicsScene* scene);


private slots:
	void changeMshSubmodelViews(QItemSelection selected, QItemSelection deselected);

private:
	const GraphicsScene* _scene;

};

#endif // MSHTABWIDGET_H

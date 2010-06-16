/**
 * \file MshTabWidget.h
 * 3/11/2009 LB Initial implementation
 * 18/05/2010 KR Re-Implementation
 *
 */


#ifndef MSHTABWIDGET_H
#define MSHTABWIDGET_H

// ** INCLUDES **
#include "ui_MshTabWidgetBase.h"

class MshModel;

/**
 * Tab Widget for data views on meshes
 */
class MshTabWidget : public QWidget, public Ui_MshTabWidgetBase
{
	Q_OBJECT

public:
	MshTabWidget(QWidget* parent = 0);

private slots:
	// Remove the currently selected mesh.
	void removeMesh();

	// Remove all currently loaded meshes.
	void removeAllMeshes();

//private slots:
//	void changeMshSubmodelViews(QItemSelection selected, QItemSelection deselected);

signals:
	void requestMeshRemoval(const QModelIndex&);

};

#endif // MSHTABWIDGET_H

/**
 * \file ViewWidget2d.h
 * 24/9/2009 LB Initial implementatio
 */
#ifndef VIEWWIDGET2D_H
#define VIEWWIDGET2D_H

// ** INCLUDES **
#include "ui_ViewWidget2dBase.h"
#include "Enums.h"

class GraphicsScene;
class QComboBox;

/**
 * The ViewWidget2d provides an Q2DGraphicsView and corresponding controls.
 */
class ViewWidget2d : public QMainWindow, public Ui_ViewWidget2dBase
{
	Q_OBJECT

public:
	ViewWidget2d(QWidget* parent = 0);
	
	/// Returns the scene which is displayed by this view
	GraphicsScene* scene() const;

	/// Sets the scene which is displayed by this view
	void setScene(GraphicsScene* scene);

	QSize minimumSizeHint() const;

	QComboBox* itemTypeComboBox;
	
public slots:
	/// Centers the view and the views zoom on the scene.
	void on_actionShow_All_triggered( bool checked = false );
	/// Sets the editable item type
	void setEditableItemType();
	void setEditableItemType(int index);

protected slots:
	/// Sets the viewplane of the scene.
	void setViewplane(int index);

private:


};

#endif // VIEWWIDGET2D_H

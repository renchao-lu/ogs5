/**
 * \file VtkVisPipelineView.h
 * 18/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINEVIEW_H
#define VTKVISPIPELINEVIEW_H

// ** INCLUDES **
#include <QTreeView>

/**
 * VtkVisPipelineView
 */
class VtkVisPipelineView : public QTreeView
{
	Q_OBJECT

public:
	VtkVisPipelineView(QWidget* parent = 0);



private:
	void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void removeSelectedPipelineItem();
	void addPipelineFilterItem();

signals:
	void requestRemovePipelineItem(QModelIndex);
	void requestAddPipelineFilterItem(QModelIndex);

};

#endif // VTKVISPIPELINEVIEW_H

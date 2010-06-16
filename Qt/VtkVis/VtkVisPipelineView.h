/**
 * \file VtkVisPipelineView.h
 * 18/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINEVIEW_H
#define VTKVISPIPELINEVIEW_H

// ** INCLUDES **
#include <QTreeView>

class QItemSelection;
class VtkVisPipelineItem;

/**
 * VtkVisPipelineView
 */
class VtkVisPipelineView : public QTreeView
{
	Q_OBJECT

public:
	VtkVisPipelineView(QWidget* parent = 0);

protected slots:
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	void contextMenuEvent(QContextMenuEvent* event);

private slots:
	void removeSelectedPipelineItem();
	void addPipelineFilterItem();

signals:
	void requestRemovePipelineItem(QModelIndex);
	void requestAddPipelineFilterItem(QModelIndex);
	void itemSelected(VtkVisPipelineItem*);

};

#endif // VTKVISPIPELINEVIEW_H

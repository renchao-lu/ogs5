/**
 * \file StationTreeView.h
 * KR Initial implementation
 */

#ifndef QSTATIONTREEVIEW_H
#define QSTATIONTREEVIEW_H

#include <QTreeView>
#include <QContextMenuEvent>
#include "PropertyBounds.h"

/**
 * \brief A view for the StationTreeModel with a number of properties adequate for this kind of data
 * \sa StationTreeModel, ModelTreeItem
 */
class StationTreeView : public QTreeView
{
	Q_OBJECT

public:
	/// Constructor
	StationTreeView(QWidget* parent = 0);
	~StationTreeView();

	/// Update the view to visualise changes made to the underlying data
	void updateView();

public slots:
	//void filterStations(std::vector<PropertyBounds> bounds);

protected slots:
	/// Instructions if the selection of items in the view has changed.
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	/// Instructions if the selection of items in the view has changed by events outside the view (i.e. by actions made in the visualisation).
	void selectionChangedFromOutside(const QItemSelection &selected, const QItemSelection &deselected);

private:
	/// Actions to be taken after a right mouse click is performed in the station view.
	void contextMenuEvent( QContextMenuEvent* e );

private slots:
	void on_Clicked(QModelIndex idx);
	void exportStation();
	void removeStationList();
	void showPropertiesDialog();

signals:
	void itemSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
	void propertiesDialogRequested(std::string name);
	void stationListRemoved(std::string name);

};

#endif //QSTATIONTREEVIEW_H

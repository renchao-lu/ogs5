/**
 * \file StationTreeModel.h
 * KR Initial implementation
 */

#ifndef QSTATIONTREEMODEL_H
#define QSTATIONTREEMODEL_H


#include <vector>
#include <QItemSelection>

#include "Point.h"
#include "TreeModel.h"
#include "ModelTreeItem.h"
#include <vtkPolyDataAlgorithm.h>

namespace GEOLIB {
	class Station;
	class StationBorehole;
}

class QString;
class QModelIndex;
class QGraphicsItem;
class ModelItem;
class PropertyBounds;

/**
 * \brief A model for the QStationTreeView implementing a tree as a double-linked list.
 * 
 * A model for the QStationTreeView implementing a tree as a double-linked list.
 * In addition to a simple TreeModel each item also contains a 2D / 3D GraphicsItem for visualization.
 * \sa TreeModel, StationTreeView, TreeItem, ModelTreeItem
 */
class StationTreeModel : public TreeModel
{
	Q_OBJECT

public:
	StationTreeModel(QObject* parent = 0);
	~StationTreeModel();

	void addStationList(QString listName, const std::vector<GEOLIB::Point*> *stations);
	void filterStations(const std::string &name, const std::vector<GEOLIB::Point*> *stations, const std::vector<PropertyBounds> &bounds);
	const std::vector<ModelTreeItem*> &getLists() { return _lists; }
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	ModelItem* itemFromIndex( const QModelIndex& index ) const;
	void removeStationList(QModelIndex index);
	void removeStationList(const std::string &name);
	GEOLIB::Station* stationFromIndex( const QModelIndex& index, QString &listName ) const;
	void setSelection( const QItemSelection & selected, const QItemSelection & deselected );
	vtkPolyDataAlgorithm* vtkSource(const std::string &name);

public slots:
	void setSelectionFromOutside( const QItemSelection & selected, const QItemSelection & deselected );

private:
	//QModelIndex getItemByName(const QModelIndex &index, const std::string &name) const;
	void updateSelection( const QItemSelection &selected, QVector<QGraphicsItem*> &selectedItems, const QItemSelection &deselected, QVector<QGraphicsItem*> &deselectedItems );

	QItemSelection _selectedItems;
	std::vector<ModelTreeItem*> _lists;


signals:
	void updateScene();

	void itemsSelectedFromOutside(QVector<QGraphicsItem*> item);
	void itemsDeselectedFromOutside(QVector<QGraphicsItem*> item);

};

#endif //QSTATIONTREEMODEL_H

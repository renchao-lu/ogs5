/**
 * \file PntsModel.h
 * 24/9/2009 LB Initial implementation
 */
#ifndef PNTSMODEL_H
#define PNTSMODEL_H

// ** INCLUDES **
#include "Point.h"
#include "Model.h"
#include <QVector>

class IGeometryPoint;
class ModelItem;
class PntGraphicsItem2d;
class QItemSelection;

/**
 * The PntsModel is a Qt model which represents Point objects.
 * Item indexes hold references to ModelItem as internal pointers.
 */
class PntsModel : public Model
{
	Q_OBJECT

public:
	PntsModel(QString name, std::vector<GEOLIB::Point*>* pntVec, QObject* parent = 0);
	~PntsModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

	/// Erases the corresponding entries in the internal points data structure.
	/// At the moment this is gli_points_vector. At the end Model::removeRows()
	/// is called.
	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex() );

	/// Returns a QModelIndex from a GEOLIB::Point.
	QModelIndex indexFromPoint(const GEOLIB::Point* pnt) const;

public slots:
	/// Reloads all items.
	void updateData();

protected slots:
	/**
	 * Every 2d pnt item is connected to this slot which emit a dataChanged
	 * signal with the index of the item.
	 */
	void item2dChanged(GEOLIB::Point* point);

private:
	std::vector<GEOLIB::Point*>* _pntVec;

signals:
	//void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif // PNTSMODEL_H
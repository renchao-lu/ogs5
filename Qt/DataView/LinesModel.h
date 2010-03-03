/**
 * \file PolylinesModel.h
 * 24/9/2009 LB Initial implementation
 */

#ifndef LINESMODEL_H
#define LINESMODEL_H

#include "Model.h"
#include "Polyline.h"

#include <QVector>

class LineGraphicsItem2d;
class QItemSelection;
class ModelItem;

/**
 * The PolylinesModel is a Qt model which represents  CGLPolylines. Item
 * indexes hold references to ModelItem as internal pointers.
 */
class PolylinesModel : public Model
{
	Q_OBJECT

public:
	/**
	 * \param pntsModel The corresponding points model
	 * \param parent The parent Qt object
	 */
	PolylinesModel( QString name, std::vector<GEOLIB::Polyline*>* polylineVec, /* PntsModel* const pntsModel, */ QObject* parent = 0 );
	~PolylinesModel();

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

	/// Returns a QModelIndex from a CGLPolyline.
	QModelIndex indexFromPolyline(const GEOLIB::Polyline* line) const;

public slots:
	/// Reloads all items.
	void updateData();

protected slots:
	void item2dChanged(GEOLIB::Polyline* line);

private:
	std::vector<GEOLIB::Polyline*>* _polylineVec;
	//PntsModel* const _pntsModel;

signals:
	void itemPositionChanged(GEOLIB::Polyline* line);
	void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};
#endif // LINESMODEL_H

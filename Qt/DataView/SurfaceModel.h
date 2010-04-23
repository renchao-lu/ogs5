/**
 * \file SurfaceModel.h
 * 23/04/2010 KR Initial implementation
 *
 */

#ifndef SURFACEMODEL_H
#define SURFACEMODEL_H

#include "Model.h"
#include "Surface.h"

#include <QVector>

class QItemSelection;
class ModelItem;

/**
 * The SurfaceModel is a Qt model which represents Surfaces. Item
 * indexes hold references to ModelItem as internal pointers.
 */
class SurfaceModel : public Model
{
	Q_OBJECT

public:
	/**
	 * \param pntsModel The corresponding points model
	 * \param parent The parent Qt object
	 */
	SurfaceModel( QString name, std::vector<GEOLIB::Surface*> *surfaceVec, /* PntsModel* const pntsModel, */ QObject* parent = 0 );
	~SurfaceModel();

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

	/// Returns a QModelIndex from a Surface.
	QModelIndex indexFromSurface(const GEOLIB::Surface* surface) const;

public slots:
	/// Reloads all items.
	void updateData();

protected slots:
	void item2dChanged(GEOLIB::Surface* surface);

private:
	std::vector<GEOLIB::Surface*>* _surfaceVec;
	//PntsModel* const _pntsModel;

signals:
	void itemPositionChanged(GEOLIB::Surface* surface);
	void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};
#endif // SURFACEMODEL_H

/**
 * \file SurfaceModel.h
 * 23/04/2010 KR Initial implementation
 *
 */

#ifndef SURFACEMODEL_H
#define SURFACEMODEL_H

#include "Model.h"
#include "TreeItem.h"
#include "Surface.h"

#include <QVector>

/**
 * The SurfaceModel is a Qt model which represents Polylines. 
 */
class SurfaceModel : public Model
{
	Q_OBJECT

public:
	SurfaceModel( QString name, std::vector<GEOLIB::Surface*>* surfaceVec, QObject* parent = 0 );
	~SurfaceModel();

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

	/// Returns a QModelIndex from a GEOLIB::Surface.
	QModelIndex indexFromSurface(const GEOLIB::Surface* surface) const;

public slots:
	/// Reloads all items.
	void updateData();

protected slots:
	void item2dChanged(GEOLIB::Surface* surface);

private:
	void setData(std::vector<GEOLIB::Surface*> *surfaces, TreeItem* parent);

	std::vector<GEOLIB::Surface*>* _surfaceVec;


signals:
	void itemPositionChanged(GEOLIB::Surface* surface);
	//void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};
#endif // SURFACEMODEL_H

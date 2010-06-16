/**
 * \file PolylinesModel.h
 * 24/9/2009 LB Initial implementation
 * 05/05/2010 KR 2d graphic functionality removed and various layout changes
 */

#ifndef LINESMODEL_H
#define LINESMODEL_H

#include "Model.h"
#include "TreeItem.h"
#include "Polyline.h"

#include <QVector>

/**
 * The PolylinesModel is a Qt model which represents Polylines. 
 */
class PolylinesModel : public Model
{
	Q_OBJECT

public:
	PolylinesModel( QString name, std::vector<GEOLIB::Polyline*>* polylineVec, QObject* parent = 0 );
	~PolylinesModel();

	int columnCount(const QModelIndex& parent = QModelIndex()) const;

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

	/// Returns a QModelIndex from a GEOLIB::Polyline.
	QModelIndex indexFromPolyline(const GEOLIB::Polyline* line) const;

public slots:
	/// Reloads all items.
	void updateData();

protected slots:
	void item2dChanged(GEOLIB::Polyline* line);

private:
	void setData(std::vector<GEOLIB::Polyline*> *lines, TreeItem* parent);

	std::vector<GEOLIB::Polyline*>* _polylineVec;


signals:
	void itemPositionChanged(GEOLIB::Polyline* line);
	//void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};
#endif // LINESMODEL_H

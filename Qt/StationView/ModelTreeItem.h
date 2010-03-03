/**
 * \file ModelTreeItem.h
 * KR Initial implementation
 */

#ifndef QMODELTREEITEM_H
#define QMODELTREEITEM_H

#include "TreeItem.h"
#include "Station.h"

class ModelItem;

/**
 * \brief A TreeItem including a ModelItem for visualisation in 2D and 3D.
 * \sa TreeItem, ModelItem
 */
class ModelTreeItem : public TreeItem
{
public:
	ModelTreeItem(const QList<QVariant> &data, TreeItem *parent, ModelItem* item = 0);
	~ModelTreeItem();

	GEOLIB::Station* getStation();
	ModelItem* getItem() const;
	void setStation(GEOLIB::Station* stn);
	void setItem( ModelItem* item );
	

private:
	ModelItem* _item;
	GEOLIB::Station* _stn;
};

#endif //QMODELTREEITEM_H

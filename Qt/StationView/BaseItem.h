/**
 * \file BaseItem.h
 * 20/01/2010 KR Initial implementation
 *
 */
#ifndef BASEITEM_H
#define BASEITEM_H

#include <QGraphicsScene>
#include "GraphicsItem2d.h"
#include "Point.h"

#include <vtkPolyDataAlgorithm.h>
#include "VtkStationSource.h"

/**
 * \brief A BaseItem contains additional Information about a subtree in the StationTreeModel.
 *
 * A BaseItem is the GraphicItem-Equivalent for the ModelItem of a normal TreeItem. It is used for list names
 * in the StationTreeModel and has the special properties that it is NOT visualised and that it contains the
 * vtkObject for visualisation of the whole list in 3D.
 */
class BaseItem : public GraphicsItem2d
{
	Q_OBJECT

public:
	BaseItem( const std::vector<GEOLIB::Point*> *stations = NULL, QGraphicsItem* parent = 0 )
		: GraphicsItem2d(NULL), _stations(stations), _vtkSource (VtkStationSource::New())
	{
		setVisible(false);

		Q_UNUSED (parent)

		// create the vtk-object for 3d-visualisation of this list
//		_vtkSource = VtkStationSource::New();
		static_cast<VtkStationSource*>(_vtkSource)->setStations(stations);
	}
	~BaseItem()
	{
		_vtkSource->Delete();
	}


	QRectF boundingRect() const
	{
		QRectF b(0, 0, 0, 0);
		return b;
	}

	const std::vector<GEOLIB::Point*> *getStations() { return _stations; }

	virtual void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
		Q_UNUSED (painter)
		Q_UNUSED (option)
		Q_UNUSED (widget)
	}

	virtual void setEditable(bool enable) { Q_UNUSED (enable) }

	virtual void updatePosition() {};

	/// Returns the Vtk polydata source object
	vtkPolyDataAlgorithm* vtkSource() const { return _vtkSource; }

private:
	const std::vector<GEOLIB::Point*> *_stations;

	/// The Vtk data source object. This is the starting point for a Vtk data
	/// visualization pipeline.
	vtkPolyDataAlgorithm* _vtkSource;

};

#endif //BASEITEM_H

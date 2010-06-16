/**
 * \file SurfaceModel.cpp
 * 23/04/2010 KR Initial implementation
 *
 */

#include "SurfaceModel.h"

#include "VtkSurfacesSource.h"

SurfaceModel::SurfaceModel( QString name, std::vector<GEOLIB::Surface*>* surfaceVec, QObject* parent /*= 0*/ )
: Model(name, parent), _surfaceVec(surfaceVec)
{
	QList<QVariant> rootData;
	delete _rootItem;
	rootData << "Id" << "x" << "y" << "z";
	_rootItem = new TreeItem(rootData, NULL);
	setData(surfaceVec, _rootItem);

	_vtkSource = VtkSurfacesSource::New();
	static_cast<VtkSurfacesSource*>(_vtkSource)->setSurfaces(surfaceVec);
}

SurfaceModel::~SurfaceModel()
{
	_vtkSource->Delete();
}

int SurfaceModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 4;
}

void SurfaceModel::setData(std::vector<GEOLIB::Surface*> *surfaces, TreeItem* parent)
{
	int nSurfaces = surfaces->size();
	for (int i=0; i<nSurfaces; i++)
	{
		QList<QVariant> surface;
		surface << "Surface " + QString::number(i);
		TreeItem* surfaceItem = new TreeItem(surface, _rootItem);
		_rootItem->appendChild(surfaceItem);

		const std::vector<GEOLIB::Point*> *nodesVec = (*surfaces)[i]->getPointVec();

		int nElems = static_cast<int>((*surfaces)[i]->getNTriangles());
		for (int j=0; j<nElems; j++)
		{
			QList<QVariant> elem;
			elem << j << static_cast<int>((*(*(*surfaces)[i])[j])[0]) << static_cast<int>((*(*(*surfaces)[i])[j])[1]) << static_cast<int>((*(*(*surfaces)[i])[j])[2]);
			TreeItem* child = new TreeItem(elem, surfaceItem);
			surfaceItem->appendChild(child);

			for (int k=0; k<3; k++)
			{
				QList<QVariant> node;
				node << static_cast<int>((*(*(*surfaces)[i])[j])[k]) << QString::number((*(*nodesVec)[(*(*(*surfaces)[i])[j])[k]])[0],'f') << QString::number((*(*nodesVec)[(*(*(*surfaces)[i])[j])[k]])[1],'f') << QString::number((*(*nodesVec)[(*(*(*surfaces)[i])[j])[k]])[2],'f');
				TreeItem* nchild = new TreeItem(node, child);
				child->appendChild(nchild);
			}
		}
	}

	reset();
}

bool SurfaceModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
	return false;
}

void SurfaceModel::updateData()
{
	clearData();
	TreeModel::updateData();
}


QModelIndex SurfaceModel::indexFromSurface( const GEOLIB::Surface* surface ) const
{
	if (surface != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex surfaceIndex = index(i, 0);

			if (surface == (*_surfaceVec)[i])
				return surfaceIndex;
		}
	}
	return QModelIndex();
}

void SurfaceModel::item2dChanged( GEOLIB::Surface* surface )
{
	QModelIndex itemIndex = indexFromSurface(surface);
	QModelIndex indexEnd = index(itemIndex.row(), columnCount());
	emit dataChanged(itemIndex, indexEnd);
}


/**
 * \file PolylinesModel.cpp
 * 24/9/2009 LB Initial implementation
 * 05/05/2010 KR 2d graphic functionality removed and various layout changes
 *
 * Implementation of PolylinesModel
 */

#include "LinesModel.h"

#include "VtkPolylinesSource.h"


PolylinesModel::PolylinesModel( QString name, std::vector<GEOLIB::Polyline*>* polylineVec, QObject* parent /*= 0*/ )
: Model(name, parent), _polylineVec(polylineVec)
{
	QList<QVariant> rootData;
	delete _rootItem;
	rootData << "Id" << "x" << "y" << "z";
	_rootItem = new TreeItem(rootData, NULL);
	setData(polylineVec, _rootItem);

	_vtkSource = VtkPolylinesSource::New();
	static_cast<VtkPolylinesSource*>(_vtkSource)->setPolylines(polylineVec);
}

PolylinesModel::~PolylinesModel()
{
	_vtkSource->Delete();
}

int PolylinesModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 4;
}

void PolylinesModel::setData(std::vector<GEOLIB::Polyline*> *lines, TreeItem* parent)
{
	Q_UNUSED(parent)

	int nLines = static_cast<int>(lines->size());
	for (int i=0; i<nLines; i++)
	{
		QList<QVariant> line;
		line << "Line " + QString::number(i);
		TreeItem* lineItem = new TreeItem(line, _rootItem);
		_rootItem->appendChild(lineItem);

		int nPoints = static_cast<int>((*lines)[i]->getSize());
		for (int j=0; j<nPoints; j++)
		{
			QList<QVariant> pnt;
			pnt << j << QString::number((*(*(*lines)[i])[j])[0],'f') << QString::number((*(*(*lines)[i])[j])[1],'f') << QString::number((*(*(*lines)[i])[j])[2],'f');
			TreeItem* child = new TreeItem(pnt, lineItem);
			lineItem->appendChild(child);
		}
	}

	reset();
}

bool PolylinesModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
	Q_UNUSED(index)
	Q_UNUSED(value)
	Q_UNUSED(role)
	return false;
}

void PolylinesModel::updateData()
{
	clearData();
	TreeModel::updateData();
}


QModelIndex PolylinesModel::indexFromPolyline( const GEOLIB::Polyline* line ) const
{
	if (line != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex lineIndex = index(i, 0);

			if (line == (*_polylineVec)[i])
				return lineIndex;
		}
	}
	return QModelIndex();
}

void PolylinesModel::item2dChanged( GEOLIB::Polyline* line )
{
	QModelIndex itemIndex = indexFromPolyline(line);
	QModelIndex indexEnd = index(itemIndex.row(), columnCount());
	emit dataChanged(itemIndex, indexEnd);
}


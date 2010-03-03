/**
 * \file PolylinesModel.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of PolylinesModel
 */

#include "LinesModel.h"
#include "PntGraphicsItem2d.h"
#include "LineGraphicsItem2d.h"
#include "ModelItem.h"

#include "VtkPolylinesSource.h"

PolylinesModel::PolylinesModel( QString name, std::vector<GEOLIB::Polyline*>* polylineVec, /* PntsModel* pntsModel, */ QObject* parent /*= 0*/ )
: Model(name, parent), /* _pntsModel(pntsModel), */ _polylineVec(polylineVec)
{
	_modelContentType = LINES_MODEL;
	_vtkSource = VtkPolylinesSource::New();
	static_cast<VtkPolylinesSource*>(_vtkSource)->setPolylines(polylineVec);

	updateData();
}

PolylinesModel::~PolylinesModel()
{
	_vtkSource->Delete();
}

int PolylinesModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 1;
}

QVariant PolylinesModel::data( const QModelIndex& index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	size_t numPolylines = _polylineVec->size();
	if ((size_t)index.row() >= numPolylines)
		return QVariant();

	GraphicsItem2d* item2d = itemFromIndex(index)->item2d();
	if (item2d == NULL)
		return QVariant();
	LineGraphicsItem2d* lineItem = static_cast<LineGraphicsItem2d*>(item2d);
	GEOLIB::Polyline* line = lineItem->line();
	if (line == NULL)
		return QVariant();

	//QString pntIds;
	//for (int i = 0; i < numPolylines; i++)
	//{
	//	pntIds.append(QString::number(_polylineVec[i]->id)).append(" ");
	//}

	QString pntsString;
	size_t numPnts = line->getSize();

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			for (size_t i = 0; i < numPnts; i++)
				pntsString.append(QString::fromStdString((*line)[i]->write())).append("\n");
			return pntsString;
		default:
			return QVariant();
		}
		break;

	case Qt::ToolTipRole:
		//return QString("Polyline '(%1)' from points (%2) with Id=%3")
		//	.arg(QString::fromStdString(line->name)).arg(pntIds).arg(line->id);
		return QVariant();

	default:
		return QVariant();
	}
}

QVariant PolylinesModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return "Points";
		//case 1: return "Name";
		//case 2: return "Point ids";
		default: return QVariant();
		}
	}
	else
		return QString("Row %1").arg(section);
}

bool PolylinesModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
	return false;
}

void PolylinesModel::updateData()
{
	clearData();

	for (vector<GEOLIB::Polyline*>::const_iterator it = _polylineVec->begin();
	it != _polylineVec->end(); ++it)
	{
		LineGraphicsItem2d* lineItem2d = new LineGraphicsItem2d(this, *it);
		ModelItem* item = new ModelItem(lineItem2d, this);
		size_t numPoints = (*it)->getSize();
		/*
		for (int i = 0; i < numPoints; i++)
		{
			PntGraphicsItem2d* pntItem = static_cast<PntGraphicsItem2d*>
				(_pntsModel->itemFromIndex(_pntsModel->indexFromPoint((**it)[i]))->item2d());
			connect(pntItem, SIGNAL(itemPositionChanged(IGeometryPoint*)),
				lineItem3d, SLOT(updatePosition()));
			// TODO
			//connect(pntItem, SIGNAL(itemPositionChanged(IGeometryPoint*)),
			//	lineAdapter, SLOT(computePositions()));
			connect(pntItem, SIGNAL(itemPositionChanged(IGeometryPoint*)),
				lineItem2d, SLOT(updatePosition()));

			connect(lineItem2d, SIGNAL(itemPositionChanged(IGeometryLine*)),
				pntItem, SLOT(updatePosition()));
		}
		*/
		//connect(lineItem2d, SIGNAL(itemPositionChanged(IGeometryLine*)),
		//	this, SLOT(item2dChanged(IGeometryLine*)));
		_data.push_back(item);
	}
	Model::updateData();
}


QModelIndex PolylinesModel::indexFromPolyline( const GEOLIB::Polyline* line ) const
{
	if (line != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex lineIndex = index(i, 0);
			LineGraphicsItem2d* itemFromIndex =
				static_cast<LineGraphicsItem2d*>(
				static_cast<ModelItem*>(lineIndex.internalPointer())->item2d());

			if (line == itemFromIndex->line())
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

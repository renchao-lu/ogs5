/**
 * \file PntsModel.cpp
 * 24/9/2009 LB Initial implementation
 *
 * Implementation of PntsModel
 */

// ** INCLUDES **
#include "PntsModel.h"
#include "geo_pnt.h"
#include "GEOObjects.h"

#include "ModelItem.h"
#include "PntGraphicsItem2d.h"

#include "VtkPointsSource.h"

PntsModel::PntsModel( QString name, std::vector<GEOLIB::Point*>* pntVec, QObject* parent /*= 0*/ )
: Model(name, parent), _pntVec(pntVec)
{
	_modelContentType = PNTS_MODEL;
	_vtkSource = VtkPointsSource::New();
	static_cast<VtkPointsSource*>(_vtkSource)->setPoints(pntVec);

	updateData();
}

PntsModel::~PntsModel()
{
	_vtkSource->Delete();
}

int PntsModel::columnCount( const QModelIndex &parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 5;
}
QVariant PntsModel::data( const QModelIndex& index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if ((size_t)index.row() >= _pntVec->size())
		return QVariant();


	GraphicsItem2d* item2d = itemFromIndex(index)->item2d();
	if (item2d == NULL)
		return QVariant();

	GEOLIB::Point* point = static_cast<PntGraphicsItem2d*>(item2d)->point();
	if (point == NULL)
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
//		case 0:
//			return QString::fromStdString(point->name);
//		case 1:
//            return (int)point->id;
		case 2:
			return (*point)[0];
		case 3:
			return (*point)[1];
		case 4:
			return (*point)[2];
		default:
			return QVariant();
		}
		break;

	case Qt::ToolTipRole:
		return QString("(%1, %2, %3)").arg((*point)[0]).arg((*point)[1]).arg((*point)[2]);

	default:
		return QVariant();
	}

}

QVariant PntsModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return "Name";
		case 1: return "Id";
		case 2: return "x";
		case 3: return "y";
		case 4: return "z";
		default: return QVariant();
		}
	}
	else
		return QString("Row %1").arg(section);
}

bool PntsModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
	if (index.isValid() && role == Qt::EditRole)
	{
		GEOLIB::Point* point = _pntVec->at(index.row());
		bool wasConversionSuccesfull = false;
		//int id;
		double x, y, z;
		switch (index.column())
		{
//		case 0:
//			point->name = value.toString().toStdString();
//		case 1:
//			id = value.toInt(&wasConversionSuccesfull);
//			if (wasConversionSuccesfull)
//				point->id = id;
//			else
//				return false;
//			break;
		case 2:
			x = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				(*point)[0] = x;
			else
				return false;
			break;
		case 3:
			y = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				(*point)[1] = y;
			else
				return false;
			break;
		case 4:
			z = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				(*point)[2] = z;
			else
				return false;
			break;
		default:
			return false;
		}

		emit dataChanged(index, index);
		return true;
	}
	return false;
}

void PntsModel::updateData()
{
	clearData();

	for (vector<GEOLIB::Point*>::const_iterator it = _pntVec->begin();
		it != _pntVec->end(); ++it)
	{
		PntGraphicsItem2d* pntItem2d = new PntGraphicsItem2d(this, *it);
		ModelItem* item = new ModelItem(pntItem2d, this);
		connect(pntItem2d, SIGNAL(itemPositionChanged(GEOLIB::Point*)),
			this, SLOT(item2dChanged(GEOLIB::Point*)));
		_data.push_back(item);
	}

	Model::updateData();
	//emit dataChanged(QModelIndex(), QModelIndex());
}

QModelIndex PntsModel::indexFromPoint( const GEOLIB::Point* pnt ) const
{
	if (pnt != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex pntIndex = index(i, 0);
			PntGraphicsItem2d* itemFromIndex =
				static_cast<PntGraphicsItem2d*>(
				static_cast<ModelItem*>(pntIndex.internalPointer())->item2d());

			if (pnt == itemFromIndex->point())
				return pntIndex;
		}
	}
	return QModelIndex();
}

void PntsModel::item2dChanged( GEOLIB::Point* point )
{
	QModelIndex itemIndex = indexFromPoint(point);
	QModelIndex indexEnd = index(itemIndex.row(), columnCount());
	emit dataChanged(itemIndex, indexEnd);
}

bool PntsModel::removeRows( int row, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
	//for (int i = count; i > 0; i--)
	//	_pntVec->erase(_pntVec->begin() + row + i - 1);
	// TODO send signal

	return Model::removeRows(row, count, parent);
}

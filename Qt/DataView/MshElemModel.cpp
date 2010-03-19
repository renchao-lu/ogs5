/**
 * \file MshElemModel.cpp
 * 22/10/2009 LB Initial implementation
 *
 * Implementation of MshElemModel
 */

// ** INCLUDES **
#include "MshElemModel.h"
#include "GraphicsItem2d.h"
#include "ModelItem.h"
#include "LineGraphicsItem2d.h"

MshElemModel::MshElemModel( QString name, Mesh_Group::CFEMesh* msh, QObject* parent /*= 0*/ )
: Model(name, parent), _msh(msh)
{
	_modelContentType = MSH_ELEM_MODEL;
	updateData();
}

int MshElemModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 1;
}

QVariant MshElemModel::data( const QModelIndex& index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= (int)_msh->ele_vector.size())
		return QVariant();

	Mesh_Group::CElem* elem = _msh->ele_vector[index.row()];

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return geoTypeString(elem->GetElementType());
		default:
			return QVariant();
		}
		break;

	case Qt::ToolTipRole:
		//return QString("(%1, %2, %3), Id: %4").arg(point->x).arg(point->y).arg(point->z).arg(point->id);

	default:
		return QVariant();
	}
}

QVariant MshElemModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return "Type";
		default: return QVariant();
		}
	}
	else
		return QString("Row %1").arg(section);
}

void MshElemModel::updateData()
{
	clearData();

	for (vector<Mesh_Group::CElem*>::const_iterator it = _msh->ele_vector.begin();
		it != _msh->ele_vector.end(); ++it)
	{
		ModelItem* item = new ModelItem(NULL, NULL);
		_data.push_back(item);
	}
	Model::updateData();
}

const QString MshElemModel::geoTypeString( int type ) const
{
	switch (type)
	{
	case 1: return "Line";
	case 2: return "Quad";
	case 3: return "Hex";
	case 4: return "Tri";
	case 5: return "Tet";
	case 6: return "Pris";
	default: return tr("Unspecified");
	}
}

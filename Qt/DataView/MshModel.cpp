/**
 * \file MshModel.cpp
 * 19/10/2009 LB Initial implementation
 *
 * Implementation of MshModel
 */

// ** INCLUDES **
#include "MshModel.h"
#include "ModelItem.h"
#include "MshGraphicsItem2d.h"
#include "MshGraphicsItem3d.h"
#include "MshNodeModel.h"
#include "MshElemModel.h"

#include "msh_mesh.h"
#include "msh_lib.h"

MshModel::MshModel( QString name, QObject* parent /*= 0*/ )
: Model(name, parent)
{
	_modelContentType = MSH_MODEL;
}

int MshModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 3;
}

QVariant MshModel::data( const QModelIndex& index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= fem_msh_vector.size())
		return QVariant();

	// Get msh here...
	MshGraphicsItem3d* msh3dItem = static_cast<MshGraphicsItem3d*>(itemFromIndex(index)->item3d());
	Mesh_Group::CFEMesh* msh = msh3dItem->msh();

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return QString::fromStdString(msh->pcs_name);
		case 1:
			return QString::fromStdString(msh->geo_name);
		case 2:
			return QString::fromStdString(msh->geo_type_name);
		default:
			return QVariant();
		}
		break;

	case Qt::ToolTipRole:
		return QString("Add msh tooltip here...");

	default:
		return QVariant();
	}

}

QVariant MshModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return "PCS";
		case 1: return "GEO";
		case 2: return "GEO type";
		default: return QVariant();
		}
	}
	else
		return QString("Row %1").arg(section);
}

bool MshModel::setData( const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/ )
{
	return false;
}

void MshModel::updateData()
{
	//clearData();

	//_subModels.clear();

	for (vector<Mesh_Group::CFEMesh*>::const_iterator it = fem_msh_vector.begin() + _data.size();
		it != fem_msh_vector.end(); ++it)
	{

		MshNodeModel* nodeModel = new MshNodeModel(this->name().append("_Nodes"), *it, this);
		//_subModels.push_back(nodeModel);

		MshElemModel* elemModel = new MshElemModel(this->name().append("_Elements"), *it, this);
		//_subModels.push_back(elemModel);

		MshGraphicsItem2d* mshItem2d = new MshGraphicsItem2d(this, *it);
		connect(nodeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
			mshItem2d, SLOT(reloadElemItems()));
		//connect(_subModels[1], SIGNAL(subModelDataChanged(Model,QModelIndex,QModelIndex)),
		//	mshItem2d, SLOT(reloadElemItems()));
		MshGraphicsItem3d* mshItem3d = new MshGraphicsItem3d(*it);
		ModelItem* item = new ModelItem(mshItem2d, mshItem3d, this);
		item->addModel(nodeModel);
		item->addModel(elemModel);

		_data.push_back(item);
	}
	Model::updateData();
}

bool MshModel::removeRows( int row, int count, const QModelIndex & parent /*= QModelIndex() */ )
{
	for (int i = count; i > 0; i--)
		fem_msh_vector.erase(fem_msh_vector.begin() + row + i - 1);

	return Model::removeRows(row, count, parent);
}
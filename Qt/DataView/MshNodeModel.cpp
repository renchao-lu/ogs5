/**
 * \file MshNodeModel.cpp
 * 21/10/2009 LB Initial implementation
 * 
 * Implementation of MshNodeModel
 */

// ** INCLUDES **
#include "MshNodeModel.h"
#include "ModelItem.h"
#include "PntGraphicsItem2d.h"
//#include "MshNodeAdapter.h"


MshNodeModel::MshNodeModel( QString name, Mesh_Group::CFEMesh* msh, QObject* parent /*= 0*/ )
: Model(name, parent), _msh(msh)
{
	_modelContentType = MSH_NODE_MODEL;
	updateData();
}

int MshNodeModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 3;
}

QVariant MshNodeModel::data( const QModelIndex& index, int role ) const
{
	if (!index.isValid())
		return QVariant();
	
	if (index.row() >= (int)_msh->nod_vector.size())
		return QVariant();

	GraphicsItem2d* item2d = itemFromIndex(index)->item2d();
	if (item2d == NULL)
		return QVariant();
/*  TODO
	Mesh_Group::CNode* node = static_cast<Mesh_Group::CNode*>
		(static_cast<PntGraphicsItem2d*>(item2d)->point())->node();
	if (node == NULL)
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole:
		switch (index.column())
		{
		case 0:
			return node->X();
		case 1:
			return node->Y();
		case 2:
			return node->Z();
		default:
			return QVariant();
		}
		break;

	case Qt::ToolTipRole:
		return QVariant();

	default:
		return QVariant();
	}
*/
	return QVariant();
}

QVariant MshNodeModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0: return "X";
		case 1: return "Y";
		case 2: return "Z";
		default: return QVariant();
		}
	}
	else
		return QString("Row %1").arg(section);
}



bool MshNodeModel::setData( const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */ )
{
	if (index.isValid() && role == Qt::EditRole)
	{
		Mesh_Group::CNode* node = _msh->nod_vector.at(index.row());
		bool wasConversionSuccesfull = false;
		double x, y, z;
		switch (index.column())
		{
		case 0:
			x = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				node->SetX(x);
			else
				return false;
			break;
		case 1:
			y = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				node->SetY(y);
			else
				return false;
			break;
		case 2:
			z = value.toDouble(&wasConversionSuccesfull);
			if (wasConversionSuccesfull)
				node->SetZ(z);
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

QModelIndex MshNodeModel::indexFromOgsNode( const Mesh_Group::CNode* node ) const
{
	if (node != NULL)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			QModelIndex nodeIndex = index(i, 0);
			PntGraphicsItem2d* itemFromIndex =
				static_cast<PntGraphicsItem2d*>(
				static_cast<ModelItem*>(nodeIndex.internalPointer())->item2d());

			// TODO 
			//if (node == static_cast<MshNodeAdapter*>(itemFromIndex->point())->node())
			//	return nodeIndex;
		}
	}
	return QModelIndex();
}

void MshNodeModel::updateData()
{
	clearData();

	for (vector<Mesh_Group::CNode*>::const_iterator it = _msh->nod_vector.begin();
		it != _msh->nod_vector.end(); ++it)
	{
		// TODO 
		//MshNodeAdapter* nodeAdapter = new MshNodeAdapter(*it);
		//PntGraphicsItem2d* pntItem2d = new PntGraphicsItem2d(this, nodeAdapter, NULL, 1.0, MSH_NODE_ITEM);
		//GraphicsItem3d* pntItem3d = new PntGraphicsItem3d(nodeAdapter, NULL, MSH_NODE_ITEM, this);
		//pntItem3d->show();
		//ModelItem* item = new ModelItem(pntItem2d, pntItem3d);
		//connect(pntItem2d, SIGNAL(itemPositionChanged(IGeometryPoint*)),
		//	this, SLOT(item2dChanged(IGeometryPoint*)));
		//connect(pntItem2d, SIGNAL(itemPositionChanged(IGeometryPoint*)),
		//	pntItem3d, SLOT(updatePosition()));
		//_data.push_back(item);
	}
	Model::updateData();
}
/*
void MshNodeModel::item2dChanged( IGeometryPoint* point )
{
	Mesh_Group::CNode* node = ((MshNodeAdapter*)point)->node();
	QModelIndex itemIndex = indexFromOgsNode(node);
	QModelIndex indexEnd = index(itemIndex.row(), columnCount());
	emit dataChanged(itemIndex, indexEnd);
}
*/
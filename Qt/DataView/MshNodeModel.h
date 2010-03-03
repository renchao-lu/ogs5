/**
 * \file MshNodeModel.h
 * 21/10/2009 LB Initial implementation
 *
 */


#ifndef MSHNODEMODEL_H
#define MSHNODEMODEL_H

// ** INCLUDES **
#include "Model.h"
#include "msh_mesh.h"

class IGeometryPoint;

/**
 * MshNodeModel
 */
class MshNodeModel : public Model
{
	Q_OBJECT

public:
	MshNodeModel(QString name, Mesh_Group::CFEMesh* msh, QObject* parent = 0);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role /* = Qt::EditRole */);

	QModelIndex indexFromOgsNode(const Mesh_Group::CNode* node) const;

public slots:
	void updateData();

protected slots:
	void item2dChanged(IGeometryPoint* point);

private:
	Mesh_Group::CFEMesh* _msh;

signals:
	void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif // MSHNODEMODEL_H

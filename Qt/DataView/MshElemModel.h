/**
 * \file MshElemModel.h
 * 22/10/2009 LB Initial implementation
 *
 */


#ifndef MSHELEMMODEL_H
#define MSHELEMMODEL_H

// ** INCLUDES **
#include "Model.h"
#include "msh_mesh.h"

/**
 * The MshElemModel is a Qt model which represents  CElem objects.
 * No graphic items are hold in this model. The data is referenced through
 * a CFEMesh pointer. Graphical representation is archived through MshModel.
 */
class MshElemModel : public Model
{
	Q_OBJECT

public:
	MshElemModel(QString name, Mesh_Group::CFEMesh* msh, QObject* parent = 0);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	public slots:
		void updateData();

private:
	const QString geoTypeString(int type) const;
	Mesh_Group::CFEMesh* _msh;

private:

};

#endif // MSHELEMMODEL_H

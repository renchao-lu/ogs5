/**
 * \file MshModel.h
 * 19/10/2009 LB Initial implementation
 *
 */


#ifndef MSHMODEL_H
#define MSHMODEL_H

// ** INCLUDES **
#include "Model.h"

class QItemSelection;

/**
 * The MshModel is a Qt model which represents  CFEMesh objects.
 * Item indexes hold references to ModelItem as internal pointers.
 */
class MshModel : public Model
{
	Q_OBJECT

public:
	MshModel(QString name, QObject* parent = 0);

	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;

	/// Erases the corresponding entries in the internal mesh data structure.
	/// At the moment this is fem_msh_vector. At the end Model::removeRows()
	/// is called.
	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex() );

	bool setData(const QModelIndex& index, const QVariant& value,
		int role = Qt::EditRole);

public slots:
	/// Reloads all items
	void updateData();

private:

signals:
	//void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};

#endif // MSHMODEL_H

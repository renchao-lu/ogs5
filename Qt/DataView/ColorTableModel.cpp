/**
 * \file ColorTableModel.cpp
 * 24/9/2009 LB Initial implementation
 * 05/05/2010 KR 2d graphic functionality removed and various layout changes
 *
 * Implementation of PolylinesModel
 */

#include "ColorTableModel.h"


ColorTableModel::ColorTableModel( std::map<std::string, GEOLIB::Color> &colorLookupTable, QObject* parent /*= 0*/ )
{
	Q_UNUSED(parent)

	this->buildTable(colorLookupTable);
}

ColorTableModel::~ColorTableModel()
{
}

int ColorTableModel::columnCount( const QModelIndex& parent /*= QModelIndex()*/ ) const
{
	Q_UNUSED(parent)

	return 2;
}

QVariant ColorTableModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const 
{ 
	if (role != Qt::DisplayRole) 
		return QVariant(); 

	if (orientation == Qt::Horizontal) 
	{ 
        switch (section) 
        { 
			case 0: return "Name"; 
			case 1: return "Colour"; 
			default: return QVariant(); 
        } 
	} 
	else 
		return QString("Row %1").arg(section); 
}

QVariant ColorTableModel::data( const QModelIndex& index, int role ) const 
{ 
    if (!index.isValid()) 
        return QVariant(); 

    if (index.row() >= _listOfPairs.size() || index.row()<0) 
        return QVariant(); 

	if (role == Qt::DisplayRole)
	{
		QPair<QString, QColor> pair = _listOfPairs.at(index.row());

		switch (index.column()) 
        { 
			case 0: 
				return pair.first;
			case 1: 
				return pair.second;
            default: 
                return QVariant(); 
		} 
    } 
	return QVariant();
}

bool ColorTableModel::buildTable(std::map<std::string, GEOLIB::Color> &colorLookupTable)
{
     int count = 0;
     beginInsertRows(QModelIndex(), 0, colorLookupTable.size()-1);

	 for (std::map<std::string, GEOLIB::Color>::const_iterator it=colorLookupTable.begin(); it !=colorLookupTable.end(); ++it)
	 {
		 QColor color((it->second)[0], (it->second)[1], (it->second)[2]);
		 QPair<QString, QColor> pair(QString::fromStdString(it->first), color);
			 /*"(" + 
			 QString::number((it->second)[0]) + ", " +
			 QString::number((it->second)[1]) + ", " +
			 QString::number((it->second)[2]) + ")");*/
         _listOfPairs.insert(count++, pair);
     }

     endInsertRows();
     return true;
}



/**
 * \file MshGraphicsItem2d.cpp
 * 29/10/2009 LB Initial implementation
 *
 * Implementation of MshGraphicsItem2d
 */

// ** INCLUDES **
#include "MshGraphicsItem2d.h"
#include "LineGraphicsItem2d.h"

#include <QPainter>
#include <QRectF>

MshGraphicsItem2d::MshGraphicsItem2d( Model* model, Mesh_Group::CFEMesh* mesh, EViewPlane viewplane /*= VIEWPLANE_XY*/, QGraphicsItem* parent /*= 0*/ )
: GraphicsItem2d(model, viewplane, 1, MSH_ITEM, parent), _mesh(mesh)
{
	reloadElemItems();
}

QRectF MshGraphicsItem2d::boundingRect() const
{
	return QRectF();
}

void MshGraphicsItem2d::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{

}

void MshGraphicsItem2d::reloadElemItems()
{
	// TODO
	/*
	foreach (Mesh_Group::CElem* elem, _mesh->ele_vector)
	{
		Mesh_Group::CNode* node;		// Actual node on element
		vector<IGeometryPoint*> pnts;	// nodes converted to points
		LineGraphicsItem2d* line;
		switch (elem->GetElementType())
		{
		case 1: // line
			{
				IGeometryLine* geoLine = new GeometryLine(new MshNodeAdapter(elem->GetNode(0)),
					new MshNodeAdapter(elem->GetNode(1)));
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
			}
			break;

		case 2: // quad
			{
				for (int i = 0; i < 5; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i%4)));
				IGeometryLine* geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();
			}
			break;

		case 3: // hex
			{
				// the bottom side of the cube
				for (int i = 0; i < 5; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i%4)));
				IGeometryLine* geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();

				// the top side of the cube
				for (int i = 0; i < 5; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode((i%4) + 4)));
				geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();

				// the 4 connection lines between bottom and top side
				for (int i = 0; i < 4; i++)
				{
					geoLine = new GeometryLine(new MshNodeAdapter(elem->GetNode(i)),
					new MshNodeAdapter(elem->GetNode(i + 4)));
					line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
					_elemItems.push_back(line);
				}
			}
			break;

		case 4: // tri
			{
				for (int i = 0; i < 4; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i%3)));
				IGeometryLine* geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();
			}
			break;

		case 5: // tetrahedral
			{
				for (int i = 0; i < 4; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i)));
				pnts.push_back(pnts[1]);
				IGeometryLine* geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				geoLine = new GeometryLine(pnts[0], pnts[3]);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();
			}
			break;

		case 6: // 3-sided prism
			{
				// the bottom side of the prism
				for (int i = 0; i < 4; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i%3)));
				IGeometryLine* geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();

				// the top side of the prism
				for (int i = 3; i < 6; i++)
					pnts.push_back(new MshNodeAdapter(elem->GetNode(i)));
				pnts.push_back(new MshNodeAdapter(elem->GetNode(3)));
				geoLine = new GeometryLine(pnts);
				line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
				_elemItems.push_back(line);
				pnts.clear();

				// the 3 connection lines between bottom and top side
				for (int i = 0; i < 3; i++)
				{
					geoLine = new GeometryLine(new MshNodeAdapter(elem->GetNode(i)),
						new MshNodeAdapter(elem->GetNode(i + 3)));
					line = new LineGraphicsItem2d(_model, geoLine, VIEWPLANE_XY, this);
					_elemItems.push_back(line);
				}
			}
			break;
		}
	}

	foreach(GraphicsItem2d* item2d, _elemItems)
		item2d->setFlag(ItemIsMovable, false);
		*/
}

void MshGraphicsItem2d::updatePosition()
{

}

void MshGraphicsItem2d::setEditable( bool enable )
{
	setFlag(ItemIsSelectable, enable);
}

#pragma once
/*
by Haibing Shao 2006
haibingshao@hotmail.com

for Isosurface function
*/
#include <vector>
#include "msh_mesh.h"

#ifndef _TABLES_H_

#define _TABLES_H_

extern int EdgeTable_Hexahedron[256];
extern int TriTable_Hexahedron[256][16];
extern int TriTable_Tetrahedron[16][7];
extern int EdgeTable_Tetrahedron[16];
extern int TriTable_Prism[64][10];
extern int EdgeTable_Prism[64];
extern int EdgeTable_Triangle[8];
extern int LineTable_Triangle[8][3];
extern int EdgeTable_Quad[16];
extern int LineTable_Quad[16][5];

#endif
class CIsoSurfaceNode
{
public:
	CIsoSurfaceNode(void);
	~CIsoSurfaceNode(void);
	double x,y,z;
	double value;
};

class CIsoSurfaceTriangle
{
public:
	CIsoSurfaceTriangle(void);
	~CIsoSurfaceTriangle(void);
	CIsoSurfaceNode point[3];
	CIsoSurfaceNode normal;
};
class CIsoSurfaceLine
{
public:
	CIsoSurfaceLine(void);
	~CIsoSurfaceLine(void);
	CIsoSurfaceNode point[2];
};


class CIsoPrism
{
public:
	CIsoPrism(void);
	~CIsoPrism(void);
	CIsoSurfaceNode* point[6];
};

class CIsoTetrahedron
{
public:
	CIsoTetrahedron(void);
	~CIsoTetrahedron(void);
	CIsoSurfaceNode* point[4];
};
class CIsoHexahedron
{
public:
	CIsoHexahedron(void);
	~CIsoHexahedron(void);
	CIsoSurfaceNode* point[8];
};

class CIsoTriangle
{
public:
	CIsoTriangle(void);
	~CIsoTriangle(void);
	CIsoSurfaceNode* point[3];
};
class CIsoQuad
{
public:
	CIsoQuad(void);
	~CIsoQuad(void);
	CIsoSurfaceNode* point[4];
};


class CIsoSurface
{
public:
	CIsoSurface(double m_isoValue);
	~CIsoSurface(void);
	std::vector<CIsoSurfaceTriangle*> vector_IsoTriangle;
	std::vector<CIsoSurfaceLine*> vector_IsoLine;
	double iso_value;
	int PolygoniseAndInsert(double isoLevel, CIsoHexahedron* hexahedron, std::vector<CIsoSurfaceTriangle*>* mIsoSurface);
	int PolygoniseAndInsert(double isoLevel, CIsoTetrahedron* tetrahedron, std::vector<CIsoSurfaceTriangle*>* mIsoSurface);
	int PolygoniseAndInsert(double isoLevel, CIsoPrism* prism, std::vector<CIsoSurfaceTriangle*>* mIsoSurface);
	int PolygoniseAndInsert(double isoLevel, CIsoTriangle* triangle, std::vector<CIsoSurfaceLine*>* mIsoLine);
	int PolygoniseAndInsert(double isoLevel, CIsoQuad* quad, std::vector<CIsoSurfaceLine*>* mIsoLine);

	CIsoSurfaceNode interpolate(double iso_Value, CIsoSurfaceNode* point1, CIsoSurfaceNode* point2);
	void calculateNormal(CIsoSurfaceTriangle* tri);
};









/*
by Haibing Shao 2006
haibingshao@hotmail.com

for Isosurface function
*/


#include "stdafx.h"
#include ".\isosurface.h"
#include "msh_mesh.h"


int EdgeTable_Hexahedron[256] = {
	0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
	0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
	0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
	0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
	0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
	0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
	0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
	0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
	0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
	0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
	0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
	0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
	0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
	0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
	0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
	0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
	0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
	0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
	0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
	0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
	0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
	0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
	0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
	0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
	0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
	0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
	0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
	0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
	0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
	0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
	0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
	0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

int TriTable_Hexahedron[256][16] = {
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
	{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
	{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
	{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
	{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
	{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
	{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
	{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
	{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
	{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
	{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
	{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
	{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
	{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
	{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
	{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
	{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
	{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
	{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
	{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
	{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
	{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
	{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
	{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
	{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
	{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
	{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
	{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
	{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
	{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
	{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
	{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
	{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
	{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
	{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
	{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
	{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
	{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
	{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
	{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
	{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
	{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
	{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
	{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
	{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
	{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
	{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
	{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
	{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
	{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
	{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
	{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
	{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
	{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
	{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
	{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
	{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
	{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
	{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
	{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
	{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
	{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
	{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
	{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
	{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
	{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
	{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
	{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
	{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
	{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
	{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
	{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
	{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
	{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
	{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
	{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
	{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
	{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
	{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
	{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
	{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
	{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
	{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
	{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
	{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
	{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
	{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
	{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
	{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
	{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
	{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
	{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
	{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
	{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
	{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
	{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
	{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
	{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
	{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
	{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
	{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
	{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
	{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
	{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
	{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
	{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
	{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
	{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
	{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
	{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
	{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
	{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
	{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
	{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
	{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
	{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
	{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
	{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
	{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
	{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
	{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
	{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
	{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
	{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
	{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
	{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
	{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
	{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
	{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
	{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
	{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
	{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
	{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
	{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
	{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
	{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
	{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
	{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
	{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
	{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
	{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
	{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
	{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
	{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
	{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
	{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
	{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
	{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
	{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
	{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
	{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
	{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
	{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
	{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
	{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
	{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
	{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
	{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
	{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
	{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
	{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
	{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
	{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
	{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
	{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
	{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
	{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
	{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
	{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
	{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
	{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
	{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
	{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
	{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
	{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
	{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};
int EdgeTable_Tetrahedron[16]={
0x0,0xd,0x13,0x1e,
0x26,0x2b,0x35,0x38,
0x38,0x35,0x2b,0x26,
0x1e,0x13,0xd,0x0
};
int TriTable_Tetrahedron[16][7]={
	{-1,-1,-1,-1,-1,-1,-1},
	{0,2,3,-1,-1,-1,-1},
	{0,1,4,-1,-1,-1,-1},
	{1,2,4,2,3,4,-1},
	{1,2,5,-1,-1,-1,-1},
	{0,1,3,1,3,5,-1},
	{0,2,4,2,4,5,-1},
	{3,4,5,-1,-1,-1,-1},
	{3,4,5,-1,-1,-1,-1},
	{0,2,4,2,4,5,-1},
	{0,1,3,1,3,5,-1},
	{1,2,5,-1,-1,-1,-1},
	{1,2,4,2,3,4,-1},
	{0,1,4,-1,-1,-1,-1},
	{0,2,3,-1,-1,-1,-1},
	{-1,-1,-1,-1,-1,-1,-1}
};
int TriTable_Prism[64][10]={
	{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
	{2,0,6,-1,-1,-1,-1,-1,-1,-1},
	{1,0,7,-1,-1,-1,-1,-1,-1,-1},
	{1,2,7,2,6,7,-1,-1,-1,-1},
	{1,2,8,-1,-1,-1,-1,-1,-1,-1},
	{1,0,8,6,0,8,-1,-1,-1,-1},
	{2,0,7,2,7,8,-1,-1,-1,-1},
	{6,7,8,-1,-1,-1,-1,-1,-1,-1},
	{3,5,6,-1,-1,-1,-1,-1,-1,-1},
	{2,0,5,3,0,5,-1,-1,-1,-1},
	{1,0,7,3,5,6,-1,-1,-1,-1},
	{2,3,5,2,3,7,1,2,7,-1},
	{1,2,8,3,5,6,-1,-1,-1,-1},
	{3,0,5,1,0,5,1,5,8,-1},
	{3,5,6,2,0,7,2,7,8,-1},
	{3,5,8,3,7,8,-1,-1,-1,-1},
	{3,4,7,-1,-1,-1,-1,-1,-1,-1},
	{2,0,6,3,4,7,-1,-1,-1,-1},
	{1,0,3,1,3,4,-1,-1,-1,-1},
	{1,2,4,2,3,4,2,3,6,-1},
	{1,2,8,3,4,7,-1,-1,-1,-1},
	{2,0,6,1,2,8,3,4,7,-1},
	{2,0,3,2,3,4,2,4,8,-1},
	{3,4,6,4,6,8,-1,-1,-1,-1},
	{4,5,6,4,6,7,-1,-1,-1,-1},
	{2,0,7,2,4,7,2,4,5,-1},
	{4,5,6,1,4,6,1,0,6,-1},
	{1,2,5,1,4,5,-1,-1,-1,-1},
	{1,2,6,1,6,7,4,5,8,-1},
	{1,0,7,4,5,8,-1,-1,-1,-1},
	{2,0,6,4,5,8,-1,-1,-1,-1},
	{4,5,8,-1,-1,-1,-1,-1,-1,-1},
	{4,5,8,-1,-1,-1,-1,-1,-1,-1},
	{2,0,6,4,5,8,-1,-1,-1,-1},
	{1,0,7,4,5,8,-1,-1,-1,-1},
	{1,2,6,1,6,7,4,5,8,-1},
	{1,2,5,1,4,5,-1,-1,-1,-1},
	{4,5,6,1,4,6,1,0,6,-1},
	{2,0,7,2,4,7,2,4,5,-1},
	{4,5,6,4,6,7,-1,-1,-1,-1},
	{3,4,6,4,6,8,-1,-1,-1,-1},
	{2,0,3,2,3,4,2,4,8,-1},
	{2,0,6,1,2,8,3,4,7,-1},
	{1,2,8,3,4,7,-1,-1,-1,-1},
	{1,2,4,2,3,4,2,3,6,-1},
	{1,0,3,1,3,4,-1,-1,-1,-1},
	{2,0,6,3,4,7,-1,-1,-1,-1},
	{3,4,7,-1,-1,-1,-1,-1,-1,-1},
	{3,5,8,3,7,8,-1,-1,-1,-1},
	{3,5,6,2,0,7,2,7,8,-1},
	{3,0,5,1,0,5,1,5,8,-1},
	{1,2,8,3,5,6,-1,-1,-1,-1},
	{2,3,5,2,3,7,1,2,7,-1},
	{1,0,7,3,5,6,-1,-1,-1,-1},
	{2,0,5,3,0,5,-1,-1,-1,-1},
	{3,5,6,-1,-1,-1,-1,-1,-1,-1},
	{6,7,8,-1,-1,-1,-1,-1,-1,-1},
	{2,0,7,2,7,8,-1,-1,-1,-1},
	{1,0,8,6,0,8,-1,-1,-1,-1},
	{1,2,8,-1,-1,-1,-1,-1,-1,-1},
	{1,2,7,2,6,7,-1,-1,-1,-1},
	{1,0,7,-1,-1,-1,-1,-1,-1,-1},
	{2,0,6,-1,-1,-1,-1,-1,-1,-1},
	{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};
int EdgeTable_Prism[64]={
	0x0,0x045,0x083,0x0C6,
    0x106,0x143,0x185,0x1C0,
	0x068,0x02D,0x0EB,0x0AE,
	0x16E,0x12B,0x1ED,0x1A8,
	0x098,0x0DD,0x01B,0x05E,
	0x19E,0x1DF,0x11D,0x158,
	0x0F0,0x0B5,0x073,0x036,
	0x1F6,0x1B3,0x175,0x130,
	0x130,0x175,0x1B3,0x1F6,
	0x036,0x073,0x0B5,0x0F0,
	0x158,0x11D,0x1DF,0x19E,
	0x05E,0x01B,0x0DD,0x098,
	0x1A8,0x1ED,0x12B,0x16E,
	0x0AE,0x0EB,0x02D,0x068,
	0x1C0,0x185,0x143,0x106,
	0x0C6,0x083,0x045,0x0
};
int EdgeTable_Triangle[8]={0x0,0x005,0x003,0x006,0x006,0x003,0x005,0x0};

int LineTable_Triangle[8][3]={
{-1,-1,-1},
{0,2,-1},
{0,1,-1},
{1,2,-1},
{1,2,-1},
{0,1,-1},
{0,2,-1},
{-1,-1,-1}
};

int EdgeTable_Quad[16]={
0x0,0x009,0x003,0x00A,
0x006,0x009,0x005,0x00C,
0x00C,0x005,0x009,0x006,
0x00A,0x003,0x009,0x0
};

int LineTable_Quad[16][5]={
{-1,-1,-1,-1,-1},
{0,3,-1,-1,-1},
{0,1,-1,-1,-1},
{1,3,-1,-1,-1},
{1,2,-1,-1,-1},
{0,3,1,2,-1},
{0,2,-1,-1,-1},
{2,3,-1,-1,-1},
{2,3,-1,-1,-1},
{0,2,-1,-1,-1},
{0,3,1,2,-1},
{1,2,-1,-1,-1},
{1,3,-1,-1,-1},
{0,1,-1,-1,-1},
{0,3,-1,-1,-1},
{-1,-1,-1,-1,-1}
};

CIsoSurface::CIsoSurface(double m_isoValue)
{
	CIsoSurface::iso_value = m_isoValue;
}

CIsoSurface::~CIsoSurface(void)
{
	for (int i=0;i<(int)vector_IsoTriangle.size();i++)
	{	for (int j=0;j<3;j++)		
		{
			vector_IsoTriangle[i]->point[j].~CIsoSurfaceNode();
		}
		vector_IsoTriangle[i]->~CIsoSurfaceTriangle();
		delete vector_IsoTriangle[i];
	}
	vector_IsoTriangle.clear();

	for (int i=0;i<(int)vector_IsoLine.size();i++)
	{	for (int j=0;j<2;j++)		
		{
			vector_IsoLine[i]->point[j].~CIsoSurfaceNode();
		}
		vector_IsoLine[i]->~CIsoSurfaceLine();
		delete vector_IsoLine[i];
	}
	vector_IsoLine.clear();
}

CIsoSurfaceTriangle::CIsoSurfaceTriangle(void)
{
}

CIsoSurfaceTriangle::~CIsoSurfaceTriangle(void)
{

}
CIsoSurfaceLine::CIsoSurfaceLine(void)
{
}

CIsoSurfaceLine::~CIsoSurfaceLine(void)
{

}
CIsoSurfaceNode::CIsoSurfaceNode(void)
{
}

CIsoSurfaceNode::~CIsoSurfaceNode(void)
{
}

CIsoPrism::CIsoPrism(void)
{
	for (int i=0; i<6 ;i++)
		point[i] = new CIsoSurfaceNode();
}

CIsoPrism::~CIsoPrism(void)
{
	for (int i=0; i<6 ;i++)
	{
		point[i]->~CIsoSurfaceNode();
		delete point[i];
	}
}

CIsoTetrahedron::CIsoTetrahedron(void)
{
	for (int i=0; i<4 ;i++)
		point[i] = new CIsoSurfaceNode();
}

CIsoTetrahedron::~CIsoTetrahedron(void)
{
	for (int i=0; i<4 ;i++)
	{
		point[i]->~CIsoSurfaceNode();
		delete point[i];
	}
}

CIsoHexahedron::CIsoHexahedron(void)
{
	for (int i=0; i<8 ;i++)
		point[i] = new CIsoSurfaceNode();
}

CIsoHexahedron::~CIsoHexahedron(void)
{
	for (int i=0; i<8 ;i++)
	{
		point[i]->~CIsoSurfaceNode();
		delete point[i];
	}
}
CIsoTriangle::CIsoTriangle(void)
{
	for (int i=0; i<3 ;i++)
		point[i] = new CIsoSurfaceNode();
}

CIsoTriangle::~CIsoTriangle(void)
{
	for (int i=0; i<3 ;i++)
	{
		point[i]->~CIsoSurfaceNode();
		delete point[i];
	}
}

CIsoQuad::CIsoQuad(void)
{
	for (int i=0; i<4 ;i++)
		point[i] = new CIsoSurfaceNode();
}

CIsoQuad::~CIsoQuad(void)
{
	for (int i=0; i<4 ;i++)
	{
		point[i]->~CIsoSurfaceNode();
		delete point[i];
	}
}
CIsoSurfaceNode CIsoSurface::interpolate(double iso_Value, CIsoSurfaceNode* point1, CIsoSurfaceNode* point2)
{
	double mu;
	CIsoSurfaceNode p;
	mu = (iso_Value - point1->value) / (point2->value - point1->value);
	p.x = point1->x + mu * (point2->x - point1->x);
	p.y = point1->y + mu * (point2->y - point1->y);
	p.z = point1->z + mu * (point2->z - point1->z);
	return p;
}

int CIsoSurface::PolygoniseAndInsert(double isoLevel, CIsoHexahedron* hexahedron, std::vector<CIsoSurfaceTriangle*>* mIsoSurface)
{
	int Index = 0;
	if (hexahedron->point[0]->value > isoLevel) Index |= 1;
	if (hexahedron->point[1]->value > isoLevel) Index |= 2;
	if (hexahedron->point[2]->value > isoLevel) Index |= 4;
	if (hexahedron->point[3]->value > isoLevel) Index |= 8;
	if (hexahedron->point[4]->value > isoLevel) Index |= 16;
	if (hexahedron->point[5]->value > isoLevel) Index |= 32;
	if (hexahedron->point[6]->value > isoLevel) Index |= 64;
	if (hexahedron->point[7]->value > isoLevel) Index |= 128;

	if (EdgeTable_Hexahedron[Index] == 0) {
		return 0;
	}

	CIsoSurfaceNode vertices[12];
	if (EdgeTable_Hexahedron[Index] & 1) {
		vertices[0] = interpolate(isoLevel, hexahedron->point[0], hexahedron->point[1]);
	}
	if (EdgeTable_Hexahedron[Index] & 2) {
		vertices[1] = interpolate(isoLevel, hexahedron->point[1], hexahedron->point[2]);
	}
	if (EdgeTable_Hexahedron[Index] & 4) {
		vertices[2] = interpolate(isoLevel, hexahedron->point[2], hexahedron->point[3]);
	}
	if (EdgeTable_Hexahedron[Index] & 8) {
		vertices[3] = interpolate(isoLevel, hexahedron->point[3], hexahedron->point[0]);
	}
	if (EdgeTable_Hexahedron[Index] & 16) {
		vertices[4] = interpolate(isoLevel, hexahedron->point[4], hexahedron->point[5]);
	}
	if (EdgeTable_Hexahedron[Index] & 32) {
		vertices[5] = interpolate(isoLevel, hexahedron->point[5], hexahedron->point[6]);
	}
	if (EdgeTable_Hexahedron[Index] & 64) {
		vertices[6] = interpolate(isoLevel, hexahedron->point[6], hexahedron->point[7]);
	}
	if (EdgeTable_Hexahedron[Index] & 128) {
		vertices[7] = interpolate(isoLevel, hexahedron->point[7], hexahedron->point[4]);
	}
	if (EdgeTable_Hexahedron[Index] & 256) {
		vertices[8] = interpolate(isoLevel, hexahedron->point[0], hexahedron->point[4]);
	}
	if (EdgeTable_Hexahedron[Index] & 512) {
		vertices[9] = interpolate(isoLevel, hexahedron->point[1], hexahedron->point[5]);
	}
	if (EdgeTable_Hexahedron[Index] & 1024) {
		vertices[10] = interpolate(isoLevel, hexahedron->point[2], hexahedron->point[6]);
	}
	if (EdgeTable_Hexahedron[Index] & 2048) {
		vertices[11] = interpolate(isoLevel, hexahedron->point[3], hexahedron->point[7]);
	}

	int triCount = 0;
	for (int i = 0; TriTable_Hexahedron[Index][i] != -1; i+=3) {
		CIsoSurfaceTriangle* tri;
		tri = new CIsoSurfaceTriangle();
		tri->point[0] = vertices[TriTable_Hexahedron[Index][i  ]];
		tri->point[1] = vertices[TriTable_Hexahedron[Index][i+1]];
		tri->point[2] = vertices[TriTable_Hexahedron[Index][i+2]];
		calculateNormal(tri);
		mIsoSurface->push_back(tri);
		triCount++;
		tri = NULL;
	}
	return triCount;
}
int CIsoSurface::PolygoniseAndInsert(double isoLevel, CIsoTetrahedron* tetrahedron, std::vector<CIsoSurfaceTriangle*>* mIsoSurface)
{
	int Index = 0;
	if (tetrahedron->point[0]->value > isoLevel) Index |= 1;
	if (tetrahedron->point[1]->value > isoLevel) Index |= 2;
	if (tetrahedron->point[2]->value > isoLevel) Index |= 4;
	if (tetrahedron->point[3]->value > isoLevel) Index |= 8;

	if (EdgeTable_Tetrahedron[Index] == 0) {
		return 0;
	}

	CIsoSurfaceNode vertices[6];
	if (EdgeTable_Tetrahedron[Index] & 1) {
		vertices[0] = interpolate(isoLevel, tetrahedron->point[1], tetrahedron->point[0]);
	}
	if (EdgeTable_Tetrahedron[Index] & 2) {
		vertices[1] = interpolate(isoLevel, tetrahedron->point[2], tetrahedron->point[1]);
	}
	if (EdgeTable_Tetrahedron[Index] & 4) {
		vertices[2] = interpolate(isoLevel, tetrahedron->point[2], tetrahedron->point[0]);
	}
	if (EdgeTable_Tetrahedron[Index] & 8) {
		vertices[3] = interpolate(isoLevel, tetrahedron->point[0], tetrahedron->point[3]);
	}
	if (EdgeTable_Tetrahedron[Index] & 16) {
		vertices[4] = interpolate(isoLevel, tetrahedron->point[1], tetrahedron->point[3]);
	}
	if (EdgeTable_Tetrahedron[Index] & 32) {
		vertices[5] = interpolate(isoLevel, tetrahedron->point[2], tetrahedron->point[3]);
	}

	int triCount = 0;
	for (int i = 0; TriTable_Tetrahedron[Index][i] != -1; i+=3) {
		CIsoSurfaceTriangle* tri;
		tri = new CIsoSurfaceTriangle();
		tri->point[0] = vertices[TriTable_Tetrahedron[Index][i  ]];
		tri->point[1] = vertices[TriTable_Tetrahedron[Index][i+1]];
		tri->point[2] = vertices[TriTable_Tetrahedron[Index][i+2]];
		calculateNormal(tri);
		mIsoSurface->push_back(tri);
		triCount++;
		tri = NULL;
	}
	return triCount;
}
int CIsoSurface::PolygoniseAndInsert(double isoLevel, CIsoPrism* prism, std::vector<CIsoSurfaceTriangle*>* mIsoSurface)
{
	int Index = 0;
	if (prism->point[0]->value > isoLevel) Index |= 1;
	if (prism->point[1]->value > isoLevel) Index |= 2;
	if (prism->point[2]->value > isoLevel) Index |= 4;
	if (prism->point[3]->value > isoLevel) Index |= 8;
	if (prism->point[4]->value > isoLevel) Index |= 16;
	if (prism->point[5]->value > isoLevel) Index |= 32;

	if (EdgeTable_Prism[Index] == 0) {
		return 0;
	}

	CIsoSurfaceNode vertices[9];
	if (EdgeTable_Prism[Index] & 1) {
		vertices[0] = interpolate(isoLevel, prism->point[0], prism->point[1]);
	}
	if (EdgeTable_Prism[Index] & 2) {
		vertices[1] = interpolate(isoLevel, prism->point[1], prism->point[2]);
	}
	if (EdgeTable_Prism[Index] & 4) {
		vertices[2] = interpolate(isoLevel, prism->point[2], prism->point[0]);
	}
	if (EdgeTable_Prism[Index] & 8) {
		vertices[3] = interpolate(isoLevel, prism->point[3], prism->point[4]);
	}
	if (EdgeTable_Prism[Index] & 16) {
		vertices[4] = interpolate(isoLevel, prism->point[4], prism->point[5]);
	}
	if (EdgeTable_Prism[Index] & 32) {
		vertices[5] = interpolate(isoLevel, prism->point[3], prism->point[5]);
	}
	if (EdgeTable_Prism[Index] & 64) {
		vertices[6] = interpolate(isoLevel, prism->point[0], prism->point[3]);
	}
	if (EdgeTable_Prism[Index] & 128) {
		vertices[7] = interpolate(isoLevel, prism->point[1], prism->point[4]);
	}
	if (EdgeTable_Prism[Index] & 256) {
		vertices[8] = interpolate(isoLevel, prism->point[2], prism->point[5]);
	}

	int triCount = 0;
	for (int i = 0; TriTable_Prism[Index][i] != -1; i+=3) {
		CIsoSurfaceTriangle* tri;
		tri = new CIsoSurfaceTriangle();
		tri->point[0] = vertices[TriTable_Prism[Index][i  ]];
		tri->point[1] = vertices[TriTable_Prism[Index][i+1]];
		tri->point[2] = vertices[TriTable_Prism[Index][i+2]];
		calculateNormal(tri);
		mIsoSurface->push_back(tri);
		triCount++;
		tri = NULL;
	}
	return triCount;
}

int CIsoSurface::PolygoniseAndInsert(double isoLevel, CIsoTriangle* triangle, std::vector<CIsoSurfaceLine*>* mIsoLine)
{
	int Index = 0;
	if (triangle->point[0]->value > isoLevel) Index |= 1;
	if (triangle->point[1]->value > isoLevel) Index |= 2;
	if (triangle->point[2]->value > isoLevel) Index |= 4;

	if (EdgeTable_Triangle[Index] == 0) {
		return 0;
	}

	CIsoSurfaceNode vertices[3];
	if (EdgeTable_Triangle[Index] & 1) {
		vertices[0] = interpolate(isoLevel, triangle->point[0], triangle->point[1]);
	}
	if (EdgeTable_Triangle[Index] & 2) {
		vertices[1] = interpolate(isoLevel, triangle->point[1], triangle->point[2]);
	}
	if (EdgeTable_Triangle[Index] & 4) {
		vertices[2] = interpolate(isoLevel, triangle->point[2], triangle->point[0]);
	}


	int lineCount = 0;
	for (int i = 0; LineTable_Triangle[Index][i] != -1; i+=2) {
		CIsoSurfaceLine* line;
		line = new CIsoSurfaceLine();
		line->point[0] = vertices[LineTable_Triangle[Index][i  ]];
		line->point[1] = vertices[LineTable_Triangle[Index][i+1]];

		mIsoLine->push_back(line);
		lineCount++;
		line = NULL;
	}
	return lineCount;
}

int CIsoSurface::PolygoniseAndInsert(double isoLevel, CIsoQuad* quad, std::vector<CIsoSurfaceLine*>* mIsoLine)
{
	int Index = 0;
	if (quad->point[0]->value > isoLevel) Index |= 1;
	if (quad->point[1]->value > isoLevel) Index |= 2;
	if (quad->point[2]->value > isoLevel) Index |= 4;
	if (quad->point[3]->value > isoLevel) Index |= 8;

	if (EdgeTable_Quad[Index] == 0) {
		return 0;
	}

	CIsoSurfaceNode vertices[4];
	if (EdgeTable_Quad[Index] & 1) {
		vertices[0] = interpolate(isoLevel, quad->point[0], quad->point[1]);
	}
	if (EdgeTable_Quad[Index] & 2) {
		vertices[1] = interpolate(isoLevel, quad->point[1], quad->point[2]);
	}
	if (EdgeTable_Quad[Index] & 4) {
		vertices[2] = interpolate(isoLevel, quad->point[2], quad->point[3]);
	}
	if (EdgeTable_Quad[Index] & 8) {
		vertices[3] = interpolate(isoLevel, quad->point[3], quad->point[0]);
	}


	int lineCount = 0;
	for (int i = 0; LineTable_Quad[Index][i] != -1; i+=2) {
		CIsoSurfaceLine* line;
		line = new CIsoSurfaceLine();
		line->point[0] = vertices[LineTable_Quad[Index][i  ]];
		line->point[1] = vertices[LineTable_Quad[Index][i+1]];

		mIsoLine->push_back(line);
		lineCount++;
		line = NULL;
	}
	return lineCount;
}

void CIsoSurface::calculateNormal(CIsoSurfaceTriangle* tri)
{
	double a[3], b[3];
	double length;
	a[0] = tri->point[0].x - tri->point[1].x;
	a[1] = tri->point[0].y - tri->point[1].y;
	a[2] = tri->point[0].z - tri->point[1].z;
	b[0] = tri->point[0].x - tri->point[2].x;
	b[1] = tri->point[0].y - tri->point[2].y;
	b[2] = tri->point[0].z - tri->point[2].z;
	tri->normal.x = a[1] * b[2] - b[1] * a[2];
	tri->normal.y = b[0] * a[2] - a[0] * b[2];
	tri->normal.z = a[0] * b[1] - b[0] * a[1];
	length = sqrt(tri->normal.x*tri->normal.x + tri->normal.y*tri->normal.y + tri->normal.z*tri->normal.z);
	tri->normal.x /= length;
	tri->normal.y /= length;
	tri->normal.z /= length;
}






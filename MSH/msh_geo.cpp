/**************************************************************************
 MSHLib
 07/2007 OK
 03/2010 TF
 **************************************************************************/

// C++
#include <string>
#include <vector>

using namespace std;

// MSHLib
#include "msh_mesh.h"
// GEOLib
#include "geo_pnt.h"
#include "geo_ply.h"

/**************************************************************************
 MSHLib-Method:
 07/2007 OK Implementation
 **************************************************************************/
void CFEMesh::GetNODOnGEO(const std::string& geo_type_name, const std::string& geo_name,
		vector<long>& msh_nod_vector)
{
	long nod_number;
	CGLPoint* m_pnt = NULL;
	CGLPolyline* m_ply = NULL;
	Surface* m_sfc = NULL;//CC

	switch (geo_type_name[3]) {
	case 'N':
		m_pnt = GEOGetPointByName(geo_name);
		if (!m_pnt)
			return;
		nod_number = GetNODOnPNT(m_pnt);
		msh_nod_vector.push_back(nod_number);
		break;
	case 'Y':
		m_ply = GEOGetPLYByName(geo_name);
		if (!m_ply)
			return;
		GetNODOnPLY(m_ply, msh_nod_vector);
		break;
	case 'F':
		//CC------------------------begin
		m_sfc = GEOGetSFCByName(geo_name);//CC
		if (!m_sfc)
			return;
		GetNODOnSFC(m_sfc, msh_nod_vector);
		//CC---------------------------end
		break;
	case 'U':
		break;
	}
}


#ifndef C1_COMMON_H_
#define C1_COMMON_H_

#include <string>
#include <vector>

#include "msh_mesh.h"
#include "rf_pcs.h"
#include "rfmat_cp.h"
#include "rf_mmp_new.h"

//#############################################################################
// Utility functions for material indexes in a mesh
//#############################################################################
inline int getDistributionID(const CFEMesh *msh, const std::string &mat_name)
{
	unsigned het_index = 0;
	for (; het_index< msh->mat_names_vector.size(); het_index++)
		if (msh->mat_names_vector[het_index].compare(mat_name) == 0)
			break;
	return het_index==msh->mat_names_vector.size() ? -1 : het_index;
}

inline int getDistributionID(CRFProcess *pcs, const std::string &mat_name)
{
    return getDistributionID(pcs->m_msh, mat_name);
}

inline int getDistributionIDforAperture(CRFProcess *pcs)
{
    static int het_index (-1);
    if (het_index < 0) het_index = getDistributionID(pcs, "APERTURE");
    return het_index;
}

inline int getDistributionIDforDb(CRFProcess *pcs)
{
    static int het_index (-1);
    if (het_index < 0) het_index = getDistributionID(pcs, "db");
    return het_index;
}

inline int getDistributionIDforRc(CRFProcess *pcs)
{
    static int het_index (-1);
    if (het_index < 0) het_index = getDistributionID(pcs, "CONTACT_RATIO");
    return het_index;
}

//#############################################################################
// Utility functions for processes
//#############################################################################

// return if free-face dissolution or pressure solution is active
inline bool isDissolutionActive()
{
    for(size_t i=0; i<vec_mineral_cp.size(); i++)
        if (vec_mineral_cp[i]->pqc_kinetic_mode>0)
            return true;
    return false;
}

inline bool isApertureUpdateActive()
{
    for(size_t i=0; i<mmp_vector.size(); i++)
        if (mmp_vector[i]->isFracture() && mmp_vector[i]->isFractureApertureVariable())
            return true;
    return false;
}

void setupFracturePropertyDistribution(MeshLib::CFEMesh* msh);

void calculateMassTransferRate(MeshLib::CFEMesh* msh, CFiniteElementStd* fem, CNumerics* m_num);

void calculateApertureChanges(MeshLib::CFEMesh* msh, CFiniteElementStd* fem, CNumerics* m_num);

void initializeGpFractureProperties(MeshLib::CFEMesh* msh, CFiniteElementStd* fem, CNumerics* m_num);

void updateElementProperties(MeshLib::CFEMesh* msh, CFiniteElementStd* fem, CNumerics* m_num);

#endif //C1_PS_H_

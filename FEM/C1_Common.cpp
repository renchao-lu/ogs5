
#include "C1_Common.h"

#include "rf_mmp_new.h"
#include "rf_num_new.h"
#include "fem_ele_std.h"

void setupFracturePropertyDistribution(MeshLib::CFEMesh* msh)
{
    //prepare element data for aperture 0, aperture 1
    std::cout << "->initialize aperture..." << std::endl;
    std::vector<double> temp_store;
    int existing_b = getDistributionID(msh, "APERTURE");
    if (existing_b>-1) {
        std::cout << "->found aperture distribution..." << std::endl;
    }
    int existing_Rc = getDistributionID(msh, "CONTACT_RATIO");
    int existing_db = getDistributionID(msh, "db");

    const size_t n_old_prop = msh->mat_names_vector.size();
    if (getDistributionID(msh, "APERTURE")==-1) {
        msh->mat_names_vector.push_back("APERTURE");
    }
    if (getDistributionID(msh, "CONTACT_RATIO")==-1) {
        msh->mat_names_vector.push_back("CONTACT_RATIO");
    }
    if (getDistributionID(msh, "db")==-1) {
        msh->mat_names_vector.push_back("db");
    }
    const size_t n_new_prop = msh->mat_names_vector.size() - n_old_prop;
    for (size_t i=0; i<msh->ele_vector.size(); i++) {
        MeshLib::CElem* ele = msh->ele_vector[i];
        const size_t mat_vector_size = ele->mat_vector.Size();
        // resize
        if (mat_vector_size > 0) {
            for (size_t c_vals = 0; c_vals < mat_vector_size; c_vals++)
                temp_store.push_back(ele->mat_vector(c_vals));
            ele->mat_vector.resize(mat_vector_size + n_new_prop);
            for (size_t c_vals = 0; c_vals < mat_vector_size; c_vals++)
                ele->mat_vector(c_vals) = temp_store[c_vals];
            temp_store.clear();
        } else {
            ele->mat_vector.resize(mat_vector_size + n_new_prop);
        }
        // set zero
        for (size_t j=0; j<n_new_prop; j++) {
            ele->mat_vector(mat_vector_size + j) = .0;
        }
        // update
        if (mmp_vector[ele->GetPatchIndex()]->isFracture()) {
            unsigned shift = 0;
            if (existing_b<0) {
                ele->mat_vector(mat_vector_size + shift) = mmp_vector[ele->GetPatchIndex()]->FractureAperture(i);
                shift++;
            }
            if (existing_Rc<0) {
                ele->mat_vector(mat_vector_size + shift) = mmp_vector[ele->GetPatchIndex()]->FractureContactAreaRatio(i);
                shift++;
            }
            if (existing_db<0) {
                ele->mat_vector(mat_vector_size + shift) = 0;
                shift++;
            }
        }
    }
    for (size_t i=0; i<mmp_vector.size(); i++) {
        if (mmp_vector[i]->isFracture()) {
            mmp_vector[i]->SetFractureApertureMode(2); //element wise
            mmp_vector[i]->SetFractureContactAreaRatioMode(2); //element wise
        }
    }
    std::cout << "-> Fracture-contact area ratio model is changed to mode 2" << std::endl;
}

void initializeGpFractureProperties(MeshLib::CFEMesh* m_msh, CFiniteElementStd* fem, CNumerics* m_num)
{
    std::cout << "-> initializing gauss point fracture properties..." << std::endl;
    const size_t mesh_ele_vector_size(m_msh->ele_vector.size());
    // for each mmp
    for (unsigned im=0; im<mmp_vector.size(); im++)
    {
        CMediumProperties* mmp = mmp_vector[im];
        if (!mmp->isFracture()) 
			continue;
        for (size_t i = 0; i < mesh_ele_vector_size; i++)
        {
            MeshLib::CElem* elem = m_msh->ele_vector[i];
            if (elem->GetPatchIndex()!=im) continue;
            fem->ConfigElement(elem, m_num->ele_gauss_points);
            fem->Config(); // allocate
        }
    }
}

void calculateMassTransferRate(MeshLib::CFEMesh* m_msh, CFiniteElementStd* fem, CNumerics* m_num)
{
    std::cout << "-> calculating mass transfer rate..." << std::endl;
    const size_t mesh_ele_vector_size(m_msh->ele_vector.size());
    for (size_t i = 0; i < mesh_ele_vector_size; i++)
    {
        MeshLib::CElem* elem = m_msh->ele_vector[i];
        if (elem->GetMark() && mmp_vector[elem->GetPatchIndex()]->isFracture())
        {
            fem->ConfigElement(elem, m_num->ele_gauss_points);
            fem->Config();
            fem->CalculateMassTransferRate();
        }
    }
}

void calculateApertureChanges(MeshLib::CFEMesh* m_msh, CFiniteElementStd* fem, CNumerics* m_num)
{
    std::cout << "-> calculating aperture changes due to chemical..." << std::endl;
    const size_t mesh_ele_vector_size(m_msh->ele_vector.size());

    // calculate aperture changes at all gauss points
    for (size_t i = 0; i < mesh_ele_vector_size; i++)
    {
        MeshLib::CElem* elem = m_msh->ele_vector[i];
        CMediumProperties* mmp = mmp_vector[elem->GetPatchIndex()];
        if (elem->GetMark() && mmp->isFracture())
        {
            fem->ConfigElement(elem, m_num->ele_gauss_points);
            fem->Config();
            fem->CalculateAperture();
        }
    }

    // decides distributed aperture changes (needed for 2D case)
    for (size_t i = 0; i < mesh_ele_vector_size; i++)
    {
        MeshLib::CElem* elem = m_msh->ele_vector[i];
        CMediumProperties* mmp = mmp_vector[elem->GetPatchIndex()];
        if (!elem->GetMark() || !mmp->isFracture())
            continue;

        ElementValue* ev = ele_gp_value[i];
        for (unsigned gp=0; gp<ev->b1.size(); gp++) {
            if (ev->db[gp]>0) { // increasing, e.g. due to FD
                ev->b1[gp] += ev->db[gp];
                ev->Rc1[gp] = ev->Rc0[gp];
            } else { // decreasing
                if (ev->Rc1[gp] > 0.0) { // under contact
                    ev->b1[gp] = ev->b0[gp] + ev->db[gp];
                    if (mmp->getApertureRcModel())
                        ev->Rc1[gp] = mmp->getApertureRcModel()->Rc(ev->b1[gp]);
                } else { // no contact
                    ev->b1[gp] = ev->b0[gp] + ev->db[gp];
                }
            }
        }
    }

}

void updateElementProperties(MeshLib::CFEMesh* m_msh, CFiniteElementStd* fem, CNumerics* m_num)
{
    const size_t mesh_ele_vector_size(m_msh->ele_vector.size());
    for (size_t i = 0; i < mesh_ele_vector_size; i++)
    {
        MeshLib::CElem* elem = m_msh->ele_vector[i];
        if (elem->GetMark() && mmp_vector[elem->GetPatchIndex()]->isFracture())
        {
            fem->ConfigElement(elem, m_num->ele_gauss_points);
            fem->Config();
            fem->ConvertGPValuesToEleValues(); // for output

            ElementValue* eval = ele_gp_value[i];
            eval->b0.assign(eval->b1.begin(), eval->b1.end());
            eval->Rc0.assign(eval->Rc1.begin(), eval->Rc1.end());

            //this->SetElementValue(idCi, i, ele_gp_value[i]->Ci1);
        }
    }
}

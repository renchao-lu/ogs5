
#include <Aperture.h>
#include <cfloat>

#include "Configure.h"

#include "fem_ele_std.h"
#include "mathlib.h"
#include "rf_mmp_new.h"
#include "rf_msp_new.h"
#include "rfmat_cp.h"
#include "pcs_dm.h"
#include "PhysicalConstant.h"
#include "C1_Common.h"

using namespace std;
using namespace PhysicalConstant;

namespace FiniteElement
{

void CFiniteElementStd::CalculateMassTransferRate()
{
	ElementValue* const element_gp_values = ele_gp_value[Index];

	// For each mineral
	for (unsigned im = 0; im < vec_mineral_cp.size(); im++)
	{
		const CompProperties* mineral = vec_mineral_cp[im];

		// get nodal reaction rate for this mineral
//		CRFProcess* pcsMineral = PCSGetMass(mineral->idx);
//		const int idx = pcsMineral->GetNodeValueIndex(mineral->pqc_kinetic_product_name + "_dmdt");
		int num = mineral->pqc_kinetic_product_number;

		std::vector<CRFProcess*> pcsSolute(num);
		std::vector<int> idx(num);

		for (int i = 0; i < num; i++){
			pcsSolute[i] = PCSGetMass(cp_name_2_idx[mineral->pqc_kinetic_product_name[i]]);
			idx[i] = pcsSolute[i]->GetNodeValueIndex(mineral->pqc_kinetic_product_name[i] + "_dmdt");
		}
				
		for (int i = 0; i < nnodes; i++){
			NodalVal_dCdt[i] = pcsSolute[0]->GetNodeValue(nodes[i], idx[0]) / mineral->pqc_kinetic_product_stoichiometry[0];
			for (int j = 1; j < num; j++){
				NodalVal_dCdt[i] -= pcsSolute[j]->GetNodeValue(nodes[i], idx[j]) / mineral->pqc_kinetic_product_stoichiometry[j];
			}
		}

		// For each Gauss point
		for (gp = 0; gp < nGaussPoints; gp++)
		{
			//---------------------------------------------------------
			//  Get local coordinates and weights
			//  Compute Jacobian matrix and its determination
			//  Compute geometry
			//---------------------------------------------------------
			{
				int gp_r, gp_s, gp_t;
				GetGaussData(gp, gp_r, gp_s, gp_t);
				ComputeShapefct(1);
			}

			//---------------------------------------------------------
			// Get gauss point values
			//---------------------------------------------------------
			//const double gp_T = interpolate(NodalVal_t1); // temperature [K]
			//assert(gp_T > .0);
			const double rho_f = FluidProp->Density(); // water density [kg/m3]
			const double gp_b = MediaProp->FractureAperture(Index, gp);
			const double gp_Rc = MediaProp->FractureContactAreaRatio(Index, gp);
			const double gp_dCdt = interpolate(NodalVal_dCdt); // reaction rate computed by PQC [dC/dt]

			//---------------------------------------------------------
			// Compute mass removal rate due to reactions
			//---------------------------------------------------------
			if (mineral->pqc_kinetic_mode == 1) { // FD
				double dCdt_fd = gp_dCdt; // [mol/kgW/s]
				double dmdt_fd = dCdt_fd * rho_f * gp_b * (1. - gp_Rc);  // [mol/m2/s]
				double dm_fd = dmdt_fd * dt;
				element_gp_values->dm_fd[im][gp] = dm_fd;
				element_gp_values->dmdt_fd[im][gp] = dmdt_fd;
			} else if (mineral->pqc_kinetic_mode == 2) { //PS
				double dCdt_ps = gp_dCdt; // [mol/kgW/s]
				double dmdt_ps = dCdt_ps * rho_f * gp_b; // * (1.-gp_Rc) * MediaProp->IntergranularThickness();
				double dm_ps = dmdt_ps * dt;  // [mol/m2]
				element_gp_values->dm_ps[im][gp] = dm_ps;
				element_gp_values->dmdt_ps[im][gp] = dmdt_ps;
			}
		}
	}
}

void CFiniteElementStd::CalculateAperture()
{
	ElementValue* const ele_val = ele_gp_value[Index];
	double avg_total_dbdt_fd = .0;
	double avg_total_dbdt_ps = .0;

	// For each Gauss point
	for (gp = 0; gp < nGaussPoints; gp++)
	{
		//---------------------------------------------------------
		//  Get local coordinates and weights
		//  Compute Jacobian matrix and its determination
		//  Compute geometry
		//---------------------------------------------------------
		{
			int gp_r, gp_s, gp_t;
			GetGaussData(gp, gp_r, gp_s, gp_t);
			ComputeShapefct(1);
		}
		//---------------------------------------------------------
		// Get gauss point values
		//---------------------------------------------------------
		//const double gp_p = interpolate(NodalVal_p1); // pressure [Pa]
		const double gp_T = T_Process ? interpolate(NodalVal_t1) : (273.15 + 25.0); // temperature [K]
		assert(gp_T > .0);
		//const double effective_stress = getEffectiveStress(pcs, gp_p); //Pa
		const double gp_Rc = MediaProp->FractureContactAreaRatio(Index, gp);

		//---------------------------------------------------------
		// Compute aperture changes
		//---------------------------------------------------------
		std::vector<double> dbdt_fd(vec_mineral_cp.size());
		std::vector<double> dbdt_ps(vec_mineral_cp.size());
		double total_dbdt_fd = .0;
		double total_dbdt_ps = .0;

		// For each mineral
		for (unsigned i=0; i<vec_mineral_cp.size(); i++)
		{
			const CompProperties* mineral = vec_mineral_cp[i];
			if (mineral->pqc_kinetic_mode==1)
			{
				// aperture changes due to free surface dissolution
				const double dmdt_fd = ele_val->dmdt_fd[i][gp];
				const double dVdt_fd = mineral->Vm * dmdt_fd;
				const double A_fd = (1. - gp_Rc) * MediaProp->PoreReactiveSurfaceAreaFactor(gp_T) * mineral->surface_area_ratio_of_mineral[1];
				dbdt_fd[i] = dVdt_fd / A_fd;
				// mean value 
				total_dbdt_fd += dbdt_fd[i] * mineral->surface_area_ratio_of_mineral[1];
			}
			else if (mineral->pqc_kinetic_mode==2)
			{
				// aperture changes at contact
				const double dmdt_ps = ele_val->dmdt_ps[i][gp];
				const double dVdt_ps = - mineral->Vm * dmdt_ps; // negative due to dissolution
				const double A_ps = gp_Rc * MediaProp->ContactReactiveSurfaceAreaFactor() * mineral->surface_area_ratio_of_mineral[2];// * grain.alpha;
				dbdt_ps[i] = dVdt_ps / A_ps;
				// mean value 
				total_dbdt_ps += dbdt_ps[i] * mineral->surface_area_ratio_of_mineral[2];
			}

		}

		//---------------------------------------------------------
		avg_total_dbdt_fd += total_dbdt_fd;
		avg_total_dbdt_ps += total_dbdt_ps;

		//---------------------------------------------------------
		// sum up
		//const double total_dbdt = dbdt_fd + dbdt_ps;
		const double total_dbdt = total_dbdt_fd + total_dbdt_ps;

		//---------------------------------------------------------
		// first-order estimate of aperture changes
		const double gp_db = total_dbdt * dt;
		ele_val->db[gp] = gp_db;
#if 0
		const double gp_b1 = ele_val->b0[gp] + gp_db;
		ele_val->b1[gp] = gp_b1;
		if (gp_b1 < .0)
		{
			if (MediaProp->_f_b_rc_model!=NULL)
			ele_val->b1[gp] = MediaProp->_f_b_rc_model->ResidualAperture();
			else
			ele_val->b1[gp] = 4e-9; //TODO residual aperture
			std::cout << "-> *** WARNING: negative aperture is not allowed. set it zero. ele=" << Index << "\n";
		}

		//---------------------------------------------------------
		// update contact area ratio if aperture decreases
		if (total_dbdt < 0)
		{
			if (MediaProp->getApertureRcModel())
			{
				const double new_RC = MediaProp->getApertureRcModel()->Rc(gp_b1);
				ele_val->Rc1[gp] = new_RC;
			}
		}
		else
		{
			//TODO how Rc should change if aperture increases
			if (gp_b1 > MediaProp->ApertureContactThreshold())
			ele_val->Rc1[gp] = 0;
		}
#endif

	}
	ele_val->db_fd = avg_total_dbdt_fd / nGaussPoints * dt;
	ele_val->db_ps = avg_total_dbdt_ps / nGaussPoints * dt;
}

void CFiniteElementStd::ConvertGPValuesToEleValues()
{
	ElementValue* ele_val = ele_gp_value[Index];
	//---------------------------------------------------------
	// Fracture properties
	//---------------------------------------------------------
	if (ele_val->b1.size()==(unsigned)nGaussPoints)
	{
		// take average
		double ele_b = .0;
		double ele_db = .0;
		double ele_Rc = .0;
		for (gp = 0; gp < nGaussPoints; gp++)
		{
			ele_b += ele_val->b1[gp];
			ele_db += ele_val->db[gp];
			ele_Rc += ele_val->Rc1[gp];
		}
		ele_b /= nGaussPoints;
		ele_db /= nGaussPoints;
		ele_Rc /= nGaussPoints;

		// update
		static int id_b1 = -1;
		if (id_b1 < 0)
			id_b1 = getDistributionIDforAperture(pcs);
		MeshElement->mat_vector(id_b1) = ele_b;
		static int id_Rc1 = -1;
		if (id_Rc1 < 0)
			id_Rc1 = getDistributionIDforRc(pcs);
		MeshElement->mat_vector(id_Rc1) = ele_Rc;
		static int id_db = -1;
		if (id_db < 0)
			id_db = getDistributionIDforDb(pcs);
		MeshElement->mat_vector(id_db) = ele_db;
	}

	//---------------------------------------------------------
	// Reaction rates
	//---------------------------------------------------------
	for (unsigned im=0; im<vec_mineral_cp.size(); im++)
	{
		const CompProperties* cp = vec_mineral_cp[im];
		CRFProcess* cp_pcs = PCSGetMass(cp->idx);
		if (cp->pqc_kinetic_mode==1)
		{
			double avg_dmdt_fd = .0;
			for (gp = 0; gp < nGaussPoints; gp++)
				avg_dmdt_fd += ele_gp_value[Index]->dmdt_fd[im][gp];
			const double dmdt_fd_ele_avg = avg_dmdt_fd / nGaussPoints;
			int id_dmdt_fd = cp_pcs->GetElementValueIndex(cp->compname + "_dmdt_fd") + 1;
			cp_pcs->SetElementValue(Index, id_dmdt_fd, dmdt_fd_ele_avg);
		}
		else if (cp->pqc_kinetic_mode==2)
		{
			double avg_dmdt_ps = .0;
			for (gp = 0; gp < nGaussPoints; gp++)
				avg_dmdt_ps += ele_gp_value[Index]->dmdt_ps[im][gp];
			const double dmdt_ps_ele_avg = avg_dmdt_ps / nGaussPoints;
			int id_dmdt_ps = cp_pcs->GetElementValueIndex(cp->compname + "_dmdt_ps") + 1;
			cp_pcs->SetElementValue(Index, id_dmdt_ps, dmdt_ps_ele_avg);
		}
	}
}

}                                                 // end namespace


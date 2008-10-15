/**************************************************************************
rf_kinreact.cpp

                                    KINETIC REACTIONS

FEMLib-Object KinReact

Programming:
01/2004    Dirk Schaefer      Original Implementation
02/2006    Sebastian Bauer    Adaption to C++ Class structure, new FEM concept
05/2007    Dirk Schaefer      Addition of NAPL-dissolution

***************************************************************************/
#include "rf_kinreact.h"
#include "stdafx.h" /* MFC */
#include "stdio.h"
#include "makros.h"
#include "tools.h"
#include "geo_strings.h"
#include "rfmat_cp.h"
#include "rf_mfp_new.h"
#include "rf_msp_new.h"
#include "msh_lib.h"
#include "rf_mmp_new.h"
#include "elements.h"
#include "nodes.h"
#include <vector>
#include <iostream>
#include <fstream>
//#include "msh_mesh.h"
using namespace std;
using SolidProp::CSolidProperties;

vector <CKinReact*> KinReact_vector; // declare instance CKinReact_vector
vector <CKinReactData*> KinReactData_vector; // declare instance CKinReact_vector
vector <CKinBlob*> KinBlob_vector;           // declare extern instance of class Blob

static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))
#define DMIN(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) < (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))


/* Constructor for MonodSubstruct */
MonodSubstruct::MonodSubstruct(void){
species = "blubb";
speciesnumber = -1;
concentration = -1.0e9;
order = -99.0;
}
/* Destructor for MonodSubstruct */
MonodSubstruct::~MonodSubstruct(void){
}


/***************************************************************************
FEMLib-Method: 
Task: CKinReact constructor
Programing:
02/2006 SB Implementation
06/2007 DS NAPL-dissolution added
***************************************************************************/
CKinReact::CKinReact(void){

	name = "NULL";
	type = "NULL";
	number_reactionpartner = 0;
	reactionpartner.clear();
	stochmet.clear();
	rateconstant = 0.0;
	rateorder = 0.0;
	number_monod=0;
	number_inhibit=0;
	number_production=0;
	monod.clear();
	inhibit.clear();
	production.clear();
    bacteria_name = "NULL";
    bacteria_number = -1;
    ProductionStoch.clear();
	grow = -1;
	ProdStochhelp.clear();
	ex_param.clear();
	ex_species.clear();
	ex_species_names.clear();
	exSurfaceID=-1;
	exType = "NULL";
//NAPL-dissolution
	blob_name = "NULL";
	blob_ID = -1;               
    Csat_pure = 0.; 
	current_Csat = 0.;
	Density_NAPL = 0.;           
//
	typeflag_monod=0;
	typeflag_exchange=0;
    typeflag_exchange_linear = 0;
    typeflag_exchange_langmuir = 0;
    typeflag_exchange_freundlich = 0;
    typeflag_napldissolution = 0;
}


/***************************************************************************
FEMLib-Method: 
Task: CKinReact destructor
Programing:
02/2006 SB Implementation
***************************************************************************/
CKinReact::~CKinReact(void){


}


/**************************************************************************
FEMLib-Method: 
Task: OBJ read function
Programing:
02/2004 SB Implementation
**************************************************************************/
bool KRRead(string file_base_name)
{
  CKinReact *m_kr = NULL, *m_kr1=NULL;
  CKinReactData *m_krd = NULL;
  CKinBlob *m_bp = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  string krc_file_name, database_file_name;
  int i, found, length;
  ios::pos_type position;
  bool w;
  string m_name, sp_name;
//  CompProperties *m_cp = NULL;


  //========================================================================
  // File handling
  krc_file_name = file_base_name + KRC_FILE_EXTENSION;
  ifstream krc_file (krc_file_name.data(),ios::in);
  if (!krc_file.good()) return false;
  krc_file.seekg(0L,ios::beg); // rewind?

  KRCDelete();
  //========================================================================
  // Keyword loop
  cout << "KinReact Read" << endl;
  while (!krc_file.eof()) {
    krc_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      break;
    //----------------------------------------------------------------------
    if(line_string.find("#REACTION")!=string::npos) { // keyword found  // Read kinetic Reactions
      m_kr = new CKinReact();
      w = m_kr->Read(&krc_file);
      m_kr->number = (int)KinReact_vector.size();
      KinReact_vector.push_back(m_kr);
    } // keyword found
    if(line_string.find("#KINREACTIONDATA")!=string::npos) { // keyword found  // Read KinReactData
      m_krd = new CKinReactData();
      w = m_krd->Read(&krc_file);
      KinReactData_vector.push_back(m_krd);
    } // keyword found
    if(line_string.find("#BLOB_PROPERTIES")!=string::npos) { // keyword found  // Read BlobProperties
      m_bp = new CKinBlob();
      w = m_bp->Read(&krc_file);
	  KinBlob_vector.push_back(m_bp);
    } // keyword found
  } // eof
  // Close input file
  krc_file.close();

  if(!m_krd){
	  cout << " No keyword #KINREACTIONDATA specified - setting defaults" << endl;
	  m_krd = new CKinReactData();
      KinReactData_vector.push_back(m_krd);
  }

  //========================================================================

  /* Check, if data has to be completed from database file */
//  cout << "checking database" << endl;
  if(KinReact_vector.size() > 0){

	// File handling, open reaction database file
	database_file_name = "reactions.dbf";
	ifstream dbf_file (database_file_name.data(),ios::in);
//	if (!dbf_file.good()) cout << " No database file found " << endl;

	// go through all reactions in reaction vector
	length = (int)KinReact_vector.size();
	for(i=0;i<length;i++){
		m_kr = KinReact_vector[i];
		// if no type is given in input file, only name, then read from database file
		if(m_kr->type == "NULL"){
			found=0;
			dbf_file.seekg(0L,ios::beg); // rewind database file
			while (!dbf_file.eof() && found == 0) {
				if(!GetLineFromFile(line,&dbf_file)){break;}
				line_string = line;
    			if(line_string.find("#REACTION")!=string::npos) { // keyword found
					m_kr1 = new CKinReact();
					position = m_kr1->Read(&dbf_file);
					//check if this is the right one
					if(m_kr->name ==m_kr1->name){
						// Insert in Reaction vector and remove old reaction (replacement)
						KinReact_vector.insert(KinReact_vector.begin()+i+1,m_kr1);
						KinReact_vector.erase(KinReact_vector.begin()+i);
						found=1;
					}
					else // not the right equation:
						delete m_kr1;
				} // end if(line_str..)  keyword found
			} // eof dbf-file

			if(found == 0){ 
				// reaction not complete in input file and also not specified in dbf_file
				cout << " ERROR! Reaction " << m_kr->name << "  not found in database file" << endl;
				cout << " Reaction removed from set of reaction " << endl;
				// remove reaction from reaction vector
				KinReact_vector.erase(KinReact_vector.begin()+i);
			}
  		} // end of if(m_kr=NULL
	} //end for
// close database file
  dbf_file.close();
  } // end if kinreaction_vec.size() >
/* end data base file read for reactions  */

  //========================================================================

  /* check reaction data consistency */
  cout << " Checking reaction data consistency for " << (int)KinReact_vector.size() << " specified reactions " << endl;
  length = (int)KinReact_vector.size();
  for(i=0;i<length;i++){
		m_kr = KinReact_vector[i];
		if(!m_kr->CheckReactionDataConsistency()){
			cout << " ERROR! Reaction " << m_kr->name << "  removed from set of reactions due to data inconsistency" << endl;
		KinReact_vector.erase(KinReact_vector.begin()+i);
		}
  }//end for(i=0;.. consistency check

  return true;

}




/**************************************************************************
FEMLib-Method: 
Task: OBJ configure function
Programing:
02/2004 SB Implementation
05/2007 DS NAPL dissolution added
**************************************************************************/
void KRConfig(void)
{
  CKinReact *m_kr = NULL;  //, *m_kr1=NULL;
  CKinReactData *m_krd = NULL;
  int i, j, k,  length; 
  int idx, idummy;
  long l, ll;
  string m_name, sp_name, s_geo_name, s_geo_type;
  CompProperties *m_cp = NULL;
  CRFProcess* m_pcs = NULL;
  vector<long>nodes_vector;

  CMediumProperties *m_mat_mp = NULL;
  long group;
  double foc, ww, w;

  string dummy;
  bool ok = true;

   //========================================================================
  if(KinReactData_vector.size() > 0){
	m_krd = KinReactData_vector[0];
	if(m_krd == NULL) cout << " Error - no m_krd data " << endl;
// Set up additional data structures for calculations

// Set number of reactions
   m_krd->NumberReactions = (int) KinReact_vector.size();
   length = (int)cp_vec.size();

// Check if all reaction partners are specified as processes
   for(j=0;j<m_krd->NumberReactions;j++){
	   m_kr = KinReact_vector[j];
	   // Check presence of bacteria species
	   if(m_kr->bacteria_name.compare("NULL") != 0){
		   m_pcs = PCSGet("MASS_TRANSPORT",m_kr->bacteria_name);
			if(m_pcs == NULL) {
				cout << " Warning: Component " << m_kr->bacteria_name << " specified in KinReact as biomass but not given as transport process " << endl;
				ok=false;
			}
		}
	   // Check if monod species are specified as components
	   for(i=0;i<(int)m_kr->number_monod;i++){
			m_pcs = PCSGet("MASS_TRANSPORT", m_kr->monod[i]->species);
			if(m_pcs == NULL) {
				cout << " Warning: Component " <<  m_kr->monod[i]->species << " specified in KinReact as monod species but not given as transport process " << endl;
				ok=false;
			}
		}
	   // Check if inhibition species are specified as components
	   for(i=0;i<(int)m_kr->number_inhibit;i++){
			m_pcs = PCSGet("MASS_TRANSPORT", m_kr->inhibit[i]->species);
			if(m_pcs == NULL) {
				cout << " Warning: Component " <<  m_kr->inhibit[i]->species << " specified in KinReact as inhibition species but not given as transport process " << endl;
				ok=false;
			}
		}
	   // Check productionstoch
	   for(i=0;i<(int)m_kr->ProdStochhelp.size();i++){
			m_pcs = PCSGet("MASS_TRANSPORT", m_kr->ProdStochhelp[i]->species);
			if(m_pcs == NULL) {
				cout << " Warning: Component " <<  m_kr->ProdStochhelp[i]->species << " specified in KinReact as produced species but not given as transport process " << endl;
				ok=false;
			}
		}
	   if(m_kr->type.compare("exchange")==0)
		   for(i=0;i<m_kr->number_reactionpartner;i++){
				m_pcs = PCSGet("MASS_TRANSPORT", m_kr->reactionpartner[i]);
				if(m_pcs == NULL) {
					cout << " Warning: Component " <<  m_kr->reactionpartner[i] << " specified in KinReact as reaction partner but not given as transport process " << endl;
					ok=false;
				}
	   }
  }
   if(ok==false){
		cout << " Components missing, Stopping" << endl;
		cout.flush();
		exit(0);
   }

// Set vector is_a_bacterium
//   cout << " Length of cp_vec: " << length << endl;
   for(j=0; j < length; j++)
        m_krd->is_a_bacterium.push_back(0); //initialize
   for(j=0;j<m_krd->NumberReactions;j++){
	  m_kr = KinReact_vector[j];
	  if(m_kr->type.compare("monod")==0)
	   for(i=0;i<length;i++){
		   m_cp = cp_vec[i];
           if(m_kr->bacteria_name.compare(m_cp->compname) == 0){
             m_krd->is_a_bacterium[i]=1;
//             cout << " is_a_bacterium[" << i << "] set to 1" << endl;
           }
	   }
   }
   // Set Vector ProductionStoch for each reaction
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("monod")==0){
		for(i=0;i<length;i++)
			m_kr->ProductionStoch.push_back(0.0); //initialize
	// Get Stochiometry
/*
	for(k=0;k<(int)m_kr->reactionpartner.size();k++){
		m_name = m_kr->reactionpartner[k];
		for(i=0;i<length;i++)
			if(m_name.compare(cp_vec[i]->compname) == 0)
				m_kr->ProductionStoch[i] = m_kr->stochmet[k];
	}
	*/
	for(k=0;k<(int)m_kr->ProdStochhelp.size();k++){
		m_name = m_kr->ProdStochhelp[k]->species;
		for(i=0;i<length;i++)
			if(m_name.compare(cp_vec[i]->compname) == 0)
				m_kr->ProductionStoch[i] = m_kr->ProdStochhelp[k]->concentration;
	}
	} //if type == monod
   } // vector ProductionStoch

   // Set numbers for monod species for each reaction
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("monod")==0)
	for(i=0;i<(int)m_kr->monod.size();i++){
		m_name = m_kr->monod[i]->species;
		for(k=0;k<length;k++)
			if(m_name.compare(cp_vec[k]->compname) == 0)
				m_kr->monod[i]->speciesnumber = k;
	}
   } // monod substructure numbers

   // Set numbers for inhibition species for each reaction
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("monod")==0)
	for(i=0;i<(int)m_kr->inhibit.size();i++){
		m_name = m_kr->inhibit[i]->species;
		for(k=0;k<length;k++)
			if(m_name.compare(cp_vec[k]->compname) == 0)
				m_kr->inhibit[i]->speciesnumber = k;
	}
   } // inhibition substructure numbers

   // Set bacteria number for each reaction
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("monod")==0){
	m_name = m_kr->bacteria_name;
	for(k=0;k<length;k++){
		m_cp = cp_vec[k];
		if(m_name.compare(m_cp->compname) == 0)
			m_kr->bacteria_number = k;
	}
	}
   } //bacteria numbers

   // Set flags type_monod and type_exchange
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	dummy=m_kr->type;
	if(m_kr->type.compare("monod")==0) m_kr->typeflag_monod=1;
	if(m_kr->type.compare("exchange")==0){
		m_kr->typeflag_exchange=1;
		if(m_kr->exType.compare("linear")==0) m_kr->typeflag_exchange_linear=1;
		if(m_kr->exType.compare("langmuir")==0) m_kr->typeflag_exchange_langmuir=1;
		if(m_kr->exType.compare("freundlich")==0) m_kr->typeflag_exchange_freundlich=1;
	}
   	if(m_kr->type.compare("NAPLdissolution")==0) m_kr->typeflag_napldissolution=1;
   } //typeflags


// exchange reactions
  for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("exchange")==0){
		// move names from equation to vector with names
		for(i=0;i<(int)m_kr->reactionpartner.size(); i++){
			m_name = m_kr->reactionpartner[i];
			m_kr->ex_species_names.push_back(m_name);
		//	find species numbers for soecies names
			for(k=0;k<length;k++)
				if(m_name.compare(cp_vec[k]->compname) == 0)
					m_kr->ex_species.push_back(k);
		}
	}
  } //exchange species numbers

//#ds NAPLdissolution reaction
  for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("NAPLdissolution")==0){
		// move names from equation to vector with names
		for(i=0;i<(int)m_kr->reactionpartner.size(); i++){
			m_name = m_kr->reactionpartner[i];
			m_kr->ex_species_names.push_back(m_name);
		//	find species numbers for species names
			for(k=0;k<length;k++)
				if(m_name.compare(cp_vec[k]->compname) == 0)
					m_kr->ex_species.push_back(k);
		}
	}

    //set index numbers for blob_id
      CKinBlob *m_kb = NULL;
      for (i=0; i<(int) KinBlob_vector.size(); i++){
	    m_kb = KinBlob_vector[i];
		if (m_kr->blob_name.compare(m_kb->name)==0 ) {
		  m_kr->blob_ID = i;
		}
      }

  } //NAPLdissolution species numbers

  // exchange reactions numbers for m_krd
  for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->exType.compare("linear")==0) m_krd->NumberLinear++;
	if(m_kr->exType.compare("langmuir")==0) m_krd->NumberLangmuir++;
	if(m_kr->exType.compare("freundlich")==0) m_krd->NumberFreundlich++;
	if(m_kr->type.compare("monod")==0) m_krd->NumberMonod++;
	if(m_kr->type.compare("NAPLdissolution")==0) m_krd->NumberNAPLdissolution++;
  }
/*
  // set up vector sp_index
   length = (int)cp_vec.size();
   for(j=0; j < length; j++)
	   m_krd->sp_index.push_back(-10); //initialize
   // Go through all reactions and set sp_index[x] active, if species x is found
   for(j=0;j<m_krd->NumberReactions;j++){
		m_kr = KinReact_vector[j];
		if(m_kr->type.compare("monod")==0){
			if(m_kr->bacteria_number > 0) m_krd->sp_index[m_kr->bacteria_number] = 1;
			for(k=0;k<m_kr->number_monod;k++)
				if(m_kr->monod[k]->speciesnumber > 0) m_krd->sp_index[m_kr->monod[k]->speciesnumber] = 1;
			for(k=0;k<m_kr->number_inhibit;k++)
				if(m_kr->inhibit[k]->speciesnumber > 0) m_krd->sp_index[m_kr->inhibit[k]->speciesnumber] = 1;
			for(k=0;k<m_kr->number_production;k++)
				if(m_kr->production[k]->speciesnumber > 0) m_krd->sp_index[m_kr->production[k]->speciesnumber] = 1;
			for(k=0;k<(int)m_kr->ProdStochhelp.size();k++)
				if(m_kr->ProdStochhelp[k]->speciesnumber > 0) m_krd->sp_index[m_kr->ProdStochhelp[k]->speciesnumber] = 1;
			for(k=0;k<length;k++)
				if(m_kr->ProductionStoch[k] !=0) m_krd->sp_index[k] = 1;
		}
		if(m_kr->type.compare("exchange")==0){
			for(k=0;k<(int)m_kr->ex_species.size();k++)
				if(m_kr->ex_species[k] > 0) m_krd->sp_index[m_kr->ex_species[k]] = 1;

		}
	}

   // renumber in sp_index
   count =0;
   for(j=0;j<length;j++)
	   if(m_krd->sp_index[j] > 0){
		   m_krd->sp_index[j] = count;
		   count++;
		}
   // output for checking
   m_krd->kr_active_species = count;
   cout << "m_krd->sp_index : " << endl;
   for(j=0;j<length;j++) cout << j << ", " << m_krd->sp_index[j] << endl;
   cout << " total number of active species: " << m_krd->kr_active_species << endl;
*/
// set up vectors sp_pcs and sp_varind
 for(j=0;j<length;j++) {
    sp_name = cp_vec[j]->compname;
	idummy = -1;
    m_pcs = PCSGet("MASS_TRANSPORT",sp_name);
	if(m_pcs != NULL){
	idummy = PCSGetPCSIndex("MASS_TRANSPORT",sp_name);
	m_krd->sp_pcsind.push_back(idummy);
    idx = m_pcs->GetNodeValueIndex(sp_name)+1; // new timelevel
	m_krd->sp_varind.push_back(idx);
//	cout << " PCS: " << j << ", " << sp_name << ", " << idummy << ", " <<idx << endl;
	}
 }
 /*
 // set up short versions of vectors is_a_bacterium2 and ProductionStoch for each reaction
 for(j=0;j<m_krd->kr_active_species;j++)
	 m_krd->is_a_bacterium2.push_back(0);
 for(j=0;j<length;j++)
	 if(m_krd->sp_index[j] > -1)
		 m_krd->is_a_bacterium2[m_krd->sp_index[j]] = m_krd->is_a_bacterium[j];
 cout << "is_a_bacterium2:" << endl;
 for(j=0;j<m_krd->kr_active_species;j++)
	 cout << m_krd->is_a_bacterium2[j] << endl;
 // Set Vector ProductionStoch2 for each reaction
   for(j=0;j<m_krd->NumberReactions;j++){
	m_kr = KinReact_vector[j];
	if(m_kr->type.compare("monod")==0){
		// initialize
		for(i=0;i<m_krd->kr_active_species;i++)
			m_kr->ProductionStoch2.push_back(0.0); //initialize
	    // Get Stochiometry from ProductionStoch
		for(i=0;i<length;i++)
			if(m_krd->sp_index[i] > -1)
				m_kr->ProductionStoch2[m_krd->sp_index[i]] = m_kr->ProductionStoch[i];
		cout << " ProductionStoch2 for reaction " << m_kr->name << endl;
		for(i=0;i<m_krd->kr_active_species;i++)
			cout << m_kr->ProductionStoch2[i] << endl;
		cout << endl << " ProductionStoch for reaction " << m_kr->name << endl;
		for(i=0;i<length;i++)
			cout << m_kr->ProductionStoch[i] << endl;
	} //if type == monod
   } // vector ProductionStoch2
*/

/********************************************************/
  // check global requirements for kinetic reactions:
  // check if porosities for phases are set
  if(m_krd->NumberMonod>0)
	for(j=0;j<(int)mmp_vector.size();j++)
		if(mmp_vector[j]->vol_bio < MKleinsteZahl) 
			cout << "Warning: Porosity of bio-phase is 0.0 ! Change Settings in *.mmp file " << endl;
//#ds  k= m_krd->NumberLinear + m_krd->NumberFreundlich + m_krd->NumberLangmuir;
  k= m_krd->NumberLinear + m_krd->NumberFreundlich + m_krd->NumberLangmuir + m_krd->NumberNAPLdissolution;
  if(k>0)
	for(j=0;j<(int)mmp_vector.size();j++)
		if(mmp_vector[j]->vol_mat < MKleinsteZahl) 
			cout << "Warning: Porosity of solid phase is 0.0 ! Change Settings in *.mmp file " << endl;

  /********************************************************/
  //Set up vector is_a_CCBC
  CFEMesh* m_msh = fem_msh_vector[0]; //SB: ToDo hart gesetzt
  if(m_msh == NULL) {cout << "No mesh in KRConfig" << endl; exit(0);}
  // Initialize vector is_a_CCBC
  for(l=0; l< (long)m_msh->nod_vector.size();l++) m_krd->is_a_CCBC.push_back(false);
  // Go through specified geometry elements
  for(j=0;j<(int)m_krd->NoReactGeoName.size();j++){
	  s_geo_name = m_krd->NoReactGeoName[j];
	  s_geo_type = m_krd->NoReactGeoType[j];
      //------------------------------------------------------------------
      if(s_geo_type.compare("POINT")==0) {
        CGLPoint* m_geo_point = NULL; // make new GEO point
        m_geo_point = GEOGetPointByName(s_geo_name);//Get GEO point by name
        if(m_geo_point)
	        l = m_msh->GetNODOnPNT(m_geo_point); // + ShiftInNodeVector; // find MSH point number stored in l
		m_krd->is_a_CCBC[l] = true;
	  }
      //------------------------------------------------------------------
      if(s_geo_type.compare("POLYLINE")==0) {
		CGLPolyline *m_polyline = NULL;
        m_polyline = GEOGetPLYByName(s_geo_name);// get Polyline by name
        if(m_polyline) {
			if(m_polyline->type==100) //WW
				m_msh->GetNodesOnArc(m_polyline,nodes_vector);
			else
                m_msh->GetNODOnPLY(m_polyline,nodes_vector);
            for(i=0;i<(long)nodes_vector.size();i++){
				ll = nodes_vector[i]; 
                l = ll; //+ShiftInNodeVector;
                m_krd->is_a_CCBC[l] = true;
            }
		}
      } // if(POLYLINE) 
      //------------------------------------------------------------------
     if(s_geo_type.compare("SURFACE")==0) {
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(s_geo_name);
        if(m_surface){
           m_msh->GetNODOnSFC(m_surface,nodes_vector);
           for(i=0;i<(long)nodes_vector.size();i++){
				ll = nodes_vector[i]; 
                l = ll; //+ShiftInNodeVector;
                m_krd->is_a_CCBC[l] = true;
           }
        }
      }      
  } // end of for(j=0;j<m_krd->NoReactGeoName.size()...
 //test output
/*  cout << " Vector KinReactData::is_a_CCBC: " << endl;
  for(l=0; l< (long)m_msh->nod_vector.size();l++) 
	  if(m_krd->is_a_CCBC[l] == true) cout << l <<", ";
  cout << endl;
*/
  nodes_vector.clear();

/********************************************************/
  //Get foc average of connected elements
	CNode* m_nod = NULL;
	CElem* m_ele = NULL;
	for(l=0; l< (long)m_msh->nod_vector.size();l++) 
		m_krd->node_foc.push_back(1.0);	
	for(l=0; l< (long)m_msh->nod_vector.size();l++) {
		m_nod = m_msh->nod_vector[l];
		ww = 0.0; 
		w = 0.0;
		for(i=0;i<(int)m_nod->connected_elements.size();i++){
			ll = m_nod->connected_elements[i];
			m_ele = m_msh->ele_vector[m_nod->connected_elements[i]];
			group = m_ele->GetPatchIndex();
			m_mat_mp = mmp_vector[group];
			ww += m_mat_mp->foc * m_ele->GetVolume();
			w += m_ele->GetVolume();
		}
		foc = ww/w;
		m_krd->node_foc[l] = foc;

	}
/********************************************************/
} // if KinReactData_vector.size() > 0
}

/**************************************************************************
FEMLib-Method:
Task: Clear KinReact_vector
Programing:
02/2006 SB Implementation
last modified:
**************************************************************************/
void KRCDelete(void){
  long i;
  int no_krc =(int)KinReact_vector.size();
  for(i=0;i<no_krc;i++){
    delete KinReact_vector[i];
  }
  KinReact_vector.clear();
}


/**************************************************************************
FEMLib-Method: 
Task: Reaction class read function
Programing:
05/2004 SB Implementation - adapted from OK rf_bc_new
02/2006 SB New Class structure, IO, C++, FEM
**************************************************************************/
bool CKinReact::Read(ifstream *rfd_file)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string, line_str1, help("1");
  string delimiter(" ");
  bool new_keyword = false, new_subkeyword = false;
  string hash("#"), dollar("$");
  std::stringstream in;
  MonodSubstruct* m_monod=NULL, *m_inhibit=NULL, *m_production=NULL;
  long index, index1;
  double dval;

//clear vectors
  monod.clear();
  inhibit.clear();
  production.clear();
  reactionpartner.clear();
  stochmet.clear();

  //========================================================================
  while (!new_keyword) {
	index = rfd_file->tellg();
//    if(!rfd_file->getline(line,MAX_ZEILE)) break;
    if(!GetLineFromFile(line,rfd_file)) break;
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
	  rfd_file->seekg(index); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
      break;
    }
	/* Keywords nacheinander durchsuchen */
    //....................................................................
    if(line_string.find("$NAME")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> name;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$TYPE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> type;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$BACTERIANAME")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> bacteria_name;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$EQUATION")!=string::npos) { // subkeyword found
      line_str1 = GetLineFromFile1(rfd_file);
	  ReadReactionEquation(line_str1);
    }
    //....................................................................
    if(line_string.find("$RATECONSTANT")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> rateconstant >> rateorder;
	  in.clear();
	}
   //....................................................................
   if(line_string.find("$GROWTH")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
	  in >> grow;
	  in.clear();
	}
   //....................................................................
    if(line_string.find("$MONODTERMS")!=string::npos) { // subkeyword found
		while(!new_subkeyword){
		index1 = rfd_file->tellg();
		line_str1 = GetLineFromFile1(rfd_file);
		// Check for end of data block
	    if((line_str1.find(hash)!=string::npos) || (line_str1.find(dollar)!=string::npos)) {
			if(line_str1.find(hash)!=string::npos) new_keyword = true;
			rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste subkeyword weg
			break;
		}
		m_monod = new MonodSubstruct();
        number_monod++;
		in.str(line_str1);
		in >> m_monod->species >> m_monod->concentration >> m_monod->order;
		if((m_monod->order != -99.0)&&(m_monod->concentration != -1.0e9)) //check for read in
			monod.push_back(m_monod);
		else{
			DisplayMsgLn(" ERROR reading Monod Terms  - skipping");
			number_monod--;
			delete m_monod;
		}
		in.clear();
	  }
    }
    //....................................................................
    if(line_string.find("$INHIBITIONTERMS")!=string::npos) { // subkeyword found
	  while(!new_subkeyword){
		index1 = rfd_file->tellg();
		line_str1 = GetLineFromFile1(rfd_file);
    	// Check for end of data block
	    if((line_str1.find(hash)!=string::npos) ||(line_str1.find(dollar)!=string::npos)) {
			if(line_str1.find(hash)!=string::npos) new_keyword = true;
			rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste subkeyword weg
			break;
		}
	    in.str(line_str1);
		m_inhibit = new MonodSubstruct();
        number_inhibit++;
		in >> m_inhibit->species;
		in >> m_inhibit->concentration;
		in  >> m_inhibit->order;
		if((m_inhibit->order != -99.0)&&(m_inhibit->concentration != -1.0e9)) //check for read in
			inhibit.push_back(m_inhibit);
		else{
			DisplayMsgLn(" ERROR reading Inhibition Terms  - skipping");
			number_inhibit--;
		}

		in.clear();
	  }
    }
    //....................................................................
    if(line_string.find("$PRODUCTIONTERMS")!=string::npos) { // subkeyword found
	  while(!new_subkeyword){
		index1 = rfd_file->tellg();
		line_str1 = GetLineFromFile1(rfd_file);
		// Check for end of data block
	    if((line_str1.find(hash)!=string::npos) ||(line_str1.find(dollar)!=string::npos)) {
			if(line_str1.find(hash)!=string::npos) new_keyword = true;
			rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste subkeyword weg
			break;
		}
		in.str(line_str1);
		m_production = new MonodSubstruct();
        number_production++;
		in >> m_production->species >> m_production->concentration >> m_production->order;
		if((m_production->order != -99.0)&&(m_production->concentration != -1.0e9)) //check for read in
			production.push_back(m_production);
		else{
			DisplayMsgLn(" ERROR reading Production Terms  - skipping");
			number_production--;
		}
		in.clear();
	  }
    }

    //....................................................................
	if(line_string.find("$PRODUCTIONSTOCH")!=string::npos) { // subkeyword found
	  while(!new_subkeyword){
		index1 = rfd_file->tellg();
		line_str1 = GetLineFromFile1(rfd_file);
		// Check for end of data block
	    if((line_str1.find(hash)!=string::npos) ||(line_str1.find(dollar)!=string::npos)) {
			if(line_str1.find(hash)!=string::npos) new_subkeyword = true;
			rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste subkeyword weg
			break;
		}
		in.str(line_str1);
		m_production = new MonodSubstruct();
		in >> m_production->species >> m_production->concentration ;
		this->ProdStochhelp.push_back(m_production);
		in.clear();
	  }


	}
    //....................................................................
	if(line_string.find("$EXCHANGE_PARAMETERS")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
	  in >> dval; // velocity
	  ex_param.push_back(dval);
	  in >> dval;   //kd
	  ex_param.push_back(dval);
	  if(this->exType.compare("langmuir")==0)
		  in >> exSurfaceID;
	  if(this->exType.compare("freundlich")==0){
		  in >> dval;
		  ex_param.push_back(dval);
	  }

	  in.clear();
	}
	 //....................................................................
	if(line_string.find("$SORPTION_TYPE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
	  in >> this->exType;
	  in.clear();
	}

	 //....................................................................
	if(line_string.find("$NAPL_PROPERTIES")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> blob_name >> Csat_pure >> Density_NAPL;
	  in.clear();

	}

	}
  return true;
}





/**************************************************************************
FEMLib-Method:
Task: 
Check consistency of reactions read in from input or database file                           
Programing:
02/2006 SB Implementation
last modified:
**************************************************************************/
int CKinReact::CheckReactionDataConsistency(){

	int i, ok=1, length;
	bool found=false;
	string name, name1;
//    CRFProcess *m_pcs = NULL;
//    int no_processes =(int)pcs_vector.size();


	// Check number of reaction partners
	if(number_reactionpartner <2){
//SB todo		ok=0;
//SB todo		cout << " Less than two reaction partners in equation found" << endl;
	}
	// Ratekonstant zero?
	if((fabs(rateconstant) < MKleinsteZahl) && (this->type.compare("monod")==0) ){
//#ds erstetzt statt:	if((fabs(rateconstant) < MKleinsteZahl) && (this->type.compare("exchange")!=0) ){
		ok=0;
		cout <<" Rateconstant is zero" << endl;
	}
	// Number of partners higher than in arrays?
	length = (int)reactionpartner.size();  //Muss so gemacht werden, weil sonst signed/unsigned warning kommt
	if(number_reactionpartner != length){
		ok=0;
		cout << " Not enough reaction partners" << endl;
		for(i=0;i<length;i++)
			cout << reactionpartner[i]<< "  ";
		cout << endl;
	}
	length = (int)stochmet.size();
	if(stochmet.size() != reactionpartner.size()){
		ok=0;
		cout << " Not enough stochieometric coefficients for reaction partners" << endl;
		for(i=0;i<length;i++)
			cout << stochmet[i]<< "  ";
		cout << endl;
	}
    // check type
	if(type.compare("monod") != 0){
		if(type.compare("exchange") != 0){
			if(type.compare("NAPLdissolution") != 0) {
			ok=0;
			cout << "Unknown reaction type" << endl;
			}
		}
	}

	/* Check Monod-, Inhibition and Production terms */
	length = (int)monod.size();
	if(length < number_monod) number_monod = (int)monod.size();
	length = (int)inhibit.size();
	if(length < number_inhibit) number_inhibit = (int)inhibit.size();
	length = (int)production.size();
	if(length < number_production) number_production = (int)production.size();

	if((number_monod+number_inhibit+number_production == 0)&&(type =="monod")){
//		cout << "Warning: no monod terms specified for monod equation "<< endl;
	}
	
	// Test concentrations and constants for > 0.0
    length = (int)monod.size();
	for(i=0;i<length;i++){
		if(this->monod[i]->concentration< MKleinsteZahl){
			cout << " Monod Concentration of reaction " << this->name << " smaller than 0.0  " << endl;
			ok=0;
		}
	}
	length = (int)this->inhibit.size();
	for(i=0;i<length;i++){
		if(this->inhibit[i]->concentration< MKleinsteZahl){
			cout << " Inhibition Concentration of reaction " << this->name << " smaller than 0.0  " << endl;
			ok=0;
		}
	}
//#ds check NAPL dissolution reactions	
	if(type.compare("NAPLdissolution") == 0) {      
	  //does blob-name exist?
      CKinBlob *m_kb = NULL;
      for (i=0; i<(int) KinBlob_vector.size(); i++){
	    m_kb = KinBlob_vector[i];
	    if (this->blob_name.compare(m_kb->name)==0 ) found=true;
      }
	  if (found==false) {
		  ok=0;
		  cout << " Blob_Name " << this->blob_name << " defined in Reaction " << this->name << " does not exist " << endl;
	  }
      // Csat_pure >= 0
	  if (this->Csat_pure < -MKleinsteZahl) {
		  ok=0;	
		  cout << " Invalid maximum solubility Csat_pure: " << this->Csat_pure << " in Reaction " << this->name << endl;		
		}
      // Density_NAPL >0
	  if (this->Density_NAPL < MKleinsteZahl) {
		  ok=0;
		  cout << " Invalid NAPL density Density_NAPL: " << this->Density_NAPL << " in Reaction " << this->name << endl;		
	  }
	} //end type=NAPLdissolution


	return ok;
}

/**************************************************************************
Reaction-Method: 
Task: Reaction class read function
Programing:
05/2004 SB Implementation - adapted from OK rf_bc_new
02/2006 SB Adapted to new FEM structure
**************************************************************************/
void CKinReact::Write(ofstream *rfe_file){

int i, flag=0, length;

// Write Keyword
*rfe_file << "#REACTION" << endl;
// Name of reaction
*rfe_file << "$NAME" << endl << name << endl;
// Type of reaction
*rfe_file << "$TYPE" << endl << type << endl;
// bacteria name
if(this->type=="monod")	*rfe_file << "$BACTERIANAME" << endl << bacteria_name << endl;
if(this->type=="exchange") *rfe_file << "$SORPTION_TYPE" << endl << this->exType << endl;
//ReactionEquation
*rfe_file << "$EQUATION" << endl ; 
for(i=0;i<number_reactionpartner;i++){
	if(stochmet[i] < 0.0){ //left side of equation
		if(i == 0) 
			*rfe_file << " " << fabs(stochmet[i]) << " " << reactionpartner[i];
		else
			*rfe_file << " + " << fabs(stochmet[i]) << " " << reactionpartner[i];
	}
	if(stochmet[i]>0 && (flag>0)) // remaining right hand side
		*rfe_file << " + " << fabs(stochmet[i]) << " " << reactionpartner[i];
	if(stochmet[i]>0 && (flag==0)){ // " = " Sign and first term on right hand side
		*rfe_file << " = " << fabs(stochmet[i]) << " " << reactionpartner[i];
		flag = 1;
	}
}
*rfe_file << endl;
// Rateconstant and order
if(this->type=="monod"){
	*rfe_file << "$RATECONSTANT" << endl << rateconstant << "   " << rateorder << endl;
	*rfe_file << "$GROWTH" << endl << grow << endl;
	//Monod terms
	*rfe_file << "$MONODTERMS" << endl ; //<< number_monod << endl;
	for(i=0;i<number_monod;i++)
		*rfe_file << monod[i]->species << "  " << monod[i]->concentration << "  " << monod[i]->order << endl;
	//Inhibition terms
	*rfe_file << "$INHIBITIONTERMS" << endl; // << number_inhibit << endl;
	for(i=0;i<number_inhibit;i++)
		*rfe_file << inhibit[i]->species << "  " << inhibit[i]->concentration << "  " << inhibit[i]->order << endl;
	// Production Terms
	//*rfe_file << "$PRODUCTIONTERMS" << endl << number_production << endl;
	//for(i=0;i<number_production;i++)
	//	*rfe_file << production[i]->species << "  " << production[i]->concentration << "  " << production[i]->order << endl;
	// Production Terms
	length = (int)this->ProdStochhelp.size();
	*rfe_file << "$PRODUCTIONSTOCH" << endl; // << length << endl;
	for(i=0;i<length;i++)
		*rfe_file << this->ProdStochhelp[i]->species << "  " << this->ProdStochhelp[i]->concentration << "  " << endl;

	//#ds output für NAPL-dissolution 
//	*rfe_file << "$NAPL_PROPERTIES" << endl;
//	*rfe_file << "blob_name " << blob_name << " Csat_pure " << Csat_pure << " Density_NAPL " << Density_NAPL << endl;
}
if(this->type=="exchange"){
	*rfe_file << "EXCHANGE_PARAMETERS" << endl;
	length = (int)this->ex_param.size();
	for(i=0;i<length;i++)
		*rfe_file << ex_param[i] << "  ";
	*rfe_file << endl;
}
*rfe_file << endl;

}



/**************************************************************************
Reaction-Method: 
Task: Reaction class read function
Programing:
05/2004 SB Implementation 
02/2006 SB Adapted to new FEM structure
**************************************************************************/
void CKinReact::ReadReactionEquation(string line_string_all){

string line_string, name, helpstring, c, calt;
string substring, subsubstring;
int indexlow, indexhigh, help1, strl, linelength, ih1, ih2, ih3, indexgleich, i, partners, p;
double zahl, vz=-1.0;


stochmet.clear();
reactionpartner.clear();

//Anfängliche Leerzeichen, Kommentar am Ende raus
 ih1 = (int)line_string_all.find_first_not_of(" ",0);
 ih2 = (int)line_string_all.find_first_of(";",ih1);
 ih3 = (int)line_string_all.length();
 line_string = line_string_all.substr(ih1,ih2-ih1);

 //Auf mehrfache Leerzeichen und Tabs überprüfen - ersetzt durch je ein Leerzeichen
 ih3 = (int)line_string.length();
 for(i=0;i<ih3;i++){
 c = line_string[i];
 if(c=="	") c=" "; //replace tab by space
 if((c==" ")&&(calt==" "))
	ih1++; //nothing
 else{
	helpstring.append(c);
	calt = c;
 }
 }

 // Leerzeichen am Ende raus
 ih1 = (int)helpstring.find_first_not_of(" ",0);
 ih2 = (int)helpstring.find_last_of(" ");
 ih3 = (int)helpstring.length();
 if(ih2 == ih3-1)// Leerzeichen am Ende
	line_string = helpstring.substr(ih1,ih2-ih1);
 else
	line_string = helpstring.substr(ih1,ih3-ih1);


 //Count reaction partners by looking for " + "
linelength = (int)line_string.length();
indexhigh=0;
partners=0;
ih1=(int)line_string.find(" = ");
if(ih1 < 0){
	DisplayMsgLn(" Error in keyword REACTION");
	// Exception handling
	}
while(indexhigh < linelength){
ih1 = (int)line_string.find(" + ",indexhigh);
if(ih1>0){ 
	partners++;
	indexhigh=ih1+3;
}
else //no " + " found
	indexhigh=linelength;
}
// Number of partners is 2 for " = " and one additional for each " + "
number_reactionpartner = partners+2;

/* Zerlegen der Gleichung in Blöche mit "Zahl Namen"*/

 indexhigh = 0;
 p=0; //Zaehlt partner hoch
 indexlow = indexhigh;
 linelength = (int)line_string.length();

 indexgleich = (int)line_string.find(" = ");

 while(indexhigh < linelength){

  /* einen Block holen  - find next occurence of +, -, = */
 ih1 = (int)line_string.find(" + ",indexlow);
 if(ih1 <0) ih1 = linelength;
 ih2 = (int)line_string.find(" - ",indexlow);
 if(ih2 <0) ih2 = linelength;
 ih3 = (int)line_string.find(" = ",indexlow);
 if(ih3 <0) ih3 = linelength;
 indexhigh = min(ih1, min(ih2,ih3));
 if(indexhigh > indexgleich) vz = 1.0; //rechte Seite der Gleichung: Vorzeichenwechsel
 substring = line_string.substr(indexlow,indexhigh-indexlow);

/* Leerzeichen drin ? */
help1 = (int)substring.find(" ",0);
strl = (int)substring.length();
if(help1 > 0){ //Leerzeichen gefunden
	subsubstring = substring.substr(0,help1);
//	if(subsubstring.find(".",0)>0) //double value given
	zahl = atof(subsubstring.data())*vz;
	stochmet.push_back(zahl);
	subsubstring = substring.substr(help1+1,strl-help1-1);
	// reactionpartner[p] = subsubstring.data();
//	sprintf(reactionpartner[p],"%s", subsubstring.data());
	reactionpartner.push_back(subsubstring);
}
else{ //kein Leerzeichen gefunden, substring enthält nur den Namen => Zahl wird 1 gesetzt
	zahl = 1.0*vz;
	stochmet.push_back(zahl);
//	reactionpartner[p] = substring.data();
//	sprintf(reactionpartner[p],"%s", subsubstring.data());
	reactionpartner.push_back(substring);
}
p++; //Reactionpartner hochzaehlen
// cout << zahl <<" "<< name << " ";

indexlow = indexhigh+3; // add length of " + "
 } //end while

 if(p != number_reactionpartner){
	 cout << "Error: Parser for kinetic equations found varying number of reaction partners" << endl;
 }
}





/***************************************************************************
FEMLib-Method: 
Task: CKinBlob constructor
Programing:
02/2007 DS Implementation
***************************************************************************/
CKinBlob::CKinBlob(void){

	// default= nonsense values for input, will be checked in CKinCheck()
	d50           =  -1.;
	Sh_factor     =  -1.;
	Re_expo       =  -1.;
	Sc_expo       =  -1.;
	Geometry_expo =  -1.;
	Mass          =  0.;
	Volume        =  0.;
	Masstransfer_k           = 0.;
	current_Interfacial_area = 0.;
	BlobGeoType.clear();
	BlobGeoName.clear();
	Area_Value.clear();
	Interfacial_area.clear();  
}

/***************************************************************************
FEMLib-Method: 
Task: CKinBlob destructor
Programing:
02/2007 DS Implementation
***************************************************************************/
CKinBlob::~CKinBlob(void){

}

/**************************************************************************
FEMLib-Method: 
Task: OBJ read function for CKinBlob-Structure
Programing:
02/2007 DS Implementation
**************************************************************************/
bool CKinBlob::Read(ifstream *rfd_file) {
  
  char line[MAX_ZEILE];
  string line_string, line_str1, s_geo_type, s_geo_name;
  string hash("#"), dollar("$");
  bool new_keyword = false, OK=true; 
  long index, index1;
  double d_inivalue;
  std::stringstream in;

  //========================================================================
  while (!new_keyword) {
	index = rfd_file->tellg();
//    if(!rfd_file->getline(line,MAX_ZEILE)) break;
    if(!GetLineFromFile(line,rfd_file)) break;
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
	  rfd_file->seekg(index); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
      break;
    }
	/* Keywords nacheinander durchsuchen */
    //....................................................................
    if(line_string.find("$NAME")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->name;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$D50")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->d50;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$CALC_SHERWOOD")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->Sh_factor >> this->Re_expo >> this->Sc_expo;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$GEOMETRY")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->Geometry_expo;
	  in.clear();
    }
    if(line_string.find("$INTERFACIAL_AREA")!=string::npos) { // subkeyword found
	  while(OK) {
   	    index1 = rfd_file->tellg();
		if(!GetLineFromFile(line,rfd_file)) break;
		line_str1 = line;
		if((line_str1.find(hash)!=string::npos)||(line_str1.find(dollar)!=string::npos)) {
		  OK = false;
		  rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
		  break;
		}
		in.str(line_str1);
//		in >> s_geo_type >> s_geo_name >> d_inivalue ;
		in >> s_geo_type ;

  	    this->BlobGeoType.push_back(s_geo_type);
		
		if(s_geo_type.compare("DOMAIN")==0) {
		  this->BlobGeoName.push_back("domain");
		  in >> d_inivalue ;
		  this->Area_Value.push_back(d_inivalue);
		}
		else {
          in >> s_geo_name >> d_inivalue;
		  this->BlobGeoName.push_back(s_geo_name);
		  this->Area_Value.push_back(d_inivalue);
		}
		in.clear();
	  }
	}

  } //end while keyword

  return true;
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ read function for CKinBlob-Structure
Programing:
02/2007 DS Implementation
**************************************************************************/
void CKinBlob::Write(ofstream *rfe_file) {

	int j;

	*rfe_file << endl;
	*rfe_file << "#BLOB_PROPERTIES" << endl;
	*rfe_file << "$NAME" << endl << this->name << endl;
	*rfe_file << "$D50" << endl << this->d50 << endl;
	*rfe_file << "$CALC_SHERWOOD" << endl;
	*rfe_file << " Sh_factor : " << this->Sh_factor << endl;
	*rfe_file << " Re_expo   : " << this->Re_expo << endl;
	*rfe_file << " Sc_expo   : " << this->Sc_expo << endl;
	*rfe_file << "$GEOMETRY" << endl << this->Geometry_expo << endl;
	*rfe_file << "$INTERFACIAL_AREA" << endl;
	for(j=0;j<(int)this->BlobGeoName.size();j++){
	*rfe_file << " Geotype : " << this->BlobGeoType[j];
	*rfe_file << "   Geoname : " << this->BlobGeoName[j];
	*rfe_file << "   Value   : " << this->Area_Value[j] << endl;
	}
	*rfe_file << endl;

}

/**************************************************************************
Reaction-Method: 
Task: Reaction data class write function for CKinBlob-Structure
Programing:
02/2007 DS Implementation
**************************************************************************/
void CKinBlob::TestWrite(void) {

	int j;

	cout << endl;
	cout << "#BLOB_PROPERTIES" << endl;
	cout << "$NAME" << endl << this->name << endl;
	cout << "$D50" << endl << this->d50 << endl;
	cout << "$CALC_SHERWOOD" << endl;
	cout << " Sh_factor : " << this->Sh_factor << endl;
	cout << " Re_expo   : " << this->Re_expo << endl;
	cout << " Sc_expo   : " << this->Sc_expo << endl;
	cout << "$GEOMETRY" << endl << this->Geometry_expo << endl;
	cout << "$INTERFACIAL_AREA" << endl;
	for(j=0;j<(int)this->BlobGeoName.size();j++){
	cout << " Geotype : " << this->BlobGeoType[j];
	cout << "   Geoname : " << this->BlobGeoName[j];
	cout << "   Value   : " << this->Area_Value[j] << endl;
	}
	cout << endl;
}

/**************************************************************************
FEMLib-Method: 
Task: OBJ configure function
Programing:
05/2007 DS Implementation
**************************************************************************/
void KBlobConfig(void)
{
  int i, j; 
  long k, l, ll;
  string s_geo_name, s_geo_type;
  vector <long> nodes_vector;

  //Vektor für Interfacial_area erstellen und mit Interfacial_area[0]= Eingabewert belegen
  CKinBlob *m_kb = NULL;
  CFEMesh* m_msh = fem_msh_vector[0]; //SB: ToDo hart gesetzt
  if(m_msh == NULL) {cout << "No mesh in KRConfig" << endl; exit(0);}

  for (i=0; i<(int) KinBlob_vector.size(); i++){
	m_kb = KinBlob_vector[i];
    for(l=0; l< (long)m_msh->nod_vector.size();l++) m_kb->Interfacial_area.push_back( -1. );  //Vorbelegung mit Area=-1

	for(j=0;j<(int)m_kb->BlobGeoName.size();j++){
	  s_geo_name = m_kb->BlobGeoName[j];
	  s_geo_type = m_kb->BlobGeoType[j];

      if(s_geo_type.compare("POINT")==0) {
        CGLPoint* m_geo_point = NULL; // make new GEO point
        m_geo_point = GEOGetPointByName(s_geo_name);//Get GEO point by name
        if(m_geo_point)
	        l = m_msh->GetNODOnPNT(m_geo_point); // + ShiftInNodeVector; // find MSH point number stored in l
		m_kb->Interfacial_area[l] = m_kb->Area_Value[j];
	  } // end if POINT

      if(s_geo_type.compare("POLYLINE")==0) {
		CGLPolyline *m_polyline = NULL;
        m_polyline = GEOGetPLYByName(s_geo_name);// get Polyline by name
        if(m_polyline) {
			if(m_polyline->type==100) //WW
				m_msh->GetNodesOnArc(m_polyline,nodes_vector);
			else
                m_msh->GetNODOnPLY(m_polyline,nodes_vector);
            for(k=0;k<(long)nodes_vector.size();k++){
				ll = nodes_vector[k]; 
                l = ll; //+ShiftInNodeVector;
         		m_kb->Interfacial_area[l] = m_kb->Area_Value[j];
            }
		}
      } // end if POLYLINE 

     if(s_geo_type.compare("SURFACE")==0) {
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(s_geo_name);
        if(m_surface){
           m_msh->GetNODOnSFC(m_surface,nodes_vector);
           for(k=0;k<(long)nodes_vector.size();k++){
				ll = nodes_vector[k]; 
                l = ll; //+ShiftInNodeVector;
         		m_kb->Interfacial_area[l] = m_kb->Area_Value[j];
           }
        }
      }  // end if SURFACE    

     if(s_geo_type.compare("DOMAIN")==0) {
           for(k=0;k<(long)m_msh->nod_vector.size();k++){
         		m_kb->Interfacial_area[k] = m_kb->Area_Value[j];
           }
      }  // end if SURFACE    

	}

  } //end loop over i = Number of blobs

  nodes_vector.clear();

}


/**************************************************************************
FEMLib-Method: 
Task: Check OBJ configuration for input errors
Programing:
05/2007 DS Implementation
**************************************************************************/
void KBlobCheck(void)
{
  int i; 
  long k, l;

  CKinBlob *m_kb = NULL;
  CFEMesh* m_msh = fem_msh_vector[0]; //SB: ToDo hart gesetzt

  cout << endl << "Checking defined blob classes" << endl;
  for (i=0; i<(int) KinBlob_vector.size(); i++){
	m_kb = KinBlob_vector[i];
	cout << m_kb->name << endl;
    
	if (m_kb->d50 <= 0.) {
		cout << "  Warning: D50 <= 0., D50 is set to 1.E-4" << endl;
        m_kb->d50 = 1.E-4;
	}
	if (m_kb->Sh_factor < 0.) {
		cout << "  Warning: Sh_factor < 0., Sh_factor is set to 1.15" << endl;
        m_kb->Sh_factor = 1.15;
	}
	if (m_kb->Re_expo < 0.) {
		cout << "  Warning: Re_expo < 0., Re_expo is set to 0.654" << endl;
        m_kb->Re_expo = 0.654;
	}
	if (m_kb->Sc_expo < 0.) {
		cout << "  Warning: Sc_expo < 0., Sc_expo is set to 0.486" << endl;
        m_kb->Sc_expo = 0.486;
	}
	if (m_kb->Geometry_expo < 0.) {
		cout << "  Warning: Geometry_expo < 0., Geometry_expo is set to 0.66" << endl;
        m_kb->Geometry_expo = 0.66;
	}

	k=0;
	for(l=0; l< (long)m_msh->nod_vector.size();l++) {
		if (m_kb->Interfacial_area[l] < 0.) {
			cout << "  Warning: no value for node " << l << " , Interfacial area is set to zero." << endl;
		    m_kb->Interfacial_area[l] = 0.;
		}
		else {
			k += 1;
		}
	}
	cout << "  Values for interfacial Surfaces have been defined for " << k << " nodes by user." << endl;

  } //end loop i over blob classes

}


/**************************************************************************
Reaction-Method: 
Task: Class constructor
Programing:
02/2006 SB Implementation
**************************************************************************/
CKinReactData::CKinReactData(void){

SolverType=1;
relErrorTolerance=1.0e-10;
minTimestep=1.0e-10;
initialTimestep=1.0e-10;
NumberReactions=0;
NumberFreundlich = 0;
NumberLangmuir = 0;
NumberLinear=0;
NumberMonod=0;
NumberNAPLdissolution=0;
usedt = -1.0;
maxBacteriaCapacity=-1.0;
is_a_bacterium.clear();
testoutput=false; //true;
maxSurfaces=3;
exSurface.clear();
//sp_index.clear();
//kr_active_species = -1;
sp_varind.clear();
sp_pcsind.clear();
//das Surface-Array könnte auch dynamisch allokiert werden
for(int i=0; i< this->maxSurfaces; i++) this->exSurface.push_back(-1.0);
NoReactGeoName.clear();
NoReactGeoType.clear();
is_a_CCBC.clear();
node_foc.clear();
}

/**************************************************************************
Reaction-Method: 
Task: Class destructor
Programing:
02/2006 SB Implementation
**************************************************************************/
CKinReactData::~CKinReactData(void){
}

/**************************************************************************
FEMLib-Method: 
Task: Reaction class data read function
Programing:
02/2006 SB New Class structure, IO, C++, FEM
**************************************************************************/
bool CKinReactData::Read(ifstream *rfd_file)
{
  char line[MAX_ZEILE];
  string sub_line;
  string line_string, line_str1, help("1");
  string delimiter(" ");
  bool new_keyword = false, OK=true;
  string hash("#"), dollar("$");
  std::stringstream in;
  long index, index1;
  int /* count_surf, */ surf_id;
  string s_geo_type, s_geo_name;


  //========================================================================
  while (!new_keyword) {
	index = rfd_file->tellg();
    if(!GetLineFromFile(line,rfd_file)) break;
    line_string = line;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
	  rfd_file->seekg(index); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
      break;
    }
	/* Keywords nacheinander durchsuchen */
    //....................................................................
    if(line_string.find("$SOLVER_TYPE")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->SolverType;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$RELATIVE_ERROR")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->relErrorTolerance;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$MIN_TIMESTEP")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->minTimestep;
	  in.clear();
    }
    //....................................................................
    if(line_string.find("$INITIAL_TIMESTEP")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->initialTimestep;
	  in.clear();
	}
   //....................................................................
    if(line_string.find("$BACTERIACAPACITY")!=string::npos) { // subkeyword found
	  in.str(GetLineFromFile1(rfd_file));
      in >> this->maxBacteriaCapacity;
	  in.clear();
    }
	//....................................................................
	if(line_string.find("$SURFACES")!=string::npos) { // subkeyword found
	  while(OK){
	    index1 = rfd_file->tellg();
		if(!GetLineFromFile(line,rfd_file)) break;
		line_str1 = line;
		if((line_str1.find(hash)!=string::npos)||(line_str1.find(dollar)!=string::npos)) {
		  OK = false;
		  rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
		  break;
		}
		in.str(line_str1);
		in >> surf_id;
		in >> this->exSurface[surf_id];
		in.clear();
		}
/*
	  in.str(GetLineFromFile1(rfd_file));
      in >> count_surf;
	  in.clear();
	  for(int i=0;i<count_surf;i++){
		in.str(GetLineFromFile1(rfd_file));
		in >> surf_id;
		in >> this->exSurface[surf_id];
		in.clear();
	  }
*/
	}
	  //....................................................................
    if(line_string.find("$NO_REACTIONS")!=string::npos) { // subkeyword found
	  while(OK){
	    index1 = rfd_file->tellg();
		if(!GetLineFromFile(line,rfd_file)) break;
		line_str1 = line;
		if((line_str1.find(hash)!=string::npos)||(line_str1.find(dollar)!=string::npos)) {
		  OK = false;
		  rfd_file->seekg(index1); //Dateipointer zurücksetzen, sonst ist das nächste keyword weg
		  break;
		}
		in.str(line_str1);
		in >> s_geo_type >> s_geo_name;
		this->NoReactGeoType.push_back(s_geo_type);
		this->NoReactGeoName.push_back(s_geo_name);
		in.clear();
		}

	}

  }
  this->usedt = DMAX(this->initialTimestep, this->minTimestep);
  return true;
}


/**************************************************************************
Reaction-Method: 
Task: Reaction data class write function
Programing:
02/2006 SB Adapted to new FEM structure
**************************************************************************/
void CKinReactData::Write(ofstream *rfe_file){

	int i;
// Write Keyword
*rfe_file << endl;
*rfe_file << "#KINREACTIONDATA" << endl;
*rfe_file << "$SOLVER_TYPE" << endl << this->SolverType << endl;
*rfe_file << "$RELATIVE_ERROR" << endl << this->relErrorTolerance << endl;
*rfe_file << "$MIN_TIMESTEP" << endl << this->minTimestep << endl;
*rfe_file << "$INITIAL_TIMESTEP" << endl << this->initialTimestep << endl;
*rfe_file << "$BACTERIACAPACITY" << endl << this->maxBacteriaCapacity << endl;
//*rfe_file << " max Surfaces : " << this->maxSurfaces << endl;
*rfe_file << "$SURFACES" << endl ; //<< (int)this->exSurface.size() << endl;
for(i=0;i<(int)this->exSurface.size();i++) *rfe_file << i+1 << "  " << this->exSurface[i] << endl;
*rfe_file << "NO_REACTIONS" << endl; // << (int)this->NoReactGeoName.size() << endl;
for(i=0;i<(int)this->NoReactGeoName.size();i++) *rfe_file << this->NoReactGeoType[i] << "  " << this->NoReactGeoName[i] << endl;
//*rfe_file << " Number of reactions: " << this->NumberReactions << endl; 
//*rfe_file << " Number of linear exchange reactions: " << this->NumberLinear << endl;
//*rfe_file << " Number of freundlich exchange reactions: " << this->NumberFreundlich << endl;
//*rfe_file << " Number of langmuir exchange reactions: " << this->NumberLangmuir << endl;
//*rfe_file << " is_a_bacterium: "  << endl;
//for(i=0;i<this->is_a_bacterium.size();i++) *rfe_file << this->is_a_bacterium[i] << " ";
//*rfe_file << endl;
*rfe_file << endl;
/*
*rfe_file << " usedt "<< this->usedt << endl;
*rfe_file << " Number Reactions "<< this->NumberReactions << endl;
*rfe_file << " this->is_a_bacterium " << endl;
for(int i=0; i < (int)this->is_a_bacterium.size();i++)
*rfe_file <<  this->is_a_bacterium[i];
*rfe_file << endl;
*/
}


/**************************************************************************
Reaction-Method: 
Task: Reaction data class write function
Programing:
02/2006 SB Adapted to new FEM structure
**************************************************************************/
void CKinReactData::TestWrite(void){
 int i;//CB
// Write Keyword
cout << "#KINREACTIONDATA" << endl;
cout << "$SOLVER_TYPE" << endl << this->SolverType << endl;
cout << "$RELATIVE_ERROR" << endl << this->relErrorTolerance << endl;
cout << "$MIN_TIMESTEP" << endl << this->minTimestep << endl;
cout << "$INITIAL_TIMESTEP" << endl << this->initialTimestep << endl;
cout << "$BACTERIACAPACITY" << endl << this->maxBacteriaCapacity << endl;
cout << endl;
cout << " usedt "<< this->usedt << endl;
cout << " Number Reactions "<< this->NumberReactions << endl;
cout << " this->is_a_bacterium " << (int)this->is_a_bacterium.size() << endl;
for(i=0; i < (int)this->is_a_bacterium.size();i++)
cout <<  this->is_a_bacterium[i] << " ";
cout << endl;
cout << " Exchange reactions : " << endl;
//ACHTUNG: die folgende Zeile wird ausgegeben BEVOR this->NumberXXX berechnet wurde !
cout << " Linear exchange : " << this->NumberLinear << endl << " Freundlich exchange : " << this->NumberFreundlich << endl << " Langmuir exchange : " << this->NumberLangmuir << endl << " NAPL dissolution : " << this->NumberNAPLdissolution << endl;
for(i=0;i<(int)this->exSurface.size();i++) cout << " " << this->exSurface[i];
cout << endl;
}

/**************************************************************************
Reaction-Method: 
Task: KinReaction write function - echo of input values to rfe - file
Programing:
05/2004 SB Implementation
02/2006 SB C++, IO
**************************************************************************/
bool KRWrite(string prot_name){
	
  CKinReact *m_kr = NULL;
  CKinReactData *m_krd = NULL;
  CKinBlob *m_kp = NULL;
  string rfe_file_name;
  int i, length;

  //========================================================================
  // File handling
  rfe_file_name = prot_name+"_echo";
  ofstream rfe_file (rfe_file_name.data(),ios::app);
  if (!rfe_file.good()) return false;
  rfe_file.seekp(0L,ios::end); // go to end
  //========================================================================
  rfe_file << endl << "; Reactions ----------------------------------------------------------------- " << endl;
  // Output all Reactions
  length = (int)KinReact_vector.size();
  for(i=0;i<length;i++){
	m_kr = KinReact_vector[i];
	m_kr->Write(&rfe_file);
	if(KinReactData_vector[0]->testoutput)	
		m_kr->TestWrite();
  }
  // Output all BlobProperties
  length = (int)KinBlob_vector.size();
  for(i=0;i<length;i++){
	m_kp = KinBlob_vector[i];
	m_kp->Write(&rfe_file);
	m_kp->TestWrite();
  }
  // Output all kinetic reaction data
  if(KinReactData_vector.size() > 0)
	m_krd = KinReactData_vector[0];
  if(m_krd != NULL){
	m_krd->Write(&rfe_file);
	if(KinReactData_vector[0]->testoutput) 
		m_krd->TestWrite();
  }
  rfe_file.close();

  return true;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ExecuteKinReact()                                 */
/* DS-TBC                                                                 */
/*                                                                        */
/* Aufgabe:                                                               */
/* Haupt-Subroutine zur Berechnung der mikrobiellen kinetischen Reaktionen*/
/* Aufruf in void ExecuteReactions(void)  (rf_react.cpp)                  */
/*                                                                        */
/*                                                                        */
/* Programmaenderungen:                                                   */
/* 05/2003     DS         Erste Version                                   */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/*                                                                        */
/**************************************************************************/

void CKinReactData::ExecuteKinReact(void){

double hmin, eps, usedtneu=0., usedttmp=1.E+30;
long node, save_node=0, nnodes;
int nok=0, nbad=0, save_nok=0, save_nbad=0;

CFEMesh* m_msh = fem_msh_vector[0]; //SB: ToDo hart gesetzt
CTimeDiscretization *m_tim = NULL;

cout << " ExecuteKineticReactions" << endl;

nnodes = (long) m_msh->nod_vector.size();

if(time_vector.size()>0){
    m_tim = time_vector[0];
    dt = m_tim->CalcTimeStep();
}
if ((dt>1.E-20)&&(aktueller_zeitschritt>0)){
    /* Einstellungen Gleichungslöser für alle Knoten gleich */
    /* relative Genauigkeit des Gleichungslösers (eps< fehler/c0) */
	eps=this->relErrorTolerance;
    /* min zulaessiger Zeitschritt*/
	hmin=this->minTimestep;

//	cout << " NumberReactions: " << this->NumberReactions << endl;
	if(this->NumberReactions > 0) 
    for(node = 0; node < nnodes; node++){ 

		// no reactions at Concentration BCs
        if(this->is_a_CCBC[node] == true){
		}
		else{
			this->Biodegradation(node, eps, hmin, &usedtneu, &nok, &nbad);
			if (usedtneu<usedttmp) {
				usedttmp=usedtneu;
				save_nok=nok;
				save_nbad=nbad;
				save_node=node;
			}	
		}
    } // end for(node...

    if (usedttmp<usedt) {
        cout << endl << "Kinetics in node " << save_node << " limit integration step - nok: ";
	    cout << save_nok << " nbad: " << save_nbad << endl;
    }

    // update des zulässigen Integrationsschritts, verwendet beim Aufruf von odeint
    // kleinster Wert, der in einem der Knoten zu einer zuverlässigen Integration geführt hat
    // konservative, aber stabile Annahme
    usedttmp=DMAX(usedttmp,hmin);
    usedt=   DMIN(usedttmp, dt);
//    cout << endl << " Next suggested integration step " << usedt << endl;
} // end if(dt>...
}

/**************************************************************************/
/* ROCKFLOW - Funktion: Biodegradation(node, )                            */
/* DS-TBC                                                                 */
/*                                                                        */
/* Aufgabe:                                                               */
/* Berechnet Bioabbau im Knoten node in dt                                */
/* Trägt berechnete Senke in ?? ein                                       */
/*                                                                        */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)                 */
/* E: node (Knotennummer), tstart (Zeitpunkt Rechenbeginn)                */
/*    tend (Zeitpunkt Rechenende)                                         */
/*                                                                        */
/* Ergebnis:                                                              */
/* - void -                                                               */
/* Ergebnisse werden in Datenstruktur zurückgeschrieben                   */
/*                                                                        */
/* Programmaenderungen:                                                   */
/* 05/2003     DS         Erste Version                                   */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/* 05/2007     DS         NAPL-dissolution added                          */
/*                                                                        */
/**************************************************************************/

void CKinReactData::Biodegradation( long node, double eps, double hmin, double *usedtneu, int *nok, int *nbad)
{

  double *Concentration;
  double *newVolume;
  double nexth=0.;
  long sp,  timelevel;
//  int nok=0, nbad=0, Number_of_Components;
  int Number_of_Components, nreactions, r, Sp1, blob, Number_of_blobs;
  double Csat_max, DensityNAPL, DensityAQ, DiffusionAQ, ViscosityAQ, PoreVelocity, d50, Reynolds, Schmidt, Sherwood;
  double tstart, tend, dt=0.0;
  double baditerations;
//  CRFProcess* m_pcs = NULL;
  CTimeDiscretization *m_tim = NULL;
//  CompProperties *m_cp = NULL;
  string speciesname = " dummy";

  CKinReact *m_kr = NULL;
  CKinBlob *m_kb = NULL;
  CKinReactData *m_krd = NULL;
  m_krd = KinReactData_vector[0];

timelevel = 1;  // concentrations are in new timelevel
if(time_vector.size()>0){
    m_tim = time_vector[0];
    dt = m_tim->CalcTimeStep();
}
//Number_of_Components = this->kr_active_species; //
Number_of_Components = (int)cp_vec.size();
// Get storage for vector of concentrations
Concentration = dvector(1,Number_of_Components);

/* Konzentrationen aller Substanzen aus Datenstruktur auslesen und in neuem Array speichern */
for(sp=0;sp<Number_of_Components;sp++){
	Concentration[sp+1] = pcs_vector[this->sp_pcsind[sp]]->GetNodeValue(node,this->sp_varind[sp]); 
	if(fabs(Concentration[sp+1]) < 1.e-19) Concentration[sp+1] = 0.0;

	//SB todo - ist abewr gerade eh noch ein dummy
    //ExchangeTerm[sp]=TBCGetExchange(node,sp)/(dt);
}


//#ds
/* PREPARE PARAMETERS FOR NAPL-DISSOLUION*/
/* calculate Mass and Volume of blobs for this node */

//set all mass(blob)=0, volume(blob)=0 
  Number_of_blobs = (int) KinBlob_vector.size();
  for (r=0; r<Number_of_blobs ; r++){
	m_kb = KinBlob_vector[r];
	m_kb->Mass   = 0.;
	m_kb->Volume = 0.;
  }

  nreactions= m_krd->NumberReactions; 
  for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];
	if(m_kr->typeflag_napldissolution){             //dissolution reaction identified
		Sp1			= m_kr->ex_species[0]+1;        //Sp1 = NAPL-species
		blob		= m_kr->blob_ID; 
		DensityNAPL = m_kr->Density_NAPL;

		m_kb = KinBlob_vector[blob];                // pointer to blob-properties set in the reaction r
		
		if(Concentration[Sp1] > 0.){
		m_kb->Mass   += DMAX(Concentration[Sp1],0.);
		m_kb->Volume += DMAX(Concentration[Sp1],0.) / DensityNAPL;
		//Sb todo Achtung - das wird ja gar nicht zurückgespeichert...
		}
	}
  } // end for nreactions

/* calculate current Csat depending on Raoults law for this node */
  for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];
	if(m_kr->typeflag_napldissolution){             //dissolution reaction identified
		Sp1			= m_kr->ex_species[0]+1;        //Sp1 = NAPL-species
		blob		= m_kr->blob_ID; 
	    Csat_max    = m_kr->Csat_pure;

		m_kb = KinBlob_vector[blob];                // pointer to blob-properties set in the reaction r
		
		if (m_kb->Mass > 0.) {
		  m_kr->current_Csat = Csat_max * DMAX(Concentration[Sp1],0.) / m_kb->Mass;   
		}
		else {
		  m_kr->current_Csat = Csat_max;      // keine NAPL-Masse vorhanden, NAPL-Bildung möglich wenn c(singleSubstance) > Csat
		}
	}
  } // end for nreactions

/* calculate current Masstransfer-coefficient k for this node */
 for (r=0; r<Number_of_blobs ; r++){
	m_kb = KinBlob_vector[r];

	d50          = m_kb->d50;
    DiffusionAQ  = mfp_vector[0]->diffusion;
	DensityAQ    = mfp_vector[0]->Density();
	ViscosityAQ  = mfp_vector[0]->Viscosity();
//#ds TODO
    PoreVelocity = 1.1574E-5;    //nach Q in *.st-file, A und ne => u= 1m/d = 1.15E-5 m/s

	Reynolds  = DensityAQ * PoreVelocity * d50 / ViscosityAQ ;
	Schmidt   = ViscosityAQ / DiffusionAQ / DensityAQ ;
    Sherwood   = m_kb->Sh_factor * pow(Reynolds, m_kb->Re_expo) * pow(Schmidt, m_kb->Sc_expo);

	m_kb->Masstransfer_k = Sherwood * DiffusionAQ / d50;      //k in m/s

  }
  
/* save current Interfacial areas for this node */
 for (r=0; r<Number_of_blobs ; r++){
	m_kb = KinBlob_vector[r];
	m_kb->current_Interfacial_area = m_kb->Interfacial_area[node];
  }


  tstart=DMAX(aktuelle_zeit-dt,0.);
  tend=aktuelle_zeit;
//  tstart=tstart/86400.0; tend = tend/86400.0 ;  // alte Version: hier wurde nur im kinetischen Teil mit Tagen gerechnet
//  cout << " times: " << tstart << ", " << tend << endl;

  /* Aufruf Gleichungslöser */
  /* eigentliche Rechenroutinen sind "derivs" und "jacobn" (namen fest vorgegeben),
   die vom Gleichungslöser aufgerufen werden */
  odeint(Concentration, Number_of_Components,tstart,tend,eps,usedt,hmin,&nexth,nok,nbad,derivs,stifbs,node);

  baditerations=double(*nbad)/double(*nok + *nbad);

  if (baditerations<0.001) {
// fehlerfreie Integration, zeitschritt kann vergrößert werden
    	  if (nexth>usedt) *usedtneu=DMAX(nexth,usedt*2.);
	      else             *usedtneu=usedt*1.5;
  }
  else {
// Integrationsfehler, zeitschritt beibehalten oder verkleinern
	      if      (*nbad==1)     *usedtneu=DMAX(nexth,usedt*1.10);
		  else if (*nok>*nbad*2) *usedtneu=DMAX(nexth,usedt*1.01);
	      else                   *usedtneu=DMAX(nexth,usedt/5.);	  
  }
  
  /* Konzentrationen aller Substanzen in Datenstruktur zurückschreiben */
  for(sp=0;sp<Number_of_Components;sp++){
	//Notlösung gegen das vollständige Absterben der Bakterien
	if ((this->is_a_bacterium[sp]) && (Concentration[sp+1]<1.E-30)) Concentration[sp+1]=1.E-30;
	pcs_vector[this->sp_pcsind[sp]]->SetNodeValue(node,this->sp_varind[sp],Concentration[sp+1]); 

	// save exchange term SB todo
  }

/* #ds calculate Interfacial areas for this node after dissolution for next time step */
  newVolume = dvector(0,Number_of_blobs);

  for (r=0; r<Number_of_blobs; r++){
	newVolume[r] = 0.;
  }

  nreactions= m_krd->NumberReactions; 
  for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];
	if(m_kr->typeflag_napldissolution){             //dissolution reaction identified
		Sp1			= m_kr->ex_species[0]+1;        //Sp1 = NAPL-species
		blob		= m_kr->blob_ID; 
		DensityNAPL = m_kr->Density_NAPL;

		newVolume[blob] += DMAX(Concentration[Sp1],0.) / DensityNAPL;
	}
  } // end for nreactions
//  double dummy;
  for (r=0; r<Number_of_blobs; r++){
	m_kb = KinBlob_vector[r];
	if ((newVolume[r] > 0.) && (m_kb->Volume >0.)) {
	  m_kb->Interfacial_area[node] = m_kb->current_Interfacial_area * pow( (newVolume[r] / m_kb->Volume), m_kb->Geometry_expo) ;
	}
	else {
		m_kb->Interfacial_area[node] = 1.E-20;                  //residual interfacial area to allow re-building of phase
	}
  }


  free_dvector(newVolume,0,Number_of_blobs);
  free_dvector(Concentration,1,Number_of_Components);

}

/*************************************************************************************/
/* Routine für Bulirsch-Stoer Gleichungslöser (steife ODEs)                          */
/* DS-TBC                                                                            */
/*                                                                                   */
/* Berechnung der ersten Ableitungen über die Zeit dC/dt=...                         */
/*                                                                                   */
/* Input:                                                                            */
/* t = aktuelle Zeit                                                                 */
/* c[1..Number_of_Components] = aktuelle Konzentration der Substanzen in Zelle       */
/* n =! Number_of_Components                                                         */
/*                                                                                   */
/* Output:                                                                           */
/* dcdt[1..Number_of_Components] = aktuelle Ableitung der Konzentrationen nach der   */
/*            Zeit, = Wachstum der Bakterien und Verbrauch der Substanzen            */
/*                                                                                   */
/* Programmaenderungen:                                                              */
/* 05/2003     DS         Erste Version                                              */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/* 05/2007     DS         NAPL-dissolution added                                     */
/*                                                                                   */
/*************************************************************************************/

void derivs(double t, double c[], double dcdt[], int n, long node)
{
	int i, r, nreactions, BacteriaNumber;
	int  Sp1, Sp2, surfaceID=-1, blob;
	double BacteriaMass, BacGrowth, Yield, sumX=0., maxkap;
	double porosity1, porosity2, exchange, exch, kd, density1, saturation2, kadsorb, kdesorb, totalSurface,
		   exponent, parameter, chochexp;
	double dt;

//#ds
//	int blob;
	double Csat;
//	double occupiedSurface[m_krd->maxSurfaces+1];
	vector <double> occupiedSurface;

    CKinReact *m_kr = NULL;
    CKinBlob *m_kb = NULL;
    CKinReactData *m_krd = NULL;
    m_krd = KinReactData_vector[0];
	double foc;

    CTimeDiscretization *m_tim = NULL;
    m_tim = time_vector[0];
    dt = m_tim->CalcTimeStep();



/* reset array with derivatives */
/* ACHTUNG, unterschiedliche Indizierung der Arrays, c[1..n] BioDegradation[0..n-1] */
	for (i=0; i<n; i++) {
//SBtodo		dcdt[i+1]=ExchangeTerm[i];
		dcdt[i+1] = 0.0;
	}


/* calculate present bacteria capacity */
	sumX = 0.0; //SB added
	maxkap=m_krd->maxBacteriaCapacity; 

	if (maxkap>1.E-30){
		for (i=0; i<n; i++){
			if (m_krd->is_a_bacterium[i]){
				sumX = sumX + c[i+1];
			}
		}
	}

/**********************************************************************************************/
/* Anzahl der mikrobiellen Reaktionen aus Datenstruktur auslesen */
	nreactions= m_krd->NumberReactions; //BioDegradation.NumberReactions; 
	
/* loop over reactions dX/dt= nymax * X * monodterms * inhibitionterms */
	for (r=0; r<nreactions; r++){
      m_kr = KinReact_vector[r];
	  if(m_kr->typeflag_monod){
		BacteriaNumber   = m_kr->bacteria_number+1;  
		BacteriaMass     = c[BacteriaNumber];
		porosity1	= m_kr->GetPorosity(BacteriaNumber-1,node);
		
		if (BacteriaMass > 1.E-40){
			BacGrowth = m_kr->BacteriaGrowth (r, c, sumX, -1);
			if (m_kr->grow){
				dcdt[BacteriaNumber] += BacGrowth;
			}
			/* microbial consumption of substances */
			for (i=0; i<n; i++){
				Yield =	m_kr->ProductionStoch[i]; 
				if (fabs(Yield) > 1.E-30){
					porosity2	= m_kr->GetPorosity(i,node); 
					dcdt[i+1] += BacGrowth * Yield * porosity1 / porosity2;
				}
			}
		}
		}// type == monod
	}


/**********************************************************************************************/
/* Berechnung der Austauschprozesse */

	// calculate already occupied surface 
	if(m_krd->NumberLangmuir > 0){
		// Initialise Surfaces for langmuir isotherms
		for (i=0; i<m_krd->maxSurfaces; i++)	occupiedSurface.push_back(0.0);

		for (r=0; r<nreactions; r++){
			m_kr = KinReact_vector[r];
			if((m_kr->typeflag_exchange)&&(m_kr->typeflag_exchange_langmuir)){
				Sp1			= m_kr->ex_species[0]+1; 
				surfaceID	= m_kr->exSurfaceID; 
				occupiedSurface[surfaceID] += c[Sp1];	
			}
		}

	} // if NumberLangmuir > 0

	for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];
	if(m_kr->typeflag_exchange){	
/* linearer Austausch ggf. mit kd */
		if(m_kr->typeflag_exchange_linear){
//Matrix
		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node);
		density1	= m_kr->GetDensity(Sp1-1,node); 
//geloest
		Sp2			= m_kr->ex_species[1]+1; 
		porosity2	= m_kr->GetPorosity(Sp2-1,node);

		exch		= m_kr->ex_param[0]; 
		kd			= m_kr->ex_param[1]; 

		if (fabs(kd)<MKleinsteZahl){
//no kd, exchange between two species in solution
			exchange = exch * ( c[Sp2] - c[Sp1] );
		    dcdt[Sp1] +=  exchange / porosity1;
		    dcdt[Sp2] += -exchange / porosity2;
		}
		else {
// with kd, exchange between matrix (mol/kg) and solution (mol/l)
  		    foc = m_krd->node_foc[node];
		    if(foc > MKleinsteZahl)	kd = kd*foc;
			exchange = exch * ( c[Sp2]*kd - c[Sp1] );
/* Die Abfrage verringert die Desorptionsgeschwindigkeit, wenn absehbar ist, dass Csorbiert im Negativen landet */
		    if (-exchange*dt > c[Sp1] )  exchange = -c[Sp1] / dt;

		    dcdt[Sp1] +=  exchange;
		    dcdt[Sp2] += -exchange * porosity1 / porosity2 *density1;
		}
	
	    } // ende if exType == linear

/* Freundlich Kinetik */
		if(m_kr->typeflag_exchange_freundlich){
//Matrix
		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node); 
		density1	= m_kr->GetDensity(Sp1-1,node); 
//geloest
		Sp2			= m_kr->ex_species[1]+1; 
		porosity2	= m_kr->GetPorosity(Sp2-1,node); 

		exponent    = m_kr->ex_param[2]; 
		parameter   = m_kr->ex_param[1]; 
		exch		= m_kr->ex_param[0]; 

		if (c[Sp2] > residual){
			// no linearisation required 
			chochexp = pow(c[Sp2],exponent);
		}
		else{
			// linearisation required due to instability of c^x if c<residual 
			chochexp = (1.-exponent)*pow(residual,exponent)+exponent*pow(residual,(exponent-1))*c[Sp2];
		}
		
		exchange = exch * ( parameter*chochexp - c[Sp1] );
/* Die Abfrage verringert die Desorptionsgeschwindigkeit, wenn absehbar ist, dass Csorbiert im Negativen landet */
		if (-exchange*dt > c[Sp1] )  exchange = -c[Sp1] / dt;

		dcdt[Sp1] += exchange;
		dcdt[Sp2] += -exchange * porosity1 / porosity2 *density1;
	    } // if freundlich

	
/* Langmuir Kinetik */
		if(m_kr->typeflag_exchange_langmuir){

// Surfaces were initialized above
//	for (i=0; i<nexchange; i++)	{
		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node);

		Sp2			= m_kr->ex_species[1]+1;
		porosity2	= m_kr->GetPorosity(Sp2-1,node);

		kadsorb		= m_kr->ex_param[0];
		kdesorb		= m_kr->ex_param[1];
//SB_langmuir		surfaceID	= m_kr->exSurfaceID; //Exchange.Langmuir[i].SurfaceID;
		totalSurface= m_krd->exSurface[m_kr->exSurfaceID]; 

//      occupiedSurface was calculated above
//		double occsurf = occupiedSurface[surfaceID];

//#ds ACHTUNG hier muss sicher gestellt sein, dass Sp1 die adsorbierte und Sp2 die gelöste Species ist !
		exchange = kadsorb*(totalSurface-occupiedSurface[surfaceID])* c[Sp2] - kdesorb* c[Sp1];
 
/* Die Abfrage verringert die Desorptionsgeschwindigkeit, wenn absehbar ist, dass Csorbiert im Negativen landet */
		if (-exchange*dt > c[Sp1] )  exchange = -c[Sp1] / dt;

		dcdt[Sp1] += exchange;
		dcdt[Sp2] += -exchange * porosity1 / porosity2;
//	}

	} // ende if exType == langmuir	

}//if type == exchange

} // for r





//#ds
/**********************************************************************************************/
/* Berechnung NAPL-Lösung */
/**********************************************************************************************/

for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];

	if(m_kr->typeflag_napldissolution){
		Sp1			= m_kr->ex_species[0]+1; //Exchange.Linear[i].Species1;    Sp1 muss NAPL sein
//		porosity1	= m_kr->GetPorosity(Sp1-1,node);
//		density1	= m_kr->GetDensity(Sp1-1,node); //GetDensity(phase);
		blob		= m_kr->blob_ID; 
		m_kb = KinBlob_vector[blob];                // pointer to blob-properties set in the reaction r

		Sp2			= m_kr->ex_species[1]+1; //Exchange.Linear[i].Species2;    Sp2 = mobile Phase
		porosity2	= m_kr->GetPorosity(Sp2-1,node);
		//#ds TODO	    saturation2 = ??
		saturation2 = 1.;

	    Csat        = m_kr->current_Csat;                                      // Csat externally calculated in Function Biodegradation
		exch		= m_kb->Masstransfer_k * m_kb->current_Interfacial_area;    // k * A externally calculated in Function Biodegradation

		exchange = exch * ( Csat - c[Sp2] );

/* Die Abfrage verringert die Lösungsgeschwindigkeit, wenn absehbar ist, dass CNAPL im Negativen landet
   Verhindert negative CNAPL-Konzentrationen */
		if (exchange*dt > c[Sp1] )  exchange = c[Sp1] / dt;

		if ((exchange < 0.) || (c[Sp1] > MKleinsteZahl)) {                     // no dissolution or NAPL mass present
			dcdt[Sp1] += -exchange;                                //NAPL
			dcdt[Sp2] +=  exchange / porosity2 / saturation2;      //concentration in solution, refers to mass / volume of water
		}

	} // ifNAPL dissolution
} // loop ofer reactions r
//#ds




occupiedSurface.clear();

}



/**************************************************************************/
/* Phase bestimmen, in der sich Substanz i befindet                       */
/* DS-TBC                                                                 */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/**************************************************************************/
int CKinReact::GetPhase( int species ){
int phase = -1;
CompProperties *cp_m = NULL;

cp_m = cp_vec[species];
if(cp_m != NULL)
	phase = cp_m->transport_phase;
else
	cout << " Error: component does not exist !" << endl;
return phase;
}


/**************************************************************************/
/* Porosität einer Phase bestimmen                                        */
/* DS-TBC                                                                 */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/**************************************************************************/

double   CKinReact::GetPorosity( int comp, long index ){

double poro = 0.0, theta = 1.0;
long group, phase;
CMediumProperties *m_mat_mp = NULL;
CRFProcess *m_pcs=NULL;
CKinReactData *m_krd = NULL;

m_krd = KinReactData_vector[0];

// Get process
m_pcs = PCSGet("MASS_TRANSPORT",cp_vec[comp]->compname); //SB todo check
m_pcs = pcs_vector[m_krd->sp_pcsind[comp]];
theta = m_pcs->m_num->ls_theta;
phase = cp_vec[comp]->transport_phase;

// Get material properties of element
group = 0; //SB todo m_pcs->m_msh->ele_vector[index]->GetPatchIndex();
m_mat_mp = mmp_vector[group];
if(phase ==0){ // water phase
	poro = m_mat_mp->Porosity(index, theta);
}
else if(phase == 1){ // solid phase
	poro = m_mat_mp->vol_mat;
}
else if (phase == 2){ // bio phase
	poro = m_mat_mp->vol_bio;
}
else if (phase == 3){ // NAPL phase (refers to REV)
	poro = 1.;
}
else
	cout << " Error: No porosity found for phase " << phase << endl;

// cout << " Get Porosity returns: vol_water: " << m_mat_mp->Porosity(index, NULL, theta) << ", vol_bio: " << m_mat_mp->vol_bio <<", vol_mat: " << m_mat_mp->vol_mat << ", phase: " << phase << ", poro: " << poro << endl;
return poro;
}

/**************************************************************************/
/* Dichte einer Phase bestimmen                                        */
/* DS-TBC                                                                 */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/**************************************************************************/

double CKinReact::GetDensity( int comp, long index ){

double dens=0.;
long group, phase;
CRFProcess *m_pcs=NULL;

group = index; // avoid warning
// Get process
m_pcs = PCSGet("MASS_TRANSPORT",cp_vec[comp]->compname);
//theta = m_pcs->m_num->ls_theta;
phase = cp_vec[comp]->transport_phase;

// Get material properties of element
group = 0; //m_pcs->m_msh->ele_vector[index]->GetPatchIndex(); //SB todo

if(phase == 0){ // component in water phase, return liquid density
	dens = mfp_vector[0]->Density(); 
}
else if(phase == 1){ // component in solid phase, return solid phase density
	dens = msp_vector[group]->Density();
}

return dens;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: BacteriaGrowth(reaction)                          */
/* DS-TBC                                                                 */
/*                                                                        */
/* Aufgabe:                                                               */
/* Berechnet mikrobielles Wachstum aufgrund Reaktion reaction             */
/*                                                                        */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)                 */
/* E: reaction - Nummer der Wachstumsreaktion                             */
/*    Concentration - Array aktuelle Konzentrationen aus Gleichungslöser  */
/*    sumX - Bakteriendichte für Berücksichtigung MaxKapazität            */
/*                                                                        */
/* Ergebnis:                                                              */
/* R: Growth - Wachstum der zur Reaktion gehoerigen Bakteriengruppe       */
/*                                                                        */
/* Programmaenderungen:                                                   */
/* 05/2003     DS         Erste Version                                   */
/*                                                                        */
/**************************************************************************/

double CKinReact::BacteriaGrowth ( int r, double *c, double sumX, int exclude)
{
	int i, BacteriaNumber, MonodSpecies, InhibitionSpecies, NumberMonod, NumberInhibition;
	double Growth, BacteriaMass, maxVelocity, maxkap;
	double MonodConcentration, InhibitionConcentration, C;
    CKinReactData *m_krd = NULL;

    m_krd = KinReactData_vector[0];

	BacteriaNumber   = this->bacteria_number+1;
	maxVelocity      = this->rateconstant; 
	BacteriaMass     = c[BacteriaNumber];

	Growth = maxVelocity * BacteriaMass;

	/* Hemmung durch Bakteriendichte sumX nur bei Wachstum */
	if ((sumX>1.E-30)&&(maxVelocity>0.)){
		/* Max. Bakterienkapazität aus Datenstruktur auslesen */
		maxkap = m_krd->maxBacteriaCapacity; 
		Growth = Growth * maxkap/(sumX+maxkap);
	}

/*  FOR-Schleife über vorhandene Monodterme */

	NumberMonod      = this->number_monod; 
    for (i=0; i<NumberMonod; i++) {
		/* Möglichkeit zum Weglassen Monodterm für partielle Ableitungen erforderlich */
		if (i != exclude){
			MonodSpecies       = this->monod[i]->speciesnumber; 
			MonodConcentration = this->monod[i]->concentration;
			C                  = c[MonodSpecies+1];
			Growth= Growth * this->Monod(MonodConcentration,C);
		}
	}

/*	FOR-Schleife über vorhandene Inhibitionsterme */
	NumberInhibition = this->number_inhibit; 
    for (i=0; i<NumberInhibition; i++) {
		InhibitionSpecies       = this->inhibit[i]->speciesnumber; 
		InhibitionConcentration = this->inhibit[i]->concentration; 
		C                       = c[InhibitionSpecies+1];
		Growth= Growth * this->Inhibition(InhibitionConcentration,C);
	}

return Growth;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: Monod(MC, C)                                      */
/* DS-TBC                                                                 */
/*                                                                        */
/* Aufgabe:                                                               */
/* Berechnet modifizierten Monod-Term                                     */
/*                                                                        */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)                 */
/* E: MC - MonodConcentration                                             */
/*    C  - Concentration of substance                                     */
/*                                                                        */
/* Ergebnis:                                                              */
/* A: Monod - Wert des Monodterms                                         */
/*                                                                        */
/* Programmaenderungen:                                                   */
/* 05/2003     DS         Erste Version                                   */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/*                                                                        */
/**************************************************************************/

double CKinReact::Monod ( double MC, double C )
{
double Monod;
//CKinReactData *m_krd = KinReactData_vector[0];
if (C>0.){
	/* normaler Monodterm */
	Monod=C/(MC+C);
}
else
   /* linearisierter Term fuer c<=0, with very small slope due to high concentrations */
   	  Monod=(C/1000.)/MC;    //Monod=(C)/MC;   CB changed on behalf of SB, 10.07.07 

return (Monod);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: Inhibition(IC, C)
   DS-TBC
                                                                          */
/* Aufgabe:
   Berechnet modifizierten Inhibitions-Term
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: IC - InhibitionConcentration
      C  - Concentration of substance
                                                                          */
/* Ergebnis:
   A: Inhibition - Wert des Inhibitionsterms
                                                                          */
/* Programmaenderungen:
   05/2003     DS         Erste Version
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/*                                                                          */
/**************************************************************************/

double CKinReact::Inhibition ( double IC, double C )
{
  double Inhibition;
//  CKinReactData *m_krd = KinReactData_vector[0];

  if (C>0.){
	/* normaler Inhibitionsterm */
	  Inhibition=IC/(IC+C);
  }
  else{
	/* linearisierter Term fuer C<=0 */
	Inhibition=1.0;   //Inhibition=1.-C/IC;   CB changed due to stimulance of growth for neg conc.
  }

  return (Inhibition);
}



/*************************************************************************************/
/* Routine für Bulirsch-Stoer Gleichungslöser (steife ODEs)                          */
/* DS-TBC                                                                            */
/*                                                                                   */
/* Berechnung der Jacobi-Matrix, partielle Ableitungen von dC/dt aus Funktion        */
/* derive nach                                                                       */
/* a) der Zeit d2C/d2t = dfdt[] = 0. für mikrobiellen Abbau                          */
/* b) den Konzentrationen d2C/dt*dC = dfdc[][]                                       */
/*                                                                                   */
/* Input:                                                                            */
/* t = aktuelle Zeit                                                                 */
/* c[1..Number_of_Components] = aktuelle Konzentration der Substanzen in Zelle       */
/* n =! Number_of_Components                                                         */
/*                                                                                   */
/* Output:                                                                           */
/* dfdt[1..Number_of_Components] = aktuelle Ableitung der Funktion nach der Zeit     */
/* dfdc[1..Number_of_Components][1..Number_of_Components] = aktuelle Ableitung       */
/*             der Funktion nach den Konzentrationen                                 */
/*                                                                                   */
/* Programmaenderungen:                                                              */
/* 05/2003     DS         Erste Version                                              */
/* 02/2006     SB         Introduced new C++ concept, Data structures     */
/*                                                                                   */
/*************************************************************************************/

void jacobn(double t, double c[], double dfdt[], double **dfdc, int n, long node)
{
	int i, j, r, nreactions, BacteriaNumber, NumberMonod, MonodSpecies, NumberInhibition, 
		InhibitionSpecies;
	int  Sp1, Sp2, SpX, surfaceID=-1, surfaceID2, blob;
	double maxkap, BacteriaMass, sumX=0., BacGrowth, maxVelocity, *d2X_dtdS,
		CMonodSpecies, MonodConcentration, CInhibitionSpecies, InhibitionConcentration,
		Yield;
	double porosity1, porosity2, exch, kd, density1, saturation2, kadsorb, kdesorb, totalSurface, adsorb, 
		exponent, parameter;
//SBtodo	double occupiedSurface[maxSurfaces+1];
	vector <double> occupiedSurface;

	CKinReact *m_kr = NULL, *m_kr1=NULL;
    CKinBlob *m_kb = NULL;
    CKinReactData *m_krd = NULL;
//	CMediumProperties *m_mat_mp = NULL;
	double foc;

    m_krd = KinReactData_vector[0];
	
/* Hilfsvektor für partielle Ableitung des Bakterienwachstums nach Species S */
    d2X_dtdS = dvector(1,n);

/* weitere Ableitungen nach t dfdt[] alle null */
/* Ableitungen nach c dfdc[][] werden inkrementiv berechnet, also erst alles null setzen */
/* ACHTUNG, unterschiedliche Indizierung der Arrays, c[1..n] BioDegradation[0..n-1] */
	for (i=0; i<n;i++) 
	{
		dfdt[i+1]=0.;
		for (j=0; j<n;j++) dfdc[i+1][j+1]=0.;
	}
    
/* calculate present bacteria capacity */
	maxkap=m_krd->maxBacteriaCapacity; 

	if (maxkap>1.E-30)
	{
/* Komplette Berechnung der Ableitungen für den Fall dass eine maximale Kapazität berücksichtigt werden muss
   Muss sein, weil Ableitungen höhere Ordnung haben (Bakterienmasse steckt auch in Kapazitätsgleichung)
*/
		sumX=0.;
		for (i=0; i<n; i++)
		{
			if (m_krd->is_a_bacterium[i]) 
			{
				BacteriaMass     = c[i+1];
				sumX += BacteriaMass;
			}

		}

/* Anzahl der mikrobiellen Reaktionen aus Datenstruktur auslesen */
		nreactions=m_krd->NumberReactions; 

/* loop over reactions dX/dt= nymax * X * monodterms * inhibitionterms */
		for (r=0; r<nreactions; r++){

			m_kr = KinReact_vector[r];
			if(m_kr->typeflag_monod){
			BacteriaNumber   = m_kr->bacteria_number+1; 
			BacteriaMass     = c[BacteriaNumber];

			if (BacteriaMass > 1.E-40)
			{
/* Ableitungen werden aus dX/dt = BacGrowth berechnet */
				BacGrowth = m_kr->BacteriaGrowth (r, c, sumX, -1);
				for (i=0; i<n; i++) d2X_dtdS[i+1]=0.;

				maxVelocity=m_kr->rateconstant; 
				if (maxVelocity > 1.E-30)
/* Wachstumsterm, berücksichtige Kapazitätsterm */
				{
/* Erst Berechnen der Ableitungen des Bakterienwachstums nach allen anderen Substanzen
   Ableitung nach der Bakterienmasse (mit Berücksichtigung Kapazitätsterm)
   d2Xi / dt*dXi = BacGrowth *(sumx+maxkap-Xi) / (Xi*(sumx+maxkap))
*/
					d2X_dtdS[BacteriaNumber] = BacGrowth * (sumX+maxkap-BacteriaMass)
						/ (BacteriaMass*(sumX+maxkap));
					for (i=0; i<n; i++){
					if (m_krd->is_a_bacterium[i] && (i+1 != BacteriaNumber))
/* Ableitung nach den anderen Bakterienmassen im Kapazitätsterm
   d2Xi / dt*dXj = BacGrowth / -(sumx+maxkap) */
							d2X_dtdS[i+1] = BacGrowth / -(sumX+maxkap);
					}	
				}
				else
/* Sterbeterm, grundsätzlich keine Berücksichtigung des Kapazitätsterms */
				{
/* d2Xi / dt*dXi = BacGrowth / Xi */
					d2X_dtdS[BacteriaNumber] = BacGrowth / BacteriaMass;

/* d2Xi / dt*dXj = 0 for decay */
				}


			
/* Schleife für Ableitungen nach Substanzen in Monodtermen */ 
				NumberMonod      = m_kr->number_monod; 
				for (i=0; i<NumberMonod; i++) 
				{
/* d2X / dt*dS_j =      S_j = monod-species
   S_j may be ZERO or below !
*/
					MonodSpecies       = m_kr->monod[i]->speciesnumber+1; 
					MonodConcentration = m_kr->monod[i]->concentration; 
					CMonodSpecies      = c[MonodSpecies];

					if (CMonodSpecies > 1.E-20)
					{
/* S_j > 0, normal Monod Term used
   divide BacGrowth through Monod-Term of spec. j
   and multiplicate with partial derivate of Monod-Term
*/
						d2X_dtdS[MonodSpecies] = BacGrowth * MonodConcentration /
							CMonodSpecies / (MonodConcentration+CMonodSpecies);
					}
					else if (CMonodSpecies < -1.E-20)
					{
/* S_j << 0, linear Monod Term used */

      //d2X_dtdS[MonodSpecies] = BacGrowth / CMonodSpecies ; 
						d2X_dtdS[MonodSpecies] = BacGrowth / CMonodSpecies /1000;  // Changed monod term with smaller slope CB 
					}
					else
					{
/* S_j near 0 numerically instable
   recompute BacGrowth without S_j
   (hope, that will only sometimes occur)
*/
						d2X_dtdS[MonodSpecies] = m_kr->BacteriaGrowth (r, c, sumX, MonodSpecies)/MonodConcentration;
					}
				}

/* Schleife für Ableitungen nach Substanzen in Inhibitionstermen */ 
				NumberInhibition = m_kr->number_inhibit; 
				for (i=0; i<NumberInhibition; i++) 
				{
/* d2X / dt*dS_j =      S_j = inhibition-species
   S_j may be Zero without any problem
*/
					InhibitionSpecies       = m_kr->inhibit[i]->speciesnumber+1; 
					InhibitionConcentration = m_kr->inhibit[i]->concentration; 
					CInhibitionSpecies      = c[InhibitionSpecies];

					if (CInhibitionSpecies > 0.)
					{
/* S_j > 0, normal Inhibition Term used
   divide BacGrowth through Inhibition-Term of spec. j
   and multiplicate with partial derivate of Inhi-Term
*/
						d2X_dtdS[InhibitionSpecies] = - BacGrowth / (InhibitionConcentration+CInhibitionSpecies); 
 					}
					else
					{
/* S_j <= 0, linear Inhibition Term used */
						 //d2X_dtdS[InhibitionSpecies] = - BacGrowth / (InhibitionConcentration-CInhibitionSpecies);// CB changed as in next line 
						d2X_dtdS[InhibitionSpecies] = - BacGrowth / (InhibitionConcentration);// CB changed due to stimulance of growth for neg conc.
     }
				}

/* transfer partial derivatives to dfdc-Array of equation solver */
				if(m_kr->grow) 
				{
					for (i=0; i<n; i++)
/* transfer der berechneten Ableitungen für die Bakteriengruppe */
					    dfdc[BacteriaNumber][i+1] += d2X_dtdS[i+1];
				}
/* Berechnung der Ableitungen für die vom Bakteriellen Wachstum abhängigen Substanzen */
/* d2S_j / dt*dS_k = yield(j) * d2X/dt*dS_k */
				porosity1	= m_kr->GetPorosity(BacteriaNumber-1,node);
				
				for (i=0; i<n; i++)
				{
					Yield=m_kr->ProductionStoch[i]; 
					if (fabs(Yield)>1.E-30)
					{
						porosity2	= m_kr->GetPorosity(i,node);

						for (j=0; j<n; j++)
						{
							if (fabs(d2X_dtdS[j+1])>1.E-30)
								dfdc[i+1][j+1] += d2X_dtdS[j+1] * Yield * porosity1 / porosity2;
						}
					}
				}
			

			} /* Ende if BacteriaMass > 1e-30*/
		
		} /* Ende Schleife über nreactions*/
	}// end if type monod

	}	
	else
	{
/* Komplette Berechnung der Ableitungen für den Fall dass KEINE maximale Kapazität berücksichtigt werden muss
*/
/* Anzahl der mikrobiellen Reaktionen aus Datenstruktur auslesen */
		nreactions=m_krd->NumberReactions; 

/* loop over reactions dX/dt= nymax * X * monodterms * inhibitionterms */
		for (r=0; r<nreactions; r++)
		{
			m_kr = KinReact_vector[r];
			if(m_kr->typeflag_monod){
			BacteriaNumber   = m_kr->bacteria_number+1; 
			BacteriaMass     = c[BacteriaNumber];

			if (BacteriaMass > 1.E-40)
			{
/* Ableitungen werden aus dX/dt = BacGrowth berechnet */
				BacGrowth = m_kr->BacteriaGrowth (r, c, sumX, -1);

				for (i=0; i<n; i++) d2X_dtdS[i+1]=0.;


/* d2Xi / dt*dXi = BacGrowth / Xi */
				d2X_dtdS[BacteriaNumber] = BacGrowth / BacteriaMass;


/* Schleife für Ableitungen nach Substanzen in Monodtermen */ 
				NumberMonod      = m_kr->number_monod; 
				for (i=0; i<NumberMonod; i++) 
				{
/* d2X / dt*dS_j =      S_j = monod-species
   S_j may be ZERO or below !
*/
					MonodSpecies       = m_kr->monod[i]->speciesnumber+1; 
					MonodConcentration = m_kr->monod[i]->concentration; 
					CMonodSpecies      = c[MonodSpecies];

					if (CMonodSpecies > 1.E-20)
					{
/* S_j > 0, normal Monod Term used
   divide BacGrowth through Monod-Term of spec. j
   and multiplicate with partial derivate of Monod-Term
*/
						d2X_dtdS[MonodSpecies] = BacGrowth * MonodConcentration /
							CMonodSpecies / (MonodConcentration+CMonodSpecies);
					}
					else if (CMonodSpecies < -1.E-20)
					{
/* S_j << 0, linear Monod Term used */
 
      d2X_dtdS[MonodSpecies] = BacGrowth / CMonodSpecies /1000; // Changed monod term with smaller slope due to neg. concentrations - CB 
						//d2X_dtdS[MonodSpecies] = BacGrowth / CMonodSpecies;
					}
					else
					{
/* S_j near 0 numerically instable
   recompute BacGrowth without S_j
   (hope, that will only sometimes occur)
*/
						d2X_dtdS[MonodSpecies] = m_kr->BacteriaGrowth (r, c, sumX, MonodSpecies)/MonodConcentration;
					}
				}

/* Schleife für Ableitungen nach Substanzen in Inhibitionstermen */ 
				NumberInhibition = m_kr->number_inhibit; 
				for (i=0; i<NumberInhibition; i++) 
				{
/* d2X / dt*dS_j =      S_j = inhibition-species
   S_j may be Zero without any problem
*/
					InhibitionSpecies       = m_kr->inhibit[i]->speciesnumber+1; 
					InhibitionConcentration = m_kr->inhibit[i]->concentration; 
					CInhibitionSpecies      = c[InhibitionSpecies];

					if (CInhibitionSpecies > 0.)
					{
/* S_j > 0, normal Inhibition Term used
   divide BacGrowth through Inhibition-Term of spec. j
   and multiplicate with partial derivate of Inhi-Term
*/
						d2X_dtdS[InhibitionSpecies] = - BacGrowth /
							 (InhibitionConcentration+CInhibitionSpecies);
 					}
					else
					{
/* S_j <= 0, linear Inhibition Term used */

						//d2X_dtdS[InhibitionSpecies] = - BacGrowth / (InhibitionConcentration-CInhibitionSpecies); // Commented out - see next line - CB
						d2X_dtdS[InhibitionSpecies] = - BacGrowth / (InhibitionConcentration- 0.0);// CB changed due to stimulance of growth for neg conc.

					}
				}
/* transfer partial derivatives to dfdc-Array of equation solver */
				if(m_kr->grow) //if (BioDegradation.Reaction[r].grow)
				{
					for (i=0; i<n; i++)
/* transfer der berechneten Ableitungen für die Bakteriengruppe */
                        dfdc[BacteriaNumber][i+1] += d2X_dtdS[i+1];
				}

/* Berechnung der Ableitungen für die vom Bakteriellen Wachstum abhängigen Substanzen */
/* d2S_j / dt*dS_k = yield(j) * d2X/dt*dS_k */
				porosity1	= m_kr->GetPorosity(BacteriaNumber-1,node);

				for (i=0; i<n; i++)
				{
					Yield=m_kr->ProductionStoch[i]; 
					if (fabs(Yield)>1.E-30)
					{
						porosity2	= m_kr->GetPorosity(i,node);

						for (j=0; j<n; j++)
						{
							if (fabs(d2X_dtdS[j+1])>1.E-30)
								dfdc[i+1][j+1] += d2X_dtdS[j+1] * Yield * porosity1 / porosity2;
						}
					}
				}
			
			} /* Ende if BacteriaMass > 1e-30*/
		} /* Ende if typeflag_monod */
		} /* Ende Schleife über nreactions*/
	
	} /* Ende if Berechnung mit maximaler Bakterienkapazität */



/**********************************************************************************************/
	/* Berechnung der Ableitungen der Austauschprozesse */
	// calculate already occupied surfaces first 
	if(m_krd->NumberLangmuir > 0){
		// Initialise Surfaces for langmuir isotherms
		for (i=0; i<m_krd->maxSurfaces; i++)	occupiedSurface.push_back(0.0);

		for (r=0; r<nreactions; r++){
			m_kr = KinReact_vector[r];
			if((m_kr->type.compare("exchange")==0)&&(m_kr->typeflag_exchange_langmuir)){
				Sp1			= m_kr->ex_species[0]+1; 
				surfaceID	= m_kr->exSurfaceID; 
				occupiedSurface[surfaceID] += c[Sp1];	
			}
		}
	} // if NumberLangmuir > 0



/* Berechnung der Ableitungen der Austauschprozesse */
	for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];

	if(m_kr->typeflag_exchange){	
/* linearer Austausch mit kd */
		if(m_kr->typeflag_exchange_linear){
//Matrix
		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node);
		density1	= m_kr->GetDensity(Sp1-1,node);
//geloest
		Sp2			= m_kr->ex_species[1]+1; 
		porosity2	= m_kr->GetPorosity(Sp2-1,node);

		exch		= m_kr->ex_param[0]; 
		kd			= m_kr->ex_param[1]; 

		if (fabs(kd)<MKleinsteZahl){
//no kd, exchange between two species in solution
		    dfdc[Sp1][Sp1] +=  -exch/porosity1;
			dfdc[Sp1][Sp2] +=   exch/porosity1;
			dfdc[Sp2][Sp1] +=   exch/porosity2;
			dfdc[Sp2][Sp2] +=  -exch/porosity2;
		}
		else {
// with kd, exchange between matrix (mol/kg) and solution (mol/l)
  		    foc = m_krd->node_foc[node];
		    if(foc > MKleinsteZahl)	kd = kd*foc;
		    dfdc[Sp1][Sp1] +=  -exch;
			dfdc[Sp1][Sp2] +=   exch*kd;
		    dfdc[Sp2][Sp1] +=   exch*porosity1/porosity2*density1;
			dfdc[Sp2][Sp2] +=  -exch*kd*porosity1/porosity2*density1;
		}

		} // linear
/* Langmuir Kinetik */
		if(m_kr->typeflag_exchange_langmuir){
// calculated already occupied surfaces above 
		
		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node); 


		Sp2			= m_kr->ex_species[1]+1; 
		porosity2	= m_kr->GetPorosity(Sp2-1,node);

		kadsorb		= m_kr->ex_param[0]; 
		kdesorb		= m_kr->ex_param[1]; 
//SB_langmuir		surfaceID	= m_kr->exSurfaceID; //Exchange.Langmuir[i].SurfaceID;
		totalSurface= m_krd->exSurface[m_kr->exSurfaceID]; 

		//      occupiedSurface is calculated just above

		adsorb = kadsorb*(totalSurface-occupiedSurface[surfaceID]);

		dfdc[Sp1][Sp1] += -kdesorb;
		dfdc[Sp2][Sp1] +=  kdesorb*porosity1/porosity2;
		dfdc[Sp1][Sp2] +=  adsorb;
		dfdc[Sp2][Sp2] += -adsorb*porosity1/porosity2;

// additional derivatives due to occupied surface 
		for (j=0; j<nreactions; j++){
			m_kr1 = KinReact_vector[j];
			if(m_kr1->type.compare("exchange")==0)
				if(m_kr1->typeflag_exchange_langmuir){	
					SpX			= m_kr1->ex_species[0]+1; 
					surfaceID2	= m_kr1->exSurfaceID; 
					if (surfaceID == surfaceID2){
						dfdc[Sp1][SpX] += -kadsorb*c[Sp2];
						dfdc[Sp2][SpX] +=  kadsorb*c[Sp2]*porosity1/porosity2;
					}
			}
		}

	} // end if langmuir
	
/* Freundlich Kinetik */
		if(m_kr->typeflag_exchange_freundlich){

		Sp1			= m_kr->ex_species[0]+1; 
		porosity1	= m_kr->GetPorosity(Sp1-1,node); 
		density1	= m_kr->GetDensity(Sp1-1,node);
		Sp2			= m_kr->ex_species[1]+1; 
		porosity2	= m_kr->GetPorosity(Sp2-1,node); 
		exponent    = m_kr->ex_param[2]; 
		parameter   = m_kr->ex_param[1]; 
		exch		= m_kr->ex_param[0]; 

		if (c[Sp2] > residual){
			// no linearisation required 
			adsorb=exch*parameter*exponent*pow(c[Sp2],(exponent-1.0));
		}
		else {
			// linearisation required due to instability of c^x if c<residual
			adsorb=exch*parameter*exponent*pow(residual,(exponent-1.0));
		}

		dfdc[Sp1][Sp1] += -exch;
		dfdc[Sp2][Sp1] +=  exch*porosity1/porosity2*density1;
		dfdc[Sp1][Sp2] +=  adsorb;
		dfdc[Sp2][Sp2] += -adsorb*porosity1/porosity2*density1;
	} // end freundlich
} // end if exchange
} // end loop over reactions


//#ds
/**********************************************************************************************/
/* NAPL-dissolution */
/**********************************************************************************************/
	for (r=0; r<nreactions; r++){
    m_kr = KinReact_vector[r];

		if(m_kr->typeflag_napldissolution){
		/* NAPL-Lösung */

		Sp1			= m_kr->ex_species[0]+1; //Exchange.Linear[i].Species1; should be NAPL
//		porosity1	= m_kr->GetPorosity(Sp1-1,node);
		blob		= m_kr->blob_ID; 
		m_kb = KinBlob_vector[blob];                // pointer to blob-properties set in the reaction r

		Sp2			= m_kr->ex_species[1]+1; //Exchange.Linear[i].Species2; should be dissolved
		porosity2	= m_kr->GetPorosity(Sp2-1,node);
//#ds TODO	    saturation2 = ??
		saturation2 = 1.;
		
		exch		= m_kb->Masstransfer_k * m_kb->current_Interfacial_area ; 
/* Remark: In function derivs the dissolution velocity can be reduced, if NAPL concentration falls below zero. 
           The partial derivatives should change, too, but this is not considered here. 
		   However, it works fine */

//		dfdc[Sp1][Sp1] = 0     derivatives for NAPL-concentration always zero
//		dfdc[Sp2][Sp1] = 0

		if ( (m_kr->current_Csat < c[Sp2]) || (c[Sp1] > MKleinsteZahl) ) {         // no dissolution or NAPL mass present
// d2CNAPL / dt dCmob = k*A
// d2Cmob  / dt dCmob = -k*A/n/Sw
			dfdc[Sp1][Sp2] +=    exch;
			dfdc[Sp2][Sp2] +=  - exch / porosity2 / saturation2;
		}

		} // NAPL-dissolution
	} // loop over reactions r
//#ds

	free_dvector(d2X_dtdS,1,n);

    
	return;
}

/**************************************************************************
Reaction-Method: 
Task: Reaction class test output function
Programing:
05/2004 SB Implementation - adapted from OK rf_bc_new
02/2006 SB Adapted to new FEM structure
**************************************************************************/
void CKinReact::TestWrite(void){

int i, length, flag=0;

// Write Keyword
cout << "8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"<<endl;
cout << " Test Output " << endl;
cout << "8888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888"<<endl;
cout << "#REACTION" << endl;
// Name of reaction
cout << "$NAME" << endl << name << endl;
// Type of reaction
cout << "$TYPE" << endl << type << endl;
// bacteria name
cout << "$BACTERIANAME" << endl << bacteria_name << endl;
//ReactionEquation
cout << "$EQUATION" << endl ; 
for(i=0;i<number_reactionpartner;i++){
	if(stochmet[i] < 0.0){ //left side of equation
		if(i == 0) 
			cout << " " << fabs(stochmet[i]) << " " << reactionpartner[i];
		else
			cout << " + " << fabs(stochmet[i]) << " " << reactionpartner[i];
	}
	if(stochmet[i]>0 && (flag>0)) // remaining right hand side
		cout << " + " << fabs(stochmet[i]) << " " << reactionpartner[i];
	if(stochmet[i]>0 && (flag==0)){ // " = " Sign and first term on right hand side
		cout << " = " << fabs(stochmet[i]) << " " << reactionpartner[i];
		flag = 1;
	}
}
cout << endl;
// Rateconstant and order
cout << "$RATEKONSTANT" << endl << rateconstant << "   " << rateorder << endl;
cout << "$GROWTH" << endl << grow << endl;
//Monod terms
cout << "$MONODTERMS" << endl << number_monod << endl;
for(i=0;i<number_monod;i++)
	cout << monod[i]->species << "  " << monod[i]->concentration << "  " << monod[i]->order << endl;
//Inhibition terms
cout << "$INHIBITIONTERMS" << endl << number_inhibit << endl;
for(i=0;i<number_inhibit;i++)
	cout << inhibit[i]->species << "  " << inhibit[i]->concentration << "  " << inhibit[i]->order << endl;
// Production Terms
cout << "$PRODUCTIONTERMS" << endl << number_production << endl;
for(i=0;i<number_production;i++)
	cout << production[i]->species << "  " << production[i]->concentration << "  " << production[i]->order << endl;
// ProductionStochhelp Terms
cout << "$PRODUCTIONSTOCH" << endl << (int)this->ProdStochhelp.size() << endl;
for(i=0;i<(int)this->ProdStochhelp.size();i++)
	cout << ProdStochhelp[i]->species << "  " << ProdStochhelp[i]->concentration << endl;
// exchange
cout << "$EXCHANGE_PARAMETERS" << endl << (int)this->ex_param.size() << endl;
for(i=0;i<(int)this->ex_param.size();i++)
	cout << this->ex_param[i] << "  " ;
cout << endl;

cout << endl;

cout << "number_reactionpartner " << (int)this->number_reactionpartner << endl;
cout << "bacteria_number " << (int)this->bacteria_number << endl;
cout << "grow " << this->grow << endl;
length = (int)this->ProductionStoch.size();
cout << "length ProductionStoch: " << length << endl;
for(i=0; i<length;i++) cout << (int)this->ProductionStoch[i]<< " ";
cout << endl;

length = (int)this->ex_species.size();
cout << "length exSpecies: " << length << endl;
for(i=0; i<length;i++) cout << this->ex_species_names[i]<< " " << this->ex_species[i] << endl;
cout << endl;
cout << " sorption type : " << this->exType << endl;

// Test output
}

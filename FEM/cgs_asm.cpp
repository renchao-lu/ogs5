/**************************************************************************/
/* ROCKFLOW - Modul: cgs_asm.c
                                                                          */
/* Aufgabe:
   Aufstellen des Gleichungssystems fuer das Modul aSM.
                                                                          */
/* Programmaenderungen:
   03/1997   RK   Uebertragen aus dem aTM
   07/1997   RK   Interpol_q_XD -> adaptiv.c
   07/1997   RK   CalcIrrNodes_ASM umbenannt zu CalcIrrNodeVals (erweitert) -> adaptiv.c
   07/1997   RK   Adaptivitaet aus dem Modul herausgenommen (DelIrrNodesXD -> adaptiv.c)
   09/1997   OK   Nichtlineare Fliessgesetze fuer 1-D Elemente
   11/1997   OK   Nichtlineare Fliessgesetze fuer 2-D Elemente
   01/1998   AH   Dirichlet-Randbedingungen nach neuem Konzept
   02/1999   AH   Aktualisierung Quellen/Senken
   09/1999   AH   GetNumericalTimeCollocation fuer stat. Fall
   11/1999   RK   EliminateIrrNodes()
   12/1999   CT   Korrektur Massenquellen
   12/1999   CT   Irr. Knoten _vor_ Dirichlet-BC
   05/2000   OK   Kapselung nichtlineare Fliessgesetze
   07/2000   OK   Kopplungsterme fuer Konsolidierungsmodelle
   11/2000   OK   Speichermodelle
   11/2000   OK   allg Assemblierungsfunktion (MakeElementEntryEQS_ASM) vorbereitet
   06/2001   MK   DM/ASM Kopplungsterme korrigiert
   03/2003   RK   Quellcode bereinigt, Globalvariablen entfernt
   10/2004   WW   3D Coupling matrix
                                                                          */
/**************************************************************************/
#include "stdafx.h" /* MFC */
#include "makros.h"
#define noTESTCGS_ASM
#include "rf_pcs.h" //OK_MOD"
#include "int_asm.h"
#include "rf_pcs.h"
#include "nodes.h"
#include "elements.h"
#include "rf_mmp_new.h"
#include "rf_num_new.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"
#include "adaptiv.h"
#include "mathlib.h"
#include "matrix.h"
#include "femlib.h"
#include "par_ddc.h"
extern double gravity_constant;

/* Interne (statische) Deklarationen */
double ASMGetSourceMultiplicator(long i);
void MakeElementEntryEQS_ASM(long,double*,double*,CPARDomain*,CRFProcess*);
void MakeElementEntryEQS_OVLF(long e,double *rechts,double *ergebnis,\
                             CPARDomain *m_dom,CRFProcess *m_pcs);

static double capacitance_matrix_cgs[64];
static double conductance_matrix_cgs[64];
static double gravity_vector_cgs[8];

//Declarations in the follwoing lines produce warnings
//static double capacitance_matrix_line[4];
//static double capacitance_matrix_quad[16];
//static double capacitance_matrix_hexa[64];
//static double conductance_matrix_line[4];
//static double conductance_matrix_quad[16];
//static double conductance_matrix_hexa[64];
//static double gravity_vector_line[2];
//static double gravity_vector_quad[4];
//static double gravity_vector_hexa[8];
/**************************************************************************
FEMLib-Method: 
Task:
Programing:
07/2004 OK Implementation
last modified:
**************************************************************************/
void SMAssembleMatrix(double *rechts,double *ergebnis,double aktuelle_zeit,\
                      CRFProcess *m_pcs)
{
  /* Datentypen der verwendeten Variablen */
  aktuelle_zeit = aktuelle_zeit;
  ergebnis = ergebnis;
#ifdef PARALLEL
  rechts = rechts;
  int j;
  CPARDomain *m_dom = NULL;
  int no_domains =(int)dom_vector.size();
  for(j=0;j<no_domains;j++){
    m_dom = dom_vector[j];
    m_dom->AssembleMatrix(m_pcs);
    ///OK m_dom->WriteMatrix();
  }
#else
  long elanz;
  long index;
  Element* element = NULL;
  elanz = ElListSize();
  for (index=0;index<elanz;index++) {
    element = ElGetElement(index);
    if(element&&element->aktiv&&(element->group_number>-1)){ //OK4104
      if (ElGetElementActiveState(index)) {
		if(m_pcs->pcs_type_name.find("OVERLAND")!=string::npos)		
			MakeElementEntryEQS_OVLF(index,rechts,ergebnis,NULL,m_pcs);
		else
        MakeElementEntryEQS_ASM(index,rechts,ergebnis,NULL,m_pcs);
      }
    }
  }
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);
#endif
}



/**************************************************************************/
/* ROCKFLOW - Funktion: MakeElementEntryEQS_ASM
                                                                          */
/* Aufgabe:
   Elementbeitraege zum Gleichungssystem (EQS)
   (Element-Matrizen und RHS-Vektor in EQS einbauen)
                                                                          */
/* Formalparameter:
   long index:     Element-Nummer
   double *rechts: RHS-Vektor
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   03/1997   RK   Uebertragen aus dem aTM
   07/1997   RK   Adaptivitaet herausgenommen DelIrrNodes3D -> adaptiv.c
   07/1997   RK   Angepasst an das aSM
   11/2000   OK   Speicheroptimierung
   11/2000   OK   Assemblierungsfunktion fuer alle Elementtypen verallgemeinert
   10/2003   OK   Bugfix
   10/2003   WW   Some correction in coupled iterm and generalization
   10/2004   WW   3D coupling matrix
   02/2005   MB   head version                                               */
/**************************************************************************/
void MakeElementEntryEQS_ASM(long e,double *rechts,double *ergebnis,\
                             CPARDomain *m_dom,CRFProcess *m_pcs)
{
 /* Datentypen der verwendeten Variablen */
 static int i,j;
   /* Zeitdiskretisierung */
 static double dt_inverse,theta;
   /* Element-Matrizen */
 static double *capacitance_matrix;
 static double *conductance_matrix;
 static double *gravity_vector;
 // static double *coupling_matrix_x;
 //static double *coupling_matrix_y;
 //static double *coupling_matrix_z;
 // static double *coupling_matrix_z;
 static double left_matrix[64];
 static double right_matrix[64];
   /* Knoten-Daten */
 long nn,nn2, nd;
 long *element_nodes;
 static long global_node_number[8];
 // static double ux_old_iter[20],uy_old_iter[20], uz_old_iter[20];
 // static double d_strain_x[8],d_strain_y[8],d_strain_z[8];
 static double p_old_time[8];
 static double explicit_pressure[8];
 //static double right_vector[8],right_vector_p[8], right_vector_u[8];
 static double right_vector_p[8], right_vector_u[8];
   /* Materialdaten */
 static int nonlinear_flow_element;
 static double k_rel_iteration, k_rel_old_time;
 // For monolithic scheme 
 static long NodeShift[4];
 int group;						  // CMCD Einbau
 double gp[3];					  // CMCD Einbau
 gp[0] = gp[1] = gp[2] = 0.0;     // CMCD Einbau
 long index;
 m_dom = m_dom;
 index = e;
 nn = NumbersOfElementNode(index);
 nd = NumbersOfElementNodeHQ(index);

 ergebnis = ergebnis;
  /* Flux calculation */
 /*static double pressure_new[4];
 static double flux[4];*/

 for(i=0; i<4; i++) NodeShift[i] = 0;
 /*************************************************************************/
 /* Daten bereitstellen                                                   */
 /*************************************************************************/
 /* Parameter fuer die Ortsdiskretisierung */
 nn = ElNumberOfNodes[ElGetElementType(index)-1];
 nn2 = nn*nn;
 element_nodes = ElGetElementNodes(index); /* Original-Knotennummern des Elements */
 //-----------------------------------------------------------------------
 // NUM
 theta = m_pcs->m_num->ls_theta; //OK
 //-----------------------------------------------------------------------
 /* Materialparameter */
 CMediumProperties *m_mmp = NULL; // CMCD Einbau
 group = ElGetElementGroupNumber(index);  // CMCD Einbau
 m_mmp = mmp_vector[group];   // CMCD Einbau
 m_mmp->mode = 1;   // CMCD Einbau
 
if(mmp_vector.size()>0){
 nonlinear_flow_element = 0;
}
else{
//SB:m nonlinear_flow_element = GetNonlinearFlowElement(index);
nonlinear_flow_element = 0;
}
 k_rel_iteration = 1.0;
 k_rel_old_time = 1.0;

 /*---------------------------------------------------------------------------*/
 /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
 if(memory_opt==0) { /* gespeicherte Element-Matrizen holen */
     capacitance_matrix = SMGetElementCapacitanceMatrix(index,m_pcs->pcs_number);
     conductance_matrix = SMGetElementConductanceMatrix(index,m_pcs->pcs_number);
     gravity_vector = SMGetElementGravityVector(index,m_pcs->pcs_number);
 }
 else if (memory_opt==1) { /* jede Element-Matrix berechnen */
     if(GetRFProcessProcessing("SD")) { 
         DisplayMsgLn("MakeElementEntryEQS_ASM: Speichermodell fuer Deformationsmodelle"); 
         abort();
     }
     printf("\r                  Element-Matrix: %ld",index);
     CalcElementMatrix_ASM_New(index,\
                               capacitance_matrix_cgs,\
                               conductance_matrix_cgs,\
                               gravity_vector_cgs);
 }
 else if (memory_opt==2) { /* Element-Prototypen benutzen */
     if(GetRFProcessProcessing("SD")) { 
         DisplayMsgLn("MakeElementEntryEQS_ASM: Speichermodell fuer Deformationsmodelle"); 
         abort();
     }
 }

#ifdef TESTCGS_ASM
    MZeigMat(capacitance_matrix,nn,nn,"Flow capacitance matrix");
    MZeigMat(conductance_matrix,nn,nn,"Flow conductance matrix");
    MZeigVec(gravity_vector,nn,"Flow gravity vector");
#endif


 /*---------------------------------------------------------------------------*/
 /* Parameter fuer die Zeitdiskretisierung */
 if (dt < MKleinsteZahl) {
     DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
     return;
 }
 else {
     dt_inverse = 1.0 / dt;
 }
 /*************************************************************************/
 /* Gleichungssystem aufbauen                                             */
 /*************************************************************************/
 /* Initialisierungen */
/* ? ueberhaupt noetig
 MNulleMat(left_matrix,nn,nn);
 MNulleMat(right_matrix,nn,nn);
 MNulleVec(right_vector,nn);
 MNulleVec(pressure_old,nn);
 MNulleVec(explicit_pressure,nn);
*/

 /*---------------------------------------------------------------------------*/
 /* Permeabilitaet fuer nichtlineares Fliessgesetz berechnen */
 /* MB: NULL Zeiger entfernt */
 if(nonlinear_flow_element == 1)
	k_rel_iteration = m_mmp->NonlinearFlowFunction(index,gp,theta);
 /* Beitraege des betrachteten Elementes
    zur linken Systemmatrix (neuer Zeitpunkt) berechnen: 
    [C]/dt + theta [K] k_rel */
 if(memory_opt==0) { 
     for(i=0;i<nn2;i++)
         left_matrix[i] = dt_inverse * capacitance_matrix[i] + \
                          theta * k_rel_iteration * conductance_matrix[i];
 }
 else if (memory_opt==1) {
     for(i=0;i<nn2;i++)
         left_matrix[i] = dt_inverse * capacitance_matrix_cgs[i] + \
                          theta * k_rel_iteration * conductance_matrix_cgs[i];
 }

#ifdef TESTCGS_ASM
    MZeigMat(left_matrix,nn,nn,"Left matrix");
#endif

 /*---------------------------------------------------------------------------*/
 /* Beitraege des betrachteten Elementes
    zur rechten Systemmatrix (alter Zeitpunkt) berechnen 
    [C]/dt - (1-theta) [K] k_rel */

   /*-------------------------------------------------------------*/
   /* 2.1 Stroemungs-Terme */
   /* Permeabilitaet fuer nichtlineares Fliessgesetz berechnen */
   /* MB: NULL Zeiger entfernt */
   if(nonlinear_flow_element == 1)
     k_rel_old_time = m_mmp->NonlinearFlowFunction(index,gp,theta);
   k_rel_old_time = k_rel_old_time* m_mmp->PermeabilityPressureFunction(index,gp,theta);//CMCD Einbau Correction
   /* rechte Matrix */
 if(memory_opt==0) { 
     for(i=0;i<nn2;i++)
         right_matrix[i] = dt_inverse * capacitance_matrix[i] - \
                           (1.0-theta) * k_rel_old_time * conductance_matrix[i];
 }
 else if (memory_opt==1) {
     for(i=0;i<nn2;i++)
         right_matrix[i] = dt_inverse * capacitance_matrix_cgs[i] - \
                           (1.0-theta) * k_rel_old_time * conductance_matrix_cgs[i];
 }

#ifdef TESTCGS_ASM
    MZeigMat(right_matrix,nn,nn,"Right matrix");
#endif
 /* Ergebnisse aus letztem Zeitschritt uebertragen */
 string pcs_primary = m_pcs->pcs_primary_function_name[0];
 int nidx0 = PCSGetNODValueIndex(pcs_primary,0);

 for (i=0;i<nn;i++){
   //p_old_time[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",0)); // ASMGetNodePress0(element_nodes[i]);
   p_old_time[i] = GetNodeVal(element_nodes[i],nidx0);
 }
   /* Rechte-Seite-Vektor bilden */
 MMultMatVec(right_matrix,nn,nn,p_old_time,nn,explicit_pressure,nn);
#ifdef TESTCGS_ASM
    MZeigVec(explicit_pressure,nn,"U vector");
#endif
 if(memory_opt==0) { 
     for (j=0;j<nn;j++)
         right_vector_p[j] = explicit_pressure[j] - k_rel_iteration * gravity_vector[j];
 }
 else if (memory_opt==1) {
     for (j=0;j<nn;j++)
         right_vector_p[j] = explicit_pressure[j] - k_rel_iteration * gravity_vector_cgs[j];
 }
#ifdef TESTCGS_ASM
    //MZeigVec(right_vector_p,nn,"RHS vector P");
#endif

  /*-------------------------------------------------------------*/
 // If deformation coupling
#ifdef TESTCGS_ASM
    //MZeigVec(right_vector_u,nn,"RHS vector U");
#endif
#ifdef TESTCGS_ASM
// MZeigVec(right_vector_u,NodeListLength,"MakeGS2D_ASM: R(u^n+1)");
#endif


 /*---------------------------------------------------------------------------*/
 /* lokale Knotennummern des Elements gegen globale
    Knotennummern (Indizes im Gesamtgleichungssystem) ersetzen */
  for(i=0;i<nn;i++)
#ifdef PARALLEL
    global_node_number[i] = element_nodes[i];
#else
    global_node_number[i] = GetNodeIndex(element_nodes[i]);
#endif

 /* irregulaere Knoten eliminieren */
 if (GetRFControlGridAdapt())
   if (AdaptGetMethodIrrNodes() == 1)
     /* MB: switch für 2D / 3D */
     switch (ElGetElementType(index)) {
       case 2: DelIrrNodes2D(index,global_node_number,left_matrix,right_vector_p);
          break;
       case 3: DelIrrNodes3D(index,global_node_number,left_matrix,right_vector_p);
          break;
	 }     
     


 /* Einspeichern in die linke Systemmatrix */
 for(i=0;i<nn;i++)
   for(j=0;j<nn;j++)
      MXInc(global_node_number[i], global_node_number[j],left_matrix[i*nn+j]);

 /* Einspeichern in den Rechte-Seite-Vektor */
 for (i=0;i<nn;i++)
   rechts[global_node_number[i]] += right_vector_p[i] + \
         right_vector_u[i];

#ifdef TESTCGS_ASM
    MZeigVec(rechts,nn,"RHS vector");
#endif

 element_nodes = NULL;

}


/*
   02/2005   MB   head version                                               
**************************************************************************/
void MakeElementEntryEQS_OVLF(long e,double *rechts,double *ergebnis,\
                             CPARDomain *m_dom,CRFProcess *m_pcs)
{
 /* Datentypen der verwendeten Variablen */
 static int i,j;
   /* Zeitdiskretisierung */
   /* Element-Matrizen */
 static double *Stiffness;
 static double *RHS;

   /* Knoten-Daten */
 long *element_nodes;
 static long global_node_number[8];

 long index;
 m_dom = m_dom;
 index = e;
 int nn = NumbersOfElementNode(index);

 ergebnis = ergebnis;
 int nn2;
 nn2 = nn*nn;
 element_nodes = ElGetElementNodes(index); /* Original-Knotennummern des Elements */

 /*---------------------------------------------------------------------------*/
 Stiffness = SMGetElementConductanceMatrix(index,m_pcs->pcs_number);
 RHS = SMGetElementGravityVector(index,m_pcs->pcs_number);

#ifdef TESTCGS_ASM
    MZeigMat(Stiffness,nn,nn,"Stiffness");
    MZeigVec(RHS,nn,"RHS");
#endif


#ifdef TESTCGS_ASM
    MZeigMat(left_matrix,nn,nn,"Left matrix");
#endif

 for(i=0;i<nn;i++)
#ifdef PARALLEL
    global_node_number[i] = element_nodes[i];
#else
    global_node_number[i] = GetNodeIndex(element_nodes[i]);
#endif


 /* Einspeichern in die linke Systemmatrix */
 for(i=0;i<nn;i++)
   for(j=0;j<nn;j++)
      MXInc(global_node_number[i], global_node_number[j],Stiffness[i*nn+j]);

 /* Einspeichern in den Rechte-Seite-Vektor */
 for (i=0;i<nn;i++)
   rechts[global_node_number[i]] += RHS[i];

#ifdef TESTCGS_ASM
    MZeigVec(rechts,nn,"RHS vector");
#endif

 element_nodes = NULL;

}

///////////////////////////////////////////
/**************************************************************************
 ROCKFLOW - Function: LOPCalcHeads_MB

 Task:
   Calculates Heads of nodes.

 Parameter: (I: Input; R: Return; X: Both)   - void -
 
 I: int phase, 

 Result:
   - void -

 Programming:
 09/2002   MB   Implementation based on LOPCalcFluxes_MB

**************************************************************************/
void ASMCalcHeads(int phase, int timelevel)
{
  long i;
  double node_head,node_pressure;
  double g, rho;
  int nidx, nidy;
  //----------------------------------------------------------------------
  // MFP fluid properties
  g = gravity_constant;
  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  m_mfp->mode = 1;
  rho = m_mfp->Density();
  m_mfp->mode = 0;
  nidx = PCSGetNODValueIndex("PRESSURE1",timelevel);
  nidy = PCSGetNODValueIndex("HEAD",timelevel);
  for (i=0;i<NodeListSize();i++) { /* alle jemals existierenden Knoten */
    /* head ohne Berücksichtung der Dichte */
    node_pressure = GetNodeVal(i,nidx);
    node_head = node_pressure / (g*rho) + GetNodeZ(i);
    SetNodeVal(i,nidy,node_head);
  }
}

/**************************************************************************
 ROCKFLOW - Funktion: ASMCalcNodeFluxes
                                                                          
 Task:
 Berechnung und Speichern der Knotenflüsse
 Parameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: long i: node index
 Result:
   - void -
                                                                          
 Programmaenderungen:
   11/2002   MB/OK  Implementation
   10/2004   MB     PCS                                                   
**************************************************************************/

void ASMCalcNodeFluxes(void)
{
 long index;
 int i;
   /* Zeitdiskretisierung */
 static double dt_inverse,theta;
   /* Element-Matrizen */
 static double *capacitance_matrix;
 static double *conductance_matrix;
 static double left_matrix[36];
 static double right_matrix[36];

 static double *gravity_vector;
   /* Knoten-Daten */
 static long nn,nn2;
 long *element_nodes;
   /* Materialdaten */
 static double k_rel_iteration;
 static double pressurenew[6], pressureold[6];
 static double flux[6], fluxnew[6], fluxold[6];

 // static double gp[3]; 

 int phase=0; int timelevel=1;
 static double g, rho;
 int test;

 
 CRFProcess *m_pcs=NULL;

 //Achtung Fusch:
 //m_pcs = m_pcs->GetProcess("OVERLAND_FLOW");
 //funktioniert nicht, deshalb:
 m_pcs = m_pcs->GetProcessByNumber(1);

 theta = m_pcs->m_num->ls_theta;
 //anzgp = m_pcs->m_num->ele_gauss_points;

 /* Time discretization */
 dt_inverse = 1.0 / dt;

 /* Initialize NodeFlux from previous time steps */
 /* Important as contributions of elements are added with */
 /* flux[i] += MODGetNodeFlux_MB(phase,element_nodes[i],-1,timelevel */
 phase = 0;
 test = PCSGetNODValueIndex("FLUX",1);
 test = PCSGetNODValueIndex("FLUX",0);
 
 timelevel = 1;
 for (i=0;i<NodeListLength;i++) {
   if (GetNode(i)!=NULL) {  /* wenn Knoten existiert */
   SetNodeVal(i,PCSGetNODValueIndex("FLUX",timelevel),0.0);
   }
 }

 /*=============================================================================*/
 for (index=0;index<ElListSize();index++) {
   if (ElGetElement(index)!=NULL) { /* Element existiert */
   /* Parameter fuer die Ortsdiskretisierung */
   nn = ElNumberOfNodes[ElGetElementType(index)-1];
   nn2 = nn*nn;
   element_nodes = ElGetElementNodes(index);
   /*-------------------------------------------------------------------------*/
   /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
   if(m_pcs){
     capacitance_matrix = SMGetElementCapacitanceMatrix(index,m_pcs->pcs_number);
     conductance_matrix = SMGetElementConductanceMatrix(index,m_pcs->pcs_number);
     gravity_vector = SMGetElementGravityVector(index,m_pcs->pcs_number);
   }  

   k_rel_iteration = 1.0; /* CalcNonlinearFlowPermeability1(index,ASMGetNodePress1,NULL); */
   /*-------------------------------------------------------------------------*/
   /* LHS matrix: [C]/dt + theta [K] k_rel */
   for(i=0;i<nn2;i++) {
     left_matrix[i] = dt_inverse * capacitance_matrix[i] + \
                      theta * k_rel_iteration * conductance_matrix[i];
   }
   /*-------------------------------------------------------------------------*/
   timelevel = 1; 
   //g = gravity_constant;
   g = 0.0;
   rho = 1000.0;
   for (i=0;i<nn;i++) {
     pressurenew[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",timelevel));
   }
   MMultMatVec(left_matrix,nn,nn,pressurenew,nn,fluxnew,nn);
   /*-------------------------------------------------------------------------*/
   /* RHS matrix */
   timelevel = 0;
   for(i=0;i<nn2;i++){
     right_matrix[i] = dt_inverse * capacitance_matrix[i] - \
                       (1.0 - theta) * k_rel_iteration * conductance_matrix[i];
   }
            
   for (i=0;i<nn;i++) {
     pressureold[i] = GetNodeVal(element_nodes[i],PCSGetNODValueIndex("PRESSURE1",timelevel));
   }
   MMultMatVec(right_matrix,nn,nn,pressureold,nn,fluxold,nn);
   
   for(i=0;i<nn;i++){
     flux[i] = fluxnew[i] - fluxold[i];
   }
          
   /*-------------------------------------------------------------------------*/
   /* Sum fluxes for nodes */
   /* Contributions from different elements are added to calculate the node flux */ 
   timelevel = 1;
   for (i=0;i<nn;i++) {
     flux[i] += GetNodeVal(element_nodes[i],PCSGetNODValueIndex("FLUX",timelevel));
     if(fabs(flux[i])<MKleinsteZahl) flux[i]=0.0;
     SetNodeVal(element_nodes[i],PCSGetNODValueIndex("FLUX",timelevel),flux[i]);
   }
   } /* endif element exists */  
 } /* endfor element loop */
 /*=============================================================================*/
}

/**************************************************************************
 ROCKFLOW - Funktion: ASMCalcNodeWDepth
                                                                          
 Task:
   Berechnung und Speichern der Knotenflüsse
 Parameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: long i: node index
 Result:
   - void -
                                                                          
 Programmaenderungen:
   11/2002   MB/OK  Implementation
   10/2004   MB     PCS                                                   
**************************************************************************/

void ASMCalcNodeWDepth(CRFProcess *m_pcs)
{
int nidx, nidy;
int timelevel = 1; 
int i;
double WDepth;

if(m_pcs->m_msh){
//  nidx = GetNodeValueIndex("HEAD")+1;
//  nidy = GetNodeValueIndex("WDEPTH")+1;
  nidx = m_pcs->GetNodeValueIndex("HEAD")+1;
  nidy = m_pcs->GetNodeValueIndex("WDEPTH");
  
  //for (i=0;i<NodeListLength;i++) {
  for(long nn=0;nn<(long)m_pcs->m_msh->nod_vector.size();nn++){
    //if (GetNode(i)!=NULL) {  /* wenn Knoten existiert */
      WDepth = m_pcs->GetNodeValue(nn, nidx) - m_pcs->m_msh->nod_vector[nn]->Z();
      if (WDepth < 0.0) {
        WDepth  = 0.0;
      }
      m_pcs->SetNodeValue(nn, nidy, WDepth);
    
  }
}
else{
  nidx = PCSGetNODValueIndex("HEAD",timelevel);
  nidy = PCSGetNODValueIndex("WDEPTH",timelevel);

  for (i=0;i<NodeListLength;i++) {
    if (GetNode(i)!=NULL) {  /* wenn Knoten existiert */
      WDepth = GetNodeVal(i,nidx) - GetNodeZ(i);
      if (WDepth < 0.0) {
        WDepth  = 0.0;
      }
      SetNodeVal(i,nidy,WDepth);
    }
  }
}
}


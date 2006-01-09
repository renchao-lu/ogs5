/**************************************************************************/
/* ROCKFLOW - Modul: cgs_MTM2.c
 */
/* Aufgabe:
   Aufstellen des Gesamtgleichungssystems und Berechnen der
   Elementmatrizen etc. fuer das Modul MTM2.

 */
/* Programmaenderungen:
   09/1994     MSR/hh        Erste Version
   03/1995     cb            SUPG
   05/1996     cb            3D
   08/1996     cb            MTranspoMat
   01.07.1997  R.Kaiser      Korrekturen und Aenderungen aus dem MTM2
   uebertragen
   16.07.1997  R.Kaiser      Interpol_q_XD -> adaptiv.c
   25.07.1997  R.Kaiser      CalcIrrNodes_ASM umbenannt zu
   CalcIrrNodeVals (erweitert) -> adaptiv.c
   25.07.1997  R.Kaiser      Adaptivitaet aus dem Modul herausgenommen
   (DelIrrNodesXD -> adaptiv.c)
   14.06.1999  OK            MakeGS_MTM2_NEW ! unterschiedlich
   10/1999     CT            Zugriff auf Saettigung wiederhergestellt (!)
   11/1999     RK            EliminateIrrNodes()
   12/1999     C.Thorenz     Irr. Knoten _vor_ Dirichlet-BC
   02/2000     C.Thorenz     Verallgemeinert, Dimensionen zusammengefasst,
   Porositaets- und Saettigungsaenderung als eigene
   Matrix
   7/2000     C.Thorenz     Bedingte RB an Zuflussraendern
   10/2004    SB			Angepasst an MTM2 und an neue Class Struktir in GS4
 */
/**************************************************************************/

#include "stdafx.h"                    /* MFC */
#include <iostream>

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTCGS_MTM2
/* Intern benutzte Module/Objekte */
#include "int_mtm2.h"
#include "elements.h"
#include "nodes.h"
#include "mathlib.h"
#include "matrix.h"
#include "edges.h"
#include "plains.h"
#include "femlib.h"
#include "adaptiv.h"
#include "rf_pcs.h"                    /* TODO */
#include "rfmat_cp.h"
#include "rfmat_cp.h"
#include "rf_tim_new.h"
#include "rf_mfp_new.h"

/* Tools */
#include "tools.h"

#include "cel_mtm2.h"
#include "rf_pcs.h"
#include "rf_react.h"
#include "rf_pcs.h" //OK_MOD" //SB: adaption_flag

/* Definitionen */
double MTM2BoundaryConditionsFunction(long knoten, int *gueltig, double val);
double MTM2SourceSinkFunction(long knoten, int *gueltig, double val);
void MakeGSxD_MTM2(long index, int component, double *rechts, double theta, CRFProcess *m_pcs);



 static int activ_phase = -1;
 static double MTM2_c_max_min;

/**************************************************************************/
/* ROCKFLOW - Funktion: MakeStatMat_MTM2
 */
/* Aufgabe:
   alle stationaeren Matrizen etc. berechnen (vor Verfeinerung)
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   10/1994     MSR        Erste Version
   01/1996     cb         MakeMat
   08/1996     cb         StMTM2at
 */
/**************************************************************************/
void MakeStatMat_MTM2( CRFProcess *m_pcs)
{
  static long i, phase=-1;
  long component = m_pcs->GetProcessComponentNumber();
	
  /* Security checks for component number and phase number */
  /* get transport phase of component */
//  if((component < 0) || (component > GetRFProcessNumComponents()-1)){
//	DisplayMsgLn(" Error - Component not available for Transport in MTM2 ");
//	exit(0);
//  }
  /* get phase in which component is transported */
  phase = cp_vec[component]->transport_phase;
  if((phase < 0) || (phase > GetRFProcessNumPhases()-1)){
	DisplayMsgLn(" Error - Transport Phase not available in MTM2 ");
	exit(0);
  }
  //----------------------------------------------------------------------
  // Calc matrices
  for(i=0;i<ElListSize();i++){
    if(ElGetElement(i)!=NULL){ // Element existiert
      if(ElGetElementActiveState(i)){ // aktives Element 
	    CECalcEleMatrix_MTM2_NEW(i,component,m_pcs); 
      }
    }
  }
i=i;
}



/**************************************************************************
   ROCKFLOW - Funktion: MakeGSxD_MTM2

   Aufgabe:
    Gleichungssystem-Eintraege der Matrizen

   Formalparameter:

   Ergebnis:
   - void -

   Programmaenderungen:
   05/1996     cb            erste Version
   25.07.1997  R.Kaiser      Adaptivitaet herausgenommen
   02/2000     C.Thorenz     Verallgemeinert, Dimensionen zusammengefasst,
                             Porositaets- und Saettigungsaenderung.
   09/2000     C.Thorenz     Mehrphasen, kuenstlich Diffusion

 **************************************************************************/
void MakeGSxD_MTM2(long index, int component, double *rechts, double theta, CRFProcess *m_pcs)
{
  double *mass = NULL, *adv = NULL, *disp = NULL, *decay=NULL;
  double edt, c_max = -1.e99, c_min = 1.e99, c_max_min, art_diff = 1., retard = 1.0;
  long nn;
  int i;
  int phase;
  static double left_matrix[64];
  static double right_matrix[64];
  static double explizit[8]; //old results
  static double right_vector[8];
  static double rho = 0;  
  //  static long type = 2;
//  static double porosity;
  double lambda = 0.0;
  double gp[3];
//SB:2p #ifdef CONTENT_CHANGE
  double *content = NULL;
//SB:2p #endif
  static long *element_nodes;
  int mtm_pcs_number = m_pcs->pcs_number;
  
  CompProperties *m_cp = cp_vec[component];
  phase = m_cp->transport_phase;
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];

#ifdef TESTTAYLOR
  theta = 0.5;
#endif

  gp[0]=gp[1]=gp[2]=0.0; // Gauss Punkt ??

  CFluidProperties *m_mfp = NULL;
  m_mfp = mfp_vector[phase];
  rho = m_mfp->Density();
 

  /* SB: Retardation hier einbauen, da sich die Konzentrationen je Zeitschritt ändern */
//  retard = MTM2CalcElementRetardationFactor(index,component,ergebnis);
    retard = m_cp->CalcElementRetardationFactor(index,gp,theta);
	/* Zerfallsmodell - lambda holen */    
    lambda = m_cp->CalcElementDecayRate(index);

#ifdef TESTCGS_MTM2
if(index < 10){
  DisplayMsg("CGS_MTM2: ");DisplayLong((long) index); DisplayMsg(" ,  ");DisplayLong((long)mtm_pcs_number); DisplayMsgLn(", ");
  DisplayMsg(" retardadtion factor  of "); DisplayLong((long)index); DisplayMsg(". element: "); DisplayDouble(retard,0,0); DisplayMsgLn("");
  DisplayMsg(" decay factor lambda of "); DisplayLong((long)index); DisplayMsg(". element: "); DisplayDouble(lambda,0,0); DisplayMsgLn("");
  DisplayMsg(" Density rho "); DisplayDouble(rho,0,0); DisplayMsgLn("");
  DisplayMsg(" Theta "); DisplayDouble(theta,0,0); DisplayMsgLn("");
}
#endif


  /* (e)ins (d)urch d(t), dt=aktueller Zeitschritt */
	/* SB: transport zeitschritte */
//MX  edt = 1.0 / (dt * 1.0);
  /* SB:3912 steady state transport */
//  edt = 1.0 / (1.0e+50);
    double dt_pcs=0.0;  //MX 01/05 time step for mass transport
    CTimeDiscretization *m_tim = NULL;
    m_pcs = PCSGet("MASS_TRANSPORT");
    if(m_pcs){
      m_tim = TIMGet("MASS_TRANSPORT");
      if(m_tim){
        dt_pcs = m_tim->time_step_vector[0];
      }
      else{
        cout << "Error in LOPTimeLoop_PCS: no time discretization" << endl;
      }
    }
  edt = 1.0 / dt_pcs;

  /* Initialisierung */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(explizit, nn);
  MNulleVec(right_vector, nn);

  /* Randbedingungen und Ergebnisse aus letztem Iterationsschritt uebertragen */
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for (i = 0; i < nn; i++){
//    explizit[i] = ergebnis[GetNodeIndex(ElGetElementNodes(index)[i])];
	explizit[i] = PCSGetNODConcentration(element_nodes[i],component,0);
#ifdef TESTCGS_MTM2
if(index < 10){
	DisplayMsg(" explizit[ "); DisplayLong((long)i); DisplayMsg("]: "); 
	DisplayDouble(explizit[i],0,0); DisplayMsgLn(" ");
}
#endif
  }


  /* Kuenstliche Diffusion? */
  if (GetArtificialDiffusion_MTM2())
    {
      for (i = 0; i < nn; i++)
        {
          c_max = MMax(c_max, explizit[i]);
          c_min = MMin(c_min, explizit[i]);
        }
      c_max_min = c_max - c_min;

      if (GetArtificialDiffusionParam_MTM2()[0] < 0.)
        {
          /* Relative Werte */
          c_max_min /= MTM2_c_max_min;
        }
      if (c_max_min > fabs(GetArtificialDiffusionParam_MTM2()[0]))
        art_diff = pow(GetArtificialDiffusionParam_MTM2()[1], (c_max_min - fabs(GetArtificialDiffusionParam_MTM2()[0])) / fabs(GetArtificialDiffusionParam_MTM2()[0] + MKleinsteZahl));
    }
  /* Matrizen zusammenbauen */

  /* Massenmatrix nS*dc/dt */
    mass = MTM2GetElementMassMatrixNew(index, mtm_pcs_number);

  if (mass)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += edt * mass[i] * rho * retard;
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += edt * mass[i] * rho * retard;
    }
#ifdef TESTCGS_MTM2
if(index < 10){
  DisplayMsgLn(" Left matrix after mass matrix inclusion: ");
  MZeigMat(left_matrix,  nn, nn, " left_matrix matrix");
//  MZeigMat(right_matrix, nn, 1, " right_vector");
	}
#endif

  /* Fluidgehaltmatrix c*d(nS)/dt */
    content = MTM2GetElementFluidContentMatrixNew(index, mtm_pcs_number); 
  if (content)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * edt * content[i] * retard;
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1. - theta) * edt * content[i] * retard ;
    }


  /* Advektionsmatrix */
    adv = MTM2GetElementAdvMatrixNew(index, mtm_pcs_number);
  if (adv)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * adv[i] * rho;
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1.0 - theta) * adv[i] * rho;
    }
#ifdef TESTCGS_MTM2
if(index < 10){
  DisplayMsgLn(" Left matrix after advection matrix inclusion: ");
  MZeigMat(left_matrix,  nn, nn, " left_matrix matrix");
  MZeigMat(adv,  nn, nn, " advection matrix");
//  MZeigMat(right_matrix, nn, 1, " right_vector");
}
#endif


  /* Decaymatrix */
    decay = MTM2GetElementDecayMatrixNew(index, mtm_pcs_number);
  if (decay)
    {
      /* Linke Seite (Matrix fuer impliziten Anteil) */
      for (i = 0; i < (nn * nn); i++)
        left_matrix[i] += theta * decay[i] * rho * lambda;
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1.0 - theta) * decay[i] * rho * lambda;
    }
#ifdef TESTCGS_MTM2
if(index < 10){
  DisplayMsgLn(" Left matrix after decay matrix inclusion: ");
  MZeigMat(left_matrix,  nn, nn, " left_matrix matrix");
//  MZeigMat(right_matrix, nn, 1, " right_vector");
}
#endif
 

  /* Diffusionsmatrix */
    disp = MTM2GetElementDispMatrixNew(index, mtm_pcs_number);
  if (disp)
    {
#ifdef TESTCGS_MTM2
		  if(index < 10) MZeigMat(disp,  nn, nn, " dispersion matrix");
#endif
      /* Linke Seite (Matrix fuer impliziten Anteil) */
	for (i = 0; i < (nn * nn); i++){
#ifdef TESTCGS_MTM2
		if(index < 10){
			DisplayMsg(" Left matrix alt [ ");DisplayLong((long)i);DisplayMsg(" ] : ");
			DisplayDouble(left_matrix[i],0,0); DisplayMsgLn(" ,  ");
		}
#endif
        left_matrix[i] += theta * disp[i] * art_diff * rho ;
#ifdef TESTCGS_MTM2
			if(index < 10){
				DisplayMsg(" Left matrix neu [ ");DisplayLong((long)i);DisplayMsg(" ] : ");
				DisplayDouble(left_matrix[i],0,0); DisplayMsg(" ,  ");
				DisplayDouble(theta,0,0); DisplayMsg(" ,  ");
				DisplayDouble(disp[i],0,0); DisplayMsg(" ,  ");
				DisplayDouble(art_diff,0,0); DisplayMsg(" ,  ");
				DisplayDouble(rho,0,0); DisplayMsgLn(" ,  ");
			}
#endif
		}
      /* Rechte Seite (Matrix fuer expliziten Anteil)  */
      for (i = 0; i < (nn * nn); i++)
        right_matrix[i] += -(1.0 - theta) * disp[i] * art_diff * rho ;
    }
#ifdef TESTCGS_MTM2
if(index < 10){
  DisplayMsgLn(" Left matrix after dispersion matrix inclusion: ");
  MZeigMat(left_matrix,  nn, nn, " left_matrix matrix");
//  MZeigMat(right_matrix, nn, 1, " right_vector");
}
#endif

  /* Negativ-Diffusion fuer Taylor-Galerkin, entfernt ab Version 3.4.23 */

  /* Rechte-Seite-Vektor aus explizitem Anteil ermitteln */
  MMultMatVec(right_matrix, nn, nn, explizit, nn, right_vector, nn);

  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index, left_matrix, right_vector, rechts);


#ifdef TESTCGS_MTM2
if(index < 10){
  MZeigMat(left_matrix,  nn, nn, " left_matrix matrix");
  MZeigMat(right_matrix, nn, 1, " right_vector");
}
#endif

}



/**************************************************************************
   ROCKFLOW - Funktion: MakeGS_MTM2_OLD

   Aufgabe:
   Gleichungssystem aufstellen

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R double *links: linke Seite Gleichungssystem (Matrix)
                    (nur alte Loeser !!!)
   R double *rechts: rechte Seite GS (Vektor)
   R double *ergebnis: Ergebnis des GS, bei Rueckgabe Nullvektor

   Ergebnis:
   - void -

   Programmaenderungen:
   10/1994     hh/MSR        Erste Version
   03/1995     cb            SUPG
   03/1995     MSR           Randbedingungs-Zeitkurven eingebaut
   08/1995     cb            irregulaere Knoten eliminieren
   11/1995     msr           alte und neue Loeser
   04/1996     cb            RB nur an Zuflussknoten
    6/1999     OK            ohne Namen-String
    7/2000     CT            Bedingte RB am Zuflussrand
   09/2000     C.Thorenz     Mehrphasen
   02/2004     SB            Angepasst an MTM2

 **************************************************************************/
void MakeGS_MTM2_old(double *rechts, double *ergebnis, int component, CRFProcess *m_pcs)
{
  /* Variablen */
  long i, index, elanz, anz_phasen, phase;
//  char name[80];
  double c_max = -1.e99, c_min = 1.e99;
  double theta = 0; 
  double *residuum;
  residuum = (double *) Malloc(NodeListLength * sizeof(double));

  theta = GetNumericalTimeCollocation("TRANSPORT");
  anz_phasen = GetRFProcessNumPhases(); //SB??
  //activ_phase = phase;
  /* get phase by component name */
  phase = cp_vec[component]->transport_phase;
  // activ_component = component;


  /* Initialisieren der Felder */
  MXInitMatrix();
  MNulleVec(rechts, NodeListLength);
  
/* SB: take out for pcs, da es reingegeben wird */
  for (i = 0l; i < NodeListLength; i++) {
      c_max = MMax(c_max, ergebnis[i]);
      c_min = MMin(c_min, ergebnis[i]);
    }  
  /* Groesste Spanne im System, fuer eventuelle Glaettung */
  MTM2_c_max_min = c_max - c_min;


  /* Linke und rechte Seite aufbauen aus Elementmatrizen aller Elemente */
  elanz = ElListSize();
  for (index = 0; index < elanz; index++)
    {
      if (ElGetElement(index) != NULL)
        {                              /* wenn Element existiert */
          if (ElGetElementActiveState(index))
            {                          /* nur aktive Elemente */
              MakeGSxD_MTM2(index, component, rechts, theta, m_pcs);
            }                          /* if = nur aktive Elemente */
        }                              /* if = Element existiert */
    }                                  /* for = Schleife ueber die Elemente */

#ifdef TESTCGS_MTM2
  DisplayMsgLn("MakeGS vor Punktquellen");
#endif

  MXResiduum(ergebnis, rechts, residuum);


  /* SB: hier ist die Stelle, um die Quellterme aufgrund der Chemie einzutragen */
// MTM2IncorporateSoureSinkChemistry(rechts,.....)

  /* Punktquellen einarbeiten */
//  sprintf(name, "%s%i", name_source_mass_tracer_component, component * anz_phasen + phase + 1);
//  IncorporateSourceSinkEx(name, rechts, MTM2SourceSinkFunction); //SB:todo wird zweimal gemacht, hier und beim solver

//  MXResiduum(ergebnis, rechts, residuum);

/*  if (adaptive_mesh_refinement_flag)  SB: ersetzt durch naechste Zeile */
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

  MXResiduum(ergebnis, rechts, residuum);


  /* Dirichlet-Randbedingungen eintragen */
//  sprintf(name, "%s%d", name_boundary_condition_tracer_component, component * anz_phasen + phase + 1);
//  IncorporateBoundaryConditions(name, rechts);  //SB:todo wird zweimal gemacht, hier und beim solver

//  MXResiduum(ergebnis, rechts, residuum);

  /* Bedingte Dirichlet-Randbedingungen eintragen */
//  sprintf(name, "%s%i", name_boundary_condition_conditional_tracer_component, component * anz_phasen + phase + 1);
//  IncorporateBoundaryConditionsEx2(name, rechts, MTM2BoundaryConditionsFunction); //SB:todo wird zweimal gemacht, hier und beim solver

//  MXResiduum(ergebnis, rechts, residuum);

  residuum = (double *) Free(residuum);

}




void MakeGS_MTM2(double *rechts,  CRFProcess *m_pcs)
{
  /* Variablen */
  long index, elanz;
  long component;
  double theta = 0; 

  theta = m_pcs->m_num->ls_theta; //OK

  /* get component and phase by component name */
  component = m_pcs->GetProcessComponentNumber();
  
  /* Linke und rechte Seite aufbauen aus Elementmatrizen aller Elemente */
  elanz = ElListSize();
  for (index = 0; index < elanz; index++){
      if (ElGetElement(index) != NULL){			/* wenn Element existiert */
          if (ElGetElementActiveState(index)){  /* nur aktive Elemente */
              MakeGSxD_MTM2(index, component, rechts, theta, m_pcs);
            }			                          /* if = nur aktive Elemente */
        }							              /* if = Element existiert */
    }											  /* for = Schleife ueber die Elemente */

#ifdef TESTCGS_MTM2
  DisplayMsgLn("MakeGS vor Punktquellen");
#endif


/*  if (adaptive_mesh_refinement_flag)  SB: ersetzt durch naechste Zeile */
  if (GetRFControlGridAdapt())
    if (AdaptGetMethodIrrNodes() == 2)
      EliminateIrrNodes(rechts);

}



/**************************************************************************
   ROCKFLOW - Funktion:  MTM2SourceSinkFunction

   Aufgabe:
   Ermoeglicht Fluidzufluesse mit 0-Konzentration

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   7/2000 C.Thorenz Erste Version
   3/2001 C.Thorenz Behandlung der Raender verbessert

 **************************************************************************/
double MTM2SourceSinkFunction(long knoten, int *gueltig, double val)
{
  double flux = 0.;

  *gueltig = 1;

  if (MTM2GetNodeFlux)
    {
//      flux = (1.-GetTimeCollocationSource_MTM2()) * MTM2GetNodeFlux(activ_phase, knoten, -1, 0) +
//             (GetTimeCollocationSource_MTM2()) * MTM2GetNodeFlux(activ_phase, knoten, -1, 1);

      if (flux > 0.)
        {
/*
          conc = (1.-GetTimeCollocationSource_MTM2()) * MTM2GetNodeConc(knoten, activ_phase, activ_component, 0) +
                 (GetTimeCollocationSource_MTM2()) * MTM2GetNodeConc(knoten, activ_phase, activ_component, 1); 
          val -= conc * flux;
*/
          /* Besser: Implizit im GLS */  
          MXInc(GetNodeIndex(knoten),GetNodeIndex(knoten),flux);

        }
    }
  return val;
}

/**************************************************************************
   ROCKFLOW - Funktion:  MTM2BoundaryConditionsFunction

   Aufgabe:
   Bedingte RB: Nur bei Zufluss angeschaltet

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
     E long knoten : Knotennummer eines Knotens mit RB
     R int *gueltig: Rueckgabe, ob RB eingebaut werden soll
     E double val: Geplanter Wert der RB

   Ergebnis:
     double val : Neuer Wert fuer RB

   Programmaenderungen:
   7/2000 C.Thorenz Erste Version

 **************************************************************************/

double MTM2BoundaryConditionsFunction(long knoten, int *gueltig, double val)
{
  double flux = -1.;

  *gueltig = 0;

  if (MTM2GetNodeFlux)
    {
      flux = MTM2GetNodeFlux(activ_phase, knoten, -1, 0);
      if (flux > MKleinsteZahl)
        {
          *gueltig = 1;
        }
    }
  return val;
}










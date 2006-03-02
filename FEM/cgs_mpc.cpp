/**************************************************************************
 ROCKFLOW - Modul: cgs_mpc.c

 Task:
 Make the equation system (EQS) for Kernel MPC

 Programming:
 01/2002 OK Implementation
 03/2003 RK model substituted by process control parameters
 05/2003 OK MPCAssembleSystemMatrixDXDXD
  

**************************************************************************/
#include "stdafx.h"                    /* MFC */
#include <iostream>
#include "makros.h"
#include "elements.h"
#include "mathlib.h"
#include "femlib.h" /* IncorporateMatrix -> matrix ? */
#include "rf_pcs.h" //OK_MOD"
#include "matrix.h"
#include "nodes.h"
#include "int_mmp.h"
#include "rf_pcs.h"
#include "rf_tim_new.h"
#include "rf_bc_new.h"
#include "rf_mfp_new.h"
#include "rf_mmp_new.h"
#include "cel_mpc.h"
extern void MPCCalculateElementMatrix(long index,int phase,int component);

void MPCAssembleSystemMatrix(int phase,double *rechts);
void MPCMakeElementEntryP(int comp,long ActiveElements,double *rechts);
void MPCMakeElementEntryS(int comp,long ActiveElements,double *rechts);
void MPCMakeElementEntryS2(int comp,long ActiveElements,double *rechts);
void MPCMakeElementEntryV1(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryV2(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryP1(int comp,long ActiveElements,double *rechts);
void MPCMakeElementEntryS3(int comp,long ActiveElements,double *rechts);
void MPCMakeElementEntryS4(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryV5(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryV7(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryS1(int comp,long index,double *ls_rhs);
void MPCMakeElementEntryP2(int comp,long ActiveElements,double *rechts);

long saturation_vector_pos;
long MPCGetSaturationVectorPosition(void) 
{
  return NodeListLength;
}
long pressure_vector_pos;
long MPCGetPressureVectorPosition(void) 
{
  return 0;
}

#define noTEST_CGS_MPC
#define noTEST_CGS_MPC1
#define noTEST_CGS_MPC_V2
#define noTEST_CGS_MPC_V3
#define noTEST_CGS_MPC_V5
#define noTEST_CGS_MPC_V7

/**************************************************************************
 ROCKFLOW - Funktion: MPCAssembleSystemMatrix

 Task:
 Make the equation system (EQS)

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 11/2001   OK/JdJ

**************************************************************************/
void MPCAssembleSystemMatrix(int comp,double *rechts) 
{
  static long i;
  int no_phases = (int)mfp_vector.size();
  for (i=0;i<ElListSize();i++) {
    if (comp==0) {
      if(no_phases==1) // if (GetRFProcessFlowModel()==11) 
        MPCMakeElementEntryP2(comp,i,rechts); 
      if((no_phases==2)&&(!pcs_monolithic_flow)) // if (GetRFProcessFlowModel()==1100)
        MPCMakeElementEntryP1(comp,i,rechts); // if (GetRFProcessFlowModel()==13000) 
      if((no_phases==2)&&(pcs_monolithic_flow))
        MPCMakeElementEntryV7(comp,i,rechts);
    }
    if (comp==1) {
      if(no_phases==1) // if (GetRFProcessFlowModel()==11) 
        MPCMakeElementEntryP2(comp,i,rechts); 
      if((no_phases==2)&&(!pcs_monolithic_flow)) // if (GetRFProcessFlowModel()==1100)
        MPCMakeElementEntryS1(comp,i,rechts);
      if((no_phases==2)&&(pcs_monolithic_flow)) // if (GetRFProcessFlowModel()==13000) 
        MPCMakeElementEntryV7(comp,i,rechts);
    }
  }
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryP

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Pressure

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 11/2001   OK/JdJ

**************************************************************************/
void MPCMakeElementEntryP(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];

  int timelevel;
  int phase;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  //OK capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
#ifdef TEST_CGS_MPC
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 1.0; /*GetTimeCollocationGlobal_HTM(); erstmal voll implizit */


/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Conductivity matrix: theta*(K_k^gs + K_k^ls) */
  for (i=0;i<nn2;i++) {
        left_matrix[i] = theta * (conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
  }
#ifdef TEST_CGS_MPC
  MZeigMat(left_matrix, nn, nn, "left_matrix");
#endif


/*---------------------------------------------------------------------------*/
/* RHS parts */
  /*-------------------------------------------------------------------------*/
  /* Conductivity matrix: (1-theta)*(K_k^gs + K_k^ls)[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = (1.0-theta)*(conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Conductivity matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix - new time : - 1/dt ( C_a^g - C_a^l ) [S^g]_t^n+1 */
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_gas[i] - capacitance_matrix_liquid[i];
   /* [S^g]^n+1 */
  phase=0;
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capacitance matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /* Capacitance matrix - old time : + 1/dt ( C_a^g - C_a^l ) [S^g]_t^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_gas[i] - capacitance_matrix_liquid[i];
   /* [S^g]^n */
  phase=0;
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += edt*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capacitance matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* V1 Capillarity matrix: theta CP_a^ls [S^g]_t^n+1 + (1-theta) CP_a^ls [S^g]^n */
  /* V2 Capillarity matrix: theta K_k^*s [p_c]_t^n+1 + (1-theta) K_k^*s [p_c]^n */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p_c]^n+1, [p_c]^n */
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodeCapillaryPressure(element_nodes[i],timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capillarity matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeCapillaryPressure(element_nodes[i],timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1.0-theta)*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capillarity matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta K_a^l [X_a^g]_t^n+1 + (1-theta) K_a^l [X_a^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = theta*diffusion_matrix_gas[i];
   /* [X^g]^n */
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
   /* [X^g]^n+1 */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1-theta)*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/
/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);

#ifdef TEST_CGS_MPC
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
/* MZeigVec(ls_rhs, NodeListLength, "ls_rhs");*/
#endif
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryS

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Saturation

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 11/2001   OK/JdJ

**************************************************************************/
void MPCMakeElementEntryS(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];

  int timelevel;
  int phase;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC1
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector: init");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);

/*
#ifdef TEST_CGS_MPC
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.5; /*GetTimeCollocationGlobal_HTM();*/


/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Capacitance matrix: 1/dt*(C_k^l - C_k^g)*/
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_gas[i]-capacitance_matrix_liquid[i]);
        /*left_matrix[i] = edt * (capacitance_matrix_liquid[i] - capacitance_matrix_gas[i]);*/
  }
#ifdef TEST_CGS_MPC1
  MZeigMat(left_matrix, nn, nn, "left_matrix: capacitance_matrix");
#endif

  /*-------------------------------------------------------------------------*/
  /* Capillarity matrix: theta CP_a^ls [S^g]_t^n+1 + (1-theta) CP_a^ls [S^g]^n */
   /* dpc/dS */
/*
  for (i=0;i<nn;i++) {
    timelevel=1;
    phase=1;
    u_new_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
*/
  for (i=0;i<nn2;i++)
    left_matrix[i] -= -5e+5 * conductance_matrix_liquid[i];
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
#ifdef TEST_CGS_MPC1
  MZeigMat(left_matrix, nn, nn, "left_matrix: capillarity_matrix_liquid");
#endif


/*---------------------------------------------------------------------------*/
/* RHS parts */
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix: 1/dt*(C_k^g) * 1 */
/*
  for (i=0;i<nn2;i++) {
        right_matrix[i] = edt * capacitance_matrix_gas[i];
  }
*/
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix: + 1/dt ( C_w^g - C_w^l ) [S^g]^n) */
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_gas[i] - capacitance_matrix_liquid[i]);
   /* [S^g]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Capacitance matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity matrix: theta*(K_k^gs + K_k^ls)[p^g]^n+1 */
  for (i=0;i<nn2;i++)
    right_matrix[i] = (theta)*(conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
   /* [p^g]^n */
  phase=0;
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Conductivity matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /* Conductivity matrix: (1-theta)*(K_k^gs + K_k^ls)[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = (1.0-theta)*(conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
  phase=0;
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Conductivity matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta * K_w^g [X_w^g]_t^n+1 */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  comp=1;
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /* Diffusion matrix: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1-theta)*right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif

/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);

#ifdef TEST_CGS_MPC1
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
  /*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryV1

 Task:
 Make the equation system (EQS)
 Monolithic scheme - V1

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 11/2001   OK/JdJ

**************************************************************************/
void MPCMakeElementEntryV1(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static long global_node_number[8];
  int timelevel;
  int phase;
  double saturation[2];
  double dpc_dS[2]={1.,1.};
  double gp[3]={0.,0.,0.};
//  int phasen=2;
  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(right_vector, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);

#ifdef TEST_CGS_MPC
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 1.0; /*GetTimeCollocationGlobal_HTM(); erstmal voll implizit */


  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();


/*===========================================================================*/
/* Pressure matrix */
/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Conductivity matrix: theta*(K_k^gs + K_k^ls) */
  for (i=0;i<nn2;i++)
        left_matrix[i] = theta * (conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
#ifdef TEST_CGS_MPC
  MZeigMat(left_matrix,nn,nn,"left_matrix: Conductivity matrix - new time");
#endif
  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }

/*===========================================================================*/
/* Saturation matrix */
/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix, gas phase: 1/dt ( C_k^g - C_k^l): [S^g] */
  for (i=0;i<nn2;i++)
    left_matrix[i] = edt*(capacitance_matrix_gas[i]-capacitance_matrix_liquid[i]);
#ifdef TEST_CGS_MPC
  MZeigMat(left_matrix,nn,nn,"left_matrix: Capacitance matrix - new time");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capillarity matrix: theta K_w^ls dpc/dS [S^g]_t^n+1 */
   /* dpc/dS */
   /*besser MRange*/
    //OK saturation[0] = MMax(0., MPCGetSaturation(0,index,0.,0.,0.,theta));
    saturation[0] = MMax(0.,PCSGetELEValue(index,gp,theta,"SATURATION1"));
    //OK saturation[1] = MMax(0., MPCGetSaturation(1,index,0.,0.,0.,theta));
    saturation[1] = MMax(0.,PCSGetELEValue(index,gp,theta,"SATURATION2"));
    //OK CECalcCapSatuDerivative(index,phasen,saturation,dpc_dS);
  /*if(comp==1) {*/
  for (i=0;i<nn2;i++)
    left_matrix[i] -= dpc_dS[1] * conductance_matrix_liquid[i];
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
#ifdef TEST_CGS_MPC1
  MZeigMat(left_matrix, nn, nn, "left_matrix: - Capillarity_matrix_liquid");
#endif
/*}*/
  /*-------------------------------------------------------------------------*/
  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }

/*---------------------------------------------------------------------------*/
/* RHS parts */
  /*-------------------------------------------------------------------------*/
  /* Conductivity matrix: (1-theta)*(K_k^gs + K_k^ls)[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = (1.0-theta)*(conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Conductivity matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix, gas phase: 1/dt (C_a^g-C_a^l) [S^g]^n */
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt*(capacitance_matrix_gas[i] - capacitance_matrix_liquid[i]);
   /* [S^g]^n */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capacitance matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capillarity matrix: theta CP_a^ls [S^g]_t^n+1 + (1-theta) CP_a^ls [S^g]^n */
  /* Capillarity matrix: theta K_a^ls [pc]_t^n+1 + (1-theta) K_a^ls [pc]^n */
/*
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
[p_c]^n+1, [p_c]^n
  phase=1;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capillarity matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1.0-theta)*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Capillarity matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
*/
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta K_a^l [X_a^g]_t^n+1 + (1-theta) K_a^l [X_a^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = theta*diffusion_matrix_gas[i];
   /* [X_k^g]^n+1, [X_k^g]^n */
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1.0-theta)*right_vector[i];
#ifdef TEST_CGS_MPC
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
  for (i=0;i<nn;i++)
    if(comp==0) {
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
    if(comp==1) {
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector[i];
    }

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  /*OKIncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);*/

#ifdef TEST_CGS_MPC
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
/*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs");*/
#endif
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryV2

 Task:
 Make the equation system (EQS)
 Monolithic scheme - V2

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK

**************************************************************************/
void MPCMakeElementEntryV2(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static long global_node_number[8];
  int timelevel;
  int phase;
  double saturation[2];
  double dpc_dS[2]={1.,1.};
//  int phasen=2;
  double drho_dp_g, drho_dp_l;
  double gp[3]={0.,0.,0.};
  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(right_vector, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V2
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);

#ifdef TEST_CGS_MPC_V2
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
  DisplayMsgLn("---------------------------------------------------------------");
#endif
/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 1.0; /* GetTimeCollocationGlobal_HTM(); erstmal voll implizit */
  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();
/*===========================================================================*/
/* LHS matrix */
/*---------------------------------------------------------------------------*/
/* Pressure matrix */
  /* Conductivity matrix: theta*(K_k^gs + K_k^ls) */
  for (i=0;i<nn2;i++)
        left_matrix[i] = theta * (conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
#ifdef TEST_CGS_MPC_V2
  MZeigMat(left_matrix,nn,nn,"[p^(n+1)]: theta * (K_k^gs + K_k^ls)");
#endif
  /* Capacitance matrix: 1/dt ( C_k^g drho^g/dp^g + C_k^l drho^l/dp^l) */
  phase=0;
//SB:m  drho_dp_g = GetFluidDensityPressureDependency(phase);
  drho_dp_g = mfp_vector[phase]->drho_dp;
  phase=1;
//SB:m  drho_dp_l = GetFluidDensityPressureDependency(phase);
  drho_dp_l = mfp_vector[phase]->drho_dp;
  for (i=0;i<nn2;i++)
    left_matrix[i] += edt * ( capacitance_matrix_gas[i]    * drho_dp_g \
                             +capacitance_matrix_liquid[i] * drho_dp_l );
#ifdef TEST_CGS_MPC_V2
  MZeigMat(left_matrix,nn,nn,"[p^(n+1)]: 1/dt ( C_k^g drho^g/dp^g + C_k^l drho^l/dp^l)");
#endif
  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }

/*---------------------------------------------------------------------------*/
/* Saturation matrix */
    /* dpc/dS */
    //OK saturation[0] = MMax(0., MPCGetSaturation(0,index,0.,0.,0.,theta));
    saturation[0] = MMax(0.,PCSGetELEValue(index,gp,theta,"SATURATION1"));
    //OK saturation[1] = MMax(0., MPCGetSaturation(1,index,0.,0.,0.,theta));
    saturation[1] = MMax(0.,PCSGetELEValue(index,gp,theta,"SATURATION2"));
    //OK CECalcCapSatuDerivative(index,phasen,saturation,dpc_dS);
  /*-------------------------------------------------------------------------*/
  /* Capacitance matrix: 1/dt ( C_k^g - C_k^l + CP_k^l dpc/dS^l): [S^g]^(n+1) */
  for (i=0;i<nn2;i++)
    left_matrix[i] =  edt * ( capacitance_matrix_gas[i] \
                             -capacitance_matrix_liquid[i] \
                             +capacitance_matrix_liquid[i] * drho_dp_l * dpc_dS[1] \
                            );
#ifdef TEST_CGS_MPC_V2
  MZeigMat(left_matrix,nn,nn,"[S^g]^(n+1): 1/dt ( C_k^g - C_k^l + CP_k^l dpc/dS^l)");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capillarity matrix: theta K_k^ls dpc/dS: [S^g]^(n+1) */
  for (i=0;i<nn2;i++)
  /* Vorzeichen + ? */
    left_matrix[i] -= theta * conductance_matrix_liquid[i] * dpc_dS[1];
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
#ifdef TEST_CGS_MPC_V2
  MZeigMat(left_matrix, nn, nn, "[S^g]^(n+1): theta K_k^ls dpc/dS");
#endif
  /*-------------------------------------------------------------------------*/
  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }

/*===========================================================================*/
/* RHS parts */
  /*-------------------------------------------------------------------------*/
  /* Pressure part */
  /* Capacitance matrix: 1/dt ( C_k^g drho^g/dp^g + C_k^l drho^l/dp^l) [p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * ( capacitance_matrix_gas[i]    * drho_dp_g \
                             +capacitance_matrix_liquid[i] * drho_dp_l );
#ifdef TEST_CGS_MPC_V2
  MZeigMat(right_matrix,nn,nn,"[p^g]^n: 1/dt ( C_k^g drho^g/dp^g + C_k^l drho^l/dp^l)");
#endif
  /* Conductivity matrix: (1-theta) * (K_k^gs + K_k^ls) [p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] -= (1.0-theta) * ( conductance_matrix_gas[i] \
                                     +conductance_matrix_liquid[i] );
#ifdef TEST_CGS_MPC_V2
  MZeigMat(right_matrix,nn,nn,"[p^g]^n: (1-theta) * (K_k^gs + K_k^ls)");
#endif
  /* [p^g]^n */
  phase=0;
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] = right_vector[i];
#ifdef TEST_CGS_MPC_V2
  MZeigVec(right_vector,nn,"right_vector: Pressure part - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Saturation part */
  /* Capacitance matrix: 1/dt ( C_k^g - C_k^l + CP_k^l dpc/dS^l): [S^g]^(n) */
  phase=0;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = edt * ( capacitance_matrix_gas[i] \
                             -capacitance_matrix_liquid[i] \
                             +capacitance_matrix_liquid[i] * drho_dp_l * dpc_dS[1] \
                            );
  }
#ifdef TEST_CGS_MPC_V2
  MZeigMat(right_matrix,nn,nn,"right_matrix: Capacitance matrix - old time");
#endif
  /* Capillarity matrix: [S^g]^(n): theta K_k^ls dpc/dS */
  for (i=0;i<nn2;i++)
    right_matrix[i] -= (1.0-theta) * conductance_matrix_liquid[i] * dpc_dS[1];
#ifdef TEST_CGS_MPC_V2
  MZeigMat(right_matrix,nn,nn,"[S^g]^(n): theta K_k^ls dpc/dS");
#endif
  /* [S^g]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V2
  MZeigVec(right_vector,nn,"right_vector: Saturation part - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta K_a^l [X_a^g]_t^n+1 + (1-theta) K_a^l [X_a^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = theta*diffusion_matrix_gas[i];
   /* [X_k^g]^n+1, [X_k^g]^n */
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1.0-theta)*right_vector[i];
#ifdef TEST_CGS_MPC_V2
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
  for (i=0;i<nn;i++)
  {
    if(comp==0) {
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
    if(comp==1) {
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
  }
/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  /* OK IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs); */

#ifdef TEST_CGS_MPC_V2
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
/* MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}

/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryP1

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Pressure equation
 (17.42)

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK
 07/2002   OK
 03/2004   WW Coupling matrix for triangle element

**************************************************************************/
void MPCMakeElementEntryP1(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  double *capacitance_matrix_gas_P = NULL;
  double *capacitance_matrix_liquid_P = NULL;
  double left_matrix_P[64];
  double *mpc_strain_coupling_matrix_x = NULL;
  double *mpc_strain_coupling_matrix_y = NULL;

  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[9];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
  double rhs_vector_P[8];
  int timelevel;
  int phase;

  static double du_x[9];
  static double du_y[9];
  static double right_vector_x[9];
  static double right_vector_y[9];

  double RHS3_factor = 1.0;

  const int EleType = ElGetElementType(index);
  int nd = 6;
  switch(EleType)
  {
     case 2: // Quadralateral element
       nd=9; 
	   break;
     case 3: // Hexehedron element 
       nd=20; 
	   break;
	 case 4:  //Triangle element
       nd = 6;
	   break;
	 default:
       nd=9;  // Quadralateral element
	   break;
  }
#define noTEST_MPC_EQS
#ifdef TEST_MPC_EQS
  double *rhs_vector_1;
  double *rhs_vector_2;
  double *rhs_vector_3;
  double *rhs_vector_4;
  double *rhs_vector_5;
  double *rhs_vector_6;
  double *rhs_vector_7;

  if(index==0) {
    rhs_vector_1 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_2 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_3 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_4 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_5 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_6 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_7 = (double*) Malloc(NodeListLength*sizeof(double));
    MNulleVec(rhs_vector_1,NodeListLength);
    MNulleVec(rhs_vector_2,NodeListLength);
    MNulleVec(rhs_vector_3,NodeListLength);
    MNulleVec(rhs_vector_4,NodeListLength);
    MNulleVec(rhs_vector_5,NodeListLength);
    MNulleVec(rhs_vector_6,NodeListLength);
    MNulleVec(rhs_vector_7,NodeListLength);
  }
#endif

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector_P, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsgLn("MPCMakeElementEntryP1");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  /*  MZeigVec(rhs_vector,nn,"rhs_vector:"); */
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
if((GetRFControlModel()==-1)) {
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
 if(GetRFProcessProcessing("SD")) {
  phase=1;
  mpc_strain_coupling_matrix_x = MPCGetELEStrainCouplingMatrixX(index,phase);
  mpc_strain_coupling_matrix_y = MPCGetELEStrainCouplingMatrixY(index,phase);
 }
}
 else {
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
 if(GetRFProcessProcessing("SD")) {
  phase=1;
  mpc_strain_coupling_matrix_x = MPCGetELEStrainCouplingMatrixX(index,phase);
  mpc_strain_coupling_matrix_y = MPCGetELEStrainCouplingMatrixY(index,phase);
 }
}
#ifdef TEST_CGS_MPC_V3
  MZeigMat(capacitance_matrix_gas,nn,nn,     "MPCMakeElementEntryP1: CS_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,  "MPCMakeElementEntryP1: CS_k^l capacitance matrix liquid");
  MZeigMat(conductance_matrix_gas,nn,nn,     "MPCMakeElementEntryP1: KP_k^g conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,  "MPCMakeElementEntryP1: KP_k^l conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas_P,nn,nn,   "MPCMakeElementEntryP1: CP_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid_P,nn,nn,"MPCMakeElementEntryP1: CP_k^l capacitance matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,       "MPCMakeElementEntryP1: D_k^g  diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,    "MPCMakeElementEntryP1: D_k^l  diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,            "MPCMakeElementEntryP1: g^g    gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,         "MPCMakeElementEntryP1: g^l    gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM(); erstmal voll implizit */
 /* theta = GetTimeCollocationGlobal_MMP();*/



/*===========================================================================*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* LHS1: Storativity term: 1/dt (CP_k^g + CP_k^l): [p^g]^n+1) */
  phase=0;
  for (i=0;i<nn2;i++) {
    left_matrix[i] = capacitance_matrix_gas_P[i] \
                   + capacitance_matrix_liquid_P[i];
    left_matrix[i] *= edt;
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS1: 1/dt (CP_k^g + CP_k^l)");
#endif
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] = left_matrix[i];
  }

  /*-------------------------------------------------------------------------*/
  /* LHS2: Conductivity term: theta (KP_k^gs + KP_k^ls): [p^g]^n+1 */
  for (i=0;i<nn2;i++) {
        left_matrix[i] = theta * (conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix,nn,nn,"LHS2: theta (KP_k^gs + KP_k^ls)");
#endif
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] += left_matrix[i];
  }

/*===========================================================================*/
/* RHS vectors */
  timelevel=0;
  /*-------------------------------------------------------------------------*/
  /* RHS1: Storativity term: + 1/dt (CP_k^g+CP_k^l) [p^g]^n */
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas_P[i] \
                    + capacitance_matrix_liquid_P[i];
  }
  phase=0;
  /* [p^g]^(n) */
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
   MZeigMat(right_matrix,nn,nn,"RHM1: CP_k^g+CP_k^l");
   MZeigVec(u_old_time,nn,"[p^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS1: + 1/dt (CP_k^g+CP_k^l) [p^g]^n");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_1[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS2: Conductivity term: -(1-theta) (K_k^gs+K_k^ls) [p^g]^n */
  for (i=0;i<nn2;i++) {
    right_matrix[i] = conductance_matrix_gas[i] \
                    + conductance_matrix_liquid[i];
  }
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= (1.0-theta);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector_P[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHM2: K_k^gs+K_k^ls");
  MZeigVec(u_old_time,nn,"[p^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS2: -(1-theta)*(K_k^gs+K_k^ls)*[p^g]^n");
  /* MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_2[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS3: Storativity term: + 1/dt (C_k^g-C_k^l) ([S^l]^n+1 - [S^l]^n) */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas[i] \
                    - capacitance_matrix_liquid[i];
  }
   /* [S^l]^n+1, [S^l]^n */
  for (i=0;i<nn;i++) {
//OK ToDo    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,1);
    u_new_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,1);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt * RHS3_factor;
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHM3: C_k^l-C_k^g");
  MZeigVec(u_theta,nn,"[S^l]^n+1 - [S^l]^n");
  */
  MZeigVec(right_vector,nn,"RHS3: + 1/dt (C_k^g-C_k^l) ([S^l]^n+1 - [S^l]^n)");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_3[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS4: Capillarity term 1: + 1/dt CP_k^l ([p_c]^n+1 - [p_c]^n) */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_liquid_P[i];
  }
   /* [p^l]^(n+1), [p^l]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"pc^n+1 - pc^n");
  */
  MZeigVec(right_vector,nn,"RHS4: + 1/dt CP_k^l (pc]^n+1 - [pc]^n)");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_4[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS5: Capillarity term 2: + KP_k^l (theta [p_c]^n+1 + (1-theta) [p_c]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p_c]^n */
//OK  timelevel=0;
  timelevel=1;
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta * u_new_time[i] + (1.-theta) * u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHS5: KP_k^l");
  MZeigVec(u_theta,nn,"(theta [p_c]^n+1 + (1-theta) [p_c]^n)");
  */
  MZeigVec(right_vector,nn,"RHS5: + KP_k^l (theta [p_c]^n+1 + (1-theta) [p_c]^n)");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_5[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS6: Diffusion term: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector_P[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHS6: K_w^g");
  MZeigVec(u_theta,nn,"theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS6: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n)");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_6[element_nodes[i]] += right_vector[i];
#endif

  /*-------------------------------------------------------------------------*/
  /* RHS7: Strain coupling term: B (S^g rho^g + S^l rho^l) 1/dt ([u^s]^n+1 - [u^s]^n) */
#ifdef PCS_OBJECTS
if(GetRFProcessProcessing("SD")) { 
   /* [u^s]^n+1 - [u^s]^n */
  for (i=0;i<nd;i++) {
     du_x[i] = -PCSGetNODValue(element_nodes[i],"DISPLACEMENT_X1",0); //OK -DMGetNodeDisplacementX0(element_nodes[i]); 
     du_y[i] = -PCSGetNODValue(element_nodes[i],"DISPLACEMENT_Y1",0); //OK -DMGetNodeDisplacementY0(element_nodes[i]);
     if(pcs_deformation%11 != 0) //WW. if not monolithic scheme. Important
     {
       du_x[i] += PCSGetNODValue(element_nodes[i],"DISPLACEMENT_X1",1); //OK DMGetNodeDisplacementX1(element_nodes[i]); 
       du_y[i] += PCSGetNODValue(element_nodes[i],"DISPLACEMENT_Y1",1); //OK DMGetNodeDisplacementY1(element_nodes[i]); 
	 }
  }
  MMultMatVec(mpc_strain_coupling_matrix_x,nn,nd,du_x,nd,right_vector_x,nn);
  MMultMatVec(mpc_strain_coupling_matrix_y,nn,nd,du_y,nd,right_vector_y,nn);

  for (i=0;i<nn;i++) {
    right_vector[i] = -(right_vector_x[i]+right_vector_y[i])*edt; 
  }

  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"RHS7: B (S^g rho^g + S^l rho^l) 1/dt ([u^s]^n+1 - [u^s]^n))");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_7[element_nodes[i]] += right_vector[i];
#endif
}
#endif

  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix_P,rhs_vector_P,ls_rhs);

#ifdef TEST_CGS_MPC_V3
  /*  MZeigMat(left_matrix, nn, nn, "left_matrix"); */
  MZeigVec(rhs_vector_P,nn, "rhs_vector_P");
  /*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
#ifdef TEST_MPC_EQS
  if(index==(ElementListLength-1)) {
    DisplayMsgLn("MPCMakeElementEntryP1: RHS Analysis");
    long i;
    printf("\n%s\n"," | 1/dt CP pg    | 1-theta KP pg | 1/dt CS dSl   | 1/dt CP^l pc  | KP^l theta pc | K theta X     |");
    printf("%s\n"," -------------------------------------------------------------------------------------------------");
    for (i=0;i<NodeListLength;i++) {
        printf(" | %e | %e | %e | %e | %e | %e | \n", \
               rhs_vector_1[i],rhs_vector_2[i],rhs_vector_3[i],\
               rhs_vector_4[i],rhs_vector_5[i],rhs_vector_6[i]);
    }
    printf("  %s\n"," -------------------------------------------------------------------------------------------------");
  }
#endif

}




/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryP2

 Task:
 Make the equation system (EQS)
 Richards model

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 07/2002   OK

**************************************************************************/
void MPCMakeElementEntryP2(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  double *capacitance_matrix_liquid_P = NULL;
  double left_matrix_P[64];

  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
  double rhs_vector_P[8];

  int timelevel;
  int phase;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector_P, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  /*  MZeigVec(rhs_vector,nn,"rhs_vector:"); */
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=1;
if((GetRFControlModel()==-1)) {
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
}
else {
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
}


/*
#ifdef TEST_CGS_MPC_V3
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: CS_k^l capacitance matrix liquid");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: KP_k^l conductivity matrix liquid");
  MZeigMat(capacitance_matrix_liquid_P,nn,nn,"MPCMakeElementEntry: CP_k^l capacitance matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,     "MPCMakeElementEntry: D_k^g  diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,  "MPCMakeElementEntry: D_k^l  diffusion matrix liquid");
  MZeigVec(gravity_vector_liquid,nn,       "MPCMakeElementEntry: g^l    gravity vector liquid");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM(); erstmal voll implizit */
  theta = GetTimeCollocationGlobal_MMP();



/*===========================================================================*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* LHS1: Storativity term: 1/dt (CP_k^l - CS_k^l dS^l/dpc): [p^l]^n+1) */
  phase=1;
  for (i=0;i<nn2;i++) {
    left_matrix[i] = capacitance_matrix_liquid_P[i] \
                   + capacitance_matrix_liquid[i] \
                   * MMPGetSaturationPressureDependency(phase,index,0.,0.,0.,theta);
    left_matrix[i] *= edt;
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS1: 1/dt (CP_k^l - CS_k^l dS^l/dpc)");
#endif
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] = left_matrix[i];
  }

  /*-------------------------------------------------------------------------*/
  /* LHS2: Conductivity term: theta KP_k^ls: [p^l]^n+1 */
  for (i=0;i<nn2;i++) {
        left_matrix[i] = theta * conductance_matrix_liquid[i];
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix,nn,nn,"LHS2: theta KP_k^ls");
#endif
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] += left_matrix[i];
  }

/*===========================================================================*/
/* RHS vectors */
  timelevel=0;
  /*-------------------------------------------------------------------------*/
  /* RHS1: Storativity term: 1/dt (CP_k^l - CS_k^l dS^l/dpc) [p^l]^n */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_liquid_P[i] \
                   + capacitance_matrix_liquid[i] \
                   * MMPGetSaturationPressureDependency(phase,index,0.,0.,0.,theta);
  }
  /* [p^l]^(n) */
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
   MZeigMat(right_matrix,nn,nn,"RHM1: CP_k^l - CS_k^l dS^l/dpc");
   MZeigVec(u_old_time,nn,"[p^l]^n");
  */ 
  MZeigVec(right_vector,nn,"RHS1: 1/dt (CP_k^l - CS_k^l dS^l/dpc) [p^l]^n");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS2: Conductivity term: -(1-theta) KP_k^l [p^l]^n */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = conductance_matrix_liquid[i];
  }
   /* [p^l]^n */
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= (1.0-theta);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector_P[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHM2: KP_k^l");
  MZeigVec(u_old_time,nn,"[p^l]^n");
  */
  MZeigVec(right_vector,nn,"RHS2: -(1-theta)*(KP_k^l)*[p^l]^n");
  /*  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS6: Diffusion term: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector_P[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHS6: K_w^g");
  MZeigVec(u_theta,nn,"theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS6: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n)");
  /* MZeigVec(rhs_vector_P,nn,"rhs_vector_P:"); */
#endif
  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
  for (i=0;i<nn;i++) {
    right_vector[i] = gravity_vector_gas[i];
    right_vector[i] += gravity_vector_liquid[i];
  }
  for (i=0;i<nn;i++)
    rhs_vector_P[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"rhs_vector: Gravity vector");
#endif

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix_P,rhs_vector_P,ls_rhs);

#ifdef TEST_CGS_MPC_V3
  /*    
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector_P,nn, "rhs_vector_P");
  MZeigVec(ls_rhs, NodeListLength, "ls_rhs");
  */
#endif
}



/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryS1

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Saturation equation
 (17.42)

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK
 07/2002   OK

**************************************************************************/
void MPCMakeElementEntryS1(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  double *capacitance_matrix_gas_P = NULL;
  double *capacitance_matrix_liquid_P = NULL;
  double left_matrix_S[64];

  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
  double rhs_vector_S[8];

  int timelevel;
  int phase;

  long global_node_number[8];  

#ifdef TEST_MPC_EQS
  double *rhs_vector_1;
  double *rhs_vector_2;
  double *rhs_vector_3;
  double *rhs_vector_4;
  double *rhs_vector_5;
  double *rhs_vector_6;

  if(index==0) {
    rhs_vector_1 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_2 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_3 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_4 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_5 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_6 = (double*) Malloc(NodeListLength*sizeof(double));
    MNulleVec(rhs_vector_1,NodeListLength);
    MNulleVec(rhs_vector_2,NodeListLength);
    MNulleVec(rhs_vector_3,NodeListLength);
    MNulleVec(rhs_vector_4,NodeListLength);
    MNulleVec(rhs_vector_5,NodeListLength);
    MNulleVec(rhs_vector_6,NodeListLength);
  }
#endif

  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector_S, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsgLn("MPCMakeElementEntryS1");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  /*  MZeigVec(rhs_vector,nn,"rhs_vector:"); */
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
/*
#ifdef TEST_CGS_MPC_V3
  MZeigMat(capacitance_matrix_gas,nn,nn,   "MPCMakeElementEntry: CS_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: CS_k^l capacitance matrix liquid");
  MZeigMat(conductance_matrix_gas,nn,nn,   "MPCMakeElementEntry: KP_k^g conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: KP_k^l conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas_P,nn,nn,   "MPCMakeElementEntry: CP_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid_P,nn,nn,"MPCMakeElementEntry: CP_k^l capacitance matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,     "MPCMakeElementEntry: D_k^g  diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,  "MPCMakeElementEntry: D_k^l  diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,          "MPCMakeElementEntry: g^g    gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,       "MPCMakeElementEntry: g^l    gravity vector liquid");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM(); erstmal voll implizit */
  theta = GetTimeCollocationGlobal_MMP();


/*===========================================================================*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* LHS1: Storativity term: 1/dt (C_k^l-C_k^g) : [S^l]^n+1  */
  phase=1;
  for (i=0;i<nn2;i++) {
    left_matrix[i] = capacitance_matrix_liquid[i] \
                   - capacitance_matrix_gas[i];
    left_matrix[i] *= edt;
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS1: 1/dt (C_k^l-C_k^g)");
#endif
  for (i=0;i<nn2;i++) {
    left_matrix_S[i] = left_matrix[i];
  }


if ((GetRFProcessFlowModel()==14000)||(GetRFProcessFlowModel()==15000)) {
  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix_S[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix_S[i*nn+j]);
      }
    }
  }
}



/*===========================================================================*/
/* RHS vectors */
  timelevel=0;
  /*-------------------------------------------------------------------------*/
  /* RHS1: Storativity term: - 1/dt (C_k^g-C_k^l) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas[i] \
                    - capacitance_matrix_liquid[i];
  }
   /* [S^l]^n+1, [S^l]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_S[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*    
  MZeigMat(right_matrix,nn,nn,"RHM3: C_k^g-C_k^l");
  MZeigVec(u_old_time,nn,"[S^l]^n");
  */
  MZeigVec(right_vector,nn,"RHS3: - 1/dt (C_k^g-C_k^l) [S^l]^n");
  /*  MZeigVec(rhs_vector_S,nn,"rhs_vector_S:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_1[element_nodes[i]] -= right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS2: Storativity term: - 1/dt (CP_k^g+CP_k^l) ([p^g]^n+1 - [p^g]^n) */
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas_P[i] \
                    + capacitance_matrix_liquid_P[i];
  }
  phase=0;
  /* [p^g]^(n), [p^g]^(n+1) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_S[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*    
  MZeigMat(right_matrix,nn,nn,"RHM1: CP_k^g+CP_k^l");
  MZeigVec(u_theta,nn,"[p^g]^n+1 - [p^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS1: - 1/dt (CP_k^g+CP_k^l) ([p^g]^n+1 - [p^g]^n)");
  /* MZeigVec(rhs_vector_S,nn,"rhs_vector_S:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_2[element_nodes[i]] -= right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS3: Conductivity term: - (K_k^gs+K_k^ls) ((1-theta)[p^g]^n + theta[p^g]^n+1)*/
  for (i=0;i<nn2;i++) {
    right_matrix[i] = conductance_matrix_gas[i] \
                    + conductance_matrix_liquid[i];
  }
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= (1.0-theta);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector_S[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*    
  MZeigMat(right_matrix,nn,nn,"RHM2: K_k^gs+K_k^ls");
  MZeigVec(u_theta,nn,"(1-theta)[p^g]^n + theta[p^g]^n+1");
  */
  MZeigVec(right_vector,nn,"RHS2: -(1-theta)*(KP_k^g+KP_k^ls)*((1-theta)[p^g]^n + theta[p^g]^n+1)");
  /*  MZeigVec(rhs_vector_S,nn,"rhs_vector_S:");*/
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_3[element_nodes[i]] -= right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS4: Capillarity term 1: + 1/dt CP_k^l ([p_c]^n+1 - [p_c]^n) */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_liquid_P[i];
  }
   /* [p^l]^(n+1), [p^l]^(n)*/ 
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_S[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*    
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"pc^n+1 - pc^n");
  */
  MZeigVec(right_vector,nn,"RHS4: - 1/dt CP_k^l (pc]^n+1 - [pc]^n)");
  /*  MZeigVec(rhs_vector_S,nn,"rhs_vector_S:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_4[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS5: Capillarity term 2: + KP_k^l (theta [p_c]^n+1 + (1-theta) [p_c]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p_c]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta * u_new_time[i] + (1-theta) * u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector_S[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  /*
  MZeigMat(right_matrix,nn,nn,"RHS5: KP_k^l");
  MZeigVec(u_theta,nn,"(theta [p_c]^n+1 + (1-theta) [p_c]^n)");
  */ 
  MZeigVec(right_vector,nn,"RHS5: + KP_k^l (theta [p_c]^n+1 + (1-theta) [p_c]^n)");
  /* MZeigVec(rhs_vector_S,nn,"rhs_vector_S:");*/
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_5[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS6: Diffusion term: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  for (i=0;i<nn;i++) {
//OK  timelevel=0;
    timelevel=1;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector_S[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3 
  /* 
  MZeigMat(right_matrix,nn,nn,"RHS6: K_w^g");
  MZeigVec(u_theta,nn,"theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n");
  */
  MZeigVec(right_vector,nn,"RHS6: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n)");
  /*  MZeigVec(rhs_vector_S,nn,"rhs_vector_S:"); */
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_6[element_nodes[i]] -= right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/


  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
if ((GetRFProcessFlowModel()==14000)||(GetRFProcessFlowModel()==15000)) {
  for (i=0;i<nn;i++) {
    if(comp==0) {
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector_S[i];
    }
    if(comp==1) {
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_S[i];
    }
  }
}
else
{
/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix_S,rhs_vector_S,ls_rhs);
}

#ifdef TEST_CGS_MPC_V3
  /*  MZeigMat(left_matrix, nn, nn, "left_matrix"); */
  MZeigVec(rhs_vector_S,nn, "rhs_vector_S");
  /* MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif

#ifdef TEST_MPC_EQS
  if(index==(ElementListLength-1)) {
    DisplayMsgLn("MPCMakeElementEntryS1: RHS Analysis");
    long i;
    printf("\n%s\n"," | - 1/dt CS Sl  | - 1/dt CP dpg | - KP theta pg | 1/dt CP^l dpc | KP^l theta pc | K theta X     |");
    printf("%s\n"," -------------------------------------------------------------------------------------------------");
    for (i=0;i<NodeListLength;i++) {
        printf(" | %e | %e | %e | %e | %e | %e | \n", \
               rhs_vector_1[i],rhs_vector_2[i],rhs_vector_3[i],\
               rhs_vector_4[i],rhs_vector_5[i],rhs_vector_6[i]);
    }
    printf("  %s\n"," -------------------------------------------------------------------------------------------------");
  }
#endif

}




/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryS2

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Saturation

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK

**************************************************************************/
void MPCMakeElementEntryS2(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
  int timelevel;
  int phase;
comp=comp;
  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector: init");
#endif
/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
#ifdef TEST_CGS_MPC_V3
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  //OK MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM();*/


/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Capacitance matrix: 1/dt*(C_k^l - C_k^g) : [S^l^(n+1)] */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS: 1/dt*(C_k^l - C_k^g)");
#endif

/*---------------------------------------------------------------------------*/
/* RHS vectors */
  /*-------------------------------------------------------------------------*/
  /* Capacitance: + 1/dt*(C_k^l-C_k^g) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
   /* [S^l]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"RHS: + 1/dt ( C_k^g - C_k^l ) [S^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity: - (K_k^gs + K_k^ls)(theta*[p^g]^n+1 + (1-theta)*[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_gas[i] + conductance_matrix_liquid[i];
   /* [p^g]^(n+1), [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: (K_k^gs + K_k^ls)");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - (K_k^gs + K_k^ls)(theta*[p^g]^n+1 - (1-theta)*[p^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Capillarity: + K_k^ls (theta*[p_c]^n+1 + (1-theta)*[p_c]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p_c]^(n+1), [p_c]^n */
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],0,timelevel) \
                  - MPCGetNodePressure(element_nodes[i],1,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],0,timelevel) \
                  - MPCGetNodePressure(element_nodes[i],1,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^ls");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: + K_k^ls (theta*[p_c]^n+1 - (1-theta)*[p_c]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt ( CP_k^g + CP_k^l) (p^l]^n+1 - [p^l]^n) */
/*
  double r;
  r=0.0;

  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * ( capacitance_matrix_gas[i] \
                              / MATCalcFluidDensity(0,index,&r,theta,0,NULL) \
                              * GetFluidDensityPressureDependency(0) \
                            + capacitance_matrix_liquid[i] \
                              / MATCalcFluidDensity(1,index,&r,theta,0,NULL) \
                              * GetFluidDensityPressureDependency(0) );
[p^l]^(n+1), [p^l]^(n) 
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel) \
                  * MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel) \
                  * MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: 1/dt ( C_k^g + C_k^l)");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt ( C_k^g + C_k^l) (p^l]^n+1 - [p^l]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
*/
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt ( C_k^g + C_k^l) (p^g]^n+1 - [p^g]^n) */
/*
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_gas[i] + capacitance_matrix_liquid[i]);
[p^g]^(n+1), [p^g]^(n) 
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^ls");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt ( C_k^g + C_k^l) (p^g]^n+1 - [p^g]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
*/
  /*-------------------------------------------------------------------------*/
  /* Capillarity: + 1/dt C_k^l (p_c]^n+1 - [p_c]^n) */
/*
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * capacitance_matrix_liquid[i];
[p_c]^(n+1), [p_c]^(n)
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],0,timelevel) \
                  - MPCGetNodePressure(element_nodes[i],1,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],0,timelevel) \
                  - MPCGetNodePressure(element_nodes[i],1,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"RHS: + 1/dt C_k^l (p_c]^n+1 - [p_c]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
*/
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n */
/*
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
[X_w^g]^n+1
  comp=1;
  timelevel=1;
  for (i=0;i<nn;i++) {
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_new_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += theta*right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - new time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += (1-theta)*right_vector[i];
#ifdef TEST_CGS_MPC1
  MZeigVec(right_vector,nn,"right_vector: Diffusion matrix - old time");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
*/
/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);

#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
/*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}





/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryS3

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Saturation

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK

**************************************************************************/
void MPCMakeElementEntryS3(int comp,long index,double *ls_rhs)
{
  static int i;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];

//  double gp[]={0.0,0.0,0.0};

  int timelevel;
  int phase;

  double r;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector: init");
#endif
/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
#ifdef TEST_CGS_MPC_V3
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  //OK MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM();*/


/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Capacitance matrix: 1/dt*(C_k^l - C_k^g) : [S^l^(n+1)] */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS: 1/dt*(C_k^l - C_k^g)");
#endif

/*---------------------------------------------------------------------------*/
/* RHS vectors */
  /*-------------------------------------------------------------------------*/
  /* Capacitance: + 1/dt*(C_k^l-C_k^g) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
   /* [S^l]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"RHS: + 1/dt ( C_k^g - C_k^l ) [S^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity: - K_k^gs (theta*[p^g]^n+1 + (1-theta)*[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_gas[i];
   /* [p^g]^(n+1), [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^gs");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_k^gs (theta*[p^g]^n+1 - (1-theta)*[p^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity: - K_k^ls (theta*[p^l]^n+1 + (1-theta)*[p^l]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p^l]^(n+1), [p^l]^n */
  phase=1;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^ls");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_k^ls (theta*[p^l]^n+1 - (1-theta)*[p^l]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n) */
  r=0.0;
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_liquid[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp;
   /* [p^l]^(n+1), [p^l]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt CP_k^g (p^g]^n+1 - [p^g]^n) */
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_gas[i] \
					/ mfp_vector[phase]->Density() \
  					* mfp_vector[phase]->drho_dp;
   /* [p^g]^(n+1), [p^g]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt C_k^g (p^g]^n+1 - [p^g]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  comp=1;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_w^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);

#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
  /*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}




/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryS4

 Task:
 Make the equation system (EQS)
 Partitioned scheme - Saturation

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK

**************************************************************************/
void MPCMakeElementEntryS4(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
//  double gp[]={0.0,0.0,0.0};

  int timelevel;
  int phase;

  static long global_node_number[8];

  double r;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V3
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector: init");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);

#ifdef TEST_CGS_MPC_V3
  MZeigMat(conductance_matrix_gas,nn,nn,"MPCMakeElementEntry: conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas,nn,nn,"MPCMakeElementEntry: capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: capacitance matrix liquid");
  //OK MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,"MPCMakeElementEntry: diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,"MPCMakeElementEntry: diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,"MPCMakeElementEntry: gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,"MPCMakeElementEntry: gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM();*/


/*---------------------------------------------------------------------------*/
/* LHS matrix */
  /* Capacitance matrix: 1/dt*(C_k^l - C_k^g) : [S^l^(n+1)] */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
  }
#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "LHS: 1/dt*(C_k^l - C_k^g)");
#endif

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }


/*---------------------------------------------------------------------------*/
/* RHS vectors */
  /*-------------------------------------------------------------------------*/
  /* Capacitance: + 1/dt*(C_k^l-C_k^g) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
   /* [S^l]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigVec(right_vector,nn,"RHS: + 1/dt ( C_k^g - C_k^l ) [S^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity: - K_k^gs (theta*[p^g]^n+1 + (1-theta)*[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_gas[i];
   /* [p^g]^(n+1), [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^gs");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_k^gs (theta*[p^g]^n+1 - (1-theta)*[p^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Conductivity: - K_k^ls (theta*[p^l]^n+1 + (1-theta)*[p^l]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p^l]^(n+1), [p^l]^n */
  phase=1;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^ls");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_k^ls (theta*[p^l]^n+1 - (1-theta)*[p^l]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n) */
  r=0.0;

  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_liquid[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp;
   /* [p^l]^(n+1), [p^l]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Storativity: - 1/dt CP_k^g (p^g]^n+1 - [p^g]^n) */
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_gas[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp;
   /* [p^g]^(n+1), [p^g]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt C_k^g (p^g]^n+1 - [p^g]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* Diffusion matrix: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  comp=1;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V3
  MZeigMat(right_matrix,nn,nn,"RHS: K_w^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: theta * K_w^g [X_w^g]_t^n+1 + (1-theta) K_w^g [X_w^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/


  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
  for (i=0;i<nn;i++) {
    if(comp==0) {
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
    if(comp==1) {
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
  }


/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
 /* IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);*/

#ifdef TEST_CGS_MPC_V3
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
  /*  MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}




/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryV5

 Task:
 Make the equation system (EQS)
 Monolithic scheme - p^g, S^l

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 05/2002   OK

**************************************************************************/
void MPCMakeElementEntryV5(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double rhs_vector[8];
  static double right_vector[8];
  static double u_theta[8];
  double gp[]={0.0,0.0,0.0};
  int timelevel;
  int phase;
  static long global_node_number[8];
  double r;
  r=0.0;
  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector, nn);
#ifdef TEST_CGS_MPC_V5
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
  MZeigVec(rhs_vector,nn,"rhs_vector: init");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
//OK   capillarity_matrix_liquid = MPCGetELECapillarityMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);

#ifdef TEST_CGS_MPC_V5
  MZeigMat(capacitance_matrix_gas,nn,nn,   "MPCMakeElementEntry: CS_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: CS_k^l capacitance matrix liquid");
  MZeigMat(conductance_matrix_gas,nn,nn,   "MPCMakeElementEntry: KP_k^g conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: KP_k^l conductivity matrix liquid");
  MZeigMat(capillarity_matrix_liquid,nn,nn,"MPCMakeElementEntry: capillarity matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,     "MPCMakeElementEntry: D_k^g  diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,  "MPCMakeElementEntry: D_k^l  diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,          "MPCMakeElementEntry: g^g    gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,       "MPCMakeElementEntry: g^l    gravity vector liquid");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM();*/


/*===========================================================================*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* LHS1: Storativity term: 1/dt CP_k^g: [p^g]^n+1) */
  phase=0;
  for (i=0;i<nn2;i++)
    left_matrix[i] = capacitance_matrix_gas[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp \
                    * MMax(0.,PCSGetELEValue(index,gp,theta,"SATURATION1")); //OK MPCGetSaturation(phase,index,0.,0.,0.,theta));
#ifdef TEST_CGS_MPC_V5
  MZeigMat(left_matrix, nn, nn, "LHS: 1/dt CP_k^g");
#endif

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }


  /*-------------------------------------------------------------------------*/
  /* LHS2: Conductivity term: K_k^gs theta: [p^g]^n+1 */
  for (i=0;i<nn2;i++)
    left_matrix[i] = theta * conductance_matrix_gas[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(left_matrix, nn, nn, "LHS: K_k^gs theta");
#endif

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }

  /*-------------------------------------------------------------------------*/
  /* LHS3: Storativity term: + 1/dt*(C_k^l - C_k^g) : [S^l^(n+1)] */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
  }
#ifdef TEST_CGS_MPC_V5
  MZeigMat(left_matrix, nn, nn, "LHS: 1/dt*(C_k^l - C_k^g)");
#endif

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix[i*nn+j]);
      }
    }
  }


/*===========================================================================*/
/* RHS vectors */
  /*-------------------------------------------------------------------------*/
  /* RHS1: Storativity term: + 1/dt CP_k^g [p^g]^n */
  phase=0;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_gas[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp;
   /* [p^g]^(n+1), [p^g]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    /*u_theta[i] = u_new_time[i] - u_old_time[i];*/
    u_theta[i] = -1.0 * u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt C_k^g (p^g]^n+1 - [p^g]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS2: Conductivity term: - K_k^gs (1-theta)*[p^g]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = (1.0-theta) * conductance_matrix_gas[i];
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^gs");
  MZeigVec(u_old_time,nn,"u_old_time");
  MZeigVec(right_vector,nn,"RHS: - K_k^gs (1-theta)*[p^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS3: Storativity term: + 1/dt*(C_k^l-C_k^g) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = edt * (capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
   /* [S^l]^n */
  timelevel=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigVec(right_vector,nn,"RHS: + 1/dt ( C_k^g - C_k^l ) [S^g]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS4: Storativity term: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n) */
  phase=1;
  for (i=0;i<nn2;i++)
    right_matrix[i] = capacitance_matrix_liquid[i] \
					/ mfp_vector[phase]->Density() \
					* mfp_vector[phase]->drho_dp;
   /* [p^l]^(n+1), [p^l]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= edt * right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - 1/dt CP_k^l (p^l]^n+1 - [p^l]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS5: Conductivity term: - K_k^ls (theta*[p^l]^n+1 + (1-theta)*[p^l]^n */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p^l]^(n+1), [p^l]^n */
  phase=1;
  for (i=0;i<nn;i++) {
    timelevel=1;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(right_matrix,nn,nn,"RHS: K_k^ls");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_k^ls (theta*[p^l]^n+1 - (1-theta)*[p^l]^n");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS6: Diffusion term: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  comp=1;
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    rhs_vector[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V5
  MZeigMat(right_matrix,nn,nn,"RHS: K_w^g");
  MZeigVec(u_theta,nn,"u_theta");
  MZeigVec(right_vector,nn,"RHS: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n)");
  MZeigVec(rhs_vector,nn,"rhs_vector:");
#endif
/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/


  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
  for (i=0;i<nn;i++) {
    if(comp==0) {
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
    if(comp==1) {
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector[i];
    }
  }


/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  /*  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs); */

#ifdef TEST_CGS_MPC_V5
  MZeigMat(left_matrix, nn, nn, "left_matrix");
  MZeigVec(rhs_vector,nn, "rhs_vector");
  /* MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif
}







/**************************************************************************
 ROCKFLOW - Funktion: MPCMakeElementEntryV7

 Task:
 Make the equation system (EQS)
 Monolithic scheme - p^g, S^l

 Parameter: (I: Input; R: Return; X: Both)

 Return:
   - void -

 Programming:
 07/2002   OK
 01/2003   OK/JdJ   dT/dt term
 01/2003   OK/JdJ   grad T term

**************************************************************************/
void MPCMakeElementEntryV7(int comp,long index,double *ls_rhs)
{
  static int i,j;
  static long nn, nn2;
  static long *element_nodes;
  static double edt, theta;
  double *conductance_matrix_gas = NULL;
  double *capacitance_matrix_gas = NULL;
  double *diffusion_matrix_gas = NULL;
  double *gravity_vector_gas = NULL;
  double *conductance_matrix_liquid = NULL;
  double *capacitance_matrix_liquid = NULL;
  double *diffusion_matrix_liquid = NULL;
  double *gravity_vector_liquid = NULL;
  double *capacitance_matrix_gas_P = NULL;
  double *capacitance_matrix_liquid_P = NULL;
  static double left_matrix[64];
  static double right_matrix[64];
  static double u_old_time[8];
  static double u_new_time[8];
  static double right_vector[8];
  static double u_theta[8];
  double rhs_vector_P[8];
  double rhs_vector_S[8];
  double rhs_vector_S2[8];
  double left_matrix_P[64];
  double left_matrix_S[64];
  double *storativity_matrix_gas_T = NULL;
  double *diffusion_thermo_comp_matrix = NULL;

  int timelevel;
  int phase;

  static long global_node_number[8];
  double r;

double S_fac = 1e-6;
double p_fac = 1e-6;


#ifdef TEST_MPC_EQS
  double *rhs_vector_1;
  double *rhs_vector_2;
  double *rhs_vector_3;
  double *rhs_vector_4;
  double *rhs_vector_5;
  double *rhs_vector_6;

  if(index==0) {
    rhs_vector_1 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_2 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_3 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_4 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_5 = (double*) Malloc(NodeListLength*sizeof(double));
    rhs_vector_6 = (double*) Malloc(NodeListLength*sizeof(double));
    MNulleVec(rhs_vector_1,NodeListLength);
    MNulleVec(rhs_vector_2,NodeListLength);
    MNulleVec(rhs_vector_3,NodeListLength);
    MNulleVec(rhs_vector_4,NodeListLength);
    MNulleVec(rhs_vector_5,NodeListLength);
    MNulleVec(rhs_vector_6,NodeListLength);
  }
#endif

  r=0.0;

  /* Parameter fuer die Ortsdiskretisierung */
  nn = ElNumberOfNodes[ElGetElementType(index) - 1];
  nn2 = nn * nn;
  element_nodes = ElGetElementNodes(index);     /* Original-Knotennummern des Elements */
  for(i=0;i<nn;i++)
    global_node_number[i] = GetNodeIndex(element_nodes[i]);

  pressure_vector_pos = MPCGetPressureVectorPosition();
  saturation_vector_pos = MPCGetSaturationVectorPosition();

  /* Initialisierungen */
  MNulleMat(left_matrix, nn, nn);
  MNulleMat(right_matrix, nn, nn);
  MNulleVec(u_old_time, nn);
  MNulleVec(rhs_vector_P, nn);
  MNulleVec(rhs_vector_S, nn);
  MNulleVec(rhs_vector_S2, nn);
#ifdef TEST_CGS_MPC_V7
  DisplayMsgLn("------------------------");
  DisplayMsg("Element: "); DisplayLong(index); DisplayMsgLn("");
#endif

/*---------------------------------------------------------------------------*/
  /* Elementmatrizen und -vektoren bereitstellen oder berechnen */
  phase=0;
  conductance_matrix_gas = MPCGetELEConductivityMatrix(index,phase);
  phase=1;
  conductance_matrix_liquid = MPCGetELEConductivityMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas = MPCGetELECapacitanceMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid = MPCGetELECapacitanceMatrix(index,phase);
  phase=0;
  capacitance_matrix_gas_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=1;
  capacitance_matrix_liquid_P = MPCGetELEStorativityPressureMatrix(index,phase);
  phase=0;
  diffusion_matrix_gas = MPCGetELEDiffusionMatrix(index,phase);
  phase=1;
  diffusion_matrix_liquid = MPCGetELEDiffusionMatrix(index,phase);
  phase=0;
  gravity_vector_gas = MPCGetELEGravityVector(index,phase);
  phase=1;
  gravity_vector_liquid = MPCGetELEGravityVector(index,phase);
  phase=0;
  storativity_matrix_gas_T = MPCGetELEStorativityTemperatureMatrix(index,phase);
  phase=0;
  diffusion_thermo_comp_matrix = MPCGetELEThermoCompDiffusionMatrix(index,phase);

  if (comp==0)
    for (i=0;i<nn2;i++) {
      capacitance_matrix_gas[i] *= S_fac;
      capacitance_matrix_liquid[i] *= S_fac;
    }
  if (comp==1)
    for (i=0;i<nn2;i++) {
      /*
        conductance_matrix_gas[i] *= p_fac;
        conductance_matrix_liquid[i] *= p_fac;
          */
      capacitance_matrix_gas_P[i] *= p_fac;
      capacitance_matrix_liquid_P[i] *= p_fac;
      diffusion_matrix_gas[i] *= p_fac;
      diffusion_matrix_liquid[i] *= p_fac;
      gravity_vector_gas[i] *= p_fac;
      gravity_vector_liquid[i] *= p_fac;
  }

#ifdef TEST_CGS_MPC_V7
  MZeigMat(capacitance_matrix_gas,nn,nn,   "MPCMakeElementEntry: CS_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid,nn,nn,"MPCMakeElementEntry: CS_k^l capacitance matrix liquid");
  MZeigMat(conductance_matrix_gas,nn,nn,   "MPCMakeElementEntry: KP_k^g conductivity matrix gas");
  MZeigMat(conductance_matrix_liquid,nn,nn,"MPCMakeElementEntry: KP_k^l conductivity matrix liquid");
  MZeigMat(capacitance_matrix_gas_P,nn,nn,   "MPCMakeElementEntry: CP_k^g capacitance matrix gas");
  MZeigMat(capacitance_matrix_liquid_P,nn,nn,"MPCMakeElementEntry: CP_k^l capacitance matrix liquid");
  MZeigMat(diffusion_matrix_gas,nn,nn,     "MPCMakeElementEntry: D_k^g  diffusion matrix gas");
  MZeigMat(diffusion_matrix_liquid,nn,nn,  "MPCMakeElementEntry: D_k^l  diffusion matrix liquid");
  MZeigVec(gravity_vector_gas,nn,          "MPCMakeElementEntry: g^g    gravity vector gas");
  MZeigVec(gravity_vector_liquid,nn,       "MPCMakeElementEntry: g^l    gravity vector liquid");
  MZeigMat(diffusion_thermo_comp_matrix,nn,nn,  "MPCMakeElementEntry: DT_w^g  thermo-componental diffusion matrix vapor");
#endif

/*---------------------------------------------------------------------------*/
  /* Parameter fuer die Zeitdiskretisierung */
  if (dt < MKleinsteZahl)
    {
      DisplayMsgLn("Zeitschritt ist Null ! Abbruch !");
      return;
    }
  else
    {
      edt = 1.0 / dt;
    }
  theta = 0.6; /*GetTimeCollocationGlobal_HTM();*/
  theta = GetTimeCollocationGlobal_MMP();


/*===========================================================================*/
/* LHS matrix */
  /*-------------------------------------------------------------------------*/
  /* LHS1: Storativity term: 1/dt (CP_k^g + CP_k^l): [p^g]^n+1) */
  phase=0;
  for (i=0;i<nn2;i++) {
    left_matrix[i] = capacitance_matrix_gas_P[i] \
                   + capacitance_matrix_liquid_P[i];
    left_matrix[i] *= edt;
  }
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] = left_matrix[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(left_matrix_P, nn, nn, "LHS1: 1/dt (CP_k^g + CP_k^l)");
#endif

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix_P[i*nn+j]);
      }
      if(comp==1) {
/*OK_IC        if(IsNodeBoundaryCondition("SATURATION2",\
                                   global_node_number[i])) \
                                   left_matrix_P[i*nn+j]=0.0;*/
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix_P[i*nn+j]);
      }
    }
  }

  /*-------------------------------------------------------------------------*/
  /* LHS2: Conductivity term: theta (KP_k^gs + KP_k^ls): [p^g]^n+1 */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = theta * \
                    (conductance_matrix_gas[i] + conductance_matrix_liquid[i]);
  }
  for (i=0;i<nn2;i++) {
    left_matrix_P[i] = left_matrix[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(left_matrix_P, nn, nn, "LHS2: theta (KP_k^gs + KP_k^ls )");
#endif


  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
        MXInc(pressure_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix_P[i*nn+j]);
      }
      if(comp==1) {
/*OK_IC        if(IsNodeBoundaryCondition("SATURATION2",\
                                   global_node_number[i])) \
                                   left_matrix_P[i*nn+j]=0.0;*/
        MXInc(saturation_vector_pos+global_node_number[i],\
              pressure_vector_pos+global_node_number[j],left_matrix_P[i*nn+j]);
      }
    }
  }


  /*-------------------------------------------------------------------------*/
  /* LHS3: Storativity term: + 1/dt (CS_k^l - CS_k^g) : [S^l^(n+1)] */
  for (i=0;i<nn2;i++) {
    left_matrix[i] = edt*(capacitance_matrix_liquid[i]-capacitance_matrix_gas[i]);
  }
  for (i=0;i<nn2;i++) {
    left_matrix_S[i] = left_matrix[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(left_matrix_S, nn, nn, "LHS3: 1/dt (CS_k^l - CS_k^g)");
#endif

/*
if(comp==0)
  for (i=0;i<nn2;i++) {
    left_matrix_S[i] = 0.0;
  }
*/

  /* Einspeichern in die linke Systemmatrix */
  for(i=0;i<nn;i++) {
    for(j=0;j<nn;j++) {
      if(comp==0) {
/*OK_IC        if(IsNodeBoundaryCondition("PRESSURE1",\
                                   global_node_number[i])) \
                                   left_matrix_S[i*nn+j]=0.0;*/
        MXInc(pressure_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix_S[i*nn+j]);
      }
      if(comp==1) {
        MXInc(saturation_vector_pos+global_node_number[i],\
              saturation_vector_pos+global_node_number[j],left_matrix_S[i*nn+j]);
      }
    }
  }


/*===========================================================================*/
/* RHS vectors */
  timelevel=0;
  /*-------------------------------------------------------------------------*/
  /* RHS1: Storativity term: + 1/dt (CP_k^g+CP_k^l) [p^g]^n */
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas_P[i] \
                    + capacitance_matrix_liquid_P[i];
  }
  phase=0;
  /* [p^g]^(n) */
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_P[i] += right_vector[i];
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHM1: CP_k^g+CP_k^l");
  MZeigVec(u_old_time,nn,"[p^g]^n");
  MZeigVec(right_vector,nn,"RHS1: + 1/dt (CP_k^g+CP_k^l) [p^g]^n");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_1[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS2: Conductivity term: -(1-theta) (K_k^gs+K_k^ls) [p^g]^n */
  for (i=0;i<nn2;i++) {
    right_matrix[i] = conductance_matrix_gas[i] \
                    + conductance_matrix_liquid[i];
  }
   /* [p^g]^n */
  phase=0;
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= (1.0-theta);
   /* add to RHS vector */
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] -= right_vector[i];
    rhs_vector_S2[i] -= right_vector[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHM2: K_k^gs+K_k^ls");
  MZeigVec(u_old_time,nn,"[p^g]^n");
  MZeigVec(right_vector,nn,"RHS2: -(1-theta)*(K_k^gs+K_k^ls)*[p^g]^n");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_2[element_nodes[i]] -= right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS3: Storativity term: - 1/dt (CS_k^g-CS_k^l) [S^l]^n */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_gas[i] \
                    - capacitance_matrix_liquid[i];
  }
   /* [S^l]^n */
  for (i=0;i<nn;i++) {
    u_old_time[i] = MPCGetNodeSaturation(element_nodes[i],phase,timelevel);
  }
  MMultMatVec(right_matrix,nn,nn,u_old_time,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++)
    rhs_vector_S[i] -= right_vector[i];
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHM3: C_k^l-C_k^g");
  MZeigVec(u_old_time,nn,"[S^l]^n");
  MZeigVec(right_vector,nn,"RHS3: + 1/dt (C_k^g-C_k^l) [S^g]^n");
  MZeigVec(rhs_vector_S,nn,"rhs_vector_S:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_3[element_nodes[i]] -= right_vector[i];
#endif

  /*-------------------------------------------------------------------------*/
  /* RHS4: Capillarity term 1: + 1/dt CP_k^l ([p_c]^n+1 - [p_c]^n) */
  phase=1;
  for (i=0;i<nn2;i++) {
    right_matrix[i] = capacitance_matrix_liquid_P[i];
  }
   /* [p^l]^(n+1), [p^l]^(n) */
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = u_new_time[i] - u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++)
    right_vector[i] *= edt;
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] += right_vector[i];
    rhs_vector_S2[i] += right_vector[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHS: CP_k^l");
  MZeigVec(u_theta,nn,"pc^n+1 - pc^n");
  MZeigVec(right_vector,nn,"RHS4: - 1/dt CP_k^l (pc]^n+1 - [pc]^n)");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_4[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS5: Capillarity term 2: + KP_k^l (theta [p_c]^n+1 + (1-theta) [p_c]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = conductance_matrix_liquid[i];
   /* [p_c]^n+1, [p_c]^n */
  for (i=0;i<nn;i++) {
    timelevel=0;
    phase=0;
    u_old_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_old_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    timelevel=1;
    phase=0;
    u_new_time[i] = MPCGetNodePressure(element_nodes[i],phase,timelevel);
    phase=1;
    u_new_time[i] -= MPCGetNodePressure(element_nodes[i],phase,timelevel);
    u_theta[i] = theta * u_new_time[i] + (1-theta) * u_old_time[i];
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] += right_vector[i];
    rhs_vector_S2[i] += right_vector[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHS5: KP_k^l");
  MZeigVec(u_theta,nn,"pc^n");
  MZeigVec(right_vector,nn,"RHS5: + KP_k^l [p_c]^n");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_5[element_nodes[i]] += right_vector[i];
#endif
  /*-------------------------------------------------------------------------*/
  /* RHS6: Diffusion term: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n) */
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_matrix_gas[i];
   /* [X_w^g]^n+1 */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetNodeMassFraction(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] -= right_vector[i];
    rhs_vector_S2[i] -= right_vector[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHS6: K_w^g");
  MZeigVec(u_theta,nn,"theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n");
  MZeigVec(right_vector,nn,"RHS6: - K_w^g (theta*[X_w^g]_t^n+1 + (1-theta)*[X_w^g]^n)");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_6[element_nodes[i]] -= right_vector[i];
#endif

  /*-------------------------------------------------------------------------*/
  /* RHS7: Storativity term: - CT_w^g ([T]^n+1 - [T]^n) / dt */
if((phase==0)&&(comp==1)) {
  for (i=0;i<nn2;i++)
    right_matrix[i] = storativity_matrix_gas_T[i];
   /* [T]^n+1,[T]^n */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetTemperatureNode(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetTemperatureNode(element_nodes[i],comp,timelevel);
    u_theta[i] = (u_new_time[i] - u_old_time[i]) / dt;
  }
   /* this RHS vector */
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
   /* add to RHS vector */
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] -= right_vector[i];
    rhs_vector_S2[i] -= right_vector[i];
  }
}
  /*-------------------------------------------------------------------------*/
  /* RHS8: Thermo-componental diffusion term: KT_w^g (theta*[T]^n+1 + (1-theta)*[T]^n) */
if((phase==0)&&(comp==1)) {
  for (i=0;i<nn2;i++)
    right_matrix[i] = diffusion_thermo_comp_matrix[i];
   /* [T]^n+1,[T]^n */
  for (i=0;i<nn;i++) {
    timelevel=0;
    u_old_time[i] = MPCGetTemperatureNode(element_nodes[i],comp,timelevel);
    timelevel=1;
    u_new_time[i] = MPCGetTemperatureNode(element_nodes[i],comp,timelevel);
    u_theta[i] = theta*u_new_time[i] + (1.0-theta)*u_old_time[i];
  }
  MMultMatVec(right_matrix,nn,nn,u_theta,nn,right_vector,nn);
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] -= right_vector[i];
    rhs_vector_S2[i] -= right_vector[i];
  }
#ifdef TEST_CGS_MPC_V7
  MZeigMat(right_matrix,nn,nn,"RHS8: KT_w^g");
  MZeigVec(u_theta,nn,"theta*[T]^n+1 + (1-theta)*[T]^n");
  MZeigVec(right_vector,nn,"RHS8: - KT_w^g (theta*[T]^n+1 + (1-theta)*[T]^n)");
  MZeigVec(rhs_vector_P,nn,"rhs_vector_P:");
#endif
#ifdef TEST_MPC_EQS
  for (i=0;i<nn;i++)
    rhs_vector_8[element_nodes[i]] -= right_vector[i];
#endif
}
/*-------------------------------------------------------------------------*/
  /* Gravity vector: - g_a^g - g_a^l */
/*
  for (i=0;i<nn;i++) {
    rhs_vector[i] -= gravity_vector_gas[i];
    rhs_vector[i] -= gravity_vector_liquid[i];
  }
#ifdef TEST_CGS_MPC
  MZeigVec(rhs_vector,nn,"rhs_vector: Gravity vector");
#endif
*/

/*
if(comp==1) {
  for (i=0;i<nn;i++) {
    rhs_vector_P[i] *= ok_fac;
  }
}
*/

  /*-------------------------------------------------------------------------*/
  /* Einspeichern in den Rechte-Seite-Vektor */
  for (i=0;i<nn;i++) {
    if(comp==0) {
/*OK*/
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector_P[i];
/*OK_IC      if(IsNodeBoundaryCondition("PRESSURE1",global_node_number[i]))
        rhs_vector_S[i]=0.0;*/
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector_S[i];
          /*
        ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_S[i];
        ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_S[i]-rhs_vector_P[i];
          */
    }
    if(comp==1) {
/*OK*/
     /*
      ls_rhs[pressure_vector_pos+global_node_number[i]] += rhs_vector_P[i];
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_P[i];
         */
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_S[i];
/*OK_IC      if(IsNodeBoundaryCondition("SATURATION2",global_node_number[i])) \
                                 rhs_vector_P[i]=0.0;*/
      ls_rhs[saturation_vector_pos+global_node_number[i]] += rhs_vector_P[i];
    }
  }


/*---------------------------------------------------------------------------*/
  /* In Gleichungssystem einspeichern, ev. irr. Knoten behandeln */
  /*  IncorporateMatrix(index,left_matrix,rhs_vector,ls_rhs);*/

#ifdef TEST_CGS_MPC_V7
  MZeigMat(left_matrix_P, nn, nn, "left_matrix_P");
  MZeigMat(left_matrix_S, nn, nn, "left_matrix_S");
  MZeigVec(rhs_vector_P,nn, "rhs_vector_P");
  MZeigVec(rhs_vector_S,nn, "rhs_vector_S");
  /* MZeigVec(ls_rhs, NodeListLength, "ls_rhs"); */
#endif


#ifdef TEST_MPC_EQS
  if(index==(ElementListLength-1)) {
    DisplayMsgLn("MPCMakeElementEntryV7: RHS Analysis");
    long i;
    printf("\n%s\n"," | - 1/dt CP pg  | - theta KP pg | - 1/dt CS Sl  | 1/dt CP^l dpc | KP^l theta pc | K theta X     |");
    printf("%s\n"," -------------------------------------------------------------------------------------------------");
    for (i=0;i<NodeListLength;i++) {
        printf(" | %e | %e | %e | %e | %e | %e | \n", \
               rhs_vector_1[i],rhs_vector_2[i],rhs_vector_3[i],\
               rhs_vector_4[i],rhs_vector_5[i],rhs_vector_6[i]);
    }
    printf("  %s\n"," -------------------------------------------------------------------------------------------------");
  }
#endif

}

/*************************************************************************
ROCKFLOW - Function: MPCCalcSecondaryVariablesRichards
Task: 
Programming: 
07/2003 OK Implementation
06/2004  WW  Remove the onwer ( CRFProcess) of PCSGetNODValueIndex
06/2005 OK PCS
ToDo: 
last modified:
**************************************************************************/
void MPCCalcSecondaryVariablesRichards(void)
{
  long i;
  double p_cap,mass_fraction,p_liquid;
  double saturation_liquid,saturation_gas;
  int phase,comp,timelevel;
  double density;
  double theta=1.0;
  double gp[3]={0.,0.,0.};
  CFluidProperties* m_mfp = NULL;
  CMediumProperties* m_mmp = NULL;
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet("COMPONENTAL_FLOW");
  if(!m_pcs){
    cout << "MPCCalcSecondaryVariablesRichards: no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcSecondaryVariablesRichards: no time discretization data !" << endl;
  //----------------------------------------------------------------------
/*
  timelevel=1;
  NodeSetBoundaryConditions("SATURATION1",\
                            PCSGetNODValueIndex("SATURATION1",timelevel), NULL);
  NodeSetBoundaryConditions("SATURATION2",\
                            PCSGetNODValueIndex("SATURATION2",timelevel), NULL);
*/
  for (i=0;i<NodeListLength;i++) {
if(m_tim->step_current>1) {
//OK_TIM if(aktueller_zeitschritt>0) {
    phase=1;
    timelevel=1;
    p_liquid = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel));
    p_cap = -1.0*p_liquid;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
    //OK_ToDo saturation_liquid = MATCalcNodeSaturationsFromCapillaryPressure(phase,i,theta);
    saturation_liquid = 1.0; //OK
    saturation_liquid = MRange(0.0,saturation_liquid,1.0);
    saturation_gas = MRange(0.0,(1.0-saturation_liquid),1.0);
    NodeSetBoundaryConditions("SATURATION1",\
                              PCSGetNODValueIndex("SATURATION1",timelevel),\
                              m_pcs->pcs_type_name);
    NodeSetBoundaryConditions("SATURATION2",\
                              PCSGetNODValueIndex("SATURATION2",timelevel),\
                              m_pcs->pcs_type_name);
    SetNodeVal(i,PCSGetNODValueIndex("SATURATION1",timelevel),saturation_gas);
    SetNodeVal(i,PCSGetNODValueIndex("SATURATION2",timelevel),saturation_liquid);
}
else
{
    /*-------------------------------------------------------------------------*/
    /* saturations */
     /* liquid saturation S^l */
/*
    timelevel=0;
    phase=1;
    saturation_liquid = GetNodeVal(i,PCSGetNODValueIndex("SATURATION2",timelevel));
    saturation_liquid = MRange(0.0,saturation_liquid,1.0);
*/
    /* gas saturation S^g */
/*
    phase=0;
    saturation_gas = MRange(0.0,(1.0-saturation_liquid),1.0);
    SetNodeVal(i,PCSGetNODValueIndex("SATURATION1",timelevel),saturation_gas);
*/
    /*-------------------------------------------------------------------------*/
    /* capillary pressure - p_c(S) */
    /* Phasendruck fuer 2. Phase: p^l = - p_c(S)*/
    phase=1;
    timelevel=0;
//    p_liquid = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel));
    //OK p_cap = MATCalcNodeCapillaryPressure(phase,index,i,0.0);
    m_mmp->mode = 1;
    p_cap = m_mmp->CapillaryPressureFunction(i,gp,theta,phase,0.0);
//    p_cap = -1.0*p_liquid;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
if(i==0) {
    p_liquid = 101325.0 - p_cap;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel),p_liquid);
}
    timelevel=1;
//    p_liquid = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel));
    //OK p_cap = MATCalcNodeCapillaryPressure(phase,index,i,theta);
    p_cap = m_mmp->CapillaryPressureFunction(i,gp,theta,phase,0.0);
//    p_cap = -1.0*p_liquid;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
if(i==0) {
    p_liquid = 101325.0 - p_cap;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel),p_liquid);
}
}
    /*-------------------------------------------------------------------------*/
    /* mass fractions */
     /* X_w^g */
    phase=0;
    comp=1;
    //OK mass_fraction = MATCalcMassFractionNode(phase,comp,0,0.0,0.0,i,theta);
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    mass_fraction = m_mfp->MassFraction(i,comp,gp,theta);
    timelevel=0;
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION2",timelevel),mass_fraction);
    timelevel=1;
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION2",timelevel),mass_fraction);
     /* X^a_l */
    phase=1;
    comp=0;
    //OK mass_fraction = MATCalcMassFractionNode(phase,comp,0,0.0,0.0,i,theta);
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    mass_fraction = m_mfp->MassFraction(i,comp,gp,theta);
    timelevel=0;
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION1",timelevel),mass_fraction);
    timelevel=1;
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION1",timelevel),mass_fraction);
    /*-------------------------------------------------------------------------*/
    /* densities */
     /* rho^g */
    phase=0;
//SB:m    density = MATCalcFluidDensityNode(phase,i,theta);
	density = mfp_vector[phase]->Density();
	if(PCSGetNODValueIndex("DENSITY1",timelevel)>0)
      SetNodeVal(i,PCSGetNODValueIndex("DENSITY1",timelevel),density);
     /* rho^l */
    phase=1;
//SB:m    density = MATCalcFluidDensityNode(phase,i,theta);
	density = mfp_vector[phase]->Density();
	if(PCSGetNODValueIndex("DENSITY2",timelevel)>0)
      SetNodeVal(i,PCSGetNODValueIndex("DENSITY2",timelevel),density);
  }

}

/**************************************************************************/
/* ROCKFLOW - Function: MPCCalcSecondaryVariables
                                                                          */
/* Task:
   Calculate secondary variables
                                                                          */
/* Programming:
   05/2003 OK derived from LOPCalcSecondaryVariables_JDJ (OK/JDJ)
                                                                          */
/**************************************************************************/
void MPCCalcSecondaryVariables(void)
{
  long i;
  int j;
  double p_cap,mass_fraction,p_gas,p_liquid;
  double saturation_liquid,saturation_gas;
  int phase,comp,timelevel;
  double density;
  double theta=1.0;
  int pcs_process_number=0;
  double gp[3]={0.0,0.0,0.0};
  CFluidProperties *m_mfp = NULL;
  long* elemente = NULL;
  long group;
  int no_1d_elements,no_2d_elements,no_3d_elements;
  double p_cap_sum;
  CMediumProperties *m_mmp = NULL;

  timelevel=1;
  for (i=0;i<NodeListLength;i++) {
   //ToDo for Richards models
    /*-------------------------------------------------------------------------*/
    /* saturations */
     /* liquid saturation S^l */
    phase=1;
	pcs_process_number=1;
    saturation_liquid = GetNodeVal(i,PCSGetNODValueIndex("SATURATION2",\
                    timelevel));
	saturation_liquid = MRange(0.0,saturation_liquid,1.0);
    /* gas saturation S^g */
    phase=0;
    saturation_gas = MRange(0.0,(1.0-saturation_liquid),1.0);
    SetNodeVal(i,PCSGetNODValueIndex("SATURATION1",timelevel),saturation_gas);
//    SetNodeVal(i,PCSGetNODValueIndex("SATURATION1",0),saturation_gas);
    /*-------------------------------------------------------------------------*/
    /* capillary pressure - p_c(S) */
    phase=1;
	pcs_process_number=0;
    p_cap_sum = 0.;
    elemente = GetNode1DElems(i,&no_1d_elements);
    for(j=0;j<no_1d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,0.0,phase,0.0);
      m_mmp->mode = 0;
      //p_cap = MATCalcNodeCapillaryPressure(activ_phase, elemente[j], number, (double) timelevel);
      p_cap_sum += p_cap;
    }
    elemente = GetNode2DElems(i,&no_2d_elements);
    for(j=0;j<no_2d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,0.0,phase,0.0);
      m_mmp->mode = 0;
      p_cap_sum += p_cap;
    }
    elemente = GetNode3DElems(i,&no_3d_elements);
    for(j=0;j<no_3d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,0.0,phase,0.0);
      m_mmp->mode = 0;
      p_cap_sum += p_cap;
    }
    p_cap_sum /= (double) (no_1d_elements + no_2d_elements + no_3d_elements);
//OK_MMP    p_cap = MATCalcNodeCapillaryPressure(phase,index,i,0.0);
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",0),0.0);
    //
//OK_MMP    p_cap = MATCalcNodeCapillaryPressure(phase,index,i,theta);
	pcs_process_number=1;
    p_cap_sum = 0.;
    elemente = GetNode1DElems(i,&no_1d_elements);
    for(j=0;j<no_1d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,theta,phase,0.0);
      m_mmp->mode = 0;
      //p_cap = MATCalcNodeCapillaryPressure(activ_phase, elemente[j], number, (double) timelevel);
      p_cap_sum += p_cap;
    }
    elemente = GetNode2DElems(i,&no_2d_elements);
    for(j=0;j<no_2d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,theta,phase,0.0);
      m_mmp->mode = 0;
      p_cap_sum += p_cap;
    }
    elemente = GetNode3DElems(i,&no_3d_elements);
    for(j=0;j<no_3d_elements;j++){
      group = ElGetElementGroupNumber(elemente[j]);
      m_mmp = mmp_vector[group];
      m_mmp->mode = 1;
      p_cap = m_mmp->CapillaryPressureFunction(i,NULL,theta,phase,0.0);
      m_mmp->mode = 0;
      p_cap_sum += p_cap;
    }
    p_cap_sum /= (double) (no_1d_elements + no_2d_elements + no_3d_elements);
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap_sum);
    /* Phasendruck fuer 2. Phase: p^l = p^g - p_c(S)*/
//OK!!! nicht richtig
    timelevel=0;
  if(aktueller_zeitschritt==0) {
	pcs_process_number=0;
    p_gas = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE1",timelevel));
	pcs_process_number=0;
    p_cap = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel));
  }
  else {
	pcs_process_number=0;
    p_gas = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE1",1));
	pcs_process_number=1;
    p_cap = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",1));
  } 

    p_liquid = p_gas - p_cap;
	pcs_process_number=0;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel),p_liquid);
    timelevel=1;
    p_gas = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE1",timelevel));
	pcs_process_number=1;
    p_cap = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel));
    p_liquid = p_gas - p_cap;
	pcs_process_number=0;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel),p_liquid);
    /*-------------------------------------------------------------------------*/
    /* mass fractions */
     /* X_w^g */
    phase=0;
    comp=1;
	pcs_process_number=1;
  if(aktueller_zeitschritt==0)
    mass_fraction = 0.0;
  else{
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    density = m_mfp->Density();
    mass_fraction = m_mfp->MassFraction(i,comp,gp,theta);
    //mass_fraction = MATCalcMassFractionNode(phase,comp,0,0.0,0.0,i,theta);
  }
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION2",timelevel),mass_fraction);
     /* X^a_l */
    phase=1;
    comp=0;
	pcs_process_number=0;
  if(aktueller_zeitschritt==0)
    mass_fraction = 0.0;
  else{
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    density = m_mfp->Density();
    mass_fraction = m_mfp->MassFraction(i,comp,gp,theta);
    //mass_fraction = MATCalcMassFractionNode(phase,comp,0,0.0,0.0,i,theta);
  }
    SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION1",timelevel),mass_fraction);
	SetNodeVal(i,PCSGetNODValueIndex("MASS_FRACTION1",0),mass_fraction);
    /*-------------------------------------------------------------------------*/
    /* densities */
     /* rho^g */
    phase=0;
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    density = m_mfp->Density();
    //density = MATCalcFluidDensityNode(phase,i,theta);
	if(PCSGetNODValueIndex("DENSITY1",timelevel)>0)
      SetNodeVal(i,PCSGetNODValueIndex("DENSITY1",timelevel),density);
     /* rho^l */
    phase=1;
    m_mfp = mfp_vector[phase];
    m_mfp->mode=1;
    density = m_mfp->Density();
    //density = MATCalcFluidDensityNode(phase,i,theta);
	if(PCSGetNODValueIndex("DENSITY2",timelevel)>0)
      SetNodeVal(i,PCSGetNODValueIndex("DENSITY2",timelevel),density);
  }
}

/*************************************************************************
ROCKFLOW - Function: MPCSetICRichards
Task: 
Programming: 05/2003 OK Implementation
last modified:
**************************************************************************/
void MPCSetICRichards(int dummy)
{
  int timelevel;
  int nidx;
  long i;
  double p_liquid,p_cap;
  dummy=dummy;

  timelevel=0;
  nidx = PCSGetNODValueIndex("SATURATION1",timelevel);
//OK_IC  SetInitialConditions("SATURATION1",nidx);
  nidx = PCSGetNODValueIndex("SATURATION2",timelevel);
//OK_IC  SetInitialConditions("SATURATION2",nidx);
  nidx = PCSGetNODValueIndex("PRESSURE1",timelevel);
//OK_IC  SetInitialConditions("PRESSURE1",nidx);
  timelevel=1;
  nidx = PCSGetNODValueIndex("SATURATION1",timelevel);
//OK_IC  SetInitialConditions("SATURATION1",nidx);
  nidx = PCSGetNODValueIndex("SATURATION2",timelevel);
//OK_IC  SetInitialConditions("SATURATION2",nidx);
  nidx = PCSGetNODValueIndex("PRESSURE1",timelevel);
//OK_IC  SetInitialConditions("PRESSURE1",nidx);

  for (i=0;i<NodeListLength;i++) {
    timelevel=0;
    p_liquid = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel));
    p_cap = -1.0*p_liquid;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
    timelevel=1;
    p_liquid = GetNodeVal(i,PCSGetNODValueIndex("PRESSURE2",timelevel));
    p_cap = -1.0*p_liquid;
    SetNodeVal(i,PCSGetNODValueIndex("PRESSURE_CAP",timelevel),p_cap);
  }

  //OK_BC if(!BCGroupExists("SATURATION1")) SetBoundaryConditions("SATURATION1");
  //OK_BC if(!BCGroupExists("SATURATION2")) SetBoundaryConditions("SATURATION2");

  //OK_BC ExecuteAllBoundaryConditions();

/*
  timelevel=0;
  NodeSetBoundaryConditions("SATURATION1",\
                            PCSGetNODValueIndex("SATURATION1",timelevel), NULL);
  NodeSetBoundaryConditions("SATURATION2",\
                            PCSGetNODValueIndex("SATURATION2",timelevel), NULL);
*/
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet("RICHARDS_FLOW");
  if(!m_pcs){
    cout << "MPCSetICRichards: no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  timelevel=1;
  NodeSetBoundaryConditions("SATURATION1",\
                            PCSGetNODValueIndex("SATURATION1",timelevel),\
                            m_pcs->pcs_type_name);
  NodeSetBoundaryConditions("SATURATION2",\
                            PCSGetNODValueIndex("SATURATION2",timelevel),\
                            m_pcs->pcs_type_name);
}

/*************************************************************************
ROCKFLOW - Function: 
Task: 
Programming: 
05/2003 OK Implementation
06/2005 OK PCS
last modified:
**************************************************************************/
void MPCSetIC(int comp)
{
  int timelevel=0;
  //OK_BC ExecuteAllBoundaryConditions();
  //----------------------------------------------------------------------
  CRFProcess* m_pcs = NULL;
  m_pcs = PCSGet("RICHARDS_FLOW");
  if(!m_pcs){
    cout << "MPCSetICRichards: no PCS data" << endl;
    return;
  }
  //----------------------------------------------------------------------
  /* Primary variables */
  if(comp==0) {
    timelevel=0;
    NodeSetBoundaryConditions("PRESSURE1",\
                              PCSGetNODValueIndex("PRESSURE1",timelevel),\
                              m_pcs->pcs_type_name);
    timelevel=1;
    NodeSetBoundaryConditions("PRESSURE1",\
                              PCSGetNODValueIndex("PRESSURE1",timelevel),\
                              m_pcs->pcs_type_name);
  }
  else if(comp==1) {
    timelevel=0;
    NodeSetBoundaryConditions("SATURATION2",\
                              PCSGetNODValueIndex("SATURATION2",timelevel),\
                              m_pcs->pcs_type_name);
    timelevel=1;
    NodeSetBoundaryConditions("SATURATION2",\
                              PCSGetNODValueIndex("SATURATION2",timelevel),\
                              m_pcs->pcs_type_name);
  }
}



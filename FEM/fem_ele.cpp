/*
   The members of class Element definitions.
   Designed and programmed by WW, 06/2004
*/

#include "stdafx.h"                    /* MFC */
#include "makros.h"
#include <iostream>

#include "fem_ele_std.h"

/* Objekte */
#include "rf_pcs.h" //OK_MOD"
#include "nodes.h"
//#include "elements.h"
#include "mathlib.h"
#include "femlib.h"
#include "matrix_class.h"
// MSHLib
#include "msh_elem.h"
// Will be removed when new FEM is ready
//=============================================
FiniteElement::CElement *elem_dm = NULL;
//=============================================


namespace FiniteElement{
  
/**************************************************************************
FEMLib-Method:
Task: Constructor of class CElement
Programing:
01/2005 WW Implementation
01/2005 OK 1D case
Last modified:
**************************************************************************/
CElement::CElement(const int CoordFlag, const int order)
        :MeshElement(NULL), Order(order),ele_dim(1), nGaussPoints(1),nGauss(1),
         ShapeFunction(NULL), ShapeFunctionHQ(NULL),
         GradShapeFunction(NULL),GradShapeFunctionHQ(NULL),
         T_Flag(false), F_Flag(false), D_Flag(0)
{
    int i;
	//
    dim = CoordFlag/10;
	coordinate_system = CoordFlag;
    for(i=0; i<4; i++) unit[i] = 0.0;
    switch(dim)
    {
        case 1: //OK
          // Memory allocated for maxium 3 nodes elements
          Jacobian = new double[1];   
          invJacobian = new double[1];   
          shapefct = new double[2];
          shapefctHQ = new double[3];
          dshapefct = new double[6];
          dshapefctHQ = new double[9];
          break;
        case 2:
          // Memory allocated for maxium 9 nodes elements
          Jacobian = new double[4];   
          invJacobian = new double[4];   
          shapefct = new double[4];
          shapefctHQ = new double[9];
          dshapefct = new double[18];
          dshapefctHQ = new double[18];
          break;
        case 3:
          // Memory allocated for maxium 20 nodes elements
          Jacobian = new double[9];   
          invJacobian = new double[9];   
          shapefct = new double[8];
          shapefctHQ = new double[20];
          dshapefct = new double[24];
          dshapefctHQ = new double[60];
          //
          break;
     }
     time_unit_factor = 1.0;
     if(M_Process)  
       D_Flag = 4;
     if(MH_Process)  
       D_Flag = 41;
     F_Flag = H_Process;
     T_Flag = T_Process;
     

}


//  Destructor of class Element 
CElement::~CElement()
{
   delete  Jacobian;   
   delete  invJacobian;   
   delete  shapefct;
   delete  shapefctHQ;
   delete  dshapefct;
   delete  dshapefctHQ;
   Jacobian = NULL;   
   shapefct = NULL;
   dshapefct = NULL;
   dshapefctHQ = NULL;
   shapefctHQ = NULL;
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
06/2004 WW Implementation
Last modified:
**************************************************************************/
void CElement::ConfigElement(CElem* MElement, bool FaceIntegration)
{
	int i; 
    MeshElement = MElement;
	Index = MeshElement->GetIndex();
    nnodes = MeshElement->nnodes;
	nnodesHQ = MeshElement->nnodesHQ;

	ConfigNumerics(MeshElement->GetElementType());
	if (MeshElement->quadratic) nNodes = nnodesHQ;
	else nNodes = nnodes;

    // Node indices    
    for(i=0; i<nNodes; i++)
       nodes[i] = MeshElement->nodes_index[i];

	// Put coordinates of nodes to buffer to enhance the computation
    if(coordinate_system%10==2&&!FaceIntegration) // Z has number
	{
        switch(dim)
		{
            case 1:
              for(i=0; i<nNodes; i++)
              {
                 X[i] = MeshElement->nodes[i]->Z();  
		         Y[i] = MeshElement->nodes[i]->Y();       
		         Z[i] = MeshElement->nodes[i]->X();        
	          }
              break;
			case 2:
              for(i=0; i<nNodes; i++)
              {
                 X[i] = MeshElement->nodes[i]->X();  
		         Y[i] = MeshElement->nodes[i]->Z();       
		         Z[i] = MeshElement->nodes[i]->Y();        
	          }
              break;
			case 3:
              if(ele_dim==1||ele_dim==2)
			  {
                 for(i=0; i<nNodes; i++)
                 {
                    X[i] =  (*MeshElement->tranform_tensor)(0,0)
						     *(MeshElement->nodes[i]->X()-MeshElement->nodes[0]->X())
		                   +(*MeshElement->tranform_tensor)(1,0)
						     *(MeshElement->nodes[i]->Y()-MeshElement->nodes[0]->Y())
				           +(*MeshElement->tranform_tensor)(2,0)
						     *(MeshElement->nodes[i]->Z()-MeshElement->nodes[0]->Z());
                    Y[i] =  (*MeshElement->tranform_tensor)(0,1)
						     *(MeshElement->nodes[i]->X()-MeshElement->nodes[0]->X())
		                   +(*MeshElement->tranform_tensor)(1,1)
						     *(MeshElement->nodes[i]->Y()-MeshElement->nodes[0]->Y())
				           +(*MeshElement->tranform_tensor)(2,1)
						     *(MeshElement->nodes[i]->Z()-MeshElement->nodes[0]->Z());
		            Z[i] = MeshElement->nodes[i]->Z();        
	             }               
			  }
			  else
			  {
                 for(i=0; i<nNodes; i++)
                 {
                    X[i] = MeshElement->nodes[i]->X();  
                    Y[i] = MeshElement->nodes[i]->Y();       
                    Z[i] = MeshElement->nodes[i]->Z();        
	             }
			  }
              break;
		}
	}
	else
	{
       for(i=0; i<nNodes; i++)
       {
           X[i] = MeshElement->nodes[i]->X();  
		   Y[i] = MeshElement->nodes[i]->Y();       
		   Z[i] = MeshElement->nodes[i]->Z();        
	   }
    }
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
07/2005 WW Implementation
Last modified:
**************************************************************************/
void CElement::setOrder(const int order)
{
    Order = order;
	if(Order==1) nNodes = nnodes;
	else if (Order==2) nNodes = nnodesHQ;   
}



/**************************************************************************
FEMLib-Method:
Task: 
Programing:
06/2004 WW Implementation
02/2005 OK Case 1: line elements
Last modified:
**************************************************************************/
void CElement::ConfigNumerics(const int EleType)
{
  // nGauss = GetNumericsGaussPoints(ElementType);
   switch(EleType)
   {
	 case 1: // Line
	   ele_dim =1;
       nGauss = 2;
	   nGaussPoints = nGauss;
       ShapeFunction = ShapeFunctionLine;
	   GradShapeFunction = GradShapeFunctionLine;
	   break;    
	 case 2: // Quadrilateral 
	   ele_dim =2;
       nGauss = 3;
	   nGaussPoints = nGauss*nGauss;
       ShapeFunction = ShapeFunctionQuad;
       ShapeFunctionHQ = ShapeFunctionQuadHQ;
	   GradShapeFunction = GradShapeFunctionQuad;
	   GradShapeFunctionHQ = GradShapeFunctionQuadHQ;
       if(Order==1) nGauss = GetNumericsGaussPoints(EleType);
	   break;    
     case 3: // Hexahedra 
       ele_dim =3;
       nGauss = 3;
	   nGaussPoints = nGauss*nGauss*nGauss;
       ShapeFunction = ShapeFunctionHex;
       ShapeFunctionHQ = ShapeFunctionHexHQ;
	   GradShapeFunction = GradShapeFunctionHex;
	   GradShapeFunctionHQ = GradShapeFunctionHexHQ;
       if(Order==1) nGauss = GetNumericsGaussPoints(EleType);
	   break;
     case 4: // Triangle 
       ele_dim =2;
	   nGaussPoints = nGauss = 3;  // Fixed to 3
       ShapeFunction = ShapeFunctionTri;
       ShapeFunctionHQ = ShapeFunctionTriHQ;
	   GradShapeFunction = GradShapeFunctionTri;
	   GradShapeFunctionHQ = GradShapeFunctionTriHQ;
	   break;
     case 5: // Tedrahedra 
       ele_dim =3;
	   nGaussPoints = nGauss = 15;  // Fixed to 5
       ShapeFunction = ShapeFunctionTet;
       ShapeFunctionHQ = ShapeFunctionTetHQ;
	   GradShapeFunction = GradShapeFunctionTet;
	   GradShapeFunctionHQ = GradShapeFunctionTetHQ;
	   break;
     case 6: // Prism 
       ele_dim =3;
	   nGaussPoints = 6;  // Fixed to 9
	   nGauss = 3;        // Fixed to 3
       ShapeFunction = ShapeFunctionPri;
       ShapeFunctionHQ = ShapeFunctionPriHQ;
	   GradShapeFunction = GradShapeFunctionPri;
	   GradShapeFunctionHQ = GradShapeFunctionPriHQ;
	   break;
   }

}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
06/2004 WW Implementation
Last modified:
**************************************************************************/
double CElement::interpolate(double *nodalVal, const int order) const
{
	int nn = nnodes;
    double* inTerpo = shapefct;
    if(order==2)
	{
		nn = nnodes;
        inTerpo = shapefctHQ;
	}
    double val = 0.0;
    for(int i=0; i<nn; i++) val += nodalVal[i]*inTerpo[i];
    return val;
}

/**************************************************************************
FEMLib-Method:
Task: 
Programing:
09/2005 WW Implementation
Last modified:
**************************************************************************/
double CElement::interpolate(const int idx, CRFProcess* m_pcs, const int order)
{
	int i;  
	int nn = nnodes;
    double* inTerpo = shapefct;
    double val = 0.0;
    if(order==2)
	{
		nn = nnodes;
        inTerpo = shapefctHQ;
	}
    //
    for(i=0; i<nn; i++) 
      node_val[i] = m_pcs->GetNodeValue(nodes[i], idx);
    for(int i=0; i<nn; i++) val += node_val[i]*inTerpo[i];
    return val;
}
/**************************************************************************
FEMLib-Method:
Task: 
Programing:
09/2005 WW Implementation
Last modified:
**************************************************************************/
//double CElement::elemnt_average (const int idx, const int order)
double CElement::elemnt_average (const int idx, CRFProcess* m_pcs, const int order )
{
	int i;  
	int nn = nnodes;
	double val = 0.0;
    double* inTerpo = shapefct;
    if(order==2)
	{
		nn = nnodes;
        inTerpo = shapefctHQ;
	}
    //
    for(i=0; i<nn; i++) 
      node_val[i] = m_pcs->GetNodeValue(nodes[i], idx);
    return val/(double)nn;
}


/************************************************************************** 
  The generalized Jacobian caculation
   
   Arguments:
    const double *unit:           Local coordiantes      
   return 
        The determinate of Jacobian
 Programmaenderungen:
   06/2006     WW
02/2005 OK case 1, line elements                                                                          
06/2005 PCH Coordinate conversion for 2D element in 3D implemented 
		by calling the existing function.
**************************************************************************/
double CElement::computeJacobian(const int order)
{
	int i, j=0, k=0;
	int nodes_number = nnodes;
	double DetJac = 0.0;
    double *dN = dshapefct;
    double L;
    double dx,dy,dz;
    dx=dy=dz=0;

    if(order==2) //OK4104
    {   
        nodes_number = nnodesHQ;
        dN = dshapefctHQ;
        GradShapeFunctionHQ(dN, unit);
    }
    else 
      GradShapeFunction(dN, unit);

    for(i=0; i<ele_dim*ele_dim; i++)
       Jacobian[i] = 0.0;

    switch(ele_dim)
	{
        case 1: 
          dx = X[1]-X[0];
          dy = Y[1]-Y[0];
          dz = Z[1]-Z[0];
          L = sqrt(dx*dx+dy*dy+dz*dz);
          Jacobian[0] = L/2.; // L/2
          invJacobian[0] = 2./L; // 2/L
          DetJac = Jacobian[0];
          break;
        case 2:

		  for(i=0; i<nodes_number; i++)
	      {  
             Jacobian[0] += X[i]*dN[i];
             Jacobian[1] += Y[i]*dN[i];
             Jacobian[2] += X[i]*dN[nodes_number+i];
             Jacobian[3] += Y[i]*dN[nodes_number+i];
	      }

    	  DetJac =  Jacobian[0]*Jacobian[3]-Jacobian[1]*Jacobian[2];
          if (fabs(DetJac)<MKleinsteZahl) { 
            cout << "\n*** Jacobian: Det == 0 " << DetJac << "\n"; 
            abort(); 
          }

		  invJacobian[0] = Jacobian[3];
		  invJacobian[1] = -Jacobian[1];
		  invJacobian[2] = -Jacobian[2];
		  invJacobian[3] = Jacobian[0];
          for(i=0; i<ele_dim*ele_dim; i++)
             invJacobian[i] /= DetJac;
		  break;
		case 3:
 
		  for(i=0; i<nodes_number; i++)
	      {
             j = i+nodes_number;
             k = i+2*nodes_number;

             Jacobian[0] += X[i]*dN[i];
             Jacobian[1] += Y[i]*dN[i];
             Jacobian[2] += Z[i]*dN[i];
             
			 Jacobian[3] += X[i]*dN[j];
             Jacobian[4] += Y[i]*dN[j];
             Jacobian[5] += Z[i]*dN[j];

			 Jacobian[6] += X[i]*dN[k];
             Jacobian[7] += Y[i]*dN[k];
             Jacobian[8] += Z[i]*dN[k];
	      }

     	  DetJac = Jacobian[0]*
			         (Jacobian[4]*Jacobian[8]-Jacobian[7]*Jacobian[5])
                  +Jacobian[6]*
			         (Jacobian[1]*Jacobian[5]-Jacobian[4]*Jacobian[2])
                  +Jacobian[3]*
			         (Jacobian[2]*Jacobian[7]-Jacobian[8]*Jacobian[1]);
          if (fabs(DetJac)<MKleinsteZahl) { 
            cout << "\n*** Jacobian: DetJac == 0 " << DetJac << "\n"; 
            abort(); 
          }
		  invJacobian[0] =  Jacobian[4]*Jacobian[8]-Jacobian[7]*Jacobian[5];
		  invJacobian[1] =  Jacobian[2]*Jacobian[7]-Jacobian[1]*Jacobian[8];
		  invJacobian[2] =  Jacobian[1]*Jacobian[5]-Jacobian[2]*Jacobian[4];
          //  
		  invJacobian[3] =  Jacobian[5]*Jacobian[6]-Jacobian[8]*Jacobian[3];
		  invJacobian[4] =  Jacobian[0]*Jacobian[8]-Jacobian[6]*Jacobian[2];
		  invJacobian[5] =  Jacobian[2]*Jacobian[3]-Jacobian[5]*Jacobian[0];
          //  
		  invJacobian[6] =  Jacobian[3]*Jacobian[7]-Jacobian[6]*Jacobian[4];
		  invJacobian[7] =  Jacobian[1]*Jacobian[6]-Jacobian[7]*Jacobian[0];
		  invJacobian[8] =  Jacobian[0]*Jacobian[4]-Jacobian[3]*Jacobian[1];          
          for(i=0; i<ele_dim*ele_dim; i++)
             invJacobian[i] /= DetJac;
          break;
	}
   
    // Use absolute value (for grids by gmsh, whose orientation is clockwise)
	return fabs(DetJac);
}
/***************************************************************************
   GeoSys - Funktion: CElement::RealCoordinates

   Aufgabe:
        Mapping to real coordaintes from the local ones of quadratic traingle 
   element. 
   Formalparameter:
           E: 
             double * x         : Array of size 3, real coordiantes 
             const double *u    : Array of size 2, unit coordiantes


   Programming:
   06/2003     WW        Erste Version
   07/2005     WW        Change due to geometry element object
**************************************************************************/
void CElement::RealCoordinates(double* realXYZ)
{
	int i;
    double* df=shapefct;
	if(Order==2) df=shapefctHQ;
	for(i=0; i<3; i++)
       realXYZ[i] = 0.0;
   
    for(i=0; i<nNodes; i++)
    {
       realXYZ[0] += df[i]*X[i];
       realXYZ[1] += df[i]*Y[i];
       realXYZ[2] += df[i]*Z[i];
    }
}
/***************************************************************************
   GeoSys - Funktion: CElement::UnitCoordinates

   Aufgabe:
        Get unit coodinates from the real ones 
   element. 
   Formalparameter:
           E: 
             double * x         : Array of size 3, real coordiantes 

   Programming:
   06/2003     WW        Erste Version
**************************************************************************/
void CElement::UnitCoordinates(double *realXYZ)
{
    int i,j;
	
	setOrder(Order);
   
    for(i=0; i<3; i++)
       x1buff[i] = 0.0;

	for(i=0; i<nNodes; i++)
	{  
		x1buff[0] += X[i];
		x1buff[1] += Y[i];
		x1buff[2] += Z[i];
	}
    for(i=0; i<3; i++)    
       x1buff[i] /= (double)nNodes; 

    for(i=0; i<ele_dim; i++)
      realXYZ[i] -= x1buff[i];

    for(i=0; i<ele_dim; i++)
    {
       unit[i] = 0.0;
       for(j=0; j<ele_dim; j++)  unit[i] += invJacobian[j*ele_dim+i]*realXYZ[j];
    }

    for(i=0; i<ele_dim; i++)
        realXYZ[i] = unit[i];
   
}


/***************************************************************************
   GeoSys - Funktion: 
           CElement:: GetGaussData(const int gp)

   Aufgabe:
          Get Gauss points and weights, compute Jacobian 
   Formalparameter:
           E: 
             const int gp   : Gauss point index
 
   Programming:
   06/2004     WW        Erste Version
   08/2005     WW        Prism element
02/2005 OK case 1
**************************************************************************/
double CElement::GetGaussData(const int gp, int& gp_r, int& gp_s, int& gp_t)
{
    double fkt = 0.0;
	switch(MeshElement->GetElementType())
    {
       case 1:    // Line
          gp_r = gp;
          unit[0] = MXPGaussPkt(nGauss, gp_r);
          fkt = computeJacobian(Order)*MXPGaussFkt(nGauss, gp_r);
          break;
       case 2:    // Quadralateral 
          gp_r = (int)(gp/nGauss);
          gp_s = gp%nGauss;
          unit[0] = MXPGaussPkt(nGauss, gp_r);
          unit[1] = MXPGaussPkt(nGauss, gp_s);
          fkt = computeJacobian(Order);
          fkt *= MXPGaussFkt(nGauss, gp_r) * MXPGaussFkt(nGauss, gp_s);
          break;
       case 3:    // Hexahedra 
          gp_r = (int)(gp/(nGauss*nGauss));
          gp_s = (gp%(nGauss*nGauss));
          gp_t = gp_s%nGauss;
          gp_s /= nGauss;
          unit[0] = MXPGaussPkt(nGauss, gp_r);
          unit[1] = MXPGaussPkt(nGauss, gp_s);
          unit[2] = MXPGaussPkt(nGauss, gp_t);
          fkt = computeJacobian(Order);
          fkt *=   MXPGaussFkt(nGauss, gp_r) * MXPGaussFkt(nGauss, gp_s)
                 * MXPGaussFkt(nGauss, gp_t);
          break;
       case 4: // Triangle 
          SamplePointTriHQ(gp, unit);
          fkt = computeJacobian(Order);
          fkt *= unit[2];      // Weights
          break;
       case 5: // Tedrahedra
          SamplePointTet15(gp, unit);
          fkt = computeJacobian(Order);
          fkt *= unit[3];      // Weights
          break;
       case 6: // Prism 
          gp_r = gp%nGauss; 
          gp_s = (int)(gp/nGauss);
          gp_t = (int)(nGaussPoints/nGauss);
          unit[0] = MXPGaussPktTri(nGauss,gp_r,0);
          unit[1] = MXPGaussPktTri(nGauss,gp_r,1);
          unit[2] = MXPGaussPkt(gp_t,gp_s);
		  fkt = computeJacobian(Order);
          fkt *= MXPGaussFktTri(nGauss,gp_r)*MXPGaussFkt(gp_t, gp_s);      // Weights
          break;
    }
    return fkt; 
}

/***************************************************************************
   GeoSys - Funktion: FaceIntegration(const double *NodeVal)
   Task:   Used to treat Nuemann type boundary conditions (3D)
   Augument
       double *NodeVal : input, values of boundary conditions at all face node
                         Output, integration results of all face nodes 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CElement::FaceIntegration(double *NodeVal) 
{
  int i, gp, gp_r, gp_s;
  double fkt=0.0, det, val;
  double *sf = shapefct;

  setOrder(Order);   
  if(Order==2)
  {
     sf = shapefctHQ;
	 if(MeshElement->GetElementType()==2)
       ShapeFunctionHQ = ShapeFunctionQuadHQ8;  
  }

  det = MeshElement->GetVolume(); 
  for (i = 0; i < nNodes; i++)
     dbuff[i] = 0.0;
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      //---------------------------------------------------------
      //  Get local coordinates and weights 
 	  //  Compute Jacobian matrix and its determinate
      //---------------------------------------------------------
	  switch(MeshElement->GetElementType())
      {
          case 1:    // Line
           gp_r = gp;
           unit[0] = MXPGaussPkt(nGauss, gp_r);
           fkt = 0.5*det*MXPGaussFkt(nGauss, gp_r);
           break;  
         case 4: // Triangle 
            SamplePointTriHQ(gp, unit);
            fkt = 2.0*det*unit[2];      // Weights
          break;
            case 2:    // Quadralateral 
             gp_r = (int)(gp/nGauss);
             gp_s = gp%nGauss;
             unit[0] = MXPGaussPkt(nGauss, gp_r);
             unit[1] = MXPGaussPkt(nGauss, gp_s);
             fkt = 0.25*det*MXPGaussFkt(nGauss, gp_r) * MXPGaussFkt(nGauss, gp_s);
          break;
      }

      ComputeShapefct(Order);
      val = 0.0;
      // Interpolation of value at Gauss point
      for (i = 0; i < nNodes; i++)
         val += NodeVal[i]*sf[i];
      // Integration
      for (i = 0; i < nNodes; i++)
         dbuff[i] += val*sf[i]*fkt;
      
  }
  for (i = 0; i < nNodes; i++)
     NodeVal[i] = dbuff[i];
}

/***************************************************************************
   GeoSys - Funktion: 
           CElement::ComputeShapefct(const double *unit, const int order)

   Aufgabe:
         Compute values of shape function at integral point unit. 
   Formalparameter:
           E: 
             const double *u    : Array of size 2, unit coordiantes
             const int order    : 1, linear
			                      2, quadratic
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CElement::ComputeShapefct(const int order)
{
    if(order==1) ShapeFunction(shapefct, unit);
	else if(order==2) ShapeFunctionHQ(shapefctHQ, unit);
}


/***************************************************************************
   GeoSys - Funktion: 
           CElement::ComputeGradShapefct(const double *unit, const int order)

   Aufgabe:
         Compute values of shape function at integral point unit. 
   Formalparameter:
           E: 
             const double *unit    : Array of size 2, unit coordiantes
             const int order    : 1, linear
			                      2, quadratic
 
   Programming:
   06/2004     WW        Erste Version
**************************************************************************/
void CElement::ComputeGradShapefct(const int order)
{
	int i, j, k;
    static double Var[3]; 
    double *dN = dshapefct;
    if(order ==2) 
        dN = dshapefctHQ;
 
    setOrder(order);
	for(i=0; i<nNodes; i++)
	{
       for(j=0; j<ele_dim; j++)
	   {
          Var[j] = dN[j*nNodes+i];
		  dN[j*nNodes+i] = 0.0;
	   }
       for(j=0; j<ele_dim; j++)
	   {
          for(k=0; k<ele_dim; k++)
              dN[j*nNodes+i] += invJacobian[j*ele_dim+k]*Var[k];
       }
	}
    // 1D element in 3D
    if(dim==3&&ele_dim==1)
	{
       for(i=0; i<nNodes; i++)
       {
          for(j=1; j<dim; j++)
			  dN[j*nNodes+i] = (*MeshElement->tranform_tensor)(j)*dN[i];                          
          dN[i] *= (*MeshElement->tranform_tensor)(0);                          
	   }
	}
    // 2D element in 3D
    if(dim==3&&ele_dim==2)
	{
       for(i=0; i<nNodes*ele_dim; i++)     
          dShapefct[i] = dN[i];
       for(i=0; i<nNodes; i++)
       {
          for(j=0; j<dim; j++)
		  { 
             dN[j*nNodes+i] = 0.0;
             for(k=0; k<ele_dim; k++)          
			    dN[j*nNodes+i] += (*MeshElement->tranform_tensor)(j,k)*dShapefct[k*nNodes+i];   
		  }
	   }
	}
  
}
/***************************************************************************
  Center of reference element
   Programming:
   09/2005     WW        Erste Version
**************************************************************************/
void CElement::SetCenterGP()
{
  // Center of the reference element
  unit[0] = unit[1] = unit[2] = 0.0;
  if(MeshElement->GetElementType()==4)
    unit[0] = unit[1] = 1.0/3.0;
  else if(MeshElement->GetElementType()==5)
    unit[0] = unit[1] = unit[2] = 0.25;
}

/**************************************************************************
   GeoSys - Function: ConfigureCoupling

   Aufgabe:
         Set coupling information for local fem calculation
   Programmaenderungen:
   01/2005   WW    Erste Version
   
**************************************************************************/
void CElement::ConfigureCoupling(CRFProcess* pcs, const int *Shift, bool dyn)
{
  int i;  


  char pcsT; 
  pcsT = pcs->pcs_type_name[0];
 if(pcs->pcs_type_name.find("GAS")!=string::npos)
    pcsT = 'A';



  if(D_Flag>0) 
  {  
     if(dyn)
     {        
        Idx_dm0[0] = pcs->GetNodeValueIndex("ACCELERATION_X1");
        Idx_dm0[1] = pcs->GetNodeValueIndex("ACCELERATION_Y1");
     }
     else
     {
       Idx_dm0[0] = pcs->GetNodeValueIndex("DISPLACEMENT_X1");
       Idx_dm0[1] = pcs->GetNodeValueIndex("DISPLACEMENT_Y1");
     }
     Idx_dm1[0] = Idx_dm0[0]+1;
     Idx_dm1[1] = Idx_dm0[1]+1; 
     //     if(problem_dimension_dm==3)
     if(dim==3)
     {
        if(dyn)       
          Idx_dm0[2] = pcs->GetNodeValueIndex("ACCELERATION_Z1");
        else
          Idx_dm0[2] = pcs->GetNodeValueIndex("DISPLACEMENT_Z1");
        Idx_dm1[2] = Idx_dm0[2]+1;
     }  

     for(i=0; i<4; i++)
         NodeShift[i] = Shift[i];
  }

  switch(pcsT){
    default:
      if(T_Flag)
      {
         idx_c0 = pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'L': // Liquid flow
      if(T_Flag)
      {
         idx_c0 = pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'U': // Unconfined flow
      break;
    case 'G': // Groundwater flow
      if(T_Flag)
      {
         idx_c0 = pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'T': // Two-phase flow
      break;
    case 'C': // Componental flow
      break;
    case 'H': // heat transport
      idx_c0 = pcs->GetNodeValueIndex("PRESSURE1");
      idx_c1 = idx_c0+1;
      break;
    case 'M': // Mass transport
      if(T_Flag)
      {
         idx_c0 = pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'O': // Liquid flow
      break;
    case 'R': // Richards flow
      if(T_Flag) //if(PCSGet("HEAT_TRANSPORT"))
      {
         idx_c0 = pcs->GetNodeValueIndex("TEMPERATURE1");
         idx_c1 = idx_c0+1;           
      }
      break;
    case 'A': //Gas flow
      break;
    }
}



/***************************************************************************
   GeoSys - Funktion: 
           CElement::ComputePUCouplingMatrix

   Formalparameter:
           E: 
              const int compIndex:  0->x
                                    1->y
                                    2->z
              double *coupling_matrix_u 
   Programming:
   10/2004     WW        Erste Version
**************************************************************************/
void CElement::ComputeStrainCouplingMatrix(const int compIndex, double *coupling_matrix_u)
{
  int k,l,kl, gp, gp_r, gp_s, gp_t;
  double fkt;
  // Loop over Gauss points
  for (gp = 0; gp < nGaussPoints; gp++)
  {
      fkt = GetGaussData(gp, gp_r, gp_s, gp_t);

      ComputeGradShapefct(2);
      ComputeShapefct(1);
	  for (k=0;k<nnodes;k++) 
      {        
         for (l=0;l<nnodes;l++) {    
            kl = (nnodes*k)+l;      
            coupling_matrix_u[kl] += shapefct[k] * dshapefctHQ[nnodes*compIndex+l] * fkt;  
         }
     }
  }
}

/************************************************************************** 
 ElementMatrix::AllocateMemory
   
   Arguments:
    const int EleIndex:  Element index,
    int type          : type
                         used to get element type.
                         type = 0, for Possion type
                         type = 1, for Possion equation with deformation coupling
                         type = 2, for Navier equation
                         type = 3, for Navier equation with pressure coupling
                         type = 4, Monlithic scheme of u-p coupling
                                    default = 0.

   Programmaenderungen:
   01/2005     WW
                                                                          
**************************************************************************/
void ElementMatrix::AllocateMemory(CElem* ele, int type)
{
   int nnodes, nnodesHQ, dim, size;
   size=0;
   // The following two lines will be updated when new FEMGEO is ready
   nnodes = ele->GetVertexNumber();
   nnodesHQ = ele->GetNodesNumber_H();
   dim = ele->GetDimension();
   switch(type)
   {
      case 0: // H || T Process
        Mass = new SymMatrix(nnodes);
//        Laplace = new SymMatrix(nnodes);
        Laplace = new Matrix(nnodes, nnodes);
        RHS = new Vec(nnodes);
        break;
      case 1: // HM Partioned scheme, Flow 
        Mass = new SymMatrix(nnodes);
//        Laplace = new SymMatrix(nnodes);
        Laplace = new Matrix(nnodes, nnodes);
        RHS = new Vec(nnodes);
        CouplingB = new Matrix(nnodes, dim*nnodesHQ);   
        break;
      case 2: // M_Process only
        size = dim*nnodesHQ;  
        Stiffness = new Matrix(size, size);
        RHS = new Vec(size);
        break;
      case 3: // MH Partioned scheme, M_Process
        size = dim*nnodesHQ;  
        Stiffness = new Matrix(size, size);
        RHS = new Vec(size);
        CouplingA = new Matrix(dim*nnodesHQ, nnodes);   
        break; 
      case 4: // HM monothlic scheme
        Mass = new SymMatrix(nnodes);
//        Laplace = new SymMatrix(nnodes);
        Laplace = new Matrix(nnodes, nnodes);
        size = dim*nnodesHQ;  
        Stiffness = new Matrix(size, size);
        RHS = new Vec(size+nnodes);
        CouplingA = new Matrix(dim*nnodesHQ, nnodes);   
        CouplingB = new Matrix(nnodes, dim*nnodesHQ);   
        break;
   }
}

/************************************************************************** 
 ElementMatrix::ElementMatrix
   
   Arguments:
      const int EleIndex:  Element index,
                           used to get element type.
   Programmaenderungen:
   01/2005     WW
                                                                          
**************************************************************************/
ElementMatrix::~ElementMatrix()
{
   if(Mass) delete Mass;
   if(Laplace)delete Laplace;
   if(RHS) delete RHS;
   if(CouplingA) delete CouplingA;
   if(CouplingB) delete CouplingB;
   Mass = NULL;
   Laplace = NULL;
   RHS = NULL;
   CouplingA = NULL;
   CouplingB = NULL;
}

} // end namespace FiniteElement

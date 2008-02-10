/**************************************************************************
Task: Linear equation
Programing:
11/2007 WW/
**************************************************************************/
#include "stdafx.h" /* MFC */

//
// NEW_EQS To be removed
#ifdef NEW_EQS    //1.11.2007 WW
#include <iomanip>
#if defined(USE_MPI) 
#include <mpi.h>
#include "par_ddc.h"
#endif

#include"rf_num_new.h"
#include"matrix_class.h"
#include"equation_class.h"
vector<Math_Group::Linear_EQS *> EQS_Vector; 
//
namespace Math_Group
{
/**************************************************************************
Task: Linear equation::Constructor
Programing:
10/2007 WW/
**************************************************************************/
Linear_EQS::Linear_EQS(const SparseTable &sparse_table, 
                       const int dof, bool messg):message(messg)
{
  long i, size;
  A = new CSparseMatrix(sparse_table, dof);
  size = A->Dim();
#if defined(USE_MPI)
  prec_M =NULL;
  x = NULL;  
  cpu_time = 0.0; 
#else
  x = new double[size];
#endif
  b = new double[size];
  //
  for(i=0; i<size; i++)
  {
#ifndef USE_MPI
    x[i] = 0.;
#endif
    b[i] = 0.;
  }
  iter = 0;
  bNorm = error = 1.0e10;  
  size_global = 0;

}
#if defined(USE_MPI)
/**************************************************************************
Task: Linear equation::Constructor
Programing:
12/2007 WW/
**************************************************************************/
Linear_EQS::Linear_EQS(const long size)
{
  long i;
  A = NULL;
  b = NULL;
  size_global = size;
  x = new double[size];
  //
  for(i=0; i<size; i++)
    x[i] = 0.;
  iter = 0;
  bNorm = error = 1.0e10;  
}
#endif
/**************************************************************************
Task: Linear equation::Destructor
Programing:
10/2007 WW/
**************************************************************************/
Linear_EQS::~Linear_EQS()
{
  if(A) delete A;
  if(x) delete [] x;
  if(b) delete [] b;
  //
  A = NULL;
  x = NULL;
  b = NULL;
  //
}
/**************************************************************************
Task: Linear equation::
Programing:
10/2007 WW/
**************************************************************************/
void Linear_EQS::ConfigNumerics(CNumerics *m_num, const long n)
{
  //
  int i, nbuffer = 0; // Number of temperary float arrays
  precond_type = m_num->ls_precond;
  solver_type = m_num->ls_method;
  switch(solver_type)
  {
    case 1:
      solver_name = "Gauss"; 
      break;
    case 2:
      solver_name = "BiCGSTab"; 
      nbuffer = 8;
      break;
    case 3:
      solver_name = "BiCG";
      break;
    case 4:
      solver_name = "QMRCGStab"; 
      break;
    case 5:
      solver_name = "CG"; 
      nbuffer = 3;
      break;
    case 6:
      solver_name = "CGNR"; 
      break;
    case 7:
      solver_name = "CGS"; 
      nbuffer = 9;
      break;
    case 8:
      solver_name = "Richardson"; 
      break;
    case 9:
      solver_name = "JOR";
      break;
    case 10:
      solver_name = "SOR";
      break;
    case 11:
      solver_name = "AMG1R5";
      break;
    case 12:
      solver_name = "UMF"; 
      break;
  }
  // Buffer
#if defined(USE_MPI)
  long size = A->Dim()+A->Dof()*dom->BSize();
#else
  long size = A->Dim();
#endif
  for(i=0; i<nbuffer; i++) 
  {
    double *new_array = new double[size];
    f_buffer.push_back(new_array);
  }  
#if defined(USE_MPI)
  if(!x)
   x = new double[size];
#if defined(NEW_BREDUCE)
  // For concatenate border entries
  double *catb = NULL;
  f_buffer.push_back(catb);
#endif
  // Buffer for a global array
  double *x_g_buffer = new double[n];
  f_buffer.push_back(x_g_buffer);
  // Buffer for border array
  double *new_array_b = new double[A->Dof()*dom->BSize()];
  f_buffer.push_back(new_array_b);
  //
#endif
  //---------------------------------------------
  switch(precond_type)
  {
    case 1:
      precond_name = "Jacobi";
#if defined(USE_MPI)
      prec_M = new double[size];
#endif
      break;
    case 100:
     // precond_name = "ILU"; break;
     // If ILU is ready, remove follows
     // ---------------------------------------------- 
     precond_name = "ILU not available. Use Jacobi"; 
     precond_type = 1;
#if defined(USE_MPI)
     prec_M = new double[size];
#endif
     // ---------------------------------------------- 
     break;
    default:
      precond_name = "No preconditioner"; break;
  }
  //  
  //
  max_iter = m_num->ls_max_iterations;
  tol = m_num->ls_error_tolerance;
  //
}
/**************************************************************************
Task: Linear equation::Alocate memory for solver
Programing:
11/2007 WW/
**************************************************************************/
void Linear_EQS::Initialize()
{
  (*A) = 0.;
  for(long i=0; i<A->Dim(); i++)
    b[i] = 0.;
  error = 1.0e10;                     
}
/**************************************************************************
Task: Linear equation::Alocate memory for solver
Programing:
11/2007 WW/
**************************************************************************/
void Linear_EQS::Clean()
{
#if defined(USE_MPI)
  double cpu_time_local = -MPI_Wtime();  
#endif
  for(int i=0; i<(int)f_buffer.size(); i++)
  {
    if(f_buffer[i]) delete [] f_buffer[i];
    f_buffer[i] = NULL;
  }
  f_buffer.clear();
#if defined(USE_MPI)
  if(prec_M) delete [] prec_M; 
  prec_M = NULL; 
  cpu_time_local += MPI_Wtime();
  cpu_time += cpu_time_local;     
#endif  
}
/**************************************************************************
Task: Linear equation::Write
Programing:
11/2007 WW/
**************************************************************************/
void Linear_EQS::Write(ostream &os)   
{
  A->Write(os);
  //
  os<<" b ( RHS): " <<endl;  
  os.width(10);
  os.precision(6); 
  //
  for(long i=0; i<A->Dim(); i++)
    os<<setw(10)<<i<<" "<<setw(15)<<b[i]<<endl;
}
/**************************************************************************
Task: Linear equation::Solver
Programing:
11/2007 WW/
**************************************************************************/
#if defined(USE_MPI)
int Linear_EQS::Solver(double *xg, const long n)
{
  //
  double cpu_time_local = -MPI_Wtime();  
  iter = 0;
  ComputePreconditioner();
  size_global = n; 
  switch(solver_type)
  {
    case 2:
      iter = BiCGStab(xg, n);
      break;
    case 3:
      iter = BiCG(xg, n);
      break;
    case 5:
      iter = CG(xg, n);
      break; 
    case 7:
      iter = CGS(xg, n);
      break;   
  }
  cpu_time_local += MPI_Wtime();
  cpu_time += cpu_time_local;   
  return iter; 
}

#else
int Linear_EQS::Solver()
{
  //
  iter = 0;
  ComputePreconditioner();
  switch(solver_type)
  {
    case 1:
      return Gauss();
    case 2:
      return BiCGStab();
    case 3:
      return BiCG();
    case 4:
      return QMRCGStab();
    case 5:
      return CG(); 
    case 6:
      return CGNR();
    case 7:
      return CGS();
    case 8:
      return Richardson();
    case 9:
      return JOR();
    case 10:
      return SOR();
    case 11:
      return AMG1R5();
    case 12:
      return UMF();
  }
  return -1; 
}
#endif
// Preconditioners
/**************************************************************************
Task: Preconditioners
Programing:
10/2007 WW
**************************************************************************/
void Linear_EQS::ComputePreconditioner()
{
  switch(precond_type)
  {
    case 1:
#if defined(USE_MPI)
      ComputePreconditioner_Jacobi();
#endif
      return;
    case 100:
      ComputePreconditioner_ILU();
      return;
    default:
      return;
  }
}
/**************************************************************************
Task: Linear equation::SetKnownXi
      Configure equation system when one entry of the vector of 
      unknown is given 
Programing:
10/2007 WW/
**************************************************************************/
void Linear_EQS::SetKnownX_i(const long i, const double x_i)
{
   A->Diagonize(i, x_i, b);  
}
/**************************************************************************
Task: Linear equation::Preconditioner
Programing:
08/2007 WW/
**************************************************************************/
void Linear_EQS::Precond(double *vec_s, double *vec_r)
{ 
  bool pre = true;
  switch(precond_type)
  {
    case 1:
#if defined(USE_MPI)
      Precond_Jacobi(vec_s, vec_r);
#else
      A->Precond_Jacobi(vec_s, vec_r);
#endif
      break;
    case 100:
      pre = false; //A->Precond_ILU(vec_s, vec_r);
      break;
    default:
      pre = false; //A->Precond_ILU(vec_s, vec_r);
      break; 
  }
  if(!pre) 
  {
#if defined(USE_MPI)
     for(long i=0; i<A->Dim()+A->Dof()*dom->BSize(); i++)
#else
     for(long i=0; i<A->Dim(); i++)
#endif
       vec_r[i] = vec_s[i];
  } 
}
/*\!
********************************************************************
   Dot production of two vectors
   Programm:  
   10/2007 WW
   12/2007 WW  Parallel
********************************************************************/
double Linear_EQS::dot (const double *xx,  const double *yy)
{
   long i;
   double val = 0.; 
#if defined(USE_MPI)
   long size = A->Dim(); 
   double val_i = dom->Dot_Interior(xx,  yy);
   MPI_Allreduce(&val_i, &val, 1, MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
   const double *xx_b = &xx[size];
   const double *yy_b = &yy[size];
   for(i=0; i<A->Dof()*dom->BSize(); i++)
     val += xx_b[i]*yy_b[i];   
#else
   for(i=0; i<A->Dim(); i++)
     val += xx[i]*yy[i];
#endif
   return val;
}
/*\!
********************************************************************
   Dot production of two vectors
   Programm:  
   01/2008 WW  
********************************************************************/
double  Linear_EQS::NormX() 
{ 
#if defined(USE_MPI)
  return sqrt(dot(x, x, size_global )); 
#else
  return sqrt(dot(x, x )); 
#endif
} 
//
#if defined(USE_MPI)
/*\!
********************************************************************
   Dot production of two vectors
   Programm:  
   12/2007 WW  
********************************************************************/
double Linear_EQS::dot (const double *xx,  const double *yy, const long n)
{
   long i;
   double val = 0.; 
   for(i=0; i<n; i++)
     val += xx[i]*yy[i];
   return val;
}
/*\!
********************************************************************
   Dot production of two vectors
   Programm:  
   12/2007 WW  
********************************************************************/
inline void Linear_EQS::MatrixMulitVec(double *xx,  double *yy)
{
  long size = A->Dim(); 
  //
  double *xx_b = &xx[size];
  double *yy_b = &yy[size];
  double *vec_b = f_buffer[(long)f_buffer.size()-1];
  //
  dom->Border2Local(xx_b, xx);
  A->multiVec(xx, yy);
#if defined(NEW_BREDUCE)
  dom->ReduceBorderV(yy);
#else
  dom->Local2Border(yy, vec_b); // to buffer
  MPI_Allreduce(vec_b, yy_b, A->Dof()*dom->BSize(), MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);   
#endif
}

#endif
/*\!
********************************************************************
   ConvergeTest
   Programm:  
   09/2007 WW
********************************************************************/
void Linear_EQS::Message()
{
#ifdef USE_MPI
  if(myrank>0)
    return;
#endif
  if (!message) return;
  cout.width(10);
  cout.precision(3);
  cout.setf(ios::scientific);
  //
  //system("color 0B");
  cout<<"\n================================================\n";         
  cout<<"Linear solver "<<solver_name<<" with "<<precond_name<<":\n";         
  //cout<<"\n------------------------------------------------ \n";         
  cout<<"Iterations |";         
  cout<<"Max Iters  |";         
  cout<<"Norm of b  |"; 
  cout<<"Error      |\n"; 
  //cout<<"\n------------------------------------------------ \n";         
  cout<<setw(11)<<iter<<"|"<<setw(11)<<max_iter<< "|"
      <<setw(11)<<bNorm<< "|"<<setw(11)<<error<<"|\n";          
  cout<<"================================================\n";         
  if (iter==max_iter) cout << " Maximum iteration reached !!! \n";
	cout.flush();    
//
}
/*\!
********************************************************************
   Check if the norm of b is samll enough for convengence. 
   normb_new is given to bNorm;
   Programm:  
   09/2007 WW
********************************************************************/
inline bool Linear_EQS::CheckNormRHS(const double normb_new)
{
  if(bNorm>0.0)
  {
    if((normb_new/bNorm)<tol)
    {
       error = normb_new/bNorm;
       bNorm = normb_new;
       Message();
       return true; 
    }    
  }
  bNorm = normb_new;
  if(bNorm<DBL_MIN)
  {
     error = 0.;
     Message();
     return true; 
  }
  return false;
}
#ifndef USE_MPI
/**************************************************************************
Task: Linear equation::CG
Programing:
11/2007 WW/
**************************************************************************/
int Linear_EQS::CG()
{
  long i, size;
  double rrM1;
  double *p, *r, *s;
  //
  size = A->Dim();
  p = f_buffer[0];
  r = f_buffer[1];
  s = f_buffer[2];
  //
  double bNorm_new = Norm(b);
  // Check if the norm of b is samll enough for convengence
  if(CheckNormRHS(bNorm_new))
    return 0;
  //
  // r0 = b-Ax 
  A->multiVec(x,s); 
  for(i=0; i<size; i++)
    r[i] = b[i]-s[i];	
  //
  // Preconditioning: M^{-1}r
  Precond(r, s);
  for(i=0; i<size; i++)
    p[i] = s[i];	
  // Check the convergence
  if ((error = Norm(r)/bNorm) < tol)
  {
    Message();
    return 1; 
  }
  //
  double rr = dot(r , s);
  //
  for (iter=1; iter<=max_iter; ++iter)
  {
	A->multiVec(p, s);
	const double alpha = rr / dot(p,s);
    // Update 
    for(i=0; i<size; i++)
    {
	  x[i] += alpha*p[i];
	  r[i] -= alpha*s[i];
    }
    if ((error = Norm(r)/bNorm) < tol)
    {
      Message();
      return iter <= max_iter; 
    }
    //
    Precond(r, s);
    //
	rrM1 = rr;
	rr   = dot(s, r);
	const double beta = rr / rrM1;
    for(i=0; i<size; i++)
      p[i] = s[i] + beta*p[i];
  }
  //
  return iter <= max_iter;
}

/*************************************************************************
GeoSys-Function:
Task: BiCGStab solver 
Programming: 
10/2007 WW  
**************************************************************************/
int Linear_EQS::BiCGStab() 
{
  long i, size;
  double rho_0, rho_1, alpha, beta, omega, tt=0., norm_r = 0.;
  double *r0, *r, *s, *s_h, *t, *v, *p, *p_h;
  // 
  size = A->Dim();
  r0 = f_buffer[0];
  r = f_buffer[1];
  s = f_buffer[2];
  s_h = f_buffer[3];
  t = f_buffer[4];
  v = f_buffer[5];
  p = f_buffer[6];
  p_h = f_buffer[7];
  //
  rho_0 = alpha = omega = 1.0;
  //
  double bNorm_new = Norm(b);
  // Check if the norm of b is samll enough for convengence
  if(CheckNormRHS(bNorm_new))
    return 0;
  //
  A->multiVec(x,s);   // s as buffer
  for(i=0; i<size; i++)
  {
    r0[i] = b[i]-s[i];	  // r = b-Ax
    r[i] = r0[i];	
    v[i] = 0.;	
    p[i] = 0.;	
  }
  if ((error = Norm(r)/bNorm) < tol)
  {
    Message();
    return 0; 
  }
  //  
  for (iter = 1; iter <= max_iter; iter++)
  {
    rho_1 = dot(r0, r);
    if (fabs(rho_1) < DBL_MIN) // DBL_EPSILON
    {
      Message();
      return 0;
    }
    if (iter == 1)
    { 
      for(i=0; i<size; i++)
        p[i] = r[i];
    }
    else
    {
      beta = (rho_1/rho_0) * (alpha/omega);
      for(i=0; i<size; i++)
        p[i] = r[i] + beta * (p[i] - omega * v[i]);
    }
    // Preconditioner
    Precond(p, p_h);  
    // A M^{-1}p-->v
    A->multiVec(p_h, v);
    //
    alpha = rho_1 / dot(r0, v);
    //
    for(i=0; i<size; i++)
      s[i] = r[i] - alpha * v[i];
    if ((error = Norm(s)/bNorm) < tol) 
    {
      for(i=0; i<size; i++)
        x[i] += alpha * p_h[i];     
      Message();
      return iter <= max_iter;
    }
    //  M^{-1}s, 
    Precond(s, s_h); 
    // A* M^{-1}s
    A->multiVec(s_h, t);
    //
    tt = dot(t,t);
    if(tt>DBL_MIN)
      omega = dot(t,s) / tt;
    else
      omega = 1.0;
    // Update solution
    for(i=0; i<size; i++)
    {
      x[i] += alpha * p_h[i] + omega * s_h[i];
      r[i] = s[i] - omega * t[i];
    }
    rho_0 = rho_1;
    //  
    norm_r = Norm(r);
    if ((error = norm_r / bNorm) < tol)
    {
      Message();
      return iter <= max_iter;
    }
    if (fabs(omega) < DBL_MIN)
    {
      error = norm_r / bNorm;
      Message(); 
      return iter <= max_iter;
    }
  }
  //
  Message(); 
  //
  return iter <= max_iter;
}

/*************************************************************************
GeoSys-Function:
Task: CGS solver 
Programming: 
11/2007 WW  
**************************************************************************/
int Linear_EQS::CGS() 
{
  long i, size;
  double rho_1, rho_2, alpha, beta;
  double *r0, *r, *p, *p_h, *q, *q_h, *v, *u, *u_h;
  // 
  size = A->Dim();
  r0 = f_buffer[0];
  r = f_buffer[1];
  p = f_buffer[2];
  p_h = f_buffer[3];
  q = f_buffer[4];
  q_h = f_buffer[5];
  v = f_buffer[6];
  u = f_buffer[7];
  u_h = f_buffer[8];
  //
  rho_1 = rho_2 = 1.0;
  //
  double bNorm_new = Norm(b);
  // Check if the norm of b is samll enough for convengence
  if(CheckNormRHS(bNorm_new))
    return 0;
  //
  A->multiVec(x,v);   // v as buffer
  for(i=0; i<size; i++)
  {
    r0[i] = b[i]-v[i];	  // r = b-Ax
    r[i] = r0[i];	
    v[i] = 0.;	
  }
  if ((error = Norm(r)/bNorm) < tol)
  {
    Message();
    return 0; 
  }
  //  
  for (iter = 1; iter <= max_iter; iter++)
  {
    rho_1 = dot(r0, r);
    if (fabs(rho_1) < DBL_MIN) //  DBL_EPSILON
    {
      Message();
      return 0;
    }
    if (iter == 1)
    { 
      for(i=0; i<size; i++)
        p[i] = u[i] = r[i];
    }
    else
    {
      beta = rho_1/rho_2;
      for(i=0; i<size; i++)
      {
        u[i] = r[i] + beta * q[i];
        p[i] = u[i] + beta * (q[i] + beta * p[i]);
      } 
    }
    // Preconditioner
    Precond(p, p_h);  
    // A M^{-1}p-->v
    A->multiVec(p_h, v);
    //
    alpha = rho_1 / dot(r0, v);
    //
    for(i=0; i<size; i++)
    {
       q[i] = u[i] - alpha * v[i];
       q_h[i] = u[i] + q[i];
    }
     // Preconditioner
    Precond(q_h, u_h);  
    for(i=0; i<size; i++)
      x[i] += alpha * u_h[i];
    //
    A->multiVec(u_h, q_h);
    //
    for(i=0; i<size; i++)
      r[i] -= alpha * q_h[i];
    rho_2 = rho_1;
    if ((error = Norm(r) / bNorm) < tol)
    {
      Message();
      return iter <= max_iter;
    }
  }
  //
  Message(); 
  //
  return iter <= max_iter;
}
#endif // If not defined USE_MPI
//
//------------------------------------------------------------------------
#if defined(USE_MPI)
/*************************************************************************
GeoSys-Function:
Task: Parallel preconditioner
Programming: 
12/2007 WW  
**************************************************************************/
void Linear_EQS::ComputePreconditioner_Jacobi()
{
  double *vec_b = f_buffer[(long)f_buffer.size()-1];
  double *prec_b = &prec_M[A->Dim()];
  //
  A->DiagonalEntries(prec_M);
  //
#if defined(NEW_BREDUCE)
  dom->ReduceBorderV(prec_M);
#else
  dom->Local2Border(prec_M, vec_b); // to buffer
  MPI_Allreduce(vec_b, prec_b, A->Dof()*dom->BSize(), MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);   
#endif  
  dom->Border2Local(prec_b, prec_M); 
}
/*************************************************************************
GeoSys-Function:
Task: Parallel preconditioner, inverse
Programming: 
12/2007 WW  
**************************************************************************/
void Linear_EQS::Precond_Jacobi(const double *vec_s, double *vec_r)
{
   double val;
   for(long i=0; i<A->Dim()+A->Dof()*dom->BSize(); i++)
   {
      val = prec_M[i];
      if(val>DBL_EPSILON)
        vec_r[i] = vec_s[i]/val;
      else
        vec_r[i] = vec_s[i];
   }
}

#define TEST_MPII
/*************************************************************************
GeoSys-Function:
Task: Parallel BiCGStab solver 
    xg:  Global solution
    n:  Size of x
Programming: 
12/2007 WW  
**************************************************************************/
int Linear_EQS::BiCGStab(double *xg, const long n) 
{
  long i, size, size_b, size_t;
  double rho_0, rho_1, alpha, beta, omega, tt=0., norm_v=0.;
  double *r0, *r, *s, *s_h, *t, *v, *p, *p_h;
  double *r0_b, *r_b, *s_b, *s_h_b, *t_b, *v_b, *p_b, *p_h_b;
  double *x_b;
  // 
  size = A->Dim();
  //
  size_b = dom->BSize()*A->Dof();
  size_t = size + size_b; //[0, size_i): internal; [size_i, size_i+size_b): border. 
  r0 = f_buffer[0];      
  r = f_buffer[1];        
  s = f_buffer[2];
  s_h = f_buffer[3];
  t = f_buffer[4];
  v = f_buffer[5];
  p = f_buffer[6];
  p_h = f_buffer[7];
  //
  r0_b = &r0[size];
  r_b = &r[size];
  s_b = &s[size];
  s_h_b = &s_h[size];
  t_b = &t[size];
  v_b = &v[size];
  p_b = &p[size];
  p_h_b = &p_h[size];
  x_b = &x[size];
  //
  rho_0 = alpha = omega = 1.0;



 #ifdef  TEST_MPI 
    //TEST
 string test = "rank";
 char stro[64];  
 sprintf(stro, "%d",myrank);
 string test1 = test+(string)stro+"Assemble.txt";   
 ofstream Dum(test1.c_str(), ios::out|ios::app);
 Dum.width(20);
 Dum.precision(15);
 Dum.setf(ios::scientific);

 Dum<<"Time step: "<<aktueller_zeitschritt<<endl;
 //  if(A->Dof()==1)
 //  Dum.close();
 #endif
  


  //*** Norm b
  double bNorm_new; 
  double buff_fl = dom->Dot_Interior(b, b);  
  MPI_Allreduce(&buff_fl, &bNorm_new, 1, MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
  dom->Local2Border(b, s_b); // s_b s_h_b as buffer
  MPI_Allreduce(s_b, s_h_b, size_b, MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
  buff_fl = bNorm_new + dot(s_h_b, s_h_b, size_b); // s_h_b as b_b (rhs on border)
  bNorm_new = sqrt(buff_fl);
  // Check if the norm of b is samll enough for convengence
  if(CheckNormRHS(bNorm_new))
    return 0;
  //*** r = b-Ax
  //    A*x
  dom->Global2Local(xg, x, n);
  dom->Global2Border(xg, x_b, n);
  A->multiVec(x,s);   // s as buffer
  for(i=0; i<size; i++)
    r0[i] = b[i]-s[i];	  // r = b-Ax
  //   Collect border r
  dom->Local2Border(r0, s_b);  // s_b as buffer
  MPI_Allreduce(s_b, r0_b, size_b, MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);



  
 #ifdef  TEST_MPI 
  //TEST
  //if(A->Dof()>1)
   {
  Dum<<" |r_0|= "<<Norm(r0)<<endl;
  Dum<<" |b_0|= "<< bNorm<<endl;
  Dum<<" x  "<<endl;
  for(i=0; i<n; i++)
    Dum<<xg[i]<<endl; 
  /*
  Dum<<" b  "<<endl;
  for(i=0; i<size; i++)
    Dum<<b[i]<<endl; 
  Dum<<"inter: r  Ax  "<<endl;
  for(i=0; i<size; i++)
   Dum<<r0[i]<<endl; 
  Dum<<"border: r  Ax  "<<endl;
  for(i=0; i<size_b; i++)
   Dum<<r0_b[i]<<endl; 
  
  Dum<<"inter: x  "<<endl;
  for(i=0; i<size; i++)
   Dum<<x[i]<<endl; 
 
  Dum<<"border: x  "<<endl;
  for(i=0; i<size_b; i++)
   Dum<<x_b[i]<<endl; 
  */
   }
#endif





  // Initial. [0, size_i): internal; [size_i, size_i+size_b): border. 
  for(i=0; i<size_t; i++)
  {
    r[i] = r0[i];	
    v[i] = 0.;	
    p[i] = 0.;	
  }
  if ((error = Norm(r)/bNorm) < tol)
  {
    if(myrank==0) // Make screen output only by processor 0
      Message();
    return 0; 
  }


    
  /*
    //TEST
  //if(A->Dof()>1)
    {
      Dum<<" bNorm  "<<bNorm <<"  Norm(r) "<<  Norm(r) <<endl;
     
    } 
  */


  //  
  for (iter = 1; iter <= max_iter; iter++)
  {
    rho_1 = dot(r0, r);
    if (fabs(rho_1) < DBL_MIN) 
      break;

 
#ifdef  TEST_MPI      
    //TEST
    //  if(A->Dof()>1)
     Dum<<" rho_1  "<<rho_1 <<endl;
 #endif   
    









    if (iter == 1)
    { 
      // p[0, size_i): internal; p[size_i, size_i+size_b)-->p_b: border. 
      for(i=0; i<size_t; i++)
        p[i] = r[i];
    }
    else
    {
      beta = (rho_1/rho_0) * (alpha/omega);
      // [0, size_i): internal; [size_i, size_i+size_b): border. 
      for(i=0; i<size_t; i++)
        p[i] = r[i] + beta * (p[i] - omega * v[i]);
    }
    // Preconditioner
    Precond(p, p_h);  
    // A M^{-1}p-->v
    MatrixMulitVec(p_h, v);
    //
    alpha = rho_1 / dot(r0, v);


    
#ifdef  TEST_MPI         
    //TEST
    //  if(A->Dof()>1)
     { 
    //TEST
    Dum<<"  alpha  "<<alpha <<" dot(r0, v) "<<dot(r0, v)<<" dot(r0, r) "<<dot(r0, r)  <<endl;

   Dum<<"\n r0, r,  v   "<<endl;
   //  for(i=0; i<size_t; i++)
   //   Dum<<r0[i]<<"       "<<r[i]<<"       "<<v[i]<<endl; 
     }
#endif    


    //
    for(i=0; i<size_t; i++)
      s[i] = r[i] - alpha * v[i]; 
    norm_v = sqrt(dot(s,s));                     
    if ((error = norm_v/bNorm) < tol) 
    {
      for(i=0; i<size_t; i++)
        x[i] += alpha * p_h[i];    
      break;
    }


#ifdef  TEST_MPI     
    /*
    //TEST
    if(A->Dof()>1)
      {
    //TEST
   Dum<<"\n  norm_v/bNorm  "<<error <<endl;
   for(i=0; i<size_t; i++)
     Dum<<s[i]<<endl; 
   exit(0);
      }
    */
#endif



    //  M^{-1}s, 
    Precond(s, s_h); 
    // A* M^{-1}s
    MatrixMulitVec(s_h, t);
    //
    tt = dot(t,t);



#ifdef  TEST_MPI         
    //TEST
  //TEST
  //  if(A->Dof()>1)
    Dum<<"  tt  "<<tt <<endl;
#endif    





    if(tt>DBL_MIN)
      omega = dot(t,s) / tt;
    else
      omega = 1.0;
    // Update solution
    for(i=0; i<size_t; i++)
    {
      x[i] += alpha * p_h[i] + omega * s_h[i];
      r[i] = s[i] - omega * t[i];
    }
    rho_0 = rho_1;
    //  
    norm_v = sqrt(dot(r,r));



    
#ifdef  TEST_MPI     
     //TEST
    //TEST
    // if(A->Dof()>1)
   {
     Dum<<" sqrt(dot(r,r))  "<<norm_v  <<endl;
     // exit(0);
   }
#endif    



    if ((error = norm_v / bNorm) < tol)
      break;
    if (fabs(omega) < DBL_MIN)
    {
      error = norm_v / bNorm;
      break;
    }



#ifdef  TEST_MPI     
  //TEST
    // Dum.close();
 //MPI_Finalize();
 // exit(0);
#endif


  }
  //
  // concancert internal x
  dom->CatInnerX(xg, x, n);
  // Form the local to global  
  dom->Border2Global(x_b, xg, n); 





#ifdef  TEST_MPI
  // if(A->Dof()>1)
    {
   Dum<<" x "<<endl;
   for(i=0; i<n; i++)
     Dum<<xg[i]<<endl; 
    Dum.close();
    // exit(0); 
    }
#endif





  //
  Message(); 
  //
  return iter <= max_iter;
}
#endif
//------------------------------------------------------------------------
} // namespace
#endif // if defined(NEW_EQS)
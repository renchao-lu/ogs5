/* Copyright (C) 2002-2007 The SSI Project. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the name of the project nor the names of its contributors 
      may be used to endorse or promote products derived from this software 
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE SCALABLE SOFTWARE INFRASTRUCTURE PROJECT
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SCALABLE SOFTWARE INFRASTRUCTURE
   PROJECT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
   OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
	#include "config.h"
#else
#ifdef HAVE_CONFIG_WIN32_H
	#include "config_win32.h"
#endif
#endif

#include <math.h>
#ifdef _OPENMP
	#include <omp.h>
#endif
#ifdef USE_MPI
	#include <mpi.h>
#endif
#include "lislib.h"

/************************************************
 * lis_vector_axpy			y   <- y + alpha * x
 * lis_vector_xpay			y   <- x + alpha * y
 * lis_vector_axpyz			z   <- y + alpha * x
 * lis_vector_copy			y   <- x
 * lis_vector_scale			y   <- alpha * x
 * lis_vector_pmul			z_i <- x_i * y_i
 * lis_vector_pdiv			z_i <- x_i / y_i
 * lis_vector_set_all		x_i <- alpha
 * lis_vector_abs			x_i <- |x_i|
 * lis_vector_reciprocal	x_i <- 1 / x_i
 * lis_vector_shift			x_i <- alpha + x_i
 ************************************************/

/**********************/
/* y <- y + alpha * x */
/**********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_axpy"
int lis_vector_axpy(LIS_SCALAR alpha, LIS_VECTOR vx, LIS_VECTOR vy)
{
	int i,n;
	LIS_SCALAR *x,*y;

	LIS_DEBUG_FUNC_IN;

	n    = vx->n;
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x    = vx->value;
	y    = vy->value;
	#ifdef USE_VEC_COMP
	#pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		y[i] += alpha * x[i];
	}

	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


/**********************/
/* y <- x + alpha * y */
/**********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_xpay"
int lis_vector_xpay(LIS_VECTOR vx, LIS_SCALAR alpha, LIS_VECTOR vy)
{
	int i,n;
	LIS_SCALAR *x,*y;

	LIS_DEBUG_FUNC_IN;

	n    = vx->n;
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x    = vx->value;
	y    = vy->value;
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	#ifdef USE_VEC_COMP
	#pragma cdir nodep
	#endif
	for(i=0; i<n; i++)
	{
		y[i] = x[i] + alpha * y[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}



/**********************/
/* z <- y + alpha * x */
/**********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_axpyz"
int lis_vector_axpyz(LIS_SCALAR alpha, LIS_VECTOR vx, LIS_VECTOR vy, LIS_VECTOR vz)
{
	int i,n;
	LIS_SCALAR *x,*y,*z;

	LIS_DEBUG_FUNC_IN;

	n    = vx->n;
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n || n!=vz->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y and z is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x    = vx->value;
	y    = vy->value;
	z    = vz->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		z[i] = alpha * x[i] + y[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


/********************/
/* y <- x           */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_copy"
int lis_vector_copy(LIS_VECTOR vx, LIS_VECTOR vy)
{
	int i,n;
	LIS_SCALAR *x,*y;

	LIS_DEBUG_FUNC_IN;

	n = vx->n;
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x = vx->value;
	y = vy->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		y[i] = x[i];
	}

	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


/********************/
/* y <- alpha * x   */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_scale"
int lis_vector_scale(LIS_SCALAR alpha, LIS_VECTOR vx)
{
	int i,n;
	LIS_SCALAR *x;

	LIS_DEBUG_FUNC_IN;

	n = vx->n;

	x = vx->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		x[i] = alpha * x[i];
	}

	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


/********************/
/* x_i <- alpha     */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_set_all"
int lis_vector_set_all(LIS_SCALAR alpha, LIS_VECTOR vx)
{
	int i,n;
	LIS_SCALAR *x;

	LIS_DEBUG_FUNC_IN;

	n  = vx->n;

	x  = vx->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		x[i] = alpha;
	}

	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


/********************/
/* z_i <- x_i * y_i */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_pmul"
int lis_vector_pmul(LIS_VECTOR vx,LIS_VECTOR vy,LIS_VECTOR vz)
{
	int i,n;
	LIS_SCALAR *x,*y,*z;

	LIS_DEBUG_FUNC_IN;

	n    = vx->n;
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n || n!=vz->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y and z is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x    = vx->value;
	y    = vy->value;
	z    = vz->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		z[i] = x[i] * y[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}

/********************/
/* z_i <- x_i / y_i */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_pdiv"
int lis_vector_pdiv(LIS_VECTOR vx,LIS_VECTOR vy,LIS_VECTOR vz)
{
	int i,n;
	LIS_SCALAR *x,*y,*z;

	LIS_DEBUG_FUNC_IN;

	n    = vx->n;	
	#ifndef NO_ERROR_CHECK
		if( n!=vy->n || n!=vz->n )
		{
			LIS_SETERR(LIS_ERR_ILL_ARG,"length of vector x and y and z is not equal\n");
			return LIS_ERR_ILL_ARG;
		}
	#endif

	x    = vx->value;
	y    = vy->value;
	z    = vz->value;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		z[i] = x[i] / y[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}

/********************/
/* x_i <- |x_i|     */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_abs"
int lis_vector_abs(LIS_VECTOR vx)
{
	int i,n;
	LIS_SCALAR *x;

	LIS_DEBUG_FUNC_IN;

	x = vx->value;
	n = vx->n;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		x[i] = fabs(x[i]);
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}

/********************/
/* x_i <- 1 / x_i   */
/********************/
#undef __FUNC__
#define __FUNC__ "lis_vector_reciprocal"
int lis_vector_reciprocal(LIS_VECTOR vx)
{
	int i,n;
	LIS_SCALAR *x;

	LIS_DEBUG_FUNC_IN;

	x = vx->value;
	n = vx->n;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		x[i] = 1.0 / x[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}

/************************/
/* x_i <- alpha + x_i   */
/************************/
#undef __FUNC__
#define __FUNC__ "lis_vector_shift"
int lis_vector_shift(LIS_SCALAR alpha, LIS_VECTOR vx)
{
	int i,n;
	LIS_SCALAR *x;

	LIS_DEBUG_FUNC_IN;

	x = vx->value;
	n = vx->n;
	#ifdef USE_VEC_COMP
    #pragma cdir nodep
	#endif
	#ifdef _OPENMP
	#pragma omp parallel for private(i)
	#endif
	for(i=0; i<n; i++)
	{
		x[i] = alpha + x[i];
	}
	LIS_DEBUG_FUNC_OUT;
	return LIS_SUCCESS;
}


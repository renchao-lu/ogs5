/*
 * \file Matrix.h
 *
 *  Created on: Mar 24, 2010
 *      Author: TF
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <new>
#include <exception>
#include <stdexcept>

#ifndef NDEBUG
#include <iostream>
#endif

namespace MATHLIB {

/** Matrix represents a matrix for a numeric data type.
 */
template <class T> class Matrix
{
public:
   Matrix (size_t rows, size_t cols);
   Matrix (const Matrix &src);
   ~Matrix ();

   size_t getNRows () const { return nrows; }
   size_t getNCols () const { return ncols; }
   /**
    * \f$y = alpha * A x + beta y \f$
    */
   void axpy ( T alpha, const T* x, T beta, T* y);

   T* operator* (const T *x) const;

   inline T & operator() (size_t row, size_t col) throw (std::range_error);
   inline T & operator() (size_t row, size_t col) const throw (std::range_error);

   T* getData () { return data; }

private:
   // zero based addressing, but Fortran storage layout
   //inline size_t address(size_t i, size_t j) const { return j*rows+i; };
   // zero based addressing, C storage layout
   inline size_t address(size_t i, size_t j) const { return i*ncols+j; };

   size_t nrows;
   size_t ncols;
   T *data;
};

template<class T> Matrix<T>::Matrix (size_t rows, size_t cols)
      : nrows (rows), ncols (cols), data (new T[nrows*ncols])
{}

template<class T> Matrix<T>::Matrix (const Matrix& src) :
	nrows (src.getNRows ()), ncols (src.getNCols ()), data (new T[nrows * ncols])
{
   for (size_t i = 0; i < nrows; i++)
      for (size_t j = 0; j < ncols; j++)
         data[address(i,j)] = src (i, j);
}

template <class T> Matrix<T>::~Matrix ()
{
   delete [] data;
}

template<class T> void Matrix<T>::axpy ( T alpha, const T* x, T beta, T* y)
{
   for (size_t i(0); i<nrows; i++) {
      y[i] += beta * y[i];
      for (size_t j(0); j<ncols; j++) {
         y[i] += alpha * data[address(i,j)] * x[j];
      }
   }
}

template<class T> T* Matrix<T>::operator* (const T *x) const
{
	T *y (new T[nrows]);
	for (size_t i(0); i < nrows; i++) {
		y[i] = 0.0;
		for (size_t j(0); j < ncols; j++) {
			y[i] += data[address(i, j)] * x[j];
		}
	}

	return y;
}

template<class T> T& Matrix<T>::operator() (size_t row, size_t col)
	throw (std::range_error)
{
   if ( (row >= nrows) | ( col >= ncols) )
      throw std::range_error ("Matrix: op() range error");
   return data [address(row,col)];
}


template<class T> T& Matrix<T>::operator() (size_t row, size_t col) const
	throw (std::range_error)
{
   if ( (row >= nrows) | ( col >= ncols) )
      throw std::range_error ("Matrix: op() const range error");
   return data [address(row,col)];
}

template <class T> T sqrFrobNrm (const Matrix<T> &mat)
{
	T nrm ((T)(0));
	size_t i,j;
	for (j=0; j<mat.getNCols(); j++)
		for (i=0; i<mat.getNRows(); i++)
			nrm += mat(i,j) * mat(i,j);

	return nrm;
}

} // end namespace MATHLIB

#endif

/**
 * \file GEOLIB::Vector.h
 * 27/10/2009 LB Initial implementation
 * From: http://www.strout.net/info/coding/classlib/intro.html
 * with modifications to derive from TemplatePoint
 */

#ifndef VECTOR3_H
#define VECTOR3_H

// ** INCLUDES **
#include "TemplatePoint.h"

#include <iostream>
#include <cmath>

namespace GEOLIB {

/**
 * The GEOLIB::Vector class defines a three-dimensional vector, with appropriate
 *	operators.  (* is cross product.)
 */
template <class T>
class TemplateVector : public TemplatePoint<T>
{
public:
	TemplateVector() : TemplatePoint<T>() {};
	TemplateVector(T x1, T x2, T x3) : TemplatePoint<T>(x1, x2, x3) {};
 	TemplateVector(const TemplatePoint<T> & rhs) : TemplatePoint<T>(rhs[0], rhs[1], rhs[2])
 	{}
	~TemplateVector() {};

	// vector arithmetic

	TemplateVector operator+(const TemplateVector & pV) const
	{
		return TemplateVector(this->x[0]+pV[0], this->x[1]+pV[1], this->x[2]+pV[2] );
	}

	TemplateVector operator-(const TemplateVector & pV) const {
		TemplateVector out( this->x[0]-pV[0], this->x[1]-pV[1], this->x[2]-pV[2] );
		return out;
	}

	TemplateVector operator-() const
	{ return TemplateVector (-this->x[0], -this->x[1], -this->x[2]); }

	TemplateVector& operator+=(const TemplateVector & pV) {
		for (size_t i(0); i<3; i++) this->x[i]+=pV[i];
		return *this;
	}

	TemplateVector& operator-=(const TemplateVector & pV)
	{
		for (size_t i(0); i<3; i++) this->x[i] -= pV[i];
		return *this;
	}

	// Accessors
	T X() const { return this->x[0]; }
	T Y() const { return this->x[1]; }
	T Z() const { return this->x[2]; }
	void setX(T value) { this->x[0] = value; }
	void setY(T value) { this->x[1] = value; }
	void setZ(T value) { this->x[2] = value; }

	/// Dot product with another vector
	double Dot(const TemplateVector & pV) const
	{
		return this->x[0]*pV[0] + this->x[1]*pV[1] + this->x[2]*pV[2];
	}

	/// Cross product as the multiplication operator
	TemplateVector operator*(const TemplateVector & pV) const {
		return TemplateVector (
				this->x[1]*pV[2]-this->x[2]*pV[1],
				this->x[2]*pV[0]-this->x[0]*pV[2],
				this->x[0]*pV[1]-this->x[1]*pV[0] );
	}

	/// Cross product with another vector
	TemplateVector Cross( const TemplateVector & pV ) const
	{ return *this * pV; }

	friend double Dot( const TemplateVector & p1, const TemplateVector & p2 )
	{ return p1.Dot(p2); }

	friend TemplateVector Cross( const TemplateVector & p1, const TemplateVector & p2 )
	{ return p1 * p2; }

	TemplateVector operator*(double pR) const		// * a scalar
	{
		return TemplateVector(this->x[0]*pR, this->x[1]*pR, this->x[2]*pR);
	}

	friend TemplateVector operator*(double pR, const TemplateVector & pV)
	{
		return TemplateVector ( pV[0]*pR, pV[1]*pR, pV[2]*pR );
	}

	TemplateVector& operator*=(double pR)
	{
		for (size_t i(0); i<3; i++) this->x[i]*=pR;
		return *this;
	}

	/// Returns the squared length
	double LenSqr(void) const
	{
		return sqrNorm2 (this->getData ());
	}

	/// Returns the length
	double Length(void) const
	{ return sqrt(LenSqr()); }

	/// Projection (component of *this parallel to pV).
	/// Note: component perpendicular to pV is:  *this - Proj(pV)
	TemplateVector Proj(const TemplateVector & pV)
	{ TemplateVector out( pV * (this->Dot(pV) / pV.LenSqr()) ); return out; }

	/// Cosine of the angle between two vectors:
	double CosAng( const TemplateVector & pV)
	{ return this->Dot(pV) / (Length() * pV.Length()); }

	/// Comparison if equal
	bool operator==( const TemplateVector & pV) const
	{
		return this->x[0]==pV[0] && this->x[1]==pV[1] && this->x[2]==pV[2];
	}

	/// Comparison if not equal
	int operator!=( const TemplateVector & pV) const
	{
		return this->x[0]!=pV[0] || this->x[1]!=pV[1] || this->x[2]!=pV[2];
	}
};

typedef TemplateVector<double> Vector;

}

#endif // VECTOR3_H

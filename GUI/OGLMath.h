// this file uses JavaDoc style comment blocks for automatic extraction of source code documentation.
//In this file are implemented some mathematical entities often used in 3D graphics (vecors, points, etc.) plus some useful constants, macros and definitions

#ifndef _3D_MATH_H
#define _3D_MATH_H

#include <iostream>
#include <assert.h>
#include <math.h>
#ifdef _AFXDLL // see if we are using MFC...
#include <afxwin.h>
#endif

#ifdef HIGH_PRECISION

//! The base type for all the math helpers
typedef double real;
//! the treshold for comparisons with zero, mainly used to avoid division by zero errors
const real epsilon=1e-12;
//! defined when high precision is requested
#define REAL_IS_DOUBLE

#else

// WARNING: these pragmas below could be MSVC compiler specific
#pragma warning( push )// memorize the warning status
#pragma warning( disable : 4305 )// disable "initializing : truncation from 'const double' to 'float'" warning
#pragma warning( disable : 4244 )// disable "double to float conversion possible loss of data" warning
#pragma warning( disable : 4136 )// disable "conversion between different floating-point types" warning
#pragma warning( disable : 4309 )// disable " 'conversion' : truncation of constant value" warning
#pragma warning( disable : 4051 )// disable " 'type conversion' ; possible loss of data" warning
//! The base type for all the math helpers
typedef float real;
//! the treshold for comparisons with zero, mainly used to avoid division by zero errors
const real epsilon=1e-7;
//! defined when high precision is not requested
#define REAL_IS_FLOAT

#endif

//=============================================================================
//=============================================================================

//!A oglvector class.
/*!
The oglvector class incapsulates a classic C++ oglvector of three real values and treats them as a 3 dimensional mathematical vectors.

The most common operations between mathematical vectors (and some which involves scalars too) are defined.
*/
class oglvector
{
private:
	static int counter;//!< counts how many oglvector objects are present
	//real vec[3];//!< the actual oglvector

public:
// operators
	real vec[3];//!< the actual oglvector
	oglvector();//!< default constructor
	oglvector(const real& x,const real& y,const real& z);//!< constructs a oglvector from three values
	oglvector(oglvector& from, oglvector& to);//!< constructs a oglvector from two other vectors
	oglvector(const oglvector& other);//!< the copy constructor
	~oglvector();//!< the distructor
	real& x();//!< accessor for the x component (can be used as l-value too)
	real& y();//!< accessor for the y component (can be used as l-value too)
	real& z();//!< accessor for the z component (can be used as l-value too)
	real x() const;//!< returns the x component (r-value only)
	real y() const;//!< returns the y component (r-value only)
	real z() const;//!< returns the z component (r-value only)
	oglvector& operator=(const oglvector& other);//!< the assignment
	oglvector& operator+=(const oglvector& other);//!< the sum & assign
	oglvector& operator-=(const oglvector& other);//!< the subtract & assign
	oglvector& operator*=(const real& fact);//!< the short multiply by a scalar factor & assign
	oglvector& operator/=(const real& fact);//!< the short divide by a scalar factor & assign
	real& operator[](const int& index);//!< an "access like a C++ oglvector"
#ifdef HIGH_PRECISION
	operator double*();//!< the conversion to a 3 double elements C++ oglvector
#else
	operator float*();//!< the conversion to a 3 float elements C++ oglvector
#endif
//	operator char*();//!< the conversion of the oglvector into a textual form (null terminated string)
	real normalize();//!< normalize the oglvector
	oglvector normalized() const;//!< normalized copy of the oglvector
	real length() const;//!< get the length of the oglvector
	real length2() const;//!< get the squared length of the oglvector
	void EpsilonCorrect(const oglvector& v);//!< if the oglvector is almost equal to the origin substitute it with v
// ststic functions
	static int howMany();//!< returns how many oglvector objects exists

// friend functions	
	friend int operator==(const oglvector& v1,const oglvector& v2);//!< the equality operator
	friend int operator!=(const oglvector& v1,const oglvector& v2);//!< the inequality operator
	friend oglvector operator+(const oglvector& v1,const oglvector& v2);//!< the sum
	friend oglvector operator-(const oglvector& v1,const oglvector& v2);//!< the difference
	friend oglvector operator-(const oglvector& v1);//!< the negation
	friend real operator*(const oglvector& v1,const oglvector& v2);//!< the oglvector dot product
	friend oglvector operator^(const oglvector& v1,const oglvector& v2);//!< the oglvector cross product
	friend oglvector operator*(const oglvector& v,const real& fact);//!< the multiply a oglvector by a scalar factor
	friend oglvector operator*(const real& fact,const oglvector& v);//!< the multiply a scalar factor by a oglvector
	friend oglvector operator/(const oglvector& v,const real& fact);//!< the divide a oglvector by a scalar factor
	friend oglvector Bisect(oglvector v0,oglvector v1);//!< returns the unit oglvector which halves the arc between v0 and v1
	friend void clamp(oglvector& vec,const real& low,const real& high);//!< clamps all the oglvector components between the given tresholds
#ifdef _AFXDLL // see if we are using MFC...
#ifdef _DEBUG		//.. and if we are in a debug build
	friend CDumpContext& operator<<(CDumpContext& cd,const oglvector& vect);//!< the print oglvector to MSVC++ debug console
#endif
#endif
};
//-----------------------------------------------------------------------------
// inlines

inline oglvector::oglvector()
{
	vec[0]=vec[1]=vec[2]=0.0;
	counter++;
}

inline oglvector::oglvector(const real& x,const real& y,const real& z)
{
	vec[0]=x;
	vec[1]=y;
	vec[2]=z;
	counter++;
}

/*!
This function constructs the oglvector which goes from the oglvector \e from to the oglvector \e to.
It is useful when a oglvector has to be constructed from the difference of two other vectors.
\param from the first oglvector
\param to the second oglvector
*/
inline oglvector::oglvector(oglvector& from, oglvector& to)
{
	vec[0]=to.vec[0]-from.vec[0];
	vec[1]=to.vec[1]-from.vec[1];
	vec[2]=to.vec[2]-from.vec[2];
	counter++;
}

inline oglvector::oglvector(const oglvector& other)
{
	*this=other;
	counter++;
}

inline oglvector::~oglvector()
{
	counter--;
}

inline real& oglvector::x()
{
	return vec[0];
}

inline real& oglvector::y()
{
	return vec[1];
}

inline real& oglvector::z()
{
	return vec[2];
}

inline real oglvector::x() const
{
	return vec[0];
}

inline real oglvector::y() const
{
	return vec[1];
}

inline real oglvector::z() const
{
	return vec[2];
}

inline oglvector& oglvector::operator=(const oglvector& other)
{
	//check for 'a=a' case
	if (this==&other) return *this;
	vec[0]=other.vec[0];
	vec[1]=other.vec[1];
	vec[2]=other.vec[2];
	return *this;
}

inline oglvector& oglvector::operator+=(const oglvector& other)
{
	vec[0]+=other.vec[0];
	vec[1]+=other.vec[1];
	vec[2]+=other.vec[2];
	return *this;
}

inline oglvector& oglvector::operator-=(const oglvector& other)
{
	vec[0]-=other.vec[0];
	vec[1]-=other.vec[1];
	vec[2]-=other.vec[2];
	return *this;
}

inline oglvector& oglvector::operator*=(const real& fact)
{
	vec[0]*=fact;
	vec[1]*=fact;
	vec[2]*=fact;
	return *this;
}

inline oglvector& oglvector::operator/=(const real& fact)
{
	assert(fabs(fact) >= epsilon);
	vec[0]/=fact;
	vec[1]/=fact;
	vec[2]/=fact;
	return *this;
}

/*!
This operator redirects the access to the internal oglvector. It does make a range check on the index in debug builds trough the ANSI assert function.
It can be used on both sides of an assignment.
\return a reference to the requested element
\param index the index which should be one of 0,1,2
*/
inline real& oglvector::operator[](const int& index)
{
	assert(index>=0 && index<=2);
	return vec[index];
}

inline real oglvector::length2() const
{
	return (*this)*(*this);
}

inline real oglvector::length() const
{
	return sqrt(this->length2());
}

/*!
The counting of the oglvector objects is realized trough a static counter variable.
\return the number of oglvector objects in memory
*/
inline int oglvector::howMany()
{
	return counter;
}

/*!
this conversion operator allows to use a oglvector in places of a classic real[3]
*/
/*
inline oglvector::operator real*()
{
	return (real*)vec;
}
*/
#ifdef HIGH_PRECISION

/*!
this conversion operator allows to use a oglvector in places of a classic double[3]
*/
inline oglvector::operator double*()
{
	return (double*)vec;
}

#else

/*!
this conversion operator allows to use a oglvector in place of a classic float[3]
*/
inline oglvector::operator float*()
{
	return (float*)vec;
}

#endif

//-----------------------------------------------------------------------------
// useful constants (declaration)

extern const oglvector ORIGIN;
extern const oglvector X_AXIS;
extern const oglvector Y_AXIS;
extern const oglvector Z_AXIS;

//=============================================================================
//=============================================================================

/*!
A transformation matrix class.

The matrix class groups sixteen real values an treats them as a 4x4 matrix. Standard
C++ matrices (2 dimensional vectors) are stored by row, since for graphical applications
a column major order is preferable the access indices are internally swapped.

The most common operators between matrices (and some which involves scalars and vectors too) are defined.
*/
class tmatrix
{
private:
	static int counter;//!< counts how many matrices objects are present
	real mat[4][4];//!< the matrix data
public:
	//! Espresses how to store a tmatrix in a single 16 elements oglvector, by column or by row
	enum ordermode
	{
		COLUMN,//!< column major order
		ROW//!< row major order
	};
	tmatrix();//!< default constructor
	tmatrix(const real& val);//!< constructs a tmatrix and fills it with a value
	tmatrix(const real oglvector[16],ordermode ord=COLUMN);//!< constructs a tmatrix from a oglvector which contains a 4x4 matrix in row major or column major order
	tmatrix(const tmatrix& other);//!< the copy constructor
	~tmatrix();//!< the distructor
	tmatrix& operator-();//!< negation
	tmatrix& operator=(const tmatrix& other);//!< assignment
	tmatrix& operator+=(const tmatrix& other);//!< sum & assign
	tmatrix& operator-=(const tmatrix& other);//!< subtract & assign
	tmatrix& operator*=(const tmatrix& other);//!< multiply by a tmatrix & assign
	tmatrix& operator*=(const real& fact);//!< multiply by a scalar factor & assign
	tmatrix& operator/=(const real& fact);//!< divide by a scalar factor & assign
	real& operator()(const int& row,const int& col);//!< an "access like a C++ tmatrix"
#ifdef HIGH_PRECISION
	operator double*();//!< conversion to a 16 doubles C++ oglvector (column major order)
#else
	operator float*();//!< conversion to a 16 floats C++ oglvector (column major order)
#endif
	void loadIdentity();//!< fills the matrix with the identity matrix
//	operator char*();//!< conversion of the tmatrix into a textual form (null terminated string)
//static functions
	static int howMany();//!< returns how many tmatrix objects exists
// friend functions	
	friend int operator==(const tmatrix& t1,const tmatrix& t2);//!< the equality operator
	friend int operator!=(const tmatrix& t1,const tmatrix& t2);//!< the inequality operator
	friend tmatrix operator+(const tmatrix& t1,const tmatrix& t2);//!< the sum
	friend tmatrix operator-(const tmatrix& t1,const tmatrix& t2);//!< the difference
	friend tmatrix operator*(const tmatrix& t1,const tmatrix& t2);//!< tmatrix product
	friend tmatrix operator*(const tmatrix& tmat,const real& fact);//!< multiply a tmatrix by a scalar factor
	friend tmatrix operator*(const real& fact,const tmatrix& tmat);//!< multiply a scalar factor by a tmatrix
	friend tmatrix operator/(const tmatrix& tmat,const real& fact);//!< divide a tmatrix by a scalar factor
#ifdef _AFXDLL // see if we are using MFC
#ifdef _DEBUG
	friend CDumpContext& operator<<(CDumpContext& cd,const tmatrix& m);//!< print tmatrix to MSVC++ debug console
#endif
#endif
};

//-----------------------------------------------------------------------------
// inlines

inline tmatrix::tmatrix(const tmatrix& other)
{
	*this=other;
	counter++;
}

inline tmatrix::~tmatrix()
{
	counter--;
}

/*!
The counting of the tmatrix objects is realized trough a static counter variable.
\return the number of tmatrix objects in memory
*/
inline int tmatrix::howMany()
{
	return counter;
}

/*!
This operator redirects the access to the internal matrix. It does make a range
check on the index in debug builds trough the ANSI assert function.
It can be used on both sides of an assignment.
\return a reference to the requested element
\param row the row index which should be one of 0,1,2,3
\param col the column index which should be one of 0,1,2,3
\date 18/06/99
*/
inline real& tmatrix::operator()(const int& row,const int& col)
{
	assert(row>=0 && row<=3);
	assert(col>=0 && col<=3);
	return mat[col][row];// swap indices to store by column
}

#ifdef HIGH_PRECISION

/*!
this conversion operator allows to use a tmatrix in places where a column major
order oglvector of 16 double elements is requested (e.g. the OpenGL functions for
retrieving/setting the modelview or projection matrix).
*/
inline tmatrix::operator double*()
{
	return (double*)mat;
}

#else

/*!
this conversion operator allows to use a tmatrix in places where a column major
order oglvector of 16 float elements is requested (e.g. the OpenGL functions for
retrieving/setting the modelview or projection matrix).
*/
inline tmatrix::operator float*()
{
	return (float*)mat;
}

#endif

//-----------------------------------------------------------------------------
// useful constants


//=============================================================================
//=============================================================================


class quaternion
{
private:
	static int counter;//!< counts how many quaternion objects are present
protected:
	//real s;//!< the scalar part of a quaternion
	//oglvector v;//!< the oglvector part of a quaternion
public:
	real s;//!< the scalar part of a quaternion
	oglvector v;//!< the oglvector part of a quaternion
	quaternion();//!< default constructor
	quaternion(const real& scal, const oglvector& vec);//!< constructs a quaternion from the scalar and oglvector components
	quaternion(const real& s1,const real& s2,const real& s3,const real& s4);//!< constructs a quaternion from four real values
	quaternion(const quaternion& other);//!< the copy constructor
	virtual ~quaternion();//!< the distructor
	real& x();//!< accessor for the x component of the oglvector part (can be used as l-value too)
	real& y();//!< accessor for the y component of the oglvector part (can be used as l-value too)
	real& z();//!< accessor for the z component of the oglvector part (can be used as l-value too)
	real& w();//!< accessor for the scalar part (can be used as l-value too)
	real& scalarPart();//!< other accessor for the scalar component (can be used as l-value too)
	oglvector& vectorPart();//!< accessor for the oglvector part (can be used as l-value too)
	quaternion& operator=(const quaternion& other);//!< assignment
	quaternion& operator+=(const quaternion& other);//!< sum & assign
	quaternion& operator-=(const quaternion& other);//!< subtract & assign
	quaternion& operator*=(const quaternion& other);//!< multiply by a quaternion & assign
	quaternion& operator/=(const quaternion& other);//!< divide by a quaternion & assign
	quaternion& operator*=(const real& fact);//!< multiply by a scalar factor & assign
	quaternion& operator/=(const real& fact);//!< divide by a scalar factor & assign
//	operator float*();//!< conversion to a 4 elements C++ oglvector
//	operator char*();//!< conversion of the quaternion into a textual form (null terminated string)
	real normalize();//!< normalize the quaternion
	quaternion normalized() const;//!< normalized copy of the quaternion
	void conjugate();//!< conjugate of the quaternion
	quaternion conjugated() const;//!< conjugated copy of the quaternion
	real inverse();//!< inverse of the quaternion
	quaternion inversed() const;//!< inversed copy of the quaternion
	real length() const;//!< get the length of the quaternion
	real norm() const;//!< get the norm (similar to the squared length) of the quaternion
	virtual tmatrix getRotMatrix();//<! constructs a rotation matrix from the quaternion
// static functions
	static int howMany();//!< returns how many quaternion objects exists
// friend functions	
	friend int operator==(const quaternion& q1,const quaternion& q2);//!< the equality operator
	friend int operator!=(const quaternion& q1,const quaternion& q2);//!< the inequality operator
	friend quaternion operator+(const quaternion& q1,const quaternion& q2);//!< the sum
	friend quaternion operator-(const quaternion& q1,const quaternion& q2);//!< the difference
	friend quaternion operator-(const quaternion& q1);//!< negation
	friend quaternion operator*(const quaternion& q1,const quaternion& q2);//!< quaternion product
	friend quaternion operator*(const quaternion& q,const real& s);//!< multiply a quaternion by a scalar
	friend quaternion operator*(const real& s,const quaternion& q);//!< multiply a scalar by a quaternion
	friend quaternion operator/(const quaternion& q,const real& s);//!< divide a quaternion by a scalar factor
	friend quaternion operator/(const quaternion& q1,const quaternion& q2);//!< divide a quaternion by a quaternion
#ifdef _AFXDLL // see if we are using MFC
#ifdef _DEBUG
	friend CDumpContext& operator<<(CDumpContext& ad,const quaternion& q);//!< print quaternion to MSVC++ debug console
#endif
#endif
};

//-----------------------------------------------------------------------------
// inlines

inline quaternion::quaternion()
{
	s=0.0;
	counter++;
};

/*!
This constructor assumes an (s,\b v) form so interprets the real calues as w,x,y,z respectively.
Another common representation for quaternions, the "homogeneous" one, is x,y,x,w ordered.
\param s1 the scalar part
\param s2 the x component of the oglvector part
\param s3 the y component of the oglvector part
\param s4 the z component of the oglvector part
\date 15/06/99
*/
inline quaternion::quaternion (const real& s1,const real& s2,const real& s3,const real& s4)
:s(s1),v(s2,s3,s4)
{
	counter++;
}

inline quaternion::quaternion (const real& scal, const oglvector& vec)
:s(scal),v(vec)
{
	s=scal;
	counter++;
}

inline quaternion::~quaternion()
{
	counter--;
}

inline quaternion::quaternion(const quaternion& other)
{
	*this=other;
	counter++;
}

/*!
The counting of the quaternion objects is realized trough a static counter variable.
\return the number of quaternion objects in memory
*/
inline int quaternion::howMany()
{
	return counter;
}

inline real& quaternion::x()
{
	return v.x();
}

inline real& quaternion::y()
{
	return v.y();
}

inline real& quaternion::z()
{
	return v.z();
}

inline real& quaternion::w()
{
	return s;
}

inline real& quaternion::scalarPart()
{
	return s;
}

inline oglvector& quaternion::vectorPart()
{
	return v;
}

inline quaternion& quaternion::operator=(const quaternion& other)
{
	//check for 'a=a' case
	if (this==&other) return *this;
	s=other.s;
	v=other.v;
	return *this;
}

inline quaternion& quaternion::operator+=(const quaternion& other)
{
	s+=other.s;
	v+=other.v;
	return *this;
}

inline quaternion& quaternion::operator-=(const quaternion& other)
{
	s-=other.s;
	v-=other.v;
	return *this;
}

inline quaternion& quaternion::operator*=(const real& fact)
{
	s*=fact;
	v*=fact;
	return *this;
}

inline quaternion& quaternion::operator/=(const real& fact)
{
	assert(fabs(fact) >= epsilon);
	s/=fact;
	v/=fact;
	return *this;
}

inline real quaternion::length() const
{
	return sqrt(norm());
}

inline void quaternion::conjugate()
{
	v=-v;
}

inline quaternion quaternion::conjugated() const
{
	return quaternion(s,-v);
}

/*!
This function check if all elements of \e v1 are equal to the corresponding elements of \e v2.
*/
inline int operator==(const oglvector& v1,const oglvector& v2)
{
	if(v1.vec[0]==v2.vec[0] && v1.vec[1]==v2.vec[1] && v1.vec[2]==v2.vec[2]) return 1;
	else return 0;
}

/*!
This function check if some of the elements of \e v1 differs from the corresponding elements of \e v2.
*/
inline int operator!=(const oglvector& v1,const oglvector& v2)
{
	if(v1.vec[0]==v2.vec[0] && v1.vec[1]==v2.vec[1] && v1.vec[2]==v2.vec[2]) return 0;
	else return 1;
}


//=============================================================================
//=============================================================================

/*!
A unit lenght quaternion class.

The unitquaternion class manages quaternions of unit length, such quaternions
can be used to represent arbitrary rotations. To mantain the unit lenght property
along quaternion calculus addition and subtraction as well as multiplication and
division by scalars are not allowed (they're made private or they're overloaded
by "trap" functions).

Actually only a few member functions of the base class are overloaded since
the quaternion calculus apply with no modification to unit quaternions.
*/
class unitquaternion : public quaternion
{
private:
	unitquaternion& operator+=(const unitquaternion& other);//!< sum & assign is not allowed
	unitquaternion& operator-=(const unitquaternion& other);//!< subtract & assign is not allowed
	unitquaternion& operator*=(const real& fact);//!< multiply by a scalar factor & assign is not allowed
	unitquaternion& operator/=(const real& fact);//!< divide by a scalar factor & assign is not allowed
	unitquaternion(const oglvector& v, const real& s);//!< constructs a unitquaternion assigning directly the scalar and oglvector parts
public:
	real x() const;//!< accessor for the x component of the oglvector part
	real y() const;//!< accessor for the y component of the oglvector part
	real z() const;//!< accessor for the z component of the oglvector part
	real w() const;//!< accessor for the scalar part
	real scalarPart() const;//!< other accessor for the scalar component
	oglvector vectorPart() const;//!< accessor for the oglvector part
	unitquaternion();//!< default constructor
	unitquaternion(const real& angle, const oglvector& axis);//!< constructs a unitquaternion representing a rotation of angle radiants about axis 
	unitquaternion(const quaternion& q);//!< constructs a unitquaternion from a generic one (conversion by normalizing)
	~unitquaternion();//!< the distructor
	unitquaternion inversed() const;//!< inversed copy of the unitquaternion
	void inverse();//!< inverse of the unitquaternion
	tmatrix getRotMatrix();//<! constructs a rotation matrix from the quaternion
	void getVectorsOnSphere(oglvector& vfrom,oglvector& vto);//<! converts a unitquaternion to two vectors on a unit sphere (the extremes of a rotation)
	unitquaternion& operator*=(const unitquaternion& other);//!< multiply by another unitquaternion & assign is not allowed
// friend functions	
	friend unitquaternion operator+(const unitquaternion& q1,const unitquaternion& q2);//!< the sum is not allowed
	friend unitquaternion operator-(const unitquaternion& q1,const unitquaternion& q2);//!< the difference is not allowed
	friend unitquaternion operator*(const unitquaternion& q,const real& s);//!< multiply a unitquaternion by a scalar is not allowed
	friend unitquaternion operator*(const real& s,const unitquaternion& q);//!< multiply a scalar by a unitquaternion is not allowed
	friend unitquaternion operator/(const unitquaternion& q,const real& s);//!< divide a unitquaternion by a scalar factor is not allowed
};

//-----------------------------------------------------------------------------
// inlines

inline unitquaternion::unitquaternion()
:quaternion(1,0,0,0)
{
};


inline unitquaternion::unitquaternion (const quaternion& q)
:quaternion(q)
{
	normalize();
}


/*!
This constructor has a different meaning from the same in the base class. Here
the oglvector means an axis of rotation while the real means the angle to rotate
about the axis.

\b NOTE: the angle and axis oglvector are not directly assigned to the real part
and the oglvector part, respectively, of the quaternion. The unit quaternion (<I>s</I>,<B>v</B>)
represents a rotation of \b angle radians about the axis \b a if:

\e s = cos(\e angle / 2)<BR>
\b v = \b a * sin( \e angle / 2)
\param angle the rotation angle
\param axis the axis of rotation
\date 18/06/99
*/
inline unitquaternion::unitquaternion (const real& angle, const oglvector& axis)
:quaternion( cos(angle/2), sin(angle/2)*axis.normalized() )
{
}

/*!
This constructor has been introduced exclusively to make the inversed function
more efficient by avoiding too many unitquaternion copies and conversions to
quaternions.
*/
inline unitquaternion::unitquaternion (const oglvector& v, const real& s)
:quaternion( s, v )
{
}

inline unitquaternion::~unitquaternion()
{
}

/*!
For unitquaternions the inverse equals the conjugate (which is simpler to calculate)
This function doesn't modifies the quaternion upon which has been called, it returns a new quaternion instead.
\return the normalized copy of the quaternion
\date 18/06/99
*/
inline unitquaternion unitquaternion::inversed() const
{
	return unitquaternion(-v,s);
}

/*!
For unitquaternions the inverse equals the conjugate (which is simpler
to calculate). The function doesn't call the base conjugate function to
avoid an expensive quaternion to unitquaternion conversion.
This function modifies the quaternion upon which has been called.
*/
inline void unitquaternion::inverse()
{
	v=-v;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the x component of the oglvector part).
*/
inline real unitquaternion::x() const
{
	return ((oglvector&)v).x();// explicit cast to use the non const oglvector x() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the y component of the oglvector part).
*/
inline real unitquaternion::y() const
{
	return ((oglvector&)v).y();// explicit cast to use the non const oglvector y() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the z component of the oglvector part).
*/
inline real unitquaternion::z() const
{
	return ((oglvector&)v).z();// explicit cast to use the non const oglvector z() function
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the scalar part).
*/
inline real unitquaternion::w() const
{
	return s;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the scalar part).
*/
inline real unitquaternion::scalarPart() const
{
	return s;
}

/*!
This function overrides the same in base class to prevent the use as an l-value
(that is to modify the oglvector part).
*/
inline oglvector unitquaternion::vectorPart() const
{
	return v;
}


//-----------------------------------------------------------------------------
// useful constants

//=============================================================================
//=============================================================================

/* inserire qui alcune macro o funzioni globali come ad esempio:
	- clamping
	- conversione angoli gradi<->radianti
*/

/*!
limits a value in a range, modifying it.
\param val the value to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline void clamp(real& val,const real& low,const real& high)
{
	if(val<low) val=low;
	if(val>high) val=high;
}

/*!
limits a value in a range, returning the clamped value.
\return the clamped value
\param val the value to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline real clamped(const real& val,const real& low,const real& high)
{
	if(val<low) return low;
	else if(val>high) return high;
		else return val;
}


/*!
limits the oglvector components in a range, modifying it.
\param theVec the oglvector to clamp
\param low the lower treshold
\param high the higher treshold
*/
inline void clamp(oglvector& theVec,const real& low,const real& high)
{
	clamp(theVec.vec[0],low,high);
	clamp(theVec.vec[1],low,high);
	clamp(theVec.vec[2],low,high);
}

/*!
\short returns \e angle expressed in degrees.
\return the angle expresses in radians
\param angle the angle value
\author Alessandro Falappa
*/
inline real RadToDeg(const real& angle)
{
	return angle*57.29577951308;
}

/*!
\short returns \e angle expressed in radians.
\return the angle expresses in degrees
\param angle the angle value
*/
inline real DegToRad(const int& angle)
{
	return angle*0.01745329251994;
}

/*!
\short converts radiants to degrees.
This function modify its argument.
\param angle the angle to be converted
*/
inline void ConvertToDeg(real& angle)
{
	angle*=57.29577951308;
}

/*!
\short converts degrees to radiants.
This function modify its argument.
\param angle the angle to be converted
*/
inline void ConvertToRad(real& angle)
{
	angle*=0.01745329251994;
}

/*!
\short absolute value function which executes a simple test
This function executes a simple test on \e val negativity returning the
opposite if true. Such a test can be faster than the call to the \e fabs
library routine
\return the absolute value of \e val
*/
inline real simpleabs(const real &val)
{
	return val>0?val:-val;
}
//! the greek pi constant
extern const real G_PI;

//! greek pi / 2
extern const real G_HALF_PI;

//!2 * greek pi
extern const real G_DOUBLE_PI;

#ifdef _AFXDLL
#undef assert
//!maps the assert function to the MFC ASSERT macro
#define assert(exp) ASSERT(exp)
#endif

#ifndef HIGH_PRECISION
// this below could be MSVC compiler specific
#pragma warning( pop )// reset the warning status
#endif

#endif // _3D_MATH_H
// OGLControl.h: interface for the CBallController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OGLCONTROL_H__196CBD82_3858_11D2_80B1_A7800FACFE7F__INCLUDED_)
#define AFX_OGLCONTROL_H__196CBD82_3858_11D2_80B1_A7800FACFE7F__INCLUDED_

#include "OGLMath.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Auxiliary Type Definitions
enum AxisSet
{
	NO_AXES,
	CAMERA_AXES,
	BODY_AXES,
	OTHER_AXES
};

class COGLControl  
{
public:
	bool bDrawConstraints;
	tmatrix bodyorientation;
	int angleKeyIncrement;
	void DrawConstraints();
	oglvector* GetUsedAxisSet();
	oglvector BallColor;
	bool bProjectionMethod2;
	bool bDrawBallArea;
	int GLdisplayList;
	//unitquaternion currentQuat;
	unitquaternion previousQuat;
	real radius;
	real winWidth;
	real winHeight;
	real xprev;
	real yprev;
	CPoint center;
	bool mouseButtonDown;
	AxisSet whichConstraints;
	int currentAxisIndex;
	oglvector cameraAxes[3];
	oglvector bodyAxes[3];
	oglvector* otherAxes;
	int otherAxesNum;

	void InitDisplayLists();
	void initVars(void);
	void ProjectOnSphere(oglvector& v) const;
	unitquaternion RotationFromMove(const oglvector& vfrom,const oglvector& vto);
	oglvector ConstrainToAxis(const oglvector& loose,const oglvector& axis);
	int NearestConstraintAxis(const oglvector& loose);
public:
	long iterator;
	unitquaternion currentQuat;
	bool GetDrawConstraints();
	void SetDrawConstraints(bool flag=true);
	void DrawBall();
	int GetAngleKeyIncrement();
	void SetAngleKeyIncrement(int ang);
	void UseConstraints(AxisSet constraints);
	void ToggleMethod();
	void SetAlternateMethod(bool flag=true);
	oglvector GetColorV();
	COLORREF GetColor();
	void SetColor(COLORREF col);
	void SetColorV(oglvector colvec);
	void DrawBallLimit();
	COGLControl();
	COGLControl(const real& rad);
	COGLControl(const real& rad,const unitquaternion& initialOrient);
	COGLControl(const COGLControl& other);
	virtual ~COGLControl();
	COGLControl& operator=(const COGLControl& other);
	void Resize(const real& newRadius);
	void ClientAreaResize(const CRect& newSize);
	void MouseDown(const CPoint& location);
	void MouseUp(const CPoint& location);
	void MouseMove(const CPoint& location);
	void IssueGLrotation();
	void Key(UINT nChar);
	void SetCamera(long iterator);

};

//---------------------------------------------------------------------------
// inlines

inline COGLControl::~COGLControl()
{
	if(otherAxes) delete[] otherAxes;
}

inline COGLControl::COGLControl(const COGLControl& other)
{
	*this=other;
}

inline void COGLControl::Resize(const real& newRadius)
{
	radius=newRadius;
}

inline void COGLControl::ClientAreaResize(const CRect& newSize)
{
	winWidth=real(newSize.Width());
	winHeight=real(newSize.Height());
//	center=CPoint( (newSize.right-newSize.left)/2 , (newSize.bottom-newSize.top)/2);
}

inline COGLControl::COGLControl()
{
	initVars();
}

inline COLORREF COGLControl::GetColor()
{
	return RGB(int(BallColor.x()*255),int(BallColor.y()*255),int(BallColor.z()*255));
}

inline oglvector COGLControl::GetColorV()
{
	return BallColor;
}

inline void COGLControl::SetAlternateMethod(bool flag)
{
	bProjectionMethod2=flag;
}

inline void COGLControl::ToggleMethod()
{
	if(bProjectionMethod2) bProjectionMethod2=false;
	else bProjectionMethod2=true;
}

inline void COGLControl::UseConstraints(AxisSet constraints)
{
	whichConstraints=constraints;
}

inline int COGLControl::GetAngleKeyIncrement()
{
	return angleKeyIncrement;
}

inline void COGLControl::SetAngleKeyIncrement(int ang)
{
	angleKeyIncrement=abs(ang)%360;
}

inline bool COGLControl::GetDrawConstraints()
{
	return bDrawConstraints;
}

inline void COGLControl::SetDrawConstraints(bool flag)
{
	bDrawConstraints=flag;
}


#endif // !defined(AFX_OGLCONTROL_H__196CBD82_3858_11D2_80B1_A7800FACFE7F__INCLUDED_)


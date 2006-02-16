// OGLControl.cpp: implementation of the 3D-BallController.
// Steuerung der OpenGL Oberfläche
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GeoSys.h"
#include "OGLControl.h"
#include "OGLMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

long RotationSwitch = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COGLControl::COGLControl(const real& rad)
{
	initVars();
	radius= clamped(rad,(real)0.1,1);
}


COGLControl::COGLControl(const real& rad,const unitquaternion& initialOrient)
{
	initVars();
	radius=clamped(rad,(real)0.1,1.0);
	currentQuat=initialOrient;
}

COGLControl& COGLControl::operator=(const COGLControl& other)
{
	if(this==&other) return *this;
	initVars();
	currentQuat=other.currentQuat;
	previousQuat=other.previousQuat;
	radius=other.radius;
	winWidth=other.winWidth;
	winHeight=other.winHeight;
	otherAxesNum=other.otherAxesNum;
	otherAxes=new oglvector[otherAxesNum];
	for(int c=0;c<otherAxesNum;c++) otherAxes[c]=other.otherAxes[c];
	BallColor=other.BallColor;
	return *this;
}

void COGLControl::MouseDown(const CPoint& location)
{
	xprev=(2*location.x-winWidth)/winWidth;
	yprev=(winHeight-2*location.y)/winHeight;
	previousQuat=currentQuat;
	mouseButtonDown=true;
	bDrawBallArea=bProjectionMethod2;// draw circle only if method 2 active

/*	real winWidth=clientArea.Width();
	real winHeight=clientArea.Height();
	vdown=oglvector((2*location.x-winWidth)/winWidth,(winHeight-2*location.y)/winHeight,0);
	ProjectOnSphere2(vdown);
	vcurr=vdown;
*/
}


void COGLControl::MouseUp(const CPoint &location)
{
	mouseButtonDown=false;
	if (&location) xprev=yprev=0.0;
	bDrawBallArea=false;
	// save current rotation axes for bodyAxes constraint at next rotation
	bodyorientation=currentQuat.getRotMatrix();
	bodyAxes[0]=oglvector(bodyorientation(0,0),bodyorientation(1,0),bodyorientation(2,0));
	bodyAxes[1]=oglvector(bodyorientation(0,1),bodyorientation(1,1),bodyorientation(2,1));
	bodyAxes[2]=oglvector(bodyorientation(0,2),bodyorientation(1,2),bodyorientation(2,2));

//	vdown=vcurr=ORIGIN;// per rubberbanding
}

void COGLControl::MouseMove(const CPoint& location)
{
	real xcurr=(2*location.x-winWidth)/winWidth;
	real ycurr=(winHeight-2*location.y)/winHeight;
	oglvector vfrom(xprev,yprev,0);
	oglvector vto(xcurr,ycurr,0);
	if(mouseButtonDown)
	{
// find the two points on sphere according to the projection method
		ProjectOnSphere(vfrom);
		ProjectOnSphere(vto);
// modify the vectors according to the active constraint
		if(whichConstraints != NO_AXES)
		{
			oglvector* axisSet=GetUsedAxisSet();
			vfrom=ConstrainToAxis(vfrom,axisSet[currentAxisIndex]);
			vto=ConstrainToAxis(vto,axisSet[currentAxisIndex]);
		};
// get the corresponding unitquaternion
		unitquaternion lastQuat=RotationFromMove(vfrom,vto);
		currentQuat*=lastQuat;
		xprev=xcurr;
		yprev=ycurr;
	}
	else if(whichConstraints != NO_AXES)
		{
			ProjectOnSphere(vto);
			currentAxisIndex=NearestConstraintAxis(vto);
		};
}

void COGLControl::IssueGLrotation()
{
#ifdef REAL_IS_DOUBLE
	glMultMatrixd(currentQuat.getRotMatrix());
#else
	glMultMatrixf(currentQuat.getRotMatrix());
#endif
}

void COGLControl::ProjectOnSphere(oglvector& v) const
{
	real rsqr=radius*radius;
	real dsqr=v.x()*v.x()+v.y()*v.y();
	if(bProjectionMethod2)
	{
		// if inside sphere project to sphere else on plane
		if(dsqr>rsqr)
		{
			register real scale=(radius-(real)0.05)/sqrt(dsqr);
			v.x()*=scale;
			v.y()*=scale;
			v.z()=0;
		}
		else
		{
			v.z()=sqrt(rsqr-dsqr);
		}
	}
	else
	{
		// if relatively "inside" sphere project to sphere else on hyperbolic sheet
		if(dsqr<(rsqr*0.5))	v.z()=sqrt(rsqr-dsqr);
		else v.z()=rsqr/(2*sqrt(dsqr));
	};
}

unitquaternion COGLControl::RotationFromMove(const oglvector& vfrom,const oglvector& vto)
{
//	vcurr=vto;// per rubberbanding
	if(bProjectionMethod2)
	{
		quaternion q;
		q.x()=vfrom.z()*vto.y()-vfrom.y()*vto.z();
		q.y()=vfrom.x()*vto.z()-vfrom.z()*vto.x();
		q.z()=vfrom.y()*vto.x()-vfrom.x()*vto.y();
		q.w()=vfrom*vto;
		return unitquaternion(q);
	}
	else
	{
// calculate axis of rotation and correct it to avoid "near zero length" rot axis
		oglvector rotaxis=(vto^vfrom);
		rotaxis.EpsilonCorrect(X_AXIS);
// find the amount of rotation
		oglvector d(vfrom-vto);
		real t=d.length()/(2*radius);
		clamp(t,-1.0,1.0);
		real phi=(real)2.0*(real)asin(t);
		return unitquaternion(phi,rotaxis);
	}
}

void COGLControl::Key(UINT nChar)
{
	switch(nChar)
	{
	//case VK_UP:
	case VK_NUMPAD8:
		currentQuat*=unitquaternion(DegToRad(angleKeyIncrement),X_AXIS);
		break;
	//case VK_DOWN:	
	case VK_NUMPAD2:
		currentQuat*=unitquaternion(DegToRad(-angleKeyIncrement),X_AXIS);
		break;
	//case VK_RIGHT:
	case VK_NUMPAD6:
		currentQuat*=unitquaternion(DegToRad(angleKeyIncrement),Y_AXIS);
		break;
	//case VK_LEFT:
	case VK_NUMPAD4:
		currentQuat*=unitquaternion(DegToRad(-angleKeyIncrement),Y_AXIS);
		break;
	case VK_PRIOR:
	case VK_NUMPAD9:
		currentQuat*=unitquaternion(DegToRad(angleKeyIncrement),Z_AXIS);
		break;
	case VK_HOME:
	case VK_NUMPAD7:
		currentQuat*=unitquaternion(DegToRad(-angleKeyIncrement),Z_AXIS);
		break;
	case VK_DELETE:
	case VK_NUMPAD5:
		currentQuat=unitquaternion(0,X_AXIS);
		break;
	case VK_F1:
		currentQuat=unitquaternion(0,X_AXIS);
		break;
	case VK_F2:
		currentQuat=unitquaternion(90,X_AXIS);
		break;
	case VK_F3:
		currentQuat=unitquaternion(90,Z_AXIS);
		break;
	case VK_ESCAPE:
		currentQuat=previousQuat;
		MouseUp(CPoint(-1,-1));
		break;
	case VK_TAB:
		if(mouseButtonDown && whichConstraints!=NO_AXES)
		{
			currentAxisIndex=(currentAxisIndex+1)%3;
			currentQuat=previousQuat;
		}
		break;
	};
}

void COGLControl::DrawBallLimit()
{
	// "spherical zone" of controller
#ifdef REAL_IS_DOUBLE
		glColor3dv(BallColor);
#else
		glColor3fv(BallColor);
#endif
		glCallList(GLdisplayList);
}

void COGLControl::InitDisplayLists()
{
	GLdisplayList=glGenLists(1);
	
}

void COGLControl::initVars()
{
	winWidth=winHeight=0;
	previousQuat=currentQuat=unitquaternion(0,X_AXIS);
	mouseButtonDown=bDrawBallArea=bProjectionMethod2=bDrawConstraints=false;
	xprev=yprev=0.0;
	center=CPoint(0,0);
	radius= (real) 0.6;
	GLdisplayList=currentAxisIndex=otherAxesNum=0;
	BallColor=oglvector(0.0,0.5,1.0);
	otherAxes=NULL;
	whichConstraints=NO_AXES;
	cameraAxes[0]=bodyAxes[0]=X_AXIS;
	cameraAxes[1]=bodyAxes[1]=Y_AXIS;
	cameraAxes[2]=bodyAxes[2]=Z_AXIS;
	bodyorientation.loadIdentity();
	angleKeyIncrement=5;
}

void COGLControl::SetColor(COLORREF col)
{
	real a = 255.0;
	BallColor.x()=GetRValue(col)/a;
	BallColor.y()=GetGValue(col)/a;
	BallColor.z()=GetBValue(col)/a;
}

void COGLControl::SetColorV(oglvector colvec)
{
	clamp(colvec,0,1);
	BallColor=colvec;
}

oglvector COGLControl::ConstrainToAxis(const oglvector& loose,const oglvector& axis)
{
	oglvector onPlane;
    register real norm;
    onPlane = loose-axis*(axis*loose);
    norm = onPlane.length();
    if (norm > 0)
	{
		if (onPlane.z() < 0.0) onPlane = -onPlane;
		return ( onPlane/=sqrt(norm) );
    };
    if (axis.z() == 1) onPlane = X_AXIS;
	else
	{
		onPlane = oglvector(-axis.y(), axis.x(), 0);
		onPlane.normalize();
    }
    return (onPlane);
}

int COGLControl::NearestConstraintAxis(const oglvector& loose)
{
	oglvector* axisSet=GetUsedAxisSet();
	oglvector onPlane;
	register float max, dot;
	register int i, nearest;
	max = -1; 
	nearest = 0;
	if(whichConstraints == OTHER_AXES)
	{
		for (i=0; i<otherAxesNum; i++)
		{
			onPlane = ConstrainToAxis(loose, axisSet[i]);
			dot = onPlane*loose;
			if (dot>max) max = dot; nearest = i;
		}
	}
	else
	{
		for (i=0; i<3; i++)
		{
			onPlane = ConstrainToAxis(loose, axisSet[i]);
			dot = onPlane*loose;
			if (dot>max)
			{
				max = dot;
				nearest = i;
			};
		}
	};
	return (nearest);
}

oglvector* COGLControl::GetUsedAxisSet()
{
    oglvector* axes=NULL;
	switch(whichConstraints)
	{
	case CAMERA_AXES: axes=cameraAxes;
		break;
	case BODY_AXES: axes=bodyAxes;
		break;
	case OTHER_AXES: axes=otherAxes;
		break;
	};
	return axes;
}

void COGLControl::DrawConstraints()
{
	glColor3f(0,.75f,0);
	if(whichConstraints==CAMERA_AXES)
	{
		glCallList(GLdisplayList);
		glBegin(GL_LINES);
			glVertex3f(-radius,0,0);
			glVertex3f(radius,0,0);
			glVertex3f(0,-radius,0);
			glVertex3f(0,radius,0);
		glEnd();
	};
	if(whichConstraints==BODY_AXES)
	{
		glPushMatrix();
#ifdef REAL_IS_DOUBLE
		glMultMatrixd(bodyorientation);
#else
		glMultMatrixf(bodyorientation);
#endif
		glCallList(GLdisplayList);
		glRotated(90,1.0,0.0,0.0);
		glCallList(GLdisplayList);
		glRotated(90,0.0,1.0,0.0);
		glCallList(GLdisplayList);

		glPopMatrix();
	};
}

void COGLControl::DrawBall()
{
// change the projection matrix to identity (no view transformation )
	glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
// reset the transformations
	glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
// prepare the circle display list the first time
	if(GLdisplayList == 0) InitDisplayLists();
// disable lighting and depth testing
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
// draw the constraints or the ball limit if appropriate
		if(bDrawConstraints && whichConstraints!=NO_AXES) DrawConstraints();
		else if(bDrawBallArea) DrawBallLimit();
	glPopAttrib();
// restore the modelview and projection matrices
		glPopMatrix();
	glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void COGLControl::SetCamera(long iterator)
{
 int i=0;
 for (i=0;i<iterator;i++)
 {
   currentQuat=unitquaternion(0,X_AXIS);

 }
}
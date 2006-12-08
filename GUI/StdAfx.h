// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__59CED12E_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)
#define AFX_STDAFX_H__59CED12E_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#pragma comment(lib,"gdiplus.lib")
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <gdiplus.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#ifdef USE_SGI_OGL
// SGI openGL libraries (link with OPENGL.LIB and GLU.LIB)
#include "gl.h" //modify the path as needed
#include "glu.h"
#include <gl\glut.h>
#include <afxdhtml.h>
#include <afxdlgs.h>
#else
// MS openGL libraries (link with OPENGL32.LIB and GLU32.LIB)
#include "gl\gl.h"
#include "gl\glu.h"
#endif
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__59CED12E_E584_11D1_ACB3_E52ED8AC9002__INCLUDED_)

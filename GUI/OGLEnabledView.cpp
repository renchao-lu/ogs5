//******************************************************/
// COGLEnabledView.cpp : implementation file
//

#include "stdafx.h"
#include "OGLEnabledView.h"
#include "GeoSysMdiFrames.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_LISTS 20
// used to identify a MCD video driver (partial OGL acceleration)
#define ICD_DRIVER_MASK  (PFD_GENERIC_ACCELERATED | PFD_GENERIC_FORMAT)

/////////////////////////////////////////////////////////////////////////////
// Global Functions/variables

#ifdef PCH_GL
struct GarbListItem{
	GLdouble *pvert;
	GarbListItem* next;};

GarbListItem* m_garbagelist=NULL;

void AddGarbage(GLdouble * ptr)
{
	ASSERT(ptr!=NULL);
// allocate mem for new list item
	GarbListItem* temp=new GarbListItem;
// store pointer
	temp->pvert=ptr;
// add at head of list
	temp->next=m_garbagelist;
	m_garbagelist=temp;
}

void DeleteGarbage()
{
	if(m_garbagelist!=NULL)
	{
		GarbListItem* punt=m_garbagelist;
		GarbListItem* temp=m_garbagelist;
// scan the list
		while(punt!=NULL)
		{
// delete vertex
			delete[] punt->pvert;
			punt=punt->next;
// delete list item
			delete temp;
			temp=punt;
		};
		m_garbagelist=NULL;
	};
}

void CALLBACK BeginCallback(GLenum type)
{
// issue corresponding GL call
	glBegin(type);
}

void CALLBACK ErrorCallback(GLenum errorCode)
{
	const GLubyte *estring;
	CString mexstr;
// get the error descritption from OGL
	estring = gluErrorString(errorCode);
// prepare and show a message box
	mexstr.Format("Tessellation/Quadric Error: %s\n", estring);
	AfxMessageBox(mexstr,MB_OK | MB_ICONEXCLAMATION);
// replicate mex to debug trace
	TRACE0(mexstr);
}

void CALLBACK EndCallback()
{
// issue corresponding GL call
	glEnd();
}

void CALLBACK VertexCallback(GLvoid *vertex)
{
// issue corresponding GL call (double is used to get max precision)
	glVertex3dv( (const double *)vertex );
}

void CALLBACK CombineCallback(GLdouble coords[3], GLdouble *data[4], GLfloat weight[4], GLdouble **dataOut )
{
// allocate memory for a new vertex  
	GLdouble *vertex;
	vertex = new GLdouble[3];
// store reported vertex
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
// return vertex to OGL
	if (data[4]||weight[4])	*dataOut = vertex;
	else *dataOut = vertex;
// add vertex pointer to garbage collection routines
	AddGarbage(vertex);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView

IMPLEMENT_DYNCREATE(COGLEnabledView, CView)

COGLEnabledView::COGLEnabledView():
	m_dAspectRatio(1.0),
	m_numchars(128),
	m_bInsideDispList(FALSE), m_bExternDispListCall(FALSE),
	m_bExternGLCall(FALSE),
	m_gmfvector(NULL),
	m_charsetDListBase(0)
{
// define a default cursor
	m_hMouseCursor=AfxGetApp()->LoadStandardCursor(IDC_CROSS);
// set the disp list vector to all zeros
	for (int c=0;c<MAX_LISTS;c++) m_DispListVector[c]=0;
}

COGLEnabledView::~COGLEnabledView()
{
	if(m_gmfvector!=NULL) delete[] m_gmfvector;
}


BEGIN_MESSAGE_MAP(COGLEnabledView, CView)
	//{{AFX_MSG_MAP(CGLEnabledView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView drawing

void COGLEnabledView::OnDraw(CDC* pDC)
{ 
   CTabSplitterFrame *TabSplit = NULL;
   TabSplit->ActivateOGLView(0);
   pDC =  GetDC( );
// prepare a semaphore
	static BOOL 	bBusy = FALSE;
// use the semaphore to enter this critic section
	if(bBusy) return;
	bBusy = TRUE;

// specify the target DeviceContext of the subsequent OGL calls
	wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);

// clear background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// call the virtual drawing procedure (to be overridden by user)
	OnDrawGL();

// execute OGL commands (flush the OGL graphical pipeline)
//	glFinish(); // useless cause swapbuffers issues an implicit glFinish

// if double buffering is used it's time to swap the buffers
	SwapBuffers(m_pCDC->GetSafeHdc());

// check for errors
	m_glErrorCode=glGetError();
	if(m_glErrorCode != GL_NO_ERROR)
	{
		const GLubyte *estring;
		CString mexstr;
// get the error descritption from OGL
		estring = gluErrorString(m_glErrorCode);
// prepare and show a message box
		mexstr.Format("OGLEnabledView:\n\tAn OpenGL error occurred: %s\n", estring);
		AfxMessageBox(mexstr,MB_OK | MB_ICONEXCLAMATION);
// replicate mex to debug trace
		TRACE0(mexstr);
// turn the semaphore "red" to avoid other wrong drawings
		bBusy=TRUE;
	}
	else
	{
// turn the semaphore "green"
		bBusy = FALSE;
	}

// free the target DeviceContext (window)
    wglMakeCurrent(NULL,NULL);

}

void COGLEnabledView::OnDrawGL()
{
// draw carthesian axes
	glBegin(GL_LINES);
		// red x axis
		glColor3f(1.f,0.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.9f,0.1f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.9f,-0.1f,0.0f);
		// green y axis
		glColor3f(0.f,1.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(0.1f,0.9f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(-0.1f,0.9f,0.0f);
		// blue z axis
		glColor3f(0.f,0.f,1.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,0.1f,0.9f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,-0.1f,0.9f);
	glEnd();

}

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView diagnostics

#ifdef _DEBUG
void COGLEnabledView::AssertValid() const
{
	CView::AssertValid();
}

void COGLEnabledView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
// dump some infos
	CString str;
	GetWindowText(str);
	afxDump<<"\nView Parameters\n\tClient Rectangle :"<<m_ClientRect<<"\n\tAspect Ratio :"<<m_dAspectRatio<<"\n";
	afxDump<<"\nWindowTitle :"<<str<<"\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////
// CGLEnabledView Constants

// these are used to construct an equilibrated 256 color palette
static unsigned char _threeto8[8] = 
{
	0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

static unsigned char _twoto8[4] = 
{
	0, 0x55, 0xaa, 0xff
};

static unsigned char _oneto8[2] = 
{
	0, 255
};

static int defaultOverride[13] = 
{
	0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

// Windows Default Palette
static PALETTEENTRY defaultPalEntry[20] = 
{
	{ 0,   0,   0,    0 },
	{ 0x80,0,   0,    0 },
	{ 0,   0x80,0,    0 },
	{ 0x80,0x80,0,    0 },
	{ 0,   0,   0x80, 0 },
	{ 0x80,0,   0x80, 0 },
	{ 0,   0x80,0x80, 0 },
	{ 0xC0,0xC0,0xC0, 0 },

	{ 192, 220, 192,  0 },
	{ 166, 202, 240,  0 },
	{ 255, 251, 240,  0 },
	{ 160, 160, 164,  0 },

	{ 0x80,0x80,0x80, 0 },
	{ 0xFF,0,   0,    0 },
	{ 0,   0xFF,0,    0 },
	{ 0xFF,0xFF,0,    0 },
	{ 0,   0,   0xFF, 0 },
	{ 0xFF,0,   0xFF, 0 },
	{ 0,   0xFF,0xFF, 0 },
	{ 0xFF,0xFF,0xFF, 0 }
};

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView initialization and palette helpers

BOOL COGLEnabledView::bSetupPixelFormat()
{
// define a default desired video mode (pixel format)
	static PIXELFORMATDESCRIPTOR pfd = 
	{
        sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
        1,								// version number
        PFD_DRAW_TO_WINDOW |			// support window
        PFD_SUPPORT_OPENGL |			// support OpenGL
        PFD_DOUBLEBUFFER ,				// double buffered
        PFD_TYPE_RGBA,                  // RGBA type
        24,                             // 24-bit color depth
        0, 0, 0, 0, 0, 0,               // color bits ignored
        0,                              // no alpha buffer
        0,                              // shift bit ignored
        0,                              // no accumulation buffer
        0, 0, 0, 0,                     // accum bits ignored
        16,                             // 16-bit z-buffer
        0,                              // no stencil buffer
        0,                              // no auxiliary buffer
        PFD_MAIN_PLANE,                 // main layer
        0,                              // reserved
        0, 0, 0                         // layer masks ignored
    };
// let the user change some parameters if he wants
	BOOL bDoublBuf;
	ColorsNumber cnum;
	ZAccuracy zdepth;
	VideoMode(cnum,zdepth,bDoublBuf);
//set the user changes
	if(bDoublBuf) pfd.dwFlags=PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |PFD_DOUBLEBUFFER;
	else pfd.dwFlags=PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	switch(cnum)
	{
	case INDEXED:
		pfd.cColorBits=8;
		pfd.iPixelType=PFD_TYPE_COLORINDEX;
		break;
	case THOUSANDS:
		pfd.cColorBits=16;
		break;
	case MILLIONS_WITH_TRANSPARENCY:
		pfd.cColorBits=32;
		break;
	case MILLIONS:
	default:
		pfd.cColorBits=24;
		break;
	};
	switch(zdepth)
	{
	case NORMAL:
		pfd.cDepthBits=16;
		break;
	case ACCURATE:
		pfd.cDepthBits=32;
		break;
	};

// ask the system for such video mode
    ASSERT(m_pCDC != NULL);
    int pixelformat;
	if ( (pixelformat = ChoosePixelFormat(m_pCDC->GetSafeHdc(), &pfd)) == 0 )
    {
        AfxMessageBox("ChoosePixelFormat failed");
        return FALSE;
    }
// try to set this video mode    
	if (SetPixelFormat(m_pCDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
    {
// the requested video mode is not available so get a default one (the first)
        pixelformat = 1;	
		if (DescribePixelFormat(m_pCDC->GetSafeHdc(), pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd)==0)
		{
// neither the requested nor the default are available: fail
			AfxMessageBox("SetPixelFormat failed (no OpenGL compatible video mode)");
			return FALSE;
		}
    }
    return TRUE;
}

void COGLEnabledView::CreateRGBPalette()
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;

// get the initially choosen video mode
	n = ::GetPixelFormat(m_pCDC->GetSafeHdc());
    ::DescribePixelFormat(m_pCDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

// if is an indexed one...
    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
// ... construct an equilibrated palette (3 red bits, 3 green bits, 2 blue bits)
// NOTE: this code has been taken from MFC example Cube
		n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE) new char[sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY)];

        ASSERT(pPal != NULL);

        pPal->palVersion = 0x300;
        pPal->palNumEntries = unsigned short(n);
        for (i=0; i<n; i++)
        {
            pPal->palPalEntry[i].peRed=ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal->palPalEntry[i].peGreen=ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal->palPalEntry[i].peBlue=ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal->palPalEntry[i].peFlags=0;
        }

// fix up the palette to include the default Windows palette
        if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)
           )
        {
			for (i = 1 ; i <= 12 ; i++)
                pPal->palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
        }

        m_CurrentPalette.CreatePalette(pPal);
        delete pPal;

// set the palette
        m_pOldPalette=m_pCDC->SelectPalette(&m_CurrentPalette, FALSE);
        m_pCDC->RealizePalette();
    }
}

unsigned char COGLEnabledView::ComponentFromIndex(int i, UINT nbits, UINT shift)
{
    unsigned char val;

    val = (unsigned char) (i >> shift);
    switch (nbits) 
	{

    case 1:
        val &= 0x1;
        return _oneto8[val];
    case 2:
        val &= 0x3;
        return _twoto8[val];
    case 3:
        val &= 0x7;
        return _threeto8[val];

    default:
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView message handlers and overridables

int COGLEnabledView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1) return -1;
	
// OpenGL rendering context creation
	PIXELFORMATDESCRIPTOR pfd;
    int         n;

// initialize the private member
	m_pCDC= new CClientDC(this);

// choose the requested video mode
    if (!bSetupPixelFormat()) return 0;
	
// ask the system if the video mode is supported
    n=::GetPixelFormat(m_pCDC->GetSafeHdc());
    ::DescribePixelFormat(m_pCDC->GetSafeHdc(),n,sizeof(pfd),&pfd);

// create a palette if the requested video mode has 256 colors (indexed mode)
    CreateRGBPalette();

// link the Win Device Context with the OGL Rendering Context
    m_hRC = wglCreateContext(m_pCDC->GetSafeHdc());

// specify the target DeviceContext (window) of the subsequent OGL calls
    wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);

// performs default setting of rendering mode,etc..
	OnCreateGL();

// free the target DeviceContext (window)
    wglMakeCurrent(NULL,NULL);
	
	return 0;


}

void COGLEnabledView::OnCreateGL()
{
// perform hidden line/surface removal (enabling Z-Buffer)
	glEnable(GL_DEPTH_TEST);

// set background color to black
	glClearColor(0.f,0.f,0.f,1.0f );

// set clear Z-Buffer value
	glClearDepth(1.0f);
}

void COGLEnabledView::OnDestroy() 
{
// specify the target DeviceContext (window) of the subsequent OGL calls
    wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);

// remove all display lists
	for (int c=0;c<MAX_LISTS;c++) if(m_DispListVector[c]) glDeleteLists(m_DispListVector[c],1);

// release definitely OGL Rendering Context
	if (m_hRC!=NULL) ::wglDeleteContext(m_hRC);

// Select our palette out of the dc
	CPalette palDefault;
	palDefault.CreateStockObject(DEFAULT_PALETTE);
	m_pCDC->SelectPalette(&palDefault, FALSE);

// destroy Win Device Context
	if(m_pCDC) delete m_pCDC;

// finally call the base function
	CView::OnDestroy();	
}

BOOL COGLEnabledView::PreCreateWindow(CREATESTRUCT& cs) 
{
// these styles are requested by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS); 

    
// call base class PreCreateWindow to get the cs.lpszClass filled in with the MFC default class name
   if( !CView::PreCreateWindow(cs) )
	 return 0;

// Register the window class if it has not already been registered.
   WNDCLASS wndcls;
   HINSTANCE hInst = AfxGetInstanceHandle();
// this is the new registered window class
#define CUSTOM_CLASSNAME _T("GL_WINDOW_CLASS")
// check if our class has been already registered (typical in MDI environment)
   if(!(::GetClassInfo(hInst, CUSTOM_CLASSNAME, &wndcls)))
   {
// get default MFC class settings
		if(::GetClassInfo(hInst, cs.lpszClass, &wndcls))
		{
// set our class name
			wndcls.lpszClassName = CUSTOM_CLASSNAME;
// these styles are set for GL to work in MDI
			wndcls.style |= (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);
			wndcls.hbrBackground = NULL;
// try to register class (else throw exception)
			if (!AfxRegisterClass(&wndcls)) AfxThrowResourceException();
		}
// default MFC class not registered
		else AfxThrowResourceException();
	}
// set our class name in CREATESTRUCT
    cs.lpszClass = CUSTOM_CLASSNAME;
// we're all set


    return 1;
}


BOOL COGLEnabledView::OnEraseBkgnd(CDC* pDC) 
{
// OGL has his own background erasing so tell Windows to skip (avoids flicker)
    pDC =  GetDC( );
	return TRUE;
}

void COGLEnabledView::OnSize(UINT nType, int cx, int cy) 
{
	//cx=cy;
	CView::OnSize(nType, cx, cy);
// when called with a nonzero window:
	if ( 0 < cx && 0 < cy )
	{
// update the rect and the aspect ratio
		m_ClientRect.right = cx;
		m_ClientRect.bottom = cy;
		m_dAspectRatio=double(cx)/double(cy);

// specify the target DeviceContext of the OGL calls below
		wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);

// call the virtual sizing procedure (to be overridden by user)
		OnSizeGL(cx,cy);

// free the target DeviceContext (window)
		wglMakeCurrent(NULL,NULL);

// force redraw
		Invalidate(TRUE);
	};
}

void COGLEnabledView::OnSizeGL(int cx, int cy)
{
// set correspondence between window and OGL viewport
	
		glViewport(0,0,cx,cy);

// update the camera
 		glPushMatrix();
			glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluPerspective(90.0,m_dAspectRatio,0.1f, 10000.0f);
				glTranslatef(0.0f,0.0f,0.0f);
				//gluLookAt(1.0,0.0,0.0,0.0,0.0,0.0,0,0,1);
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
}

BOOL COGLEnabledView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	ASSERT(m_hMouseCursor!=NULL);
	::SetCursor(m_hMouseCursor);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGLEnabledView public members

void COGLEnabledView::VideoMode(ColorsNumber &c, ZAccuracy &z, BOOL &dbuf)
{
// set default videomode
	c=MILLIONS;
	z=NORMAL;
	dbuf=TRUE;
}

void COGLEnabledView::SetMouseCursor(HCURSOR mcursor)
{
// set the specified cursor (only if it is a valid one)
	if(mcursor!=NULL) m_hMouseCursor=mcursor;
}

GLInfoStruct COGLEnabledView::GetInformation()
{
	PIXELFORMATDESCRIPTOR pfd;
	GLInfoStruct str;
	HDC curr_dc=m_pCDC->GetSafeHdc();
// Get information about the DC's current pixel format 
	::DescribePixelFormat(curr_dc , ::GetPixelFormat(curr_dc),sizeof(PIXELFORMATDESCRIPTOR), &pfd ); 
// specify the target Rendering Context of the subsequent OGL calls
	wglMakeCurrent(curr_dc, m_hRC);
// Extract driver information
	if( 0==(ICD_DRIVER_MASK & pfd.dwFlags) )
		str.acceleration="Fully Accelerated (ICD)"; // fully in hardware (fastest)
		else if (ICD_DRIVER_MASK==(ICD_DRIVER_MASK & pfd.dwFlags) )
			str.acceleration="Partially Accelerated (MCD)"; // partially in hardware (pretty fast, maybe..)
			else str.acceleration="Not Accelerated (Software)";	// software
// get the company name responsible for this implementation
	str.vendor=(char*)::glGetString(GL_VENDOR);
	if ( ::glGetError()!=GL_NO_ERROR) str.vendor.Format("Not Available");// failed!
// get the renderer name; this is specific of an hardware configuration
	str.renderer=(char*)::glGetString(GL_RENDERER);
	if ( ::glGetError()!=GL_NO_ERROR) str.renderer.Format("Not Available");// failed!
// get the version of the GL library
	str.glversion=(char*)::glGetString(GL_VERSION);
	if ( ::glGetError()!=GL_NO_ERROR) str.glversion.Format("Not Available");// failed!
// return a space separated list of extensions
	str.glextensions=(char*)::glGetString(GL_EXTENSIONS);
	if ( ::glGetError()!=GL_NO_ERROR) str.glextensions.Format("Not Available");// failed!
// get the version of the GLU library
	str.gluversion=(char*)::gluGetString(GLU_VERSION);
	if ( ::glGetError()!=GL_NO_ERROR) str.gluversion.Format("Not Available");// failed!
// as above a space separated list of extensions
	str.gluextensions=(char*)::gluGetString(GLU_EXTENSIONS);
	if ( ::glGetError()!=GL_NO_ERROR) str.gluextensions.Format("Not Available");// failed!
	glGetIntegerv(GL_MAX_LIGHTS,&str.max_lights);
	glGetIntegerv(GL_MAX_CLIP_PLANES,&str.max_clip_planes);
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH,&str.max_model_stack_depth);
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH,&str.max_proj_stack_depth);
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH,&str.max_txtr_stack_depth);
	glGetIntegerv(GL_MAX_NAME_STACK_DEPTH,&str.max_name_stack_depth);
	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH,&str.max_attrib_stack_depth);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&str.max_texture_size);
	glGetIntegerv(GL_MAX_LIST_NESTING,&str.max_list_nesting);
	glGetIntegerv(GL_MAX_EVAL_ORDER,&str.max_eval_order);
	int tempint[2];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS,&tempint[0]);
	str.max_viewport_dims.cx=tempint[0];
	str.max_viewport_dims.cy=tempint[1];
	glGetIntegerv(GL_AUX_BUFFERS,&str.auxiliary_buffers);
	float tempval[2];
	glGetFloatv(GL_POINT_SIZE_RANGE,&tempval[0]);
	str.min_smooth_point_size=int(tempval[0]);
	str.max_smooth_point_size=int(tempval[1]);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY,&str.smooth_point_granularity);
	glGetFloatv(GL_LINE_WIDTH_RANGE,&tempval[0]);
	str.min_smooth_line_size=int(tempval[0]);
	str.max_smooth_line_size=int(tempval[1]);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY,&str.smooth_line_granularity);
	glGetIntegerv(GL_RED_BITS,&str.red_bits);
	glGetIntegerv(GL_BLUE_BITS,&str.blue_bits);
	glGetIntegerv(GL_GREEN_BITS,&str.green_bits);
	glGetIntegerv(GL_ALPHA_BITS,&str.alpha_bits);
	glGetIntegerv(GL_DEPTH_BITS,&str.depth_bits);
	glGetIntegerv(GL_STENCIL_BITS,&str.stencil_bits);
// free the target DeviceContext (window) and return the result
	wglMakeCurrent(NULL,NULL);
	return str;
}

void COGLEnabledView::DrawStockDispLists()
{
// check if we are already inside a drawing session
	if(m_hRC==wglGetCurrentContext() && m_pCDC->GetSafeHdc()==wglGetCurrentDC() )
	{
// draw directly all display lists
		for (int c=0;c<MAX_LISTS;c++) if(m_DispListVector[c]) glCallList(m_DispListVector[c]);
	}
	else
	{
// specify the target DeviceContext of the subsequent OGL calls
		wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);
// draw all display lists
		for (int c=0;c<MAX_LISTS;c++) if(m_DispListVector[c]) glCallList(m_DispListVector[c]);
// free the target DeviceContext (window)
		wglMakeCurrent(NULL,NULL);
	};
}

void COGLEnabledView::StartStockDListDef()
{
// check if we aren't inside another couple begin/end
	if(!m_bInsideDispList)
	{
// search a free slot
		int c;//HS 15.06.2007
		for (/*int*/ c=0;m_DispListVector[c]!=0;c++);
// check if we are inside a drawing session or not....
		if(!( m_hRC==wglGetCurrentContext() && m_pCDC->GetSafeHdc()==wglGetCurrentDC() ))
		{
// ...if not specify the target DeviceContext of the subsequent OGL calls
			wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);
// set a warning for EndDispList
			m_bExternDispListCall=TRUE;
		};
// create a handle to the disp list (actually an integer)
		m_DispListVector[c]=glGenLists(1);
// set a semaphore
		m_bInsideDispList=TRUE;
// start the disp list: all subsequent OGL calls will be redirected to the list
		glNewList(m_DispListVector[c],GL_COMPILE);
	};
}

void COGLEnabledView::EndStockListDef()
{
// close the disp list
	glEndList();
// unset the semaphore
	m_bInsideDispList=FALSE;
// if beginDispList set the warn free the target DeviceContext
	if(m_bExternDispListCall) wglMakeCurrent(NULL,NULL);
}

void COGLEnabledView::ClearStockDispLists()
{
// check if we are referring to the right Rendering Context
	if(m_hRC==wglGetCurrentContext() && m_pCDC->GetSafeHdc()==wglGetCurrentDC() )
	{
// delete active display lists
		for (int c=0;c<MAX_LISTS;c++) if(m_DispListVector[c]) glDeleteLists(m_DispListVector[c],1);
	}
	else
	{
// specify the target Rendering Context of the subsequent OGL calls
		wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);
// delete active display lists
		for (int c=0;c<MAX_LISTS;c++) if(m_DispListVector[c]) glDeleteLists(m_DispListVector[c],1);
// free the target Rendering Context (window)
		wglMakeCurrent(NULL,NULL);
	};
}

void COGLEnabledView::BeginGLCommands()
{
// check if we are inside a drawing session or not....
	if(!( m_hRC==wglGetCurrentContext() && m_pCDC->GetSafeHdc()==wglGetCurrentDC() ))
	{
// ...if not specify the target DeviceContext of the subsequent OGL calls
		wglMakeCurrent(m_pCDC->GetSafeHdc(), m_hRC);
// set a warning for EndGLCommands
		m_bExternGLCall=TRUE;
	};
}

void COGLEnabledView::EndGLCommands()
{
// if BeginGLCommands set the warn free the target DeviceContext
	if(m_bExternGLCall) wglMakeCurrent(NULL,NULL);
}

void COGLEnabledView::PrepareCharset3D(CString fontname,float extrusion,BOOL boldface,BOOL italicface,BOOL uselines,float precision)
{
	// debug checks
	ASSERT(extrusion>=0.f && precision>=0.f);
	ASSERT(!fontname.IsEmpty());
	// variables initialization
	int mode=uselines ? WGL_FONT_LINES : WGL_FONT_POLYGONS;
	if(m_gmfvector==NULL) m_gmfvector=new GLYPHMETRICSFLOAT[m_numchars];
	// prepare to ask for the requested font
	LOGFONT lg;
	memset(&lg, 0, sizeof(LOGFONT));
	lg.lfHeight=14;// for 3D text the height doesn't matter since the characters will be always of unit height
	lg.lfWidth=0;
	lg.lfEscapement=lg.lfOrientation=0;
	lg.lfItalic=unsigned char(italicface);
	if(boldface)lg.lfWeight=FW_BOLD;
	else lg.lfWeight=FW_NORMAL;
	lg.lfUnderline=FALSE;
	lg.lfStrikeOut=FALSE;
	lg.lfCharSet=ANSI_CHARSET;
	lg.lfOutPrecision=OUT_DEFAULT_PRECIS;
	lg.lfClipPrecision=CLIP_DEFAULT_PRECIS; 
	lg.lfQuality=DEFAULT_QUALITY;
	lg.lfPitchAndFamily=DEFAULT_PITCH |FF_DONTCARE;
	strcpy(lg.lfFaceName, fontname);        
	// ask for the font
	CFont fnt;
	if(!fnt.CreateFontIndirect(&lg))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create the requested font.\n");
		m_charsetDListBase=0;
		return;
	};
	//select the font into the DC
	CFont* def_font = m_pCDC->SelectObject(&fnt);
	// create or recreate the display lists
	if(m_charsetDListBase>0)
		glDeleteLists(m_charsetDListBase,m_numchars);	
	if (0 == (m_charsetDListBase=glGenLists(m_numchars)))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create the charset (no memory for display lists).\n");
		goto end;
	};
	// build the charset display lists
	if(!wglUseFontOutlines(m_pCDC->GetSafeHdc(),0,m_numchars-1,m_charsetDListBase,precision,extrusion,mode,m_gmfvector))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create the charset (internal error).\n");
		glDeleteLists(m_charsetDListBase,m_numchars);
		m_charsetDListBase=0;
	};
	// deselect and dispose of the GDI font
end:
	m_pCDC->SelectObject(def_font);
	fnt.DeleteObject();
}

void COGLEnabledView::PrepareCharset3D(const LOGFONT* pLF,float extrusion,BOOL uselines,float precision)
{
	// debug checks
	ASSERT(extrusion>=0.f && precision>=0.f);
	ASSERT(pLF!=NULL);
	// variables initialization
	int mode=uselines ? WGL_FONT_LINES : WGL_FONT_POLYGONS;
	if(m_gmfvector==NULL) m_gmfvector=new GLYPHMETRICSFLOAT[m_numchars];
	// ask for the font
	CFont fnt;
	if(!fnt.CreateFontIndirect(pLF))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create a font from the passed logical font descriptor.\n");
		return;
	}
	//select the font into the DC
	CFont* def_font = m_pCDC->SelectObject(&fnt);
	// create or recreate the display lists
	if(m_charsetDListBase>0) glDeleteLists(m_charsetDListBase,m_numchars);	
	if (0 == (m_charsetDListBase=glGenLists(m_numchars)))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create the charset (no memory for display lists).\n");
		goto end;
	};
	// build the charset display lists
	if(!wglUseFontOutlines(m_pCDC->GetSafeHdc(),0,m_numchars-1,m_charsetDListBase,precision,extrusion,mode,m_gmfvector))
	{
		TRACE("CGLEnabledView::PrepareCharset3D:\n\tUnable to create the charset (internal error).\n");
		glDeleteLists(m_charsetDListBase,m_numchars);
		m_charsetDListBase=0;
	};
	// deselect and dispose of the GDI font
end:
	m_pCDC->SelectObject(def_font);
	fnt.DeleteObject();
}

void COGLEnabledView::PrepareCharset2D(CString fontname,int height,BOOL boldface,BOOL italicface)
{
	// debug checks
	ASSERT(!fontname.IsEmpty() && height>0);
	// prepare to ask for the requested font
	LOGFONT lg;
	memset(&lg, 0, sizeof(LOGFONT));
	lg.lfHeight=height;// for 2D text the height is taken into consideration
	lg.lfWidth=0;
	lg.lfEscapement=lg.lfOrientation=0;
	lg.lfItalic=unsigned char(italicface);
	if(boldface)lg.lfWeight=FW_BOLD;
	else lg.lfWeight=FW_NORMAL;
	lg.lfUnderline=FALSE;
	lg.lfStrikeOut=FALSE;
	lg.lfCharSet=ANSI_CHARSET;
	lg.lfOutPrecision=OUT_DEFAULT_PRECIS;
	lg.lfClipPrecision=CLIP_DEFAULT_PRECIS; 
	lg.lfQuality=DEFAULT_QUALITY;
	lg.lfPitchAndFamily=DEFAULT_PITCH |FF_DONTCARE;
	strcpy(lg.lfFaceName, fontname);        
	// ask for the font
	CFont fnt;
	if(!fnt.CreateFontIndirect(&lg))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create the requested font.\n");
		m_charsetDListBase=0;
		return;
	};
	//select the font into the DC
	CFont* def_font = m_pCDC->SelectObject(&fnt);
	// create or recreate the display lists
	if(m_charsetDListBase>0)
		glDeleteLists(m_charsetDListBase,m_numchars);	
	if (0 == (m_charsetDListBase=glGenLists(m_numchars)))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create the charset (no memory for display lists).\n");
		goto end;
	};
	// build the charset display lists
	if(!wglUseFontBitmaps(m_pCDC->GetSafeHdc(),0,m_numchars-1,m_charsetDListBase))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create the charset (internal error).\n");
		glDeleteLists(m_charsetDListBase,m_numchars);
		m_charsetDListBase=0;
	};
	// deselect and dispose of the GDI font
end:
	m_pCDC->SelectObject(def_font);
	fnt.DeleteObject();
}

void COGLEnabledView::PrepareCharset2D(const LOGFONT* pLF)
{
	// debug checks
	ASSERT(pLF!=NULL);
	// ask for the font
	CFont fnt;
	if(!fnt.CreateFontIndirect(pLF))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create a font from the passed logical font descriptor.\n");
		return;
	}
	//select the font into the DC
	CFont* def_font = m_pCDC->SelectObject(&fnt);
	// create or recreate the display lists
	if(m_charsetDListBase>0) glDeleteLists(m_charsetDListBase,m_numchars);	
	if (0 == (m_charsetDListBase=glGenLists(m_numchars)))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create the charset (no memory for display lists).\n");
		goto end;
	};
	// build the charset display lists
	if(!wglUseFontBitmaps(m_pCDC->GetSafeHdc(),0,m_numchars-1,m_charsetDListBase))
	{
		TRACE("CGLEnabledView::PrepareCharset2D:\n\tUnable to create the charset (internal error).\n");
		glDeleteLists(m_charsetDListBase,m_numchars);
		m_charsetDListBase=0;
	};
	// deselect and dispose of the GDI font
end:
	m_pCDC->SelectObject(def_font);
	fnt.DeleteObject();
}

float COGLEnabledView::Text3D(CString text)
{
	float retlen=0.f;
	int textlen=0;
	if(m_charsetDListBase!=0)
	{
		if( (textlen=text.GetLength()) >0)
		{
			// output the outlines corresponding to the requested text srting
			glListBase(m_charsetDListBase);
			glCallLists(textlen,GL_UNSIGNED_BYTE,LPCTSTR(text));
			// calculate and return the length of the produced outlines
			for(int c=0;c<textlen;c++)
				retlen+=m_gmfvector[char(text[c])].gmfCellIncX;
		}
	}
	else TRACE("CGLEnabledView::Text3D:\n\tNo charset available. Use PrepareCharset3D routines first.\n");
	return retlen;
}

void COGLEnabledView::Text2D(CString text)
{
	int textlen=0;
	if(m_charsetDListBase!=0)
	{
		if( (textlen=text.GetLength()) >0)
		{
			// output the outlines corresponding to the requested text srting
			glListBase(m_charsetDListBase);
			glCallLists(textlen,GL_UNSIGNED_BYTE,LPCTSTR(text));
		}
	}
	else TRACE("CGLEnabledView::Text2D:\n\tNo charset available. Use PrepareCharset2D routines first.\n");
}

//////////////////////////////////////////////////////////////////////
//
// Implementation of CGLEnabledView::CGLDispList class.
//
/*** DESCRIPTION

  This is actually a helper class which wraps the
  use of display list in OGL.
  It must be used inside an OGLEnabledView cause
  a display list must refer to a Rendering Context.
  At present there is no support for Disp. Lists
  Sharing among multiple RCs (that is multiple MDI
  child windows).

****************************************/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

COGLEnabledView::CGLDispList::CGLDispList():
	m_glListId(0), m_bIsolated(FALSE)
{
}

COGLEnabledView::CGLDispList::~CGLDispList()
{
// remove display list
	glDeleteLists(m_glListId,1); 
}

//////////////////////////////////////////////////////////////////////
// Member functions

void COGLEnabledView::CGLDispList::Draw()
{
// if the list is not empty...
	if(m_glListId)
	{
		if(m_bIsolated)
		{
// save current transformation matrix
			glPushMatrix();
// save current OGL internal state (lighting, shading, and such)
			glPushAttrib(GL_ALL_ATTRIB_BITS);
		};
// draw the list
		glCallList(m_glListId);
		if(m_bIsolated)
		{
// restore transformation matrix
			glPopMatrix();
// restore OGL internal state
			glPopAttrib();
		};
	};
}

void COGLEnabledView::CGLDispList::StartDef(BOOL bImmediateExec)
{
// check if another list is under construction
	int cur;
	glGetIntegerv(GL_LIST_INDEX,&cur);
	if(cur != 0)
	{
		TRACE0("CGLEnabledView\n\tError: Nested display list definition!\n");
		ASSERT(FALSE);
	};
// if the list is empty firstly allocate one
	if(!m_glListId) m_glListId=glGenLists(1);

// start or replace a list definition
	if (bImmediateExec) glNewList(m_glListId,GL_COMPILE_AND_EXECUTE);
	else  glNewList(m_glListId,GL_COMPILE);
}

void COGLEnabledView::CGLDispList::EndDef()
{
// check the coupling with a preceding call to StartDef()
	int cur;
	glGetIntegerv(GL_LIST_INDEX,&cur);
	if(cur != m_glListId) {TRACE0("CGLDispList:Missing StartDef() before EndDef()\n");return;};
// close list definition
	glEndList();
}



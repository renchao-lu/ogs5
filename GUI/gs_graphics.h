#if !defined(AFX_GS_GRAPHICS_H__F6B19B1E_0E1D_4E76_9C0F_305B9553DF72__INCLUDED_)
#define AFX_GS_GRAPHICS_H__F6B19B1E_0E1D_4E76_9C0F_305B9553DF72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// gs_graphics.h : Header-Datei
//
#include "afxwin.h"
//C++ STL
#include <string>
#include <vector>
using namespace std;
// GEOLib
#include "geo_ply.h" 
// MSHLib
#include "nodes.h"
#include "fem_ele.h"
// PCSLib
#include "rf_tim_new.h"
#include "rf_fct.h"
#include "rf_mmp_new.h"
#include "rf_pcs.h"

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CGraphics 

class CGraphics : public CDialog
{
  friend class CGeoSysView;
  friend class CPolyline;
// Konstruktion
private:
public:
	CGraphics(CWnd* pParent = NULL);   // Standardkonstruktor
	~CGraphics();
    // Draw functions
    void DrawLinePixelOffset(int x1, int y1, int x2, int y2, CDC *dc);
    void DrawLineOffset(double x1, double y1, double x2, double y2, CDC *dc);
    void DrawLinePixel(int x1, int y1, int x2, int y2, CDC *dc);
    void DrawElement(long index,CDC *dc);
    void DrawCoordinateAxes(CDC *dc);
    void DrawIsosurfaces(long index,CDC *dc);
    void DrawIsosurfacesNew(CElem*,CRFProcess*,CDC*);
    void DrawPCSIsosurfaces(CRFProcess*,CDC*); //OK
    void DrawGridAxes(CDC *dc);
    void DrawTemporalBreakthroughCurves(CDC*,CTimeDiscretization*);
    void DrawProfileBreakthroughCurves(CDC *dc);
    void DrawPointPixel(CDC*,CGLPoint*);
    void DrawPointOffset(CDC *dc, CGLPoint *m_point);
    void DrawPoints(CDC*); //OK
    void DrawIsolines(CDC*,CRFProcess*); //OK
    void DrawIsolinesELE(long index,double dx,double dy,CDC *dc,int width,int height);
    void DrawIsolinesELE(CDC*,CElem*,CRFProcess*); //OK
    void DrawPolylines(CDC*);
    void DrawPolyline(CDC*,CGLPolyline*);
    void DrawSurface(CDC*,Surface*,int);
    void DrawElementMaterialGroup(long,CDC*);
    void DrawPolylineValues(CDC*,CGLPolyline*);
    void DrawSurfaceTIN(CDC*,Surface*);
    void DrawDoublePointPixel(CDC*,CGLPoint*);
    void DrawSurfaces(CDC*);
    void DrawPolylinesPoints(CDC *pDC);
    void DrawPolylinesLines(CDC *pDC);
    void DrawNodes(CDC*);
    void DrawMaterialGroups(CDC*);
    void DrawElements(CDC*);
    void DrawLoadPathAnalysis(CDC *dc);
    void DrawFCT(CDC*dc,CFunction*m_fct);
    void DrawMesh(CDC*);
    void DrawMSHElements(CDC*,CFEMesh*);
    void DrawMSHElement(CDC*,CElem*);
    void MarkPoints(CDC*dc,CPoint point); //OK
    bool MarkPoint(CDC*,CPoint,CGLPoint*); //OK
    void DrawMSHNodes(CDC*,CFEMesh*); //OK
    void DrawMSHNode(CDC*,CNode*); //OK
    void DrawXYPlot(CDC*,vector<double>&,vector<double>&); //OK
    void DrawMATFunction(CDC*,CMediumProperties*,CString); //OK
    void DrawMSHElementsLabels(CDC*pDC,CFEMesh*m_fem_msh); //OK
    void DrawMSHElementLabel(CDC*dc,CElem*m_ele); //OK
    void DrawMSHMaterialGroups(CDC*); //OK
    void DrawVolumes(CDC*); //OK
    void DrawDOMElements(CDC*); //OK
    void DrawSurfacesNew(CDC*); //OK
    void DrawSurfacesLabels(CDC*); //OK
    void DrawSurfaceNew(CDC*,Surface*); //OK
    void DrawSurfacesValues(CDC*,CRFProcess*); //OK

    void FillMSHElement(CDC*,int,long*);
    void MFCFillElement(int,double*,double*,CDC*,CBrush*);

   
    void ChooseContourColours(double farbwert);
    int ContourColourRed; 
    int ContourColourGreen; 
    int ContourColourBlue; 
    void MFC_FillElement1(int nn,double *nx,double *ny,CDC *dc,int r,int g,int b);
	void MFC_FillElement2(int nn,double *nx,double *ny,CDC *dc,int r,int g,int b,double u[8]);
    void plot(double x,double y,int ipen,\
          int sc_xmax,int sc_ymax,CDC *dc);
    void iso3k (double h[3],double x[3],double y[3],\
            double h1, double hd, double h2,\
            CDC *dc,int width,int height);
    void iso4k (long el4[4], double h[], double x[], double y[],
            double h1, double hd, double h2,\
            CDC *dc,int width,int height);
    void plot4k (int nstep, double rs[2],long el4[4],double xk[],double yk[],double ff[2][4],\
             CDC *dc,int width,int height);
    // Coordinate transformations
    int xpixel(double x);
    int ypixel(double y);
    int width,height;
    CString m_strIsolineLabel;
    bool label;
    BOOL display_labels;
    int label_elements;
// Dialogfelddaten
	enum { IDD = IDD_DIALOG_GRAPHICS };
    CListBox    m_LBTime;
	double	m_dXmin;
	double	m_dXmax;
	double	m_dYmin;
	double	m_dYmax;
    double m_dDX;
    double m_dDY;
    double m_dXStep;
    double m_dYStep;
	//CString	m_strUmin;
	//CString	m_strUmax;
    double m_dUmin,m_dUmax;
    int m_bDisplayIC;
    int m_iDisplayBC;
    int m_iDisplayPLY;
    int m_iDisplayMSH;
    int m_iDisplayPNT;
    int m_iDisplaySUF;
    int m_iDisplayIsolines;
    int m_iDisplayObservaWells;
    int m_iDisplayIsosurfaces;
    int m_iDisplayMSHLine;
    int m_iDisplayMSHTri;
    BOOL m_bDisplayMSHQuad;
    BOOL m_bDisplayMSHPris;
	BOOL m_bDisplayMSHTet;
    CString m_strPCSName;
    bool m_bDataRFO;
    int m_iQuantityIndex;
    CString m_strQuantityName;
    int m_iTimeSelected;
    bool m_bTimeSelectedAll;
    int m_levels;
    CString m_strPolylineName;
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CGraphics)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
public:
  CFont IsolineFont;
  CPen RedBoldPen;
protected:
  CFont AxisFont;
  CFont NumberFont;
  CFont LargeNumberFont;
  CFont FontArial14;
  CPen BlackDashedPen;
  // Pens solid
  CPen BlackSolidPen;
  CPen RedSolidPen;
  CPen ZyanSolidPen;
  CPen MagentaSolidPen;
  CPen YellowSolidPen;
  CPen BlueSolidPen;
  CPen GreenSolidPen;
  CPen DarkYellowSolidPen;
  CPen HighlightingPen;
  // Pens bold
  CPen BlackBoldPen;
  CPen BlackSolidBoldPen;
  CPen RedSolidBoldPen;
  CPen GreenBoldPen;
  CPen GreenSolidBoldPen;
  CPen BlueBoldPen;
  CPen BlueSolidBoldPen;
  CPen DarkPinkSolidBoldPen;
  CPen DarkYellowSolidBoldPen;
  CPen BlueVeloPen;
  // Pens dashed
  CPen BlueDashedPen;
  // Brushes
  CBrush BlackBrush;
  CBrush RedBrush;
  CBrush GreenBrush;
  CBrush HellBlauBrush;
  CBrush BlueBrush;
  CBrush ZyanBrush;
  CBrush MagentaBrush;
  CBrush WhiteBrush;
  CBrush DarkBlueBrush;
  CBrush DarkGreenBrush;
  CBrush DarkZyanBrush;
  CBrush DarkRedBrush;
  CBrush DarkMagentaBrush;
  CBrush DarkYellowBrush;
  CBrush DarkGrayBrush;
  CBrush LightGrayBrush;
  CBrush YellowBrush;
  CBrush TransparentBlackBrush;
  CBrush HighlightingBrush;
  char c_string[80];
  int m_iNx, m_iNy;
  // draw functions
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
public:
    BOOL m_bDisplayTINs;
    BOOL m_bDisplayVOL;
public:
    BOOL m_bShowLabels;
    BOOL m_bDisplayPLYPoints;
    BOOL m_bDisplayPLYLines;
    BOOL m_bDisplayNOD;
    BOOL m_bDisplayMSHDoubleElements;
    BOOL m_bDisplayMaterialGroups;
    BOOL m_bDisplayST;
    // Data
    FEMNodesElements *m_nodes_elements;
    afx_msg void OnLbnSelchangeListTimes();
    afx_msg void OnBnClickedButtonSelectAll();
  
    afx_msg void OnBnClickedButtonUpdate();
    BOOL m_showaxis;
    BOOL m_Displaylegend;    
    CComboBox m_CB_MSH;
    CComboBox m_CB_PCS;
    afx_msg void OnCbnSelchangeComboMSH();
    afx_msg void OnCbnSelchangeComboPCS();
    CListBox m_LB_FCT;
    afx_msg void OnLbnSelchangeListFCT();
    afx_msg void OnBnClickedButtonSelectAllFCT();
    afx_msg void OnBnClickedButtonSelectNoneFCT();
    BOOL m_bDisplayPNTLabels;
    BOOL m_bDisplayPLYLabels;
    BOOL m_bDisplaySFCLabels;
    BOOL m_bDisplayVOLLabels;
    afx_msg void OnBnClickedButtonFctInterpolate();
    CString m_strFCTName;
    CComboBox m_CB_Quantities;
    afx_msg void OnCbnSelchangeComboQuantities();
    CString m_strMSHName;
    afx_msg void OnBnClickedButtonCalcMinMax();
    afx_msg void OnEnChangeEditXMin();
    afx_msg void OnEnChangeEditXMax();
    afx_msg void OnBnClickedCheckDisplayMSH(); //OK...
    BOOL m_bDisplayMSHLabels;
    afx_msg void OnBnClickedCheckDisplayMSHLabels();
    afx_msg void OnBnClickedCheckDisplaySFC();
    afx_msg void OnBnClickedCheckDisplayPLY();
    afx_msg void OnBnClickedCheckDisplayPNT();
    BOOL m_bDisplayMSHHex;
    afx_msg void OnBnClickedCheckDisplayIC();
    afx_msg void OnBnClickedCheckDisplayBC();
    afx_msg void OnBnClickedCheckDisplayST();
    afx_msg void OnBnClickedCheckDisplayIsolines();
    afx_msg void OnBnClickedCheckDisplayIsosurfaces();
    afx_msg void OnEnChangeEditUmax();
    afx_msg void OnEnChangeEditUmin();
    BOOL m_bDisplayBMP;
    afx_msg void OnBnClickedCheckDisplayBMP();
    afx_msg void OnBnClickedCheckDisplayMATGroups();
    afx_msg void OnBnClickedButtonOK();
    afx_msg void OnBnClickedButtonDrawMSHAll();
    afx_msg void OnBnClickedCheckDisplayPlyPnt();
    afx_msg void OnBnClickedCheckDisplayPlyLin();
    afx_msg void OnBnClickedShowPlyLabels();
    afx_msg void OnBnClickedShowPntLabels();
    afx_msg void OnBnClickedShowSfcLabels();
    afx_msg void OnBnClickedCheckDisplayMshMat();
    BOOL m_bDisplayMATGroupsMSH;
    afx_msg void OnBnClickedCheckDisplayVOL();
    afx_msg void OnBnClickedCheckDisplayNod();
    long m_lNoElements2Draw;
    afx_msg void OnEnChangeEditMshElementsDraw();
    afx_msg void OnLbnSelchangeListDOM(); //OK
    CListBox m_LB_DOM;
    BOOL m_bDisplayDOM;
    afx_msg void OnBnClickedCheckDisplayDOM();
    afx_msg void OnBnClickedButtonWriteDomTec();
    CComboBox m_CB_MAT;
    afx_msg void OnCbnSelchangeComboMat();
    BOOL m_bDisplaySFC_MAT;
    afx_msg void OnBnClickedShowSfcMat();
    BOOL m_bDisplaySFCValues;
    afx_msg void OnBnClickedCheckDisplaySFCValues();
};


//CQuantity   07/2004 CC Implementation--------------------------------------------------
class CQuantity
{

public:
	CQuantity(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQuantity();
public:
    CString m_strQuantityName;
    double min;
    double max;
    bool selected;
    int index;
    int levels;
 

};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_GS_GRAPHICS_H__F6B19B1E_0E1D_4E76_9C0F_305B9553DF72__INCLUDED_

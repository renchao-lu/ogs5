// gs_legend.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_legend.h"
#include ".\gs_legend.h"


// CGSLegend dialog

IMPLEMENT_DYNAMIC(CGSLegend, CDialog)
CGSLegend::CGSLegend(CWnd* pParent /*=NULL*/)
	: CDialog(CGSLegend::IDD, pParent)
{
}

CGSLegend::~CGSLegend()
{
}

void CGSLegend::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGSLegend, CDialog)
    ON_WM_PAINT()
END_MESSAGE_MAP()


// CGSLegend message handlers

void CGSLegend::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CDialog::OnPaint() for painting messages
     CPen RedSolidPen;
     RedSolidPen.CreatePen(PS_SOLID, 0, RGB(255,0,0));
     dc.SelectObject(&RedSolidPen);
     dc.MoveTo(5,5);
     dc.LineTo(45,5);
     dc.TextOut (50,5,"FIRST",5);
     
}

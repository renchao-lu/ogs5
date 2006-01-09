// gs_plot.cpp : implementation file
//
/**************************************************************************
GeoLibGUI-Method:plot concentration
Task: 
Programing:
05/2004 CC Implementation
06/2004 CC Modification
**************************************************************************/
#include "stdafx.h"
#include "GeoSys.h"
#include "GeoSysDoc.h"
#include ".\gs_plot.h"


// CGSPlot dialog

IMPLEMENT_DYNAMIC(CGSPlot, CDialog)
CGSPlot::CGSPlot(CWnd* pParent /*=NULL*/)
	: CDialog(CGSPlot::IDD, pParent)
{
    
}

CGSPlot::~CGSPlot()
{
}

void CGSPlot::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGSPlot, CDialog)
    ON_WM_PAINT()
    
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CGSPlot message handlers

/**************************************************************************
GeoLibGUI-Method: draw concentration curve on dialog
Task: 
Programing:
05/2004 CC Implementation
06/2004 CC Modification
**************************************************************************/
void CGSPlot::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: Add your message handler code here
    // Do not call CDialog::OnPaint() for painting messages
    // dc.SetBkColor(RGB(0,0,0));
    CRect rcClient;
    GetClientRect(&rcClient);

    int iDestHeight = rcClient.Height();
    int iDestWidth = rcClient.Width();
    //pen
    LOGBRUSH logBrush;
    logBrush.lbStyle = BS_SOLID;
    logBrush.lbColor = RGB(255,0,255);
    CPen myPen2(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_ROUND, 2, &logBrush);
   
    dc.SelectObject(&myPen2);
    //////////////////////////////////////////////////////////
    //font
    CFont font_hor;
    LOGFONT lf;
 
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = 14;
    
    //lf.lfItalic = 1;
    strcpy(lf.lfFaceName, "Tahoma");
    VERIFY(font_hor.CreateFontIndirect(&lf));
    dc.SelectObject(&font_hor);
   //----------------------------------------------------------

    
    double timevalue;
    double max1 = 10e-5;
    int Xmax;
    int timestep = m_document->m_nodes_elements->number_of_times;
    double Axistepy = ( double)MaxConcentration/10.0 ;

 //------------------------------------------------------------
   
//calculation of the max step of concentration

    for(int j = 0;j <timestep;j++)
    {
    timevalue = m_document->m_nodes_elements->values_at_times[j];
    if(timevalue >= max1)
        max1 = timevalue;
    }
    Xmax = (int)(max1/86400.) + 1;
    double AxisStep =(double)Xmax/10.0;
//------------------------------------------
   
    char c_str[20];
    //sprintf(c_str,"%i: %13.7e",i,value);
    
 //draw hor axes.
    dc.MoveTo(50,iDestHeight-40 );
    dc.LineTo(350,iDestHeight-40 );
    dc.TextOut(iDestWidth/2- 20,iDestHeight-20,"Time(day)",9);
    
    //origin
    dc.TextOut(40,iDestHeight-40,"0",1);
    //draw axis x:
    //change into iteration:
    for(int step = 1; step<= 10; step++){
    
    dc.MoveTo(50+step*30,iDestHeight-40);
    dc.LineTo (50+step*30,iDestHeight-43);
   
    sprintf(c_str,"%lg",AxisStep*step);
   // sprintf(c_str,"%g",0.2*step);
    dc.TextOut(50+step*30-5,iDestHeight-35,(CString)c_str);
    

   }
 //--------------------------------------------------------------
    //draw axis y:
  
    char c_stry[20];
    for(int stepy = 1; stepy<= 10; stepy++){

    dc.MoveTo(50,iDestHeight-40-18*stepy);
    dc.LineTo (53,iDestHeight-40-18*stepy);

    sprintf(c_stry,"%lg",(double)stepy*Axistepy);
      //sprintf(c_stry,"%g",0.02*stepy);
    dc.TextOut(20,iDestHeight-40-18*stepy-5,(CString)c_stry);

    }
    //---------------------------------------------------------------
    //draw vertical fonts for y axis;
    CFont font_ver;
    lf.lfEscapement = 900;
    VERIFY(font_ver.CreateFontIndirect(&lf));
    dc.SelectObject(&font_ver);

   
    dc.MoveTo(50,iDestHeight-10);
    dc.LineTo (50,iDestHeight-180-41);
    dc.TextOut(0,iDestHeight/2 + 20,"Concentration(c)",16);
   //--------------------------------------------------------------
   

   ///////////////////////////////////////////////////////////////    
   //plot concentration: need to transform real data into coordinate system.

    //-------------------------------------------------------------
    LOGBRUSH logBrush2;
    logBrush2.lbStyle = BS_SOLID;
    logBrush2.lbColor = RGB(0,0,255);
    CPen myPen3(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_ROUND, 1, &logBrush2);
   
    dc.SelectObject(&myPen3);
    double x = m_document->m_nodes_elements->values_at_times[0]/86400;
   
    double y = m_document->m_nodes_elements->nodevalues[m_out->msh_node_number][1][0];
    dc.MoveTo((int)(50.0 + (double)x/AxisStep*30.0),(int)(iDestHeight-40-(double)y/Axistepy*18.0)); //OK
    for(int m = 1;m <timestep;m++ )
    {
    int x = (int)(50 + (double)(m_document->m_nodes_elements->values_at_times[m])/(86400.0*AxisStep)*30.0);
    int y = (int)(iDestHeight-40-(double)(m_document->m_nodes_elements->nodevalues[m_out->msh_node_number][1][m])/Axistepy*18.0); //OK
    dc.LineTo(x,y);
    }
 
    //deconstructor
    myPen2.DeleteObject();
    myPen3.DeleteObject();
    font_hor.DeleteObject();

    font_ver.DeleteObject();

}

/**************************************************************************
GeoSys GUI - Object: set the background color of dialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
BOOL CGSPlot::OnInitDialog() 
{
  CDialog::OnInitDialog();
  m_brush.CreateSolidBrush(RGB(0, 0, 0)); 

 

  return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/**************************************************************************
GeoSys GUI - Object: set the background color of dialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
HBRUSH CGSPlot::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    pDC=pDC;//TK
    pWnd=pWnd;//TK
    nCtlColor=nCtlColor;//TK
    //HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Change any attributes of the DC here

    // TODO:  Return a different brush if the default is not desired
    //return hbr;
    return m_brush;

}

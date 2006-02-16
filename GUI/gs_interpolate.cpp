// gs_interpolate.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "gs_interpolate.h"
#include ".\gs_interpolate.h"
#include "gs_idw.h"
#include "gs_kriging.h"
#include "gs_rbf.h"

// interpolation
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <strstream> 
using namespace std;

char ipolfile[80];
// GSInterpolate dialog

IMPLEMENT_DYNAMIC(GSInterpolate, CDialog)
GSInterpolate::GSInterpolate(CWnd* pParent /*=NULL*/)
	: CDialog(GSInterpolate::IDD, pParent)
{
    
  
}

GSInterpolate::~GSInterpolate()
{
}

void GSInterpolate::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_METHOD, m_method);
    DDX_Control(pDX, IDC_LIST_VARIABLE, m_variable);
    
}


BEGIN_MESSAGE_MAP(GSInterpolate, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnBnClickedButtonAdvanced)
    ON_BN_CLICKED(IDOK, OnBnClickedInterpolate)
END_MESSAGE_MAP()


// GSInterpolate message handlers
/**************************************************************************
GeoSys GUI - Object: OnInitDialog
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
BOOL GSInterpolate::OnInitDialog() 
{
  CDialog::OnInitDialog();
    
  m_method.InitStorage(3,1024);
  m_method.ResetContent();
  m_method.AddString("Inverse Distance Weighting");
  m_method.AddString("Kriging");
  m_method.AddString("Radial Basis Function");


  m_method.SetCurSel(0);
  
  
  m_variable.InsertString(0,"Pressure");
  m_variable.InsertString(1,"Concentration");
  m_variable.SetCurSel(0);
  UpdateData(FALSE);
 return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/**************************************************************************
GeoSys GUI - Object: OnBnClickedButtonAdvanced
Task: 
Programing:
07/2004 CC/CB Implementation
**************************************************************************/
void GSInterpolate::OnBnClickedButtonAdvanced()
{
    UpdateData(TRUE);
  
        int pos = m_basename.ReverseFind('\\');
        int lengthstring = m_basename.GetLength();
        int pos2 = lengthstring - pos -1;
        m_basename = m_basename.Right(pos2);

        int method = m_method.GetCurSel()+1;

        if(m_variable.GetCurSel() == 0)         strcpy(ipolfile, "interpolmethod0");
        else if (m_variable.GetCurSel() == 1)   strcpy(ipolfile, "interpolmethod1");   	   
 
        ofstream out;
        out.open(ipolfile);
        // obligatory: general values
        out << m_basename << endl;   // change later
        out << method << endl;
 
    if(m_method.GetCurSel()== 0)
    {
        //IDW
        CGSIdw gs_idw;
        gs_idw.DoModal();
        m_anglevalue = gs_idw.m_anglevalue;
        m_ratiovalue = gs_idw.m_ratiovalue;
        m_powervalue = gs_idw.m_powervalue;
        
        m_smoothingvalue = gs_idw.m_smoothingvalue;
        //defaultsetting = gs_idw.m_settings;
          //-----------------------------------------


      
        // obligatory: default flag
        out << gs_idw.m_settings << endl;
        // optional: user defined parameters
        out << m_anglevalue  << endl;
        out << m_ratiovalue << endl;
        out << m_powervalue  << endl;
        out << m_smoothingvalue << endl;
        out.close();

          
    }
    else if(m_method.GetCurSel()== 1)
    {
    CGSKriging gs_kriging;
    gs_kriging.DoModal();
    trendtype = gs_kriging.trendtype;
    variotype = gs_kriging.variotype+1;
    out << gs_kriging.m_settings << endl;
        // optional: user defined parameters
    out << gs_kriging.m_anglevalue  << endl;
    out << gs_kriging.m_ratiovalue << endl;
    out << variotype  << endl;
    out << gs_kriging.m_nuggetvalue << endl;
    out << gs_kriging.m_slopevalue << endl;
    out << gs_kriging.m_sillvalue << endl;
    out << gs_kriging.m_rangevalue << endl;
    out <<trendtype << endl;


    out.close();

    }
    else if(m_method.GetCurSel()== 2)
    {
        // RBF
        CGSRbf gs_rbf;
        gs_rbf.DoModal();
        m_anglevalue = gs_rbf.m_anglerbfvalue;
        m_ratiovalue = gs_rbf.m_ratiorbfvalue;
        m_r2value = gs_rbf.m_rsquaredvalue;
        RBFtype = gs_rbf.m_functionvalue+1;
        
       
        // obligatory: default flag
        out << gs_rbf.m_settings << endl;
        // optional: user defined parameters
        out << m_anglevalue  << endl;
        out << m_ratiovalue << endl;
        out << RBFtype  << endl;
        out << m_r2value << endl;
        out.close();
    
    }
 }
/**************************************************************************
GeoSys GUI - Object: OnBnClickedButtonAdvanced
Task: 
Programing:
07/2004 CC Implementation
**************************************************************************/
void GSInterpolate::OnBnClickedInterpolate()
{
    strcpy(ipolfile, "interpolmethod");
   
    int n=2-1; // number of variables-1=index of file
    char index[10];
	itoa (n,index,10);
   	strcat(ipolfile, index);
    ifstream ein;
    ein.open(ipolfile);
    if(ein){
        ein.close();
   		char *execute; //SB
        execute ="interpol.exe"; //SB
        system(execute);
    }
    else{ 
        cout << "warnung!";
        //print warning
    }
    OnOK();
}
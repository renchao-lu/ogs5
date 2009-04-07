// petrel_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "GeoSys.h"
#include "petrel_dlg.h"
#include <iostream>
#include <fstream>
extern string GetLineFromFile1(ifstream*);

// CDialogPETREL dialog

IMPLEMENT_DYNAMIC(CDialogPETREL, CDialog)

CDialogPETREL::CDialogPETREL(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPETREL::IDD, pParent)
{

}

CDialogPETREL::~CDialogPETREL()
{
}

void CDialogPETREL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPETREL, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_PLY, &CDialogPETREL::OnBnClickedButtonCreatePly)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_ELE, &CDialogPETREL::OnBnClickedButtonCreateEle)
END_MESSAGE_MAP()


// CDialogPETREL message handlers

void CDialogPETREL::OnBnClickedButtonCreatePly()
{
    // TODO: Add your control notification handler code here
//Declarations
    string petrel_file_name,line_string;
    char buffer[MAX_ZEILE];
    std::stringstream line_stream;
    double xyz[3];  
    float fdummy;
  //File dialog
    CFileDialog fileDlg(TRUE,"txt",NULL,OFN_ENABLESIZING,"petrel_well_trace_data(*.txt)|*.txt|");
    if (fileDlg.DoModal()==IDOK) 
    {
  //File handling
    petrel_file_name = fileDlg.GetPathName();
  //Open PETREL file
    ifstream petrel_file;
    petrel_file.open(petrel_file_name.data(),ios::in);
    if(!petrel_file.good()) return;
  //Read file header
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
  //Create GEO data
    while(!petrel_file.eof()) 
    {
  //MD              X              Y             Z          
  //taking the line from the file
    line_string = GetLineFromFile1(&petrel_file); 
    line_stream.clear();
  //copy string to stringstream
    line_stream.str(line_string);
  //if line is empty skip 
    if(line_string.size()<1) break; 
    line_stream >> fdummy >> xyz[0] >> xyz[1] >> xyz[2] >> ws;
  //read data as a block:
    CGLPoint *m_pnt = NULL; 
    m_pnt = new CGLPoint();
    m_pnt->x = xyz[0];
    m_pnt->y = xyz[1];
    m_pnt->z = xyz[2]; 
    m_pnt->id = (long)gli_points_vector.size();
    gli_points_vector.push_back(m_pnt);
    }

    CGLPolyline* m_ply = NULL;
    m_ply = new CGLPolyline();
    m_ply->id = (long)polyline_vector.size();
    for (long i=0 ; i < (long)gli_points_vector.size() ; i++ )
    {
    m_ply->point_vector.push_back(gli_points_vector[i]);
    }
    polyline_vector.push_back(m_ply);
    petrel_file.close(); 
    }
    GEOWrite("PETREL-CO2-FLOW");
}

void CDialogPETREL::OnBnClickedButtonCreateEle()
{
    // TODO: Add your control notification handler code here
 //Declarations
    string petrel_file_name,line_string;
    char buffer[MAX_ZEILE];
    std::stringstream line_stream;
    double xyz[3], a[3000];
    long i=0;
    long DelN=98;
    long DelL=27;
    long DelS=2646;
    long DelH=1;
    CFEMesh *m_msh = NULL;
    m_msh = new CFEMesh();
  //File dialog
    CFileDialog fileDlg(TRUE,"txt",NULL,OFN_ENABLESIZING,"petrel_well_trace_data(*.txt)|*.txt|");
    if (fileDlg.DoModal()==IDOK) 
    {
  //File handling
    petrel_file_name = fileDlg.GetPathName();
  //Open PETREL file
    ifstream petrel_file;
    petrel_file.open(petrel_file_name.data());
    if(!petrel_file.good()) return;
    petrel_file.seekg(0L,ios::beg); 
  //Read file header
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
    petrel_file.getline(buffer,MAX_ZEILE); // just read line
  //Create GEO data
    while(!petrel_file.eof()) 
    {
  //MD              X              Y             Z          
  //taking the line from the file
   esc: line_string = GetLineFromFile1(&petrel_file);
    line_stream.clear();
  //copy string to stringstream
    line_stream.str(line_string);
  //if line is empty skip 
    if(line_string.size()<1) break; 
    line_stream >> xyz[0] >> xyz[1] >> xyz[2] >> ws;
  //read data as a block:
    CNode *m_nod = NULL;
    m_nod = new CNode(i);
    a[i]=xyz[1];
    if(a[i]==a[i-1]) goto esc; 
    m_nod->SetCoordinates(xyz);    
    m_nod->SetIndex(i); 
    i+=1;
    m_msh->nod_vector.push_back(m_nod); 
    }
    petrel_file.close();
    }

    for(long k1=0;k1<DelH;k1++) 
    {
    for(long k=0;k<DelL-1;k++) 
    {
    for ( long j = 0; j <= DelN-2; j = j++ ) 
    {
    CElem* m_ele=NULL;
    m_ele = new CElem();
    m_ele->SetPatchIndex(0);
    m_ele->GetPatchIndex();
    m_ele->SetIndex((DelN-1)*k+j);
    m_ele->SetElementType(3);
    m_ele->SetNodesNumber(8);
    m_ele->nodes_index.resize(m_ele->GetNodesNumber(0));
    m_ele->nodes_index[3]=k1*DelS+DelN*k+j;
    m_ele->nodes_index[0]=k1*DelS+DelN*k+j+1;
    m_ele->nodes_index[1]=k1*DelS+DelN+(DelN)*k+j+1;
    m_ele->nodes_index[2]=k1*DelS+DelN+(DelN)*k+j;
    m_ele->nodes_index[4]=(k1+1)*DelS+DelN*k+j+1;
    m_ele->nodes_index[5]=(k1+1)*DelS+DelN+(DelN)*k+j+1;
    m_ele->nodes_index[6]=(k1+1)*DelS+DelN+(DelN)*k+j;
    m_ele->nodes_index[7]=(k1+1)*DelS+DelN*k+j;
    m_msh->ele_vector.push_back(m_ele);
    }
    }
    }

    fstream msh_file;
    msh_file.open("PETREL-CO2-FLOW.msh",ios::trunc|ios::out);
    if(!msh_file.good()) return;
    m_msh->pcs_name="MULTI_PHASE_FLOW";
    m_msh->InitialNodesNumber();
    m_msh->Write(&msh_file);
    msh_file.close();  
}

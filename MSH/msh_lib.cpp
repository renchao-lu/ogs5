/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C
#include "math.h"
// C++
#include <string>
#include <vector>
using namespace std;
// GEOLib
#include "geo_lib.h"
#include "geo_strings.h"
// MSHLib
#include "msh_lib.h"
#include "msh_nodes_rfi.h"
// PCSLib
#include "mathlib.h"
#include "nodes.h"
#include "elements.h"
extern void RFConfigRenumber(void);
extern void ConfigRenumberProperties(void);
extern int CreateEdgeList(void);
extern int CreatePlainList(void);
extern int ReadRFIFile(string g_strFileNameBase);
#include "rf_pcs.h"
#include "gs_project.h"

vector<Mesh_Group::CFEMesh*>fem_msh_vector;

#define FEM_FILE_EXTENSION ".msh"

double msh_x_min,msh_x_max; //OK
double msh_y_min,msh_y_max; //OK
double msh_z_min,msh_z_max; //OK
double msh_x_mid,msh_y_mid,msh_z_mid; //OK

bool msh_file_binary = false;
#define MSH_SIZE 1e5

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
04/2005 OK Implementation
**************************************************************************/
void MSHDelete(string m_msh_name)
{
  CFEMesh* m_fem_msh = NULL;
  int fem_msh_vector_size = (int)fem_msh_vector.size();
  for(int i=0;i<fem_msh_vector_size;i++){
    m_fem_msh = fem_msh_vector[i];
    if(m_fem_msh->pcs_name.compare(m_msh_name)==0){
      if(m_fem_msh) delete m_fem_msh;
      fem_msh_vector.erase((fem_msh_vector.begin()+i));
    }
  }
}

/**************************************************************************
GeoSys-Method: MSHOpen
Task: 
Programing:
11/2003 OK Implementation
**************************************************************************/
void MSHOpen(string file_name_base)
{
  if(!NODListExists())
    CreateNodeList();
  if(!ELEListExists())
    ElCreateElementList();
  CreateEdgeList();
  CreatePlainList();
  ReadRFIFile(file_name_base);
  ConfigRenumberProperties();
  RFConfigRenumber();
  ELEConfigJacobianMatrix();
  //OK3909  CalcElementsGeometry();
}

/**************************************************************************
ROCKFLOW - Modul:
Aufgabe:
Moves Z-Values of nodes according to a specified surface in 
filename.dat file. Works for regular grids only!!!
const char *dateiname     :   file name
const int NLayers         :   Number of layers in the
const int row             :   row number of the row to be mapped
Programmaenderungen:
10/2003 WW/MB Erste Version
04/2004 WW faster method
02/2005 CC Modification change the grid height read order  
07/2005 OK MSH
09/2005 OK/CC Empty lines in DAT files
***************************************************************************/
void MSHMapping(const char *dateiname, const int NLayers,\
            const int row, const int DataType,\
            CFEMesh*m_msh)
{
  FILE *f;
  fpos_t pos;
  char *s;  /* gelesene Zeile */
  int i, j;
  int ncols = 0;
  int nrows = 0;
  int nx, ny;
  int NPoints = 0;
//  int count = 0;
  double x,y, z;
  double MinX = 1.0e+10; 
  double MinY = 1.0e+10; 
  double MaxX = -1.0e+10; 
  double MaxY = -1.0e+10; 
  double dx =  1.0e+10;
  double dy =  1.0e+10;
  double xi =  0.0;
  double eta =  0.0;
  double locX[4];
  double locY[4];
  double locH0[4];
  double geo_tolerance = 1.0e-3;
  double* GridX =NULL;  
  double* GridY =NULL;  
  double* H0=NULL;
  double **H=NULL;
  double ome[4];
  int NNodesPerRow = 0;
  long NNodes = 0;
  char charbuff[41];
  long counter = 0; //OK
  string s_str; //OK/CC
  //----------------------------------------------------------------------
  if(m_msh)
    NNodes = (long)m_msh->nod_vector.size();
  else
    NNodes = NodeListSize();
  //----------------------------------------------------------------------
  // Open grid file
  if((f = fopen(dateiname,"rb"))==NULL)  {
    DisplayErrorMsg("Fehler: Cannot open .dat file. It may not exist !");
    abort();
  }
  s = (char *) Malloc(MAX_ZEILE);
  //----------------------------------------------------------------------
  switch(DataType)
  {
    //====================================================================
    case 1:   
       /* Count number of points of the grid */
       if( fgetpos(f, &pos ) != 0 )  {
         perror( "fgetpos error" );
         abort();
       }
       else  { 
         NPoints=-1;
         while(!feof(f))    {
           fgets(s,MAX_ZEILE,f);
           s_str = s; //OK/CC
           is_line_empty(&s_str);
           if(s_str.size()>2) 
           NPoints++;      
         }
       }
       rewind(f);
       fsetpos(f, &pos ); 
       /* Allocate memory for grid and the specified surface*/
       GridX = (double *) Malloc(NPoints * sizeof(double));
       GridY = (double *) Malloc(NPoints * sizeof(double));
       H0 = (double *) Malloc(NPoints * sizeof(double));
       /* Read surface data and determine the range*/
       for(i=0; i<NPoints; i++)   {
         if(fgets(s,MAX_ZEILE,f)==NULL)	 {
           //DisplayErrorMsg("Error: End of .grd file reached ");
#ifdef MFC
           AfxMessageBox("1");
#endif
           abort();
         }
         s_str = s; //OK/CC
         is_line_empty(&s_str);
         if(s_str.size()<=2){
           --i;
           continue; //OK/CC
         }
         if (sscanf(s," %lf %lf %lf  ", &(GridX[i]),&(GridY[i]),&(H0[i]))!=3) 	   {
           //DisplayErrorMsg("Error: More than one integal in .grd file!");
#ifdef MFC
           AfxMessageBox("Empty line in DAT file");
#endif
           abort();
         }
       }
	   for(i=0; i<NPoints; i++)   {
          // Determine the range of the grid  
          if(GridX[i]>=MaxX) MaxX = GridX[i];   
          if(GridY[i]>=MaxY) MaxY = GridY[i];   
          if(GridX[i]<=MinX) MinX = GridX[i];   
          if(GridY[i]<=MinY) MinY = GridY[i];
          if(i>0&&(GridX[i]-GridX[i-1]!=0)) 
          if(dx>=fabs(GridX[i]-GridX[i-1])) dx = fabs(GridX[i]-GridX[i-1]);
          if(i>0&&(GridY[i]-GridY[i-1]!=0)) 
	      if(dy>=fabs(GridY[i]-GridY[i-1])) dy = fabs(GridY[i]-GridY[i-1]);
       } 
       //.................................................................
       if(dx>dy)
         geo_tolerance = dy*1e-3;
       else
         geo_tolerance = dx*1e-3;
       //.................................................................
	   // Copy the data of H0 to H[][] in order to enhance the computation
       ncols = (int)((MaxX-MinX)/dx)+1;
       nrows = (int)((MaxY-MinY)/dy)+1;
       H = new double*[nrows];
	   double dist;
	   for(i=0; i<nrows; i++)  
         H[i] = new double[ncols];
       for(i=0; i<nrows; i++)
	   {
         y = MinY+i*dy;
         for(j=0; j<ncols; j++)
		 {
           x = MinX+j*dx;         
           for(int k=0; k<NPoints; k++)  
		   {
             dist = sqrt((x-GridX[k])*(x-GridX[k])+(y-GridY[k])*(y-GridY[k]));
			 if(dist<geo_tolerance)
			 {
               counter++; //OK
			   H[i][j] = H0[k];
			   break;
			 } 
		   }                        
		 }
	   }
       if(counter<NPoints){
         cout << "Warning: not all grid points found, increase tolerance" << endl;
#ifdef MFC
         AfxMessageBox("Warning: not all grid points found, increase tolerance");
#endif
       }
	   break;
    //====================================================================
	 case 2:
       ncols = 0;
       nrows = 0;
	   double x0, y0, z0;
       fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d  ", charbuff, &ncols);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %d ", charbuff, &nrows);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &x0);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf", charbuff, &y0);
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &dx);
	   dy = dx;
	   fgets(s,MAX_ZEILE,f);
       sscanf(s," %40s %lf ", charbuff, &z0);
       MinX = x0;
       MaxX = x0+dx*ncols;
       MinY = y0;
       MaxY = y0+dy*nrows;
       // Allocate memory for grid and the specified surface
       H = new double*[nrows];
	   for(i=0; i<nrows; i++)  H[i] = new double[ncols];
	   // Compute the grid points:
	 /*  for(i=0; i<nrows; i++) 
	   {          
    	  for(j=0; j<ncols; j++)
		  {
             fscanf(f,"%lf", &H[i][j]);
		  } 
	   } */
       //CC 02/2005
         // Compute the grid points:
	   for(i=nrows-1; i>=0; i--) 
	   {          
    	  for(j=0; j<ncols; j++)
		  {
             fscanf(f,"%lf", &H[i][j]);
		  } 
	   } 
	   //
       break; 
  }
  //----------------------------------------------------------------------
  NNodesPerRow = NNodes / (NLayers+1); 
  /* 1. Compute the height of points to be attached*/
  for(i=0; i<NNodes; i++)   {  
    if(i >= (row-1) * NNodesPerRow  &&  
       i <= (row * NNodesPerRow) -1 )   
    {           
      if(m_msh){ //OK
        x = m_msh->nod_vector[i]->X();
        y = m_msh->nod_vector[i]->Y();
      }
      else{
        x = GetNodeX(i);
        y = GetNodeY(i);
      }
      //..................................................................
	  if(x<MinX||x>MaxX||y<MinY||y>MaxY)   {
#ifdef MFC
        AfxMessageBox("Error: MSH coordinates are out of x-y range of MAP");
#endif
        // Release memory
        if(GridX) GridX  = (double*) Free(GridX);
        if(GridY) GridY  = (double*) Free(GridY);
        if(H0) H0 = (double*) Free(H0);
        if(s) s = (char *)Free(s);
        return;
	  }
      //..................................................................
	  nx = (int)((x-MinX)/dx);	
      ny = (int)((y-MinY)/dy);
      if(nx*dx+MinX>=x)  nx -= 1;
      if(ny*dy+MinY>=y)  ny -= 1;
	  if(nx>ncols) nx = ncols-2;
	  if(ny>nrows) ny = nrows-2;
      if(nx<0) nx = 0;
      if(ny<0) ny = 0;
         
	  locX[0] = MinX+nx*dx;
	  locY[0] = MinY+ny*dy;
	  locH0[0] = H[ny][nx];

	  locX[1] = MinX+(nx+1)*dx;
	  locY[1] = MinY+ny*dy;
	  locH0[1] = H[ny+1][nx];

	  locX[2] = MinX+(nx+1)*dx;
	  locY[2] = MinY+(ny+1)*dy;
	  locH0[2] = H[ny+1][nx+1];

	  locX[3] = MinX+nx*dx;
	  locY[3] = MinY+(ny+1)*dy;
	  locH0[3] = H[ny][nx+1];

      // Interpolate 
      xi =  2.0*(x-0.5*(locX[0]+locX[1]))/dx;             
      eta =  2.0*(y-0.5*(locY[1]+locY[2]))/dy;              
      MPhi2D(ome, xi, eta);   
        
      z=0.0;
      for(j=0; j<4; j++) z += ome[j]*locH0[j]; 
      // Set new z component 
      if(m_msh){ //OK
        m_msh->nod_vector[i]->SetZ(z);
      }
      else{
        SetNodeZ(i, z);           
      }
    }		 
  }   
  //
/*
  CFileDialog dlg(FALSE, NULL, "new", OFN_ENABLESIZING ,
                 " Geometry Files (*.rfi)|*.rfi| All Files (*.*)|*.*||" );
  dlg.DoModal();
  CString m_filepath = dlg.GetPathName();
  //
  const char* cpsz = static_cast<LPCTSTR>(m_filepath);
  xxxxx = cpsz;     
  DATWriteRFIFile(xxxxx);
*/
  //----------------------------------------------------------------------
  // Release memory
  if(GridX) GridX  = (double*) Free(GridX);
  if(GridY) GridY  = (double*) Free(GridY);
  if(H0) H0 = (double*) Free(H0);
  if(s) s = (char *)Free(s);
  fclose(f);
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
05/2005 TK modified
**************************************************************************/
void FEMDeleteAll()
{
  for(int i=0;i<(int)fem_msh_vector.size();i++){
      delete fem_msh_vector[i];
      fem_msh_vector[i]=NULL;
  }
  fem_msh_vector.clear();
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
03/2005 OK Implementation
08/2005 WW Topology construction and rfi compatible
10/2005 OK BINARY
**************************************************************************/
bool FEMRead(string file_base_name)
{
 
  //----------------------------------------------------------------------
  FEMDeleteAll();  
  //----------------------------------------------------------------------
  CFEMesh *m_fem_msh = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  ios::pos_type position;
  //========================================================================
  // File handling
  msh_file_binary = false;
  string msh_file_name_bin = file_base_name + "_binary" + FEM_FILE_EXTENSION;
  string msh_file_name_ascii = file_base_name + FEM_FILE_EXTENSION;
  ifstream msh_file_bin;
  ifstream msh_file_ascii;
  msh_file_bin.open(msh_file_name_bin.c_str(),ios::binary|ios::in);
  if(msh_file_bin.good()){ 
    msh_file_binary = true;
  }
/*
  //......................................................................
  fem_msh_file_name = file_base_name + FEM_FILE_EXTENSION;
  ifstream fem_msh_file;
  //......................................................................
  fem_msh_file.open(fem_msh_file_name.data(),ios::in);
  if (!fem_msh_file.good()) 
    return false;
  fem_msh_file.getline(line,MAX_ZEILE);
  line_string = line;
  if(line_string.find("BINARY")!=string::npos)
    msh_file_binary = true;
  else
    msh_file_binary = false;
  fem_msh_file.close();
*/
  //----------------------------------------------------------------------
  cout << "MSHRead: ";
  if(msh_file_binary){
    cout << "BINARY file" << endl;
    if (!msh_file_bin.good()) 
      return false;
  }
  else{
    cout << "ASCII file" << endl;
    msh_file_ascii.open(msh_file_name_ascii.data(),ios::in);
    if (!msh_file_ascii.good()){
#ifdef MFC
      AfxMessageBox("Reading MSH file failed");
#endif
      return false;
    }
  }
  //----------------------------------------------------------------------
  // RFI - WW
if(!msh_file_binary){
  bool rfiMesh = true;
  getline(msh_file_ascii,line_string); // The first line
  if(line_string.find("#FEM_MSH")!=string::npos)
    rfiMesh = false;
  if(line_string.find("GeoSys-MSH")!=string::npos) //OK
    rfiMesh = false;
  msh_file_ascii.seekg(0L,ios::beg);
  if (rfiMesh) 
  {
     m_fem_msh = new CFEMesh();
     Read_RFI(msh_file_ascii, m_fem_msh);
        m_fem_msh->FaceNormal();      //YD 
     fem_msh_vector.push_back(m_fem_msh);
	 msh_file_ascii.close();
	 return true;
  }
}
  //========================================================================
  // Keyword loop
  //----------------------------------------------------------------------
  if(msh_file_binary){
    while(!msh_file_bin.eof()){
      char keyword_char[9];
      msh_file_bin.read((char*)(&keyword_char),sizeof(keyword_char));
      line_string = keyword_char;
      if(line_string.find("#STOP")!=string::npos)
        return true;
      //..................................................................
      if(line_string.find("#FEM_MSH")!=string::npos){ // keyword found
        m_fem_msh = new CFEMesh();
        position = m_fem_msh->ReadBIN(&msh_file_bin);
        m_fem_msh->FaceNormal();      //YD  
        fem_msh_vector.push_back(m_fem_msh);
        msh_file_bin.seekg(position,ios::beg);
      } // keyword found
    } // eof
    msh_file_bin.close();
  }
  //----------------------------------------------------------------------
  else{
    while(!msh_file_ascii.eof()){
      msh_file_ascii.getline(line,MAX_ZEILE);
      line_string = line;
      if(line_string.find("#STOP")!=string::npos)
        return true;
      //..................................................................
      if(line_string.find("#FEM_MSH")!=string::npos) { // keyword found
        m_fem_msh = new CFEMesh();
        position = m_fem_msh->Read(&msh_file_ascii);
        fem_msh_vector.push_back(m_fem_msh);
        msh_file_ascii.seekg(position,ios::beg);
      } // keyword found
    } // eof
    msh_file_ascii.close();
  }
  //========================================================================
  return true;
}
/**************************************************************************
MSHLib-Method: Read rfi file () 
Task:
Programing:
08/2005 WW Re-implememtation
**************************************************************************/
void Read_RFI(istream& msh_file,CFEMesh* m_msh)
{
  long id;
  long i=0;
  int NumNodes=0;
  int NumElements=0;
  int End = 1;
  double x,y,z;
  string strbuffer;

  CNode* node = NULL;
  CElem* elem = NULL;
  //----------------------------------------------------------------------
  while (End) 
  {
    getline(msh_file, strbuffer);// The first line
    msh_file>>i>>NumNodes>>NumElements>>ws;
    //....................................................................
	// Node data
    for(i=0;i<NumNodes;i++){
      msh_file>>id>>x>>y>>z>>ws;
      node = new CNode(id,x,y,z);
	  m_msh->nod_vector.push_back(node);
    }
    for(i=0;i<NumElements; i++)
	{
      elem = new CElem(i);
	  elem->Read(msh_file, 1);
      m_msh->ele_vector.push_back(elem);
	}
    End =0;
  }
}


/**************************************************************************
FEMLib-Method: 
Task:
Programing:
02/2006 WW Implementation
**************************************************************************/
void CompleteMesh()
{
   int i;
   bool quad=false;
   CRFProcess* m_pcs = NULL;
   for(i=0;i<(int)pcs_vector.size();i++){
     m_pcs = pcs_vector[i];
     if(m_pcs->pcs_type_name.find("DEFORMATION")!=string::npos)
     {
       quad=true;
       break;
     }
   }
   for(i=0;i<(int)fem_msh_vector.size(); i++)
   {
      fem_msh_vector[i]->ConstructGrid(quad);
      fem_msh_vector[i]->FillTransformMatrix();
      fem_msh_vector[i]->FaceNormal();
   }      
}
/**************************************************************************
FEMLib-Method:
Task: Master write functionn
Programing:
03/2005 OK Implementation
10/2005 OK BINARY
last modification:
**************************************************************************/
void FEMWrite(string file_base_name)
{
  int i;
  CFEMesh* m_fem_msh = NULL;
  string sub_line;
  string line_string;
  msh_file_binary = false;
  //----------------------------------------------------------------------
  // File handling
  string fem_msh_file_name = file_base_name + FEM_FILE_EXTENSION;
  fstream fem_msh_file;
  string msh_file_test_name = file_base_name + "_test" + FEM_FILE_EXTENSION;
  fstream msh_file_test;
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_fem_msh = fem_msh_vector[i];
    if(m_fem_msh->ele_vector.size()>MSH_SIZE)
      //msh_file_binary = true;
      msh_file_binary = false; //TK 21.12.05 default false / TODO!!!
  }
  if(msh_file_binary){
    fem_msh_file_name = file_base_name + "_binary" + FEM_FILE_EXTENSION;
    fem_msh_file.open(fem_msh_file_name.c_str(),ios::binary|ios::out);
    //msh_file_test.open(msh_file_test_name.c_str(),ios::trunc|ios::out);OK
    //msh_file_test.open(fem_msh_file_name.c_str(),ios::trunc|ios::out);//TK
    if(!fem_msh_file.good()) return;
  }
  else{
    fem_msh_file.open(fem_msh_file_name.c_str(),ios::trunc|ios::out);
    if(!fem_msh_file.good()) return;
    fem_msh_file.setf(ios::scientific,ios::floatfield);
    fem_msh_file.precision(12);
  }
  //----------------------------------------------------------------------
  for(i=0;i<(int)fem_msh_vector.size();i++){
    m_fem_msh = fem_msh_vector[i];
    if(msh_file_binary)
      m_fem_msh->WriteBIN(&fem_msh_file,&msh_file_test);
    else
      m_fem_msh->Write(&fem_msh_file);
  }
  //----------------------------------------------------------------------
  if(msh_file_binary){
    char binary_char[6] = "#STOP";
    fem_msh_file.write((char*)(&binary_char),sizeof(binary_char));
  }
  else{
    fem_msh_file << "#STOP";
  }
  fem_msh_file.close();
}

/**************************************************************************
MSHLib-Method: 
Task: 
Programing:
05/2005 OK
last modification:
**************************************************************************/
void MSHTopology()
{
  CFEMesh* m_msh = NULL;
  //----------------------------------------------------------------------
  // Soil elements
  m_msh = FEMGet("RICHARDS_FLOW");
  if(m_msh)
    m_msh->SetNOD2ELETopology();
  //----------------------------------------------------------------------
  // Groundwater elements
  m_msh = FEMGet("GROUNDWATER_FLOW");
  if(m_msh)
    m_msh->SetELE2NODTopology();
}



/**************************************************************************
FEMLib-Method:
Task:
Programing:
03/2005 OK Implementation
last modification:
**************************************************************************/
CFEMesh* FEMGet(string msh_name)
{
  int no_msh = (int)fem_msh_vector.size();
  // If there is only one msh file available, use it for all process. WW
  if(no_msh==1) return fem_msh_vector[0]; //WW
  CFEMesh* m_msh = NULL;
  for(int i=0;i<no_msh;i++){
    m_msh = fem_msh_vector[i];
    if(m_msh->pcs_name.compare(msh_name)==0)
      return m_msh;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
09/2004 OK Implementation
**************************************************************************/
void MSHCalcMinMaxMidCoordinates()
{
  double m_dXMin1 = 1.e+19;
  double m_dXMax1 = -1.e+19;
  double m_dYMin1 = 1.e+19;
  double m_dYMax1 = -1.e+19;
  double m_dZMin1 = 1.e+19;
  double m_dZMax1 = -1.e+19;
  double value;
  CFEMesh* m_msh = NULL;
  //----------------------------------------------------------------------
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
    for(long i=0;i<(long)m_msh->nod_vector.size();i++) 
    {
      value = m_msh->nod_vector[i]->X();
      if(value<m_dXMin1) m_dXMin1 = value;
      if(value>m_dXMax1) m_dXMax1 = value;
      value = m_msh->nod_vector[i]->Y();
      if(value<m_dYMin1) m_dYMin1 = value;
      if(value>m_dYMax1) m_dYMax1 = value;
      value = m_msh->nod_vector[i]->Z();
      if(value<m_dZMin1) m_dZMin1 = value;
      if(value>m_dZMax1) m_dZMax1 = value;
      //..................................................................
      // Shrink a bit
      msh_x_min = m_dXMin1 - 0.05*(m_dXMax1-m_dXMin1);
      msh_x_max = m_dXMax1 + 0.05*(m_dXMax1-m_dXMin1);
      msh_y_min = m_dYMin1 - 0.05*(m_dYMax1-m_dYMin1);
      msh_y_max = m_dYMax1 + 0.05*(m_dYMax1-m_dYMin1);
      msh_z_min = m_dZMin1 - 0.05*(m_dZMax1-m_dZMin1);
      msh_z_max = m_dZMax1 + 0.05*(m_dZMax1-m_dZMin1);
    }
  }
  //----------------------------------------------------------------------
  msh_x_mid = 0.5*(msh_x_min+msh_x_max);
  msh_y_mid = 0.5*(msh_y_min+msh_y_max);
  msh_z_mid = 0.5*(msh_z_min+msh_z_max);
  //----------------------------------------------------------------------
}


/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2004 OK Implementation
01/2005 OK File handling
09/2005 OK MSH ToDo
last modification: 
**************************************************************************/
void MSHWriteVOL2TEC(string m_msh_name)
{
  long i,j;
  CGLVolume *m_vol = NULL;
  vector<CGLVolume*>::const_iterator p_vol;
  string name("VOLUMES");
  vector<Surface*>::const_iterator p_sfc;
  string delimiter(", ");
  double x,y,z;
  CGLPoint m_point;
  ios::pos_type position;
  int vol_number = -1;
  Surface* m_sfc = NULL;
  //--------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  m_msh = FEMGet(m_msh_name);
  if(!m_msh)
    return;
  long no_nodes = (long)m_msh->nod_vector.size();
  long ep_layer = (long)m_msh->ele_vector.size() / m_msh->no_msh_layer;
  //--------------------------------------------------------------------
  // File handling
  string tec_path;
  CGSProject* m_gsp = GSPGetMember("gli");
  if(m_gsp)
    tec_path = m_gsp->path; 
  //======================================================================
  p_vol = volume_vector.begin();
  while(p_vol!=volume_vector.end()) {
    m_vol = *p_vol;
    if(m_vol->layer==0){ //OK
      p_vol++;
      continue;
    }
    p_sfc = m_vol->surface_vector.begin();
    m_sfc = *p_sfc;
    if(!m_sfc)
      return;
    //--------------------------------------------------------------------
    long jb = (m_vol->layer-1)*ep_layer;
    long je = jb + ep_layer;
    vol_number++;
    //--------------------------------------------------------------------
    // file handling
    string vol_file_name = tec_path + "VOL_" + m_vol->name + TEC_FILE_EXTENSION;
    fstream vol_file (vol_file_name.data(),ios::trunc|ios::out);
    vol_file.setf(ios::scientific,ios::floatfield);
	vol_file.precision(12);
    if (!vol_file.good()) return;
    vol_file.seekg(0L,ios::beg);
    //--------------------------------------------------------------------
    vol_file << "VARIABLES = X,Y,Z,VOL" << endl;
    //--------------------------------------------------------------------
    long no_mat_elements = 0;
    CElem* m_ele = NULL;
    vec<long>node_indeces(6);
    for(i=jb;i<je;i++){
      m_ele = m_msh->ele_vector[i];
	  if(m_ele->GetElementType()==6){
        m_ele->GetNodeIndeces(node_indeces);
        //nodes = m_msh->ele_vector[i]->nodes;
        x=0.0; y=0.0; z=0.0;
        for(j=0;j<6;j++) {
          x += m_msh->nod_vector[node_indeces[j]]->X();
          y += m_msh->nod_vector[node_indeces[j]]->Y();
          z += m_msh->nod_vector[node_indeces[j]]->Z();
        }
        x /= double(6);
        y /= double(6);
        z /= double(6);
        m_point.x = x;
        m_point.y = y;
        m_point.z = z;
        if(m_sfc->PointInSurface(&m_point)){
          no_mat_elements++; 
        }
      }
    }
#ifdef MFC
    if(no_mat_elements<1){
      CString m_str = "Warning: no ELE data for VOL ";
      m_str += m_vol->name.data();
      AfxMessageBox(m_str);
    }
#endif
    //--------------------------------------------------------------------
    position = vol_file.tellg();
    vol_file << "ZONE T = " << m_vol->name << ", " \
             << "N = " << no_nodes << ", " \
             << "E = " << no_mat_elements << ", " \
             << "F = FEPOINT" << ", " << "ET = BRICK" << endl;
    for(i=0;i<no_nodes;i++) {
      vol_file \
        << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << " " << vol_number << endl;
    }
    for(long i=jb;i<je;i++){
      m_ele = m_msh->ele_vector[i];
	  if(m_ele->GetElementType()==6){
        m_ele->GetNodeIndeces(node_indeces);
        x=0.0; y=0.0; z=0.0;
        for(j=0;j<6;j++) {
          x += m_msh->nod_vector[node_indeces[j]]->X();
          y += m_msh->nod_vector[node_indeces[j]]->Y();
          z += m_msh->nod_vector[node_indeces[j]]->Z();
        }
        x /= double(6);
        y /= double(6);
        z /= double(6);
        m_point.x = x;
        m_point.y = y;
        m_point.z = z;
        if(m_sfc->PointInSurface(&m_point)){
          vol_file \
            << node_indeces[0]+1 << " " << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " << node_indeces[2]+1 << " " \
            << node_indeces[3]+1 << " " << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " << node_indeces[5]+1 << endl;
        }
      }
    }
    ++p_vol;
  //======================================================================
  }
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2005 OK Implementation
11/2005 OK OO-ELE
**************************************************************************/
void MSHWriteTecplot()
{
  int ele_type  = -1;
  long no_nodes;
  long no_elements;
  string delimiter(", ");
  long i;
  CElem* m_ele = NULL;
  vec<long>node_indeces(8);
  //----------------------------------------------------------------------
  // File handling
  string file_path = "MSH";
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(m_gsp)
    file_path = m_gsp->path + "MSH";
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
    no_nodes = (long)m_msh->nod_vector.size();
    no_elements = (long)m_msh->ele_vector.size();
    // Test ele_type
    if(no_elements>0){
      m_ele = m_msh->ele_vector[0];
      ele_type = m_ele->GetElementType();
    }
    // File handling
    string msh_file_name = file_path + "_" + m_msh->pcs_name + TEC_FILE_EXTENSION;
    fstream msh_file (msh_file_name.data(),ios::trunc|ios::out);
    msh_file.setf(ios::scientific,ios::floatfield);
    msh_file.precision(12);
    if (!msh_file.good()) return;
    msh_file.seekg(0L,ios::beg);
    msh_file << "VARIABLES = X,Y,Z" << endl;
    msh_file << "ZONE T = " << m_msh->pcs_name << delimiter \
             << "N = " << no_nodes << delimiter \
             << "E = " << no_elements << delimiter;
    msh_file << "F = FEPOINT" << delimiter;
    switch(ele_type){
      //..................................................................
      case 1:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[1]+1 << " " << node_indeces[0]+1 << endl;
        }
        break;
      //..................................................................
      case 2:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 3:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << " " \
            << node_indeces[4]+1 << " " << node_indeces[5]+1 << " " \
            << node_indeces[6]+1 << " " << node_indeces[7]+1 << endl;
        }
        break;
      //..................................................................
      case 4:
        msh_file << "ET = TRIANGLE" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << endl;
        }
        break;
      //..................................................................
      case 5:
        msh_file << "ET = TETRAHEDRON" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 6:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<no_nodes;i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=0;i<no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[2]+1 << " " \
            << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " \
            << node_indeces[5]+1 << " " << node_indeces[5]+1 << endl;
        }
        break;
    }
  }
}

/**************************************************************************
GeoLib-Method: 
Task: 
Programing:
04/2005 OK Implementation
11/2005 OK OO-ELE
**************************************************************************/
void MSHLayerWriteTecplot()
{
  int ele_type  = -1;
  long no_nodes;
  long no_elements;
  string delimiter(", ");
  long i;
  CElem* m_ele = NULL;
  vec<long>node_indeces(8);
  int k;
  string no_layer_str;
  char no_layer_char[3];
  //----------------------------------------------------------------------
  // File handling
  string file_path = "MSH";
  CGSProject* m_gsp = NULL;
  m_gsp = GSPGetMember("msh");
  if(m_gsp)
    file_path = m_gsp->path;
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int j=0;j<(int)fem_msh_vector.size();j++){
    m_msh = fem_msh_vector[j];
for(k=0;k<m_msh->no_msh_layer;k++){
    sprintf(no_layer_char,"%i",k+1);
    no_layer_str = no_layer_char;
    no_nodes = (long)m_msh->nod_vector.size()/(m_msh->no_msh_layer+1);
    no_elements = (long)m_msh->ele_vector.size()/m_msh->no_msh_layer;
    // Test ele_type
    if(no_elements>0){
      m_ele = m_msh->ele_vector[0];
      ele_type = m_ele->GetElementType();
    }
    // File handling
    string msh_file_name = file_path + "MSH_LAYER" + no_layer_str + "_" + m_msh->pcs_name + TEC_FILE_EXTENSION;
    fstream msh_file (msh_file_name.data(),ios::trunc|ios::out);
    msh_file.setf(ios::scientific,ios::floatfield);
    msh_file.precision(12);
    if (!msh_file.good()) return;
    msh_file.seekg(0L,ios::beg);
    msh_file << "VARIABLES = X,Y,Z" << endl;
    msh_file << "ZONE T = " << m_msh->pcs_name << delimiter \
             << "N = " << (long)m_msh->nod_vector.size() << delimiter \
             << "E = " << no_elements << delimiter;
    msh_file << "F = FEPOINT" << delimiter;
    switch(ele_type){
      //..................................................................
      case 1:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[1]+1 << " " << node_indeces[0]+1 << endl;
        }
        break;
      //..................................................................
      case 2:
        msh_file << "ET = QUADRILATERAL" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 3:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << " " \
            << node_indeces[4]+1 << " " << node_indeces[5]+1 << " " \
            << node_indeces[6]+1 << " " << node_indeces[7]+1 << endl;
        }
        break;
      //..................................................................
      case 4:
        msh_file << "ET = TRIANGLE" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << endl;
        }
        break;
      //..................................................................
      case 5:
        msh_file << "ET = TETRAHEDRON" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[3]+1 << endl;
        }
        break;
      //..................................................................
      case 6:
        msh_file << "ET = BRICK" << endl;
        for(i=0;i<(long)m_msh->nod_vector.size();i++) {
          msh_file \
            << m_msh->nod_vector[i]->X() << " " << m_msh->nod_vector[i]->Y() << " " << m_msh->nod_vector[i]->Z() << endl;
        }
        for(i=k*no_elements;i<(k+1)*no_elements;i++) {
          m_ele = m_msh->ele_vector[i];
          m_ele->GetNodeIndeces(node_indeces);
          msh_file \
            << node_indeces[0]+1 << " " << node_indeces[1]+1 << " " \
            << node_indeces[2]+1 << " " << node_indeces[2]+1 << " " \
            << node_indeces[3]+1 << " " << node_indeces[4]+1 << " " \
            << node_indeces[5]+1 << " " << node_indeces[5]+1 << endl;
        }
        break;
    }
} // layer
  }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* MSHGet(string geo_name)
{
  CFEMesh* m_msh = NULL;
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    if(m_msh->geo_name.compare(geo_name)==0){
      return m_msh;
    }
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* MSHGet(string pcs_type_name,string geo_name)
{
  CFEMesh* m_msh = NULL;
  for(int i=0;i<(int)fem_msh_vector.size();i++){
    m_msh = fem_msh_vector[i];
    if((m_msh->pcs_name.compare(pcs_type_name)==0)&&\
       (m_msh->geo_name.compare(geo_name)==0)){
      return m_msh;
    }
  }
  return NULL;
}

/**************************************************************************
PCSLib-Method:
12/2005 OK Implementation
**************************************************************************/
CFEMesh* MSHGetGEO(string geo_name)
{
  int no_msh = (int)fem_msh_vector.size();
  // If there is only one msh file available, use it for all process. WW
  if(no_msh==1) 
    return fem_msh_vector[0]; //WW
  //----------------------------------------------------------------------
  CFEMesh* m_msh = NULL;
  for(int i=0;i<no_msh;i++){
    m_msh = fem_msh_vector[i];
    if(m_msh->geo_name.compare(geo_name)==0)
      return m_msh;
  }
  //----------------------------------------------------------------------
  return NULL;
}

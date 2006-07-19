/**************************************************************************
FEMLib - Object: Initial Conditions IC
Task: 
Programing:
08/2004 OK Implementation
12/2005 OK Restart
last modified
**************************************************************************/
#include "stdafx.h" // MFC
#include "makros.h"
#include "gs_project.h"
// C++ STL
#include <list>
#include <iostream>
using namespace std;
// FEM-Makros
#include "geo_strings.h"
#include "mathlib.h"
//#include "mshlib.h"//CC
#include "rfstring.h"
// GEOLib
// MSHLib
#include "msh_lib.h"
// FEMLib
#include "rf_ic_new.h"

#include "rf_pcs.h"
#include "nodes.h"
#include "elements.h"

//==========================================================================
vector<CInitialConditionGroup*>ic_group_vector;
vector<CInitialCondition*>ic_vector;

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
**************************************************************************/
CInitialCondition::CInitialCondition(void)
{
  string delimiter_type(" ");
  geo_type_name = "DOMAIN";
  dis_type_name = "CONSTANT";
  CNodeValue* m_node = new CNodeValue();        
  m_node->node_value = 0.0;
  SubNumber=0;
  m_pcs = NULL; //OK

  a0=b0=c0=d0=NULL; //WW
}

/**************************************************************************
FEMLib-Method: 
Task: BC deconstructor
Programing:
04/2004 OK Implementation
**************************************************************************/
CInitialCondition::~CInitialCondition(void) 
{
  node_value_vector.clear();
  //WW
  if(a0) delete a0;
  if(b0) delete b0;
  if(c0) delete c0;
  if(d0) delete d0;
  a0=b0=c0=d0=NULL; 
}
/**************************************************************************
FEMLib-Method: 
Task: IC read function
Programing:
08/2004 OK Implementation
01/2005 OK Boolean type
01/2005 OK Destruct before read
**************************************************************************/
bool ICRead(string file_base_name)
{
  //----------------------------------------------------------------------
  //ICDelete();  
  CInitialCondition *m_ic = NULL;
  char line[MAX_ZEILE];
  string sub_line;
  string line_string;
  string ic_file_name;
  ios::pos_type position;
  //========================================================================
  // File handling
  ic_file_name = file_base_name + IC_FILE_EXTENSION;
  ifstream ic_file (ic_file_name.data(),ios::in);
  if (!ic_file.good()){
    cout << "! Error in ICRead: No initial conditions !" << endl;
    return false;
  }
  ic_file.seekg(0L,ios::beg);
  //========================================================================
  // Keyword loop
  cout << "ICRead" << endl;
  while (!ic_file.eof()) {
    ic_file.getline(line,MAX_ZEILE);
    line_string = line;
    if(line_string.find("#STOP")!=string::npos)
      return true;
    //----------------------------------------------------------------------
    if(line_string.find("#INITIAL_CONDITION")!=string::npos) { // keyword found
      m_ic = new CInitialCondition();
      position = m_ic->Read(&ic_file);
      ic_vector.push_back(m_ic);
      ic_file.seekg(position,ios::beg);
    } // keyword found
  } // eof
  return true;
}

/**************************************************************************
FEMLib-Method: ICWrite
Task: master write function
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
void ICWrite(string base_file_name)
{
  CInitialCondition *m_ic = NULL;
  string sub_line;
  string line_string;
  long i;
  //========================================================================
  // File handling
  string ic_file_name = base_file_name + IC_FILE_EXTENSION;
  fstream ic_file (ic_file_name.data(),ios::trunc|ios::out);
  ic_file.setf(ios::scientific,ios::floatfield);
  ic_file.precision(12);
  if (!ic_file.good()) return;
  ic_file.seekg(0L,ios::beg);
  //========================================================================
  ic_file << "GeoSys-IC: Initial Conditions ------------------------------------------------\n";
  //========================================================================
  // IC list
  long no_ic =(long)ic_vector.size();
  for(i=0;i<no_ic;i++){
    m_ic = ic_vector[i];
    m_ic->Write(&ic_file);
  }
  ic_file << "#STOP";
  ic_file.close();
}

/**************************************************************************
FEMLib-Method: 
Task: ST read function
Programing:
08/2004 OK Implementation (DOMAIN)
10/2004 OK POLYLINE
11/2004 CMCD String streaming, Gradient
01/2005 WW Initialize by sub-domain
05/2005 OK PRIMARY_VARIABLE
12/2005 OK RESTART
07/2006 WW Read data by expression
**************************************************************************/
ios::pos_type CInitialCondition::Read(ifstream *ic_file)
{
  string line_string;
  std::stringstream in;
  ios::pos_type position;
  string dollar("$");
  string hash("#");
  bool new_subkeyword = false;
  bool new_keyword = false;
  CNodeValue *m_node = NULL;
  int i, k, ibuf;
  double d_buf;
  i=0; ibuf=0; d_buf=0.0;
  k=0;
  //========================================================================
  // Schleife ueber alle Phasen bzw. Komponenten 
  while (!new_keyword) {
    new_subkeyword = false;
    position = ic_file->tellg();
	line_string = GetLineFromFile1(ic_file);
	if(line_string.size() < 1) break;
    if(line_string.find(hash)!=string::npos) {
      new_keyword = true;
      break;
    }
    //....................................................................
    if(line_string.find("$PCS_TYPE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(ic_file));
      in >> pcs_type_name;
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$PRIMARY_VARIABLE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(ic_file));
      in >> pcs_pv_name;
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$GEO_TYPE")!=string::npos) { //subkeyword found
      in.str(GetLineFromFile1(ic_file));
      in >> geo_type_name;
      if(geo_type_name.find("POINT")!=string::npos) {
        geo_type = 0;
        in >> geo_name;
      }
      if(geo_type_name.find("POLYLINE")!=string::npos) {
        geo_type = 1;
        in >> geo_name;
      }
      if(geo_type_name.find("SURFACE")!=string::npos) {
        geo_type = 2;
      }
      if(geo_type_name.find("VOLUME")!=string::npos) {
        geo_type = 3;
      }
      if(geo_type_name.find("DOMAIN")!=string::npos) {
        geo_type = 4;
        //  Give initial condition by patches of domain. WW
        if(geo_type_name.find("SUB")!=string::npos) 
        {
           *ic_file>>SubNumber;
           if(pcs_pv_name.find("STRESS")!=string::npos)
		   {
              string str_buff;
              vector<string> tokens;
              stringstream buff;
              char *pch;
              char seps[] = "+ \n";
              char seps1[] = "*";
              double f_buff;
              a0 = new double [SubNumber];
              b0 = new double [SubNumber];
              c0 = new double [SubNumber];
              d0 = new double [SubNumber];
              for(i=0; i<SubNumber; i++) 
              {
                 a0[i] = b0[i]=c0[i]=d0[i]=0.0;
                 *ic_file>> ibuf>>str_buff>>ws;  
                 subdom_index.push_back(ibuf); 
                 pch = strtok (const_cast<char*> (str_buff.c_str()),seps);   
                 buff<<pch;
                 buff>>a0[i];
                 buff.clear();
                 while (pch != NULL)
                 { 
                    pch = strtok (NULL, seps);
                    if(pch==NULL) break;
                    string token = pch;
                    tokens.push_back(token);
                 }
                 for(k=0; k<(int)tokens.size(); k++)
                 {
                    pch = strtok (const_cast<char*> (tokens[k].c_str()),seps1);   
                    buff<<pch;
                    buff>>f_buff;
                    buff.clear();
                    pch = strtok (NULL,seps1);   
                    switch(pch[0])
                    {
                       case 'x':  b0[i]=f_buff; break;
                       case 'y':  c0[i]=f_buff; break;
                       case 'z':  d0[i]=f_buff; break;
                    }   
                 }
                 tokens.clear();
			  }
		   }
		   else
		   {
             for(i=0; i<SubNumber; i++) 
             {
                 *ic_file>>ibuf>>d_buf;
                 subdom_index.push_back(ibuf);
                 subdom_ic.push_back(d_buf);
             }
		   }
        }
      }
      in.clear();
      continue;
    }
    //....................................................................
    if(line_string.find("$DIS_TYPE")!=string::npos) { // subkeyword found
      in.str(GetLineFromFile1(ic_file));
      in >> dis_type_name;
      if(dis_type_name.find("CONSTANT")!=string::npos) {
        m_node = new CNodeValue();        
        in >> m_node->node_value;
        node_value_vector.push_back(m_node);
      }
      if(dis_type_name.find("GRADIENT")!=string::npos) {
		in >> gradient_ref_depth;  //CMCD
        in >> gradient_ref_depth_value; //CMCD
		in >> gradient_ref_depth_gradient; //CMCD
       }
      if(dis_type_name.find("RESTART")!=string::npos) { //OK
		in >> rfr_file_name;
      }
      in.clear();
      continue;
    }
  }  // Schleife ueber alle Phasen bzw. Komponenten
  return position;
}

/**************************************************************************
FEMLib-Method: 
Task: write function
Programing:
02/2004 OK Implementation
01/2005 OK Extensions
05/2005 OK PRIMARY_VARIABLE
last modification:
**************************************************************************/
void CInitialCondition::Write(fstream* ic_file)
{
  //KEYWORD
  *ic_file  << "#INITIAL_CONDITION" << endl;
  //--------------------------------------------------------------------
  //NAME+NUMBER
  *ic_file << " $PCS_TYPE" << endl;
  *ic_file << "  ";
  *ic_file << pcs_type_name << endl;
  *ic_file << " $PRIMARY_VARIABLE" << endl;
  *ic_file << "  ";
  *ic_file << pcs_pv_name << endl;
  //--------------------------------------------------------------------
  //GEO_TYPE
  *ic_file << " $GEO_TYPE" << endl;
  *ic_file << "  ";
  *ic_file << geo_type_name << delimiter_type << geo_name << endl;
  //--------------------------------------------------------------------
  //DIS_TYPE
  *ic_file << " $DIS_TYPE" << endl;
  *ic_file << "  ";
  *ic_file << dis_type_name;
  *ic_file << " ";
  CNodeValue* m_node = NULL;
  int node_value_vector_size = (int)node_value_vector.size();
  if(node_value_vector_size>0){
    m_node = node_value_vector[0];
   *ic_file << m_node->node_value;
  }
  //--------------------------------------------------------------------
  *ic_file << endl;
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
**************************************************************************/
void CInitialCondition::Set(int nidx)
{
  switch(geo_type){
    case 0: // PNT
      SetPoint(nidx);
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 1: // PLY
      SetPolyline(nidx);
      break;
    case 2: // SFC
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 3: // VOL
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 4: // DOM
	  SetDomain(nidx);
	  break; 
    }
}

/**************************************************************************
FEMLib-Method:
Task: set polyline values
Programing:
06/2006 MX Implementation
last modification:
**************************************************************************/
void CInitialCondition::SetPoint(int nidx)
{
//  long i;
  CGLPoint *m_point = NULL;
  CFEMesh* m_msh = m_pcs->m_msh;

  if((m_msh) && dis_type_name.find("CONSTANT")!=string::npos) {
    m_point = GEOGetPointByName(geo_name);//CC
    m_pcs->SetNodeValue(m_msh->GetNODOnPNT(m_point),nidx,node_value_vector[0]->node_value);
  }
  else{
      cout << "Error in CInitialCondition::SetPoint - point: " << geo_name << " not found" << endl;
  }
}

/**************************************************************************
FEMLib-Method: 
Task: 
Programing:
08/2004 OK Implementation
**************************************************************************/
void CInitialCondition::SetEle(int nidx)
{
  switch(geo_type){
    case 0: // PNT
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 1: // PLY
      // SetPolyline(nidx);
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 2: // SFC
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 3: // VOL
      cout << "Warning CInitialCondition::Set - ToDo" << endl;
      break;
    case 4: // DOM
	  SetDomainEle(nidx);
	  break; 
    }
}

/**************************************************************************
FEMLib-Method:
Task: set polyline values
Programing:
10/2004 OK Implementation
last modification:
**************************************************************************/
void CInitialCondition::SetPolyline(int nidx)
{
  long i;
  CGLPolyline *m_polyline = NULL;
  long* msh_nodes = NULL;
  long no_nodes = 0;
  if(dis_type_name.find("CONSTANT")!=string::npos) {
    m_polyline = GEOGetPLYByName(geo_name);//CC
    if(m_polyline){
      msh_nodes = MSHGetNodesClose(&no_nodes, m_polyline);//CC
      for(i=0;i<no_nodes;i++){
        SetNodeVal(msh_nodes[i],nidx,node_value_vector[0]->node_value);
      }
    }
    else{
      cout << "Error in CInitialCondition::SetPolyline - polyline: " << geo_name << " not found" << endl;
    }
  }
  if(dis_type_name.find("LINEAR")!=string::npos){
  }
}

/**************************************************************************
FEMLib-Method: 
Task:
Programing:
04/2004 OK Implementation
last modification:
**************************************************************************/
CInitialConditionGroup* GetInitialConditionGroup(string this_pcs_name)
{
  long i;
  CInitialConditionGroup *m_ic_group = NULL;
  long no_ic_groups = (long)ic_group_vector.size();
  for(i=0;i<no_ic_groups;i++){
    m_ic_group = ic_group_vector[i];
    if(m_ic_group->pcs_type_name.find(this_pcs_name)!=string::npos) 
      return m_ic_group;
  }
  return NULL;
}

/**************************************************************************
FEMLib-Method:
Task: set ST group member
Programing:
02/2004 OK Implementation
last modification:
**************************************************************************/
void CInitialConditionGroup::Set(void)
{
  long number_of_nodes;
  long *nodes = NULL;
  vector<long>nodes_vector;
  long i;

  CInitialCondition *m_ic = NULL;
  CNodeValue *m_node_value = NULL;

  long no_ics =(long)ic_vector.size();
  for(i=0;i<no_ics;i++){
    m_ic = ic_vector[i];
    //====================================================================
    if((m_ic->pcs_type_name.compare(pcs_type_name)==0)&&(m_ic->pcs_pv_name.compare(pcs_pv_name)==0)){
      //------------------------------------------------------------------
      if(m_ic->geo_type_name.compare("POINT")==0) {
        m_node_value = new CNodeValue();
//OK_IC        m_node_value->geo_node_number = m_ic->node_vector[0].geo_node_number;
        CGLPoint* m_point = NULL;
       m_point = GEOGetPointByName(m_ic->geo_name);//CC
        if(m_point)
          m_node_value->msh_node_number = GetNodeNumberClose(m_point->x,m_point->y,m_point->z);
        else
          m_node_value->msh_node_number = -1;
//OK_IC        m_node_value->node_value = m_ic->node_vector[0].geo_node_value;
        group_vector.push_back(m_node_value);
      }
      //------------------------------------------------------------------
      if(m_ic->geo_type_name.compare("POLYLINE")==0) {
        CGLPolyline *m_polyline = NULL;
        m_polyline = GEOGetPLYByName(m_ic->geo_name);//CC
       if(m_polyline) {
         m_polyline->type = 3;
        //................................................................
        if(m_ic->dis_type_name.compare("CONSTANT")==0){
          nodes = MSHGetNodesClose(&number_of_nodes, m_polyline);//CC
          for(i=0;i<number_of_nodes;i++){
            m_node_value = new CNodeValue();
            m_node_value->msh_node_number = -1;
            m_node_value->msh_node_number = nodes[i];
            m_node_value->geo_node_number = nodes[i];
//OK_IC            m_node_value->node_value = m_ic->node_vector[0].geo_node_value / (double)number_of_nodes;  //dis_prop[0];
            group_vector.push_back(m_node_value);
          }
        }
        if(m_ic->dis_type_name.compare("POINTS")==0){
          long node_vector_length = (long)m_ic->node_value_vector.size();
          for(i=0;i<node_vector_length;i++) {
            m_node_value = new CNodeValue();
            m_node_value = m_ic->node_value_vector[i];
//OK_IC            m_node_value->msh_node_number = m_ic->node_vector[i].msh_node_number;
            //m_node_value->geo_node_number = m_bc->geo_node_number;
//OK_IC            m_node_value->node_value = m_ic->node_vector[i].node_value;
            group_vector.push_back(m_node_value);
          }
        }
        //delete(values);
        Free(nodes);
       }
      }
      //------------------------------------------------------------------
      if(m_ic->geo_type_name.compare("SURFACE")==0) {
        Surface *m_surface = NULL;
        m_surface = GEOGetSFCByName(m_ic->geo_name);//CC
        if(m_surface) {
          nodes_vector = GetMSHNodesClose(m_surface);//CC
          //nodes = m_surface->GetPointsIn(&number_of_nodes);
          long nodes_vector_length = (long)nodes_vector.size();
          for(i=0;i<nodes_vector_length;i++){
            m_node_value = new CNodeValue();
            m_node_value->msh_node_number = -1;
            m_node_value->msh_node_number = nodes_vector[i]; //nodes[i];
            m_node_value->geo_node_number = nodes_vector[i]; //nodes[i];
//OK_IC            m_node_value->node_value = m_ic->node_vector[i].geo_node_value;  //dis_prop[0];
            group_vector.push_back(m_node_value);
          }
        }
      }
    //====================================================================
    }
  }
}
/**************************************************************************
FEMLib-Method:
Task: set domain values
Programing:
02/2004 OK Implementation
10/2004 CMCD Gradient
02/2005 MB Water Depth // ToDo
03/2005 WW By patches of domain //ToDo
06/2005 OK OVERLAND_FLOW
12/2005 OK RESTART
**************************************************************************/
void CInitialCondition::SetDomain(int nidx)
{
    int k, onZ;
	long i;
	double node_val, node_depth;
    vector<long>nodes_vector;
    CFEMesh* m_msh = m_pcs->m_msh;
    onZ = 0;
    if(m_msh) //OK
      onZ = m_msh->GetCoordinateFlag()%10;
    node_depth = 0.0;
    k=0;
    if(SubNumber==0) 
    {
      //----------------------------------------------------------------------
      if(dis_type_name.find("CONSTANT")!=string::npos) 
      {
        //....................................................................
        if(m_pcs->pcs_type_name.compare("OVERLAND_FLOW")==0) 
        {
            if(m_pcs->m_msh)
	        {
	           for(i=0;i<m_pcs->m_msh->GetNodesNumber(false);i++){ //OK MSH
	              node_val = node_value_vector[0]->node_value + m_pcs->m_msh->nod_vector[i]->Z();
  	              m_pcs->SetNodeValue(i,nidx,node_val);
                }
            }
            else{
              for(i=0;i<NodeListLength;i++){
                node_val = node_value_vector[0]->node_value + GetNodeZ(i);
                SetNodeVal(i,nidx,node_val);        
              }
            }
         }    
         else
         {
           //................................................................
           node_val = node_value_vector[0]->node_value;
           if(m_msh){
              for(i=0;i<m_msh->GetNodesNumber(true);i++) //OK MSH
                 m_pcs->SetNodeValue(i,nidx,node_val);
           }
           //................................................................
           else{
              for(i=0;i<NodeListLength;i++)
                 SetNodeVal(i,nidx,node_val);
           }
         }
      }
      //--------------------------------------------------------------------
      if(dis_type_name.find("GRADIENT")!=string::npos)
	  {
          if(m_msh)
          {
             for(i=0;i<m_msh->GetNodesNumber(true);i++) //WW
 	         {
                if(onZ==1) //2D 
                 	node_depth = m_msh->nod_vector[i]->Y();
                if(onZ==2) //3D
                    node_depth = m_msh->nod_vector[i]->Z();
	            node_val = ((gradient_ref_depth_gradient)*(gradient_ref_depth-node_depth))+
                        gradient_ref_depth_value;
				m_pcs->SetNodeValue(m_msh->nod_vector[i]->GetIndex(),nidx,node_val);
             }
		  }
		  else
          {
             for(i=0;i<NodeListLength;i++)
	         {
                if(max_dim==1) //2D 
                     node_depth =  GetNodeY(i);
                if(max_dim==2) //3D
		        node_depth = GetNodeZ(i);
	            node_val = ((gradient_ref_depth_gradient)*(gradient_ref_depth-node_depth))+
                        gradient_ref_depth_value;
             	SetNodeVal(i,nidx,node_val);
             }
		  }// If else if(m_pcs->m_msh)
	   } //if(dis_type_name.find("GRADIENT")!=string::npos)
  //----------------------------------------------------------------------
  if(dis_type_name.compare("RESTART")==0)
  {
    char line[MAX_ZEILEN];
    int no_var;
    int* var_n;
    vector<string>var_name;
    string var_name_string;
    string sdummy;
    double ddummy, dddummy;
    long ldummy;
    //....................................................................
    // File handling  
    if(rfr_file_name.size()==0){
      cout << "Warning in CInitialCondition::SetDomain - no RFR file" << endl;
      return;
    }
    ifstream rfr_file;
    CGSProject* m_gsp = GSPGetMember("msh");
    string restart_file_name;
    restart_file_name = rfr_file_name;
    if(m_gsp)
      restart_file_name = m_gsp->path + rfr_file_name;
    rfr_file.open(restart_file_name.c_str(),ios::in);
    if(!rfr_file.good()){
      cout << "Warning in CInitialCondition::SetDomain - no RFR file" << endl;
      return;
    }
    rfr_file.seekg(0L,ios::beg); // rewind?
    rfr_file.getline(line,MAX_ZEILEN); //#0#0#0#1#100000#0#4.2.13 ###########################################
    rfr_file.getline(line,MAX_ZEILEN); //1 1 4
    rfr_file >> no_var; //2 1 1
    var_n = new int[no_var];
    for(i=0;i<no_var;i++){
      rfr_file >> var_n[i];
    }
    size_t pos;
    for(i=0;i<no_var;i++){ // HEAD, m ...
      rfr_file >> var_name_string >> sdummy;
      pos = var_name_string.find_first_of(',');
      var_name_string.erase(pos,1);
      var_name.push_back(var_name_string);
    }
    while(!rfr_file.eof()) { //TODO Implementation/Manual Correction of Keyword #STOP: TK
      rfr_file >> dddummy;
        ldummy  = (long)dddummy;
        cout << ldummy << endl;
      for(i=0;i<no_var;i++){ // HEAD, m ...
         rfr_file >> ddummy;
          m_pcs->SetNodeValue(ldummy,nidx,ddummy);      
      }
    }
    //....................................................................
    delete var_n;
    var_name.clear();
  }
  //----------------------------------------------------------------------
}
//========================================================================
    else //WW
    {
       bool quadratic = false;
       /// In case of P_U coupling monolithic scheme
       if(m_pcs->type==41) //WW Mono
       {
         if(pcs_pv_name.find("DISPLACEMENT")!=string::npos) //Deform 
            quadratic = true; 
         else quadratic = false;  
       }
       else if(m_pcs->type==4) quadratic = true; 
       else quadratic = false; 
    if (m_msh){   
       for(k=0; k<SubNumber; k++)
       {
          GEOGetNodesInMaterialDomain(m_msh, subdom_index[k], nodes_vector, quadratic);
          if(dis_type_name.find("GRADIENT")!=string::npos)
          {
             if(k==0) //TEST for DECOVALEX
             {
                 for(i=0;i<(int)nodes_vector.size();i++)
                   m_pcs->SetNodeValue(nodes_vector[i],nidx, subdom_ic[k]);
             }
             else
             for(i=0;i<(int)nodes_vector.size();i++)
	         {
                if(max_dim==1) //2D 
                    node_depth =  m_msh->nod_vector[nodes_vector[i]]->Y();
                if(max_dim==2) //3D
		            node_depth =  m_msh->nod_vector[nodes_vector[i]]->Z();
	            node_val = ((gradient_ref_depth_gradient)*(gradient_ref_depth-node_depth))+
                        gradient_ref_depth_value;
                m_pcs->SetNodeValue(nodes_vector[i],nidx,node_val);
             }
             
           }
           else
           {
              for(i=0;i<(int)nodes_vector.size();i++)
                 m_pcs->SetNodeValue(nodes_vector[i],nidx, subdom_ic[k]);
           }
        }
        }
    else {
       for(k=0; k<SubNumber; k++)
       {
           GEOGetNodesInMaterialDomain(subdom_index[k], nodes_vector);
          if(dis_type_name.find("GRADIENT")!=string::npos)
          {
             if(k==0) //TEST for DECOVALEX
             {
                for(i=0;i<(int)nodes_vector.size();i++)
                   SetNodeVal(nodes_vector[i],nidx, subdom_ic[k]);
             }
             else
             for(i=0;i<(int)nodes_vector.size();i++)
	         {
                if(max_dim==1) //2D 
                    node_depth = GetNodeY(nodes_vector[i]);
                if(max_dim==2) //3D
		            node_depth = GetNodeZ(nodes_vector[i]);
	            node_val = ((gradient_ref_depth_gradient)*(gradient_ref_depth-node_depth))+
                        gradient_ref_depth_value;
                SetNodeVal(nodes_vector[i],nidx,node_val);
             }
             
           }
           else
           {
              for(i=0;i<(int)nodes_vector.size();i++)
                 SetNodeVal(nodes_vector[i],nidx, subdom_ic[k]);
           }

/*
           if(dis_type_name.find("CONSTANT")!=string::npos) {
              for(i=0;i<(int)nodes_vector.size();i++)
		      {
		          SetNodeVal(nodes_vector[i],nidx,node_val);
              }
           }
          if(dis_type_name.find("GRADIENT")!=string::npos)
             for(i=0;i<(int)nodes_vector.size();i++)
	         {
		        node_depth = GetNodeZ(nodes_vector[i]);
	            node_val = ((gradient_ref_depth_gradient)*(gradient_ref_depth-node_depth))+
                        gradient_ref_depth_value;
                SetNodeVal(nodes_vector[i],nidx,node_val);
             }
*/
        }
      }
    }
}
/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void ICDelete()
{
  long i;
  int no_ic =(int)ic_vector.size();
  for(i=0;i<no_ic;i++){
    delete ic_vector[i];
  }
  ic_vector.clear();
}

/**************************************************************************
FEMLib-Method:
Task:
Programing:
01/2005 OK Implementation
last modified:
**************************************************************************/
void ICGroupDelete()
{
  long i;
  int no_ic_group =(int)ic_group_vector.size();
  for(i=0;i<no_ic_group;i++){
    delete ic_group_vector[i];
  }
  ic_group_vector.clear();
}

/**************************************************************************
FEMLib-Method:
Task: set domain values to elements
Programing:
06/2006 MX Implementation
**************************************************************************/
void CInitialCondition::SetDomainEle(int nidx)
{
    int k;
	long i;
	double ele_val=0.0;
    vector<long>ele_vector;
    CFEMesh* m_msh = m_pcs->m_msh;
    k=0;
    CElem* m_ele = NULL;

    if(SubNumber==0){  //only for constant values
      ele_val = node_value_vector[0]->node_value;
      for(i=0;i<(long)m_msh->ele_vector.size();i++)
      {
          m_ele = m_msh->ele_vector[i];
          if(m_ele->GetMark()) // Marked for use
          {
            m_pcs->SetElementValue(i,nidx, ele_val);
          } 
       }
    }
//========================================================================
    else //MX
    {
       if (m_msh){   
         for(k=0; k<SubNumber; k++)
         {
           for(i=0;i<(long)m_msh->ele_vector.size();i++)
           {
             m_ele = m_msh->ele_vector[i];
             if(m_ele->GetMark() && m_ele->GetPatchIndex() == subdom_index[k] ) // Marked for use
             {
               m_pcs->SetElementValue(i, nidx, subdom_ic[k]);
               ele_val = m_pcs->GetElementValue(i, nidx);
             } 
           }
         }
       }  //if
    }
}

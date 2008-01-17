/**************************************************************************
MSHLib - Object: 
Task: 
Programing:
08/2005 WW/OK Encapsulated from mshlib
**************************************************************************/
#include "stdafx.h" // MFC
// C++
#include <iomanip>
#include <string>
#include <vector>
using namespace std;
// MSHLib
#include "msh_elements_rfi.h"
#include "msh_node.h"
// PCSLib
#ifdef RFW_FRACTURE
#include "rf_pcs.h" //RFW 06/2006
#endif
//========================================================================
namespace Mesh_Group
{
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
CNode::CNode(const int Index, const double x, 
            const double y, const double z):CCore(Index)
{
  
   eqs_index = -1; 
   coordinate[0] = x;
   coordinate[1] =y;
   coordinate[2] =z; 
   free_surface=-1; 
   interior_test=0;
   selected=0;
   epsilon=0.0;
   // PCH
   crossroad = 0;	// Set to be no crossroad
   patch_area = -1.; //OK4310
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CNode::operator = (const CNode& n)
{
   boundary_type = n.boundary_type; 
   index = n.index;
   mark = n.mark;
   eqs_index = n.eqs_index;
   coordinate[0] = n.coordinate[0]; 
   coordinate[1] = n.coordinate[1]; 
   coordinate[2] = n.coordinate[2]; 
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
bool CNode::operator == (const CNode& n)
{
   if(index == n.index)
      return true;
   else 
	  return false;
}
/**************************************************************************
MSHLib-Method: 
06/2005 WW Implementation
03/2006 OK patch_area
**************************************************************************/
void CNode::Write(ostream& osm) const
{
  osm.setf(ios::scientific, ios::floatfield);
  string deli(" ");
  setw(14);
  osm.precision(14);
  osm<<index<<deli
     <<coordinate[0]<<deli
     <<coordinate[1]<<deli
     <<coordinate[2]<<deli; 
  if(patch_area>0.0)
  {
    osm<<"$AREA"<<deli<<patch_area;
  }
  osm<<endl;
}
/**************************************************************************
MSHLib-Method: 
Task:
Programing:
06/2005 WW Implementation
**************************************************************************/
void CNode::SetCoordinates(const double* argCoord)
{
    coordinate[0] = argCoord[0];
  	coordinate[1] = argCoord[1];
  	coordinate[2] = argCoord[2];
}

#ifdef RFW_FRACTURE
/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::X_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double x_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_X1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_X1")+1;
	}
	x_displaced = X() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return x_displaced;
}

/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::Y_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double y_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_Y1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_Y1")+1; 
	}
	y_displaced = Y() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return y_displaced;
}

/**************************************************************************
MSHLib-Method: 
Task: Returns displaced coordinates
Programing:
05/2006 RFW Implementation
**************************************************************************/
double CNode::Z_displaced()
{
	int nidx_dm=0;
	CRFProcess* m_pcs = NULL;
	double z_displaced=0;
	if(M_Process||MH_Process)
	{
		m_pcs = PCSGet("DISPLACEMENT_Z1",true);
		nidx_dm = m_pcs->GetNodeValueIndex("DISPLACEMENT_Z1")+1;
	}
	z_displaced = Z() + m_pcs->GetNodeValue(GetIndex(), nidx_dm);
	return z_displaced;
}
#endif

} // namespace Mesh_Group
//========================================================================

//////////////////////////////////////////////////////////////////////////
// RFI methods
//////////////////////////////////////////////////////////////////////////

/**************************************************************************
MshLib-Method: RemoveNode
Task: 
Programing:
05/2004 TK Implementation
**************************************************************************/
void RemoveNode(long nnr, string projectname)
{
	int i=0, j=0, k=0;
	long number_of_all_elements, number_of_nodes_to_remesh,number_of_all_nodes ;
	long id, nnb;    
	long treffer=0;
	long nnr1=0, nnr2=0, nnr3=0, next_nnr1=0, next_nnr2=0,start_nnr=0;

	vector<CMSHNodes*> nodes_vector;
	vector<CMSHElements*> element_vector;
    CMSHElements *m_elements = NULL;
	vector<CMSHNodes*> remesh_nodes_vector;
	CMSHNodes *m_remesh_nodes = NULL;
	nodes_vector = MSHGetNodeVector();
	element_vector = MSHGetElementVector();
	number_of_all_nodes	   = (long)nodes_vector.size();
	number_of_all_elements = (long)element_vector.size();
	remesh_nodes_vector.clear();

//Search all Nodes and Elements connected to the Points 
	for (i=0;i<number_of_all_elements;i++)
	{
		if( nnr == element_vector[i]->node1 || 
			nnr == element_vector[i]->node2 ||
			nnr == element_vector[i]->node3  )
		{			
			id = element_vector[i]->element_id;
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
			
			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  nnb = element_vector[i]->node1;
			  if (element_vector[i]->node1==remesh_nodes_vector[j]->nodenumber &&
				  element_vector[i]->node1 != nnr ) treffer++;
			}
			if (treffer==0 && nnr != element_vector[i]->node1) 
			{
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node1;
				m_remesh_nodes->x =  element_vector[i]->x1;
				m_remesh_nodes->y =  element_vector[i]->y1;
				m_remesh_nodes->z =  element_vector[i]->z1;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();

			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  nnb = element_vector[i]->node2;
			  if (element_vector[i]->node2==remesh_nodes_vector[j]->nodenumber &&
				  element_vector[i]->node2 != nnr ) treffer++;
			}
			if (treffer==0 && nnr != element_vector[i]->node2) 
			{
				nnb = element_vector[i]->node2;
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node2;
				m_remesh_nodes->x =  element_vector[i]->x2;
				m_remesh_nodes->y =  element_vector[i]->y2;
				m_remesh_nodes->z =  element_vector[i]->z2;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
			number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();

			for (j=0;j<number_of_nodes_to_remesh;j++)
			{
			  nnb = element_vector[i]->node3;
			  nnb = remesh_nodes_vector[j]->nodenumber;
			  if (element_vector[i]->node3==remesh_nodes_vector[j]->nodenumber &&
				  element_vector[i]->node3 != nnr ) treffer++;
			}
			if (treffer==0 && nnr != element_vector[i]->node3) 
			{
				m_remesh_nodes = new CMSHNodes;
				m_remesh_nodes->nodenumber = element_vector[i]->node3;
				m_remesh_nodes->x =  element_vector[i]->x3;
				m_remesh_nodes->y =  element_vector[i]->y3;
				m_remesh_nodes->z =  element_vector[i]->z3;
				remesh_nodes_vector.push_back(m_remesh_nodes);
				treffer=0;
			}
			else treffer=0;
		}

	}
	
	//sortieren der Reihenfolge
	treffer=0;
	number_of_nodes_to_remesh = (long)remesh_nodes_vector.size();
	for (j=0;j<number_of_nodes_to_remesh;j++)
	{
	   nnb = remesh_nodes_vector[j]->nodenumber;
	   if (j==0)
	   {
		for (i=0;i<number_of_all_elements;i++)
		{
			nnr1 = element_vector[i]->node1;
			nnr2 = element_vector[i]->node2; 
			nnr3 = element_vector[i]->node3;		

			if ((nnr == nnr1 || nnr == nnr2 || nnr == nnr3) &&
				(nnb == nnr1 || nnb == nnr2 || nnb == nnr3) )
			{
				if (treffer == 0 ){
				if((nnr == nnr1 && nnb == nnr2) || (nnr == nnr2 && nnb == nnr1)) next_nnr1 = nnr3;
				if((nnr == nnr2 && nnb == nnr3) || (nnr == nnr3 && nnb == nnr2)) next_nnr1 = nnr1;
				if((nnr == nnr1 && nnb == nnr3) || (nnr == nnr3 && nnb == nnr1)) next_nnr1 = nnr2;
				remesh_nodes_vector[j]->serialized_rfi_node_number = 0;
                start_nnr = nnb;
				next_nnr2 = nnb;
				treffer=1;
				}
				else treffer=0;
			}
		}
	   }

	   if (j>0)
	   {
		for (i=0;i<number_of_all_elements;i++)
		{
			nnr1 = element_vector[i]->node1;
			nnr2 = element_vector[i]->node2; 
			nnr3 = element_vector[i]->node3;		

			if ((nnr == nnr1 || nnr == nnr2 || nnr == nnr3) &&
				(next_nnr1 == nnr1 || next_nnr1 == nnr2 || next_nnr1 == nnr3) &&
				(next_nnr2 != nnr1 || next_nnr2 != nnr2 || next_nnr2 != nnr3))
			{
				next_nnr2=next_nnr1;
					if(((nnr == nnr1 && next_nnr1 == nnr2) || (nnr == nnr2 && next_nnr1 == nnr1))&& treffer==0){
						next_nnr1 = nnr3;
						treffer=1;
					}
					if(((nnr == nnr2 && next_nnr1 == nnr3) || (nnr == nnr3 && next_nnr1 == nnr2))&& treffer==0) {
						next_nnr1 = nnr1;
						treffer=1;
					}
					if(((nnr == nnr1 && next_nnr1 == nnr3) || (nnr == nnr3 && next_nnr1 == nnr1))&& treffer==0) {
						next_nnr1 = nnr2;
						treffer=1;
					}
				
				k++;
				remesh_nodes_vector[j]->serialized_rfi_node_number = k;
				if (treffer==1)break;
			}
			else treffer=0;
		}
	   }

		remesh_nodes_vector[j]->serialized_rfi_node_number;
	}



// Write GeoFile
	string geo_filename = projectname + ".geo";
	const char *file = 0;
	file = geo_filename.data(); 
	FILE *geo_file;
	geo_file = fopen(file, "w+t");   
    /*POINTS*/ 
	for (j=0;j<number_of_nodes_to_remesh;j++)
	{     
		fprintf(geo_file,"%s","Point(");
		fprintf(geo_file,"%ld",remesh_nodes_vector[j]->nodenumber);
		fprintf(geo_file,"%s",") = {");
		fprintf(geo_file,"%f",remesh_nodes_vector[j]->x);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%f",remesh_nodes_vector[j]->y);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%f",remesh_nodes_vector[j]->z);
		fprintf(geo_file,"%s",", ");
		fprintf(geo_file,"%s","1.0");
		fprintf(geo_file,"%s\n","};");

		next_nnr1 = remesh_nodes_vector[j]->serialized_rfi_node_number;
		next_nnr2 = remesh_nodes_vector[j]->nodenumber;
	}
    /*LINES*/ 
	for (j=0;j<number_of_nodes_to_remesh;j++)
	{   
		if(remesh_nodes_vector[j]->serialized_rfi_node_number == j)
		{
		   if(j<number_of_nodes_to_remesh-1)
		   {
			fprintf(geo_file,"%s","Line(");
			fprintf(geo_file,"%i",j+1);
			fprintf(geo_file,"%s",") = {");
			fprintf(geo_file,"%ld",remesh_nodes_vector[j]->nodenumber);
			fprintf(geo_file,"%s",", ");
			for (k=0;k<number_of_nodes_to_remesh;k++)
			{   
			  if(remesh_nodes_vector[k]->serialized_rfi_node_number == j+1)
			  {
    		  fprintf(geo_file,"%ld",remesh_nodes_vector[k]->nodenumber);
			  fprintf(geo_file,"%s\n","};");
			  }
			}
		   }
		   if(j==number_of_nodes_to_remesh-1)
		   {
			fprintf(geo_file,"%s","Line(");
			fprintf(geo_file,"%i",j+1);
			fprintf(geo_file,"%s",") = {");
			fprintf(geo_file,"%ld",remesh_nodes_vector[j]->nodenumber);
			fprintf(geo_file,"%s",", ");
			for (k=0;k<number_of_nodes_to_remesh;k++)
			{   
			  if(remesh_nodes_vector[k]->serialized_rfi_node_number == 0)
			  {
    		  fprintf(geo_file,"%ld",remesh_nodes_vector[k]->nodenumber);
			  fprintf(geo_file,"%s\n","};");
			  }
			}
		   }
		}	
	}
    /*POLYLINES*/ 
	fprintf(geo_file,"%s","Line Loop(1");
	fprintf(geo_file,"%s",") = {");	
	for (j=0;j<number_of_nodes_to_remesh;j++)
	{	
	   fprintf(geo_file,"%i",j+1);
	   if (j<number_of_nodes_to_remesh-1) fprintf(geo_file,"%s",", ");
	}
	fprintf(geo_file,"%s\n","};");
    /*SURFACES*/ 
	fprintf(geo_file,"%s","Plane Surface(1");
	fprintf(geo_file,"%s",") = {1");
	fprintf(geo_file,"%s\n","};");
	/*Datei Schlie?n*/ 
	fclose(geo_file);

// Run MeshGeneration
  string m_strExecuteGEO = "..\\LIB\\gmsh " + geo_filename +" -2";
  const char *m_strExecute=0;
  m_strExecute = m_strExecuteGEO.data();
  system(m_strExecute);
  
  string rfi_temp_filename = projectname + "_temp.rfi";
  const char *file_name_const_char = 0;
  FILE *rfi_file2=NULL;
  file_name_const_char = rfi_temp_filename.data();
  rfi_file2 = fopen(file_name_const_char, "w+t"); 
  char file_name__char [1024];

  string msh_filename = projectname + ".msh";
  file_name_const_char = msh_filename.data();
  strcpy(file_name__char,file_name_const_char);
 
 //OK TEST  MSH2RFI (file_name__char, rfi_file2);
  
// Read Temp File und Schreibe RFI
  char text[1024];
char tetrakey [5];
long ldummy  = 0l;
long material_group  = 0l;
long knotendummy1,knotendummy2,knotendummy3;
     knotendummy1=knotendummy2=knotendummy3=0l;
 i=0;
 while (!feof(rfi_file2)) 
 {
   fgets(text,1024,rfi_file2);

      if (6 <= sscanf(text,"%ld %ld %ld %s %ld %ld %ld", &id,&material_group, &ldummy, tetrakey, &knotendummy1,&knotendummy2,&knotendummy3))
	  {
			number_of_all_elements = (long)element_vector.size();
			m_elements = new CMSHElements;
			m_elements->element_id = number_of_all_elements;
			m_elements->elementtype= 4;
			m_elements->materialnumber= -1;
			m_elements->node1 = knotendummy1;
			m_elements->node2 = knotendummy2;
			m_elements->node3 = knotendummy3;
			element_vector.push_back(m_elements);
			i++;
	  }
	  if( i>0 &&
		  0 == sscanf(text,"%ld %ld %ld %s %ld %ld %ld", &id,&material_group, &ldummy, tetrakey, &knotendummy1,&knotendummy2,&knotendummy3))
         break;
 }
  
  //l?chen der Elemente
	for (i=0;i<number_of_all_elements;i++)
	{
		if(	nnr == element_vector[i]->node1 || 
			nnr == element_vector[i]->node2 ||
			nnr == element_vector[i]->node3  )
		{
			id = element_vector[i]->element_id;
			element_vector.erase(element_vector.begin()+i);
			number_of_all_elements = (long)element_vector.size();
			i--;
		} 
	}

	//l?chen des Punktes
	for (i=0;i<number_of_all_nodes;i++)
	{
		id = nodes_vector[i]->nodenumber;
        if (nnr == id)
		{		
			nodes_vector.erase(nodes_vector.begin()+i);
			number_of_all_nodes = (long)nodes_vector.size();
			i--;
		} 
	}

   
	//RFI neu schreiben
	string filename = projectname + ".rfi";
	static char kopf[81];
	long output_nr=0,output_geom=0;
	char rf_version_string[16];	
    long nb_prismnodes;
    long nb_elements;
	file = filename.data(); 
	FILE *dat_out_rfi;
	dat_out_rfi = fopen(file, "w+t");   

	/*Lesen der Versionsnummer und loeschen der Punkte aus dem Versions-String */ 
		strcpy(rf_version_string,ROCKFLOW_VERSION);
		j = 0;
		for (i=0;((i<(int)(int)strlen(rf_version_string))&&(rf_version_string[i]));i++) 
		{
			if (rf_version_string[i]!='.')
				rf_version_string[i-j] = rf_version_string[i];
				else
				j++;
		}   
		rf_version_string[i-j] = 0;

	/*Schreibe Kopf*/ 
		for (i=0;i<80;i++)
			kopf[i] = ' ';
		kopf[80] = '\0';
		sprintf(kopf," 0 0 %ld %ld %#g 0 %s",output_nr,output_geom,0.0,rf_version_string);
		for (i=0;i<80;i++)
			if ((kopf[i]==' ') || (kopf[i]=='\0'))
				kopf[i] = '#';
		if (fprintf(dat_out_rfi,"%s\n",kopf)!=81) {
			DisplayErrorMsg("Fehler beim Schreiben der RF-Ausgabedatei (Kopf) !!");
		}

	/*Schreibe 2.Zeile*/ 
		nb_prismnodes = (long)nodes_vector.size();
		nb_elements = (long)element_vector.size();
		fprintf(dat_out_rfi,"0 ");
		fprintf(dat_out_rfi,"%ld ",nb_prismnodes);
		fprintf(dat_out_rfi,"%ld \n",nb_elements);

	/*Schreibe Geometrie*/ 
		for (i=0;i<nb_prismnodes;i++)
		{
		fprintf(dat_out_rfi,"%ld ",nodes_vector[i]->nodenumber);
		fprintf(dat_out_rfi,"%20.14f ",nodes_vector[i]->x);
		fprintf(dat_out_rfi,"%20.14f ",nodes_vector[i]->y);
		fprintf(dat_out_rfi,"%20.14f \n",nodes_vector[i]->z);     
		}

	/*Schreibe Topologie*/ 
		for (i=0;i<nb_elements;i++)
		{
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->element_id);
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->materialnumber);
			fprintf(dat_out_rfi,"-1 ");

			fprintf(dat_out_rfi,"tri ");
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->node1);
			fprintf(dat_out_rfi,"%ld ",element_vector[i]->node2);
			fprintf(dat_out_rfi,"%ld \n",element_vector[i]->node3);
		}

	/*Datei Schlie?n*/ 
	fclose(dat_out_rfi);
	fclose(rfi_file2);

   	/*Vektoren leeren*/ 
    msh_nodes_vector.clear();
    msh_elements_vector.clear();
	remesh_nodes_vector.clear();
    nodes_vector.clear();
    element_vector.clear();
	
	/*Serialisierung der Daten*/ 
	Serialize_RFI (filename);
}


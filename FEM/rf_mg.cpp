/**************************************************************************/
/* ROCKFLOW - Modul: rf_mg.c
                                                                          */
/* Aufgabe:
   - Datenstrukturen und Definitionen fuer den Datentyp MESH_GENERATION
                                                                          */
/* Programmaenderungen:
   02/2003    MK      Erste Version
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "mathlib.h"
#include "geo_strings.h"
#include "files.h"
#include "rf_mg.h"


static LIST_MESH_GENERATION *list_of_mesh_generations = NULL;





/**************************************************************************/
/* ROCKFLOW - Funktion: create_mesh_generation
                                                                          */
/* Aufgabe:
   Konstruktor for MESH_GENERATION
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des SOSI's.
                                                                          */
/* Ergebnis:
   - Adresse des MG's im Erfolgsfall, ansonsten NULL-Zeiger -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
MESH_GENERATION *create_mesh_generation(char *name)
{
  MESH_GENERATION *mg;

  mg  = (MESH_GENERATION *) Malloc(sizeof(MESH_GENERATION));
  if ( mg == NULL ) return NULL;


  mg->name = (char *) Malloc((int)strlen(name)+1);
  if ( mg->name == NULL ) {
         Free(mg );
         return NULL;
  }
  strcpy(mg->name,name);

  mg->file_name = NULL;
  return mg ;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_mesh_generation
                                                                          */
/* Aufgabe:
   Destructor for MESH_GENERATION
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E void *member: Zeiger auf die Adresse des SOSI's (hier vom Typ void).
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void destroy_mesh_generation(void *member)
{
  MESH_GENERATION *mg=(MESH_GENERATION *)member;

  if ( mg->name ) mg->name=(char *)Free(mg->name);
  if ( mg->file_name ) mg->file_name=(char *)Free(mg->file_name);
  if ( mg ) mg =(MESH_GENERATION *)Free(mg );

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_mg_type
                                                                          */
/* Aufgabe:
   Setzt Typ des MG's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur mg.
   E long Typ                                                                            */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void set_mg_type(MESH_GENERATION *mg, long type)
{
  if (mg) mg->type=type;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_mg_type
                                                                          */
/* Aufgabe:
   Liefert Typ des MG's.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur mg.
                                                                          */
/* Ergebnis:
   - Typ -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
long get_mg_type(MESH_GENERATION *mg)
{
        return (mg)?mg->type:-1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: set_mg_file_name
                                                                          */
/* Aufgabe:
   Setzt Dateinamen des MG's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur mg.
   E char *file_name: Zeiger auf den Dateinamen des MG's.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void set_mg_file_name(MESH_GENERATION *mg, char *file_name)
{
  if (!file_name) return;
  if(mg) {
    mg->file_name=(char *)Free(mg->file_name);
    mg->file_name = (char *) Malloc((int)strlen(file_name)+1);
    if ( mg->file_name == NULL ) return;
    strcpy(mg->file_name, file_name);
    return;
  }
  else return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: get_mg_file_name
                                                                          */
/* Aufgabe:
   Liefert Dateinamen des MG's
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur mg.
                                                                          */
/* Ergebnis:
   - Name des MG's -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
char *get_mg_file_name(MESH_GENERATION *mg)
{
  if (mg->file_name)
    return mg->file_name;
  else
    return NULL;
}


/* =======================================================================*/
/* Verwaltung der Liste
                                                                          */
/* =======================================================================*/


/**************************************************************************/
/* ROCKFLOW - Funktion: create_mesh_generation_list
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Anfangszustaende;
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Listenname
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
LIST_MESH_GENERATION *create_mesh_generation_list (char *name)
{
  LIST_MESH_GENERATION *list_of_mesh_generations;

  list_of_mesh_generations = (LIST_MESH_GENERATION *) Malloc(sizeof(LIST_MESH_GENERATION));
  if ( list_of_mesh_generations == NULL ) return NULL;


  list_of_mesh_generations->name = (char *) Malloc((int)strlen(name)+1);
  if ( list_of_mesh_generations->name == NULL ) {
         Free(list_of_mesh_generations);
         return NULL;
  }
  strcpy(list_of_mesh_generations->name,name);

  list_of_mesh_generations->mg_list=create_list();
  if ( list_of_mesh_generations->mg_list == NULL ) {
    Free(list_of_mesh_generations->name);
        Free(list_of_mesh_generations);
        return NULL;
  }

  list_of_mesh_generations->names_of_mesh_generations=NULL;
  list_of_mesh_generations->count_of_mesh_generations_name=0;

  return list_of_mesh_generations;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: destroy_mesh_generation_list
                                                                          */
/* Aufgabe:
   Entfernt komplette Liste der Anfangszustaende aus dem Speicher
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
void destroy_mesh_generation_list(void)
{
  if (list_of_mesh_generations->name) list_of_mesh_generations->name=(char *)Free(list_of_mesh_generations->name);
  if(list_of_mesh_generations->names_of_mesh_generations) \
    list_of_mesh_generations->names_of_mesh_generations = \
      (char **)Free(list_of_mesh_generations->names_of_mesh_generations);


  if (list_of_mesh_generations->mg_list) {
    delete_list(list_of_mesh_generations->mg_list,destroy_mesh_generation);
    list_of_mesh_generations->mg_list=destroy_list(list_of_mesh_generations->mg_list);
  }

  if (list_of_mesh_generations) list_of_mesh_generations=(LIST_MESH_GENERATION *)Free(list_of_mesh_generations);

  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: mesh_generations_list_empty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 Liste --> leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/

int mesh_generations_list_empty(void)
{
  if (!list_of_mesh_generations) return 0;
  else if (!list_of_mesh_generations->mg_list) return 0;
  else return list_empty(list_of_mesh_generations->mg_list);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: list_mg_init
                                                                          */
/* Aufgabe:
   Liste der MG's initialisieren.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void list_mg_init(void)
{
  if (list_of_mesh_generations) list_current_init(list_of_mesh_generations->mg_list);

  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: get_list_mg_next
                                                                          */
/* Aufgabe:
   Holt naechstes AR aus der Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur mg.
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void *get_list_mg_next (void)
{
  return list_of_mesh_generations->mg_list ?
             get_list_next(list_of_mesh_generations->mg_list): NULL;
}




/**************************************************************************/
/* ROCKFLOW - Funktion: declare_mesh_generation_name
                                                                          */
/* Aufgabe:
   Setzt den Anfangszustand in allen betroffenen Knoten.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *name: Zeiger auf den Namen des AR's.
                                                                          */
/* Ergebnis:
   - Anzahl der vorhandenen AR's_Namen -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
long declare_mesh_generation_name (char *name)
{
  long i;
  char *new_name;

  if (!name) return 0;

  for ( i=0; i<list_of_mesh_generations->count_of_mesh_generations_name; i++)
    if(strcmp(list_of_mesh_generations->names_of_mesh_generations[i],name) == 0) break;
  if (i < list_of_mesh_generations->count_of_mesh_generations_name) return i;

  list_of_mesh_generations->names_of_mesh_generations= \
          (char **) Realloc(list_of_mesh_generations->names_of_mesh_generations, \
          (list_of_mesh_generations->count_of_mesh_generations_name+1)*sizeof(char **) );

  new_name=(char *)Malloc((int)strlen(name)+1);
  strcpy(new_name,name);
  list_of_mesh_generations->names_of_mesh_generations[i]=new_name;

  return ++(list_of_mesh_generations->count_of_mesh_generations_name);
}



/**************************************************************************/
/* ROCKFLOW - Funktion: undeclare_mesh_generations_names
                                                                          */
/* Aufgabe:
   Alle Vorverfeinerer von der Liste entfernen.
   Hier werden nur die Namen von der Namensliste entfernt.

                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - Anzahl der entferneten AR's_Namen -
                                                                          */
/* Programmaenderungen:
   10/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
long undeclare_mesh_generations_names (void)
{
  long i;

  for (i=0; i<list_of_mesh_generations->count_of_mesh_generations_name; i++)
    if(list_of_mesh_generations->names_of_mesh_generations[i]) \
      list_of_mesh_generations->names_of_mesh_generations[i] = \
        (char *)Free(list_of_mesh_generations->names_of_mesh_generations[i]);

  return list_of_mesh_generations->count_of_mesh_generations_name;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: insert_mesh_generation_list
                                                                          */
/* Aufgabe:
   Fuegt eine Instanz vom Typ MESH_GENERATION in die Liste.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E MESH_GENERATION *mg: Zeiger auf die Datenstruktur, die angehaengt
                             werden soll.
                                                                          */
/* Ergebnis:
   Plazierung in der Liste
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
long insert_mesh_generation_list (MESH_GENERATION *mg)
{
  list_of_mesh_generations->count_of_mesh_generations=append_list(list_of_mesh_generations->mg_list,(void *) mg);
  return list_of_mesh_generations->count_of_mesh_generations;
}








/* ================================================================= */
/* Interface

                                                                     */
/* ================================================================= */


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateMeshGenerationList
                                                                          */
/* Aufgabe:
   Erzeugt leeres Verzeichnis von Anfangszustaende (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void CreateMeshGenerationList(void)
{
  list_of_mesh_generations = create_mesh_generation_list("MESH_GENERATION");
  return;
}



/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyMeshGenerationList
                                                                          */
/* Aufgabe:
   Zerstoert Verzeichnis von Anfangszustaende (unbenannt);
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   NULL bei Fehler, sonst eine Speicheradresse
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
void DestroyMeshGenerationList(void)
{
  undeclare_mesh_generations_names();
  destroy_mesh_generation_list();
  return;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: MeshGenerationListEmpty
                                                                          */
/* Aufgabe:
   Gibt Auskunft ob die Liste leer ist, oder mindestens ein Objekt
   enthaelt.
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - 0 Liste --> leer, Ungleich 0 --> Liste nicht leer -
                                                                          */
/* Programmaenderungen:
   04/2003     MK         Erste Version
                                                                          */
/**************************************************************************/
int MeshGenerationListEmpty(void)
{
  return mesh_generations_list_empty();
}




/***  mg_lib.c                      **********************************************/
/***  Mesh Generator Library          **********************************************/
/***  Martin Kohlmeier / Gesa Ungruh  **********************************************/
/***  2001-2003                       **********************************************/
/***********************************************************************************/

/***********************************************************************************/
/***********************************************************************************/
/* Netzgenerator zur Erzeugung von 2D Rechtecknetzen mit 4-Knoten-Elemente                
                                                                                */
/***********************************************************************************/
/* Entstanden aus:
   NETZGENERATOR ZUR ERZEUGUNG VON 3D RECHTECKNETZEN
   17.01.2000 Kim Mittendorf
   Version 2 */

/* Programmaenderungen                             
   08/2001 Gesa Ungruh            2D Rechteck 
                                                                                    */
/************************************************************************************/
MESH2D *OmegaRectangle (char *file_name, long material_group)
{
  MESH2D *mesh;
  long no_of_nodes=0;
  long no_of_elements=0;

  char dateiname[1024], text [1024], *dummy;
  NODE anf_knoten3d, end_knoten3d;
  NODE anf_knoten , end_knoten;
  FILE *datei_1;
  int nx3d, ny3d, nz3d;
  int nx=0, ny=0;
  int inode, zahl=0,j=0, k=0, k1=0, k2=0, k3=0, k4=0;
  double dx, dy, node_x=0.0, node_y=0.0, node_z=0.0;
  double dum1, dum2, dum3;
  int Bezugsebene;	

  anf_knoten.x=0.0; /*TK Initialisierung*/ 
  anf_knoten.y=0.0;
  anf_knoten.z=0.0;
  end_knoten.x=0.0;
  end_knoten.y=0.0;
  end_knoten.z=0.0;

  printf("\n\n Path and file name: ");
  if (!file_name) {
    fgets(dateiname, 1024, stdin);
    if((dummy = strchr(dateiname, '\n')) != NULL) *dummy = '\0';
  }
  else {
    strcpy(dateiname,file_name);
    printf(file_name);
	}

 
	if ((datei_1 = fopen(dateiname, "r"))==NULL)
	{
		printf("\n\n FEHLER BEIM OEFFNEN!!!!!!!!");
		exit(1);
	}

	rewind(datei_1);

	printf("\n\nDATEN aus EINGABEFILE");
	printf("\n---------------------\n");

/* Anfangsknoten */
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%lf %lf %lf", &dum1, &dum2, &dum3);
    anf_knoten3d.x = dum1;
    anf_knoten3d.y = dum2;
    anf_knoten3d.z = dum3;
	printf("\n %f %f %f" ,anf_knoten3d.x,anf_knoten3d.y,anf_knoten3d.z);

/* Endknoten */
	fscanf(datei_1,"%lf %lf %lf", &dum1, &dum2, &dum3);
    end_knoten3d.x = dum1;
    end_knoten3d.y = dum2;
    end_knoten3d.z = dum3;
	printf("\n %f %f %f" ,end_knoten3d.x,end_knoten3d.y,end_knoten3d.z);

/* Anzahl der Unterteilungen */
	fscanf(datei_1,"%s",text);
	printf("\n%s\n",text);
	fscanf(datei_1,"%i%i%i", &nx3d, &ny3d, &nz3d);
	printf(" nx:%i, ny:%i, nz:%i" ,nx3d, ny3d,nz3d);

/* Bezugsebene */
	fscanf(datei_1,"%s",text);
	printf("\n%s\n",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%i", &Bezugsebene);
	printf("%i \n" ,Bezugsebene);

  switch (Bezugsebene) {
    case 0: 
   		anf_knoten.x=anf_knoten3d.x;
   		anf_knoten.y=anf_knoten3d.y;
	  	end_knoten.x=end_knoten3d.x;
	  	end_knoten.y=end_knoten3d.y;
      nx=nx3d;
      ny=ny3d;
    break;
    case 1: 
  		anf_knoten.x=anf_knoten3d.x;
  		anf_knoten.y=anf_knoten3d.z;
	  	end_knoten.x=end_knoten3d.x;
	  	end_knoten.y=end_knoten3d.z;
      nx=nx3d;
      ny=nz3d;
    break;
    case 2: 
  		anf_knoten.x=anf_knoten3d.y;
  		anf_knoten.y=anf_knoten3d.z;
	  	end_knoten.x=end_knoten3d.y;
	  	end_knoten.y=end_knoten3d.z;
      nx=ny3d;
      ny=nz3d;
    break;
	}
  	  
	printf("Anfangs-u.Endpunkt der Bezugsebene \n");
	printf("%5.1f%5.1f\n",anf_knoten.x, anf_knoten.y);
	printf("%5.1f%5.1f\n",end_knoten.x, end_knoten.y);


/* Materialnummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &material_num);
	printf("\n %i ", material_num);
*/
/* Numeriknummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &numerik_num);
	printf("\n %i ", numerik_num);  
*/

	fclose(datei_1);

/* --------------------------------------------------------------------------------*/
/* BERECHNEN DER KNOTENANZAHL UND DER ELEMENTANZAHL UND ERZEUGUNG DER KNOTEN UND ELEMENTE */
/* --------------------------------------------------------------------------------*/

	
		dx = (end_knoten.x - anf_knoten.x)/ (double) nx;
 		dy = (end_knoten.y - anf_knoten.y)/ (double) ny;

		no_of_nodes = (nx+1)*(ny+1);
                printf("\nAnzhl der erstellten Knoten: %7li", no_of_nodes);

                        
                no_of_elements = (nx * ny);
                printf("\nAnzahl der erstellten Elemente: %5li", no_of_elements);
	
		
		printf("\nElementkantenlaenge in X:%5.2f, in Y:%5.2f \n",dx, dy);
				

/* -----------------------------------------------------------------------------------*/
/* -------------- ERZEUGEN DES N�IGEN SPEICHERPLATZES ----------------------------------*/
/* -----------------------------------------------------------------------------------*/


    mesh       = (MESH2D *)    Malloc(sizeof(MESH2D)); 
    mesh->node = (NODE *)      Malloc(no_of_nodes*sizeof(NODE)); 
    mesh->elem = (ELEMENT2D *) Malloc(no_of_elements*sizeof(ELEMENT2D)); 
    mesh->node_elem = NULL;
	
/* -----------------------------------------------------------------------------------*/
/* -------------- ERZEUGEN DER KNOTEN-DATEI 2D ------------------------------------------------*/
/* -----------------------------------------------------------------------------------*/

    printf(" *** Knotenliste fuer mesh_3d ************************************\n"); 

    inode=0;


  	for(k=0;k<=ny;k++) {
	  	for(j=0;j<=nx;j++) {
        switch (Bezugsebene) {
          case 0: 
    		  	node_x = anf_knoten.x+j*dx;
		   	    node_y = anf_knoten.y+k*dy; 
			      node_z = 0.0;
          break;
          case 1: 
    		  	node_x = anf_knoten.x+j*dx;
			      node_y = 0.0;
		   	    node_z = anf_knoten.y+k*dy; 
          break;
          case 2: 
			    node_x = 0.0;
    		  	node_y = anf_knoten.x+j*dx;
		   	    node_z = anf_knoten.y+k*dy; 
          break;
      	}
				/* ------------------- Ausgabe der Koordinaten am Bildschirm -------------------------*/ 
			  printf("Knoten :%2i, j:%f, k:%f , l:%f \n", inode, (float)(node_x), (float)(node_y),(float)(node_z));

				/* ------------ Speichern der Knotendaten fuer mesh_3d --------------------------------*/
					
  			mesh->node[inode].x = node_x;
	  		mesh->node[inode].y = node_y;
  	  	mesh->node[inode].z = node_z;
        inode++; 			
			}					
	
		}



/* ---------------------------------------------------------------------------------------*/
/* -------------------------------- ERZEUGEN DER ELEMENT-DATEI 2D --------------------------------*/    
/* ---------------------------------------------------------------------------------------*/

  printf(" *** Elementliste fuer mesh_3d ***********************************\n"); 

  k1=0; zahl=0;
  for(k=0;k<=(ny-1);k++){
  	for(j=0;j<=(nx-1);j++){

			k2=k1+1;
			k3=k1+nx+2;
			k4=k1+nx+1;

                                printf("%i %li hex %3i %3i %3i %3i \n",zahl, material_group, k1, k2, k3, k4);

				/* ------------ Speichern der Knotendaten fuer mesh_3d --------------------------------*/

				mesh->elem[zahl].node[0] = (long)k1;
				mesh->elem[zahl].node[1] = (long)k2;
				mesh->elem[zahl].node[2] = (long)k3;
				mesh->elem[zahl].node[3] = (long)k4;
				mesh->elem[zahl].material_group = material_group;
   
				zahl++;
				k1++;
		
		}

		k1++;
	}


		mesh->no_of_nodes = no_of_nodes;
		mesh->no_of_elements = no_of_elements;

  return mesh; 

}
/**********************************************************************************/
/**********************************************************************************/




/**********************************************************************************/
/**********************************************************************************/


/**********************************************************************************/
/**********************************************************************************/



/**************************************************************************/
/* Funktion: OmegaMeshCombine2D
                                                                          */
/* Aufgabe:
   Kombiniert 2 Netze (2D) zu einem Gesamtnetz (2D)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh_1, mesh_2 - die zu kombinierenden Netze
   R   mesh_c - das Gesamtnetz
                                                                          */
/* Ergebnis:
   Gesamtnetz bestehend aus beiden zu kombinierenden Netzen
                                                                          */
/* Programmaenderungen:
   10/2001               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

MESH2D *OmegaMeshCombine2D (MESH2D *mesh_1,MESH2D *mesh_2)
{

  MESH2D *mesh_c;
  int lv1, lv2, lv3,lv;
  const double float_error=0.000001;
  int equal_node;
  ELEMENT2D *el_node_twice;
  

/*##########################################################################
      Allokieren des bentigten Speichers
  ########################################################################*/

  
  el_node_twice = (ELEMENT2D *) Malloc((mesh_1->no_of_elements+mesh_2->no_of_elements)*sizeof(ELEMENT2D)); 

  mesh_c       = (MESH2D *)    Malloc(sizeof(MESH2D)); 
  mesh_c->node = (NODE *)      Malloc((mesh_1->no_of_nodes+mesh_2->no_of_nodes)*sizeof(NODE)); 
  mesh_c->elem = (ELEMENT2D *) Malloc((mesh_1->no_of_elements+mesh_2->no_of_elements)*sizeof(ELEMENT2D)); 
  mesh_c->node_elem = NULL;
 

/*##########################################################################
      Suchen gleicher Knoten in den zu kombinierenden Netzen
  ########################################################################*/

/*Die Element-Knoten-Beziehung el_node_twice aller Elemente wird auf -1 gesetzt*/

 for(lv1=0;lv1<(mesh_1->no_of_elements+mesh_2->no_of_elements);lv1++) 
   for(lv2=0;lv2<4;lv2++) 
     el_node_twice[lv1].node[lv2] = -1;


/*�erprfen, ob 2 Knoten an der selben Stelle liegen*/
/*Wenn ja bekommt der Knoten die Nummer des Netzes 2!!! */

 for(lv1=0;lv1<mesh_1->no_of_nodes;lv1++) 
  { 
     equal_node=-1;  

     for(lv2=0;lv2<mesh_2->no_of_nodes;lv2++) 
	  { 
        if (OmegaNodeDistance(&mesh_1->node[lv1],&mesh_2->node[lv2])<float_error)	                                   
         equal_node=lv2;     
	  }


/*Die Knotennummer in der Element-Knoten-Beziehung wird aktualisiert*/

    if (equal_node != -1)
    {
      for(lv2=0;lv2<4;lv2++) 
      { 
		if (mesh_1->node_elem[lv1].elem[lv2] != -1)    
        {										
          for(lv3=0;lv3<4;lv3++)				
          {										
    		if (mesh_1->elem[mesh_1->node_elem[lv1].elem[lv2]].node[lv3]  == lv1) 
			el_node_twice[mesh_1->node_elem[lv1].elem[lv2]].node[lv3] = equal_node;
		  }
        }
		mesh_1->node_elem[lv1].elem[lv2] =-1;
      }
    }
  }
 
/*Schreiben der Knotendaten von Netz 2 in das neue Netz mesh_c*/

 for(lv1=0;lv1<(mesh_2->no_of_nodes);lv1++) {
   mesh_c->node[lv1].x=mesh_2->node[lv1].x;
   mesh_c->node[lv1].y=mesh_2->node[lv1].y;
   mesh_c->node[lv1].z=mesh_2->node[lv1].z;
 }

/*Schreiben der Elementdaten von Netz 2 in das neue Netz mesh_c*/

 for(lv1=0;lv1<(mesh_2->no_of_elements);lv1++) {
   mesh_c->elem[lv1].node[0]=mesh_2->elem[lv1].node[0];
   mesh_c->elem[lv1].node[1]=mesh_2->elem[lv1].node[1];
   mesh_c->elem[lv1].node[2]=mesh_2->elem[lv1].node[2];
   mesh_c->elem[lv1].node[3]=mesh_2->elem[lv1].node[3];
   mesh_c->elem[lv1].material_group=mesh_2->elem[lv1].material_group;
 }


/*##########################################################################
      Schreiben aller Knoten in das Gesamtnetz mesh_c
  ########################################################################*/

/*Alle Knoten von mesh_1, die ein Element besitzen, werden in mesh_c bertragen*/
/*Die Nummerierung der Knoten in mesh_c beginnt mit den Knoten von mesh_2, dann kommen die von mesh_1*/

  lv=-1;
  for(lv1=0;lv1<mesh_1->no_of_nodes;lv1++) 
  { 
	if (mesh_1->node_elem[lv1].elem[0] != -1)	
    {
      lv=lv+1;
	  mesh_c->node[lv+mesh_2->no_of_nodes].x=mesh_1->node[lv1].x;
	  mesh_c->node[lv+mesh_2->no_of_nodes].y=mesh_1->node[lv1].y;
	  mesh_c->node[lv+mesh_2->no_of_nodes].z=mesh_1->node[lv1].z;
    }

/*Das Herausstreichen einiger Knoten von mesh_1 wird in der Nummerierung bercksichtigt*/
/*Diese soll fortlaufend sein*/
    else
    {
      for(lv2=0;lv2<mesh_1->no_of_elements;lv2++) 
      { 
      for(lv3=0;lv3<4;lv3++) 
        { 
		  if (mesh_1->elem[lv2].node[lv3] > lv)
			  mesh_1->elem[lv2].node[lv3]=mesh_1->elem[lv2].node[lv3]-1;
        }  
      }
    }
  }
  

/*##########################################################################
      Schreiben aller Elemente in das Gesamtnetz mesh_c
  ########################################################################*/

  for(lv1=0;lv1<mesh_1->no_of_elements;lv1++) 
  { 
    mesh_c->elem[lv1+mesh_2->no_of_elements].material_group=mesh_1->elem[lv1].material_group;
    for(lv2=0;lv2<4;lv2++) 
    { 
	   if(el_node_twice[lv1].node[lv2] != -1)
	     mesh_c->elem[lv1+mesh_2->no_of_elements].node[lv2]=el_node_twice[lv1].node[lv2];
       else   
	     mesh_c->elem[lv1+mesh_2->no_of_elements].node[lv2]=mesh_1->elem[lv1].node[lv2]+mesh_2->no_of_nodes;	
    }
  }
	mesh_c->no_of_elements=mesh_2->no_of_elements+mesh_1->no_of_elements;
	mesh_c->no_of_nodes=mesh_2->no_of_nodes+lv+1;				 /*lv  von oben !!!*/


  el_node_twice = (ELEMENT2D *) Free(el_node_twice); 
  return mesh_c; 
}


/**************************************************************************/
/* Funktion: OmegaMeshCombine3D
                                                                          */
/* Aufgabe:
   Kombiniert 2 Netze (3D) zu einem Gesamtnetz (3D)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh_1, mesh_2 - die zu kombinierenden Netze
   R   mesh_c - das Gesamtnetz
                                                                          */
/* Ergebnis:
   Gesamtnetz bestehend aus beiden zu kombinierenden Netzen
                                                                          */
/* Programmaenderungen:
                                                          */
/**************************************************************************/

MESH3D *OmegaMeshCombine3D (MESH3D *mesh_1,MESH3D *mesh_2)
/* 
   Ausgangsnetz:         mesh_2
   Hinzuzufgendes Netz: mesh_1
*/
{
  MESH3D *mesh_c;
  int lv1, lv2, lv3,lv;
  const double float_error=0.000001;
  int equal_node;
  ELEMENT3D *el_node_twice;
  

/*##########################################################################
      Allokieren des bentigten Speichers
  ########################################################################*/

  
  el_node_twice = (ELEMENT3D *) Malloc((mesh_1->no_of_elements+mesh_2->no_of_elements)*sizeof(ELEMENT3D)); 

  mesh_c       = (MESH3D *)    Malloc(sizeof(MESH3D)); 
  mesh_c->node = (NODE *)      Malloc((mesh_1->no_of_nodes+mesh_2->no_of_nodes)*sizeof(NODE)); 
  mesh_c->elem = (ELEMENT3D *) Malloc((mesh_1->no_of_elements+mesh_2->no_of_elements)*sizeof(ELEMENT3D)); 
  mesh_c->node_elem = NULL;

/*##########################################################################
      Suchen gleicher Knoten in den zu kombinierenden Netzen
  ########################################################################*/

/*Die Element-Knoten-Beziehung el_node_twice aller Elemente wird auf -1 gesetzt*/

 for(lv1=0;lv1<(mesh_1->no_of_elements+mesh_2->no_of_elements);lv1++) 
   for(lv2=0;lv2<8;lv2++) 
     el_node_twice[lv1].node[lv2] = -1;


/*�erprfen, ob 2 Knoten an der selben Stelle liegen*/
/*Wenn ja bekommt der Knoten die Nummer des Netzes 2!!! */

 for(lv1=0;lv1<mesh_1->no_of_nodes;lv1++) 
  { 
     equal_node=-1;  

     for(lv2=0;lv2<mesh_2->no_of_nodes;lv2++) 
	  { 
        if (OmegaNodeDistance(&mesh_1->node[lv1],&mesh_2->node[lv2])<float_error)	                                   
        equal_node=lv2;     
	  }


/*Die Knotennummer in der Element-Knoten-Beziehung wird aktualisiert*/

    if (equal_node != -1)
    {
      for(lv2=0;lv2<8;lv2++) 
      { 
		if (mesh_1->node_elem[lv1].elem[lv2] != -1)    
        {										
          for(lv3=0;lv3<8;lv3++)				
          {										
    		if (mesh_1->elem[mesh_1->node_elem[lv1].elem[lv2]].node[lv3]  == lv1) 
			el_node_twice[mesh_1->node_elem[lv1].elem[lv2]].node[lv3] = equal_node;
		  }
        }
		mesh_1->node_elem[lv1].elem[lv2] =-1;
      }
    }
  }
 
/*Schreiben der Knotendaten von Netz 2 in das neue Netz mesh_c*/

 for(lv1=0;lv1<(mesh_2->no_of_nodes);lv1++) {
   mesh_c->node[lv1].x=mesh_2->node[lv1].x;
   mesh_c->node[lv1].y=mesh_2->node[lv1].y;
   mesh_c->node[lv1].z=mesh_2->node[lv1].z;
 }

/*Schreiben der Elementdaten von Netz 2 in das neue Netz mesh_c*/

 for(lv1=0;lv1<(mesh_2->no_of_elements);lv1++) {
   mesh_c->elem[lv1].node[0]=mesh_2->elem[lv1].node[0];
   mesh_c->elem[lv1].node[1]=mesh_2->elem[lv1].node[1];
   mesh_c->elem[lv1].node[2]=mesh_2->elem[lv1].node[2];
   mesh_c->elem[lv1].node[3]=mesh_2->elem[lv1].node[3];
   mesh_c->elem[lv1].node[4]=mesh_2->elem[lv1].node[4];
   mesh_c->elem[lv1].node[5]=mesh_2->elem[lv1].node[5];
   mesh_c->elem[lv1].node[6]=mesh_2->elem[lv1].node[6];
   mesh_c->elem[lv1].node[7]=mesh_2->elem[lv1].node[7];
   mesh_c->elem[lv1].material_group=mesh_2->elem[lv1].material_group;
 }


/*##########################################################################
      Schreiben aller Knoten in das Gesamtnetz mesh_c
  ########################################################################*/

/*Alle Knoten von mesh_1, die ein Element besitzen, werden in mesh_c bertragen*/
/*Die Nummerierung der Knoten in mesh_c beginnt mit den Knoten von mesh_2, dann kommen die von mesh_1*/

  lv=-1;
  for(lv1=0;lv1<mesh_1->no_of_nodes;lv1++) 
  { 
	if (mesh_1->node_elem[lv1].elem[0] != -1)	
    {
      lv=lv+1;
	  mesh_c->node[lv+mesh_2->no_of_nodes].x=mesh_1->node[lv1].x;
	  mesh_c->node[lv+mesh_2->no_of_nodes].y=mesh_1->node[lv1].y;
	  mesh_c->node[lv+mesh_2->no_of_nodes].z=mesh_1->node[lv1].z;
    }

/*Das Herausstreichen einiger Knoten von mesh_1 wird in der Nummerierung bercksichtigt*/
/*Diese soll fortlaufend sein*/
    else
    {
      for(lv2=0;lv2<mesh_1->no_of_elements;lv2++) 
      { 
      for(lv3=0;lv3<8;lv3++) 
        { 
		  if (mesh_1->elem[lv2].node[lv3] > lv)
			  mesh_1->elem[lv2].node[lv3]=mesh_1->elem[lv2].node[lv3]-1;
        }  
      }
    }
  }
  

/*##########################################################################
      Schreiben aller Elemente in das Gesamtnetz mesh_c
  ########################################################################*/

  for(lv1=0;lv1<mesh_1->no_of_elements;lv1++) 
  { 
    mesh_c->elem[lv1+mesh_2->no_of_elements].material_group=mesh_1->elem[lv1].material_group;
    for(lv2=0;lv2<8;lv2++) 
    { 
	   if(el_node_twice[lv1].node[lv2] != -1)
	     mesh_c->elem[lv1+mesh_2->no_of_elements].node[lv2]=el_node_twice[lv1].node[lv2];
       else   
	     mesh_c->elem[lv1+mesh_2->no_of_elements].node[lv2]=mesh_1->elem[lv1].node[lv2]+mesh_2->no_of_nodes;	
    }
  }
	mesh_c->no_of_elements=mesh_2->no_of_elements+mesh_1->no_of_elements;
	mesh_c->no_of_nodes=mesh_2->no_of_nodes+lv+1;				 /*lv  von oben !!!*/

  el_node_twice = (ELEMENT3D *) Free(el_node_twice); 
  return mesh_c; 
}


/**************************************************************************/
/* Funktion: OmegaBuildNodeElRel2D
                                                                          */
/* Aufgabe:
   Feststellen der Knoten-Element-Beziehungen eines Netzes (2D)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh - bisher ohne die Daten der Knoten-Element-Beziehung
   R   mesh - das Netz inklusive der Knoten-Element-Beziehung
                                                                          */
/* Ergebnis:
   Ein Netz (2D) mit Angabe der bestehenden Knoten-Element-Beziehungen
                                                                          */
/* Programmaenderungen:
   10/2001               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

MESH2D *OmegaBuildNodeElRel2D (MESH2D *mesh)
{

 
  int lv2, lv3, lv;    

  if (mesh->node_elem) {
    mesh->node_elem = (NODEELEMENTS2D *) Free(mesh->node_elem);
    mesh->node_elem = NULL;
  }
  mesh->node_elem = (NODEELEMENTS2D *) Malloc(mesh->no_of_nodes*sizeof(NODEELEMENTS2D)); 

/*Die Matrix der Knoten-Element-Beziehungen wird auf -1 gesetzt*/
  for(lv2=0;lv2<mesh->no_of_nodes;lv2++) 
  { 

    for(lv3=0;lv3<4;lv3++) 
    { 
      mesh->node_elem[lv2].elem[lv3]=-1;
    }

/*Existiert eine Element-Knoten-Beziehung, wird die entsprechende Elementnummer in die*/
/*Knoten-Element-Beziehung eingesetzt*/
  }
  for(lv2=0;lv2<mesh->no_of_elements;lv2++) 
  { 
    for(lv3=0;lv3<4;lv3++) 
    { 
      lv=0;
      while (mesh->node_elem[mesh->elem[lv2].node[lv3]].elem[lv]!=-1) lv=lv+1;
      mesh->node_elem[mesh->elem[lv2].node[lv3]].elem[lv]=lv2;
    }
  }

  return mesh; 
}

/**************************************************************************/
/* Funktion: OmegaBuildNodeElRel3D
                                                                          */
/* Aufgabe:
   Feststellen der Knoten-Element-Beziehungen eines Netzes (3D)
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh - bisher ohne die Daten der Knoten-Element-Beziehung
   R   mesh - das Netz inklusive der Knoten-Element-Beziehung
                                                                          */
/* Ergebnis:
   Ein Netz (3D) mit Angabe der bestehenden Knoten-Element-Beziehungen
                                                                          */
/* Programmaenderungen:
   10/2001               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/


MESH3D *OmegaBuildNodeElRel3D (MESH3D *mesh) 
{

  int lv2, lv3, lv, no_el = DEF_no_of_el_per_node_3d, nn = 8;    
  char text_dum[1024], *dummy;
  if (mesh->node_elem) {
    mesh->node_elem = (NODEELEMENTS3D *) Free(mesh->node_elem);
    mesh->node_elem = NULL;
  }
  mesh->node_elem = (NODEELEMENTS3D *) Malloc (mesh->no_of_nodes*sizeof(NODEELEMENTS3D)); 

  /*Die Matrix der Knoten-Element-Beziehungen wird auf -1 gesetzt*/
  for(lv2=0;lv2<mesh->no_of_nodes;lv2++) 
  { 
    for(lv3=0;lv3<no_el;lv3++) 
    { 
      mesh->node_elem[lv2].elem[lv3]=-1;
    }
  }

/*Existiert eine Element-Knoten-Beziehung, wird die entsprechende Elementnummer in die*/
/*Knoten-Element-Beziehung eingesetzt*/
  for(lv2=0;lv2<mesh->no_of_elements;lv2++) 
  { 
    for(lv3=0;lv3<nn;lv3++) 
    { 
      lv=0;
      while (mesh->node_elem[mesh->elem[lv2].node[lv3]].elem[lv]!=-1) {
        lv=lv+1;
        if (lv>no_el) {
          printf("\n\n FEHLER: DEF_no_of_el_per_node_3d kleiner als vorhandene Elenete am Knoten %3li: no_el>=%3i\n",mesh->elem[lv2].node[lv3],lv);
          fgets(text_dum, 1024, stdin);
          if((dummy = strchr(text_dum, '\n')) != NULL) *dummy = '\0';
        }
      }
      mesh->node_elem[mesh->elem[lv2].node[lv3]].elem[lv]=lv2;
    }
  }

  return mesh; 
}

/**************************************************************************/
/* Funktion: OmegaExpand2DTo3D
                                                                          */
/* Aufgabe:
   Expndiest ein 2D-Netz zu einem 3D-Netz
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   Name der Eingaedatei
   E   Materialgruppe
   E   mesh_2d
   R   mesh_3d
                                                                          */
/* Ergebnis:
   Ein Netz (3D) mit Angabe der bestehenden Knoten-Element-Beziehungen
                                                                          */
/* Programmaenderungen:
   12/2001    G.Ungruh                                                    */
/**************************************************************************/
MESH3D *OmegaExpand2DTo3D (char *file_name, long material_group, MESH2D *mesh_2d)
{
  MESH3D *mesh_3d;
  long no_of_nodes=0;
  long no_of_elements=0;

  char dateiname[1024], text [1024], *dummy;
  NODE anf_knoten, end_knoten;
  FILE *datei_1;
  int nx, ny, nz,lv1;
  int i=0, k=0;
  double dx, dy,dz;
  float dum1, dum2, dum3;
  int Bezugsebene;	
  int num_el_3d,num_el_2d,num_node_3d;

/* ----------------------------------------------------------------------------------*/
/* ----------------- DATEN LADEN ANFANGSPUNKT ENDPUNKT NX NY NZ ---------------------*/
/* ----------------------------------------------------------------------------------*/
  
    printf("\n\n\n\n Path and file name: ");
    if (!file_name) {
      fgets(dateiname, 1024, stdin);
      if((dummy = strchr(dateiname, '\n')) != NULL) *dummy = '\0';
    }
    else {
      strcpy(dateiname,file_name);
      printf(dateiname);
	}
	if ((datei_1 = fopen(dateiname, "r"))==NULL)
	{ 
		printf("\n\n FEHLER BEIM OEFFNEN!!!!!!!!");
		exit(1);
	}

	rewind(datei_1);

	printf("\n\nDATEN aus EINGABEFILE");
	printf("\n---------------------\n");

/* Anfangsknoten */
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
    anf_knoten.x = dum1;
    anf_knoten.y = dum2;
    anf_knoten.z = dum3;
	printf("\n %f %f %f" ,anf_knoten.x,anf_knoten.y,anf_knoten.z);

/* Endknoten */
	fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
    end_knoten.x = dum1;
    end_knoten.y = dum2;
    end_knoten.z = dum3;
	printf("\n %f %f %f" ,end_knoten.x,end_knoten.y,end_knoten.z);

/* Anzahl der Unterteilungen */
	fscanf(datei_1,"%s",text);
	printf("\n%s\n",text);
	fscanf(datei_1,"%i%i%i", &nx, &ny, &nz);
	printf(" nx:%i, ny:%i, nz:%i" ,nx, ny,nz);

/* Bezugsebene */
	fscanf(datei_1,"%s",text);
	printf("\n%s\n",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%s",text);
	fscanf(datei_1,"%i", &Bezugsebene);
	printf("%i \n" ,Bezugsebene);

/* Materialnummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &material_num);
	printf("\n %i ", material_num);
*/

/* Numeriknummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &numerik_num);
	printf("\n %i ", numerik_num);  
*/
 
	fclose(datei_1);





/* --------------------------------------------------------------------------------*/
/* BERECHNEN DER KNOTENANZAHL UND DER ELEMENTANZAHL UND ERZEUGUNG DER KNOTEN UND ELEMENTE */
/* --------------------------------------------------------------------------------*/

		dx = (end_knoten.x - anf_knoten.x)/ (double) nx;
 		dy = (end_knoten.y - anf_knoten.y)/ (double) ny;	
		dz = (end_knoten.z - anf_knoten.z)/ (double) nz;
 

	lv1=0;     

	  if(Bezugsebene==0)
		{ lv1=nz; }
	  if(Bezugsebene==1)
		{ lv1=ny; }
	  if(Bezugsebene==2)
		{ lv1=nx; }
		  

		no_of_nodes = (mesh_2d->no_of_nodes) * (lv1+1);
                printf("\nANZAHL DER ERSTELLTEN KNOTEN:%7li", no_of_nodes);
			
		no_of_elements = (mesh_2d->no_of_elements) * (lv1);
                printf("\nANZAHL DER ERSTELLTEN ELEMENTE:%5li", no_of_elements);
		
		printf("\nKANTENLAeNGE IN X:%5.2f, IN Y:%5.2f, IN Z:%5.2f \n",dx, dy, dz);

				
/* -----------------------------------------------------------------------------------*/
/* ----------------- SPEICHER ALLOKIEREN --------------------------------------*/
/* -----------------------------------------------------------------------------------*/


  mesh_3d       = (MESH3D *)    Malloc(sizeof(MESH3D)); 
  mesh_3d->node = (NODE *)      Malloc(no_of_nodes*sizeof(NODE)); 
  mesh_3d->elem = (ELEMENT3D *) Malloc(no_of_elements*sizeof(ELEMENT3D)); 
  mesh_3d->node_elem = NULL;

/* -----------------------------------------------------------------------------------*/
/* -------------- ERZEUGEN DER KNOTEN-DATEI 3D ------------------------------------------------*/
/* -----------------------------------------------------------------------------------*/

	  lv1=0;     
      num_node_3d=0;

	  if(Bezugsebene==0)
		{ lv1=nz; }
	  if(Bezugsebene==1)
		{ lv1=ny; }
	  if(Bezugsebene==2)
		{ lv1=nx; }
		  

	  for(i=0;i<=lv1;i++){
	    for(k=0;k<=(mesh_2d->no_of_nodes-1);k++){


 			if(Bezugsebene==0){
				mesh_3d->node[num_node_3d].x = mesh_2d->node[k].x;
		   	mesh_3d->node[num_node_3d].y = mesh_2d->node[k].y;
				mesh_3d->node[num_node_3d].z = (double)anf_knoten.z+i*dz;
		
				num_node_3d++;
			}

			if(Bezugsebene==1){
				mesh_3d->node[num_node_3d].x = mesh_2d->node[k].x;
				mesh_3d->node[num_node_3d].y = (double)anf_knoten.y+i*dy;
				mesh_3d->node[num_node_3d].z = mesh_2d->node[k].z;
		
				num_node_3d++;
			}

			if(Bezugsebene==2){
			    mesh_3d->node[num_node_3d].x = (double)anf_knoten.x+i*dx;
	        mesh_3d->node[num_node_3d].y = mesh_2d->node[k].y;
			    mesh_3d->node[num_node_3d].z = mesh_2d->node[k].z;
		
			    num_node_3d++;
			}
		}

	  }
      mesh_3d->no_of_nodes = num_node_3d;


/* ---------------------------------------------------------------------------------------*/
/* -------------------------------- ERZEUGEN DER ELEMENTDATEI 3D --------------------------------*/    
/* ---------------------------------------------------------------------------------------*/


  printf(" *** Elementliste 3D ***********************************\n"); 

	i=0; num_el_3d=0;
	lv1=0;     
    
	if(Bezugsebene==0)
		{ lv1=nz; }
	if(Bezugsebene==1)
		{ lv1=ny; }
	if(Bezugsebene==2)
		{ lv1=nx; }

 	for(i=0;i<=(lv1-1);i++){
		for(num_el_2d=0;num_el_2d<=(mesh_2d->no_of_elements-1);num_el_2d++){

      if ((Bezugsebene==0) || (Bezugsebene==2)) {
        mesh_3d->elem[num_el_3d].node[4] = (mesh_2d->elem[num_el_2d].node[0])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[5] = (mesh_2d->elem[num_el_2d].node[1])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[6] = (mesh_2d->elem[num_el_2d].node[2])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[7] = (mesh_2d->elem[num_el_2d].node[3])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[0] = mesh_3d->elem[num_el_3d].node[4]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[1] = mesh_3d->elem[num_el_3d].node[5]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[2] = mesh_3d->elem[num_el_3d].node[6]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[3] = mesh_3d->elem[num_el_3d].node[7]+(mesh_2d->no_of_nodes);
      } else if (Bezugsebene==1)  { /* sonst negatives Volumen */ 
        mesh_3d->elem[num_el_3d].node[0] = (mesh_2d->elem[num_el_2d].node[0])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[1] = (mesh_2d->elem[num_el_2d].node[1])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[2] = (mesh_2d->elem[num_el_2d].node[2])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[3] = (mesh_2d->elem[num_el_2d].node[3])+i*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[4] = mesh_3d->elem[num_el_3d].node[0]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[5] = mesh_3d->elem[num_el_3d].node[1]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[6] = mesh_3d->elem[num_el_3d].node[2]+(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[7] = mesh_3d->elem[num_el_3d].node[3]+(mesh_2d->no_of_nodes);
      }
      if (material_group==-1) {
        mesh_3d->elem[num_el_3d].material_group = mesh_2d->elem[num_el_2d].material_group;
      }  
      else	{
        mesh_3d->elem[num_el_3d].material_group = material_group;
      }
			num_el_3d++;

		}

		num_el_2d=0;
	}
   
     mesh_3d->no_of_elements = num_el_3d;

     return (mesh_3d);

}



/**************************************************************************
  Funktion: OmegaRotate2DTo3D
                                                                          
  Aufgabe:
   Expndiest ein 2D-Netz zu einem 3D-Netz
                                                                          
  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   Materialgruppe (-1: Materialgruppe wie im 2D-Netz)
   E   mesh_2d
   R   mesh_3d
  Eingabedaten ber Datei:

    Viertelrohr
    0.5 0.0 0.0  1.0 1.0 0.0
    Anzahl
    1 2 8
    Bezugsebene 
    x-y->Eingabe0
    x-z->Eingabe1
    y-z->Eingabe2
    0
    Drehachse
    0.0 -10.0 0.0  0.0 10.0 0.0
    Drehwinkelanzahl
    2
    Drehwinkelkoordinaten
    <Drehwinkel>-oder-<Drehwinkelkoorinaten>
    Beginn-der-Drehachse-und-Drehwinkelkoordinaten-(Anzahl_Winkel_+1)-in-eine-Ebene!
     1.0 -10.0 0.0
     1.0 -10.0 -1.0
    -1.0 -10.0  0.0

   Ergebnis:
   Ein Netz (3D) mit Angabe der bestehenden Knoten-Element-Beziehungen
                                                                          
   Programmaenderungen:
   03/2002    M.Kohlmeier/G.Ungruh                                                    
 **************************************************************************/

MESH3D *OmegaRotate2DTo3D (char *file_name, char *path_name, long material_group, MESH2D *mesh_2d)
{
  MESH3D *mesh_3d;
  MESH2D *mesh_2d_rot_ax_is_origin;
  MESH2D *mesh_2d_rot;
  long no_of_nodes=0;
  long no_of_elements=0;

  char dateiname[1024], text [1024], text_dum [1024], *dummy;
  NODE anf_knoten, end_knoten,  rot_ax_end_point,  rot_ax_start_point, unit_rot_ax;
  FILE *datei_1;
  long nx, ny, nz,lv,lv1,n_alpha;
  long i=0, k=0;
  double dx, dy,dz;
  double alpha;
  double *angle_of_rotation;
  NODE   *angle_of_rotation_by_coord, point_of_origin;
  float dum1, dum2, dum3;
  long Bezugsebene;
  long num_el_3d,num_el_2d,num_node_3d;
  int coord_2_angle_of_rotation=0;
/* ----------------------------------------------------------------------------------*/
/* ----------------- DATEN LADEN ANFANGSPUNKT ENDPUNKT NX NY NZ ---------------------*/
/* ----------------------------------------------------------------------------------*/
  point_of_origin.x=0.0;
  point_of_origin.y=0.0;
  point_of_origin.z=0.0;

  printf("\n\n\n\n Path and file name: ");
  if (!file_name) {
    fgets(dateiname, 1024, stdin);
    if((dummy = strchr(dateiname, '\n')) != NULL) *dummy = '\0';
  }
  else {
    if (path_name) {
      strcpy(dateiname, path_name);
      strcpy(dateiname, strcat(dateiname,file_name));
    }  else
    strcpy(dateiname,file_name);
    printf(dateiname);
  }
  if ((datei_1 = fopen(dateiname, "r"))==NULL)
  { 
    printf("\n\n FEHLER BEIM OEFFNEN!!!!!!!!");
    exit(1);
  }

  rewind(datei_1);

  printf("\n\nDATEN aus EINGABEFILE");
  printf("\n---------------------\n");

/* Anfangsknoten */
  fscanf(datei_1,"%s",text);
  printf("\n%s",text);
  fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
  anf_knoten.x = dum1;
  anf_knoten.y = dum2;
  anf_knoten.z = dum3;
  printf("\n %f %f %f" ,anf_knoten.x,anf_knoten.y,anf_knoten.z);

/* Endknoten */
  fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
  end_knoten.x = dum1;
  end_knoten.y = dum2;
  end_knoten.z = dum3;
  printf("\n %f %f %f" ,end_knoten.x,end_knoten.y,end_knoten.z);

/* Anzahl der Unterteilungen */
  fscanf(datei_1,"%s",text);
  printf("\n%s\n",text);
  fscanf(datei_1,"%li%li%li", &nx, &ny, &nz);
  printf(" nx:%li, ny:%li, nz:%li" ,nx, ny,nz);

/* Bezugsebene */
  fscanf(datei_1,"%s",text);
  printf("\n%s\n",text);
  fscanf(datei_1,"%s",text);
  fscanf(datei_1,"%s",text);
  fscanf(datei_1,"%s",text);
  fscanf(datei_1,"%li", &Bezugsebene);
  printf("%li \n" ,Bezugsebene);

/* Materialnummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &material_num);
	printf("\n %i ", material_num);
*/

/* Numeriknummer
	fscanf(datei_1,"%s",text);
	printf("\n%s",text);
	fscanf(datei_1,"%i", &numerik_num);
	printf("\n %i ", numerik_num);  
*/
/* Drehachse */
  fscanf(datei_1,"%s",text);
  printf("\n%s\n",text);
/* Anfangsknoten */
  fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
  rot_ax_start_point.x = dum1;
  rot_ax_start_point.y = dum2;
  rot_ax_start_point.z = dum3;
  printf("\n %f %f %f" ,rot_ax_start_point.x,rot_ax_start_point.y,rot_ax_start_point.z);
/* Endknoten */
  fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
  rot_ax_end_point.x = dum1;
  rot_ax_end_point.y = dum2;
  rot_ax_end_point.z = dum3;
  printf("\n %f %f %f" ,rot_ax_end_point.x,rot_ax_end_point.y,rot_ax_end_point.z);

/* Anzahl Drehwinkel */
  fscanf(datei_1,"%s",text);
  printf("\n%s\n",text);
  fscanf(datei_1,"%li", &n_alpha);
  printf(" n_alpha:%li" ,n_alpha);

/* Drehwinkel */
  angle_of_rotation      = (double *)    Malloc((n_alpha+1)*sizeof(double)); 
  angle_of_rotation_by_coord = (NODE *)      Malloc((n_alpha+1)*sizeof(NODE)); 

  fscanf(datei_1,"%s",text);
  fscanf(datei_1,"%s",text_dum);
  fscanf(datei_1,"%s",text_dum);
  printf("\n%s\n",text);
  if (strcmp(text,"Drehwinkel")==0) {
    angle_of_rotation[0]=0.0;
    for(lv=1;lv<=n_alpha;lv++){
      fscanf(datei_1,"%f", &dum1);
      angle_of_rotation[lv]=dum1;
      printf(" alpha %3li: %f \n",lv,angle_of_rotation[lv]);
    }
  }
  else if (strcmp(text,"Drehwinkelkoordinaten")==0) {
    coord_2_angle_of_rotation=1;
    for(lv=0;lv<=n_alpha;lv++){
      fscanf(datei_1,"%f %f %f", &dum1, &dum2, &dum3);
      angle_of_rotation_by_coord[lv].x=dum1;
      angle_of_rotation_by_coord[lv].y=dum2;
      angle_of_rotation_by_coord[lv].z=dum3;
    }
  }
  fclose(datei_1);



/* --------------------------------------------------------------------------------*/
/* BERECHNEN DER KNOTENANZAHL UND DER ELEMENTANZAHL UND ERZEUGUNG DER KNOTEN UND ELEMENTE */
/* --------------------------------------------------------------------------------*/

  dx = (end_knoten.x - anf_knoten.x)/ (double) nx;
  dy = (end_knoten.y - anf_knoten.y)/ (double) ny;
  dz = (end_knoten.z - anf_knoten.z)/ (double) nz;


  lv1=0;

  if(Bezugsebene==0)
    { lv1=nz; }
  if(Bezugsebene==1)
    { lv1=ny; }
  if(Bezugsebene==2)
    { lv1=nx; }

  no_of_nodes = (mesh_2d->no_of_nodes) * (lv1*n_alpha+1);
  printf("\nANZAHL DER ZU ERSTELLENDEN KNOTEN:%7li", no_of_nodes);

  no_of_elements = (mesh_2d->no_of_elements) * (lv1*n_alpha);
  printf("\nANZAHL DER ZU ERSTELLENDEN ELEMENTE:%5li", no_of_elements);

  printf("\nKANTENLAENGE IN X:%5.2f, IN Y:%5.2f, IN Z:%5.2f \n",dx, dy, dz);

/* -----------------------------------------------------------------------------------*/
/* ----------------- SPEICHER ALLOKIEREN ---------------------------------------------*/
/* -----------------------------------------------------------------------------------*/


  mesh_3d       = (MESH3D *)    Malloc(sizeof(MESH3D)); 
  mesh_3d->node = (NODE *)      Malloc(no_of_nodes*sizeof(NODE)); 
  mesh_3d->elem = (ELEMENT3D *) Malloc(no_of_elements*sizeof(ELEMENT3D)); 
  mesh_3d->node_elem = NULL; 


/* -----------------------------------------------------------------------------------*/
/* --------- VERSCHIEBEN DES DREHACHSENSTARTPUNKTS INKL. KOERPER IN DEN URSPRUNG -----*/
/* -----------------------------------------------------------------------------------*/

  rot_ax_start_point.x = -rot_ax_start_point.x; 
  rot_ax_start_point.y = -rot_ax_start_point.y;
  rot_ax_start_point.z = -rot_ax_start_point.z; 


  mesh_2d_rot_ax_is_origin=OmegaDuplicateMesh2D(mesh_2d);
  OmegaDisplaceMesh2D(mesh_2d_rot_ax_is_origin,&rot_ax_start_point);
  for(lv=0;lv<=n_alpha;lv++){
    OmegaDisplaceNode(&angle_of_rotation_by_coord[lv],&rot_ax_start_point);
  }

  rot_ax_start_point.x = -rot_ax_start_point.x; 
  rot_ax_start_point.y = -rot_ax_start_point.y; 
  rot_ax_start_point.z = -rot_ax_start_point.z; 

  unit_rot_ax.x=(rot_ax_end_point.x-rot_ax_start_point.x) / OmegaNodeDistance(&rot_ax_start_point,&rot_ax_end_point);
  unit_rot_ax.y=(rot_ax_end_point.y-rot_ax_start_point.y) / OmegaNodeDistance(&rot_ax_start_point,&rot_ax_end_point);
  unit_rot_ax.z=(rot_ax_end_point.z-rot_ax_start_point.z) / OmegaNodeDistance(&rot_ax_start_point,&rot_ax_end_point);

/* -----------------------------------------------------------------------------------*/
/* -------------- ERZEUGEN DER KNOTEN-DATEN 3D ---------------------------------------*/
/* -----------------------------------------------------------------------------------*/
  lv1=0;
  num_node_3d=0;

  if(Bezugsebene==0)
  { lv1=nz; }
  if(Bezugsebene==1)
  { lv1=ny; }
  if(Bezugsebene==2)
  { lv1=nx; }
  

/* --------------- Koordinaten in Winkel umrechnen ----------------------------------*/

  if (coord_2_angle_of_rotation) {
    angle_of_rotation[0]=0.0;
    for(lv=1;lv<=n_alpha;lv++){
      angle_of_rotation[lv]=acos( (angle_of_rotation_by_coord[0].x*angle_of_rotation_by_coord[lv].x
                                  +  angle_of_rotation_by_coord[0].y*angle_of_rotation_by_coord[lv].y
                                  +  angle_of_rotation_by_coord[0].z*angle_of_rotation_by_coord[lv].z)
                                  /  (OmegaNodeDistance(&point_of_origin, &angle_of_rotation_by_coord[0])
                                       * OmegaNodeDistance(&point_of_origin, &angle_of_rotation_by_coord[lv])) );
    }
  }

/* --------------- Neue Knoten errechnen ---------------------------------------------*/

  mesh_2d_rot=OmegaDuplicateMesh2D(mesh_2d);
  for(k=0;k<=(mesh_2d->no_of_nodes-1);k++){
    mesh_3d->node[num_node_3d].x = mesh_2d_rot->node[k].x;
    mesh_3d->node[num_node_3d].y = mesh_2d_rot->node[k].y;
    mesh_3d->node[num_node_3d].z = mesh_2d_rot->node[k].z;
    num_node_3d++;
  }
  for(lv=1;lv<=n_alpha;lv++){
  alpha=angle_of_rotation[lv]-angle_of_rotation[lv-1];
    for(i=1;i<=lv1;i++){
      if (mesh_2d_rot) {
        if (mesh_2d_rot->node)       mesh_2d_rot->node =      (NODE *) Free(mesh_2d_rot->node);
        if (mesh_2d_rot->elem)       mesh_2d_rot->elem =      (ELEMENT2D *) Free(mesh_2d_rot->elem);
        if (mesh_2d_rot->node_elem)  mesh_2d_rot->node_elem = (NODEELEMENTS2D *) Free(mesh_2d_rot->node_elem);
        mesh_2d_rot = (MESH2D *) Free(mesh_2d_rot);
      }
      mesh_2d_rot=OmegaDuplicateMesh2D(mesh_2d_rot_ax_is_origin);
      OmegaRotateMesh2D(mesh_2d_rot,&unit_rot_ax,angle_of_rotation[lv-1]+alpha*i/lv1);
      OmegaDisplaceMesh2D(mesh_2d_rot,&rot_ax_start_point);
      for(k=0;k<=(mesh_2d->no_of_nodes-1);k++){
        mesh_3d->node[num_node_3d].x = mesh_2d_rot->node[k].x;
        mesh_3d->node[num_node_3d].y = mesh_2d_rot->node[k].y;
        mesh_3d->node[num_node_3d].z = mesh_2d_rot->node[k].z;
        num_node_3d++;
      }
    }
  }
  mesh_3d->no_of_nodes = num_node_3d;


/* ---------------------------------------------------------------------------------------*/
/* -------------------------------- ERZEUGEN DER ELEMENTDATEN 3D -------------------------*/    
/* ---------------------------------------------------------------------------------------*/


  printf(" *** Elementliste 3D ***********************************\n"); 

  num_el_3d=0;

  for(i=0;i<=(lv1*n_alpha-1);i++){
    for(num_el_2d=0;num_el_2d<=(mesh_2d->no_of_elements-1);num_el_2d++){
        mesh_3d->elem[num_el_3d].node[4] = (mesh_2d->elem[num_el_2d].node[0])+(i+1)*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[5] = (mesh_2d->elem[num_el_2d].node[1])+(i+1)*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[6] = (mesh_2d->elem[num_el_2d].node[2])+(i+1)*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[7] = (mesh_2d->elem[num_el_2d].node[3])+(i+1)*(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[0] = mesh_3d->elem[num_el_3d].node[4]-(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[1] = mesh_3d->elem[num_el_3d].node[5]-(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[2] = mesh_3d->elem[num_el_3d].node[6]-(mesh_2d->no_of_nodes);
        mesh_3d->elem[num_el_3d].node[3] = mesh_3d->elem[num_el_3d].node[7]-(mesh_2d->no_of_nodes);
        if (material_group==-1) mesh_3d->elem[num_el_3d].material_group = mesh_2d->elem[num_el_2d].material_group;
        else mesh_3d->elem[num_el_3d].material_group = material_group;
      num_el_3d++;
    }
  }
  mesh_3d->no_of_elements = num_el_3d;

  OmegaBuildNodeElRel3D(mesh_3d);

  if (mesh_2d_rot_ax_is_origin) {
    if (mesh_2d_rot_ax_is_origin->node)       mesh_2d_rot_ax_is_origin->node =      (NODE *) Free(mesh_2d_rot_ax_is_origin->node);
    if (mesh_2d_rot_ax_is_origin->elem)       mesh_2d_rot_ax_is_origin->elem =      (ELEMENT2D *) Free(mesh_2d_rot_ax_is_origin->elem);
    if (mesh_2d_rot_ax_is_origin->node_elem)  mesh_2d_rot_ax_is_origin->node_elem = (NODEELEMENTS2D *) Free(mesh_2d_rot_ax_is_origin->node_elem);
    mesh_2d_rot_ax_is_origin = (MESH2D *) Free(mesh_2d_rot_ax_is_origin);
  }
  if (mesh_2d_rot) {
    if (mesh_2d_rot->node)       mesh_2d_rot->node =      (NODE *) Free(mesh_2d_rot->node);
    if (mesh_2d_rot->elem)       mesh_2d_rot->elem =      (ELEMENT2D *) Free(mesh_2d_rot->elem);
    if (mesh_2d_rot->node_elem)  mesh_2d_rot->node_elem = (NODEELEMENTS2D *) Free(mesh_2d_rot->node_elem);
    mesh_2d_rot = (MESH2D *) Free(mesh_2d_rot);
  }
  angle_of_rotation      = (double *) Free(angle_of_rotation); 
  angle_of_rotation_by_coord = (NODE *) Free(angle_of_rotation_by_coord); 

  return mesh_3d;
}




/**************************************************************************/
/* Funktion: OmegaRotateNode
                                                                          */
/* Aufgabe:
   Dreht einen Punkt um eine Achse
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   node0, unit_axis, alpha
   R   node1 
                                                                          */
/* Ergebnis:
   node1: node0 um den Winkel alpha um die Achse unit_axis 
          (|unit_axis|=1) gedreht
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaRotateNode(NODE *node0, NODE *unit_axis, double alpha)
{
  static NODE node_dum;
  node_dum.x=node0->x;
  node_dum.y=node0->y;
  node_dum.z=node0->z;

  node0->x = (1-cos(alpha)) * unit_axis->x * (unit_axis->x*node_dum.x + unit_axis->y*node_dum.y + unit_axis->z*node_dum.z) \
              + cos(alpha) * node_dum.x + sin(alpha) * (unit_axis->y*node_dum.z - unit_axis->z*node_dum.y);
  node0->y = (1-cos(alpha)) * unit_axis->y * (unit_axis->x*node_dum.x + unit_axis->y*node_dum.y + unit_axis->z*node_dum.z) \
              + cos(alpha) * node_dum.y + sin(alpha) * (unit_axis->z*node_dum.x - unit_axis->x*node_dum.z);
  node0->z = (1-cos(alpha)) * unit_axis->z * (unit_axis->x*node_dum.x + unit_axis->y*node_dum.y + unit_axis->z*node_dum.z) \
              + cos(alpha) * node_dum.z + sin(alpha) * (unit_axis->x*node_dum.y - unit_axis->y*node_dum.x);
  return;
}                  

/**************************************************************************/
/* Funktion: OmegaDisplaceNode
                                                                          */
/* Aufgabe:
   Verschiebt einen Punkt um eine Vektor
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   node0, vector
   R   node0 
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaDisplaceNode(NODE *node0, NODE *vec)
{
  node0->x = node0->x + vec->x;
  node0->y = node0->y + vec->y;
  node0->z = node0->z + vec->z;
  return;
} 

/**************************************************************************/
/* Funktion: OmegaDuplicateMesh2D
                                                                          */
/* Aufgabe:
   Dupliziert ein 2D-Netz
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh
   R   mesh_duplicated 
                                                                          */
/* Ergebnis:
   mesh_duplicated  
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

MESH2D *OmegaDuplicateMesh2D(MESH2D *mesh)
{
  long lv1,lv2;
  MESH2D *mesh_duplicated;
  mesh_duplicated       = (MESH2D *)    Malloc(sizeof(MESH2D)); 
  mesh_duplicated->node = (NODE *)      Malloc((mesh->no_of_nodes)*sizeof(NODE)); 
  mesh_duplicated->elem = (ELEMENT2D *) Malloc((mesh->no_of_elements)*sizeof(ELEMENT2D)); 
  mesh_duplicated->node_elem = NULL;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) {
    mesh_duplicated->node[lv1].x=mesh->node[lv1].x;
    mesh_duplicated->node[lv1].y=mesh->node[lv1].y;
    mesh_duplicated->node[lv1].z=mesh->node[lv1].z;
  }

  for(lv1=0;lv1<(mesh->no_of_elements);lv1++) {
    for(lv2=0;lv2<4;lv2++) {
      mesh_duplicated->elem[lv1].node[lv2]=mesh->elem[lv1].node[lv2];
    }
    mesh_duplicated->elem[lv1].material_group=mesh->elem[lv1].material_group;
  }
  mesh_duplicated->no_of_elements = mesh->no_of_elements;
  mesh_duplicated->no_of_nodes    = mesh->no_of_nodes;
  
  OmegaBuildNodeElRel2D(mesh_duplicated);
  return mesh_duplicated;
}


/**************************************************************************/
/* Funktion: OmegaDuplicateMesh3D
                                                                          */
/* Aufgabe:
   Dupliziert ein 3D-Netz
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh
   R   mesh_duplicated 
                                                                          */
/* Ergebnis:
   mesh_duplicated  
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

MESH3D *OmegaDuplicateMesh3D(MESH3D *mesh)
{
  long lv1,lv2;
  MESH3D *mesh_duplicated;
  mesh_duplicated       = (MESH3D *)    Malloc(sizeof(MESH3D)); 
  mesh_duplicated->node = (NODE *)      Malloc((mesh->no_of_nodes)*sizeof(NODE)); 
  mesh_duplicated->elem = (ELEMENT3D *) Malloc((mesh->no_of_elements)*sizeof(ELEMENT3D)); 
  mesh_duplicated->node_elem = NULL;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) {
    mesh_duplicated->node[lv1].x=mesh->node[lv1].x;
    mesh_duplicated->node[lv1].y=mesh->node[lv1].y;
    mesh_duplicated->node[lv1].z=mesh->node[lv1].z;
  }

  for(lv1=0;lv1<(mesh->no_of_elements);lv1++) {
    for(lv2=0;lv2<8;lv2++) {
      mesh_duplicated->elem[lv1].node[lv2]=mesh->elem[lv1].node[lv2];
    }
    mesh_duplicated->elem[lv1].material_group=mesh->elem[lv1].material_group;
  }  
  mesh_duplicated->no_of_elements = mesh->no_of_elements;
  mesh_duplicated->no_of_nodes    = mesh->no_of_nodes;
  
  OmegaBuildNodeElRel3D(mesh_duplicated);
  return mesh_duplicated;
}

/**************************************************************************/
/* Funktion: OmegaRotateMesh2D
                                                                          */
/* Aufgabe:
   Dreht ein 2D-Netz um eine Achse
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh, unit_axis, alpha
   R   mesh 
                                                                          */
/* Ergebnis:
   mesh2d wird um den Winkel alpha um die Achse unit_axis 
          (|unit_axis|=1) gedreht
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaRotateMesh2D(MESH2D *mesh, NODE *unit_axis, double alpha)
{
  long lv1;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) 
    OmegaRotateNode(&mesh->node[lv1], unit_axis, alpha);
  return;
}

/**************************************************************************/
/* Funktion: OmegaRotateMesh3D
                                                                          */
/* Aufgabe:
   Dreht ein 3D-Netz um eine Achse
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh, unit_axis, alpha
   R   mesh 
                                                                          */
/* Ergebnis:
   mesh3d wird um den Winkel alpha um die Achse unit_axis 
          (|unit_axis|=1) gedreht
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaRotateMesh3D(MESH3D *mesh, NODE *unit_axis, double alpha)
{
  long lv1;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) 
    OmegaRotateNode(&mesh->node[lv1], unit_axis, alpha);
  return;
}

/**************************************************************************/
/* Funktion: OmegaDisplaceMesh2D
                                                                          */
/* Aufgabe:
   Verschiebt 2D-Netz um eine Vektor
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh, vector
   R   mesh 
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaDisplaceMesh2D(MESH2D *mesh, NODE *vec)
{
  long lv1;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) 
    OmegaDisplaceNode(&mesh->node[lv1], vec);
  return;
}

/**************************************************************************/
/* Funktion: OmegaDisplaceMesh3D
                                                                          */
/* Aufgabe:
   Verschiebt ein 3D-Netz um eine Vektor
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh, vector
   R   mesh 
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   03/2002     M.Kohlmeier                                                */
/**************************************************************************/

void OmegaDisplaceMesh3D(MESH3D *mesh, NODE *vec)
{
  long lv1;

  for(lv1=0;lv1<mesh->no_of_nodes;lv1++) 
    OmegaDisplaceNode(&mesh->node[lv1], vec);
  return;
}

/**************************************************************************/
/* Funktion: OmegaNodeDistance
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   node0, node1
   R   double 
                                                                          */
/* Ergebnis:
   Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   10/2001               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

double OmegaNodeDistance(NODE *node0, NODE *node1)
{
  return sqrt((node1->x-node0->x)*(node1->x-node0->x)+(node1->y-node0->y)*(node1->y-node0->y)+(node1->z-node0->z)*(node1->z-node0->z));
}


/**************************************************************************/
/* Funktion: OmegaDelEqualNodes3D
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh3d
                                                                          */
/* Ergebnis:
   Lscht Knoten mit gleichen Koordinaten  
                                                                          */
/* Programmaenderungen:
   03/2002               M.Kohlmeier/G.Ungruh                             */
/**************************************************************************/

void OmegaDelEqualNodes3D(MESH3D *mesh) 
{
  const double float_error=0.000001;
  int lv1, lv2, lv3, lv4, lv5;    


  for(lv1=0;lv1<mesh->no_of_nodes-1;lv1++) 
  { 
    for(lv2=lv1+1;lv2<mesh->no_of_nodes;lv2++) 
    { 
      if (OmegaNodeDistance(&mesh->node[lv1],&mesh->node[lv2]) < float_error)
      {
        mesh->no_of_nodes--;
        for(lv3=lv2;lv3<mesh->no_of_nodes;lv3++)
        {
          mesh->node[lv3].x=mesh->node[lv3+1].x;
          mesh->node[lv3].y=mesh->node[lv3+1].y;
          mesh->node[lv3].z=mesh->node[lv3+1].z;
        }


        for(lv4=0;lv4<mesh->no_of_elements;lv4++)
        {
          for(lv5=0;lv5<8;lv5++)
          {
            if(mesh->elem[lv4].node[lv5]==lv2)
            {
              mesh->elem[lv4].node[lv5]=lv1;
            }
            if(mesh->elem[lv4].node[lv5]>lv2)
            {
              mesh->elem[lv4].node[lv5]--;
            }
          }
        }
      }
    }
  }
  return;
}



/**************************************************************************/
/* Funktion: OmegaMinNodeDistance2D
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh32
   R   double 
                                                                          */
/* Ergebnis:
   Minimaler Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   05/2002               M.Kohlmeier                                      */
/**************************************************************************/

void OmegaMinNodeDistance2D(MESH2D *mesh) 
{
  int lv1, lv2;    
  double min_dist;

  min_dist = OmegaNodeDistance(&mesh->node[0],&mesh->node[1]);

  for(lv1=0;lv1<mesh->no_of_nodes-1;lv1++) 
  { 
    for(lv2=lv1+1;lv2<mesh->no_of_nodes;lv2++) 
    { 
      if (min_dist > OmegaNodeDistance(&mesh->node[lv1],&mesh->node[lv2]))
       min_dist = OmegaNodeDistance(&mesh->node[lv1],&mesh->node[lv2]);
    }
  }
  DisplayMsgLn("");
  DisplayMsg("Minimaler Knotenabstand: ");
  DisplayDouble(min_dist,15,10);
  DisplayMsgLn("");
  return;
}

/**************************************************************************/
/* Funktion: OmegaMinNodeDistance3D
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh3d
   R   double 
                                                                          */
/* Ergebnis:
   Minimaler Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   01/2002               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

void OmegaMinNodeDistance3D(MESH3D *mesh) 
{
  int lv1, lv2;    
  double min_dist;

  min_dist = OmegaNodeDistance(&mesh->node[0],&mesh->node[1]);

  for(lv1=0;lv1<mesh->no_of_nodes-1;lv1++) 
  { 
    for(lv2=lv1+1;lv2<mesh->no_of_nodes;lv2++) 
    { 
      if (min_dist > OmegaNodeDistance(&mesh->node[lv1],&mesh->node[lv2]))
       min_dist = OmegaNodeDistance(&mesh->node[lv1],&mesh->node[lv2]);
    }
  }
  DisplayMsgLn("");
  DisplayMsg("Minimaler Knotenabstand: ");
  DisplayDouble(min_dist,15,10);
  DisplayMsgLn("");
  return;
}


/**************************************************************************/
/* Funktion: OmegaMinElSize2D
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh3d
   R   double 
                                                                          */
/* Ergebnis:
   Minimaler Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   05/2002               M.Kohlmeier                                      */
/**************************************************************************/

void OmegaMinElSize2D(MESH2D *mesh) 
{
  int lv1, lv2, lv3, nn = 4;    
  double min_el_size;

  min_el_size = OmegaNodeDistance(&mesh->node[mesh->elem[0].node[0]],&mesh->node[mesh->elem[0].node[1]]);

  for(lv1=0;lv1<mesh->no_of_elements;lv1++) {
    for(lv2=0;lv2<nn-1;lv2++) 
    { 
      for(lv3=lv2+1;lv3<nn;lv3++) 
      { 
        if ((min_el_size > OmegaNodeDistance(&mesh->node[mesh->elem[lv1].node[lv2]],&mesh->node[mesh->elem[lv1].node[lv3]]))
             && (mesh->elem[lv1].node[lv2] != mesh->elem[lv1].node[lv3]))
           min_el_size = OmegaNodeDistance(&mesh->node[mesh->elem[lv1].node[lv2]],&mesh->node[mesh->elem[lv1].node[lv3]]);
      }
    }
  }
  DisplayMsgLn("");
  DisplayMsg("Minimaler Knotenabstand im Element: ");
  DisplayDouble(min_el_size,15,10);
  DisplayMsgLn("");
  return;
}


/**************************************************************************/
/* Funktion: OmegaMinElSize3D
                                                                          */
/* Aufgabe:
   Ermittelt den Abstand zweier Knoten im Raum
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh3d
   R   double 
                                                                          */
/* Ergebnis:
   Minimaler Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   01/2002               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

void OmegaMinElSize3D(MESH3D *mesh) 
{
  int lv1, lv2, lv3, nn = 8;    
  double min_el_size;

  min_el_size = OmegaNodeDistance(&mesh->node[mesh->elem[0].node[0]],&mesh->node[mesh->elem[0].node[1]]);

  for(lv1=0;lv1<mesh->no_of_elements;lv1++) {
    for(lv2=0;lv2<nn-1;lv2++) 
    { 
      for(lv3=lv2+1;lv3<nn;lv3++) 
      { 
        if ((min_el_size > OmegaNodeDistance(&mesh->node[mesh->elem[lv1].node[lv2]],&mesh->node[mesh->elem[lv1].node[lv3]]))
             && (mesh->elem[lv1].node[lv2] != mesh->elem[lv1].node[lv3]))
           min_el_size = OmegaNodeDistance(&mesh->node[mesh->elem[lv1].node[lv2]],&mesh->node[mesh->elem[lv1].node[lv3]]);
      }
    }
  }
  DisplayMsgLn("");
  DisplayMsg("Minimaler Knotenabstand im Element: ");
  DisplayDouble(min_el_size,15,10);
  DisplayMsgLn("");
  return;
}


/**************************************************************************/
/* Funktion: OmegaOut_rfi2D/3D
                                                                          */
/* Aufgabe:
   Ausgabe in als rfi-Datei
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E   mesh3d
   R   double 
                                                                          */
/* Ergebnis:
   Minimaler Abstand der Knoten
                                                                          */
/* Programmaenderungen:
   01/2002               M.Kohlmeier/G.Ungruh                                        */
/**************************************************************************/

void OmegaOut_rfi2D (char *file_name, char *path_name, MESH2D *mesh)
{
  long i;
  FILE *datei;
  char dateiname[1024];

  if (!file_name) {
    if (path_name) {
      strcpy(dateiname, path_name);
      strcpy(dateiname, strcat(dateiname,"out2d.rfi"));
    }  else
    strcpy(dateiname, "out2d.rfi");
  }
  else {
    if (path_name) {
      strcpy(dateiname, path_name);
      strcpy(dateiname, strcat(dateiname,file_name));
    }  else
    strcpy(dateiname, file_name);
	}
 
	DisplayMsgLn("");
	DisplayMsgLn("Path and output file name: ");
	DisplayMsgLn(dateiname);
	DisplayMsgLn("");

  if ((datei = fopen(dateiname,"w")) == NULL) {
    printf("\n\n FEHLER BEIM �FNEN DER DATEI\n");
    exit(1);
  } 
    
  rewind(datei);

  fprintf(datei,"#0#0#0#1#0.0#0#################################################################\n");
  fprintf(datei,"0 %6li %6li \n", mesh->no_of_nodes, mesh->no_of_elements);

  
  for(i=0;i<mesh->no_of_nodes;i++){
                        
/* ------------------- Ausgabe der Koordinaten am Bildschirm -------------------------*/
    printf("Node:%7li, x:%f, y:%f, z:%f \n", i, mesh->node[i].x, mesh->node[i].y, mesh->node[i].z);
/* ------------- Ausgabe der Koordinaten in eine Datei -------------------------------*/                 
    fprintf(datei,"%7li %15.10f %15.10f %15.10f \n", i, mesh->node[i].x, mesh->node[i].y, mesh->node[i].z);
                                
  }
                             


  for(i=0;i<(mesh->no_of_elements);i++){
        printf("%li %li hex %3li %3li %3li %3li \n",i,mesh->elem[i].material_group,
              mesh->elem[i].node[0],
              mesh->elem[i].node[1],
              mesh->elem[i].node[2],
              mesh->elem[i].node[3]);
        fprintf(datei,"%li %li quad %3li %3li %3li %3li \n",i,mesh->elem[i].material_group,
              mesh->elem[i].node[0],
              mesh->elem[i].node[1],
              mesh->elem[i].node[2],
              mesh->elem[i].node[3]);
  }
  fclose(datei); 

}


void OmegaOut_rfi3D (char *file_name, char *path_name, MESH3D *mesh)
{
  long i;
  FILE *datei;
  char dateiname[1024];

  if (!file_name) {
    if (path_name) {
      strcpy(dateiname, path_name);
      strcpy(dateiname, strcat(dateiname,"out3d.rfi"));
    }  else
    strcpy(dateiname, "out3d.rfi");
  }
  else {
    if (path_name) {
      strcpy(dateiname, path_name);
      strcpy(dateiname, strcat(dateiname,file_name));
    }  else
    strcpy(dateiname, file_name);
	}

	DisplayMsgLn("");
	DisplayMsgLn("Path and output file name: ");
	DisplayMsgLn(dateiname);
	DisplayMsgLn("");

  if ((datei = fopen(dateiname,"w")) == NULL) {
    printf("\n\n FEHLER BEIM �FNEN DER DATEI\n");
    exit(1);
  } 
    
  rewind(datei);

  fprintf(datei,"#0#0#0#1#0.0#0#################################################################\n");
  fprintf(datei,"0 %6li %6li \n", mesh->no_of_nodes, mesh->no_of_elements);

  for(i=0;i<mesh->no_of_nodes;i++){
                        
/* ------------------- Ausgabe der Koordinaten am Bildschirm -------------------------*/
    printf("Node:%7li, x:%f, y:%f, z:%f \n", i, mesh->node[i].x, mesh->node[i].y, mesh->node[i].z);
/* ------------- Ausgabe der Koordinaten in eine Datei -------------------------------*/                 
    fprintf(datei,"%7li %15.10f %15.10f %15.10f \n", i, mesh->node[i].x, mesh->node[i].y, mesh->node[i].z);
                                
  }

  for(i=0;i<(mesh->no_of_elements);i++){
        printf("%li %li hex %3li %3li %3li %3li %3li %3li %3li %3li \n",i,mesh->elem[i].material_group,
              mesh->elem[i].node[0],
              mesh->elem[i].node[1],
              mesh->elem[i].node[2],
              mesh->elem[i].node[3],
              mesh->elem[i].node[4],
              mesh->elem[i].node[5],
              mesh->elem[i].node[6],
              mesh->elem[i].node[7]);
        fprintf(datei,"%li %li hex %3li %3li %3li %3li %3li %3li %3li %3li \n",i,mesh->elem[i].material_group,
              mesh->elem[i].node[0],
              mesh->elem[i].node[1],
              mesh->elem[i].node[2],
              mesh->elem[i].node[3],
              mesh->elem[i].node[4],
              mesh->elem[i].node[5],
              mesh->elem[i].node[6],
              mesh->elem[i].node[7]);
  }
  fclose(datei); 

}




/* mesh 2d_h_r_pl.c  **********************************************************/
/******************************************************************************/
/******************************************************************************/


/**************************************************************************

Geometrie fr Viertel-Scheibe mit Loch und Ring
05/99 Martin Kohlmeier    Erste Version 
06/99 MK           Kombinieren von Netzen 
06/99 MK           Randknoten
06/99 MK           Sortieren der Randknoten
06/99 MK           Aussortieren der Knoten mit einem angrenzenden Element (Eckknoten)
                   und weiterer vorgegebener Eckknoten
11/99 MK           Korrektur bei negativer Flaeche (Randknotenzuordnung)
02/00 MK           Loch-Diskretisierung
08/01 MK           Speichern der Knotendaten fuer mesh_3d
03/02 MK           Ring
**************************************************************************/

#define omega_max_num_mesh 10     /* MK_ToDo */
#define omega_max_num_el   2000
#define omega_max_num_node 2000

#define omega_max_sectors  20
#define omega_max_rings    20

//#define omega_max_num_mesh 10
//#define omega_max_num_el   5000
//#define omega_max_num_node 5000

//#define omega_max_sectors  40
//#define omega_max_rings    40
 
/* Ausgabesteuerung */

#define omega_pr_b_node 1
#define omega_pr_c_node 1

/* Diese Deklarationen werden wieder entfernt MK rf3819 */
void mesh_comb(int,int,int);
void node_el_relation(int);
void boundary_node(int);
void corner_node(int);

double node_x[omega_max_num_mesh][omega_max_num_node];
double node_y[omega_max_num_mesh][omega_max_num_node];

double fc_node_x[omega_max_num_node], fc_node_y[omega_max_num_node];
int num_fc_node;
int num_edge_node[omega_max_num_node];
int el_node[omega_max_num_mesh][omega_max_num_el][5];
int node_el[omega_max_num_mesh][omega_max_num_node][5];
int node_edge[omega_max_num_mesh][omega_max_num_node];
int b_node[omega_max_num_mesh][omega_max_num_node];     /* Randknoten */
int c_node[omega_max_num_mesh][omega_max_num_node];     /* Eckknoten  */
int num_node[omega_max_num_mesh];                 /* Anzahl Knoten  */
int num_el[omega_max_num_mesh];                  /* Anzahl Elemente */
int num_b_node[omega_max_num_mesh];            /* Anzahl Randknoten */
int num_c_node[omega_max_num_mesh];             /* Anzahl Eckknoten */
int dum1, dum0;
int AnzElem;
int AnzElx , AnzEly, ModAnzElx, Kn_nr, El_nr, AnzKn;  

double x_koord, y_koord;
double Randq2x[omega_max_sectors], Randq2y[omega_max_sectors]; 
double Randl2x[omega_max_sectors], Randl2y[omega_max_sectors]; 

/* Einige Geometrie-Funktionen: */  

double Laenge(double Vx,double Vy)
 {
 return sqrt(Vx*Vx+Vy*Vy);
 }

void node_el_rel(int mesh)
/* Ermittelet die Konoten-Element-Beziehungen fuer [mesh] */
{
  int lv2, lv3, lv;

  for(lv2=1;lv2<=num_node[mesh];lv2++) 
  { 
    for(lv3=1;lv3<=4;lv3++) 
    { 
      node_el[mesh][lv2][lv3]=0;
    }
  }
  for(lv2=1;lv2<=num_el[mesh];lv2++) 
  { 
    for(lv3=1;lv3<=4;lv3++) 
    { 
      lv=1;
      while(node_el[mesh][el_node[mesh][lv2][lv3]][lv]!=0) lv=lv+1;
      node_el[mesh][el_node[mesh][lv2][lv3]][lv]=lv2;
    }
  }
}


void boundary_node(int mesh)
/* Ermittelet die Randknoten von [mesh] */
  {
  int lv2; 
  int omepb = omega_pr_b_node; //WW for avoding the annoying .net warnings

  num_b_node[mesh]=0;
  for(lv2=1;lv2<=num_node[mesh];lv2++) 
    { 
    if (node_el[mesh][lv2][3]==0)
      {
      num_b_node[mesh]=num_b_node[mesh]+1;
      b_node[mesh][num_b_node[mesh]]=lv2;
      }
    }
    if (omepb)
    {
    printf(" *** Randknoten %d  ***\n",mesh );
    for(lv2=1;lv2<=num_b_node[mesh];lv2++) 
      { 
      printf(" %d    %d  %f  %f\n", lv2,b_node[mesh][lv2],
                     node_x[mesh][b_node[mesh][lv2]], node_y[mesh][b_node[mesh][lv2]]); 
      }
    }
  }   



void corner_node(int mesh)
/* Ermittelt die Eckknoten von [mesh], d.h.
   Knoten mit einem Nachbarelement + [num_fc_node] weitere Eckknoten [fc_node] */
 
  {
  int lv2,lv3;
  double floaterror;

  int omepb = omega_pr_c_node; //WW for avoding the annoying .net warnings

  floaterror = 0.00000001;
  num_c_node[mesh]=0;
  for(lv2=1;lv2<=num_node[mesh];lv2++) 
    { 
    if (node_el[mesh][lv2][2]==0)
      {
      num_c_node[mesh]=num_c_node[mesh]+1;
      c_node[mesh][num_c_node[mesh]]=lv2;
      }
    else 
      {
      for(lv3=1;lv3<=num_fc_node;lv3++) 
        {
        if (((node_x[mesh][lv2]>=(fc_node_x[lv3] - floaterror) ) && (node_x[mesh][lv2]<=(fc_node_x[lv3] + floaterror)) ) && ((node_y[mesh][lv2]>=(fc_node_y[lv3] - floaterror)) && (node_y[mesh][lv2]<=(fc_node_y[lv3] + floaterror))))   
          {
          num_c_node[mesh]=num_c_node[mesh]+1;
          c_node[mesh][num_c_node[mesh]]=lv2;  
          }
        }   
      } 
    }
  if (omepb)
    {
    printf(" *** Knoten mit einem Nachbarelement + weitere Eckknoten %d  ***\n",mesh ); 
    for(lv2=1;lv2<=num_c_node[mesh];lv2++) 
      { 
      printf(" %d   %f  %f\n", lv2,node_x[mesh][c_node[mesh][lv2]],node_y[mesh][c_node[mesh][lv2]]); 
      }
    }
  }



void mesh_comb(int mesh_old1, int mesh_old2, int mesh_new) 

/* Kombiniert die Netze [mesh_old1] und [mesh_old2] zu [mesh_new].
   Die Konten von [mesh_old2] bleiben dabei erhalten, doppelte Knoten
   werden gelscht. 
*/
      
{ 
  double comb_node_x[3][omega_max_num_node]; /*Achtung Dimension 3: [0][1][2]!!!*/
  double comb_node_y[3][omega_max_num_node];

  int comb_el_node[3][omega_max_num_el][5];
  int comb_node_el[3][omega_max_num_node][5];

  int comb_num_el[3], comb_num_node[3];

  int lv1, lv2, lv3, lv;
  int equal_node;

  double floaterror;
  floaterror = 0.00000001;


  /* Wertzuweisung in [1] und [2] neues Netz wird [2] mit seinen Knoten*/
  comb_num_node[1]=num_node[mesh_old1];
  comb_num_node[2]=num_node[mesh_old2];
  for(lv1=1;lv1<=comb_num_node[1];lv1++) 
  { 
    comb_node_x[1][lv1]=node_x[mesh_old1][lv1]; 
    comb_node_y[1][lv1]=node_y[mesh_old1][lv1]; 
  }
  for(lv1=1;lv1<=comb_num_node[2];lv1++) 
  { 
    comb_node_x[2][lv1]=node_x[mesh_old2][lv1]; 
    comb_node_y[2][lv1]=node_y[mesh_old2][lv1]; 
  }

  comb_num_el[1]=num_el[mesh_old1];
  comb_num_el[2]=num_el[mesh_old2];
  for(lv1=1;lv1<=comb_num_el[1];lv1++) 
  { 
    for(lv2=1;lv2<=4;lv2++) 
    { 
      comb_el_node[1][lv1][lv2]=el_node[mesh_old1][lv1][lv2]; 
    }
  }
  for(lv1=1;lv1<=comb_num_el[2];lv1++) 
  { 
    for(lv2=1;lv2<=4;lv2++) 
    { 
      comb_el_node[2][lv1][lv2]=el_node[mesh_old2][lv1][lv2]; 
    }
  }

  /* Knoten-Elementbeziehungen */

  for(lv1=1;lv1<=2;lv1++) 
  {
    for(lv2=1;lv2<=comb_num_node[lv1];lv2++) 
    { 
      for(lv3=1;lv3<=4;lv3++) 
      { 
      comb_node_el[lv1][lv2][lv3]=0;
      }
    }

    for(lv2=1;lv2<=comb_num_el[lv1];lv2++) 
    { 
      for(lv3=1;lv3<=4;lv3++) 
      { 
        lv=1;
        while(comb_node_el[lv1][comb_el_node[lv1][lv2][lv3]][lv]!=0) lv=lv+1;
        comb_node_el[lv1][comb_el_node[lv1][lv2][lv3]][lv]=lv2;
      }
    }
  }


  /* Suchen gleicher Knoten */

  for(lv1=1;lv1<=comb_num_node[1];lv1++) 
  { 
    equal_node=0;   
    for(lv2=1;lv2<=comb_num_node[2];lv2++) 
    { 
      if ( ((comb_node_x[1][lv1] > comb_node_x[2][lv2]-floaterror)&&(comb_node_x[1][lv1] < comb_node_x[2][lv2]+floaterror)) && ((comb_node_y[1][lv1] > comb_node_y[2][lv2]-floaterror)&&(comb_node_y[1][lv1] < comb_node_y[2][lv2]+floaterror)) )
        equal_node=lv2;     /* gleicher Knoten bekommt Nummer des neuen Netzes!!! */
    }
    if (equal_node != 0)
    {
      for(lv2=1;lv2<=4;lv2++) 
      { 
        if (comb_node_el[1][lv1][lv2] != 0) /* an den Knoten anliegende Elemente  */
        {                            /* lv1 Nummer des gleichen Knotens im alten Netz */
          for(lv3=1;lv3<=4;lv3++)    /* lv2 Nummer des des Element an der Ecke (1-4) */ 
          {         /* gleichen Knotens suchen und Nummer in -equal_node ersetzen!!! */
            if  (comb_el_node[1][comb_node_el[1][lv1][lv2]][lv3] == lv1) 
                 comb_el_node[1][comb_node_el[1][lv1][lv2]][lv3] = -equal_node ;
          }
        }


        comb_node_el[1][lv1][lv2] = 0; /* zu lschender Knoten hat kein Element mehr */
      }
    }
  }
  /* Suchen gleicher Knoten Ende */

  /* Knoten hinzufgen */

  lv=0;
  for(lv1=1;lv1<=comb_num_node[1];lv1++) 
  { 
    if (comb_node_el[1][lv1][1] != 0)  /* Knoten hat Element */
    {
      lv=lv+1;
      comb_node_x[2][lv+comb_num_node[2]]=comb_node_x[1][lv1]; 
      comb_node_y[2][lv+comb_num_node[2]]=comb_node_y[1][lv1]; 
    }
    else
    {
      for(lv2=1;lv2<=comb_num_el[1];lv2++) 
      { 
      for(lv3=1;lv3<=4;lv3++) 
        { 
        if (comb_el_node[1][lv2][lv3] > lv) comb_el_node[1][lv2][lv3]=comb_el_node[1][lv2][lv3]-1;
        }  
      }
    }
  }
  
  /* Elemente hinzufgen */

  for(lv1=1;lv1<=comb_num_el[1];lv1++) 
  { 
    for(lv2=1;lv2<=4;lv2++) 
    { 
      if (comb_el_node[1][lv1][lv2] < 0)
       comb_el_node[2][lv1+comb_num_el[2]][lv2]=-comb_el_node[1][lv1][lv2]; 
      else   
       comb_el_node[2][lv1+comb_num_el[2]][lv2]=comb_el_node[1][lv1][lv2]+comb_num_node[2]; 
    }
  }
  comb_num_el[2]=comb_num_el[2]+comb_num_el[1];
  comb_num_node[2]=comb_num_node[2]+lv; /*lv  von oben !!!*/




  /* �ergabe der Ergebnisse */

  num_node[mesh_new]=comb_num_node[2];
  for(lv1=1;lv1<=num_node[mesh_new];lv1++) 
  { 
    node_x[mesh_new][lv1]=comb_node_x[2][lv1]; 
    node_y[mesh_new][lv1]=comb_node_y[2][lv1]; 
  }

  num_el[mesh_new]=comb_num_el[2];
  for(lv1=1;lv1<=num_el[mesh_new];lv1++) 
  { 
    for(lv2=1;lv2<=4;lv2++) 
    { 
      el_node[mesh_new][lv1][lv2]=comb_el_node[2][lv1][lv2]; 
    }
  }
  return; 
} 




/*****************************************************************************/
/* Funktion OmegaHoleRingPlateQuarter   
   Aufgabe: Scheibe mit Loch umgeben von einem Ring 

   Formalparameter:
           E: material_group_circle, material_group_ring, material_group_plate
              plane: 0-xy, 1-xz, 2-yz, 
              part:  0-all 1-hole, 2-ring, 3-plate 
           E: x0,y0,x1,y1
           E: radius_circle, radius_ring
           E: no_rings_ring, no_rings_rect, no_sectors
              (no ring if  (no_rings_ring==0))
              (no plate if (no_rings_rect==0))
           E: sort_bound_nodes (0/1)
   Ergebnis:
           *MESH2D
   Aenderungen/Korrekturen:
      2000    Martin Kohlmeier        Erste Version                                       
   12/2001    Martin Kohlmeier                                                                      
   03/2002    Martin Kohlmeier        Ring                                   */
/*****************************************************************************/
                       
MESH2D *OmegaHoleRingPlateQuarter (long material_group_circle, long material_group_ring, long material_group_rect,
               long plane, long part,
               double x0, double y0, double x1, double y1, 
               double radius_circle, double radius_ring, 
               long no_rings_ring, long no_rings_rect, long no_sectors, 
               int sort_bound_nodes) 
{
  MESH2D *mesh;
  NODE circle_center;
  double bound_rect_x[omega_max_sectors], bound_rect_y[omega_max_sectors];   
  double SumB;
  double norm_radial_distance[omega_max_rings];
  double Pi, width, height;
  int El_nr;
  int lv, lv1, lv2, lv3, min_dist_node=0; 
  double min_dist=0.0, area=0.0;
  int first_c_node;
  double floaterror;
  circle_center.x = circle_center.y=circle_center.z=0.0;
  if ((no_rings_ring>omega_max_rings) || (no_rings_rect>omega_max_rings) || (no_sectors>omega_max_sectors)) {

      printf("Max_rings or max_sectors exceeded!!!"); 
      exit(0); 
  }   
  for (lv=0; lv < omega_max_num_mesh;lv++) {
    num_el[lv]   = 0;
    num_node[lv] = 0;
  }

  floaterror = 0.00000001;
  Pi = 4.0*atan(1.0); //3.1415926535897932384626433832795;	 
  width   = x1-x0;
  height  = y1-y0;
  
  /* Randkoordinaten oben und rechts fuer Loch links unten */
  for(lv=0;lv<=no_sectors;lv++)
  {
    if (lv <= (no_sectors/2)) 
      {
      bound_rect_x[lv] = width*lv/(no_sectors/2); 
      bound_rect_y[lv] = height;
      }
    else 
      {
      bound_rect_x[lv] = width;
      bound_rect_y[lv] = height*(no_sectors-lv)/(no_sectors/2); 
      }
  }

  /************* Platte bestimmen ******************************/

  if (no_rings_rect!=0)
  {
  /* relative Lage der Ringe in der Platte berechnen  */
    SumB=0.0;
    norm_radial_distance[0]=0.0;
    for(lv=1;lv<=no_rings_rect;lv++) 
    { 
      SumB += 1; /*  �uidistant  */
      norm_radial_distance[lv]=SumB; 
    } 	   

    Kn_nr = 0; 
    for(lv1=0;lv1<=no_rings_rect;lv1++)
    {
      for(lv2=0;lv2<=no_sectors;lv2++)
      {
        Kn_nr = Kn_nr + 1; 

        x_koord=(radius_ring + norm_radial_distance[lv1] / SumB * (Laenge(bound_rect_x[lv2],bound_rect_y[lv2])-radius_ring)) 
                 / Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_x[lv2];
        y_koord=(radius_ring + norm_radial_distance[lv1] / SumB * (Laenge(bound_rect_x[lv2],bound_rect_y[lv2])-radius_ring))
                 / Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_y[lv2];

        node_x[1][Kn_nr]=x_koord+x0; /* Anpassen der Lage */ 
        node_y[1][Kn_nr]=y_koord+y0; 

        num_node[1]=Kn_nr;
      } 	  
    } 	  

    El_nr = 0; 
    for(lv1=1;lv1<=no_rings_rect;lv1++)
    {
      for(lv2=1;lv2<=no_sectors;lv2++)
      {
        El_nr = El_nr + 1;
        el_node[1][El_nr][1] = lv2+(no_sectors+1)*(lv1-1);
        el_node[1][El_nr][2] = lv2+(no_sectors+1)*(lv1-1)+1;
        el_node[1][El_nr][3] = lv2+(no_sectors+1)*(lv1)+1;
        el_node[1][El_nr][4] = lv2+(no_sectors+1)*(lv1);
        num_el[1]=El_nr;
      } 	  
    } 	  
  }

  /************* Ring bestimmen ******************************/
  if (no_rings_ring!=0)
  {
  /* relative Lage der Ringe im Ring berechnen  */
    SumB=0.0;
    norm_radial_distance[0]=0.0;
    for(lv=1;lv<=no_rings_ring;lv++) 
    { 
      SumB += 1; /*  �uidistant  */
      norm_radial_distance[lv]=SumB; 
    } 	    

    Kn_nr = 0; 
    for(lv1=0;lv1<=no_rings_ring;lv1++)
    {
      for(lv2=0;lv2<=no_sectors;lv2++)
      {
        Kn_nr = Kn_nr + 1; 
  
        x_koord=(radius_circle + norm_radial_distance[lv1] / SumB * (radius_ring-radius_circle)) 
                 / Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_x[lv2];
        y_koord=(radius_circle + norm_radial_distance[lv1] / SumB * (radius_ring-radius_circle))
                 / Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_y[lv2];

        node_x[2][Kn_nr]=x_koord+x0; /* Anpassen der Lage */ 
        node_y[2][Kn_nr]=y_koord+y0; 

        num_node[2]=Kn_nr;
      } 	  
    } 	  

    El_nr = 0; 
    for(lv1=1;lv1<=no_rings_ring;lv1++)
    {
      for(lv2=1;lv2<=no_sectors;lv2++)
      {
        El_nr = El_nr + 1;
        el_node[2][El_nr][1] = lv2+(no_sectors+1)*(lv1-1);
        el_node[2][El_nr][2] = lv2+(no_sectors+1)*(lv1-1)+1;
        el_node[2][El_nr][3] = lv2+(no_sectors+1)*(lv1)+1;
        el_node[2][El_nr][4] = lv2+(no_sectors+1)*(lv1);
        num_el[2]=El_nr;
      }
    } 	  
  }


  /************* Kreisscheibe  ******************************/

  /* Rechteck */
  for(lv=0;lv<=no_sectors;lv++)
  {
    if (lv <= (no_sectors/2)) 
      {
      Randq2x[lv] = radius_circle*lv/(no_sectors);
      Randq2y[lv] = radius_circle/2.0;
      }
    else 
      {
      Randq2x[lv] = radius_circle/2.0;
      Randq2y[lv] = radius_circle*(no_sectors-lv)/(no_sectors); 
      }
  }

  Kn_nr = 0; 
  for(lv1=0;lv1<=no_sectors;lv1++)
  {
    if (lv1 <= ( no_sectors/2))
    for(lv2=0;lv2<=no_sectors;lv2++)
    {
      Kn_nr = Kn_nr + 1; 
      
      if (lv1==0)
    	{  
        x_koord=radius_circle/Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_x[lv2];
        y_koord=radius_circle/Laenge(bound_rect_x[lv2],bound_rect_y[lv2])*bound_rect_y[lv2];
        Randl2x[lv2]=x_koord;
        Randl2y[lv2]=y_koord;
      }
      if (lv1 > 0) 
    	{  
        x_koord=Randq2x[lv2]+(Randl2x[lv2]-Randq2x[lv2])*(no_sectors/2-lv1)/(no_sectors/2);
        y_koord=Randq2y[lv2]+(Randl2y[lv2]-Randq2y[lv2])*(no_sectors/2-lv1)/(no_sectors/2);
      }
     
      node_x[3][Kn_nr]=x_koord+x0; /* Anpassen der Lage */ 
      node_y[3][Kn_nr]=y_koord+y0; 
    } 	  

    if (lv1 > ( no_sectors/2))
    for(lv2=0;lv2 <=((no_sectors-lv1)*2);lv2++)
    {
      Kn_nr = Kn_nr + 1; 
      

      if (lv2 <= (no_sectors-lv1))
      {
        x_koord=Randq2x[lv2];
        y_koord=Randq2y[lv1];
      }
      else      
      {
        x_koord=Randq2x[no_sectors-lv1];
        y_koord=Randq2y[lv1+lv2-(no_sectors-lv1)];
      }
      
      node_x[3][Kn_nr]=x_koord+x0; /* Anpassen der Lage */ 
      node_y[3][Kn_nr]=y_koord+y0; 
    } 	  
  } 	  

  num_node[3]=Kn_nr;


  for(lv=3;lv<=3;lv++)
  {
    El_nr = 0; 
    dum1=0;
    dum0=0; 
    for(lv1=1;lv1<=no_sectors;lv1++)
    {
      if (lv1 <= ( no_sectors/2))
      for(lv2=1;lv2<=no_sectors;lv2++)
      {
        El_nr = El_nr + 1;
        el_node[lv][El_nr][1] = lv2+(no_sectors+1)*(lv1);
        el_node[lv][El_nr][2] = lv2+(no_sectors+1)*(lv1)+1;
        el_node[lv][El_nr][3] = lv2+(no_sectors+1)*(lv1-1)+1; 
        el_node[lv][El_nr][4] = lv2+(no_sectors+1)*(lv1-1); 

        num_el[lv]=El_nr;
      }
      if (lv1 > ( no_sectors/2))
	{        
        dum0=dum1;
        dum1=dum1+(no_sectors-lv1+1)*2-1+2;        
        for(lv2=1;lv2 <=((no_sectors-lv1+1)*2-1);lv2++)
          {
          if (lv2 < (no_sectors-lv1+1))
            { 
            El_nr = El_nr + 1;
            el_node[lv][El_nr][1] = lv2+(no_sectors+1)*( no_sectors/2)+dum1;
            el_node[lv][El_nr][2] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum1;
            el_node[lv][El_nr][3] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum0; 
            el_node[lv][El_nr][4] = lv2+(no_sectors+1)*( no_sectors/2)+dum0; 
            }
          if (lv2 == (no_sectors-lv1+1))   
	    { 
            El_nr = El_nr + 1;
            el_node[lv][El_nr][1] = lv2+(no_sectors+1)*( no_sectors/2)+dum1;
            el_node[lv][El_nr][2] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum0+1;
            el_node[lv][El_nr][3] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum0; 
            el_node[lv][El_nr][4] = lv2+(no_sectors+1)*( no_sectors/2)+dum0; 
            }
          if (lv2 > (no_sectors-lv1+1))
   	    { 
            El_nr = El_nr + 1;
            el_node[lv][El_nr][1] = lv2+(no_sectors+1)*( no_sectors/2)+dum1;
            el_node[lv][El_nr][2] = lv2+(no_sectors+1)*( no_sectors/2)+1+1+dum0;
            el_node[lv][El_nr][3] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum0; 
            el_node[lv][El_nr][4] = lv2+(no_sectors+1)*( no_sectors/2)+1+dum1-2; 
            }
          num_el[lv]=El_nr;
          }
       }
    } 	  
  } 	  

  node_el_rel(3);


  /* Koordinaten evtl. mit Randnummer */ 
  for(lv1=3;lv1<=3;lv1++) 
    { 
    printf(" *** Koordinaten evtl. mit Randnummer 3 ***\n"); 
    for(lv2=1;lv2<=num_node[lv1];lv2++) 
      { 
        printf("     %d     %f  %f\n", lv2,node_x[lv1][lv2],node_y[lv1][lv2]); 
      }
    }
   
  printf(" *** Elemente  3 **************************************\n"); 

  for(lv1=3;lv1<=3;lv1++) 
    {
    for(lv=1;lv<=num_el[lv1];lv++) 
      { 
      printf("     %d  %d  %d  %d  %d \n",lv, el_node[lv1][lv][1],el_node[lv1][lv][2],el_node[lv1][lv][3],el_node[lv1][lv][4]);
      }
    }

  printf(" *** Elemente 3 Excel **************************************\n"); 


  corner_node(3);


 /* Knoten-Elementbeziehungen */

  node_el_rel(1);
  node_el_rel(2);



/* Zusammenfgen der Netze [1],[2] (und [3]) in [4]*/
  switch (part) {
    case 0:
      mesh_comb(1, 2, 5);
      node_el_rel(5);
      mesh_comb(3, 5, 4);
    break;
    case 1:
      mesh_comb(0, 3, 4);
    break;
    case 2:
      mesh_comb(0, 2, 4);
    break;
    case 3:
      mesh_comb(0, 1, 4);
    break;
  }  


  /* Knoten-Elementbeziehungen */
  /* fuer Netz [4]*/
  node_el_rel(4);
  corner_node(4);

  num_fc_node=0;
    

  /* Eckknoten (Knoten mit einem Nachbarelement + weitere Eckknoten) */
  corner_node(4);

  num_node[5]=num_c_node[4];
  num_el[5]=0;
  for(lv2=1;lv2<=num_c_node[4];lv2++) 
    { 
    node_x[5][lv2]=node_x[4][c_node[4][lv2]];
    node_y[5][lv2]=node_y[4][c_node[4][lv2]];
    }

  mesh_comb(4, 5, 5);

  node_el_rel(5);

  first_c_node = 1;                           

  printf(" *** Randknoten sortieren beginnend bei Knoten: \n");

  printf(" %d   %f  %f\n", first_c_node,node_x[4][c_node[4][first_c_node]],node_y[4][c_node[4][first_c_node]]);

  /* Randknoten fr 5 */
  boundary_node(5);
 
  /* Randknoten beginnend mit first_c_node sortieren 5 in 6 */

  for(lv1=5;lv1<=5;lv1++) 
    {
    b_node[lv1+1][1] = 1; 
    num_b_node[lv1+1] = 1;
    num_node[lv1+1] = 1;
    node_x[lv1+1][num_node[lv1+1]]=node_x[lv1][first_c_node];   
    node_y[lv1+1][num_node[lv1+1]]=node_y[lv1][first_c_node];   
    b_node[lv1][first_c_node] = -1;   
    for(lv2=1;lv2<=(num_b_node[lv1]-1);lv2++) 
      { 
      min_dist = -1.0;
      for(lv3=1;lv3<=num_b_node[lv1];lv3++) 
        { 
        if (b_node[lv1][lv3] > 0) 
          {  /* Suche nach naechstliegendem Randknoten, der noch nicht sortiert ist. */
          if ((min_dist < 0.0) || (min_dist > Laenge(node_x[lv1+1][b_node[lv1+1][lv2]] - node_x[lv1][b_node[lv1][lv3]], node_y[lv1+1][b_node[lv1+1][lv2]] - node_y[lv1][b_node[lv1][lv3]])))  
	    {
            min_dist = Laenge(node_x[lv1+1][b_node[lv1+1][lv2]] - node_x[lv1][b_node[lv1][lv3]], node_y[lv1+1][b_node[lv1+1][lv2]] - node_y[lv1][b_node[lv1][lv3]]);  
            min_dist_node = lv3;
            }
          } 
        }
      num_b_node[lv1+1] = num_b_node[lv1+1] + 1;
      num_node[lv1+1]=num_node[lv1+1]+1;
      b_node[lv1+1][num_b_node[lv1+1]] = num_node[lv1+1];
      node_x[lv1+1][num_node[lv1+1]]=node_x[lv1][b_node[lv1][min_dist_node]];
      node_y[lv1+1][num_node[lv1+1]]=node_y[lv1][b_node[lv1][min_dist_node]];
      b_node[lv1][min_dist_node]=-1;
      }
    }
    
  printf(" Sortierte Randknoten 6 **************************************\n"); 
  for(lv1=6;lv1<=6;lv1++) 
    { 
    for(lv=1;lv<=num_b_node[lv1];lv++) 
      { 
      printf("%d  %f  %f \n",lv, node_x[lv1][lv],node_y[lv1][lv]); 
      }  	   
    } 

  printf(" *** Flaeche Randknoten 6 **************************************\n"); 
  for(lv1=6;lv1<=6;lv1++) 
    { 
    area = 0.0;
    for(lv=1;lv<=(num_b_node[lv1]-1);lv++) 
      { 
      area = area + 0.50 * (node_x[lv1][b_node[lv1][lv]] - node_x[lv1][b_node[lv1][lv+1]]) * (node_y[lv1][b_node[lv1][lv]] + node_y[lv1][b_node[lv1][lv+1]]);
      }  	    
    printf(" %f  \n",area);
    } 
   
  /* Eventuell Laufrichtung aendern */
  
  for(lv1=6;lv1<=6;lv1++) 
    { 
    num_node[(lv1+1)]=num_node[lv1];
    num_b_node[(lv1+1)]=num_b_node[lv1];
    /* Achtung 1. Knoten beleibt erster, letzter wird zweiter!!! */ 
    lv=1;
        b_node[lv1+1][lv]=b_node[lv1][lv];
        node_x[lv1+1][lv]=node_x[lv1][lv];
        node_y[lv1+1][lv]=node_y[lv1][lv];
    for(lv=2;lv<=num_b_node[lv1];lv++) 
      { 
      if (area < 0.0)
        {
         b_node[lv1+1][lv]=b_node[lv1][lv];
         node_x[lv1+1][lv]=node_x[lv1][num_b_node[lv1]-lv+2];
         node_y[lv1+1][lv]=node_y[lv1][num_b_node[lv1]-lv+2];
        }
      else
        {
        b_node[lv1+1][lv]=b_node[lv1][lv];
        node_x[lv1+1][lv]=node_x[lv1][lv];
        node_y[lv1+1][lv]=node_y[lv1][lv];
        }
      }  	   
    } 

  num_el[7]=0;
  mesh_comb(5, 7, 7);

  node_el_rel(7);

  corner_node(7);

  /* Anzahl Konten zwischen Eckknoten */
    
  for(lv1=7;lv1<=7;lv1++) 
    { 
    for(lv2=1;lv2<=(num_c_node[lv1]-1);lv2++) 
      { 
      num_edge_node[lv2] = c_node[lv1][lv2+1] - c_node[lv1][lv2] - 1;
      }       
    lv2 = num_c_node[lv1];

    /* Achtung: Es gibt keine letzten Eckknoten */
    num_edge_node[lv2] = num_b_node[lv1] - c_node[lv1][lv2];
   
    printf(" Anzahl Knoten zwischen Eckknoten 7 ****\n"); 
    for(lv2=1;lv2<=num_c_node[lv1];lv2++) 
      { 
      printf(" %d  %d\n", lv2, num_edge_node[lv2]); 
      }
    }

  num_node[8]=num_c_node[7];
  num_el[8]=0;
  for(lv2=1;lv2<=num_c_node[7];lv2++) 
    { 
    node_x[8][lv2]=node_x[7][c_node[7][lv2]];
    node_y[8][lv2]=node_y[7][c_node[7][lv2]];
    }

  mesh_comb(7, 8, 8);

  node_el_rel(8);

  boundary_node(8);
 
  corner_node(8);  
  /* Randnummer der Randknoten bestimmen */
    
  for(lv1=8;lv1<=8;lv1++) 
    { 
    for(lv3=1;lv3<=num_node[lv1];lv3++) 
      { 
      node_edge[lv1][lv3]=0;
      }
    lv = 0;
    for(lv2=1;lv2<=num_c_node[lv1];lv2++) 
      {
      lv = lv + 1;
      }        
 
    for(lv3=1;lv3<=num_c_node[lv1];lv3++) 
      { 
      for(lv2=1;lv2<=num_edge_node[lv3];lv2++) 
	{
        lv = lv + 1;
        node_edge[lv1][b_node[lv1][lv]]=lv3;
        }        
      }

    printf(" *** Randknoten mit Randnummer 8 ****\n"); 
    for(lv2=1;lv2<=num_b_node[lv1];lv2++) 
      { 
       printf(" %d %d  %f  %f\n", lv2,node_edge[lv1][b_node[lv1][lv2]],node_x[lv1][b_node[lv1][lv2]],node_y[lv1][b_node[lv1][lv2]]); 
      }
    }

  /* Koordinaten evtl. mit Randnummer */  
  for(lv1=8;lv1<=8;lv1++) 
    { 
    printf(" *** Koordinaten evtl. mit Randnummer 8 ***\n"); 
    for(lv2=1;lv2<=num_node[lv1];lv2++) 
      { 
      if (node_edge[lv1][lv2] != 0)
        {
        printf("     %d  %d  %f  %f\n", lv2,node_edge[lv1][lv2],node_x[lv1][lv2],node_y[lv1][lv2]); 
        }
      else 
	{
        printf("     %d     %f  %f\n", lv2,node_x[lv1][lv2],node_y[lv1][lv2]); 
        }
      }
    }
   
  printf(" *** Elemente  8 **************************************\n"); 

  for(lv1=8;lv1<=8;lv1++) 
    {
    for(lv=1;lv<=num_el[lv1];lv++) 
      { 
      printf("     %d  %d  %d  %d  %d \n",lv, el_node[lv1][lv][1],el_node[lv1][lv][2],el_node[lv1][lv][3],el_node[lv1][lv][4]);
      }
    }


/*** Speichern der Knotendaten fuer mesh_3d ***************************************/  

    if (sort_bound_nodes==1) lv1=8; 
    else 
    if (sort_bound_nodes==0) lv1=4;

    mesh       = (MESH2D *)    Malloc(sizeof(MESH2D)); 
    mesh->node = (NODE *)      Malloc(num_node[lv1]*sizeof(NODE)); 
    mesh->elem = (ELEMENT2D *) Malloc(num_el[lv1]*sizeof(ELEMENT2D)); 
    mesh->node_elem = NULL;
    
	printf(" *** Knotenliste fuer mesh_2d ************************************\n"); 
    mesh->no_of_nodes = num_node[lv1];
    for(lv2=0;lv2<(mesh->no_of_nodes);lv2++) { 
      switch (plane) {
        case 0: 
          mesh->node[lv2].x = node_x[lv1][lv2+1];
          mesh->node[lv2].y = node_y[lv1][lv2+1];
          mesh->node[lv2].z = 0.0;
        break;
        case 1: 
          mesh->node[lv2].x = node_x[lv1][lv2+1];
          mesh->node[lv2].y = 0.0;
          mesh->node[lv2].z = node_y[lv1][lv2+1];
        break;
        case 2: 
          mesh->node[lv2].x = 0.0;
          mesh->node[lv2].y = node_x[lv1][lv2+1];
          mesh->node[lv2].z = node_y[lv1][lv2+1];
        break;
      }
    }
    switch (plane) {
      case 0: 
        circle_center.x=x0;
        circle_center.y=y0;
        circle_center.z=0.0;
      break;
      case 1: 
        circle_center.x=x0;
        circle_center.y=0.0;
        circle_center.z=y0;
      break;
      case 2: 
        circle_center.x=0.0;
        circle_center.y=x0;
        circle_center.z=y0;
      break;
    }
  printf(" *** Elementliste fuer mesh_2d ***********************************\n"); 
    mesh->no_of_elements = num_el[lv1];
    for(lv2=0;lv2<(mesh->no_of_elements);lv2++) { 
      mesh->elem[lv2].node[0] = el_node[lv1][lv2+1][1]-1;
      mesh->elem[lv2].node[1] = el_node[lv1][lv2+1][2]-1;
      mesh->elem[lv2].node[2] = el_node[lv1][lv2+1][3]-1;
      mesh->elem[lv2].node[3] = el_node[lv1][lv2+1][4]-1;
      if (((OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[0]],&circle_center) + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[1]],&circle_center)  \
           + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[2]],&circle_center) + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[3]],&circle_center))/4.0) < radius_circle )
 	    mesh->elem[lv2].material_group = material_group_circle;
      else {
        if (((OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[0]],&circle_center) + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[1]],&circle_center)  \
           + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[2]],&circle_center) + OmegaNodeDistance(&mesh->node[mesh->elem[lv2].node[3]],&circle_center))/4.0) < radius_ring )
 	      mesh->elem[lv2].material_group = material_group_ring;
        else
 	  	  mesh->elem[lv2].material_group = material_group_rect;
      }
    }
	
  OmegaBuildNodeElRel2D(mesh);
  return mesh; 
}



/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/********** end. mg_lib.c ***********************************************
***************************************************************************/






/* Lesefunktion */


/**************************************************************************/
/* ROCKFLOW - Funktion: FctMeshGeneration
                                      */
/* Aufgabe:
   Liest die zu dem Schluesselwort MESH_GENERATION gehoerigen
   Daten und fuehrt die Netzerstellung aus.

                                      */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *data: Zeiger auf die aus der Datei eingelesenen Zeichen
   E int found: Schluesselwort gefunden: 1, sonst 0
   E FILE *f: Dateizeiger der Protokolldatei
                                      */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                      */
/* Programmaenderungen:
   04/2003    MK      Erste Version
                                      */
/**************************************************************************/
int FctMeshGeneration ( char *data,int found, FILE *f )
{
    int ok = 1;
    int pos = 0;
    int p = 0;
    int beginn;
    int p_sub = 0, unknown_subkeyword = 0;
    int p_mg, pos_mg;
    char *sub = NULL, *name = NULL;
    int ivalue;
    int ivalue1,ivalue2,ivalue3,ivalue4,ivalue5,ivalue6,ivalue7,ivalue8,ivalue9;
    double dvalue1,dvalue2,dvalue3,dvalue4,dvalue5,dvalue6;
    char *string;

    char mg_name[]="MESH_GENERATION";
    FILE *mg_file ;
    char mg_file_data_line[256];
    MESH_GENERATION *mg=NULL;
    MESH_GENERATION *mg_prior=NULL;


    long omega_mesh_max = 100;
    char *data_mg = NULL;
    char *mg_file_name = NULL;
    char *mg_path_name = NULL;
    long mg_type;
    int mesh_id = 0;
    int first_input_mesh_id=0;
    int second_input_mesh_id=0;
    NODE unit_axis, disp_vector;
    long i;
    MESH2D **mesh_2d;
    MESH3D **mesh_3d;




/* MG's-Name in der Namensliste einfuegen */
    declare_mesh_generation_name(mg_name);

    LineFeed(f);
    FilePrintString(f, "; 10 Miscellaneous -----------------------------------------------------------");
    LineFeed(f);

#ifdef TESTFILES
    DisplayMsgLn("Eingabedatenbank, Schluesselwort MESH_GENERATION");
#endif
#ifdef EXT_RFD
    FilePrintString(f,"*** C?.?.?  Optionale Netzerstellung ***"); LineFeed(f);
    LineFeed(f);
#endif
    if (!found) {   /* Schluesselwort nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f,"*** C?.?.?  Optionale Netzerstellung ***");
        LineFeed(f);
        FilePrintString(f,"; Schluesselwort: MESH_GENERATION (Optionale Netzerstellung)");
        LineFeed(f);
#endif
    }
    else {   /* Schluesselwort gefunden */
        name = (char *) Malloc(((int)strlen(data) + 2) * sizeof(char));
        sub = (char *) Malloc(((int)strlen(data) + 2) * sizeof(char));

        mesh_2d   = (MESH2D **)   Malloc(omega_mesh_max*sizeof(MESH2D *)); 
        mesh_3d   = (MESH3D **)   Malloc(omega_mesh_max*sizeof(MESH3D *)); 
        for (i=0;i<omega_mesh_max;i++) mesh_2d[i]=NULL;
        for (i=0;i<omega_mesh_max;i++) mesh_3d[i]=NULL;

        mg_path_name = (char *) Malloc(((int)strlen(GetPathRFDFile())+1) * sizeof(char));
        strcpy(mg_path_name,GetPathRFDFile());

        if(found==1) {
          char str_head_line1[256];
          char str_head_line2[256];

          strcpy(str_head_line1,"    ****       ");
          strcat(str_head_line1,MG_NAME);
          strcat(str_head_line1,"         ****");
          strcpy(str_head_line2,"    ****       ");
          strcat(str_head_line2,MG_VERSION);
          strcat(str_head_line2,"        --        ");
          strcat(str_head_line2,MG_DATE);
          strcat(str_head_line2,"         ****");
  
          DisplayMsgLn("");
          DisplayMsgLn("    ****************************************************************");
          DisplayMsgLn("    ****                                                        ****");
          DisplayMsgLn(str_head_line1);
          DisplayMsgLn("    ****                                                        ****");
          DisplayMsgLn(str_head_line2);
          DisplayMsgLn("    ****                                                        ****");
          DisplayMsgLn("    ****       ISEB  --  Institute for Fluid Mechanics          ****");
          DisplayMsgLn("    ****        and Computer Appl. in Civil Engineering         ****");
          DisplayMsgLn("    ****                                                        ****");
          DisplayMsgLn("    ****************************************************************");

          FilePrintString(f,"#MESH_GENERATION");
          LineFeed(f);
        }  /* if (found==1) */

        else if(found==2) {
            if(!MeshGenerationListEmpty()) {
                FilePrintString(f,"#MESH_GENERATION");
                LineFeed(f);
            }
            list_mg_init();
        }

#ifdef EXT_RFD
        FilePrintString(f,"; Schluesselwort: #MESH_GENERATION"); LineFeed(f);
        FilePrintString(f,"; Das Schluesselwort muss nicht vorhanden sein, es wird dann ohne"); LineFeed(f);
        FilePrintString(f,"; Netzgenerierung gearbeitet."); LineFeed(f);
#endif

        while (StrTestLong(&data[p+=pos])||(found==2)) {

            if(found==1) {
                mg=create_mesh_generation(mg_name);
                if(mg==NULL) break;
            }
            else if (found==2) {
                mg=(MESH_GENERATION *)get_list_mg_next();
                if(mg==mg_prior) return ok;
                mg_prior = mg;
            }

            if(mg==NULL) {
                break;
            }
            else {
                if(found==1) {
                    ok = (StrReadLong(&mg_type,&data[p],f,TFLong,&pos) && ok);
                    set_mg_type(mg,mg_type);
                }
                else if (found==2) {
                    fprintf(f," %ld ",get_mg_type(mg));
                }

                if (get_mg_type(mg)==0) {  /* Makro-Befehle aus Datei lesen */
                    if(found==1) {
                        StrReadString (&string,&data[p+=pos],f,TFString,&pos);
                        set_mg_file_name(mg, string);

                        mg_file_name=(char *)Free(mg_file_name);
                        mg_file_name = (char *) Malloc((int)strlen(mg_path_name)+(int)strlen(string)+1);
                        mg_file_name = strcat(strcpy(mg_file_name,mg_path_name),string);
                        string = (char *) Free(string);

                        if ((mg_file = fopen(mg_file_name,"r"))==NULL) {
                            DisplayErrorMsg("Error: #MESH_GENERATION: File not found !");
                            exit(1);
                        }
                        mg_file_name=(char *)Free(mg_file_name);

                        while (fgets(mg_file_data_line,256,mg_file)) {  /* Zeile einlesen */
                          /* Ende der Datei */
                          if (strstr(mg_file_data_line,"#STOP")) break;
                          if ((mg_file_data_line[0]!='*') &&
                             (mg_file_data_line[0]!='c')  &&   
                             (mg_file_data_line[0]!='C')  &&
                             (mg_file_data_line[0]!=';')  &&
                             (mg_file_data_line[0]!='\n') )  {
                            if (!data_mg) {
                              data_mg = (char *) Malloc(2* sizeof(char));
                              strcpy (data_mg," ");
                            } 
                            data_mg = (char *) Realloc(data_mg,((int)strlen(data_mg)+(int)strlen(mg_file_data_line)+2)* sizeof(char));
                            sscanf(mg_file_data_line,"%[^\n#*;]",mg_file_data_line); /* alles lesen bis * oder # oder ; oder \n */
                            data_mg = strcat(data_mg,mg_file_data_line);
                            
                            sub = (char *) Realloc(sub,((int)strlen(data_mg) + 2) * sizeof(char));
                            name = (char *) Realloc(name,((int)strlen(data_mg) + 2) * sizeof(char));
                          }
                        } 
                        LineFeed(f);


                        /* Alle Sub-Keywords lesen  */
                        unknown_subkeyword = 1;
                        p_mg = 0;
                        pos_mg = 0;
                        while (StrReadSubKeyword(sub, data_mg, p_mg += pos_mg, &beginn, &p_mg)) /* Schleife ueber Sub-Keywords */
                        {
                          /* Lesen des Sub-Keywords */
                          ok = StrReadStr(name, sub, f, TFString, &p_sub) && ok;

                          /* Auswerten des Sub-Keywords */

                          if (!strcmp(name, "$READ_MG_DATA")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);

                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
                              mg_file_name=(char *)Free(mg_file_name);
                              mg_file_name = (char *) Malloc((int)strlen(mg_path_name)+(int)strlen(string)+1);
                              mg_file_name = strcat(strcpy(mg_file_name,mg_path_name),string);
                              mg_file = fopen(mg_file_name,"r");
                              string = (char *) Free(string);
                              mg_file_name=(char *)Free(mg_file_name);
              
                              while (fgets(mg_file_data_line,256,mg_file)) {  /* Zeile einlesen */
                                /* Ende der Datei */
                                if (strstr(mg_file_data_line,"#STOP")) break;
                                if ((mg_file_data_line[0]!='*') &&
                                   (mg_file_data_line[0]!='c')  &&   
                                   (mg_file_data_line[0]!='C')  &&
                                   (mg_file_data_line[0]!=';')  &&
                                   (mg_file_data_line[0]!='\n') )  {
                                  data_mg = (char *) Realloc(data_mg,((int)strlen(data_mg)+(int)strlen(mg_file_data_line)+2)* sizeof(char));
                                  sscanf(mg_file_data_line,"%[^\n#*;]",mg_file_data_line); /* alles lesen bis * oder # oder ; oder \n */
                                  data_mg = strcat(data_mg,mg_file_data_line);
                                  sub = (char *) Realloc(sub,((int)strlen(data_mg) + 2) * sizeof(char));
                                }
                                name = (char *) Realloc(name,((int)strlen(data_mg) + 2) * sizeof(char));
                              } 
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $READ_MG_DATA */

                          if (!strcmp(name, "$MESH_ID")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadInt(&ivalue, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            mesh_id=ivalue;
            
                            LineFeed(f);
                            if (mesh_id >= omega_mesh_max) {
                              DisplayErrorMsg("Error !!! Max. MESH_ID exceeded.");
                              exit(1);
                            }
                          }  /* $MESH_ID */
              
                          if (!strcmp(name, "$FIRST_INPUT_MESH_ID")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadInt(&ivalue, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            first_input_mesh_id=ivalue;
              
                            LineFeed(f);
                          }  /* $FIRST_INPUT_MESH_ID */
              
                          if (!strcmp(name, "$SECOND_INPUT_MESH_ID")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadInt(&ivalue, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            second_input_mesh_id=ivalue;
              
                            LineFeed(f);
                          }  /* $SECOND_INPUT_MESH_ID */
              
                          if (!strcmp(name, "$RECTANGLE")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
                              mg_file_name=(char *)Free(mg_file_name);
                              mg_file_name = (char *) Malloc((int)strlen(mg_path_name)+(int)strlen(string)+1);
                              mg_file_name = strcat(strcpy(mg_file_name,mg_path_name),string);
                              string = (char *) Free(string);
                              ok = (StrReadInt(&ivalue, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                              mesh_2d[mesh_id]=OmegaRectangle(mg_file_name,ivalue);  
                              mg_file_name=(char *)Free(mg_file_name);
                              mesh_2d[mesh_id]=OmegaBuildNodeElRel2D(mesh_2d[mesh_id]);
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $RECTANGLE */
              
                          if (!strcmp(name, "$EXPAND_2D_TO_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
                              mg_file_name=(char *)Free(mg_file_name);
                              mg_file_name = (char *) Malloc((int)strlen(mg_path_name)+(int)strlen(string)+1);
                              mg_file_name = strcat(strcpy(mg_file_name,mg_path_name),string);
                              string = (char *) Free(string);
                              ok = (StrReadInt(&ivalue, &sub[p_sub +=pos_mg], f, TFInt, &pos_mg) && ok);
                              mesh_3d[mesh_id]=OmegaExpand2DTo3D(mg_file_name,ivalue,mesh_2d[mesh_id]);   
                              mg_file_name=(char *)Free(mg_file_name);
                              mesh_3d[mesh_id]=OmegaBuildNodeElRel3D(mesh_3d[mesh_id]);
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $EXPAND_2D_TO_3D */
              
              
           
                          if (!strcmp(name, "$BUILD_NODE_EL_REL_2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            mesh_2d[mesh_id]=OmegaBuildNodeElRel2D(mesh_2d[mesh_id]);
               
                            LineFeed(f);
                          }  /* $BUILD_NODE_EL_REL_2D */
              
              
                          if (!strcmp(name, "$BUILD_NODE_EL_REL_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            mesh_3d[mesh_id]=OmegaBuildNodeElRel3D(mesh_3d[mesh_id]);
               
                            LineFeed(f);
                          }  /* $BUILD_NODE_EL_REL_3D */
              
              
                          if (!strcmp(name, "$HOLE_RING_PLATE_QUARTER")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadInt(&ivalue1, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue2, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue3, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue4, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue5, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue1, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue2, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue3, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue4, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue5, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue6, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
              
                            ok = (StrReadInt(&ivalue6, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue7, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue8, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                            ok = (StrReadInt(&ivalue9, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
               
                            mesh_2d[mesh_id]=OmegaHoleRingPlateQuarter (ivalue1,ivalue2,ivalue3, ivalue4,ivalue5, dvalue1,dvalue2, dvalue3,dvalue4, dvalue5, dvalue6, ivalue6,ivalue7,ivalue8, ivalue9);
                            LineFeed(f);
                          }  /* $HOLE_RING_PLATE_QUARTER */
              
              
                          if (!strcmp(name, "$ROTATE_2D_TO_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
                              ok = (StrReadInt(&ivalue, &sub[p_sub += pos_mg], f, TFInt, &pos_mg) && ok);
                              mesh_3d[mesh_id]=OmegaRotate2DTo3D(string, mg_path_name, ivalue,mesh_2d[first_input_mesh_id]);      
                              string = (char *) Free(string);
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $ROTATE_2D_TO_3D */
              
              
                          if (!strcmp(name, "$DISPLACE_MESH2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadDouble(&dvalue1, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue2, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue3, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
              
                            disp_vector.x=dvalue1;
                            disp_vector.y=dvalue2;
                            disp_vector.z=dvalue3;
                            OmegaDisplaceMesh2D (mesh_2d[mesh_id], &disp_vector);
                            LineFeed(f);
                          }  /* $DISPLACE_MESH2D */
              
              
                          if (!strcmp(name, "$DISPLACE_MESH3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadDouble(&dvalue1, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue2, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue3, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);

                            disp_vector.x=dvalue1;
                            disp_vector.y=dvalue2;
                            disp_vector.z=dvalue3;
                            OmegaDisplaceMesh3D (mesh_3d[mesh_id], &disp_vector);
                            LineFeed(f);
                          }  /* $DISPLACE_MESH3D */
              
              
              
                          if (!strcmp(name, "$ROTATE_MESH2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadDouble(&dvalue1, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue2, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue3, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue4, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
              
                            unit_axis.x=dvalue1;
                            unit_axis.y=dvalue2;
                            unit_axis.z=dvalue3;
                            OmegaRotateMesh2D (mesh_2d[mesh_id], &unit_axis, dvalue4);
                            LineFeed(f);
                          }  /* $ROTATE_MESH2D */


                          if (!strcmp(name, "$ROTATE_MESH3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            ok = (StrReadDouble(&dvalue1, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue2, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue3, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
                            ok = (StrReadDouble(&dvalue4, &sub[p_sub += pos_mg], f, TFDouble, &pos_mg) && ok);
              
                            unit_axis.x=dvalue1;
                            unit_axis.y=dvalue2;
                            unit_axis.z=dvalue3;
                            OmegaRotateMesh3D (mesh_3d[mesh_id], &unit_axis, dvalue4);
                            LineFeed(f);
                          }  /* $ROTATE_MESH3D */
              
              
                          if (!strcmp(name, "$DEL_EQUAL_NODES_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            OmegaDelEqualNodes3D (mesh_3d[mesh_id]);
                            mesh_3d[mesh_id]=OmegaBuildNodeElRel3D(mesh_3d[mesh_id]);
                            LineFeed(f);
                          }  /* $DEL_EQUAL_NODES_3D */
              
              
                          if (!strcmp(name, "$DUPLICATE_MESH_2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
                            
                            mesh_2d[mesh_id]=OmegaDuplicateMesh2D (mesh_2d[first_input_mesh_id]);
                            LineFeed(f);
                          }  /* $DUPLICATE_MESH_2D */
              
                          if (!strcmp(name, "$DUPLICATE_MESH_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            mesh_3d[mesh_id]=OmegaDuplicateMesh3D (mesh_3d[first_input_mesh_id]);
                            LineFeed(f);
                          }  /* $DUPLICATE_MESH_3D */
              
              
                          if (!strcmp(name, "$MESH_COMBINE_2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            mesh_2d[mesh_id]=OmegaMeshCombine2D (mesh_2d[first_input_mesh_id],mesh_2d[second_input_mesh_id]);
                            mesh_2d[mesh_id]=OmegaBuildNodeElRel2D(mesh_2d[mesh_id]);
                            LineFeed(f);
                          }  /* $MESH_COMBINE_2D */
              
                          if (!strcmp(name, "$MESH_COMBINE_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            mesh_3d[mesh_id]=OmegaMeshCombine3D (mesh_3d[first_input_mesh_id],mesh_3d[second_input_mesh_id]);
                            mesh_3d[mesh_id]=OmegaBuildNodeElRel3D(mesh_3d[mesh_id]);
                            LineFeed(f);
                          }  /* $MESH_COMBINE_3D */
              
              
                          if (!strcmp(name, "$MIN_NODE_DISTANCE_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            OmegaMinNodeDistance3D (mesh_3d[mesh_id]);
                            LineFeed(f);
                          }  /* $MIN_NODE_DISTANCE_3D */
              
              
                          if (!strcmp(name, "$MIN_EL_SIZE_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            OmegaMinElSize3D (mesh_3d[mesh_id]);
                            LineFeed(f);
                          }  /* $MIN_EL_SIZE_3D */
              
              
              
              
                         if (!strcmp(name, "$OUTPUT_RFI_2D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
             	                OmegaOut_rfi2D (string, mg_path_name, mesh_2d[mesh_id]);
                              string = (char *) Free(string);
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $OUTPUT_RFI_2D */
                         
                          if (!strcmp(name, "$OUTPUT_RFI_3D")) {
                            unknown_subkeyword = 0;
                            LineFeed(f);
              
                            if (StrTestString(&sub[p_sub])) {
                              StrReadString(&string, &sub[p_sub], f, TFString, &pos_mg);
             	                OmegaOut_rfi3D (string, mg_path_name, mesh_3d[mesh_id]);
                              string = (char *) Free(string);
                            }
                            else {
                              ok = 0;
                              break;
                            }
                            LineFeed(f);
                          }  /* $OUTPUT_RFI_3D */
                         
              
                          /* Auswerten des Sub-Keywords beendet */
              
                    		  if ((unknown_subkeyword) && (found == 1)) {
                            FilePrintString(f, "* !!! Error: Unknown Subkeyword! $");
                            FilePrintString(f, name);
                            LineFeed(f);
                            DisplayMsgLn("* !!! Keyword #MESH_GENERATION");
                            DisplayMsg("* !!! Error: Unknown Subkeyword! ");
                            DisplayMsgLn(name);
                            exit(1);
                          }
                          pos_mg = 0; /* weitere Suche nach Sub-Keyword erfolgt direkt nach letztem gefundenen */
                        } /* Schleife ueber Sub-Keywords ENDE */

                        data_mg = (char *)Free(data_mg);               

                    }
                    else if (found==2) {
                        fprintf(f, " %s ",get_mg_file_name(mg));
                        LineFeed(f);
                    }
                }  /* Makro-Befehle aus Datei lesen */
                if (mg) insert_mesh_generation_list (mg);
            } /* else */
            LineFeed(f);
        } /* while */

        name = (char *) Free(name);
        sub = (char *) Free(sub);
        mg_path_name = (char *) Free(mg_path_name);


        for (i=0;i<omega_mesh_max;i++) {
          if (mesh_2d[i]) {
            if (mesh_2d[i]->node)      mesh_2d[i]->node = (NODE *) Free(mesh_2d[i]->node); 
            if (mesh_2d[i]->elem)      mesh_2d[i]->elem = (ELEMENT2D *) Free(mesh_2d[i]->elem); 
            if (mesh_2d[i]->node_elem) mesh_2d[i]->node_elem = (NODEELEMENTS2D *) Free(mesh_2d[i]->node_elem); 
            mesh_2d[i] = (MESH2D *) Free(mesh_2d[i]);     
          } 
        } 

        for (i=0;i<omega_mesh_max;i++) {
          if (mesh_3d[i]) {
            if (mesh_3d[i]->node)      mesh_3d[i]->node = (NODE *) Free(mesh_3d[i]->node); 
            if (mesh_3d[i]->elem)      mesh_3d[i]->elem = (ELEMENT3D *) Free(mesh_3d[i]->elem); 
            if (mesh_3d[i]->node_elem) mesh_3d[i]->node_elem = (NODEELEMENTS3D *) Free(mesh_3d[i]->node_elem); 
            mesh_3d[i] = (MESH3D *) Free(mesh_3d[i]);     
          } 
        }
        mesh_2d = (MESH2D **) Free(mesh_2d);
        mesh_3d = (MESH3D **) Free(mesh_3d);
    }
  return ok;
}






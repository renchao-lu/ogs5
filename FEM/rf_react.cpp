/*

	rf_react.cpp

Reaction package to go with MTM2

*/

#include "stdafx.h" /* MFC */
#include <signal.h>
#include "display.h"
#include "makros.h"
#include "rf_pcs.h"
#include "nodes.h"
#include "rf_pcs.h"
#include "mathlib.h"
#include "cel_mtm2.h"
//#include "rfmat_tp.h"
#include "rfmat_cp.h"
#include "rf_react.h"
#include "stdio.h"
#include "geo_strings.h"
#include "rf_pcs.h" //OK_MOD"
#include "files.h"
#include "nodes.h"
#include "elements.h"           /* für ElGetElementGroupNumber */
#include "edges.h"              /* für GetEdge */
#include <iostream>
#include <fstream>
#include "rf_tim_new.h"

#include <vector>
using namespace std;

void destroy_react(void *item);


#define PHREEQC
int NR_INORG_COMP_EQU;
int NR_MIN_EQU;
extern char *file_name;
extern char *crdat; /* MX */
//REACTION_MODEL *rcml=NULL;

vector <REACT*> REACT_vec;

/**************************************************************************
   ROCKFLOW - Funktion: ExecuteReactions

   Aufgabe:
   Calculates chemical reactions between components
   first Version
   
   Programmaenderungen:
   08/2003     SB         Erste Version
**************************************************************************/
void REACT::ExecuteReactions(void){

	if(this->flag_pqc) this->ExecuteReactionsPHREEQC();
//	if(use_chem_flag == 2) ExecuteReactionsV2();

}






/**************************************************************************
   ROCKFLOW - Funktion: ExecuteReactionsPHREEQC

   Aufgabe:
   Berechnet chemische Reaktionen zwischen den einzelnen Komponenten
   allererste VErsion
   
   Programmaenderungen:
   06/2003     SB         Erste Version
   06/2003     MX         Read and Reaction functions
   09/2003     SB         Included temperature
   11/2003     SB         included time step fro PHEEQC kinetics
						  bugfix for many species (large files)
						  adapted to faster output file setup

**************************************************************************/
void REACT::ExecuteReactionsPHREEQC(void){
 
 long i,  ok = 0;
// CRFProcess *m_pcs = NULL;
// static REACTION_MODEL *rcml;
 FILE *indatei, *fphinp, *fsel_out=NULL;
 char fsout[80];


 DisplayMsgLn("ExecuteReactionsPHREEQC:");

 /* Initialize arrays of concentrations and array for reaction rates (list, in pretimeloop)*/
 GetTransportResults();
 
 /* Perform reaction step */
 /* --------------------------------------------------------------------------*/
  if(flag_pqc){
    	indatei=fopen(crdat,"r");
	/* do only at first timestep */
    if (indatei != NULL){
//		if (aktueller_zeitschritt<2){
//		rcml = CreateReactionModel();
		ok= this->ReadReactionModel(indatei);
//		}
    }
    else {
      printf("ERROR: Can not open file *.pqc");
      exit(0);
    }

 /* Read the input file (*.pqc) and set up the input file for PHREEQC ("phinp.dat")*/
	/* set input file name */
	strcpy(fsout, this->outfile);

	fphinp = fopen("phinp.dat", "w");
    if ((fphinp) && ok){
      for(i=0;i<this->nodenumber;i++){
		if(this->rateflag[i] > 0){
			rewind(indatei);
			ok = ReadInputPhreeqc(i,indatei, fphinp);
		}
      }
      fclose(indatei);
      fclose(fphinp);
    }
	else{
		DisplayMsgLn("The file phinput.dat could not be opened !");
		exit(0);
	}

	/* Extern Program call to PHREEQC */
    if(ok) ok = Call_Phreeqc();
	if(ok ==0) exit(0);
 

   /* Set up the output values for rockflow after Phreeqc reaction*/
    fsel_out=fopen(fsout,"r");
    if ((ok) && !fsel_out){
        DisplayMsgLn("The selected output file doesn't exist!!!");
        exit(0);
    }
    else if(ok){
		ok = ReadOutputPhreeqc(fsout);
      if(!ok) DisplayMsgLn(" Error in call to PHREEQC !!!");
      fclose(fsel_out);
	}
 } /* if flag */
 
 /* Calculate Rates */
 CalculateReactionRates();
 
 SetConcentrationResults();

/* determine where to calculate the chemistry */
// CalculateReactionRateFlag();

/* pH and pe constant or variable */
// ResetpHpe(rc, rcml);
 
/* test output*/
/*
 for(comp=0; comp<this->number_of_comp;comp++){
	 DisplayMsg("component : "); DisplayLong(comp);DisplayMsg(", name = "); DisplayMsg(this->name[comp]);DisplayMsgLn(". ");
	 DisplayMsgLn("val_in: ");
	 for(i=0;i<this->nodenumber;i++){	DisplayDouble(this->val_in[comp][i],0,0); DisplayMsg(", ");}
	 DisplayMsgLn(" ");
	 DisplayMsgLn("val_out: ");
	 for(i=0;i<this->nodenumber;i++){	DisplayDouble(this->val_out[comp][i],0,0); DisplayMsg(", ");}
	 DisplayMsgLn(" ");
	
	 DisplayMsgLn("rate : ");
	 for(i=0;i<this->nodenumber;i++){	DisplayDouble(this->rate[comp][i],0,0); DisplayMsg(", ");}
	 DisplayMsgLn(" ");
 }
 	 DisplayMsgLn("rateflag : ");
	 for(i=0;i<this->nodenumber;i++){	DisplayDouble((double) this->rateflag[i],0,0); DisplayMsg(", ");}
	 DisplayMsgLn(" ");
*/

}/* End of ExecuteReactionsPHREEQC */


/**************************************************************************/
/* Constructor */
REACT::REACT(void){
	name = NULL;
	val_in=NULL;
	val_out=NULL;
	rate=NULL;
	rateflag = NULL;
	heatflag = 0;
	nodenumber = 0;
	flag_pqc = false;

  rcml_number_of_master_species=0;
  rcml_number_of_equi_phases=0;
  rcml_number_of_ion_exchanges=0;
  rcml_number_of_kinetics=0;
  rcml_pH_flag=1;
  rcml_pe_flag=1;
  rcml_heat_flag = 0;
  rcml_number_of_pqcsteps = 1; //standard = 1;
  outfile = NULL;
	}
/* Destructor */
REACT::~REACT(void){
}
/**************************************************************************
   ROCKFLOW - Funktion: CreateREACT

   Aufgabe:
   Stellt Datenstruktur für Ratenkommunikation zwischen Raecations and MTM2
   zur verfügung
   
   Programmaenderungen:
   06/2003     SB         Erste Version
**************************************************************************/
void REACT::CreateREACT(void){

int  comp, i, vector_size, np=0;
//REACT * rc = NULL;
CRFProcess *m_pcs = NULL;

//rc = new REACT;
vector_size = (int) pcs_vector.size();
for(i=0;i<vector_size;i++){
	m_pcs = pcs_vector[i];
	if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0) np++;
	if(m_pcs->pcs_type_name.compare("HEAT_TRANSPORT")==0) heatflag = 1;
	nodenumber = (long) m_pcs->m_msh->GetNodesNumber(false);
 }
 number_of_comp = np;

 np = np + heatflag; // allocate one for temperature
 // allocate memory for the concentration arrays 

 name     = (char **)   Malloc(sizeof(char *)   * np);
 val_in   = (double **) Malloc(sizeof(double *) * np);
 val_out  = (double **) Malloc(sizeof(double *) * np);
 rate     = (double **) Malloc(sizeof(double *) * np);
 rateflag = (int *) Malloc(sizeof(int) * nodenumber);
 
 for(comp=0; comp<np;comp++){
	val_in[comp]  = (double *) Malloc(sizeof(double) * nodenumber);
	val_out[comp] = (double *) Malloc(sizeof(double) * nodenumber);
	rate[comp]    = (double *) Malloc(sizeof(double) * nodenumber);
	name[comp]    = (char *)   Malloc(sizeof(char)  * 80);
 }

// REACT_vec.push_back(rc);	
}


/**************************************************************************
   ROCKFLOW - Funktion: DestroyREACT

   Aufgabe:
   Zerstört Datenstructur REACT
   
   Programmaenderungen:
   06/2003     SB         Erste Version
**************************************************************************/
void DestroyREACT(void)
{
 REACT* rc = NULL;
 rc=rc->GetREACT();
 /* Free memeory for concentration arrays */
 if(rc != NULL){
	rc->val_in   = (double **)Free(rc->val_in);
	rc->val_out  = (double **)Free(rc->val_in);
	rc->rate     = (double **)Free(rc->val_in);
	rc->name     = (char **)  Free(rc->name);
	rc->rateflag = (int *) Free(rc->rateflag);
	rc = (REACT *) Free(rc);
 }
}


/**************************************************************************
   ROCKFLOW - Funktion: REACT::InitREACT

   Aufgabe:
   Inserts reaction rate of component at index in field RECT->rate
   
   Programmaenderungen:
   06/2003     SB         Erste Version
**************************************************************************/
void REACT::InitREACT(void){

 int comp, phase, j, timelevel, np;
 long i;
 CRFProcess *m_pcs = NULL;
// rc = rc->GetREACT();

 timelevel = 1;  // concentrations are in new timelevel
 phase = 0;      //single phase so far
 
 /* Initialize arrays of concentrations and array for reaction rates */
 np = (int)pcs_vector.size();
 for(j=0;j<np;j++){ // for all processes
	m_pcs = pcs_vector[j];
	if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0){ // if it is a mass transport process
		comp = m_pcs->pcs_component_number; // get component number
		this->name[comp] = (char *) cp_vec[comp]->compname.data(); // get component name
		for(i=0;i<this->nodenumber;i++){
			// get concentration values
			this->val_in[comp][i] = m_pcs->GetNodeValue(i,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0])+timelevel);
			if((this->val_in[comp][i] < 0.0) && (strcmp(this->name[comp],"pe")!= 0) ) {  //MX, 01.2005
				if(abs(this->val_in[comp][i]) > MKleinsteZahl) {
					DisplayMsg(" Neg. conc for component "); DisplayLong((long) comp);
					DisplayMsg(" at node "); DisplayLong((long)i);
					DisplayMsg("; conc = "); DisplayDouble(this->val_in[comp][i],0,0);
				DisplayMsgLn(" ");
				}
				this->val_in[comp][i] = 0.0 * this->val_in[comp][i];
			}
		this->val_out[comp][i] = this->val_in[comp][i];
		this->rate [comp][i] = 0.0;
		}
	}
 }


 for(i=0;i<this->nodenumber;i++)
	this->rateflag[i] = 1;
 this->countsteps = 50;

 // Get Temperature values
 if(this->heatflag > 0){
	this->name[np-2] = "temp";//MX CMCD
    m_pcs = PCSGet("HEAT_TRANSPORT");
    int index = m_pcs->GetNodeValueIndex("TEMPERATURE1");
	for(i=0;i<this->nodenumber;i++)
		this->val_in[np-2][i] = m_pcs->GetNodeValue(i, index); //OK PCSGetNODTemperature1L(i)//MX CMCD -2, Liquid Flow, Heat Transport
  }	
}


/**************************************************************************
   ROCKFLOW - Funktion: CalculateReactionRates

   Aufgabe:
   Calculates the reaction rates by: rate =  (val_out-val_in)/dt
   
   Programmaenderungen:
   06/2003     SB         Erste Version
**************************************************************************/
void REACT::CalculateReactionRates(void)
{
/* Calculate Rates */
 
 int comp, i;
 double teta, help=0.0, maxi, dc, delta_t;

  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcCharacteristicNumbers: no time discretization data !" << endl;
  delta_t = m_tim->CalcTimeStep();
//OK_TIM delta_t = GetDt(aktueller_zeitschritt-1l);
 teta = 0.0; /* teta = 0; concentration before reactions */
 for(comp=0; comp<this->number_of_comp; comp++){
	maxi = 0.0;
	for(i=0;i<this->nodenumber;i++){
		maxi = max(maxi, this->val_out[comp][i]);
		this->rate[comp][i] = (this->val_out[comp][i] - this->val_in[comp][i])/delta_t;

		help = (teta*this->val_out[comp][i] + (1.0-teta)*this->val_in[comp][i]);
		if(help < MKleinsteZahl) {       // umgekehrt wichten
			help = (1.0-teta)*this->val_out[comp][i] + teta*this->val_in[comp][i];
			if(help < MKleinsteZahl) 
				this->rate[comp][i] = 0.0;
			else
				this->rate[comp][i] = this->rate[comp][i] /help;
		}
		else
			this->rate[comp][i] = this->rate[comp][i] /help;

		
	}


/* Reaction - Number dC */
	for(i=0;i<this->nodenumber;i++){
		if(fabs(maxi)< MKleinsteZahl) 
			dc = 0;
		else
			dc = this->rate[comp][i]*delta_t/maxi;
//SB:todo		MTM2SetElementDamkohlerNumNew(i,0,comp,dc);
	}
 } /* end for(comp=... */
}












/**************************************************************************
   ROCKFLOW - Funktion: SetConcentrationResults

   Aufgabe:
   Save concentrations after reaction in concentration array
   
   Programmaenderungen:
   06/2003     SB         Erste Version
**************************************************************************/
void REACT::SetConcentrationResults(void)
{
/*  */
 int comp, timelevel, np, idx;
 long i;
 string name;
 CRFProcess *m_pcs = NULL;

 //Fix Process here ??
 //Rücksrache mit Sebastian

 timelevel = 1;  // concentrations are in new timelevel
 np = (int)pcs_vector.size();
/*  SB4218
 for(comp=0; comp<this->number_of_comp;comp++){
	 for(i=0;i<this->nodenumber;i++){
		name = this->name[comp];
		idx = m_pcs->GetNodeValueIndex(name)+1;
		m_pcs->SetNodeValue(i, idx,this->val_out[comp][i]);
	 }
 }
 */
for(comp=0; comp<this->number_of_comp;comp++){
	name = this->name[comp];
    m_pcs = PCSGet("MASS_TRANSPORT",name); //???
	idx = m_pcs->GetNodeValueIndex(name)+1;
	for(i=0;i<this->nodenumber;i++)
			m_pcs->SetNodeValue(i, idx,this->val_out[comp][i]);
}

}

REACT* REACT::GetREACT(void)
{
  REACT *rc = NULL;
  /* Tests */
  if(REACT_vec.capacity() > 0)
	rc = REACT_vec[0];
  return rc;
}


/**************************************************************************
   ROCKFLOW - Funktion: ReadReactionModel

   Aufgabe:
   Liest die Eingabedatei *.pqc und geben die Model-values zurück
                                                                          
   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   R REACTION_MODEL *rcml:Zeiger des REACTION_MODELs
   E FILE *File :Eigabedatei
                                                                          
   Ergebnis:
   0 bei Fehler oder Ende aufgrund Dateitest, sonst 1
                                                                       
   Programmaenderungen:
   06/2003     MX         Erste Version
**************************************************************************/
int REACT::ReadReactionModel( FILE *File){

  int n_master_species=0;
  int n_equi_phases=0;
  int n_ion_exchange=0;
  int beginn=0, p=0;
  int pos;
  int nj;
  char str[256],*sub, *sub1;

  FILE *indatei=NULL;
  indatei=File;

  CRFProcess* m_pcs = NULL;
  int no_processes = (int)pcs_vector.size();

//  crdat = (char *) Malloc((int)(int)strlen(file_name));
//  crdat = strcat(strcpy(crdat,file_name),CHEM_REACTION_EXTENSION);  /*MX:0603*/
//  indatei=fopen(crdat, "r");


/* Open input file  and read the reaction model values*/
  if(indatei == NULL){           /*input dateien does not exist*/
    DisplayMsgLn("");
    DisplayMsgLn(" The input file *.pqc does not exist!!!");
    exit(0);
  } /*end if*/

  /* zeilenweise lesen */
   while(fgets(str,256,indatei)){
 //   DisplayMsgLn(str);


/* Schleife ueber Keyword Solution */
/*-------------------------------------------------------------------------------------*/
    pos=0;
    beginn=1;
    p=0;
//    while ((!strstr(str, "END")) && (!strstr(str, "#ende")) && StrOnlyReadStr(sub, str, f, TFString, &p)) {
    while ((!strstr(str, "END")) && (!strstr(str, "#ende")) && StringReadStr(&sub, str, &p)) {
      if (strcmp(sub, "SOLUTION")==0) {
        while(fgets(str,256,indatei) && (!strstr(str,"#ende"))){
          if (strstr(str,"# comp")){
             StringReadStr(&sub, str, &p);
             if (!strcmp(sub, "pH")==0 && !strcmp(sub, "pe")==0 ){
                n_master_species +=1;
             }
          }
		  if (strstr(str,"# temp")) 
			 for(int i=0;i<no_processes;i++){
				m_pcs = pcs_vector[i];
				if(m_pcs->pcs_type_name.find("HEAT_TRANSPORT")!=string::npos){
//			  if(GetRFProcessProcessingAndActivation("HT")) 
					this->rcml_heat_flag=1;
					break;}
			}
        }
		this->rcml_number_of_master_species = n_master_species;
      }
      else if (strcmp(sub, "EQUILIBRIUM_PHASES")==0) {
        while(fgets(str,256,indatei) && (!strstr(str,"#ende"))){
          if (strstr(str,"# comp")){
             StringReadStr(&sub, str, &p);
            n_equi_phases +=1;
          }
        }
		this->rcml_number_of_equi_phases = n_equi_phases;
       } /*end if*/
      else if (strcmp(sub, "EXCHANGE")==0) {
        while(fgets(str,256,indatei) && (!strstr(str,"#ende"))){
          if (strstr(str,"# comp")){
             StringReadStr(&sub, str, &p);
             n_ion_exchange +=1;
          }
        }
		this->rcml_number_of_ion_exchanges = n_ion_exchange;
       } /*end if*/
//SB:neu
	     else if (strcmp(sub, "SELECTED_OUTPUT")==0) {
           while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
				if (strstr(str,"-file")){
					DisplayMsgLn("-file in *.pqc found");
					p=0;
					StringReadStr(&sub,str,&p);
					StringReadStr(&sub1,&str[p],&p);
					this->outfile = sub1;
				}
              }
            }

       else{
         while(fgets(str,256,indatei) && (!strstr(str,"#ende"))){}
         break;
 //        return 1;
       }
     }/*end while 2*/
   } /*end while 1*/
    nj=rcml_number_of_master_species+rcml_number_of_equi_phases+rcml_number_of_ion_exchanges;
	if (nj+2!= this->number_of_comp){
      DisplayMsgLn("!!!Error:Number of components in file *.pqc is not equal to that in file *.rfd!");
//      fclose(indatei);
//      return 0; 
    }
//    fclose(indatei);
    return 1;
}


/**************************************************************************
   ROCKFLOW - Funktion: ReadInputPhreeqc

   Aufgabe:
   Liest die Eingabedatei *.pqc und erstellt aktuelles PHREEQC-Eingabefile
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler oder Ende aufgrund Dateitest, sonst 1
                                                                       
   Programmaenderungen:
   06/2003     SB         Erste Version 
   06/2003     MX         Read function realisation
**************************************************************************/
int REACT::ReadInputPhreeqc(long index, FILE *fpqc, FILE *Fphinp){

  int i=0, np;
  int nn=0,nep=0, nj=0, nk=0;
  int beginn=0, p=0, found;
  int pH_flag=-1, pe_flag=-1, iheat;
  int pos,j;
  double dvalue, d_help;
  char str[256],sub[256], s[256];
  char *s1, *s2;

  s1 = s2 = NULL;
  FILE *indatei=NULL, *f;
  np = this->number_of_comp;
 // crdat = (char *) Malloc((int)(int)strlen(file_name));
 // crdat = strcat(strcpy(crdat,file_name),CHEM_REACTION_EXTENSION);  /*MX:0603*/

/* Open input and output file */
 // indatei = fopen(crdat, "r");   /*input dateien*/
  indatei = fpqc;   /*input dateien*/
  f=Fphinp;                       /*output dateien*/


  if(indatei == NULL){
    DisplayMsgLn("Erro:The input file *.pqc doesn't exist!!!");
    return 0;
  }

  /* zeilenweise lesen */
  while(fgets(str,256,indatei)){
//    DisplayMsgLn("");
//    DisplayMsgLn(str);

    pos=0;
    beginn=1;
    p=0;
    while ((!strstr(str, "END")) && (!strstr(str, "#ende")) && StrOnlyReadStr(sub, str, f, TFString, &p)) {
    LineFeed(f);
    found=0;

/* Schleife ueber Keyword Solution */
/*-------------------------------------------------------------------------------------*/
         if (!strcmp(sub, "SOLUTION")) {
            found=1;
            FilePrintString(f, "SOLUTION  ");
            FilePrintInt(f, index+1);
            LineFeed(f);

            FilePrintString(f, "#GRID  ");
            FilePrintInt(f, index+1);
            LineFeed(f);

           
 //           while (fgets(str,256,indatei) && (!StrTestDollar(str, &pos))){
           while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
              if ((!strstr(str, "# comp")) && (!strstr(str, "# temp"))){
//               DisplayMsgLn(" #ende and # not found"); 
               FilePrintString(f, str);

              }
              else{
//                DisplayMsgLn(" # comp found ");
                sscanf("%s", str);
                StrReadStr(s, str, f, TFString, &pos);

				/* SB: temperature introduced */
				if(strcmp(s,"temp")==0){
					if(rcml_heat_flag == 1){
						iheat = np;
//                        printf("Temperature is %lf", val_in[iheat][index]);              
						/* convert temperature from Kelvin to degree celsius for PHREEQC */
						if (val_in[iheat][index]<273.0) val_in[iheat][index] += 273.15; //MX 03.05
                        FilePrintDouble(f, val_in[iheat][index] - 273.15);
                        FilePrintString(f, " # temp ");
                        LineFeed(f);
						}
					/* else write line to file as read in */
						else{
							FilePrintString(f, str);
						};


				};
				/* SB: end temperature */

     /* find the concentration of each component from val_in*/
                for (j=0;j<np;j++){
                  if (strcmp(s,name[j])==0){
                    if(strcmp("pH",name[j])!=0){
                        nn += 1;
						d_help = val_in[j][index];
//                        printf("Component %s: concentration %lf", s, d_help);              
						if(fabs(d_help) < 1.0e-019) d_help = 0.0;
                        FilePrintDouble(f, d_help);
                        FilePrintString(f, " # comp ");
                        FilePrintInt(f, j+1);
                        LineFeed(f);
                        if (strcmp("pe",name[j])==0)
                            pe_flag=0;
                        break;
					}
/*		                        else{  // pH in RF input file 
                        pH_flag=0;
                        printf("Component %s: concentration %lf", s, val_in[j][index]);              
                        FilePrintDouble(f, val_in[j][index]);
                        FilePrintString(f, " charge ");
                        FilePrintString(f, " # comp ");
                        FilePrintInt(f, j+1);
                        LineFeed(f);
                        break;
                     }
*/
                   }

                   else if (strcmp(s,"pH")==0){ /* if pH will change with the reaction! */
                     for (i=0;i<np;i++){
                       if (strcmp(s,name[i])==0){
                         if(strcmp("pH",name[i])==0){         /* pH in RF input file */
                                           
                           pH_flag=0;
//                           printf("Component %s: concentration %lf", s, val_in[i][index]);              
                           FilePrintDouble(f, val_in[i][index]);
//MX						   FilePrintString(f, " charge ");

                           if (strstr(str, "charge")) FilePrintString(f, " charge ");
                           FilePrintString(f, " # comp ");
                           FilePrintInt(f, rcml_number_of_master_species+1);
                           LineFeed(f);
                           break;
                         } 
                       }
                      } /*end for*/
                      
                     if (pH_flag<0){ 

                       DisplayMsgLn("pH is not included in the transport but will be calculated in the reaction"); 
                       pH_flag=1;             
                       p=0;
                       StrReadDouble(&dvalue, &str[p+=pos], f, TFDouble, &pos);
                       StrReadStr(s, &str[p+=pos], f, TFString, &pos);
                       FilePrintString(f, " # comp "); 
                       FilePrintInt(f, rcml_number_of_master_species+1);
                       LineFeed(f);
                       name[rcml_number_of_master_species+1]="pH";
                       break;}
                     break;
                   } 

                   else if (strcmp(s,"pe")==0){  /* if pe will change with the reaction! */
                     for (i=0;i<np;i++){
                       if (strcmp(s,name[i])==0){
                         if(strcmp("pe",name[i])==0){         /* pe in RF input file */
                           pe_flag=0;
//                           printf("Component %s: concentration %lf", s, val_in[i][index]);              
                           FilePrintDouble(f, val_in[i][index]);
                           FilePrintString(f, " # comp ");
                           FilePrintInt(f, rcml_number_of_master_species+2);
                           LineFeed(f);
                           break;
                         }
                       }
                      } /*end for*/
                      
                     if (pe_flag<0){ 
                       DisplayMsgLn("pe is not included in the transport but will be calculated in the reaction");              
                       pe_flag=1;
                       StrReadDouble(&dvalue, &str[pos], f, TFDouble, &pos);
                       FilePrintString(f, " # comp ");
                       FilePrintInt(f, rcml_number_of_master_species+2);
                       LineFeed(f);
                       break;}
                     break;
                   }
                                 
                }/*end-for*/
            } /*end-else*/
        } /*end while -3*/
        if (nn != rcml_number_of_master_species) {
           FilePrintString(f, "Warnung:Concentration of master species not found in *.rfd file!!!");
        }
        pos=0;
        beginn=1;
        p=0;
  }/*if_SOLUTION*/

/* Schleife ueber Keyword SOLUTION_SPECIES */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "SOLUTION_SPECIES")) {
            found=1;
            FilePrintString(f, "SOLUTION_SPECIES  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
               FilePrintString(f, str);
              }
          }

/* Schleife ueber Keyword SOLUTION_MASTER_SPECIES */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "SOLUTION_MASTER_SPECIES")) {
            found=1;
            FilePrintString(f, "SOLUTION_MASTER_SPECIES  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
               FilePrintString(f, str);
              }
          }


/* Schleife ueber Keyword EQUILIBRIUM_PHASES */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "EQUILIBRIUM_PHASES")) {
            found=1;
            FilePrintString(f, "EQUILIBRIUM_PHASES  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
                 pos=0;
                 p=0;
                 StrReadStr(s, &str[p+=pos], f, TFString, &pos);
				 if (strcmp(s,"")==0) break;
                 StrReadDouble(&dvalue, &str[pos], f, TFDouble, &pos);

     /* find the mass of each phase from val_in*/
                 for (i=0;i<np;i++){
                    if (strcmp(s,name[i])==0){
                        nep += 1;
//                        printf("  Phase %s:  %lf \n", s, val_in[i][index]);              
                        FilePrintDouble(f, val_in[i][index]);
                        FilePrintString(f, " # comp ");
                        FilePrintInt(f, i+1);
                        LineFeed(f);
                        break;
                    }
                  } /*end for*/
              }/*end while*/
                if (nep != rcml_number_of_equi_phases) {
                    FilePrintString(f, "Warning: One or more solid phase(s) in EQUILIBRIUM_PHASES not found in *.rfd file!!!");
//                    FilePrintString(f, "Value(s) in *.pqc file used!!!");
//                    FilePrintString(f, str);
                }
          } /*end if*/

/* Schleife ueber Keyword KINETICS */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "KINETICS")) {
            found=1;
            FilePrintString(f, "KINETICS  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
				if(strstr(str,"-steps")){
				/* zerlegen und RockFlow timestep einfügen */
				  sscanf(str," %s %lf %s %i %s", s1,&dvalue,s,&j,s2); ///OK
				  /* steps in reaction model eintragen */
				  this->rcml_number_of_pqcsteps = j;
				  
				  if(index > -1){
				  FilePrintString(f, " -steps ");
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcCharacteristicNumbers: no time discretization data !" << endl;
	              FilePrintDouble(f,m_tim->CalcTimeStep());
//OK_TIM	              FilePrintDouble(f, GetDt(aktueller_zeitschritt-1l));
				  FilePrintString(f, " in ");
				  FilePrintInt(f, j);
				  FilePrintString(f, " steps ");
	              LineFeed(f);
				  }

				}
				else
               FilePrintString(f, str);
            }
       }

/* Schleife ueber Keyword RATES */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "RATES")) {
            found=1;
            FilePrintString(f, "RATES  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
               FilePrintString(f, str);
            }
       }

/*  Keyword PHASE */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "PHASE")) {
            found=1;
            FilePrintString(f, "PHASE  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
               FilePrintString(f, str);
            }
       }

/*  Keyword EXCHANGE */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "EXCHANGE")) {
            found=1;
            FilePrintString(f, "EXCHANGE  ");
            FilePrintInt(f, index+1);
            LineFeed(f);
            while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
                 pos=0;
                 p=0;
                 StrReadStr(s, &str[p+=pos], f, TFString, &pos);

     /* find the mass of each phase from val_in*/
                 for (i=0;i<np;i++){
                    if (strcmp(s,name[i])==0){
                        nj += 1;
//                        printf("  Phase %s:  %lf \n", s, val_in[i][index]);              
                        FilePrintDouble(f, val_in[i][index]);
                        FilePrintString(f, " # comp ");
                        FilePrintInt(f, i+1);
                        LineFeed(f);
                        break;
                    }
                  } /*end for*/
              }/*end while*/
                if (nj != rcml_number_of_ion_exchanges) {
                    FilePrintString(f, "Warning: Problem in reading EXCHANGE  in *.rfd or *.pqc file!!!");
//                    FilePrintString(f, "Value(s) in *.pqc file used!!!");
//                    FilePrintString(f, str);
                }
          } /*end if*/


 /*  Keyword PRINT */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "PRINT")) {
            found=1;
            if (index==0){
              FilePrintString(f, "PRINT  ");
              LineFeed(f);
              while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
                 FilePrintString(f, str);
              }
            }
            else
              while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
              }
        }
 /*  Keyword SELECTED_OUTPUT */
/*-------------------------------------------------------------------------------------*/
        else if (!strcmp(sub, "SELECTED_OUTPUT")) {
            found=1;
            if (index==0){
            FilePrintString(f, "SELECTED_OUTPUT  ");
            LineFeed(f);
              while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
                FilePrintString(f, str);
                p=0;
                StrOnlyReadStr(sub, str, f, TFString, &p);
                if (!strcmp(sub, "-file")) {
                    StrOnlyReadStr(sub, &str[p], f, TFString, &p);
/* SB: moved to structure rcml
                    strcpy(fsout, sub);
*/
                }
              }
            }
            else
              while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
              }
       }

 /*  Keyword USER_PUNCH */
/*-------------------------------------------------------------------------------------*/
       else if (!strcmp(sub, "USER_PUNCH")) {
            found=1;
            if (index==0){
                FilePrintString(f, "USER_PUNCH  ");
                LineFeed(f);
        
    /*------------head------------------------------*/
                fprintf(f, " -head");
                for (j=0; j<rcml_number_of_master_species; j++){
                    if((strcmp("pH",name[j])!=0) &&(strcmp("pe",name[j])!=0))
                    fprintf(f, " %s", name[j]);
                }
                fprintf(f, " pH pe");
                
                nj=rcml_number_of_master_species;
                nk=rcml_number_of_equi_phases;

                for (j=nj+2; j<nj+rcml_number_of_equi_phases+2; j++){
                    fprintf(f, " %s", name[j]);
                }

                for (j=nj+nk+2; j<nj+nk+rcml_number_of_ion_exchanges+2; j++){
                    fprintf(f, " %s", name[j]);
                }
                LineFeed(f);

   /*------------master speciese---------------------*/
                fprintf(f, " 10 PUNCH");
                for (j=0; j<nj-1; j++){
                    if((strcmp("pH",name[j])!=0) && (strcmp("pe",name[j])!=0))
                    fprintf(f, " TOT(%c%s%c),", 34, name[j], 34);
                }
                    fprintf(f, " TOT(%c%s%c)", 34, name[nj-1], 34);

 
    /*------------pH pe-------------------------------*/
               LineFeed(f);
                fprintf(f, " 20 PUNCH");
                fprintf(f, " -LA(%c%s%c),", 34, "H+", 34);
                fprintf(f, " -LA(%c%s%c)", 34, "e-", 34);

 
    /*------------equilibrium phases-------------------*/
               if (rcml_number_of_equi_phases>0){ 
                  LineFeed(f);
                  fprintf(f, " 40 PUNCH");
                  for (j=nj+2; j<nj+rcml_number_of_equi_phases+1; j++){
                      fprintf(f, " EQUI(%c%s%c),", 34, name[j], 34);
                  }
                      fprintf(f, " EQUI(%c%s%c),", 34, name[nj+rcml_number_of_equi_phases+1], 34);
                }

    /*------------exchange-------------------*/
               if (rcml_number_of_ion_exchanges>0){ 
                  LineFeed(f);
                  fprintf(f, " 60 PUNCH");
                  for (j=nj+nk+2; j<nj+nk+rcml_number_of_ion_exchanges+1; j++){
                      fprintf(f, " MOL(%c%s%c),", 34, name[j], 34);
                  }
                      fprintf(f, " MOL(%c%s%c),", 34, name[nj+nk+rcml_number_of_ion_exchanges+1], 34);
                }
                while (fgets(str,256,indatei) && (!strstr(str, "#ende"))){
//                     FilePrintString(f, str);
                }
            }
            else
                while (fgets(str,256,indatei) && ((!strstr(str, "#ende"))||(!strstr(str, "END")))){
                }
          } /* end if_USER_PUNCH */
          /* SB: added: time discretisation: Keyword is "-steps" */
	      else if (!strcmp(sub, "-steps")) {
			  found = 1;
			  sscanf(str," %s %lf %s %i %s", s1, &dvalue,s,&j,s2); ///OK
			  if(index > -1){
			  FilePrintString(f, " -steps ");
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else
    cout << "Error in MPCCalcCharacteristicNumbers: no time discretization data !" << endl;
	              FilePrintDouble(f,m_tim->CalcTimeStep());
//OK_TIM              FilePrintDouble(f, GetDt(aktueller_zeitschritt-1l));
              FilePrintString(f, " in ");
			  FilePrintInt(f, j);
			  FilePrintString(f, " steps ");
              LineFeed(f);
			  }
		  }
			/* Not found */
          else {
              if (found !=1){
              fprintf(f, " %s %s ", str, " in the *.pqc file is an unknown keyword!!!");
              exit(0);}
          }
       }/* end while - 2 */
    } /* end while - 1 */

    LineFeed(f);
    fprintf(f, "END");
    LineFeed(f);
    LineFeed(f);
//    if (indatei !=NULL) fclose(indatei);

    return 1;
}   /* end-if */


/**************************************************************************
   ROCKFLOW - Funktion: Call_Phreeqc

   Aufgabe:
   Ruft PHREEQC auf

   Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          
   Ergebnis:
   0 bei Fehler oder Ende aufgrund Dateitest, sonst 1

   
   Programmaenderungen:
   06/2003     SB         Erste Version
 **************************************************************************/

int REACT::Call_Phreeqc(void){

  char *m_phreeqc;
//  m_phreeqc="phrqc phinp.dat  phinp.out  phreeqc.dat";
   m_phreeqc="phreeqc phinp.dat  phinp.out  phreeqc.dat";

#ifdef PHREEQC
  if (!system(m_phreeqc)){
//    DisplayMsgLn("Phreeqc runs succesfully! ");
	return 1;
  }
   else {
    DisplayMsgLn("Warnung: Phreeqc doesn't run properly!!! ");
    exit(0);   
  }
#endif 

#ifndef PHREEQC
  return 1;
#endif
}


 
/**************************************************************************
   ROCKFLOW - Funktion: ReadOutputPhreeqc

   Aufgabe:
   Liest Ergebnisse der PHREEQC-Berechnungen aus PHREEQC-Ausdgabedatei
  
  Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *File: Dateiname der PHREEQC-Ausdgabedatei
   R char **val_out: Zeiger für chemische Conzentration 
   E char **name: Componentnamen 

  Ergebnis:
   0 bei Fehler oder Ende aufgrund Dateitest, sonst 1

   Programmaenderungen:
   06/2003     MX/SB         Erste Version
   11/2003     SB            Bugfix for large files, read long lines 
							 switched to iostreams
************************************************************************************************/
int REACT::ReadOutputPhreeqc(char* fout){

    int ok = 0;
    int nj,nk, ntot;
    int i, j, ii, zeilenlaenge=10000, anz;
    char str[4000];
    double dval;

    ifstream ein;
	ein.open(fout);
    if (! ein){
        DisplayMsgLn("The selected output file doesn't exist!!!");
        return 0;
    }
	/* get total number of species in PHREEQC output file */
	ntot = rcml_number_of_master_species+2 + rcml_number_of_equi_phases + rcml_number_of_ion_exchanges; 
	/* get lines to skip */
	anz = this->rcml_number_of_pqcsteps;

	ein.getline(str,zeilenlaenge);	 /* lies header-Zeile */

    for (i=0; i<this->nodenumber; i++){
	 if(this->rateflag[i] > 0){
		/* skip one line, if keyword steps larger than 1 even more lines */
		for(j=0;j<anz;j++){
			for(ii=0;ii<ntot;ii++){
				ein >> dval;
			}
		}
//		if(1 == 1){
/*-----------Read the concentration of all master species and pH pe values-------*/
           for (j=0; j<rcml_number_of_master_species+2; j++){
				 if(ein >> dval)
					this->val_out[j][i] = dval;
           }

/*--------------------Read the concentration of all equilibrium phases -------*/
           nj=rcml_number_of_master_species;
           for (j=nj+2; j<nj+2+rcml_number_of_equi_phases; j++){
             	if(ein >> dval)
					this->val_out[j][i] = dval;
           }

/*--------------------Read the concentration of all ion exchangers -------*/
           nk=rcml_number_of_equi_phases;
           for (j=nj+nk+2; j<nj+nk+2+rcml_number_of_ion_exchanges; j++){
             if(ein >> dval)
				this->val_out[j][i] = dval;
           }
//         }

	 } //if rateflag
		else{
			for(j=0;j<ntot;j++)
				this->val_out[j][i] =  this->val_in[j][i];
		}
     }

	ok=1;
    return ok; 
}




/**************************************************************************
   ROCKFLOW - Funktion: REACT::TestPHREEQC

   Aufgabe:
   Testet, ob ein externes PHREEQC- Eingabefile da ist
   allererste Version
   
   Programmaenderungen:
   09/2003     SB         Erste Version 
   01/2006     SB         New File handling
**************************************************************************/
void REACT::TestPHREEQC(string file_base_name){
 
string pqc_file_name = file_base_name + CHEM_REACTION_EXTENSION;
ifstream pqc_file (pqc_file_name.data(),ios::in);
if (pqc_file.good()) 
	flag_pqc = true;
pqc_file.close();
}



/**************************************************************************
   ROCKFLOW - Funktion: CalculateREactionRateFlag

   Aufgabe:
   Sets all reaction rates to Zero
   
   Programmaenderungen:
   03/2004     SB         Erste Version
**************************************************************************/
void REACT::CalculateReactionRateFlag(void){

long i, ni, comp, np, j;
double rate, schwellwert;
int level=1;
static int counti;
int *help;
double *helprates;
// Knoten * node;
// long *neighbor_nodes=NULL;
// int anz_neighbor_nodes;

/* Wann soll gerechnet werden: rel. Genauigkeit der Konzentration = deltaC/Cin /Zeitschrittlänge */
schwellwert = 1.0e-12;
if(dt>0.0)	schwellwert = 1.0e-4/dt;


ni = this->nodenumber;
np = this->number_of_comp;


/* Calculate chemmistry all rc->countsteps timesteps as well as always in the first and second timestep */
counti++;
// test = (aktueller_zeitschritt+1) % rc->count7steps;
if(((aktueller_zeitschritt+1) % this->countsteps) == 0) counti = 0;
if(aktueller_zeitschritt < 2) counti = 0;
if(counti == 0){
	for(i=0;i<ni;i++)
		this->rateflag[i] = 1;
}
else{
//determine for each node separately
DisplayMsgLn(" sum of rates ");

helprates = (double*) Malloc(np*sizeof(double));
for(j=0; j<np; j++) helprates[j]=0.0;


/* Go through all nodes */
for(i=0;i<ni;i++){
	rate = 0.0;
	for(comp=0; comp<np; comp++)
		rate = rate + fabs(this->rate[comp][i]*this->val_in[comp][i]);
	DisplayLong(i); DisplayMsg(": "); DisplayDouble(rate,0,0); DisplayMsgLn(" ");

	if(rate > schwellwert)
		this->rateflag[i] = 1;
	else
		this->rateflag[i] = 0;
} //end for(i<ni
DisplayMsgLn("  ");

help = (int*) Malloc(ni*sizeof(int));
for(i=0;i<ni;i++)
	help[i] = this->rateflag[i];

for(i=0;i<ni;i++){
	if(this->rateflag[i] > 0){
		//get sourrounding elements and nodes
		/* Aussuchen der Nachbarknoten */
		  SetNeighborNodesActive (i,level,help); 
//		  DisplayMsgLn(" ");
	}
}

//set nodes active finally
j=0;
for(i=0;i<ni;i++){
	DisplayMsg(" i: "); DisplayLong(i);
	DisplayMsg(",   this->rf[i]: "); DisplayLong(this->rateflag[i]); 
	DisplayMsg(",   this->rate[i]: "); DisplayDouble(this->rate[0][i],0,0); 
	DisplayMsg(",   help[i]: ");DisplayLong(help[i]); DisplayMsgLn(" ");
	if(help[i] > 0){
		this->rateflag[i] = 1;
		j++;}
}
DisplayMsg("Total number of nodes, at which chemistry is calculated: ");DisplayLong(j); DisplayMsgLn(" ");

//give back storage
help = (int*) Free(help);
helprates = (double*) Free(helprates);
}

}
/**************************************************************************
   ROCKFLOW - Funktion: REACT::GetTransportResults

   Aufgabe:
   Inserts reaction rate of component at index in field RECT->rate
   
   Programmaenderungen:
   03/2004     SB         Erste Version
**************************************************************************/
void REACT::GetTransportResults(void){

 int comp, phase, j, timelevel, np;
 CRFProcess *m_pcs = NULL;
 long  i, index;

 timelevel = 1;  // concentrations are in new timelevel
 phase = 0;      //single phase so far
 
 np = (int)pcs_vector.size();
 for(j=0;j<np;j++){ // for all processes
	m_pcs = pcs_vector[j];
	if(m_pcs->pcs_type_name.compare("MASS_TRANSPORT")==0){ // if it is a mass transport process
		comp = m_pcs->pcs_component_number; // get component number
			for(i=0;i<nodenumber;i++){
			// get concentration values
			val_in[comp][i] = m_pcs->GetNodeValue(i,m_pcs->GetNodeValueIndex(m_pcs->pcs_primary_function_name[0])+timelevel);
			if((val_in[comp][i] < 0.0) && (strcmp(name[comp],"pe")!= 0) ) {  //MX, 01.2005
				if(abs(val_in[comp][i]) > MKleinsteZahl) {
					DisplayMsg(" Neg. conc for component "); DisplayLong((long) comp);
					DisplayMsg(" at node "); DisplayLong((long)i);
					DisplayMsg("; conc = "); DisplayDouble(val_in[comp][i],0,0);
					DisplayMsgLn(" ");
				}
				val_in[comp][i] = 0.0 * val_in[comp][i];
			}
		}
	}
 }


 if(heatflag > 0){
	name[np-2] = "temp";//MX CMCD
    m_pcs = PCSGet("HEAT_TRANSPORT");
    index = m_pcs->GetNodeValueIndex("TEMPERATURE1");
	for(i=0;i<this->nodenumber;i++)
		this->val_in[np-2][i] = m_pcs->GetNodeValue(i, index); //OK PCSGetNODTemperature1L(i)//MX CMCD -2, Liquid Flow, Heat Transport
	}	
}


void REACT::SetNeighborNodesActive(long startnode, long level, int* help) //ToDo - reactivate with new structure
{
//  long *kanten=NULL;
  int anz_n;
//  long nd1[2];  
  int j=0,k;
  long *knoten;
//  long* neighbor_nodes =NULL;
  long *elems1d,*elems2d,*elems3d;
  int num_elems1d,num_elems2d,num_elems3d,num_elems;

 // DisplayMsg(" RSNNA: startnode: "); DisplayLong(startnode); DisplayMsg(", level: "); DisplayLong(level); DisplayMsgLn("");
  /* Ende rekursiv */
  if(level == 0) {
	  help[startnode] = 1;
//	  DisplayMsg(" Knoten, Level 0 "); DisplayLong(startnode); DisplayMsgLn(" ");
  }
  else {

  elems1d = GetNode1DElems(NodeNumber[startnode],&num_elems1d); //SB: liefert zu viele Elemente zurück
  if(num_elems1d > 0) num_elems1d =2;
  elems2d = GetNode2DElems(NodeNumber[startnode],&num_elems2d);
  if(num_elems2d > 4) num_elems2d =4;
  elems3d = GetNode3DElems(NodeNumber[startnode],&num_elems3d);
  if(num_elems3d > 8) num_elems3d =8;
  num_elems=num_elems1d+num_elems2d+num_elems3d;

  /* Alle benachbarten Knoten holen */
  for (j=0; j<num_elems1d; j++) {
	knoten = ElGetElementNodes(elems1d[j]);
	anz_n = ElGetElementNodesNumber(elems1d[j]);
	for(k=0;k<anz_n;k++){
		if(knoten[k]<0) {
			DisplayMsgLn(" Error");
		}
		SetNeighborNodesActive(knoten[k],level-1,help);
    }
  }
 for (j=0; j<num_elems2d; j++) {
	knoten = ElGetElementNodes(elems2d[j]);
	anz_n = ElGetElementNodesNumber(elems2d[j]);
	for(k=0;k<anz_n;k++){
		SetNeighborNodesActive(knoten[k],level-1,help);
    }
  }
for (j=0; j<num_elems3d; j++) {
	knoten = ElGetElementNodes(elems3d[j]);
	anz_n = ElGetElementNodesNumber(elems3d[j]);
	for(k=0;k<anz_n;k++){
		SetNeighborNodesActive(knoten[k],level-1,help);
    }
}

  } //endifs   
}
void REACT::ResetpHpe(void){
int i, j;

for (j=0; j<rcml_number_of_master_species+2; j++){
	if(strcmp(this->name[j], "pe")==0)
		for(i=0;i<this->nodenumber;i++) this->val_out[j][i] = this->val_in[j][i];
	if(strcmp(this->name[j], "pH")==0)
		for(i=0;i<this->nodenumber;i++) this->val_out[j][i] = this->val_in[j][i];
}
}
double MATCalcIonicStrengthNew(long index){
  int component, j, n=0, nn, timelevel, z_i;
  static long *element_nodes;
//  long component;
  double conc[100];
  double ion_strength=0.0;
//  if (rcml) 
  REACT *rc = NULL; //SB
  rc->GetREACT();
	n=rc->rcml_number_of_master_species;

//  n = get_rcml_number_of_master_species(rcml);
  for ( component=0; component<n; component++) {
	conc[component] =0.0;
      if (ElGetElementActiveState(index)){  
         nn = ElGetElementNodesNumber(index);
         element_nodes = ElGetElementNodes(index);
         for (j=0;j<nn;j++) {
            timelevel=1;
			conc[component] += PCSGetNODConcentration(element_nodes[j],component,timelevel);
		}
        conc[component] /= (double)nn;
        element_nodes = NULL;
      } 

/* calculate the solid phase: volume =v_mi*Ci */
//      conc[i]=CalcElementMeanConcentration (index, i, timelevel, ergebnis);
     CompProperties *m_cp = cp_vec[component];
     z_i = m_cp->valence;

   ion_strength += 0.5 * conc[component]*z_i*z_i;
	
  } /*for */
  return ion_strength;
}


void RCRead(string filename){

REACT *rc = new REACT(); //SB
rc->TestPHREEQC(filename); // Test if *.pqc file is present
REACT_vec.push_back(rc);
}


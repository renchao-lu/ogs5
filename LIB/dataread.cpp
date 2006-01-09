//bsp0078.cpp

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <list>

using namespace std;

#define MAX_ZEILE 2048

string read_MAT_name(string in, string *z_rest_out);

//klasse für die erstellung der keywordlist
class Keywordlist{

private:
    char datei[50];

public:
    list<string> mat_name_list;//liste der vorhandenen Materialgruppe "MATERIALS"
    list<string> keywd_list;//keyword-referenzliste "kw"
    list<string>mat_medium_properties_list;//im eingabefile vorhandene keywords
    list<double>mat_medium_properties_values_list;//im eingabefile vorhandene werte für die keywords
    list<double>mat_medium_conductivity_values_list;
    list<double>mat_medium_permeability_values_list;
    //void MATLoadDB(void);//liest materialgruppen(surfaces) ein
    void read_keywd_list(void);//liest referenzliste "kw"
    void comp_keywd_list(void);//gleicht referenzlidte ab und liest werte
    
};



void Keywordlist::read_keywd_list(void)
{
//File handling=============================
    //cout << "Dateiname: "<< flush;
	//cin.get(datei,50);
    strcpy(datei,"kw");
    ifstream eingabe(datei,ios::in);
	if (eingabe.good())
	{
	eingabe.seekg(0L,ios::beg);
//==========================================  
  string keyword("MATERIALS");
  string in;
  string line;
  string z_rest;
  string mat_name;
  string delimiter(";");
  

  string keywd_tmp("KEYWD");
  char line_char[MAX_ZEILE];
 

  // Read MATERIALS group names
       //1 get string with keywords
  while (!eingabe.eof()) {
	eingabe.getline(line_char, MAX_ZEILE);
    line = line_char;
    if(line.find_first_not_of(delimiter)!=string::npos) {
      in = line.substr(line.find_first_not_of(delimiter));//schneidet delimiter ab
      keywd_tmp = in.substr(0,in.find_first_of(delimiter));
      keywd_list.push_back(keywd_tmp);
      }
        keywd_list.remove(keyword);
      } // eof
           
    }// if eingabe.good
    return;
}


void Keywordlist::comp_keywd_list(void)
{
//File handling------------------------------------
    char datei[50];
	string sp;

	cout << "Dateiname: "<< flush;
	cin.get(datei,50);
	ifstream eingabe(datei,ios::in);
	if (eingabe.good())
	{
	    eingabe.seekg(0L,ios::beg);//rewind um materialgruppen auszulesen
  //-----------------------------------------------  
  //string keyword("MATERIALS"); hier müssen list inhalte von keywdlist abgeglichen werden
  string in;
  string in1;//zwischenstring zum abschneiden der einheit
  string line;
  string z_rest;
  //string mat_name;
  string delimiter(";");
  string keyword("MATERIALS");
  //string in;
//  string line;
//  string z_rest;
  string mat_name;
//  string delimiter(";");
  string mat_name_tmp("MAT_NAME");
  char line_char[MAX_ZEILE];
  double kwvalue;
  
//  char line_char[MAX_ZEILE];
 

   //1 get string where keyword hits 
  while (!eingabe.eof()) {
	eingabe.getline(line_char, MAX_ZEILE);
    line = line_char;
    
    string sp;
    list<string>::const_iterator pm =keywd_list.begin();
            while(pm!=keywd_list.end()) {
            sp = *pm;
            
           
            
    
    if(line.find(sp)!=string::npos)// if keyword found 
     {
      mat_medium_properties_list.push_back(sp);//füllt die liste mit vorhandenen keywords
      in1 = line.substr(line.find_first_not_of(delimiter)+sp.length());//schneidet keyword ab
      in = in1.substr(in1.find_first_of(delimiter));//schneidet evtl einheit nach keyword ab
      // 2 read values
      string mat_name_tmp("MAT_NAME");
      while(!mat_name_tmp.empty()) {
         mat_name_tmp = read_MAT_name(in,&z_rest);
        
            //value von string to double
            kwvalue = atof(mat_name_tmp.data());
           
         //if(mat_name_tmp.empty()) {
         //cout <<  "kein keyword-value mehr"  << endl;
         //return;
         //}
        //else {
                int ze;
                if(sp=="conductivity"){ze=1;}
                else if(sp=="permeability"){ze=2;}
                else {}
                switch(ze){
                  case 1:
                    //if(kwvalue!=0)//wenn nahc dem keyword eine einheit steht wird diese bein der stringumwandlung als 0 ausgegeben die dann mit in die liste eingetragen wird
                    mat_medium_conductivity_values_list.push_back(kwvalue);
                    break;
                  case 2:
                    //if(kwvalue!=0)
                    mat_medium_permeability_values_list.push_back(kwvalue);
                    break;
                }//switch(ze)
              in = z_rest;
              //}//else
            } // while mat_name_tmp               
          } // if keyword found
        ++pm;
        }//while des pm-iterators
    } // eof



/*if (eingabe.eof()){eingabe.seekg(0L,ios::beg);}
while (!eingabe.eof()) {
	eingabe.getline(line_char, MAX_ZEILE);
    line = line_char;
    if(line.find(keyword)!=string::npos) {
      in = line.substr(line.find_first_not_of(delimiter)+keyword.length());//schneidet keyword ab
      // 2 Read material group names
      while(!mat_name_tmp.empty()) {
         mat_name_tmp = read_MAT_name(in,&z_rest);
        if(mat_name_tmp.empty()) 
          {return;} 
        else {
          mat_name = mat_name_tmp;
          mat_name_list.push_back(mat_name);
          in = z_rest;
       }
      } // while mat_name
    } // keyword found
  } // eof*/ 

  

}// if eingabe.good
	return;
}

int main()
{
    /*/kontrollausgabe der eingelesenen keywrdlist===============
    Keywordlist kwd;  //instanz der klasse Keywordlist
    kwd.read_keywd_list();

    string sp;
    
    list<string>::const_iterator pm =kwd.keywd_list.begin();
            while(pm!=kwd.keywd_list.end()) {
            sp = *pm;
            cout <<  sp  << endl;
            ++pm;
            }
    //==========================================================*/
	//aufruf von comp_keywordlist damit daten in ihre strings bzw doubles eingetragen werden
    
    //kontrollausgabe der eingelesenen keywrdlist===============
    Keywordlist kwdl;  //instanz der klasse Keywordlist
    //kwdl.MATLoadDB();
    kwdl.read_keywd_list();
    kwdl.comp_keywd_list();

    string mg;
    string sp;
    double cv;
    double pv;
    
    /*list<string>::const_iterator lg =kwdl.mat_name_list.begin();
            cout <<  "gefundene Materialgruppen:"  << endl;
            while(lg!=kwdl.mat_medium_properties_list.end()) {
            mg = *lg;
            cout <<  mg  << endl;
            ++lg;
            }*/


    list<string>::const_iterator pm =kwdl.mat_medium_properties_list.begin();
            cout <<  "gefundene Keywords:"  << endl;
            while(pm!=kwdl.mat_medium_properties_list.end()) {
            sp = *pm;
            cout <<  sp  << endl;
            ++pm;
            }

    list<double>::const_iterator dm =kwdl.mat_medium_conductivity_values_list.begin();
            cout <<  "gefundene conductivity-Werte:"  << endl;
            while(dm!=kwdl.mat_medium_conductivity_values_list.end()) {
            cv = *dm;
            cout <<  cv  << endl;
            ++dm;
            }

    list<double>::const_iterator km =kwdl.mat_medium_permeability_values_list.begin();
            cout <<  "gefundene Werte:"  << endl;
            while(km!=kwdl.mat_medium_permeability_values_list.end()) {
            pv = *km;
            cout <<  pv  << endl;
            ++km;
            }
    //==========================================================*/
    return 0;
}


string read_MAT_name(string in, string *z_rest_out) 
{
    string mat_name;
    string z_rest;
    string delimiter(";");
    if(in.find_first_not_of(delimiter)!=string::npos)//wenn eine mg gefunden wird nach dem Schlüsselwort
    {
        z_rest = in.substr(in.find_first_not_of(delimiter));//string nach
        mat_name = z_rest.substr(0,z_rest.find_first_of(delimiter)); //string mat_name Name der Materialgruppe
        *z_rest_out = z_rest.substr(mat_name.length());
    	return mat_name;
    }
    else
    return "";
}
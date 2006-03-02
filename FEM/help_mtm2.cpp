/* SB: help - file for rebuilding MTM2 kernel to PCS structure */


#include "stdafx.h"                    /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

#define noTESTHELP_MTM2

/* Header / Andere intern benutzte Module */
#include "cel_mtm2.h"
#include "int_mtm2.h"
#include "help_mtm2.h"
#include "material.h" /*GetSoilPorosity(number); */
#include "elements.h"

//#include "mod__sb.h"





/* SB: aus mod_sb */
/*
void MTM2SetElementDtNew ( long number, int phase, double zeitschritt )
{
//  ElSetElementVal(number,index_timestep_SB,zeitschritt); //SB:todo
}

void MTM2SetElementPecletNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3; //SB:todo
//	ElSetElementVal(number,index,value);
}

double MTM2GetElementPecletNumNew ( long number, int phase, int component)
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3;  //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}


void MTM2SetElementCourantNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3 +1; //SB:todo
//	ElSetElementVal(number,index,value);
}

double MTM2GetElementCourantNumNew ( long number, int phase, int component)
{
//	int index;
//	index = index_TransportNumbers_SB + (component)*3 +1; //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}


void MTM2SetElementDamkohlerNumNew ( long number, int phase, int component, double value )
{
//	int index;
//	if(number < ElListSize()) {
//		index = index_TransportNumbers_SB + (component)*3 + 2; //SB:todo
//		ElSetElementVal(number,index,value);
//	}
}

double MTM2GetElementDamkohlerNumNew ( long number, int phase, int component)
{
//	int index;
//	if(number >= ElListSize()) return 0.0;
//	index = index_TransportNumbers_SB + (component)*3 +2; //SB:todo
//	return ElGetElementVal(number,index);
	return 1.0;
}
*/




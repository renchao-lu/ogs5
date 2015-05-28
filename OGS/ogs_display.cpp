/**************************************************************************/
/* ROCKFLOW - Modul: display.c
 */
/* Aufgabe:
   Enthaelt alle Funktionen fuer Standard Ein- und Ausgabe (Bildschirm,
   Tastatur)
 */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   10/1999     AH         Warnung entfernt
   01/2002     MK         Umleitung der DisplayX-Funktionen in MSG-Datei
                          Ausnahmen: DisplayStartMsg/DisplayEndMsg                                                  */
/**************************************************************************/
#include "ogs_display.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "Configure.h"
//#include "makros.h"


/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayStartMsg
 */
/* Aufgabe:
   Gibt Eroeffnungsbildschirm aus
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
   01/2010     NW         Automatic centering of the version information
 */
/**************************************************************************/
void DisplayStartMsg ( void )
{
	int i, pad_len;
	char buf[128];

	printf("\n");
	printf("          ###################################################\n");
	printf("          ##                                               ##\n");
	printf("          ##               OpenGeoSys-Project              ##\n");
	printf("          ##                                               ##\n");
	printf("          ##  Helmholtz Center for Environmental Research  ##\n");
	printf("          ##    UFZ Leipzig - Environmental Informatics    ##\n");
	printf("          ##                  TU Dresden                   ##\n");
	printf("          ##              University of Kiel               ##\n");
	printf("          ##            University of Edinburgh            ##\n");
	printf("          ##         University of Tuebingen (ZAG)         ##\n");
	printf("          ##       Federal Institute for Geosciences       ##\n");
	printf("          ##          and Natural Resources (BGR)          ##\n");
	printf("          ##  German Research Centre for Geosciences (GFZ) ##\n");
	printf("          ##                                               ##\n");

	//align the version information to center of the line
	printf("          ## ");
	sprintf(buf, "Version %s  Date %s", OGS_VERSION, OGS_DATE);
	pad_len = 45 - (int)strlen(buf);
	for (i = 0; i < pad_len / 2; i++)
		printf(" ");
	printf("%s", buf);
	for (i = 0; i < pad_len - pad_len / 2; i++)
		printf(" ");
	printf(" ##\n");

	printf("          ##                                               ##\n");
	printf("          ###################################################\n");
	printf("\n          File name (without extension): ");
}

/**************************************************************************/
/* ROCKFLOW - Funktion: DisplayEndMsg
 */
/* Aufgabe:
   Gibt Programm-Abspann aus.
 */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
 */
/* Ergebnis:
   - void -
 */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
 */
/**************************************************************************/
void DisplayEndMsg ( void )
{
	printf("\n          Programm beendet!\n\n\n");
}

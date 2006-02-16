/*************************************************************************
 ROCKFLOW - Modul: files2.c

 Aufgabe:
   Enthaelt die Datei- Ein- und Ausgaberoutinen, die UCD betreffen.

 Programmaenderungen:

*************************************************************************/

#include "stdafx.h"             /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Objekte */
#include "files.h"              /* TODO */
#include "geo_strings.h"
#include "nodes.h"              /*SetNodeVal */
#include "elements.h"           /*ElSetElementVal */
#include "files.h"              /* file_name */
#include "mathlib.h"
#include "rf_pcs.h"

/* Interne (statische) Deklarationen */
static long offset_number;      /* Falls die Elementnummern nicht mit 0 anfangen, wird ein Offset benutzt */

typedef struct {
    char *rfr_name;
    char *rfp_name;
    int art;
    int bin;
    int geom;
    double zeitschritt;
} RFFiles;

//OK_TIM
double startzeit = 0.0;

/* Definitionen */

/*************************************************************************
 ROCKFLOW - Funktion: ReadRFRRestartData

 Aufgabe:
   Liest Knoten- und Elementdaten

   Es waeren noch mehr Tests moeglich, aber die rf-Datei sollte ja nicht
   von Hand erzeugt werden, so dass (vorerst) darauf verzichtet wird.

 Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Stammname der Eingabedatei

 Ergebnis:
   0 bei Fehler, sonst 1

 Programmaenderungen:
   05/1999   OK   Implementierung
   02/2000   C.Thorenz    Restart wieder hergestellt
   06/2003 OK Restart wieder Wiederhergestellt
   11/2004 OK neues FileHandlingI
*************************************************************************/
int ReadRFRRestartData(string file_name_base)
{
    char name[31];              /* Daten-Bezeichner */
    char *s;                    /* gelesene Zeile */
    double *daten = NULL;       /* Feld mit Daten */
    int dateityp, anz_n, anz_e; /* Variablen fuer Kopfzeile */
    int p = 0;
    int pos = 0;
    int anz;                    /* Anzahl der Datenfelder */
    int *dgroessen = NULL;      /* Feld mit anz Datengroessen */
    long i, j, k, l;            /* Laufvariablen */
    long nr;                    /* Knoten- bzw. Elementnummer */
    int rfr_mesh_doesnt_fit_data = 0;   /* Sollen die Ergebnisse direkt zugeordnet werden? */
    int rfr_nodes = 0;
    int rfr_elements = 0;
    int data_entry = -1;
    double data_distance = 1.e99, *data_value = NULL, data_weight, point_weight, koor[3], point_weight_sum, *distance = NULL, *node_dist = NULL, biggest_dist, value;
    long node, number, *node_num, biggest_index;
    FILE *f;
    FILE *prot;
    static int datlen;
#define RESULTS_EXTENSION            ".rfr"
#define RESTART_PROTOCOL_EXTENSION   ".rfp"
    /* Dateinamen-Erweiterung fuer Text-Eingabedatei */
    static char kopf[81];
    static RFFiles *rfr_file;
    rfr_file = (RFFiles *) Malloc(sizeof(RFFiles));
    s = (char *) Malloc(MAX_ZEILE);
/*-----------------------------------------------------------------------*/
    /* File-Management */
    datlen = (int)strlen(file_name_base.c_str()) + 5; //OK
    rfr_file->rfr_name = (char*)Malloc(datlen);
    strcpy(rfr_file->rfr_name,file_name_base.c_str()); //OK
    strcat(rfr_file->rfr_name,RESULTS_EXTENSION);
    rfr_file->rfp_name = (char*)Malloc(datlen);
    strcpy(rfr_file->rfp_name,file_name_base.c_str()); //OK
    strcat(rfr_file->rfp_name,RESTART_PROTOCOL_EXTENSION);
    if ((f = fopen(rfr_file->rfr_name, "rb")) == NULL) {
      cout << "RFR: no restart data" << endl;
      return 0;
    }
    if ((prot = fopen(rfr_file->rfp_name, "w")) == NULL) {
      DisplayErrorMsg("Fehler: RFP-Datei konnte nicht geoeffnet werden !");
      return 0;
    }
/*-----------------------------------------------------------------------*/
    /* Kopf binaer lesen */
    if (fread((void *) kopf, 80, 1, f) != 1) {
        DisplayErrorMsg("Fehler beim Lesen der RFR-Datei (Kopf) !!");
        fclose(f);
        return 0;
    }
    kopf[80] = '\0';
    for (i = 0; i < 80; i++)
        if (kopf[i] == '#')
            kopf[i] = ' ';
    sscanf(kopf, "%d %d %ld %d %lg %lg %d %d", \
           &rfr_file->art, &rfr_file->bin, &nr, &rfr_file->geom, &startzeit, &rfr_file->zeitschritt, &rfi_filetype, &rfr_mesh_doesnt_fit_data);
    /* richtige Datei ? */
    if (rfr_file->art != 0) {
        DisplayErrorMsg("Fehler: Falsche RF-Eingabedatei !!");
        fclose(f);
        return 0;
    }
    /* startzeit kontrollieren */
    if (startzeit < 0.0) {
        FilePrintString(prot, "* startzeit<0.0, Korrektur: 0.0 !");
        LineFeed(prot);
        startzeit = 0.0;
    }
    /* zu lesende Daten bestimmen
       OK rf3261 - unklare Funktionsweise */
    /* Olaf: BITTE(!!!) nimm das nur dann heraus, wenn Du _ganz_ sicher
       bitte, dass der Restart danach noch einwandfrei funktioniert. Du
       hast es zweimal geloescht und ich habe es jetzt zweimal wieder 
       reingetan. Gruss, Carsten, 24.7.02  */
/*
    if (startzeit > 0.0)
        SelectData(2);
    else
        SelectData(1);
*/
    /* Ich bin mir ziemlich sicher, aber man weiß ja nie, ... 
       OK 23.06.2003 */
    /* ASCII oder binaer ? */
    if (!rfr_file->bin) {
        fclose(f);
        /* RF-Datei erneut oeffnen (ascii) */
        if ((f = fopen(rfr_file->rfr_name, "r")) == NULL) {
            DisplayErrorMsg("Fehler: RFI-Datei konnte nicht geoeffnet werden !");
            return 0;
        }
        /* Kopf ueberlesen */
        if (fgets(s, MAX_ZEILE, f) == NULL) {
            DisplayErrorMsg("Fehler: RFI-Datei konnte nicht angelesen werden !");
            fclose(f);
            return 0;
        }
    }
    /* nr auswerten */
/*OK_OUT
    if (output_restart)
        output_nr = 0;
    else {
        if (output_start)
            output_nr = nr;
        else
            output_nr = nr + 1;
    }
*/
/*-----------------------------------------------------------------------
 Vorpann lesen */
    /* muss ueberhaupt gelesen werden ??? */
    if (!(danz_n + danz_e)) {
        s = (char *) Free(s);
        return 1;
    }
    /* Kopfzeile lesen */
    if (fgets(s, MAX_ZEILE, f) == NULL) {
        FilePrintString(prot, "* !!! Fehler: Kopf-Zeile nicht gelesen; Einlesen beendet !");
        LineFeed(prot);
        s = (char *) Free(s);
        return 0;
    }
    if (sscanf(s, " %d %d %d %d %d", &dateityp, &anz_n, &anz_e, &rfr_nodes, &rfr_elements) < 3) {
        FilePrintString(prot, "* !!! Fehler: Kopf-Daten unvollstaendig; Einlesen beendet !");
        LineFeed(prot);
        s = (char *) Free(s);
        return 0;
    }
    /* Test, ob richtiger Abschnitt */
    if (!dateityp) {            /* dateityp==0: Geometrie, dateityp==1: Knoten-Daten, dateityp==2: XYZ-Daten */
        FilePrintString(prot, "* !!! Fehler: Falsche Daten-Kennung; Einlesen beendet !");
        LineFeed(prot);
        s = (char *) Free(s);
        return 0;
    }
    /* sind genuegend Knotendaten vorhanden ??? */
    if (danz_sum_n > anz_n) {
        FilePrintString(prot, "* !!! Fehler: Nicht genuegend Knotendaten vorhanden; Einlesen beendet !");
        LineFeed(prot);
        s = (char *) Free(s);
        return 0;
    }
    /* Knotendaten bearbeiten */
    if (anz_n) {
        p = pos = 0;
        /* Knotendaten-Indizes bestimmen */
        if (fgets(s, MAX_ZEILE, f) == NULL) {
            FilePrintString(prot, "* !!! Fehler: Knotendaten-Zeile nicht gelesen; Einlesen beendet !");
            LineFeed(prot);
            s = (char *) Free(s);
            return 0;
        }
        if (sscanf(s, " %d%n", &anz, &pos) != 1) {
            FilePrintString(prot, "* !!! Fehler: Knotendaten-Anzahl nicht gelesen; Einlesen beendet !");
            LineFeed(prot);
            s = (char *) Free(s);
            return 0;
        }
        if (danz_n > anz) {
            FilePrintString(prot, "* !!! Fehler: Nicht genuegend Knotendatenfelder vorhanden; Einlesen beendet !");
            LineFeed(prot);
            s = (char *) Free(s);
            return 0;
        }
        /* Feldgroessen lesen */
        dgroessen = (int *) Malloc(anz * sizeof(int));
        for (i = 0; i < anz; i++) {
            if (sscanf(&s[p += pos], " %d%n", &(dgroessen[i]), &pos) != 1) {
                FilePrintString(prot, "* !!! Fehler: Feldgroesse nicht gelesen; Einlesen beendet !");
                LineFeed(prot);
                dgroessen = (int *) Free(dgroessen);
                s = (char *) Free(s);
                return 0;
            }
            /* evtl. Test: sind dgroessen[i] positiv ??? */
        }
        /* Name und Einheit lesen */
        for (i = 0; i < anz; i++) {
            if (fgets(s, MAX_ZEILE, f) == NULL) {
                FilePrintString(prot, "* !!! Fehler: Einheiten-Zeile nicht gelesen; Einlesen beendet !");
                LineFeed(prot);
                dgroessen = (int *) Free(dgroessen);
                s = (char *) Free(s);
                return 0;
            }
            sscanf(s, "%[^,]", name);
            j = 0;
//SB:namehelp            while ((j < danz_n) && (strcmp(name, datafield_n[j].name) != 0))
			while ((j < danz_n) && (strcmp(name, GetCompNamehelp(datafield_n[j].name)) != 0))
                j++;
            if (j < danz_n) {   /* gefunden */
                datafield_n[j].idx_file = i;
                if (datafield_n[j].transfer != dgroessen[i]) {
                    FilePrintString(prot, "* !!! Fehler: Feldgroesse von ");
                    FilePrintString(prot, name);
                    FilePrintString(prot, " stimmt nicht; Einlesen beendet !");
                    LineFeed(prot);
                    dgroessen = (int *) Free(dgroessen);
                    s = (char *) Free(s);
                    return 0;
                }
            }
        }
        /* sind alle Daten vorhanden ??? */
        dgroessen = (int *) Free(dgroessen);
        for (i = 0; i < danz_n; i++) {
            if (datafield_n[i].idx_file < 0) {
                DisplayMsgLn("Warning: Restart data missing!");
                FilePrintString(prot, "* !!! Warning: Restart data missing: ");
//SB:namehelp                FilePrintString(prot, datafield_n[i].name);
			    FilePrintString(prot, GetCompNamehelp(datafield_n[i].name));
                LineFeed(prot);
            }
        }
/*-----------------------------------------------------------------------*/
        /* Knotendaten lesen */
        /* Knotennummer oder Koordinaten lesen */
        if (dateityp == 1) {
            daten = (double *) Malloc(anz_n * sizeof(double));
            for (i = 0; i < NodeListLength; i++) {
                p = pos = 0;
                if (fgets(s, MAX_ZEILE, f) == NULL) {
                    FilePrintString(prot, "* !!! Fehler: Knotendaten-Zeile nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* Knotennummer lesen */
                if (sscanf(s, " %ld%n", &nr, &pos) != 1) {
                    FilePrintString(prot, "* !!! Fehler: Knotennummer nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* existiert Knotennummer ??? */
                if ((nr < 0l) || (GetNode(nr) == NULL)) {
                    FilePrintString(prot, "* !!! Fehler: Knoten existiert nicht; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* Knotenwerte lesen */
                for (j = 0; j < anz_n; j++) {
                    if (sscanf(&s[p += pos], " %lf%n", &(daten[j]), &pos) != 1) {
                        FilePrintString(prot, "* !!! Fehler: Knotenwert nicht gelesen; Einlesen beendet !");
                        LineFeed(prot);
                        daten = (double *) Free(daten);
                        s = (char *) Free(s);
                        return 0;
                    }
                }
                /* Daten uebertragen */
                for (j = 0; j < danz_n; j++)
                    for (k = 0; k < datafield_n[j].transfer; k++) {
                        if (datafield_n[j].idx_file >= 0)
                            SetNodeVal(nr, k + datafield_n[j].idx_program, daten[k + datafield_n[j].idx_file]);
                    }
            }
            daten = (double *) Free(daten);
        }
        if (dateityp == 2) {
            daten = (double *) Malloc((anz_n + 3) * rfr_nodes * sizeof(double));
            for (i = 0; i < rfr_nodes; i++) {
                p = pos = 0;
                if (fgets(s, MAX_ZEILE, f) == NULL) {
                    FilePrintString(prot, "* !!! Fehler: Knotendaten-Zeile nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* Koordinaten lesen */
                for (j = 0; j < 3; j++) {
                    if (sscanf(&s[p += pos], " %lf%n", &(daten[i * (anz_n + 3) + j]), &pos) != 1) {
                        FilePrintString(prot, "* !!! Fehler: Koordinaten nicht gelesen; Einlesen beendet !");
                        LineFeed(prot);
                        daten = (double *) Free(daten);
                        s = (char *) Free(s);
                        return 0;
                    }
                }
                /* Knotenwerte lesen */
                for (j = 0; j < anz_n; j++) {
                    if (sscanf(&s[p += pos], " %lf%n", &(daten[i * (anz_n + 3) + 3 + j]), &pos) != 1) {
                        FilePrintString(prot, "* !!! Fehler: Knotenwert nicht gelesen; Einlesen beendet !");
                        LineFeed(prot);
                        daten = (double *) Free(daten);
                        s = (char *) Free(s);
                        return 0;
                    }
                }
            }
            /* Knotenwerte eintragen */
            for (i = 0; i < NodeListLength; i++) {
                node = NodeNumber[i];
                koor[0] = GetNodeX(node);
                koor[1] = GetNodeY(node);
                koor[2] = GetNodeZ(node);
                /* Nearest point */
                if (rfr_mesh_doesnt_fit_data == 1) {
                    data_entry = -1;
                    data_distance = 1.e99;
                    for (j = 0; j < rfr_nodes; j++) {
                        /* Den naechsten Datenpunkt suchen */
                        if (MCalcDistancePointToPoint(koor, &daten[j * (anz_n + 3)]) < data_distance) {
                            data_distance = MCalcDistancePointToPoint(koor, &daten[j * (anz_n + 3)]);
                            data_entry = j;
                        }
                    }
                    if (data_entry > -1) {
                        /* Daten uebertragen */
                        for (j = 0; j < danz_n; j++)
                            for (k = 0; k < datafield_n[j].transfer; k++)
                                if (datafield_n[j].idx_file >= 0)
                                    SetNodeVal(node, k + datafield_n[j].idx_program, daten[data_entry * (anz_n + 3) + 3 + k + datafield_n[j].idx_file]);
                    }           /* endif */
                }


                /* Inverse distance weighting */
                if ((rfr_mesh_doesnt_fit_data > 1) && (rfr_mesh_doesnt_fit_data < 10)) {
                    data_value = (double *) Malloc((anz_n + 3) * sizeof(double));
                    distance = (double *) Malloc(rfr_nodes * sizeof(double));

                    for (k = 0; k < anz_n; k++)
                        data_value[k + 3] = 0.;

                    data_weight = 0.;

                    for (j = 0; j < rfr_nodes; j++) {
                        distance[j] = MCalcDistancePointToPoint(koor, &daten[j * (anz_n + 3)]);
                        data_weight += pow(1. / (distance[j] + MKleinsteZahl), (double) rfr_mesh_doesnt_fit_data - 1);
                    }

                    point_weight_sum = 0.;
                    for (j = 0; j < rfr_nodes; j++) {
                        point_weight = 1. / (distance[j] + MKleinsteZahl);
                        point_weight = pow(point_weight, (double) rfr_mesh_doesnt_fit_data - 1);
                        point_weight /= data_weight;
                        point_weight_sum += point_weight;
                        for (k = 0; k < anz_n; k++)
                            data_value[k + 3] += daten[j * (anz_n + 3) + k + 3] * point_weight;
                    }

                    /* Daten uebertragen */
                    for (j = 0; j < danz_n; j++)
                        for (k = 0; k < datafield_n[j].transfer; k++)
                            if (datafield_n[j].idx_file >= 0)
                                SetNodeVal(node, k + datafield_n[j].idx_program, data_value[3 + k + datafield_n[j].idx_file]);

                    data_value = (double *) Free(data_value);
                    distance = (double *) Free(distance);
                }

                /* Inverse distance weighting without near field */
                if ((rfr_mesh_doesnt_fit_data > 11) && (rfr_mesh_doesnt_fit_data < 20)) {
                    data_value = (double *) Malloc((anz_n + 3) * sizeof(double));
                    distance = (double *) Malloc(rfr_nodes * sizeof(double));
                    for (k = 0; k < anz_n; k++)
                        data_value[k + 3] = 0.;

                    data_weight = 0.;

                    for (j = 0; j < rfr_nodes; j++) {
                        distance[j] = MCalcDistancePointToPoint(koor, &daten[j * (anz_n + 3)]);
                        if (distance[j] > MKleinsteZahl)
                            data_weight += pow(1. / (distance[j] + MKleinsteZahl), (double) rfr_mesh_doesnt_fit_data - 1);
                    }

                    point_weight_sum = 0.;
                    for (j = 0; j < rfr_nodes; j++) {
                        if (distance[j] > MKleinsteZahl) {
                            point_weight = 1. / (distance[j] + MKleinsteZahl);
                            point_weight = pow(point_weight, (double) rfr_mesh_doesnt_fit_data - 1);
                            point_weight /= data_weight;
                            point_weight_sum += point_weight;
                            for (k = 0; k < anz_n; k++)
                                data_value[k + 3] += daten[j * (anz_n + 3) + k + 3] * point_weight;
                        }
                    }

                    /* Daten uebertragen */
                    for (j = 0; j < danz_n; j++)
                        for (k = 0; k < datafield_n[j].transfer; k++)
                            if (datafield_n[j].idx_file >= 0)
                                SetNodeVal(node, k + datafield_n[j].idx_program, data_value[3 + k + datafield_n[j].idx_file]);

                    data_value = (double *) Free(data_value);
                    distance = (double *) Free(distance);
                }

                /* Nearest N */
                if ((rfr_mesh_doesnt_fit_data > 20) && (rfr_mesh_doesnt_fit_data < 30)) {
                    distance = (double *) Malloc(rfr_nodes * sizeof(double));
                    number = 1 + (long) pow(2., (double) (rfr_mesh_doesnt_fit_data - 20));
                    node_num = (long *) Malloc(number * sizeof(long));
                    node_dist = (double *) Malloc(number * sizeof(double));

                    /* Berechnen der Entfernung */
                    for (j = 0; j < rfr_nodes; j++)
                        distance[j] = MCalcDistancePointToPoint(koor, &daten[j * (anz_n + 3)]);

                    /* Bestimmen der n naechsten Punkte */
                    for (j = 0; j < number; j++) {
                        node_dist[j] = 1.e99;
                        node_num[j] = -1;
                    }

                    for (j = 0; j < rfr_nodes; j++) {
                        /* Groesste gespeicherte Entfernung suchen */
                        biggest_dist = 0.;
                        biggest_index = -1;
                        for (k = 0; k < number; k++) {
                            if (node_dist[k] > biggest_dist) {
                                biggest_dist = node_dist[k];
                                biggest_index = k;
                            }
                        }
                        /* jetzt ist der groesste bisherige Wert bekannt */

                        if (distance[j] < biggest_dist) {
                            node_dist[biggest_index] = distance[j];
                            node_num[biggest_index] = j;
                        }
                    }

                    /* Daten uebertragen */
                    for (j = 0; j < danz_n; j++) {
                        for (k = 0; k < datafield_n[j].transfer; k++) {
                            if (datafield_n[j].idx_file >= 0) {
                                value = 0.;
                                for (l = 0; l < number; l++)
                                    value += daten[node_num[l] * (anz_n + 3) + 3 + k + datafield_n[j].idx_file] / (double) number;
                                SetNodeVal(node, k + datafield_n[j].idx_program, value);
                            }
                        }
                    }
                    distance = (double *) Free(distance);
                    node_num = (long *) Free(node_num);
                    node_dist = (double *) Free(node_dist);
                }
            }
            daten = (double *) Free(daten);
        }                       /* endif */
    }                           /* endif Knotendaten */



/*-----------------------------------------------------------------------
 Element-Daten lesen */

    if (dateityp == 1) {
        /* sind genuegend Elementdaten vorhanden ??? */
        if (danz_sum_e > anz_e) {
            FilePrintString(prot, "* !!! Fehler: Nicht genuegend Elementdaten vorhanden; Einlesen beendet !");
            LineFeed(prot);
            s = (char *) Free(s);
            return 0;
        }
        /* Elementdaten bearbeiten */
        if (anz_e) {
            p = pos = 0;
            /* Elementdaten-Indizes bestimmen */
            if (fgets(s, MAX_ZEILE, f) == NULL) {
                FilePrintString(prot, "* !!! Fehler: Elementdaten-Zeile nicht gelesen; Einlesen beendet !");
                LineFeed(prot);
                s = (char *) Free(s);
                return 0;
            }
            if (sscanf(s, " %d%n", &anz, &pos) != 1) {
                FilePrintString(prot, "* !!! Fehler: Elementdaten-Anzahl nicht gelesen; Einlesen beendet !");
                LineFeed(prot);
                s = (char *) Free(s);
                return 0;
            }
            if (danz_e > anz) {
                FilePrintString(prot, "* !!! Fehler: Nicht genuegend Elementdatenfelder vorhanden; Einlesen beendet !");
                LineFeed(prot);
                s = (char *) Free(s);
                return 0;
            }
            /* Feldgroessen lesen */
            dgroessen = (int *) Malloc(anz * sizeof(int));
            for (i = 0; i < anz; i++) {
                if (sscanf(&s[p += pos], " %d%n", &(dgroessen[i]), &pos) != 1) {
                    FilePrintString(prot, "* !!! Fehler: Feldgroesse nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    dgroessen = (int *) Free(dgroessen);
                    s = (char *) Free(s);
                    return 0;
                }
                /* evtl. Test: sind dgroessen[i] positiv ??? */
            }
            /* Name und Einheit lesen */
            for (i = 0; i < anz; i++) {
                if (fgets(s, MAX_ZEILE, f) == NULL) {
                    FilePrintString(prot, "* !!! Fehler: Einheiten-Zeile nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    dgroessen = (int *) Free(dgroessen);
                    s = (char *) Free(s);
                    return 0;
                }
                sscanf(s, "%[^,]", name);
                j = 0;
                while ((j < danz_e) && (strcmp(name, datafield_e[j].name) != 0))
                    j++;
                if (j < danz_e) {       /* gefunden */
                    datafield_e[j].idx_file = i;
                    if (datafield_e[j].transfer != dgroessen[i]) {
                        FilePrintString(prot, "* !!! Fehler: Feldgroesse von ");
                        FilePrintString(prot, name);
                        FilePrintString(prot, " stimmt nicht; Einlesen beendet !");
                        LineFeed(prot);
                        dgroessen = (int *) Free(dgroessen);
                        s = (char *) Free(s);
                        return 0;
                    }
                }
            }
            /* sind alle Daten vorhanden ??? */
            dgroessen = (int *) Free(dgroessen);
            for (i = 0; i < danz_e; i++) {
                if (datafield_e[i].idx_file < 0) {
                    FilePrintString(prot, "* !!! Fehler: Datum ");
                    FilePrintString(prot, datafield_e[i].name);
                    FilePrintString(prot, " fehlt; Einlesen beendet !");
                    LineFeed(prot);
                    s = (char *) Free(s);
                    return 0;
                }
            }
            /* Elementdaten lesen */
            daten = (double *) Malloc(anz_e * sizeof(double));
            for (i = 0; i < (ElementListLength); i++) {
                p = pos = 0;
                if (fgets(s, MAX_ZEILE, f) == NULL) {
                    FilePrintString(prot, "* !!! Fehler: Elementdaten-Zeile nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* Elementnummer lesen */
                if (sscanf(s, " %ld%n", &nr, &pos) != 1) {
                    FilePrintString(prot, "* !!! Fehler: Elementnummer nicht gelesen; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }

                nr -= offset_number;    /* Falls die Elementnummern nicht mit 0 anfingen */

                /* existiert Elementnummer ??? */
                if ((nr < 0) || (ElGetElement(nr) == NULL)) {
                    FilePrintString(prot, "* !!! Fehler: Element existiert nicht; Einlesen beendet !");
                    LineFeed(prot);
                    daten = (double *) Free(daten);
                    s = (char *) Free(s);
                    return 0;
                }
                /* Elementwerte lesen */
                for (j = 0; j < anz_e; j++) {
                    if (sscanf(&s[p += pos], " %lf%n", &(daten[j]), &pos) != 1) {
                        FilePrintString(prot, "* !!! Fehler: Elementwert nicht gelesen; Einlesen beendet !");
                        LineFeed(prot);
                        daten = (double *) Free(daten);
                        s = (char *) Free(s);
                        return 0;
                    }
                }
                /* Daten uebertragen */
                for (j = 0; j < danz_e; j++)
                    for (k = 0; k < datafield_e[j].transfer; k++)
                        ElSetElementVal(nr, k + datafield_e[j].idx_program, daten[k + datafield_e[j].idx_file]);
            }
            daten = (double *) Free(daten);
        }
    }
    if (dateityp == 2) {
        DisplayMsgLn("Reading of element-data not yet supported!!");
    }
    s = (char *) Free(s);
    rfr_file->rfr_name = (char *) Free(rfr_file->rfr_name);
    rfr_file->rfp_name = (char *) Free(rfr_file->rfp_name);
    rfr_file = (RFFiles *) Free(rfr_file);

    fclose(f);
    fclose(prot);

    return 1;
}

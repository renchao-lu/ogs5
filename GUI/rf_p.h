/**************************************************************************/
/* ROCKFLOW - Modul: adaptiv.h
                                                                          */
/* Aufgabe:
   verschiedene Funktionen, die von verschiedenen Modulen gebraucht
   werden und Adaptivitaet voraussetzen
                                                                          */
/**************************************************************************/

#ifndef rf_p_INC
#define rf_p_INC
  /* Schutz gegen mehrfaches Einfuegen */


/* Andere oeffentlich benutzte Module */

/* Deklarationen */

extern double Welt_xmin,Welt_xmax,Welt_ymin,Welt_ymax;

extern int xpixel(double x);
extern int ypixel(double y);
extern double x_real(int xpixel,int width);
extern double y_real(int ypixel,int height);

extern void GetWindowAttributes(HWND window, int *width, int *height);
extern void MFC_line(double xa, double ya, double xe, double ye, CClientDC *dc);
extern void MFC_setcor(double xmin, double xmax, double ymin, double ymax, int width, int height);
extern void MFC_text(double x, double y, char *string, CClientDC *dc);
extern void iso4k (long el4[4], double h[], double x[], double y[],
            double h1, double hd, double h2,\
            CClientDC *dc,int width,int height);
extern void iso3k (double h[3],double x[3],double y[3],\
            double h1, double hd, double h2,\
            CClientDC *dc,int width,int height);
extern void MFCLinePixel(int x1, int y1, int x2, int y2, CClientDC *dc);

#endif

/**************************************************************************/
/* Plotfunktionen                                                         */
/**************************************************************************/

#include "stdafx.h"

#include "rf_p.h"
//#include "rf_geo.h"

HDC    hdc_main;
static int width,height;
int xminp,yminp;
double skalex,skaley;
double Welt_xmin,Welt_xmax,Welt_ymin,Welt_ymax;


void GetWindowAttributes(HWND window, int *width, int *height)
{
  RECT rect;
  GetClientRect(window,&rect);
  *width=rect.right;
  *height=rect.bottom;
  return;
}

void setcor(double xmin, double xmax, double ymin, double ymax)
{
  Welt_xmin=xmin; Welt_xmax=xmax;
  Welt_ymin=ymin; Welt_ymax=ymax;

  skalex=( (double)width ) / (Welt_xmax-Welt_xmin);
  skaley=( (double)height ) / (Welt_ymax-Welt_ymin);

  xminp = (int) ( - xmin * skalex);
  yminp = (int) ( height + ymin * skaley);

  return;
}      	  	                                     


int xpixel(double x)
{
  return (int)(xminp + x * skalex);
}      	  	                                     

int ypixel(double y)
{
  return (int)( yminp - y * skaley);
}      	  	                                     


double x_real(int xpixel,int width)
{
  return ((double) xpixel/(double) width)*(Welt_xmax-Welt_xmin);
}      	  	                                     

double y_real(int ypixel,int height)
{
  return ((double) (yminp-ypixel)/(double) height)*(Welt_ymax-Welt_ymin);
}      	  	                                     



void pline(int x1, int y1, int x2, int y2)
{ 
#ifdef WIN32
  MoveToEx(hdc_main,x1,y1,NULL);
#else
  MoveTo(hdc_main,x1,y1);
#endif

  LineTo(hdc_main,x2,y2);

  return;
}


void line(double xa, double ya, double xe, double ye)
{
  pline(xpixel(xa), ypixel(ya), xpixel(xe), ypixel(ye) );
  return;
}


void polyline(double* x, double* y, long n)
{
  long i;
  for(i=0; i<n-1; i++) line(x[i], y[i], x[i+1], y[i+1] );
}


void vpolyline(double* x, double* y, long n)
{
  POINT*  points = (POINT*)malloc(n*sizeof(POINT));
  long i;

  for(i=0; i<n; i++) {
    points[i].x = xpixel(x[i]);
    points[i].y = ypixel(y[i]);
  }

  Polyline(hdc_main,points,n); // ? Windows-Funktion

  free(points);
}



/*===========================================================================*/
/* Windows-Version */
/* MFC-Funktionen */
/* 06/1999   O.Kolditz   Erste Version */

/*---------------------------------------------------------------------------*/
/* Coordinates */
void MFC_setcor(double xmin, double xmax, double ymin, double ymax, int width, int height)
{
  Welt_xmin=xmin; Welt_xmax=xmax;
  Welt_ymin=ymin; Welt_ymax=ymax;

  skalex=( (double)width ) / (Welt_xmax-Welt_xmin);
  skaley=( (double)height ) / (Welt_ymax-Welt_ymin);

  xminp = (int) ( - xmin * skalex);
  yminp = (int) ( height + ymin * skaley);

  return;
}      	  	                                     


/*---------------------------------------------------------------------------*/
/* Lines */
void MFCLinePixel(int x1, int y1, int x2, int y2, CClientDC *dc)
{ 
  dc->MoveTo(x1,y1);
  dc->LineTo(x2,y2);
  return;
}

void MFC_line(double xa, double ya, double xe, double ye, CClientDC *dc)
{
	MFCLinePixel(xpixel(xa), ypixel(ya), xpixel(xe), ypixel(ye), dc);
	return;
}



/*---------------------------------------------------------------------------*/
/* Text */
void MFC_text(double x, double y, char *string, CClientDC *dc)
{
	TextOut(*dc, xpixel(x),ypixel(y),string, (int)strlen(string));
	return;
}


/*===========================================================================*/
/* Iso-Linien */
/* von R.Ratke */
#include <math.h>

double sign (double a1, double a2) // sign wie bei FTN
{ if (a2 >=0.0) return (a1); return (-a1);
} // sign

double dabs (double a){if (a >= 0.0) return (a); return (-a);}

int md1(int a,int b) // simples mod
{ if (a < b) return(a); return(a-b);
}

void plot(double x,double y,int ipen,\
          int sc_xmax,int sc_ymax,CClientDC *dc) /* plotroutine */
{
 //static int sc_xmax, sc_ymax;           /* merken: screensize */
 static double xy_fakt; /* Massstab */
 static double x_ref, y_ref;            /* Nullpunkt */

    //if (ipen==1)    /* initialize graphics mode */
 xy_fakt=min((double)sc_xmax/x,(double)sc_ymax/y);
 x_ref = 0.0; /* Nullpunkt */
 y_ref = 0.0;

/* Plot-Zweige: nur 2,3 implementiert */
    if (ipen==2) dc->LineTo (xpixel(x),ypixel(y));
    if (ipen==3) dc->MoveTo (xpixel(x),ypixel(y));

    return;
}

void plot4k (int nstep, double rs[2],long el4[4],double xk[],double yk[],double ff[2][4],\
             CClientDC *dc,int width,int height)
// Teilstueck plotten nach nat. Koordinate r,s
{double  r, s, x=0., y=0.;
 static double p, q;
 int i, is;
      if (nstep == 0) // initialize
      {  p=rs[0]; q=rs[1]; // save!
         for (i=0; i<4; i++)
         { x=x+(1.+ff[0][i]*p)*(1.+ff[1][i]*q)*xk[el4[i]];
           y=y+(1.+ff[0][i]*p)*(1.+ff[1][i]*q)*yk[el4[i]];
         }
         plot (x*0.25,y*0.25,3,width,height,dc);
         return;
       }

// Geraden im r-s-System bei nstep >1 evtl. Kurven in x-y-System
      r=p; s=q; // saved
      for (is=0; is < nstep; is++)
      {  r +=(rs[0]-p)/nstep;  s +=(rs[1]-q)/nstep;
         x=0.;  y=0.;
         for (i=0; i<4; i++)
         {  x +=(1.+ff[0][i]*r)*(1.+ff[1][i]*s)*xk[el4[i]];
            y +=(1.+ff[0][i]*r)*(1.+ff[1][i]*s)*yk[el4[i]];
         }
         plot (x*0.25,y*0.25,2,width,height,dc);
      }
      p=r; q=s;  // save!
      return;
}//  end PLOT4K



void iso4k (long el4[4], double h[], double x[], double y[],
            double h1, double hd, double h2,\
            CClientDC *dc,int width,int height)
/* Alle Hîhenschichtlinien eines Elements zeichnen, Vierecke
   bilin. Ansatz - R. Ratke, Jan 2000 -
   Parameter: el4  [4]: die 4 Eckennummern des Elements >=0
              h    [*]: Hîhen aller Knoten
              x    [*]: Plotkoordinaten (cm) aller Knoten, x
              y    [*]: Plotkoordinaten (cm) aller Knoten, y
              h1      : Wert der kleinsten Hîhenschichtlinie
              hd      : Abstand der Hîhenlinien
              h2      : Wert der grî·ten Hîhenschichtlinie
   es wird nichts verÑndert in den Feldern!
*/
{     double r4[2][4] ={{1., -1., -1.,  1.},  // Formfunktionen bzw.
                        {1.,  1., -1., -1.}}; // Eckenkoordinaten, r-s
      double r[2],p[2],a[2],q[2];

      double eps =1.e-4;  // rel. Genauigkeit gegenÅber 1.0
      double eps0=1.e-7;  // abs. Genauigkeit bezgl. Koord.
      int i, nstep=10;    // Anzahl Teilstriche je BogenstÅck

      double hmin=h[el4[0]], hmax= h[el4[0]];

      for (i=1; i<4; i++) {  
         hmin=min(hmin,h[el4[i]]);// extreme h
         hmax=max(hmax,h[el4[i]]);// im Element
         //hmin=min(hmin,h[i]);// extreme h
         //hmax=max(hmax,h[i]);// im Element
      }

      h1 -=hd; // wird Wert der zu zeichnenden HSL
Neue_HSL:
      h1 +=hd;
      if (h1 > h2)   return;
      if (h1 > hmax) return;
      if (h1 < hmin) goto Neue_HSL;

// h1-Linie geht durch Element
      { int nix[4]= {0, 0, 0, 0}, i1= el4[3], i, i2, j, k, l, gerade;
        double c, d, t1, t2, z, t, dt, et;
// Vorwerte
        a[0]=  h[el4[0]]-h[el4[1]]-h[el4[2]]+h[el4[3]];
        a[1]=  h[el4[0]]+h[el4[1]]-h[el4[2]]-h[el4[3]];
        c   =  h[el4[0]]-h[el4[1]]+h[el4[2]]-h[el4[3]];
        //a[0]=  h[0]-h[1]-h[2]+h[3];
        //a[1]=  h[0]+h[1]-h[2]-h[3];
        //c   =  h[0]-h[1]+h[2]-h[3];

        gerade=dabs(c) <= eps0;
        if (! gerade) {a[0]=a[0]/c; a[1]=a[1]/c;}

        i1=el4[3];
        for (i=0; i<4 ;i++) //Kanten auf Schnittpunkt absuchen
        { i2=el4[i];
          if (nix[i]) goto End_Kant; //Linie an Kante schon gezeichnet

          t1=h[i2]-h1;  t2=h1-h[i1];     //Differenzen auf Kante
          if (t1*t2 <0.0) goto End_Kant; //kein Schnittpunkt
          d=h[i2]-h[i1];
          if (dabs(d) <= eps0)      //gesamte Kante ist HSL
          {  plot (x[i1],y[i1],3,width,height,dc); //move_to
             plot (x[i2],y[i2],2,width,height,dc); //draw_to
             goto End_Kant;
          }

// h1-Linie schneidet Kante
          t1=t1/d; t2=t2/d; //nat. Koord. bezÅglich Kante
          for (l=0; l<2; l++)
          {  p[l] = r4[l][md1(i+3,4)]*t1+r4[l][i]*t2; //r,s Schnittpunkt
             r[l]=p[l]; q[l]=p[l]+a[1-l];      // Zielpunkt
          }

          // normales Zeichnen - keine Hyperbeln
          plot4k (0, r, el4, x, y, r4,dc,width,height); //move_to Schnittpunkt
                                        ;//nach r-s Koordinate (r)

          j=0; k=1; // vorl. Annahme: r hat Betrag 1.
          if (gerade)  // --- c=0.0:es gibt nur Geraden ---
          { if (dabs(a[0])  > eps) goto Test_a2;
            if (dabs(a[1]) <= eps) goto End_Kant;

// a1=0.0: beliebiges r, s=const, gerade auch in x-y
xy_gerade:  r[j]=-r[j];              //Ziel auf Gegenseite
            plot4k (1,r,el4,x,y,r4,dc,width,height);
            goto Gegenueber;

// a2=0.0: beliebiges s, r=const, ebenfalls gerade in x-y-system
Test_a2:    if (dabs(a[1]) <= eps) {j=1; goto xy_gerade;} //wie oben, s statt r

// a1 und a2  .ne. 0.0, c=0.0
            if (dabs(p[0]) < dabs(p[1])) j=1; // s hat Betrag 1.
            k=1-j;                          // die andere Koordinate
// direkt auf Gegenseite zugehen
            r[j] = -r[j];   r[k] = (p[j]-r[j])*a[j]/a[k] +p[k];
            if (dabs(r[k]) <= 1.0-eps) // Kurve in r-s, stueckweise
            {  plot4k (nstep,r,el4,x,y,r4,dc,width,height);
               goto Gegenueber;
            }

            r[k]= sign(1.0,r[k]);
            r[j]= (p[k]-r[k])*a[k]/a[j] +p[j];
            plot4k (nstep,r,el4,x,y,r4,dc,width,height);
            goto Nachbarseite;
          } // end if (gerade)

// ----- c von Null verschieden: es gibt Hyperbeln im r-s-System -----
          if (dabs(p[0])+eps < 1.0) j=1; k=1-j;
          z=q[0]*q[1];

          if (dabs(q[j]) <= eps)
// z/Nenner = 0./0. :gerade r=p oder s=q, gerade im x-y-system
          {  r[k]=-p[k]; plot4k (1,r,el4,x,y,r4,dc,width,height);
             goto End_Kant;
          }

          if (dabs(q[k]) <= eps)
// auf anderer Asymptote
          {  r[j]=-1.0;  plot4k (0,r,el4,x,y,r4,dc,width,height);
             r[j]= 1.0;  plot4k (1,r,el4,x,y,r4,dc,width,height);
             goto End_Kant;
          }

// echte Hyperbel, Asymptoten bei r=-a2 bzw. s=-a1
// Plotten ueber die Parameterdarstellung in Teilen
// Aenderung von t aus Ableitung  dr[j]/ dt= e**t *q[j]
          t = 0.0; dt= sign (1./nstep, -q[j]*p[j]); // max. nstep TeilstÅcke

Hyperbel: t +=dt;  et=exp(t);
          r[j]=q[j]*et -a[k];  if (dabs(r[j]) > 1.0+eps) goto Rest_G;
          r[k]=q[k]/et -a[j];  if (dabs(r[k]) > 1.0+eps) goto Rest_N;
          plot4k (1,r,el4,x,y,r4,dc,width,height);
          goto Hyperbel;

// Gegenkante erreicht
Rest_G:   r[j]=sign(1.0,r[j]);
          r[k]=z/(a[k]+r[j])-a[j]; if (dabs(r[k]) > 1.0+eps) goto Rest_N;
          plot4k (1,r,el4,x,y,r4,dc,width,height);
          goto Gegenueber;

//Nachbarkante erreicht
Rest_N:   r[k]=sign(1.0,r[k]);
          r[j]=z/(a[j]+r[k])-a[k];
          plot4k (1,r,el4,x,y,r4,dc,width,height);

Nachbarseite:   // in nix merken!
          if (i<3)
          { if ((r4[k][i]+r4[k][i+1])*r[k] > 1.0) nix[i+1]=1;
          }
          if (i==0)
          { if ((r4[k][2]+r4[k][  3])*r[k] > 1.0) nix[i+3]=1;
          }
          goto End_Kant;

Gegenueber:
          if (i<2) nix[i+2]=1;

End_Kant: i1=i2;
         } // for i...
      }  goto Neue_HSL;
}  // end ISO4K



//C ALLE HOEHENSCHICHTLINIEN EINES ELEMENTS ZEICHNEN, DREIECKE
//C***            ICOL1    (I,ARG) : FARBE FUER HSL =< 0.5*(H1+H2)
//C***                               (GROESSERE LINIEN MIT ICOL1+1)
void iso3k (double h[3],double x[3],double y[3],\
            double h1, double hd, double h2,\
            CClientDC *dc,int width,int height)
{
      int i,i1,i2,ipen;      
      double epsh=1.E-8;
      double h05,hh;
      double r1,r2;

      h05=0.5*(h1+h2);

      double hmin=h[0], hmax= h[0];
      for(i=0;i<3;i++) {
         hmin=min(hmin,h[i]);// extreme h
         hmax=max(hmax,h[i]);// im Element
      }

      hh=h1-hd;

Marke10:   
      hh=hh+hd;
      if (hh > h2)   return;
      if (hh > hmax) return;
      if (hh <= hmin) goto Marke10;
      //if (HH.GT.H05) CALL NEWPEN(ICOL1+1)  //ANDRE FARBE

// HH-LINIE GEHT DURCH ELEMENT
      ipen=3;
      i1=2;
      for(i2=0;i2<3;i2++) { //DO 50 I2=1,3
          r1=h[i2]-hh;
          r2=hh-h[i1];
          //if (r1*r2) Marke50,Marke20,Marke30;
          if ((r1*r2)<0.0) goto Marke50;
          else if((r1*r2)==0.0) goto Marke20;
          else if((r1*r2)>0.0) goto Marke30;
// EINE DIFFERENZ 0.0
Marke20:   
          if (dabs(r2) > epsh) goto Marke50;
          r1=1.0;
          goto Marke40;
// HH-LINIE SCHNEIDET KANTE
Marke30:   
          r1=r1/(h[i2]-h[i1]);
          r2=r2/(h[i2]-h[i1]);
Marke40:   
          plot (r1*x[i1]+r2*x[i2],r1*y[i1]+r2*y[i2],ipen,width,height,dc);
      //CALL PLOT (R1*XY(1,I1)+R2*XY(1,I2),R1*XY(2,I1)+R2*XY(2,I2),IPEN)
      ipen=2;
Marke50:   
      i1=i2;
      }

      goto Marke10;

      return;
}
/**************************************************************************/
/* ATM - Modul: plot.c
                                                                          */
/* Aufgabe:
   Alle Graphische  Ein- und Ausgaberoutinen (X11-Bibiliothek)

                                                                          */
/* Programmaenderungen:
   08/1997     AH           Erste Version
   09/1997     Rene         1D-Kluefte in netz_2d
   10/1997     A. Habbar    Neues Steuerwoert BCURVE (Grafik)   
   03/2003     RK           Quellcode bereinigt, Globalvariablen entfernt   
      
   
   last modified: RK 12.03.2003
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */

/* Preprozessor-Definitionen */
#include "makros.h"

/* Header / Andere intern benutzte Module */
#include "plot.h"
#include "geo_strings.h"
#include "files.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
#include "testvar.h"

#include "indicatr.h"
#include "refine.h"
#include "edges.h"


/* Interne (statische) Deklarationen */

/******************************************************/
/* C1.10 On-line Graphical Output                                 */
/******************************************************/

/* Grafik: #GRAF, #BCURVE, #MESH2D, #MESH3D, #GRAF_EXTD */

int bcurve = 0;                           /* Grafik-Schalter fuer 1D-Durchbr.*/
int bcurve_coor = 0;                      /* x-index */
int bcurve_value = 0;                     /* y-index */
double bcurve_xmin = 0.0;                 /* Weltkoordinaten xmin */
double bcurve_xmax = 0.0;                 /* Weltkoordinaten xmax */
double bcurve_ymin = 0.0;                 /* Weltkoordinaten ymin */
double bcurve_ymax = 0.0;                 /* Weltkoordinaten ymax */
int bcurve_cycle = 1;

#ifdef __RFGRAF
/* ah b */
int grafic1d;                             /* Grafik Schalter */
int graf1d_coor, graf1d_value;            /* x-, y-index */
double graf_xmin;                         /* Weltkoordinaten xmin */
double graf_xmax;                         /* Weltkoordinaten xmax */
double graf_ymin;                         /* Weltkoordinaten ymin */
double graf_ymax;                         /* Weltkoordinaten ymax */
int graf_cycle;

int mesh2d;                               /* Netz Schalter */
int mesh2d_1c, mesh2d_2c;                 /* x-, y-index */
double mesh2d_xmin;                       /* Weltkoordinaten xmin */
double mesh2d_xmax;                       /* Weltkoordinaten xmax */
double mesh2d_ymin;                       /* Weltkoordinaten ymin */
double mesh2d_ymax;                       /* Weltkoordinaten ymax */
int mesh2d_cycle;


int mesh3d;                               /* Netz Schalter */
int mesh3d_1c, mesh3d_2c;                 /* x-, y-index */
double mesh3d_alpha;                      /* Drehungswinkel um X-Achse */
double mesh3d_beta;                       /* Drehungswinkel um Y-Achse */
double mesh3d_gamma;                      /* Drehungswinkel um Z-Achse */
double mesh3d_xmin;                       /* Weltkoordinaten xmin */
double mesh3d_xmax;                       /* Weltkoordinaten xmax */
double mesh3d_ymin;                       /* Weltkoordinaten ymin */
double mesh3d_ymax;                       /* Weltkoordinaten ymax */
int mesh3d_numbering;                     /* Ausgabe Knotennummer */
int mesh3d_cycle;


int graf_wait;
int graf_clear;
int graf_grid;
double graf_gdx;
double graf_gdy;

int root_win,draw_win,netz_win,out_win,inp_win;
int but[10];
int out_curves[20];


/* #include "rfx11win.c" */
/**************************************************************************/
/* ATM - Modul: x11.c, rf_graf.c
                                                                          */
/* Aufgabe:
   Enthaelt die uebergeordneten Datei- Ein- und Ausgaberoutinen, die
   die Text-Eingabedatei betreffen.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "files.h"
#include "txtinout.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
#include "indicatr.h"
#include "refine.h"
#include "testvar.h"
#include "edges.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

/* #include "x11_win.h" */
enum {white,black,red,green,blue,yellow,cyan,magenta,grey86,grey70,grey35};

/* ----------------------------------------------------------- */
/*                                                             */
/*            X11 ( TEIL 1)                                    */
/*                                                             */
/* ----------------------------------------------------------- */

#define RETURN    XK_Return
#define BACKSPACE XK_BackSpace

static int get_text_width(char *string);
static int get_text_height(char *string);
int load_text_font(char* font_name);
void open_grafic(void);
void close_grafic(void);
int insert_color(char* col_name);
void init_color(void);
int create_base_window(int window_x0, int window_y0,
                       int window_width, int window_height,
                       int fg_col, int bg_col,  char* header, char* label,
                       int parent);
void open_base_window(int window, int xpos, int ypos, int width, int height,
                      int pw);
void map_base_window(int window);
void unmap_base_window(int window);
void destroy_base_window(int window);
void clear_base_window(int window);
void clear_area_base_window(int window, int x, int y,
                            int width, int height);
int set_color(int window, int color);
int wait(void);
int wait_button(void);
int read_pixel(int window, int *x, int *y);
int read_mouse_xy(int window, int *x, int *y);
char read_char(int window, int x, int y);
int read_pline(int window, int *x1, int *y1, int *x2, int *y2); /*Nicht*/
int read_string(int window, int x, int y, char *string, int lenght);
void put_text(int window, int x, int y, char* string, int nstring);
void put_text_win(int window, double x, double y, char* string, int nstring);
void center_text(int window, int x, int y, char* string);
void draw_pixel(int window, int x, int y);
void draw_point(int window, int x, int y);
int wingeo(int window);
int winres(int window, int width, int height);
int winmov(int window, int x, int y);
int setcor(int window, double xmin, double xmax, double ymin, double ymax);
int xpixel(int window, double x);
int ypixel(int window, double y);
double xreal(int window, int xp);
double yreal(int window, int yp);
int linsty(int window, int style);
int linwid(int window, int width);
void gflush(void);
void pline(int window, int x1, int y1, int x2, int y2);
void prect(int window, int x1, int y1, int x2, int y2);
void pbox(int window, int x1, int y1, int x2, int y2);
void pcircle(int window, int x0, int y0, int r);
void fill_pcircle(int window, int x0, int y0, int r);
int point(int window, double x, double y);
int line(int window, double xa, double ya, double xe, double ye);
int rect(int window, double xa, double ya, double xe, double ye);
int poly(int window, double *x, double *y, int n);
void scircle(int window, double x0, double y0, int r);
void sfill_circle(int window, double x0, double y0, int r);
int grid(int window, double dx, double dy);
int reset();
int get_event_window();
int read_gc(int w);
int reset_gc(int w);


/*typedef unsigned long uint;
 */

/*enum {white,black,red,green,blue,yellow,cyan,magenta,grey86,grey70,grey35};
 */
static Display     *display=NULL;
static int          screen;
static unsigned int display_width, display_height;
static Window       root_window;
static Colormap     color_map;
static GC gc;
static depth;

static XEvent            event;
static XWindowAttributes win_attrib;

static char* font_name = "9x15";
static XFontStruct *text_font;

static char* small_font_name = "9x10";

#define MAX_COLORS 16

static unsigned long color_pixel[MAX_COLORS];
static char* color_name[MAX_COLORS];
static int   color_count= 0;


static int Line_Style=0;
static int Line_Width=1;


static int Grid_Width=0;
static int Grid_Style=1;
static int Grid_Status=0;



typedef struct {

  Window       window;
  GC           gc;

  XGCValues    values;

  int foreground_color;
  int background_color;

  int          line_width;
  int          line_style;
  int          draw_mode;

  XEvent    event;

  int window_x0,window_y0;
  unsigned int window_width, window_height;

  int state;

  int window_xminp; int window_yminp;
  double window_skalex; double window_skaley;
  double Welt_xmin; double Welt_xmax; double Welt_ymin; double Welt_ymax;

} base_window;



#define MAXWIN  256

static base_window* base_window_list[MAXWIN];
static int          base_window_count = 0;

static int get_text_width(char *string)
{ XCharStruct overall;
  int ascent, descent, direction;
  XQueryTextExtents(display,text_font->fid,string,(int)strlen(string),
                    &direction,&ascent,&descent,&overall);
  return overall.width;
}



static int get_text_height(char *string)
{ XCharStruct overall;
  int ascent, descent, direction;
  XQueryTextExtents(display,text_font->fid,string,(int)strlen(string),
                    &direction,&ascent,&descent,&overall);
  return ascent+descent;
}



int load_text_font(char* font_name)
{ XFontStruct* fp = XLoadQueryFont(display,font_name);
  if (fp) text_font = fp;

  return (fp != NULL);
}


void open_grafic()
{
    base_window* wp = (base_window *) malloc(sizeof(base_window));

   if ( display != NULL)
    {
          free(wp);
          return;
    }


/* Verbindung zum X Server herstellen */
   if ( (display=XOpenDisplay((char *)getenv("DISPLAY"))) == (Display *) 0 )
    {
          fprintf(stderr,"Cannot Open Display\n");
          exit(1);
    }

    screen = DefaultScreen(display);
    color_map = DefaultColormap(display,screen);
    gc = DefaultGC(display,screen);
    depth = DefaultDepth(display, screen);

/*    XSetForeground(display,gc, BlackPixel(display, screen));  */
/*    XSetBackground(display,gc, WhitePixel(display, screen));  */

    display_width=DisplayWidth(display,screen);
    display_height=DisplayHeight(display,screen);

    root_window=RootWindow(display, screen);

    wp->window=root_window;
    base_window_list[0]=wp;

  if ((text_font = XLoadQueryFont(display,font_name)) == NULL)
    {
      fprintf(stderr,"Error: Cannot load font : %s",font_name);
      abort();
    }
  init_color();

  return;
}

void close_grafic()
{
  int w;
  base_window* wp= base_window_list[0];

  if (display) {
    XCloseDisplay(display);
    for(w=0; w <= base_window_count; w++) {
      wp = base_window_list[w];
      if (wp->gc) free(wp->gc);
      if (wp)     free(wp);
    }
    display = NULL;
   }
}

int insert_color(char* col_name)
{
  int i;
  XColor color;
  char* s;

  for (i=0; i<color_count; i++)
    if(strcmp(color_name[i],col_name) == 0) break;
  if (i < color_count) return i;

  if(color_count == MAX_COLORS) return 0;

  XParseColor(display,color_map,col_name,&color);
  XAllocColor(display,color_map,&color);
  color_pixel[color_count]=color.pixel;

  s=(char *)malloc((int)strlen(col_name)+1);
  strcpy(s,col_name);
  color_name[color_count]=s;

  return color_count++;
}

void init_color(void)
{

/* Vordefinierte Farben */
  insert_color("white");
  insert_color("black");
  insert_color("red");
  insert_color("green");
  insert_color("blue");
  insert_color("yellow");
  insert_color("cyan");
  insert_color("magenta");

  insert_color("grey86");
  insert_color("grey70");
  insert_color("grey35");

  return;
}


int create_base_window(int window_x0, int window_y0,
                     int window_width, int window_height,
                     int fg_col, int bg_col,  char* header, char* label,
                     int parent)
{
    XSetWindowAttributes attr;
    unsigned long mask, valuemask;

    XSizeHints win_sizehints;
    XWMHints win_wmhints;


    base_window* wp = (base_window *) malloc(sizeof(base_window));

    wp->window_x0=window_x0;
    wp->window_y0=window_y0;
    wp->window_width=window_width;
    wp->window_height=window_height;


    valuemask = CWBackPixel | CWBitGravity | CWWinGravity | CWBackingStore;
    attr.background_pixel = color_pixel[bg_col];
    attr.bit_gravity = CenterGravity;
    attr.win_gravity = CenterGravity;
    attr.backing_store = Always;




/*    valuemask = CWBackingStore;
    attr.backing_store = Always; */

/* Das Graphicfenster kreieren */
    wp->window=XCreateWindow(display, base_window_list[parent]->window,
                        wp->window_x0, wp->window_y0,
                        wp->window_width, wp->window_height,1,
                        DefaultDepth(display,screen), InputOutput,
                        DefaultVisual(display,screen),
                        valuemask, &attr);

/*    wp->window=XCreateSimpleWindow(display, base_window_list[parent]->window,
                        wp->window_x0, wp->window_y0,
                        wp->window_width, wp->window_height,2,
                        color_pixel[1],color_pixel[bg_col]);*/

/*   wp->window=XCreateWindow(display, base_window_list[parent]->window,
                        wp->window_x0, wp->window_y0,
                        wp->window_width, wp->window_height,2,
                        color_pixel[1],color_pixel[bg_col]); */


  if (DefaultDepth(display,screen) == 1)
    XSetWindowBackground(display,wp->window, WhitePixel(display, screen));
  else
    XSetWindowBackground(display,wp->window,color_pixel[bg_col]);

/*    mask = GCForeground | GCFunction;
    if (DefaultDepth(display,screen) == 1)
       wp->values.foreground = BlackPixel(display, screen);
    else
       wp->values.foreground = color_pixel[fg_col];
    wp->values.function = GXcopy;
    */
/* Den Graphic Context kreieren */
/*    wp->gc = XCreateGC(display, wp->window, mask, &wp->values);  */
/*
    wp->gc = DefaultGC(display,screen);
    XSetForeground(display,wp->gc, BlackPixel(display, screen));
    XSetBackground(display,wp->gc, WhitePixel(display, screen));
    */

/* Die benoetigten Events selektieren */
    XSelectInput(display, wp->window, ExposureMask | KeyPressMask |
                                   ButtonPressMask | ButtonReleaseMask  |
                                 PointerMotionMask | StructureNotifyMask);


/* Icon Bennenung */
  XSetIconName(display,wp->window,label);

/* Fenstertitel */
  XStoreName(display,wp->window,header);

  win_wmhints.flags = StateHint | InputHint;
  win_wmhints.initial_state = NormalState;
  win_wmhints.input = True;

  win_sizehints.flags = PPosition;
  win_sizehints.x=window_x0;
  win_sizehints.y=window_y0;

/*  XSetWMProperties(display,wp->window,0,0,0,0,&win_sizehints,&win_wmhints,0); */
  XSetWMProperties(display,wp->window,0,0,0,0,0,&win_wmhints,0);

/* Den Graphic Context kreieren */

  /*    mask = GCForeground | GCFunction;
    if (DefaultDepth(display,screen) == 1)
       wp->values.foreground = BlackPixel(display, screen);
    else
       wp->values.foreground = color_pixel[fg_col];
    wp->values.function = GXcopy;
    */
/* Den Graphic Context kreieren */
/*    wp->gc = XCreateGC(display, wp->window, mask, &wp->values);  */
/*
    wp->gc = DefaultGC(display,screen);
    XSetForeground(display,wp->gc, BlackPixel(display, screen));
    XSetBackground(display,wp->gc, WhitePixel(display, screen));
    */

  wp->values.background = color_pixel[bg_col];
  wp->values.foreground = color_pixel[fg_col];
  wp->values.function  = GXcopy;
  wp->values.line_style = LineSolid;
  wp->values.line_width = 1;
  wp->values.font = text_font->fid;

  wp->gc = XCreateGC(display,RootWindow(display,screen),
                    GCBackground | GCForeground |GCFunction |
                    GCLineStyle  | GCLineWidth  |GCFont,
                    &(wp->values));



  XSetLineAttributes(display,wp->gc,1,LineSolid,CapButt,JoinMiter);


/* Font setzen */
   XSetFont(display, wp->gc, text_font->fid);


/* Forder-,Hintergrundfarbe */
    wp->foreground_color=fg_col;
    wp->background_color=bg_col;


/* Liniendicke und -staerke setzen */
    wp->line_width=1;
    wp->line_style=0;


    wp->state=0;
    base_window_list[++base_window_count] = wp;

/* Skalierungsfaktoren und Weltkoordinaten setzen */
    wp->window_skalex=1.0;
    wp->window_skaley=1.0;

    wp->Welt_xmin=0.;  wp->Welt_xmax=(double)wp->window_width-1.0;
    wp->Welt_ymin=0.;  wp->Welt_ymax=(double)wp->window_height-1.0;
    setcor(base_window_count,wp->Welt_xmin,wp->Welt_xmax,
                               wp->Welt_ymin,wp->Welt_ymax);

    gflush();
    return base_window_count;
}


void open_base_window(int window, int xpos, int ypos, int width, int height, int pw)
{
  base_window* wp = base_window_list[window];
  XSizeHints size_hints;

  if (wp->state) return;

  if (pw)
    XReparentWindow(display,wp->window,base_window_list[pw]->window,xpos,ypos);

  XMoveResizeWindow(display,wp->window,xpos,ypos,width,height);

  size_hints.flags = PPosition;
  size_hints.x = xpos;
  size_hints.y = ypos;

  XSetWMProperties(display,wp->window,0,0,0,0,&size_hints,0,0);

  XMapWindow(display,wp->window);
  gflush();

  wp->state = 1;

  /*while (XCheckTypedEvent(display,Expose,&event));*/
/*
  if (! XDoesBackingStore(XScreenOfDisplay(display,screen)))
  {
    do
       XNextEvent(display, &event);
    while (event.type != Expose);
   }
   */
/*
   while (XCheckMaskEvent(display,
                         KeyPressMask    |  PointerMotionMask  |
                       ButtonPressMask | ButtonReleaseMask  |
                      ExposureMask    | StructureNotifyMask, &event));
                      */
/*  XNextEvent(display, &e); */

   while (1)
    {
      XNextEvent(display, &event);
      switch (event.type) {
          case Expose:
          case MotionNotify:
          case ConfigureNotify:
             return;
          default:;
        }
    }

  /*
    do
       XNextEvent(display, &event);
    while (event.type != Expose);
    */

}




void map_base_window(int window)
{
  base_window* wp = base_window_list[window];

  if (!wp) return;

  if (wp->state) return;

 /* Graphicfenster sichbar machen */
    XMapWindow(display,wp->window);
    wp->state=1;
    gflush();

    return;
}


void unmap_base_window(int window)
{
  base_window_list[window]->state = 0;
  XUnmapWindow(display,base_window_list[window]->window);
}

void destroy_base_window(int window)
{
  base_window* wp= base_window_list[window];

  XDestroyWindow(display,wp->window);
  if (wp->gc) free(wp->gc);
  if (wp)     free(wp);
  base_window_list[window] = base_window_list[base_window_count--];
}

void clear_base_window(int window)
{
  base_window* wp = base_window_list[window];
  XClearWindow(display,wp->window);
}

void clear_area_base_window(int window, int x, int y,
                                        int width, int height)
{
  base_window* wp = base_window_list[window];
  XClearArea(display,wp->window,x,y,width,height,False);
}



int set_color(int window, int color)
{
  base_window* wp= base_window_list[window];
  int save = wp->foreground_color;

  wp->foreground_color = color;
  wp->values.foreground = color_pixel[color];
/*  if (wp->mode == xor_mode) wp->gc_val.foreground  ^= color_pixel[white]; */
  XChangeGC(display,wp->gc,GCForeground,&(wp->values));
  return save;
 }


wait()
{
   while (1)
    {
      XNextEvent(display, &event);
      switch (event.type) {
          case KeyPress:
             return;
          default:;
        }
    }
}


wait_button()
{
   while (1)
    {
      XNextEvent(display, &event);
      switch (event.type) {
          case ButtonPress:
             return;
          default:;
        }
    }
}



int read_pixel(int window, int *x, int *y)
{
  base_window* wp = base_window_list[window];
  int button=0;

  while (1)
    {
      XNextEvent(display, &event);
      if (event.xany.window == wp->window)
        switch (event.type) {
          case ButtonPress:
                 *x = event.xkey.x;
                 *y = event.xkey.y;
                 button=event.xbutton.button;
                 return button;
                 break;

          default:;
        }
    }
}


int read_mouse_xy(int window, int *x, int *y)
{
  base_window* wp = base_window_list[window];
  int button=0;

  while (1)
    {
      XNextEvent(display, &event);
      if (event.xany.window == wp->window)
        switch (event.type) {
          case ButtonPress:
                 *x = event.xkey.x;
                 *y = event.xkey.y;
                 button=event.xbutton.button;
                 return button;
                 break;
          default:;
        }
    }
}


char read_char(int window, int x, int y)
{
  base_window* wp = base_window_list[window];
  char c=0;
  KeySym     keysym;

  while (1)
    {
      XNextEvent(display, &event);
      if (event.xany.window == wp->window)
        switch (event.type) {
          case KeyPress:
                 XLookupString(&event.xkey, &c, 1, &keysym, 0);
                 if (keysym == RETURN) c='\n';
                 if (keysym == BACKSPACE) c='\b';
                 return c;
                 /*return (c) ? c : keysym;*/
                 break;

          default:;
        }
    }
}


/* noch nicht lauffaehig */
int read_pline(int window, int *x1, int *y1, int *x2, int *y2)
{
  base_window* wp = base_window_list[window];

  XGCValues gc_values;
  gc_values.function = GXxor;
  gc_values.foreground ^= BlackPixel(display,screen);
  gc_values.line_style = LineSolid;
  gc_values.line_width = 1;
  XChangeGC(display,base_window_list[window]->gc,
            GCForeground|GCFunction|GCLineStyle|GCLineWidth,&gc_values);
  gflush();

  while (1)
    {
      XNextEvent(display, &event);
      if (event.xany.window == wp->window)
      switch (event.type) {
          case MotionNotify:
/* Die alte linie wird geloescht */
             pline(window, *x1, *y1, *x2, *y2);
/* Die neue linie wird gezeichnet */
             *x2 = event.xbutton.x;
             *y2 = event.xbutton.y;
             pline(window, *x1, *y1, *x2, *y2);
             break;

          case ButtonPress:
/* Linieursprung wird festgelegt */
             *x2 = event.xbutton.x;
             *y2 = event.xbutton.y;
             *x1 = *x2;
             *y1 = *y2;
             break;
          default:
             break;
        }
    }
  XChangeGC(display,wp->gc,
            GCForeground|GCFunction|GCLineStyle|GCLineWidth,&(wp->values));
  gflush();


}



int read_string(int window, int x, int y, char *string, int lenght)
{
  base_window* wp = base_window_list[window];
  int n=0;
  char c=0;

  while ( n<lenght && (c=read_char(window,x,y))!='\n' )
    {
     if(c == '\b') {
        if(n!=0) {
        string[n-1]=' ';
/*        clear_area_base_window(window,x,y,get_text_width(string),
                                          get_text_height(string)); */
        put_text(window,x,y,string,n);
        string[n]='\0';
        --n;
        }
     }
     else string[n++]=c;

/*     clear_base_window(window);     */
     put_text(window,x,y,string,n);
    }
  return n;
}


void put_text(int window, int x, int y, char* string, int nstring)
{
  base_window* wp = base_window_list[window];
  XSetFont(display,wp->gc,text_font->fid);
  y += text_font->ascent;
  if (nstring > (int)strlen(string)) nstring = (int)strlen(string);
  XDrawString(display,wp->window,wp->gc,x,y,string,nstring);

}


void put_text_win(int window, double x, double y, char* string, int nstring)
{
  put_text(window,xpixel(window,x),ypixel(window,y),string,nstring);
}


void center_text(int window, int x, int y, char* string)
{
  x -= get_text_width(string)/2;
  y -= get_text_height(string)/2;
  put_text(window,x,y,string,(int)strlen(string));
}



void draw_pixel(int window, int x, int y)
{
  base_window* wp = base_window_list[window];

  XDrawPoint(display,wp->window,wp->gc,x,y);
}

void draw_point(int window, int x, int y)
{
  base_window* wp = base_window_list[window];

  XDrawPoint(display,wp->window,wp->gc,x,y);
  XDrawPoint(display,wp->window,wp->gc,x-2,y-2);
  XDrawPoint(display,wp->window,wp->gc,x-1,y-1);
  XDrawPoint(display,wp->window,wp->gc,x+1,y+1);
  XDrawPoint(display,wp->window,wp->gc,x+2,y+2);
  XDrawPoint(display,wp->window,wp->gc,x-2,y+2);
  XDrawPoint(display,wp->window,wp->gc,x-1,y+1);
  XDrawPoint(display,wp->window,wp->gc,x+1,y-1);
  XDrawPoint(display,wp->window,wp->gc,x+2,y-2);
}



wingeo(int window)
{
  base_window* wp = base_window_list[window];
  XWindowAttributes win_attrib;

  XGetWindowAttributes(display,wp->window,&win_attrib);
  wp->window_x0 = win_attrib.x;
  wp->window_y0 = win_attrib.y;
  wp->window_width=win_attrib.width;
  wp->window_height=win_attrib.height;
}


winres(int window, int width, int height)
{
  base_window* wp = base_window_list[window];

  XResizeWindow(display, wp->window, width, height);
}



winmov(int window, int x, int y)
{
  base_window* wp = base_window_list[window];

  XMoveWindow(display, wp->window, x, y);
}

setcor(int window, double xmin, double xmax, double ymin, double ymax)
{
  base_window* wp = base_window_list[window];

  wp->Welt_xmin=xmin; wp->Welt_xmax=xmax;
  wp->Welt_ymin=ymin; wp->Welt_ymax=ymax;

  wp->window_skalex=( (double)wp->window_width ) / (wp->Welt_xmax-wp->Welt_xmin);
  wp->window_skaley=( (double)wp->window_height ) / (wp->Welt_ymax-wp->Welt_ymin);

  wp->window_xminp = (int) ( - xmin * wp->window_skalex);
  wp->window_yminp = (int) ( wp->window_height + ymin * wp->window_skaley);
}



int xpixel(int window, double x)
{
  base_window* wp = base_window_list[window];

  return (int)( wp->window_xminp + x * wp->window_skalex);
}

int ypixel(int window, double y)
{
  base_window* wp = base_window_list[window];

  return (int)( wp->window_yminp - y * wp->window_skaley);
}


double xreal(int window, int xp)
{
  base_window* wp = base_window_list[window];

  return (double)( (xp - wp->window_xminp) / wp->window_skalex);
}

double yreal(int window, int yp)
{
  base_window* wp = base_window_list[window];

  return (double)( (wp->window_yminp - yp) / wp->window_skaley);
}


linsty(int window, int style)
{
  base_window* wp = base_window_list[window];

  int line_style=wp->line_style;
  unsigned int line_width=wp->line_width;

  if ( style == 0 ) {
     wp->line_style = style;
     line_style = LineSolid;
  }
  else if ( style == 1 ) {
     wp->line_style = style;
     line_style = LineOnOffDash;
  }

  XSetLineAttributes(display, wp->gc, line_width, line_style, 0, 0);
}


linwid(int window, int width)
{
  base_window* wp = base_window_list[window];

  int with_store=wp->line_width;

  wp->line_width=width;
  wp->values.line_width=width;
  XChangeGC(display,wp->gc,GCLineWidth,&wp->values);
  return with_store;
}

void gflush(void)
{
    XFlush(display);
}

void pline(int window, int x1, int y1, int x2, int y2)
{
  base_window* wp = base_window_list[window];
  set_color(window,wp->foreground_color);
  XDrawLine(display,wp->window,wp->gc,x1,y1,x2,y2);
}


void prect(int window, int x1, int y1, int x2, int y2)
{
  base_window* wp = base_window_list[window];
  int x,y;

  if (x1 > x2)
  { x = x1; x1 = x2; x2 = x; }
  if (y1 > y2)
  { y = y1; y1 = y2; y2 = y; }

  XDrawRectangle(display,wp->window,wp->gc,x1,y1,x2-x1,y2-y1);
}


void pbox(int window, int x1, int y1, int x2, int y2)
{
  base_window* wp = base_window_list[window];

  if (x1 > x2)
  { int x = x1;
    x1 = x2;
    x2 = x;
   }
  if (y1 > y2)
  { int y = y1;
    y1 = y2;
    y2 = y;
   }
  XFillRectangle(display,wp->window,wp->gc,x1,y1,x2-x1+1,y2-y1+1);
}


void pcircle(int window, int x0, int y0, int r)
{
  base_window* wp = base_window_list[window];

  XDrawArc(display,wp->window,wp->gc,x0-r,y0-r,2*r,2*r,0,360*64);
}


void fill_pcircle(int window, int x0, int y0, int r)
{
  base_window* wp = base_window_list[window];

  XFillArc(display,wp->window,wp->gc,x0-r,y0-r,2*r,2*r,0,360*64);
}



point(int window, double x, double y)
{
    draw_point(window, xpixel(window,x), ypixel(window,y));
}


line(int window, double xa, double ya, double xe, double ye)
{
    pline(window, xpixel(window,xa), ypixel(window,ya),
                  xpixel(window,xe), ypixel(window,ye) );
}


rect(int window, double xa, double ya, double xe, double ye)
{
    prect(window, xpixel(window,xa), ypixel(window,ya),
                  xpixel(window,xe), ypixel(window,ye) );
}


poly(int window, double *x, double *y, int n)
{
    int i;
    reset();

    for (i=0; i<n-1; i++)
    pline(window, xpixel(window,x[i]), ypixel(window,y[i]),
                  xpixel(window,x[i+1]), ypixel(window,y[i+1]) );
}


void scircle(int window, double x0, double y0, int r)
{
  pcircle(window, xpixel(window,x0), ypixel(window,y0), r);
}

void sfill_circle(int window, double x0, double y0, int r)
{
  fill_pcircle(window, xpixel(window,x0), ypixel(window,y0), r);
}


grid(int window, double dx, double dy)
{
 base_window* wp = base_window_list[window];

 double x,y;
 double sx=dx,sy=dy;
 int s_Line_Style=wp->line_style;
 int s_Line_Width=wp->line_width;





void put_text(int, int, int, char*, int);
void gflush(void);
void pline(int, int, int, int, int);
double xreal(int, int);
double yreal(int, int);

/* if ( Grid_Status == 0 ) return; */

 linsty(window,Grid_Style);
 linwid(window,Grid_Width);


 for (x=wp->Welt_xmin; x<wp->Welt_xmax; x+=sx )
   for (y=wp->Welt_ymin; y<wp->Welt_ymax; y+=sy )
     {
        line(window,wp->Welt_xmin,y,wp->Welt_xmax,y);
        line(window,x,wp->Welt_ymin,x,wp->Welt_ymax);
     }

        line(window,wp->Welt_xmin,y,wp->Welt_xmax,y);
        line(window,x,wp->Welt_ymin,x,wp->Welt_ymax);

 rect(window,wp->Welt_xmin,wp->Welt_ymin,wp->Welt_xmax,wp->Welt_ymax);

 wp->line_style=s_Line_Style;
 wp->line_width=s_Line_Width;
 reset_gc(window);

}


reset()
{
  base_window* wp = base_window_list[0];
  Window window;
  int i=base_window_count;

  if (XCheckMaskEvent(display,StructureNotifyMask,&event) == 0) return;
  else {
    window = event.xany.window;
    base_window_list[0]->window = window;
    while (base_window_list[i]->window != window) i--;
    base_window_list[0]->window = root_window;
    wp = base_window_list[i];

    wingeo(i);
    setcor(i,wp->Welt_xmin,wp->Welt_xmax,wp->Welt_ymin,wp->Welt_ymax);
  }
}


int get_event_window()
{
  base_window* wp = base_window_list[0];
  Window window;
  int i=base_window_count;

  if (XCheckMaskEvent(display,ButtonPressMask,&event) == 0) return -1;
  else {
    window = event.xany.window;
    base_window_list[0]->window = window;
    while (base_window_list[i]->window != window) i--;
    base_window_list[0]->window = root_window;
    wp = base_window_list[i];
/*    XPutBackEvent(display,&event);  */
    return i;
  }
}



int read_gc(int w)
{
  XGCValues gc_values;
  gc_values.function = GXxor;
  gc_values.foreground = BlackPixel(display,screen);
  gc_values.line_style = LineSolid;
  gc_values.line_width = 1;
  XChangeGC(display,base_window_list[w]->gc,
            GCForeground|GCFunction|GCLineStyle|GCLineWidth,&gc_values);
  gflush();
}

int reset_gc(int w)
{
  base_window* wp = base_window_list[w];

  XChangeGC(display,wp->gc,
            GCForeground|GCFunction|GCLineStyle|GCLineWidth,&(wp->values));
  gflush();
}


/* ----------------------------------------------------------- */
/*                                                             */
/*            R F G R A F ( TEIL 2)                            */
/*                                                             */
/* ----------------------------------------------------------- */

int open_rf_window(int width, int height,
                   double xmin, double xmax, double ymin, double ymax);
void close_rf_window(int window);
int rf_line(int window, double xa, double ya, double xe, double ye);
int rf_clear(int window);
int rf_text(int window, int x, int y, char *string, int lenght);
int rf_text_win(int window, double x, double y, char *string, int lenght);
int rf_read_string(int window, int x, int y, char *string, int lenght,
                   char *text);
int rf_grid(int window, double dx, double dy);
int rf_coor(int window, double xmin, double xmax, double ymin, double ymax);
int rf_read_mouse_xy(int window, double *x, double *y);
int open_rf_button(int parent, int x, int y,
                   int width, int height, char *string);
void close_rf_button(int button);
int open_rf_root_window(int x, int y, int width, int height);
int open_rf_draw_window(int parent, int x, int y, int width, int height,
                        double xmin, double xmax, double ymin, double ymax);
int open_rf_out_window(int parent, int x, int y, int width, int height);
int open_rf_inp_window(int parent, int x, int y, int width, int height);
int stop_rf_window(int parent);
int rf_reset(void);
int rf_grafic(int flag);
int init_rf_windows(void);
int close_rf_windows(void);
int set_param(void);
int set_stop(void);
int stop_if_time(double cur_time);

static int RF_GRAF=0;
static int RF_INIT=0;

#define MAXRFWIN 32
/*#define MAXRFBUT 16*/

static int rf_window_list[MAXRFWIN];
static int rf_window_count = 0;

/*
static int rf_button_list[MAXRFBUT];
static int rf_button_count = 0;
*/

int root_win,draw_win,netz_win,out_win,inp_win;
int but[10];
int iwin;


static double Welt_xmin; double Welt_xmax; double Welt_ymin; double Welt_ymax;
static stop_time;
int rf_stop=0;


int open_rf_window(int width, int height,
                   double xmin, double xmax, double ymin, double ymax)
{
  int win;

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  win=create_base_window(0,0,width,height,
                      white,white,"ROCKFLOW","ROCKFLOW",0/*root_window*/);
  open_base_window(win,0,0,width,height,0/*root_window*/);


/*  map_base_window(win); */
  setcor(win,xmin,xmax,ymin,ymax);
  clear_base_window(win);
/*  reset_gc(win); */
  gflush();

  rf_window_list[rf_window_count]=win;

  return ++rf_window_count;
}


void close_rf_window(int window)
{
  int win;

/*  if ( !(window>=0 && window<rf_window_count) ) return; */

  win=rf_window_list[window-1];
/*  if ( !(win>0) ) return; */
  unmap_base_window(win);
  rf_window_count--;

  if(rf_window_count==0) close_grafic();
}




rf_line(int window, double xa, double ya, double xe, double ye)
{
  int win=rf_window_list[window-1];

  reset();
  line(win,xa,ya,xe,ye);
  gflush();
}

rf_clear(int window)
{
  int win=rf_window_list[window-1];

  clear_base_window(win);
  gflush();
}

rf_text(int window, int x, int y, char *string, int lenght)
{
  int win=rf_window_list[window-1];

  reset();
  put_text(win,x,y,string,lenght);
  gflush();
}

rf_text_win(int window, double x, double y, char *string, int lenght)
{
  int win=rf_window_list[window-1];

  reset();
  put_text_win(win,x,y,string,lenght);
  gflush();
}



rf_read_string(int window, int x, int y, char *string, int lenght, char *text)
{
  int win=rf_window_list[window-1];
  int text_lenght=get_text_width(text);

  reset();
  put_text(win,x,y,text,(int)strlen(text));
  read_string(win,x+text_lenght+2,y,string,lenght);
  gflush();
}



rf_grid(int window, double dx, double dy)
{
  int win=rf_window_list[window-1];

  reset();
  grid(win,dx,dy);
  gflush();
}


rf_coor(int window, double xmin, double xmax, double ymin, double ymax)
{
  int win=rf_window_list[window-1];

  reset();
  setcor(win,xmin,xmax,ymin,ymax);
  gflush();
}



rf_read_mouse_xy(int window, double *x, double *y)
{
  int win=rf_window_list[window-1];
  int xp,yp;

  reset();
  read_mouse_xy(win,&xp,&yp);
  *x=xreal(win,xp);
  *y=yreal(win,yp);
  gflush();

  return 1;
}


int open_rf_button(int parent, int x, int y,
                   int width, int height, char *string)
{
  int but,pwin;

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  pwin=rf_window_list[parent-1];
  but=create_base_window(x,y,width,height,
                      black,grey86,"ROCKFLOW","ROCKFLOW",
                      pwin/*parent_window*/);
  open_base_window(but,x,y,width,height,pwin/*parent_window*/);
  clear_base_window(but);
/*  rect(but,0.,0.,(double)width,(double)height); */
  center_text(but,width/2,height/2,string);
  gflush();

  rf_window_list[rf_window_count]=but;
  return ++rf_window_count;
}


void close_rf_button(int button)
{
  int but;

  if ( !(but>0) ) return;
  but=rf_window_list[button-1];
  unmap_base_window(but);
  rf_window_count--;

}


int open_rf_root_window(int x, int y, int width, int height)
{
  int win;

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  win=create_base_window(x,y,width,height,
                      black,white,"ROCKFLOW","ROCKFLOW",0/*root_window*/);
  open_base_window(win,x,y,width,height,0/*root_window*/);


/*  map_base_window(win); */
  clear_base_window(win);
  gflush();

  rf_window_list[rf_window_count]=win;

  return ++rf_window_count;
}

int open_rf_draw_window(int parent, int x, int y, int width, int height,
                        double xmin, double xmax, double ymin, double ymax)
{
  int win,parent_win;

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  parent_win=rf_window_list[parent-1];
  win=create_base_window(x,y,width,height,
                         black,white/*grey86*/,"ROCKFLOW","ROCKFLOW",
                      parent_win/*root__window*/);
  open_base_window(win,x,y,width,height,parent_win/*parent_window*/);

  clear_base_window(win);
/*  rect(win,0.,0.,(double)width,(double)height); */
  setcor(win,xmin,xmax,ymin,ymax);
  gflush();

  rf_window_list[rf_window_count]=win;
  return ++rf_window_count;
}


int open_rf_out_window(int parent, int x, int y, int width, int height)
{
  int win,parent_win;

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  parent_win=rf_window_list[parent-1];
  win=create_base_window(x,y,(unsigned long)width,(unsigned long)height,
                      black,white,"ROCKFLOW","ROCKFLOW",
                      parent_win/*parent__window*/);
  open_base_window(win,x,y,width,height,parent_win/*parent_window*/);

  clear_base_window(win);
/*  rect(win,0.,0.,(double)width,(double)height); */
  gflush();

  rf_window_list[rf_window_count]=win;
  return ++rf_window_count;
}

int open_rf_inp_window(int parent, int x, int y, int width, int height)
{
  int win,parent_win;
  char string[80];

  if (RF_GRAF==0) {
    open_grafic();
    RF_GRAF=1;
  }
  if(rf_window_count == MAXRFWIN) return 0;
  parent_win=rf_window_list[parent-1];
  win=create_base_window(x,y,(unsigned long)width,(unsigned long)height,
                      black,white,"ROCKFLOW","ROCKFLOW",
                      parent_win/*parent__window*/);
  open_base_window(win,x,y,width,height,parent_win/*parent_window*/);

  clear_base_window(win);
/*  rect(win,0.,0.,(double)width,(double)height); */
  gflush();

/*  read_string(win,5,5,string,80); */

  rf_window_list[rf_window_count]=win;
  return ++rf_window_count;
}


int stop_rf_window(int parent)
{
  int win,window;

  win=rf_window_list[parent-1];

  window=get_event_window();
  if ( win == window ) wait();

  return 1;
}



int rf_reset(void)
{
  int base_window,button=-1,rf_win=-1;
  double x1,y1,x2,y2,x,y;
  char string[80];

  if( (base_window=get_event_window()) > 0) {

     rf_win=rf_window_list[base_window-1];

     if (rf_win >= 0 && rf_win < rf_window_count) {
        if (rf_win == root_win) {
         return 1;
        }
        else
        if (rf_win == draw_win) {
          wait_button();
          return 1;
        }
        else
        if (rf_win == but[0]) {
          set_stop();
          return 1;
        }
        else
        if (rf_win == but[1]) {
          rf_read_mouse_xy(draw_win,&x1,&y1);
          point(draw_win,x1,y1);
          rf_read_mouse_xy(draw_win,&x2,&y2);
          point(draw_win,x2,y2);
          if (x1 > x2)
            { x = x1; x1 = x2; x2 = x; }
          if (y1 > y2)
            { y = y1; y1 = y2; y2 = y; }
          rf_coor(draw_win,x1,x2,y1,y2);
          rf_clear(draw_win);
          return 1;
        }
        else
        if (rf_win == but[2]) {
          rf_read_mouse_xy(netz_win,&x1,&y1);
          point(netz_win,x1,y1);
          rf_read_mouse_xy(netz_win,&x2,&y2);
          point(netz_win,x2,y2);
          if (x1 > x2)
            { x = x1; x1 = x2; x2 = x; }
          if (y1 > y2)
            { y = y1; y1 = y2; y2 = y; }
          rf_coor(netz_win,x1,x2,y1,y2);
          rf_clear(netz_win);
          return 1;
        }
        else
        if (rf_win == but[3]) {
          if ( grafic1d>0 || mesh2d>0 || bcurve>0 || mesh3d>0)
          rf_clear(draw_win);
          rf_coor(draw_win,Welt_xmin,Welt_xmax,Welt_ymin,Welt_ymax);
          rf_coor(netz_win,Welt_xmin,Welt_xmax,Welt_ymin,Welt_ymax);
          return 1;
        }
        if (rf_win == but[4]) {
          set_param();
          return 1;
        }
        else
        if (rf_win == but[5]) {
          close_rf_windows();
          grafic1d=0;
          mesh2d=0;
          bcurve=0;
          mesh3d=0;
          return 1;
        }
        else return 1;
     }
  }

}


int rf_grafic(int flag)
{
  if (flag > 0) {
    if (RF_INIT==0){
      if (grafic1d > 0 || mesh2d > 0 || bcurve >0 || mesh3d > 0) {
        init_rf_windows();
        RF_INIT=1;
      }
    }

    if (grafic1d > 0) {
      plot_1d(graf1d_coor,graf1d_value);
    }
    else if (mesh2d > 0) {
      netz_2d(mesh2d_1c,mesh2d_2c);
    }
    else if (mesh3d > 0) {
      netz_3d(mesh3d_1c,mesh3d_2c);
    }
    else if (bcurve > 0) {
      bcurve_1d(bcurve_coor,bcurve_value);
    }
  }
  else {
    if (grafic1d > 0 || mesh2d > 0 || bcurve > 0 || mesh3d > 0) {
      wait();
      /*      close_rf_windows(); */
    }
  }

  return 1;

}


int init_rf_windows(void)
{
 char **titel[]={"STOP","ZOOM C","ZOOM M","RESET","SETTING","QUIT",
                 "ROCKFLOW"};

  if( grafic1d>0 && mesh2d>0 || grafic1d>0 && bcurve>0 \
      || mesh2d>0 && bcurve>0 || grafic1d>0 && mesh3d>0
      || bcurve>0 && mesh3d>0 || mesh2d>0 && mesh3d>0   ) {
         printf("\n Fehler in der Eingabe-Datei *.rfd");
         printf("\n Nur ein Grafik-Schalter darf eingeschaltet werden !!");
         printf("\n Bitte Korrektur vornehmen und nochmal versuchen.\n");
         exit(1);
  }

  root_win=open_rf_root_window(0,0,700,700);
  if (grafic1d > 0) draw_win=open_rf_draw_window(root_win,20,70,650,450,
                          graf_xmin,graf_xmax,graf_ymin,graf_ymax);
  else if (mesh2d > 0) draw_win=open_rf_draw_window(root_win,20,70,650,450,
                          mesh2d_xmin,mesh2d_xmax,mesh2d_ymin,mesh2d_ymax);
  else if (mesh3d > 0) draw_win=open_rf_draw_window(root_win,20,70,650,450,
                          mesh3d_xmin,mesh3d_xmax,mesh3d_ymin,mesh3d_ymax);
  else if (bcurve > 0) draw_win=open_rf_draw_window(root_win,20,70,650,450,
                          bcurve_xmin,bcurve_xmax,bcurve_ymin,bcurve_ymax);

  if (grafic1d > 0) netz_win=open_rf_draw_window(root_win,20,530,650,50, \
                          graf_xmin,graf_xmax,graf_ymin,graf_ymax);
  else if (bcurve > 0) netz_win=open_rf_draw_window(root_win,20,530,650,50, \
                          bcurve_xmin,bcurve_xmax,bcurve_ymin,bcurve_ymax);
  out_win=open_rf_out_window(root_win,20,590,650,60);
  inp_win=open_rf_inp_window(root_win,20,660,650,30);
  for (iwin=0; iwin<=6; iwin++)
    but[iwin]=open_rf_button(root_win,30+(iwin)*90,10,80,50,(char *)titel[iwin]);



/*  if (RF_INIT==0) {
    set_param();
    RF_INIT=1;
  }
  */

 if (grafic1d > 0) {
  Welt_xmin = graf_xmin;                         /* Weltkoordinaten xmin */
  Welt_xmax = graf_xmax;                         /* Weltkoordinaten xmax */
  Welt_ymin = graf_ymin;                         /* Weltkoordinaten ymin */
  Welt_ymax = graf_ymax;                         /* Weltkoordinaten ymax */
 }
 else if (mesh2d > 0) {
  Welt_xmin = mesh2d_xmin;                         /* Weltkoordinaten xmin */
  Welt_xmax = mesh2d_xmax;                         /* Weltkoordinaten xmax */
  Welt_ymin = mesh2d_ymin;                         /* Weltkoordinaten ymin */
  Welt_ymax = mesh2d_ymax;                         /* Weltkoordinaten ymax */
 }
 else if (mesh3d > 0) {
  Welt_xmin = mesh3d_xmin;                         /* Weltkoordinaten xmin */
  Welt_xmax = mesh3d_xmax;                         /* Weltkoordinaten xmax */
  Welt_ymin = mesh3d_ymin;                         /* Weltkoordinaten ymin */
  Welt_ymax = mesh3d_ymax;                         /* Weltkoordinaten ymax */
 }
 else if (bcurve > 0) {
  Welt_xmin = bcurve_xmin;                         /* Weltkoordinaten xmin */
  Welt_xmax = bcurve_xmax;                         /* Weltkoordinaten xmax */
  Welt_ymin = bcurve_ymin;                         /* Weltkoordinaten ymin */
  Welt_ymax = bcurve_ymax;                         /* Weltkoordinaten ymax */
 }


  return 1;
}

int close_rf_windows(void)
{

  for (iwin=0; iwin<=6; iwin++) close_rf_window(but[iwin]);
  close_rf_window(inp_win);
  close_rf_window(out_win);
  close_rf_window(netz_win);
  close_rf_window(draw_win);
  close_rf_window(root_win);

  return 1;
}

int set_param(void)
{
  int set_win;
  char string[80]="                   ";
  int cycle;
  double xmin,xmax,ymin,ymax;

  set_win=open_rf_root_window(0,0,300,300);

  rf_read_string(set_win,5,10,string,80," >> Input time step :" );
  cycle=atoi(string);
  if (cycle > 0)
    if (grafic1d > 0) graf_cycle=cycle;
    else if (mesh2d > 0) mesh2d_cycle=cycle;
    else if (bcurve > 0) bcurve_cycle=cycle;


/*
  rf_read_string(set_win,5,25,string,80," >> Input xmin      :" );
  xmin=atof(string);
  rf_read_string(set_win,5,40,string,80," >> Input xmax      :" );
  xmax=atof(string);
  rf_read_string(set_win,5,55,string,80," >> Input ymin      :" );
  ymin=atof(string);
  rf_read_string(set_win,5,70,string,80," >> Input ymax      :" );
  ymax=atof(string);

  rf_coor(draw_win,xmin,xmax,ymin,ymax);
  rf_coor(netz_win,xmin,xmax,ymin,ymax);
  */

  close_rf_window(set_win);

  return 1;
}

int set_stop(void)
{
  int set_win;
  char string[80]="                   ";
  double s_time;

  set_win=open_rf_root_window(0,0,300,300);

  rf_read_string(set_win,5,10,string,80," >> Input stopping time :" );
  s_time=atof(string);
  if (s_time > stop_time && rf_stop == 0) {
     rf_stop=1;
     stop_time=s_time;
  }
  close_rf_window(set_win);

  return 1;
}



int stop_if_time(double cur_time)
{
  int set_win;
  char string[80]="                   ";

  if (cur_time >= stop_time && rf_stop != 0) {
   rf_stop=0;
   return 1;
  }
  else return 0;
}


/* ----------------------------------------------------------- */
/*                                                             */
/*            R F G R A F ( TEIL 3)                            */
/*                                                             */
/* ----------------------------------------------------------- */


/* Definitionen */

int plotxy ( double *x, double *y, int n );
int plot_1d ( int coord_ind, int value_ind );
int plot_1d_old ( int coord_ind, int value_ind );
int bcurve_1d ( int node_ind, int value_ind );
int netz_2d ( int first_c, int second_c );
int netz_3d ( int first_c, int second_c );
void  dreh3d ( int achse, double winkel,
               double x, double y, double z,
               double *xs, double *ys, double *zs);

/**************************************************************************/
/* ATM - Funktion: plotxy
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   03/1997     AH        Erste Version
                                                                          */
/**************************************************************************/
int plotxy ( double *x, double *y, int n )
{
   int i;
   int ok=1;

/*   if ( graf_clear == 1 ) clear(); */

/*   if ( graf_grid == 1 ) grid(&graf_gdx,&graf_gdy);
*/
/*  Die Funktionswerte y plotten  */
/*   linsty(0); */
   for (i=0;i<n-1;i++)
     rf_line(draw_win,x[i],y[i],x[i+1],y[i+1]);

   if ( graf_wait == 1 ) wait();

  return ok;
}



/**************************************************************************/
/* ATM - Modul: plot.c
                                                                          */
/* Aufgabe:
   Enthaelt die uebergeordneten Datei- Ein- und Ausgaberoutinen, die
   die Text-Eingabedatei betreffen.
                                                                          */
/* Programmaenderungen:
   03/1994     MSR        Erste Version
                                                                          */
/**************************************************************************/

/* Preprozessor-Definitionen */
#include "makros.h"


/* Header / Andere intern benutzte Module */
#include "files.h"
#include "txtinout.h"
#include "nodes.h"
#include "elements.h"
#include "mathlib.h"
#include "indicatr.h"
#include "refine.h"
#include "testvar.h"
#include "edges.h"


/* Interne (statische) Deklarationen */


/* Definitionen */

/**************************************************************************/
/* ATM - Funktion: plotxy
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   08/1997     AH        Erste Version
                                                                          */
/**************************************************************************/
int plot_1d ( int coord_ind, int value_ind )
{
  static long i;
  static int nn;

  static double node1_geo,node2_geo;   /* node geometry */
  static double node1_val,node2_val;  /* Werte an den Knoten */
  static long *element_nodes;

  int ok=1;
  char string[80];
  int stop;

/*  rf_reset(); */
  stop=stop_if_time(aktuelle_zeit);

  if ( grafic1d > 0 ) {
    if ((aktueller_zeitschritt % graf_cycle) == 0l) {
    rf_clear(netz_win);
    if ( graf_clear == 1 ) rf_clear(draw_win);
    if ( graf_grid == 1 ) rf_grid(draw_win,graf_gdx,graf_gdy);
    }
    else return 0;
  }
  else return 0;
/* ah e */


   for (i=0;i<ElListSize();i++)
      if (ElGetElement(i)!=NULL)  /* Element existiert */
         if (ElGetElementActiveState(i)) { /* aktives Element */
           if(ElGetElementType(i)==1) {    /* Nur 1D Elemente */
             element_nodes = ElGetElementNodes(i);
             node1_val  = GetNodeVal(element_nodes[0],value_ind);
             node2_val  = GetNodeVal(element_nodes[1],value_ind);
             if (coord_ind==1) node1_geo = GetNodeX(element_nodes[0]);
             if (coord_ind==1) node2_geo = GetNodeX(element_nodes[1]);
             if (coord_ind==2) node1_geo = GetNodeY(element_nodes[0]);
             if (coord_ind==2) node2_geo = GetNodeY(element_nodes[1]);
             if (coord_ind==3) node1_geo = GetNodeZ(element_nodes[0]);
             if (coord_ind==3) node2_geo = GetNodeZ(element_nodes[1]);

             rf_line(draw_win,node1_geo,node1_val,node2_geo,node2_val);
             rf_line(netz_win,node1_geo,0.,node2_geo,0.);
             point(netz_win,node1_geo,0.5);
             point(netz_win,node2_geo,0.5);

/*             scircle(netz_win,node1_geo,0.,3);
             scircle(netz_win,node2_geo,0.,3); */

             sfill_circle(netz_win,node1_geo,0.,3);
             sfill_circle(netz_win,node2_geo,0.,3);
           }

         }


   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,1,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,16,string,(int)strlen(string));
   sprintf(string,"NodeListLength        : %ld",NodeListLength);
   rf_text(out_win,10,31,string,(int)strlen(string));
   sprintf(string,"anz_active_1D         : %ld",anz_active_1D);
   rf_text(out_win,10,46,string,(int)strlen(string));


  rf_reset();
  if (stop) wait();

  return ok;
}

/**************************************************************************/
/* ATM - Funktion: plotxy
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   08/1997     AH        Erste Version
                                                                          */
/**************************************************************************/
int plot_1d_old ( int coord_ind, int value_ind )
{
   int i;
   int ok=1;
   int n=NodeListLength;
   char string[80];
  static double *node_geo = NULL;    /* node geometry */
  static double *node_value = NULL;  /* node_value = Wert an den Knoten */
  static long *element_nodes;
  static double node1_geo,node2_geo;
  int stop;

/*  rf_reset(); */
  stop=stop_if_time(aktuelle_zeit);

      if ( grafic1d > 0 ) {
         if ((aktueller_zeitschritt % graf_cycle) == 0l) {
           node_geo  = (double *) Malloc(sizeof(double)*NodeListLength);
           node_value = (double *) Malloc(sizeof(double)*NodeListLength);
           for (i=0;i<NodeListLength;i++) {
                  if (coord_ind==1) node_geo[i] = GetNodeX(NodeNumber[i]);
                  if(coord_ind==2)node_geo[i] = GetNodeY(NodeNumber[i]);
                  if(coord_ind==3)node_geo[i] = GetNodeZ(NodeNumber[i]);
                  node_value[i] = GetNodeVal(NodeNumber[i],value_ind);
             printf("\n%d %lf %lf",i,node_geo[i],node_value[i]);
           }

/*        plotxy(node_geo,node_value,NodeListLength);   */
         }
         else return 0;
      }
      else return 0;
/* ah e */

   if ( graf_clear == 1 ) rf_clear(draw_win);

  if ( graf_grid == 1 ) rf_grid(draw_win,graf_gdx,graf_gdy);

/*  Die Funktionswerte y plotten  */
/*   linsty(0); */
   for (i=0;i<n-1;i++)
     rf_line(draw_win,node_geo[i],node_value[i],node_geo[i+1],node_value[i+1]);
/*
   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,5,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,20,string,(int)strlen(string));
   sprintf(string,"NodeListLength        : %ld",NodeListLength);
   rf_text(out_win,10,35,string,(int)strlen(string));
   sprintf(string,"anz_active_1D         : %ld",anz_active_1D);
   rf_text(out_win,10,50,string,(int)strlen(string));
   */

   rf_clear(netz_win);
   for (i=0;i<ElListSize();i++)
      if (ElGetElement(i)!=NULL)  /* Element existiert */
         if (ElGetElementActiveState(i)) { /* aktives Element */
            /* vorlaeufiges Behelfskonstrukt: Setzt Gausspunkte */
           if(ElGetElementType(i)==1) {
             element_nodes = ElGetElementNodes(i);
             if (coord_ind==1) node1_geo = GetNodeX(element_nodes[0]);
             if (coord_ind==1) node2_geo = GetNodeX(element_nodes[1]);
             if (coord_ind==2) node1_geo = GetNodeY(element_nodes[0]);
             if (coord_ind==2) node2_geo = GetNodeY(element_nodes[1]);
             if (coord_ind==3) node1_geo = GetNodeZ(element_nodes[0]);
             if (coord_ind==3) node2_geo = GetNodeZ(element_nodes[1]);
             rf_line(netz_win,node1_geo,0.,node2_geo,0.);
             point(netz_win,node1_geo,0.5);
             point(netz_win,node2_geo,0.5);

/*             scircle(netz_win,node1_geo,0.,3);
             scircle(netz_win,node2_geo,0.,3); */

             sfill_circle(netz_win,node1_geo,0.,3);
             sfill_circle(netz_win,node2_geo,0.,3);
           }

         }


   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,1,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,16,string,(int)strlen(string));
   sprintf(string,"NodeListLength        : %ld",NodeListLength);
   rf_text(out_win,10,31,string,(int)strlen(string));
   sprintf(string,"anz_active_1D         : %ld",anz_active_1D);
   rf_text(out_win,10,46,string,(int)strlen(string));


  rf_reset();
/*   stop_rf_window(draw_win);  */
/*   if ( graf_wait == 1 ) wait();  */

   node_geo = Free(node_geo);
   node_value = Free(node_value);


  if (stop) wait();

  return ok;
}




/**************************************************************************/
/* ATM - Funktion: bcurve_1d
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   08/1997     AH        Erste Version
                                                                          */
/**************************************************************************/
int bcurve_1d ( int node_ind, int value_ind )
{
   int i;
   int ok=1;
   double node_value;
   char string[80];
   int stop;

/*  rf_reset(); */
  stop=stop_if_time(aktuelle_zeit);

      if ( bcurve > 0 ) {
         if ((aktueller_zeitschritt % bcurve_cycle) == 0l) {
           for (i=0;i<NodeListLength;i++)
               if (NodeNumber[i]==node_ind)
                  node_value = GetNodeVal(NodeNumber[i],value_ind);
         }
         else return 0;
      }
      else return 0;
/* ah e */

   if ( graf_clear == 1 ) rf_clear(draw_win);

   if ( graf_grid == 1 ) rf_grid(draw_win,graf_gdx,graf_gdy);

/*  Die Funktionswerte y plotten  */
   point(draw_win,aktuelle_zeit,node_value);

   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,5,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,16,string,(int)strlen(string));
   sprintf(string,"NodeListLength        : %ld",NodeListLength);
   rf_text(out_win,10,31,string,(int)strlen(string));
   sprintf(string,"anz_active_1D         : %ld",anz_active_1D);
   rf_text(out_win,10,46,string,(int)strlen(string));

  rf_reset();
  if (stop) wait();

  return ok;
}


/**************************************************************************/
/* ATM - Funktion: netz_2d
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   08/1997     AH        Erste Version
                                                                          */
/**************************************************************************/
int netz_2d ( int first_c, int second_c )
{
  /* Variablen */
  static long i;  /* Laufvariablen */
  static long vorgaenger;  /* Vorgaenger eines evtl. zu vergroebernden Elements */
  static long max_index;  /* hoechster Elementindex */
  static int level, anzkant;  /* Verfeinerungslevel */

  int ok=1;
  int n=NodeListLength;
  char string[80];
  static double *node_geo = NULL;    /* node geometry */
  static double *node_value = NULL;  /* node_value = Wert an den Knoten */
  static long *element_nodes;
  static double node1_geo,node2_geo;
  int stop;

  long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */

  int j;
  long inode,jnode;
  double x1,y1,x2,y2;

  if ( mesh2d==0 ) return;
  rf_reset();
  stop=stop_if_time(aktuelle_zeit);

  if ( mesh2d > 0 ) {
     if ((aktueller_zeitschritt % mesh2d_cycle) == 0l) {
        if ( graf_clear == 1 ) rf_clear(draw_win);
        if ( graf_grid == 1 ) rf_grid(draw_win,graf_gdx,graf_gdy);
   for (i=0;i<ElListSize();i++) {  /* Schleife ueber alle Elemente */
           if (ElGetElement(i)!=NULL) { /* wenn Element existiert */
               if (ElGetElementActiveState(i)) {  /* nur aktive Elemente untersuchen */
                  if (ElGetElementType(i)==1) anzkant = 1;
                  if (ElGetElementType(i)==2) anzkant = 4;
                  kanten = ElGetElementEdges(i);  /* Kanten von number holen */
                  for (j=0;j<anzkant;j++) {  /* Schleife ueber alle Kanten */
                     kante = GetEdge(kanten[j]);
                     inode=kante->knoten[0];
                     jnode=kante->knoten[1];
                     /*
                     x1=GetNodeX(NodeNumber[inode]);
                     y1=GetNodeY(NodeNumber[inode]);
                     x2=GetNodeX(NodeNumber[jnode]);
                     y2=GetNodeY(NodeNumber[jnode]);
                     rf_line(draw_win,x1,y1,x2,y2);
                     */
            if (mesh2d_1c==1) {
               x1=GetNodeX(inode);
               x2=GetNodeX(jnode);
            }
            else if (mesh2d_1c==2) {
               x1=GetNodeY(inode);
               x2=GetNodeY(jnode);
            }
            else if (mesh2d_1c==3) {
               x1=GetNodeZ(inode);
               x2=GetNodeZ(jnode);
            }

            if (mesh2d_2c==1) {
               y1=GetNodeX(inode);
               y2=GetNodeX(jnode);
            }
            else if (mesh2d_2c==2) {
               y1=GetNodeY(inode);
               y2=GetNodeY(jnode);
            }
            else if (mesh2d_2c==3) {
               y1=GetNodeZ(inode);
               y2=GetNodeZ(jnode);
            }
            rf_line(draw_win,x1,y1,x2,y2);

                  }
               }
           }
        }
     }



   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,1,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,16,string,(int)strlen(string));
   sprintf(string,"ElementListLength        : %ld",ElementListLength);
   rf_text(out_win,10,31,string,(int)strlen(string));
   sprintf(string,"anz_active_2D         : %ld",anz_active_2D);
   rf_text(out_win,10,46,string,(int)strlen(string));


  if (stop) wait();
  }
  return ok;
}


/**************************************************************************/
/* ATM - Funktion: netz_3d
                                                                          */
/* Aufgabe:
   PLOTTEN DER FUNKTIONSWERTE AUF DEM BILDSCHIRM
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname ohne Extension
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1
                                                                          */
/* Programmaenderungen:
   01/1998     AH        Erste Version
                                                                          */
/**************************************************************************/
int netz_3d ( int first_c, int second_c )
{
  /* Variablen */
  static long i;  /* Laufvariablen */
  static long vorgaenger;  /* Vorgaenger eines evtl. zu vergroebernden Elements */
  static long max_index;  /* hoechster Elementindex */
  static int level, anzkant;  /* Verfeinerungslevel */

  int ok=1;
  int n=NodeListLength;
  char string[80];
  static double *node_geo = NULL;    /* node geometry */
  static double *node_value = NULL;  /* node_value = Wert an den Knoten */
  static long *element_nodes;
  static double node1_geo,node2_geo;
  int stop;

  long *kanten = NULL;  /* Feld mit Elementkanten */
  Kante *kante = NULL;  /* Eine Nachbarkante */
  long *flaechen = NULL;  /* Feld mit Elementflaechen */
  int fanz;  /* Anzahl der Flaechen */
  Flaeche *flaeche = NULL;  /* Eine (Nachbar-)Flaeche */

  int j,k;
  long inode,jnode;
  double x1,y1,z1,x2,y2,z2;
  double x1s,y1s,z1s,x2s,y2s,z2s;

  int width_old;

  if ( mesh3d==0 ) return;
  rf_reset();
  stop=stop_if_time(aktuelle_zeit);

  if ( mesh3d > 0 ) {
     if ((aktueller_zeitschritt % mesh3d_cycle) == 0l) {

        if ( graf_clear == 1 ) rf_clear(draw_win);
        if ( graf_grid == 1 ) rf_grid(draw_win,graf_gdx,graf_gdy);
   for (i=0;i<ElListSize();i++) {  /* Schleife ueber alle Elemente */
           if (ElGetElement(i)!=NULL) { /* wenn Element existiert */
               if (ElGetElementActiveState(i)) {  /* nur aktive Elemente anzeigen */
                  if (ElGetElementType(i)==1) anzkant = 1;
                  else if (ElGetElementType(i)==2) anzkant = 4;
                  else if (ElGetElementType(i)==3) anzkant = 12;

                  if      (ElGetElementType(i)==1) width_old=linwid(draw_win,3);
                  else if (ElGetElementType(i)==2) width_old=linwid(draw_win,2);
                  else if (ElGetElementType(i)==3) width_old=linwid(draw_win,1);

                  if (ElGetElementType(i)!=3) {
                  kanten = ElGetElementEdges(i);  /* Kanten von number holen */
                  for (j=0;j<anzkant;j++) {  /* Schleife ueber alle Kanten */
                     kante = GetEdge(kanten[j]);
                     inode=kante->knoten[0];
                     jnode=kante->knoten[1];

                     x1=GetNodeX(NodeNumber[inode]);
                     y1=GetNodeY(NodeNumber[inode]);
                     z1=GetNodeZ(NodeNumber[inode]);
                     x2=GetNodeX(NodeNumber[jnode]);
                     y2=GetNodeY(NodeNumber[jnode]);
                     z2=GetNodeZ(NodeNumber[jnode]);

                     dreh3d(1,mesh3d_alpha,x1,y1,z1,&x1s,&y1s,&z1s);
                     dreh3d(2,mesh3d_beta,x1s,y1s,z1s,&x1,&y1,&z1);
                     dreh3d(3,mesh3d_gamma,x1,y1,z1,&x1s,&y1s,&z1s);

                     dreh3d(1,mesh3d_alpha,x2,y2,z2,&x2s,&y2s,&z2s);
                     dreh3d(2,mesh3d_beta,x2s,y2s,z2s,&x2,&y2,&z2);
                     dreh3d(3,mesh3d_gamma,x2,y2,z2,&x2s,&y2s,&z2s);

                     if (mesh3d_1c==1) {
                        x1=x1s;
                        x2=x2s;
                     }
                     else if (mesh3d_1c==2) {
                        x1=y1s;
                        x2=y2s;
                     }
                     else if (mesh3d_1c==3) {
                        x1=z1s;
                        x2=z2s;
                     }

                     if (mesh3d_2c==1) {
                        y1=x1s;
                        y2=x2s;
                     }
                     else if (mesh3d_2c==2) {
                        y1=y1s;
                        y2=y2s;
                     }
                     else if (mesh3d_2c==3) {
                        y1=z1s;
                        y2=z2s;
                     }

                     rf_line(draw_win,x1,y1,x2,y2);
                     if ( mesh3d_numbering > 0 ) {
                       sprintf(string,"%ld",inode);
                       rf_text_win(draw_win,x1,y1,string,(int)strlen(string));
                       sprintf(string,"%ld",jnode);
                       rf_text_win(draw_win,x2,y2,string,(int)strlen(string));
                     }

                  } /* for (j=0;j<anzkant;j++) */
                  } /* if (ElGetElementType(i)!=3) */

                  else if (ElGetElementType(i)==3) {
                    linwid(draw_win,1);
                    fanz = ElGetElementPlainsNumber(i);
                    flaechen = ElGetElementPlains(i);

                    for (j=0; j<fanz; j++) {
                  flaeche = GetPlain(flaechen[j]);  /* Flaeche holen */

                  for (k=0;k<4;k++) {  /* Schleife ueber alle Knoten */
                     inode=flaeche->knoten[k];
                     if (k!=3) jnode=flaeche->knoten[k+1];
                     else      jnode=flaeche->knoten[0];

                     x1=GetNodeX(NodeNumber[inode]);
                     y1=GetNodeY(NodeNumber[inode]);
                     z1=GetNodeZ(NodeNumber[inode]);
                     x2=GetNodeX(NodeNumber[jnode]);
                     y2=GetNodeY(NodeNumber[jnode]);
                     z2=GetNodeZ(NodeNumber[jnode]);

                     dreh3d(1,mesh3d_alpha,x1,y1,z1,&x1s,&y1s,&z1s);
                     dreh3d(2,mesh3d_beta,x1s,y1s,z1s,&x1,&y1,&z1);
                     dreh3d(3,mesh3d_gamma,x1,y1,z1,&x1s,&y1s,&z1s);

                     dreh3d(1,mesh3d_alpha,x2,y2,z2,&x2s,&y2s,&z2s);
                     dreh3d(2,mesh3d_beta,x2s,y2s,z2s,&x2,&y2,&z2);
                     dreh3d(3,mesh3d_gamma,x2,y2,z2,&x2s,&y2s,&z2s);

                     if (mesh3d_1c==1) {
                        x1=x1s;
                        x2=x2s;
                     }
                     else if (mesh3d_1c==2) {
                        x1=y1s;
                        x2=y2s;
                     }
                     else if (mesh3d_1c==3) {
                        x1=z1s;
                        x2=z2s;
                     }

                     if (mesh3d_2c==1) {
                        y1=x1s;
                        y2=x2s;
                     }
                     else if (mesh3d_2c==2) {
                        y1=y1s;
                        y2=y2s;
                     }
                     else if (mesh3d_2c==3) {
                        y1=z1s;
                        y2=z2s;
                     }
                     rf_line(draw_win,x1,y1,x2,y2);
                     if ( mesh3d_numbering > 0 ) {
                       sprintf(string,"%ld",inode);
                       rf_text_win(draw_win,x1,y1,string,(int)strlen(string));
                       sprintf(string,"%ld",jnode);
                       rf_text_win(draw_win,x2,y2,string,(int)strlen(string));
                     }

                  } /* for (k=0;k<4;k++) */
                  } /* for (j=0; j<fanz; j++) */
               } /* else if (ElGetElementType(i)==3) */
               linwid(draw_win,width_old);

         }   /* if (ElGetElementActiveState(i)) */
       }   /* if (ElGetElement(i)!=NULL) */
     }   /* for (i=start_elements;i<ElListSize();i++) */
   }   /* if ((aktueller_zeitschritt % mesh3d_cycle) == 0l) */


   rf_clear(out_win);
   sprintf(string,"aktueller_zeitschritt : %ld",aktueller_zeitschritt);
   rf_text(out_win,10,1,string,(int)strlen(string));
   sprintf(string,"aktuelle_zeit         : %lf",aktuelle_zeit);
   rf_text(out_win,10,16,string,(int)strlen(string));
   sprintf(string,"ElementListLength        : %ld",ElementListLength);
   rf_text(out_win,10,31,string,(int)strlen(string));
   sprintf(string,"anz_active_3D         : %ld",anz_active_3D);
   rf_text(out_win,10,46,string,(int)strlen(string));


  if (stop) wait();
  }  /* if ( mesh3d > 0 ) */
  return ok;
}

void  dreh3d ( int achse, double winkel,
               double x, double y, double z,
               double *xs, double *ys, double *zs)
{
  double co=cos(winkel);
  double si=sin(winkel);

  /* Drehtrafo */
  if ( achse == 1 ) {
    *xs =  x;
    *ys =   y * co - z * si;
    *zs =   y * si + z * co;
  }
  else if ( achse == 2 ) {
    *xs =   x * co + z * si;
    *ys =   y;
    *zs = - x * si + z * co;
  }
  else if ( achse == 3 ) {
    *xs =   x * co - y * si;
    *ys =   x * si + y * co;
    *zs =   1;
  }

}

#else

int rf_grafic(int flag);

/* dummy function */
int rf_grafic(int flag)
{
  return flag;
}



#endif /* __RFGRAF */

/**************************************************************************/
/* C1.10 Grafik */
/**************************************************************************/

/**************************************************************************/
/* ATM - Funktion: FctBcurve
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort BCURVE gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   BCURVE: Durchbruchskurve-Grafik Parameter; standardmaessig dokumentiert
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
   08/1997   AH   Erste Version
   06/1999   OK   Direktes Daten-Protokoll (found==2)
                                                                          */
/**************************************************************************/
int FctBcurve(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;

    LineFeed(f);
    FilePrintString(f, "; 11 Grafics -----------------------------------------------------------");
    LineFeed(f);

#ifdef TESTFILES1
    DisplayMsgLn("Eingabedatenbank, Schluesselwort BCURVE");
#endif

    if (!found) {               /* BCURVE nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #BCURVE (Grafik Parameter)");
        LineFeed(f);
#endif
        bcurve = 0;
        bcurve_coor = 1;
        bcurve_value = 0;
        bcurve_xmin = 0.0;
        bcurve_xmax = 0.0;
        bcurve_ymin = 0.0;
        bcurve_ymax = 0.0;
        bcurve_cycle = 1;
    } else {                    /* BCURVE gefunden */
        FilePrintString(f, "#BCURVE");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #BCURVE (Grafik Parameter)");
        LineFeed(f);
        FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann die Grafik");
        LineFeed(f);
        FilePrintString(f, "; nicht beruecksichtigt.");
        LineFeed(f);
#endif
        /*--------------------------------------------------------------------*/
        if(found==1) {
            ok = (StrReadInt(&bcurve, &data[p], f, TFInt, &pos) && ok);
            ok = (StrReadInt(&bcurve_coor, &data[p += pos], f, TFInt, &pos) && ok);
            ok = (StrReadInt(&bcurve_value, &data[p += pos], f, TFInt, &pos) && ok);
        }
        else if (found==2) {
            fprintf(f," %ld ",(long)bcurve);
            fprintf(f," %ld ",(long)bcurve_coor);
            fprintf(f," %ld ",(long)bcurve_value);
        }
#ifdef EXT_RFD
        FilePrintString(f, "; - Grafik-Schalter (bcurve) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, ";   0 : ohne Grafik, 1 : mit Grafik.");
        LineFeed(f);
        FilePrintString(f, "; - Knotennummer (bcurve_coor) [..;1]");
        LineFeed(f);
        FilePrintString(f, "; - Index in der internen Datenstruktur fuer die Plot-Variable");
        LineFeed(f);
        FilePrintString(f, ";   (bcurve_value) [..;0]");
#else
        FilePrintString(f, "; bcurve, bcurve_coor, bcurve_value");
#endif
        LineFeed(f);
        /*--------------------------------------------------------------------*/
        if(found==1) {
            ok = (StrReadDouble(&bcurve_xmin, &data[p += pos], f, TFDouble, &pos) && ok);
            ok = (StrReadDouble(&bcurve_xmax, &data[p += pos], f, TFDouble, &pos) && ok);
            ok = (StrReadDouble(&bcurve_ymin, &data[p += pos], f, TFDouble, &pos) && ok);
            ok = (StrReadDouble(&bcurve_ymax, &data[p += pos], f, TFDouble, &pos) && ok);
        }
        else if (found==2) {
            fprintf(f," %e ",bcurve_xmin);
            fprintf(f," %e ",bcurve_xmax);
            fprintf(f," %e ",bcurve_ymin);
            fprintf(f," %e ",bcurve_ymax);
        }
#ifdef EXT_RFD
        FilePrintString(f, "; - Weltkoordinate xmin (bcurve_xmin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmax (bcurve_xmax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymin (bcurve_ymin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymax (bcurve_ymax) [..;0.0]");
#else
        FilePrintString(f, "; xmin, mmax, ymin, ymax");
#endif
        LineFeed(f);
        /*--------------------------------------------------------------------*/
        if(found==1) {
            ok = (StrReadInt(&bcurve_cycle, &data[p += pos], f, TFInt, &pos) && ok);
        }
        else if (found==2) {
            fprintf(f," %ld ", (long)bcurve_cycle);
        }
#ifdef EXT_RFD
        FilePrintString(f, "; - Zyklus der grafischen Ausgabe (bcurve_cycle) [..;0.0]");
#else
        FilePrintString(f, "; bcurve_cycle");
#endif
        LineFeed(f);
    }

    return ok;
}


#ifdef __RFGRAF
/* ah  b */
/**************************************************************************/
/* ATM - Funktion: FctGraf
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort GRAF gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   GRAF: 1D-Plot-Grafik Parameter; standardmaessig dokumentiert
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
   08/1997     AH         Erste Version
                                                                          */
/**************************************************************************/
int FctGraf(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
#ifdef TESTFILES1
    DisplayMsgLn("Eingabedatenbank, Schluesselwort GRAF");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif

    if (!found) {               /* GRAF nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #GRAF (Grafik Parameter)");
        LineFeed(f);
#endif
        grafic1d = 0;
        graf1d_coor = 1;
        graf1d_value = 0;
        graf_xmin = 0.0;
        graf_xmax = 0.0;
        graf_ymin = 0.0;
        graf_ymax = 0.0;
        graf_cycle = 1;
    } else {                    /* GRAF gefunden */
        FilePrintString(f, "#GRAF");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #GRAF (Grafik Parameter)");
        LineFeed(f);
        FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann die Grafik");
        LineFeed(f);
        FilePrintString(f, "; nicht beruecksichtigt.");
        LineFeed(f);
        FilePrintString(f, "; - Grafik-Schalter (grafic1d) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, ";   0 : ohne Grafik, 1 : mit Grafik.");
        LineFeed(f);
        FilePrintString(f, "; - Koordinatenindex fuer x-Achse (graf1d_coor) [1=x,2=y oder 3=z;1]");
        LineFeed(f);
        FilePrintString(f, "; - Index in der internen Datenstruktur fuer die Plot-Variable");
        LineFeed(f);
        FilePrintString(f, ";   (graf1d_value) [..;0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmin (graf_xmin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmax (graf_xmax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymin (graf_ymin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymax (graf_ymax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Zyklus der grafischen Ausgabe (graf_cycle) [..;0.0]");
        LineFeed(f);
#endif
        ok = (StrReadInt(&grafic1d, &data[p], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&graf1d_coor, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&graf1d_value, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadDouble(&graf_xmin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&graf_xmax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&graf_ymin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&graf_ymax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadInt(&graf_cycle, &data[p += pos], f, TFInt, &pos) && ok);

        LineFeed(f);
    }


    return ok;
}


/**************************************************************************/
/* ATM - Funktion: FctMesh2D
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort MESH2D gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   MESH2D: Netz2D-Plot Parameter; standardmaessig undokumentiert
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
   08/1997     AH         Erste Version
                                                                          */
/**************************************************************************/
int FctMesh2D(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
#ifdef TESTFILES1
    DisplayMsgLn("Eingabedatenbank, Schluesselwort MESH2D");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif

    if (!found) {               /* MESH2D nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #MESH2D (Netz2D Parameter)");
        LineFeed(f);
#endif
        mesh2d = 0;
        mesh2d_1c = 1;
        mesh2d_2c = 2;
        mesh2d_xmin = 0.0;
        mesh2d_xmax = 0.0;
        mesh2d_ymin = 0.0;
        mesh2d_ymax = 0.0;
        mesh2d_cycle = 1;
    } else {                    /* MESH2D gefunden */
        FilePrintString(f, "#MESH2D");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #MESH2D (Netz2D Parameter)");
        LineFeed(f);
        FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann die Grafik");
        LineFeed(f);
        FilePrintString(f, "; nicht beruecksichtigt.");
        LineFeed(f);
        FilePrintString(f, "; - Grafik-Schalter (mesh2d) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, ";   0 : ohne Grafik, 1 : mit Grafik.");
        LineFeed(f);
        FilePrintString(f, "; - Koordinatenindex fuer x-Achse (mesh2d_1c) [1=x,2=y oder 3=z;1]");
        LineFeed(f);
        FilePrintString(f, "; - Koordinatenindex fuer y-Achse (mesh2d_2c) [1=x,2=y oder 3=z;1]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmin (mesh2d_xmin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmax (mesh2d_xmax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymin (mesh2d_ymin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymax (mesh2d_ymax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Zyklus der grafischen Ausgabe (mesh2d_cycle) [..;0.0]");
        LineFeed(f);
#endif
        ok = (StrReadInt(&mesh2d, &data[p], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&mesh2d_1c, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&mesh2d_2c, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadDouble(&mesh2d_xmin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh2d_xmax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh2d_ymin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh2d_ymax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadInt(&mesh2d_cycle, &data[p += pos], f, TFInt, &pos) && ok);

        LineFeed(f);
    }


    return ok;
}


/**************************************************************************/
/* ATM - Funktion: FctMesh3D
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort MESH3D gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   MESH3D: Netz3D-Plot Parameter; standardmaessig undokumentiert
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
   01/1998     AH         Erste Version
                                                                          */
/**************************************************************************/
int FctMesh3D(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
#ifdef TESTFILES1
    DisplayMsgLn("Eingabedatenbank, Schluesselwort MESH3D");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif

    if (!found) {               /* MESH3D nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #MESH3D (Netz3D Parameter)");
        LineFeed(f);
#endif
        mesh3d = 0;
        mesh3d_1c = 1;
        mesh3d_2c = 2;
        mesh3d_alpha = 0.0;
        mesh3d_beta = 0.0;
        mesh3d_gamma = 0.0;
        mesh3d_xmin = 0.0;
        mesh3d_xmax = 0.0;
        mesh3d_ymin = 0.0;
        mesh3d_ymax = 0.0;
        mesh3d_numbering = 0;
        mesh3d_cycle = 1;
    } else {                    /* MESH3D gefunden */
        FilePrintString(f, "#MESH3D");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #MESH3D (Netz3D Parameter)");
        LineFeed(f);
        FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann die Grafik");
        LineFeed(f);
        FilePrintString(f, "; nicht beruecksichtigt.");
        LineFeed(f);
        FilePrintString(f, "; - Grafik-Schalter (mesh3d) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, ";   0 : ohne Grafik, 1 : mit Grafik.");
        LineFeed(f);
        FilePrintString(f, "; - Koordinatenindex fuer x-Achse des Plotts (mesh3d_1c) [1=x,2=y oder 3=z;1]");
        LineFeed(f);
        FilePrintString(f, "; - Koordinatenindex fuer y-Achse des Plotts (mesh3d_2c) [1=x,2=y oder 3=z;1]");
        LineFeed(f);
        FilePrintString(f, "; - Drehungswinkel um X-Achse (mesh3d_alpha) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Drehungswinkel um Y-Achse (mesh3d_beta) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Drehungswinkel um Z-Achse (mesh3d_gamma) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmin (mesh3d_xmin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate xmax (mesh3d_xmax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymin (mesh3d_ymin) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Weltkoordinate ymax (mesh3d_ymax) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Ausgabe der Knotennummer (mesh3d_numbering) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, ";   0 : ohne Nummer, 1 : mit Nummer.");
        LineFeed(f);
        FilePrintString(f, "; - Zyklus der grafischen Ausgabe (mesh3d_cycle) [..;0.0]");
        LineFeed(f);
#endif
        ok = (StrReadInt(&mesh3d, &data[p], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&mesh3d_1c, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&mesh3d_2c, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_alpha, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_beta, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_gamma, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_xmin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_xmax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_ymin, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadDouble(&mesh3d_ymax, &data[p += pos], f, TFDouble, &pos) && ok);
        ok = (StrReadInt(&mesh3d_numbering, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&mesh3d_cycle, &data[p += pos], f, TFInt, &pos) && ok);

        LineFeed(f);
    }


    return ok;
}


/**************************************************************************/
/* ATM - Funktion: FctGrafExtended
                                                                          */
/* Aufgabe:
   Liest die zu dem Schluesselwort GRAF_EXTD gehoerigen Daten ein und erstellt
   den zugehoerigen Protokollabschnitt.
   GRAF_EXTD: Grafik extra Parameter; standardmaessig dokumentiert
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
   08/1997     AH         Erste Version
                                                                          */
/**************************************************************************/
int FctGrafExtended(char *data, int found, FILE * f)
{
    int ok = 1;
    int pos = 0;
    int p = 0;
#ifdef TESTFILES1
    DisplayMsgLn("Eingabedatenbank, Schluesselwort GRAF_EXTD");
#endif

#ifdef EXT_RFD
    LineFeed(f);
#endif

    if (!found) {               /* GRAF_EXTD nicht gefunden */
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #GRAF_EXTD (Grafik extra Parameter)");
        LineFeed(f);
#endif
        graf_wait = 0;
        graf_clear = 1;
        graf_grid = 0;
        graf_gdx = 0.0;
        graf_gdy = 0.0;
    } else {                    /* GRAF_EXTD gefunden */
        FilePrintString(f, "#GRAF_EXTD");
        LineFeed(f);
#ifdef EXT_RFD
        FilePrintString(f, "; Schluesselwort: #GRAF_EXTD (Grafik extra Parameter)");
        LineFeed(f);
        FilePrintString(f, "; Das Schluesselwort muss nicht vorhanden sein, es wird dann die Grafik");
        LineFeed(f);
        FilePrintString(f, "; nicht beruecksichtigt.");
        LineFeed(f);
        FilePrintString(f, "; !!! Diese Parameter gelten fuer alle graphischen Ausgaben. !!!");
        LineFeed(f);
        FilePrintString(f, ";     0 : Nein, 1 : Ja");
        LineFeed(f);
        FilePrintString(f, "; - Halten nach jedem Zyklus ? (graf_wait) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, "; - Fenster vor jedem Zyklus loeschen ? (graf_clear) [0,1;1]");
        LineFeed(f);
        FilePrintString(f, "; - Gitter zeichnen ? (graf_grid) [0,1;0]");
        LineFeed(f);
        FilePrintString(f, "; - Gitterabstand in Weltkoordinaten fuer x-Richtung (graf_gdx) [..;0.0]");
        LineFeed(f);
        FilePrintString(f, "; - Gitterabstand in Weltkoordinaten fuer y-Richtung (graf_gdy) [..;0.0]");
        LineFeed(f);
#endif
    ok = (StrReadInt(&graf_wait, &data[p], f, TFInt, &pos) && ok);
    ok = (StrReadInt(&graf_clear, &data[p += pos], f, TFInt, &pos) && ok);
        ok = (StrReadInt(&graf_grid,&data[p+=pos],f,TFInt,&pos) && ok);
        ok = (StrReadDouble(&graf_gdx,&data[p+=pos],f,TFDouble,&pos) && ok);
        ok = (StrReadDouble(&graf_gdy,&data[p+=pos],f,TFDouble,&pos) && ok);

        LineFeed(f);
    }

    return ok;
}

/* ah e */
#endif /* __RFGRAF */

/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigGrafics
                                                                          */
/* Aufgabe:
   Konfigurationen fuer Grafik-Ausgabe
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   12/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
void ConfigGrafics(void)
{
#ifdef __RFGRAF
    rf_grafic(1);
#endif
}




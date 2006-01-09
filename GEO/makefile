# GeoSys - Makefile for LINUX
# Last modification 04/11/2004 WW

COBJS =geo_db.o geo_dom.o geo_lin.o geo_ply.o geo_pnt.o geo_sfc.o \
       geo_strings.o geo_vol.o geo_lib.o   

FEMINCLUDE=../FEM
GEOINCLUDE=../GEO
MSHINCLUDE=../MSH
GEN_INCLUDES = -I$(FEMINCLUDE) -I$(GEOINCLUDE) -I$(MSHINCLUDE)  

OPTFLAG = -O3 -fno-nonansi-builtins
DEBFLAG = #-ggdb
WARN = -Wall

C++ = g++ -c $(WARN) -pedantic $(GEN_INCLUDES) $(OPTFLAG) $(DEBFLAG) -D_POSIX_SOURCE -DNO_ERROR_CONTROL

PARAM = -lm
.SUFFIXES: .o .cpp .h 
.cpp.o:
	$(C++)   $<
 dest: $(COBJS)
clean:; rm -rf *.o

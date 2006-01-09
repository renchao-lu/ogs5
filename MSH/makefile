# GeoSys - Makefile for LINUX
# Last modification 26/08/2005 WW

COBJS = msh_pnt.o msh_nodes_rfi.o msh_node.o msh_mat.o\
        msh_lin.o msh_lib.o msh_gen_pris.o msh_gen.o\
        msh_elements_rfi.o msh_mesh.o msh_elem.o msh_edge.o msh_core.o

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

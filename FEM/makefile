# GeoSys - Makefile for LINUX
#  04.11.2004 WW
#  17.12.2008 WW
#  30.04.2009 WW
ifneq ($(strip $(CHOSEN_TARGET)),)
include $(TOPDIR)/configs/$(CHOSEN_TARGET)
endif

## 30.04.2009 WW------------------------
include objects.mak

ifdef NEW_SP
FEMOBJS += equation_class.o
endif
ifdef OLD_MX
FEMOBJS += matrix.o solver.o
endif
ifdef GEMS
FEMOBJS += rf_REACT_GEM.o
endif
FEMOBJS += ../gs_project.o
#######-----------------------------

FEMINCLUDE     = ../FEM
GEOINCLUDE     = ../GEO
MSHINCLUDE     = ../MSH
#UTLINCLUDE     = ../UTL
ROOTINCLUDE    = ../

## For different releases. 30.04.2009. WW
ifdef GEMS
GEMINCLUDE     = ../GEM
endif
ifdef MPI
MPIINCLUDE     = ../MPI
endif
ifdef DO_PQC
PQCINCLUDE     = ../PQC
endif
####--------------------------------

CXX_INCS      += -I$(FEMINCLUDE) -I$(GEOINCLUDE) -I$(MSHINCLUDE) -I$(ROOTINCLUDE)
ifdef MPI
CXX_INCS      += -I$(MPIINCLUDE)
endif
ifdef GEMS
CXX_INCS     += -I$(GEMINCLUDE)
endif
ifdef DO_PQC
CXX_INCS     += -I$(PQCINCLUDE)
endif

#### Only temporarliy until random walk compiles on every platform
## Commented by WW
#ifdef NO_RANDOM_WALK
#CXX_FLAGS       += -URANDOM_WALK
#else
#CXX_FLAGS       += -DRANDOM_WALK
#endif



.SUFFIXES: .o .cpp .h 
%.o:	%.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -c $<


.PHONY : $(CONFIG_NAMES)
$(CONFIG_NAMES): $(FEMOBJS)


distclean clean:
	rm -rf *.o
	rm -rf *~
	rm -rf *.ti


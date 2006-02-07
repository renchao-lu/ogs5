# GeoSys - Makefile for LINUX
# Last modification 04/11/2004 WW

ifneq ($(strip $(CHOSEN_TARGET)),)
include $(TOPDIR)/configs/$(CHOSEN_TARGET)
endif

include objects.mak

FEMINCLUDE     = ../FEM
GEOINCLUDE     = ../GEO
MSHINCLUDE     = ../MSH
MPIINCLUDE     = ../MPI
ROOTINCLUDE    = ../

CXX_INCS      += -I$(FEMINCLUDE) -I$(GEOINCLUDE) -I$(MSHINCLUDE) -I$(ROOTINCLUDE)
ifdef MPI
CXX_INCS      += -I$(MPIINCLUDE)
endif

#### Only temporarliy until random walk compiles on every platform

ifdef NO_RANDOM_WALK
CXX_FLAGS       += -URANDOM_WALK
endif


.SUFFIXES: .o .cpp .h 
%.o:	%.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCS) -c $<


.PHONY : $(CONFIG_NAMES)
$(CONFIG_NAMES): $(FEMOBJS)


distclean clean:
	rm -rf *.o
	rm -rf *~
	rm -rf *.ti

